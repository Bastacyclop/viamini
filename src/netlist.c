#include "binary_heap.h"
#include "list.h"
#include "avl_tree.h"

#include "netlist.h"

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
BinaryHeap sweep_init(const Netlist* nl);
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
void graph_node_drop(GraphNode* n);

typedef enum {
    UNVISITED_NODE,
    A_OPENED_NODE,
    B_OPENED_NODE,
    CLOSED_NODE
} NodeMark;

static
Vec Graph_find_odd_cycle(const Graph* g, Vec* marks);
static
Vec new_marks(size_t node_count);
static
void add_via_with_cycle(BitSet* solution, const Vec* cycle, const Graph* g);
static
void reset_marks(Vec* marks, const BitSet* solution, const Graph* g);
static
bool Graph_find_odd_cycle_from(const Graph* g, size_t root, Vec* rev_path,
                               Vec* marks, NodeMark mark);
static
Vec rev_path_into_cycle_nodes(Vec rev_path);
static
void Graph_solve_faces(const Graph* g, BitSet* solution);
static
void Graph_solve_faces_from(const Graph* g, size_t root, BitSet* solution,
                            BitSet* visited, bool face);

#define SYNTAX_ERROR(desc) {                        \
    perror("circuit file syntax error: "desc"\n");  \
    exit(1);                                        \
}

Netlist Netlist_from_file(const char* path) {
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

    return (Netlist) {
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

void Netlist_drop(Netlist* nl) {
    Vec_drop_with(&nl->nets, (void (*)(void*))drop_net);
}

void drop_net(Net* net) {
    Vec_drop(&net->points);
    Vec_drop(&net->segments);
}

void Netlist_print(const Netlist* nl) {
    size_t net_count = Vec_len(&nl->nets);
    printf("Net count: %zu\n", net_count);

    for (size_t i = 0; i < net_count; i++) {
        const Net* net = Vec_get(&nl->nets, i);
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

size_t Netlist_segment_count(const Netlist* nl) {
    size_t count = 0;

    size_t net_count = Vec_len(&nl->nets);
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);

        count += Vec_len(&net->segments);
    }

    return count;
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

IntersectionVec Netlist_intersections_naive(const Netlist* nl) {
    IntersectionVec intersections = Vec_new(sizeof(Intersection));

    size_t net_count = Vec_len(&nl->nets);
    for (size_t i = 0; i < net_count; i++) {
        const Net* a_net = Vec_get(&nl->nets, i);

        size_t a_segment_count = Vec_len(&a_net->segments);
        for (size_t s = 0; s < a_segment_count; s++) {
            const Segment* a = Vec_get(&a_net->segments, s);
            SegmentRef a_ref = {
                .beg = Vec_get(&a_net->points, a->beg),
                .end = Vec_get(&a_net->points, a->end)
            };

            // Internet intersections
            for (size_t j = i + 1; j < net_count; j++) {
                const Net* b_net = Vec_get(&nl->nets, j);

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


IntersectionVec Netlist_intersections_vec_sweep(const Netlist* nl) {
    BinaryHeap breakpoints = sweep_init(nl);
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

    Vec_drop(&segments);
    BinaryHeap_drop(&breakpoints);

    return intersections;
}

BinaryHeap sweep_init(const Netlist* nl) {
    BinaryHeap breakpoints = BinaryHeap_new(sizeof(Breakpoint),
        (bool (*)(const void*, const void*))sweep_order);

    size_t net_count = Vec_len(&nl->nets);
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);

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


IntersectionVec Netlist_intersections_list_sweep(const Netlist* nl) {
    BinaryHeap breakpoints = sweep_init(nl);
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

    List_clear(&segments);
    BinaryHeap_drop(&breakpoints);

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

IntersectionVec Netlist_intersections_avl_sweep(const Netlist* nl) {
    BinaryHeap breakpoints = sweep_init(nl);
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

    AVLTree_clear(&segments);
    BinaryHeap_drop(&breakpoints);

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

void Netlist_intersections_to_file(IntersectionVec* inters, const char* path) {
    FILE* f = fopen(path, "w");

    size_t inter_count = Vec_len(inters);
    for (size_t i = 0; i < inter_count; i++) {
        const Intersection* inter = Vec_get(inters, i);

        fprintf(f, "%zu %zu %zu %zu\n",
                inter->a.net, inter->a.seg,
                inter->b.net, inter->b.seg);
    }

    fclose(f);
}

Graph Graph_new(const Netlist* nl, const char* int_path) {
    size_t nodes_count = 0;

    size_t net_count = Vec_len(&nl->nets);
    Vec net_offsets = Vec_with_capacity(net_count, sizeof(size_t));
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);
        Vec_push(&net_offsets, &nodes_count);

        nodes_count += Vec_len(&net->points);
        nodes_count += Vec_len(&net->segments);
    }

    GraphNodeVec nodes = Vec_with_capacity(nodes_count, sizeof(GraphNode));

    for (size_t n = 0; n < net_count; n++) {
        size_t net_offset = Vec_len(&nodes);
        const Net* net = Vec_get(&nl->nets, n);

        size_t point_count = Vec_len(&net->points);
        for (size_t p = 0; p < point_count; p++) {
            GraphNode node = {
                .type = POINT_NODE,
                .point = Vec_get(&net->points, p),
                .continuity = Vec_new(sizeof(GraphEdge)),
                .conflict = Vec_new(sizeof(GraphEdge))
            };
            Vec_push(&nodes, &node);
        }

        size_t segment_count = Vec_len(&net->segments);
        for (size_t s = 0; s < segment_count; s++) {
            GraphNode node = {
                .type = SEGMENT_NODE,
                .segment = Vec_get(&net->segments, s),
                .continuity = Vec_new(sizeof(GraphEdge)),
                .conflict = Vec_new(sizeof(GraphEdge))
            };
            size_t node_index = Vec_len(&nodes);
            size_t beg_index = (net_offset + node.segment->beg);
            size_t end_index = (net_offset + node.segment->end);
            GraphEdge from_beg = { .u = beg_index,  .v = node_index };
            GraphEdge to_beg   = { .u = node_index, .v = beg_index  };
            GraphEdge from_end = { .u = end_index,  .v = node_index };
            GraphEdge to_end   = { .u = node_index, .v = end_index  };
            Vec_push(&node.continuity, &to_beg);
            Vec_push(&node.continuity, &to_end);
            GraphNode* beg = Vec_get_mut(&nodes, beg_index);
            GraphNode* end = Vec_get_mut(&nodes, end_index);
            Vec_push(&beg->continuity, &from_beg);
            Vec_push(&end->continuity, &from_end);
            Vec_push(&nodes, &node);
        }
    }

    FILE* int_f = fopen(int_path, "r");

    char line[255];
    while (fgets(line, 255, int_f)) {
        SegmentLoc a_loc, b_loc;
        if (sscanf(line, "%zu %zu %zu %zu",
                   &a_loc.net, &a_loc.seg, &b_loc.net, &b_loc.seg) != 4) {
            SYNTAX_ERROR("expected `a_net a_seg b_net b_seg` intersection description");
        }

        size_t a_net_offset = *(const size_t*)Vec_get(&net_offsets, a_loc.net);
        size_t b_net_offset = *(const size_t*)Vec_get(&net_offsets, b_loc.net);
        const Net* a_net = Vec_get(&nl->nets, a_loc.net);
        const Net* b_net = Vec_get(&nl->nets, b_loc.net);
        size_t a_index = a_net_offset + Vec_len(&a_net->points) + a_loc.seg;
        size_t b_index = b_net_offset + Vec_len(&b_net->points) + b_loc.seg;
        GraphEdge ab_conflict = { .u = a_index, .v = b_index };
        GraphEdge ba_conflict = { .u = b_index, .v = a_index };
        GraphNode* a = Vec_get_mut(&nodes, a_index);
        GraphNode* b = Vec_get_mut(&nodes, b_index);
        Vec_push(&a->conflict, &ab_conflict);
        Vec_push(&b->conflict, &ba_conflict);
    }

    fclose(int_f);

    return (Graph) {
        .nodes = nodes,
        .net_offsets = net_offsets
    };
}

void Graph_drop(Graph* g) {
    Vec_drop_with(&g->nodes, (void (*)(void*))graph_node_drop);
    Vec_drop(&g->net_offsets);
}

void graph_node_drop(GraphNode* n) {
    Vec_drop(&n->continuity);
    Vec_drop(&n->conflict);
}

BitSet Graph_hv_solve(const Graph* g, const Netlist* nl) {
    // point: false -> no via
    //        true -> via
    // segment: false -> face A
    //          true -> face B
    BitSet solution = BitSet_with_capacity(Vec_len(&g->nodes));

    size_t offset = 0;
    size_t net_count = Vec_len(&nl->nets);
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);
        offset += point_count;

        for (size_t i = offset; i < (offset + segment_count); i++) {
            const GraphNode* node = Vec_get(&g->nodes, i);
            const Point* beg = Vec_get(&net->points, node->segment->beg);
            const Point* end = Vec_get(&net->points, node->segment->end);
            if (beg->x == end->x) { // |
                BitSet_insert(&solution, i);
            } // else -,  is already false
        }
        offset += segment_count;
    }

    offset = 0;
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);

        for (size_t i = offset; i < (offset + point_count); i++) {
            const GraphNode* node = Vec_get(&g->nodes, i);
            size_t continuity_count = Vec_len(&node->continuity);
            if (continuity_count > 0) {
                const GraphEdge* first_edge = Vec_get(&node->continuity, 0);
                bool first_face = BitSet_contains(&solution, first_edge->v);

                for (size_t c = 1; c < continuity_count; c++) {
                    const GraphEdge* edge = Vec_get(&node->continuity, c);
                    bool face = BitSet_contains(&solution, edge->v);
                    if (face != first_face) { // a via is needed
                        BitSet_insert(&solution, i);
                        break;
                    }
                }
            }
        }
        offset += point_count + segment_count;
    }

    return solution;
}

BitSet Graph_odd_cycle_solve(const Graph* g) {
    size_t node_count = Vec_len(&g->nodes);
    BitSet solution = BitSet_with_capacity(node_count);

    Vec marks = new_marks(node_count);

    Vec cycle = Graph_find_odd_cycle(g, &marks);
    while (!Vec_is_empty(&cycle)) {
        add_via_with_cycle(&solution, &cycle, g);
        Vec_drop(&cycle);

        reset_marks(&marks, &solution, g);
        cycle = Graph_find_odd_cycle(g, &marks);
    }
    Vec_drop(&cycle);

    Vec_drop(&marks);

    Graph_solve_faces(g, &solution);

    return solution;
}

Vec new_marks(size_t node_count) {
    Vec marks = Vec_with_capacity(node_count, sizeof(NodeMark));
    NodeMark u = UNVISITED_NODE;
    for (size_t n = 0; n < node_count; n++) {
        Vec_push(&marks, &u);
    }
    return marks;
}

void add_via_with_cycle(BitSet* solution, const Vec* cycle, const Graph* g) {
    size_t len = Vec_len(cycle);
    for (size_t i = 0; i < len; i++) {
        size_t c = *(const size_t*)Vec_get(cycle, i);
        const GraphNode* node = Vec_get(&g->nodes, c);
        if (node->type == POINT_NODE) {
            assert(BitSet_insert(solution, c));
            return;
        }
    }
    assert(false);
}

void reset_marks(Vec* marks, const BitSet* solution, const Graph* g) {
    size_t len = Vec_len(marks);
    for (size_t n = 0; n < len; n++) {
        const GraphNode* node = Vec_get(&g->nodes, n);
        NodeMark* m = Vec_get_mut(marks, n);
        if (node->type == POINT_NODE && BitSet_contains(solution, n)) {
            *m = CLOSED_NODE;
        } else {
            *m = UNVISITED_NODE;
        }
    }
}

Vec Graph_find_odd_cycle(const Graph* g, Vec* marks) {
    size_t node_count = Vec_len(&g->nodes);
    Vec rev_path = Vec_new(sizeof(size_t));

    for (size_t n = 0; n < node_count; n++) {
        NodeMark m = *(const NodeMark*)Vec_get(marks, n);
        if (m == UNVISITED_NODE &&
            Graph_find_odd_cycle_from(g, n, &rev_path, marks, A_OPENED_NODE)) {

            return rev_path_into_cycle_nodes(rev_path);
        }
    }

    return rev_path;
}

Vec rev_path_into_cycle_nodes(Vec rev_path) {
    size_t c_0 = *(const size_t*)Vec_get(&rev_path, 0);
    size_t len = Vec_len(&rev_path);
    size_t i;
    for (i = 1; i < len; i++) {
        size_t c_i = *(const size_t*)Vec_get(&rev_path, i);
        if (c_i == c_0) break;
    }
    while (Vec_len(&rev_path) > i) {
        Vec_pop(&rev_path, NULL);
    }
    return rev_path;
}

bool Graph_find_odd_cycle_from(const Graph* g, size_t root, Vec* path,
                               Vec* marks, NodeMark mark) {
    bool found = false;

    NodeMark* m = Vec_get_mut(marks, root);
    *m = mark;

    const GraphNode* n = Vec_get(&g->nodes, root);
    size_t conflict_count = Vec_len(&n->conflict);
    size_t continuity_count = Vec_len(&n->continuity);
    for (size_t c = 0; c < (conflict_count + continuity_count); c++) {
        const GraphEdge* e;
        if (c < conflict_count)
            e = Vec_get(&n->conflict, c);
        else e = Vec_get(&n->continuity, c - conflict_count);
        size_t v = e->v;

        NodeMark v_mark = *(const NodeMark*)Vec_get(marks, v);
        if (v_mark == UNVISITED_NODE) {
            if (mark == A_OPENED_NODE)
                v_mark = B_OPENED_NODE;
            else v_mark = A_OPENED_NODE;
            if (Graph_find_odd_cycle_from(g, v, path, marks, v_mark)) {
                found = true;
                Vec_push(path, &root);
                break;
            }
        } else if (v_mark == mark) {
            found = true;
            Vec_push(path, &v);
            Vec_push(path, &root);
            break;
        }
    }

    return found;
}

void Graph_solve_faces(const Graph* g, BitSet* solution) {
    size_t node_count = Vec_len(&g->nodes);
    BitSet visited = BitSet_with_capacity(node_count);

    for (size_t n = 0; n < node_count; n++) {
        if (!BitSet_contains(&visited, n)) {
            Graph_solve_faces_from(g, n, solution, &visited, false);
        }
    }

    BitSet_drop(&visited);
}

void Graph_solve_faces_from(const Graph* g, size_t root, BitSet* solution,
                            BitSet* visited, bool face) {
    BitSet_insert(visited, root);

    const GraphNode* n = Vec_get(&g->nodes, root);
    if (n->type == SEGMENT_NODE) {
        if (face) BitSet_insert(solution, root);
    } else {
        if (BitSet_contains(solution, root)) return;
    }

    size_t conflict_count = Vec_len(&n->conflict);
    for (size_t c = 0; c < conflict_count; c++) {
        const GraphEdge* e = Vec_get(&n->conflict, c);
        size_t v = e->v;

        if (!BitSet_contains(visited, v)) {
            Graph_solve_faces_from(g, v, solution, visited, !face);
        } else {
            assert(BitSet_contains(solution, v) != face);
        }
    }

    size_t continuity_count = Vec_len(&n->continuity);
    for (size_t c = 0; c < continuity_count; c++) {
        const GraphEdge* e = Vec_get(&n->continuity, c);
        size_t v = e->v;

        if (!BitSet_contains(visited, v)) {
            Graph_solve_faces_from(g, v, solution, visited, face);
        }
    }
}
