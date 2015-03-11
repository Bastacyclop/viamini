#include "binary_heap.h"
#include "list.h"
#include "avl_tree.h"
#include "circuit.h"

static
Net net_from_file(FILE* f);
static
void point_from_line(Vec* points, const char* line);
static
void segment_from_line(Vec* segments, const char* line);
static
void check_net_segments(Net* net);

static
AABB compute_aabb(Vec* nets);
static
void AABB_include(AABB* aabb, Point p);

static
void drop_net(Net* net);

typedef struct {
    const Point* beg;
    const Point* end;
} SegmentRef;

static
bool segment_intersects(SegmentRef a, SegmentRef b, Point* sect);
static
bool hv_intersects(SegmentRef h, SegmentRef v, Point* sect);

typedef enum {
    H_SEGMENT_BEGIN,
    H_SEGMENT_END,
    V_SEGMENT
} BreakpointType;

typedef struct {
    SegmentLoc loc;
    SegmentRef ref;
} BreakpointData;

typedef struct {
    BreakpointType type;
    BreakpointData data;
} Breakpoint;

static
BinaryHeap sweep_init(const Circuit* c);
static
bool sweep_order(const Breakpoint* a, const Breakpoint* b);
static
int32_t Breakpoint_get_x(const Breakpoint* b);
static
void sweep_memorize(BinaryHeap* bh, SegmentLoc sl,
                    const Net* net, const Segment* segment);

static
void vec_sweep_comes_across(Vec* segments, BreakpointData* d);
static
void vec_sweep_goes_past(Vec* segments, const BreakpointData* d);
static
void vec_sweep_check_intersections(IntersectionVec* intersections,
                                   const Vec* segments, const BreakpointData* d);

static
void list_sweep_comes_across(List* segments, BreakpointData* d);
static
void list_sweep_goes_past(List* segments, const BreakpointData* d);
static
void list_sweep_check_intersections(IntersectionVec* intersections,
                                    const List* segments, const BreakpointData* d);

static
int8_t compare(const BreakpointData* a, const BreakpointData* b);
static
void avl_sweep_comes_across(AVLTree* segments, BreakpointData* d);
static
void avl_sweep_goes_past(AVLTree* segments, BreakpointData *d);
static
void avl_sweep_check_intersections(IntersectionVec* intersections,
                                   const AVLTree* segments, const BreakpointData* d);
static
const AVLNode* avl_find_sup_eq(const AVLNode* n, const BreakpointData* vd);
static
void avl_sweep_check_iter(IntersectionVec* intersections, const AVLNode* n,
                          const BreakpointData* vd);

static
GraphNode* new_graph_node(GraphNodeType t, void* d);

#define SYNTAX_ERROR(desc) {                        \
    perror("circuit file syntax error: "desc"\n");  \
    exit(1);                                        \
}

Circuit Circuit_from_file(const char* path) {
    FILE* f = fopen(path, "r");
    if (!f) {
        perror("cannot read circuit input file");
        exit(1);
    }

    char line[255];
    if (!fgets(line, 255, f)) {
        SYNTAX_ERROR("expected first line");
    }

    size_t net_count;
    if (sscanf(line, "%zu", &net_count) != 1) {
        SYNTAX_ERROR("expected net count on first line");
    }

    Vec nets = Vec_with_capacity(net_count, sizeof(Net));

    for (size_t i = 0; i < net_count; i++) {
        Net n = net_from_file(f);
        Vec_push(&nets, &n);
    }

    fclose(f);

    return (Circuit) {
        .nets = nets,
        .aabb = compute_aabb(&nets)
    };
}

Net net_from_file(FILE* f) {
    char line[255];
    if (!fgets(line, 255, f)) {
        SYNTAX_ERROR("expected net description");
    }

    size_t point_count, segment_count;
    if (sscanf(line, "%*u %zu %zu", &point_count, &segment_count) != 2) {
        SYNTAX_ERROR("expected `net_number point_count segment_count` net description");
    }

    Net n = {
        .points = Vec_with_capacity(point_count, sizeof(Point)),
        .segments = Vec_with_capacity(segment_count, sizeof(Segment))
    };

    for (size_t i = 0; i < point_count; i++) {
        if (!fgets(line, 255, f)) {
            SYNTAX_ERROR("expected point description");
        }
        point_from_line(&n.points, line);
    }

    for (size_t i = 0; i < segment_count; i++) {
        if (!fgets(line, 255, f)) {
            SYNTAX_ERROR("expected segment description");
        }
        segment_from_line(&n.segments, line);
    }

    check_net_segments(&n);

    return n;
}

void point_from_line(Vec* points, const char* line) {
    Point p;
    if (sscanf(line, "%*u %d %d", &p.x, &p.y) != 2) {
        SYNTAX_ERROR("expected `point_number point_x point_y` point description");
    }
    Vec_push(points, &p);
}

void segment_from_line(Vec* segments, const char* line) {
    Segment s;
    if (sscanf(line, "%zu %zu", &s.beg, &s.end) != 2) {
        SYNTAX_ERROR("expected `segment_begin segment_end` segment description");
    }
    Vec_push(segments, &s);
}

void check_net_segments(Net* net) {
    size_t segment_count = Vec_len(&net->segments);
    for (size_t i = 0; i < segment_count; i++) {
        Segment* s = Vec_get_mut(&net->segments, i);
        const Point* beg = Vec_get(&net->points, s->beg);
        const Point* end = Vec_get(&net->points, s->end);

        assert((beg->x == end->x) || (beg->y == end->y));
        if ((end->x < beg->x) || (end->y < beg->y)) {
            mem_swap(&s->beg, &s->end, sizeof(size_t));
        }
    }
}

AABB compute_aabb(Vec* nets) {
    // Will crash if there is no point in the first net, seems legit.
    Point first = *(const Point*)Vec_get(&((const Net*)Vec_get(nets, 0))->points, 0);
    AABB aabb = (AABB) { .inf = first, .sup = first };

    size_t net_count = Vec_len(nets);
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(nets, n);

        size_t point_count = Vec_len(&net->points);
        for (size_t p = 0; p < point_count; p++) {
            const Point* point = Vec_get(&net->points, p);

            AABB_include(&aabb, *point);
        }
    }

    return aabb;
}

void AABB_include(AABB* aabb, Point p) {
    if (p.x < aabb->inf.x) {
        aabb->inf.x = p.x;
    } else if (p.x > aabb->sup.x) {
        aabb->sup.x = p.x;
    }
    if (p.y < aabb->inf.y) {
        aabb->inf.y = p.y;
    } else if (p.y > aabb->sup.y) {
        aabb->sup.y = p.y;
    }
}

void Circuit_drop(Circuit* c) {
    Vec_drop(&c->nets, (void (*)(void*))drop_net);
}

void drop_net(Net* net) {
    Vec_plain_drop(&net->points);
    Vec_plain_drop(&net->segments);
}

void Circuit_print(const Circuit* c) {
    size_t net_count = Vec_len(&c->nets);
    printf("Net count: %zu\n", net_count);

    for (size_t i = 0; i < net_count; i++) {
        const Net* net = Vec_get(&c->nets, i);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);
        printf("Net: %zu %zu %zu\n", i, point_count, segment_count);

        for (size_t p = 0; p < point_count; p++) {
            const Point* point = Vec_get(&net->points, p);
            printf("  Point: %zu %d %d\n", p, point->x, point->y);
        }

        for (size_t s = 0; s < segment_count; s++) {
            const Segment* segment = Vec_get(&net->segments, s);
            printf("  Segment: %zu %zu\n", segment->beg, segment->end);
        }
    }
}


bool segment_intersects(SegmentRef a, SegmentRef b, Point* sect) {
    if (a.beg->x == a.end->x) { // |
        if (b.beg->x == b.end->x) { // | & |
            return false;
       } else { // | & -
            return hv_intersects(b, a, sect);
       }
    } else { // -
        if (b.beg->y == b.end->y) { // - & -
            return false;
        } else { // - & |
            return hv_intersects(a, b, sect);
        }
    }
}

bool hv_intersects(SegmentRef h, SegmentRef v, Point* sect) {
    if (h.beg->x <= v.beg->x && v.beg->x <= h.end->x &&
        v.beg->y <= h.beg->y && h.beg->y <= v.end->y) {

        sect->x = v.beg->x;
        sect->y = h.beg->y;
        return true;
    }
    return false;
}

IntersectionVec Circuit_intersections_naive(const Circuit* c) {
    IntersectionVec intersections = Vec_new(sizeof(Intersection));
    size_t net_count = Vec_len(&c->nets);

    for (size_t i = 0; i < net_count; i++) {
        const Net* a_net = Vec_get(&c->nets, i);
        size_t a_segment_count = Vec_len(&a_net->segments);

        for (size_t s = 0; s < a_segment_count; s++) {
            const Segment* a = Vec_get(&a_net->segments, s);
            SegmentRef a_ref = {
                .beg = Vec_get(&a_net->points, a->beg),
                .end = Vec_get(&a_net->points, a->end)
            };

            // Internet intersections
            for (size_t j = i + 1; j < net_count; j++) {
                const Net* b_net = Vec_get(&c->nets, j);
                size_t b_segment_count = Vec_len(&b_net->segments);

                for (size_t t = 0; t < b_segment_count; t++) {
                    const Segment* b = Vec_get(&b_net->segments, t);
                    SegmentRef b_ref = {
                        .beg = Vec_get(&b_net->points, b->beg),
                        .end = Vec_get(&b_net->points, b->end)
                    };

                    Point sect;
                    if (segment_intersects(a_ref, b_ref, &sect)) {
                        Intersection intersection = {
                            .a = { .net = i, .seg = s },
                            .b = { .net = j, .seg = t },
                            .sect = sect
                        };
                        Vec_push(&intersections, &intersection);
                    }
                }
            }
        }
    }

    return intersections;
}


IntersectionVec Circuit_intersections_vec_sweep(const Circuit* c) {
    BinaryHeap breakpoints = sweep_init(c);
    IntersectionVec intersections = Vec_new(sizeof(Intersection));
    Vec segments = Vec_new(sizeof(BreakpointData));

    Breakpoint breakpoint;
    while (BinaryHeap_pop(&breakpoints, &breakpoint)) {
        switch (breakpoint.type) {
            case H_SEGMENT_BEGIN:
                vec_sweep_comes_across(&segments, &breakpoint.data);
                break;
            case H_SEGMENT_END:
                vec_sweep_goes_past(&segments, &breakpoint.data);
                break;
            case V_SEGMENT:
                vec_sweep_check_intersections(&intersections, &segments,
                                              &breakpoint.data);
                break;
        }
    }

    Vec_plain_drop(&segments);
    BinaryHeap_plain_drop(&breakpoints);

    return intersections;
}

BinaryHeap sweep_init(const Circuit* c) {
    BinaryHeap breakpoints = BinaryHeap_new(sizeof(Breakpoint),
        (bool (*)(const void*, const void*))sweep_order);

    size_t net_count = Vec_len(&c->nets);
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&c->nets, n);

        size_t segment_count = Vec_len(&net->segments);
        for (size_t s = 0; s < segment_count; s++) {
            const Segment* segment = Vec_get(&net->segments, s);

            sweep_memorize(&breakpoints, (SegmentLoc) { .net = n, .seg = s },
                           net, segment);
        }
    }

    return breakpoints;
}

bool sweep_order(const Breakpoint* a, const Breakpoint* b) {
    int32_t x_a = Breakpoint_get_x(a);
    int32_t x_b = Breakpoint_get_x(b);

    if (x_a == x_b) {
        BreakpointType t_a = a->type;
        BreakpointType t_b = b->type;

        if (t_a == V_SEGMENT) {
            return t_b == H_SEGMENT_END;
        } else if (t_a == H_SEGMENT_BEGIN) {
            return t_b == V_SEGMENT;
        }

        return false;
    }

    return x_a < x_b;
}

int32_t Breakpoint_get_x(const Breakpoint* b) {
    BreakpointType t = b->type;
    if (t == H_SEGMENT_BEGIN) {
        return b->data.ref.beg->x;
    } else {
        return b->data.ref.end->x;
    }
}

void sweep_memorize(BinaryHeap* bh, SegmentLoc sl,
                    const Net* net, const Segment* segment) {
    const Point* beg = Vec_get(&net->points, segment->beg);
    const Point* end = Vec_get(&net->points, segment->end);
    BreakpointData data = { .loc = sl, { .beg = beg, .end = end } };

    if (beg->x == end->x) { // |
        Breakpoint breakpoint = { .type = V_SEGMENT, .data = data };
        BinaryHeap_push(bh, &breakpoint);
    } else { // -
        Breakpoint breakpoint = { .type = H_SEGMENT_BEGIN, .data = data };
        BinaryHeap_push(bh, &breakpoint);
        breakpoint.type = H_SEGMENT_END;
        BinaryHeap_push(bh, &breakpoint);
    }
}

void vec_sweep_comes_across(Vec* segments, BreakpointData* d) {
    Vec_push(segments, d);
}

void vec_sweep_goes_past(Vec* segments, const BreakpointData* d) {
    size_t seg_count = Vec_len(segments);
    for (size_t i = 0; i < seg_count; i++) {
        const BreakpointData* d_i = Vec_get(segments, i);

        if (memcmp(&d->loc, &d_i->loc, sizeof(SegmentLoc)) == 0) {
            Vec_swap_remove(segments, i, NULL);
            return;
        }
    }
}

void vec_sweep_check_intersections(IntersectionVec* intersections,
                               const Vec* segments, const BreakpointData* vd) {
    size_t seg_count = Vec_len(segments);
    for (size_t i = 0; i < seg_count; i++) {
        const BreakpointData* hd = Vec_get(segments, i);
        int32_t hy = hd->ref.beg->y;

        if (hd->loc.net != vd->loc.net) {
            if (vd->ref.beg->y <= hy && hy <= vd->ref.end->y) {
                Intersection intersection = {
                    .a = vd->loc, .b = hd->loc,
                    .sect = { vd->ref.beg->x, hy }
                };
                Vec_push(intersections, &intersection);
            }
        }
    }
}


IntersectionVec Circuit_intersections_list_sweep(const Circuit* c) {
    BinaryHeap breakpoints = sweep_init(c);
    IntersectionVec intersections = Vec_new(sizeof(Intersection));
    List segments = List_new(sizeof(BreakpointData));

    Breakpoint breakpoint;
    while (BinaryHeap_pop(&breakpoints, &breakpoint)) {
        switch (breakpoint.type) {
            case H_SEGMENT_BEGIN:
                list_sweep_comes_across(&segments, &breakpoint.data);
                break;
            case H_SEGMENT_END:
                list_sweep_goes_past(&segments, &breakpoint.data);
                break;
            case V_SEGMENT:
                list_sweep_check_intersections(&intersections, &segments,
                                               &breakpoint.data);
                break;
        }
    }

    List_plain_clear(&segments);
    BinaryHeap_plain_drop(&breakpoints);

    return intersections;
}

void list_sweep_comes_across(List* segments, BreakpointData* d) {
    int32_t y = d->ref.beg->y;

    ListNode* n = List_front_node_mut(segments);
    if (!n) {
        List_push(segments, d);
    } else {
        const BreakpointData* nd = ListNode_elem(n);
        int32_t ny = nd->ref.beg->y;

        if (y <= ny) {
            List_push(segments, d);
        } else {
            ListNode* p = n;
            n = ListNode_next_mut(n);

            while (n) {
                nd = ListNode_elem(n);
                ny = nd->ref.beg->y;
                if (y <= ny) {
                    break;
                }

                p = n;
                n = ListNode_next_mut(n);
            }

            List_insert(segments, p, d);
        }
    }
}

void list_sweep_goes_past(List* segments, const BreakpointData* d) {
    ListNode* n = List_front_node_mut(segments);
    if (n) {
        const BreakpointData* nd = ListNode_elem(n);

        if (memcmp(&nd->loc, &d->loc, sizeof(SegmentLoc)) == 0) {
            List_pop(segments, NULL);
        } else {
            ListNode* p = n;
            n = ListNode_next_mut(n);

            while (n) {
                nd = ListNode_elem(n);
                if (memcmp(&nd->loc, &d->loc, sizeof(SegmentLoc)) == 0) {
                    List_remove(segments, p);
                    return;
                }

                p = n;
                n = ListNode_next_mut(n);
            }
        }
    }
}

void list_sweep_check_intersections(IntersectionVec* intersections,
                                    const List* segments, const BreakpointData* vd) {
    int32_t y_min = vd->ref.beg->y;
    int32_t y_max = vd->ref.end->y;

    const ListNode* n = List_front_node(segments);

    while (n) {
        const BreakpointData* hd = ListNode_elem(n);
        int32_t hy = hd->ref.beg->y;

        if (hy >= y_min) break;

        n = ListNode_next(n);
    }

    while (n) {
        const BreakpointData* hd = ListNode_elem(n);
        int32_t hy = hd->ref.beg->y;

        if (hy > y_max) break;

        if (hd->loc.net != vd->loc.net) {
            Intersection intersection = {
                .a = vd->loc, .b = hd->loc,
                .sect = { vd->ref.beg->x, hy }
            };
            Vec_push(intersections, &intersection);
        }

        n = ListNode_next(n);
    }
}

IntersectionVec Circuit_intersections_avl_sweep(const Circuit* c) {
    BinaryHeap breakpoints = sweep_init(c);
    IntersectionVec intersections = Vec_new(sizeof(Intersection));
    AVLTree segments = AVLTree_new(sizeof(BreakpointData),
                                   (int8_t (*)(const void*, const void*))compare);

    Breakpoint breakpoint;
    while (BinaryHeap_pop(&breakpoints, &breakpoint)) {
        switch (breakpoint.type) {
            case H_SEGMENT_BEGIN:
                avl_sweep_comes_across(&segments, &breakpoint.data);
                break;
            case H_SEGMENT_END:
                avl_sweep_goes_past(&segments, &breakpoint.data);
                break;
            case V_SEGMENT:
                avl_sweep_check_intersections(&intersections, &segments,
                                              &breakpoint.data);
                break;
        }
    }

    AVLTree_plain_clear(&segments);
    BinaryHeap_plain_drop(&breakpoints);

    return intersections;
}

int8_t compare(const BreakpointData* a, const BreakpointData* b) {
    int32_t y_a = a->ref.beg->y;
    int32_t y_b = b->ref.beg->y;

    if (y_a < y_b) {
        return -1;
    } else if (y_a > y_b) {
        return  1;
    } else {
        SegmentLoc l_a = a->loc;
        SegmentLoc l_b = b->loc;

        if (l_a.net < l_b.net) {
            return -1;
        } else if (l_a.net > l_b.net) {
            return  1;
        } else if (l_a.seg < l_b.seg) {
            return -1;
        } else if (l_a.seg > l_b.seg) {
            return  1;
        } else {
            return  0;
        }
    }
}

void avl_sweep_comes_across(AVLTree* segments, BreakpointData* d) {
    AVLTree_insert(segments, d);
}

void avl_sweep_goes_past(AVLTree* segments, BreakpointData* d) {
    AVLTree_remove(segments, d, NULL);
}

void avl_sweep_check_intersections(IntersectionVec* intersections,
                                   const AVLTree* segments, const BreakpointData* vd) {
    const AVLNode* n = avl_find_sup_eq(segments->root, vd);
    // y >= y_min, but not y <= y_max.
    avl_sweep_check_iter(intersections, n, vd);
}

const AVLNode* avl_find_sup_eq(const AVLNode* n, const BreakpointData* vd) {
    if (n) {
        int32_t y_min = vd->ref.beg->y;
        int32_t y = ((BreakpointData*)n->elem)->ref.beg->y;

        if (y < y_min) {
            n = avl_find_sup_eq(n->right, vd);
        }
    }
    return n;
}

void avl_sweep_check_iter(IntersectionVec* intersections, const AVLNode* n,
                          const BreakpointData* vd) {
    if (n) {
        int32_t y_min = vd->ref.beg->y;
        int32_t y_max = vd->ref.end->y;
        const BreakpointData* hd = n->elem;
        int32_t y = hd->ref.beg->y;

        if (y < y_min) {
            avl_sweep_check_iter(intersections, n->right, vd);
        } else if (y > y_max) {
            avl_sweep_check_iter(intersections, n->left, vd);
        } else {
            if (vd->loc.net != hd->loc.net) {
                Intersection i = {
                    .a = vd->loc, .b = hd->loc,
                    .sect = { vd->ref.beg->x, hd->ref.beg->y }
                };
                Vec_push(intersections, &i);
            }

            avl_sweep_check_iter(intersections, n->right, vd);
            avl_sweep_check_iter(intersections, n->left, vd);
        }
    }
}

GraphNode* new_graph_node(GraphNodeType t, void* d) {
    GraphNode* n = malloc(sizeof(GraphNode));
    assert_alloc(n);

    *n = (GraphNode) {
        .type = t, .data = d,
        .continuity = Vec_new(sizeof(GraphNode)),
        .conflict = Vec_new(sizeof(GraphNode))
    };
    return n;
}

Graph Graph_new(const Circuit* c, const IntersectionVec* inters) {
    size_t net_count = Vec_len(&c->nets);
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&c->nets, n);

        size_t point_count = Vec_len(&net->points);
        for (size_t p = 0; p < point_count; p++) {

        }

        size_t seg_count = Vec_len(&net->points);
        for (size_t s = 0; s < seg_count; s++) {

        }
    }

    size_t inter_count = Vec_len(inters);
    for (size_t i = 0; i < inter_count; i++) {

    }

    return (Graph) {
        .n = new_graph_node(POINT_NODE, NULL)
    };
}
