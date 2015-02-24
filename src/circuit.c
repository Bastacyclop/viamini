#include "binary_heap.h"
#include "circuit.h"

void point_from_line(Vec* points, const char* line);
void segment_from_line(Vec* segments, const char* line);
void check_net_segments(Net* net);
Net net_from_file(FILE* f);
AABB compute_aabb(Vec* nets);
void drop_net(Net* net);

typedef struct {
    const Point* beg;
    const Point* end;
} SegmentRef;

bool hv_intersects(SegmentRef h, SegmentRef v);
bool segment_intersects(SegmentRef a, SegmentRef b, Point* sect);

typedef enum {
    H_SEGMENT_BEGIN,
    H_SEGMENT_END,
    V_SEGMENT
} BreakpointType;

typedef struct {
    size_t net;
    size_t seg;
    SegmentRef ref;
} BreakpointData;

typedef struct {
    BreakpointType type;
    BreakpointData data;
} Breakpoint;

void register_break(BinaryHeap* bh, size_t n, const Net* net, size_t s, const Segment* segment);
int32_t Breakpoint_get_x(const Breakpoint* b);
bool break_order(const Breakpoint* a, const Breakpoint* b);

#define SYNTAX_ERROR(desc) {                        \
    perror("circuit file syntax error: "desc"\n");  \
    exit(1);                                        \
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

        if ((end->x < beg->x) || (end->y < beg->y)) {
            mem_swap(&s->beg, &s->end, sizeof(size_t));
        }
    }
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

AABB compute_aabb(Vec* nets) {
    Point first = *(const Point*)Vec_get(&((const Net*)Vec_get(nets, 0))->points, 0);
    AABB aabb = (AABB) { .inf = first, .sup = first };

    size_t net_count = Vec_len(nets);
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(nets, n);

        size_t point_count = Vec_len(&net->points);
        for (size_t p = 0; p < point_count; p++) {
            const Point* point = Vec_get(&net->points, p);

            if (point->x < aabb.inf.x) {
                aabb.inf.x = point->x;
            } else if (point->x > aabb.sup.x) {
                aabb.sup.x = point->x;
            }
            if (point->y < aabb.inf.y) {
                aabb.inf.y = point->y;
            } else if (point->y > aabb.sup.y) {
                aabb.sup.y = point->y;
            }
        }
    }

    return aabb;
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

void drop_net(Net* net) {
    Vec_plain_drop(&net->points);
    Vec_plain_drop(&net->segments);
}

void Circuit_drop(Circuit* c) {
    Vec_drop(&c->nets, (void (*)(void*))drop_net);
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

bool hv_intersects(SegmentRef h, SegmentRef v) {
    assert(h.beg->x <= h.end->x);
    assert(v.beg->y <= v.end->y);
    return h.beg->x <= v.beg->x && v.beg->x <= h.end->x &&
           v.beg->y <= h.beg->y && h.beg->y <= v.end->y;
}

bool segment_intersects(SegmentRef a, SegmentRef b, Point* sect) {
    if (a.beg->x == a.end->x) { // |
        if (b.beg->x == b.end->x) { // | & |
            //assert(a.beg->x != b.beg->x);
            return false;
       } else { // | & -
            if (hv_intersects(b, a)) {
                sect->x = a.beg->x;
                sect->y = b.beg->y;
                return true;
            } else {
                return false;
            }
       }
    } else { // -
        if (b.beg->y == b.end->y) { // - & -
            //assert(a.beg->y != b.beg->y);
            return false;
        } else { // - & |
            if (hv_intersects(a, b)) {
                sect->x = b.beg->x;
                sect->y = a.beg->y;
                return true;
            } else {
                return false;
            }
        }
    }
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

            // We don't want to check intranet intersections.

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
                            .a_net = i, .a_seg = s,
                            .b_net = j, .b_seg = t,
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

void register_break(BinaryHeap* bh, size_t n, const Net* net, size_t s, const Segment* segment) {
    const Point* beg = Vec_get(&net->points, segment->beg);
    const Point* end = Vec_get(&net->points, segment->end);
    if (beg->x == end->x) { // |
        Breakpoint breakpoint = {
            .type = V_SEGMENT,
            .data = { .net = n, .seg = s, { .beg = beg, .end = end } }
        };
        BinaryHeap_insert(bh, &breakpoint);
    } else { // -
        Breakpoint breakpoint = {
            .type = H_SEGMENT_BEGIN,
            .data = { .net = n, .seg = s, { .beg = beg, .end = end } }
        };
        BinaryHeap_insert(bh, &breakpoint);
        breakpoint.type = H_SEGMENT_END;
        BinaryHeap_insert(bh, &breakpoint);
    }
}

int32_t Breakpoint_get_x(const Breakpoint* b) {
    BreakpointType t = b->type;
    if (t == H_SEGMENT_BEGIN) {
        return b->data.ref.beg->x;
    } else {
        return b->data.ref.end->x;
    }
}

bool break_order(const Breakpoint* a, const Breakpoint* b) {
    return Breakpoint_get_x(a) < Breakpoint_get_x(b);
}

IntersectionVec Circuit_intersections_sweep(const Circuit* c) {
    BinaryHeap breakpoints = BinaryHeap_new(sizeof(Breakpoint),
                                            (bool (*)(const void*, const void*))break_order);

    size_t net_count = Vec_len(&c->nets);
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&c->nets, n);

        size_t segment_count = Vec_len(&net->segments);
        for (size_t s = 0; s < segment_count; s++) {
            const Segment* segment = Vec_get(&net->segments, s);

            register_break(&breakpoints, n, net, s, segment);
        }
    }

    IntersectionVec intersections = Vec_new(sizeof(Intersection));
    Vec segments = Vec_new(sizeof(BreakpointData));

    Breakpoint breakpoint;
    while (BinaryHeap_pop(&breakpoints, &breakpoint)) {
        if (breakpoint.type == H_SEGMENT_BEGIN) {
            Vec_push(&segments, &breakpoint.data);
        } else if (breakpoint.type == H_SEGMENT_END) {
            for (size_t i = 0; i < Vec_len(&segments); i++) {
                if (memcmp(&breakpoint.data, Vec_get(&segments, i), sizeof(BreakpointData)) == 0) {
                    Vec_swap_remove(&segments, NULL, i);
                    break;
                }
            }
        } else if (breakpoint.type == V_SEGMENT) {
            BreakpointData a = breakpoint.data;

            size_t seg_count = Vec_len(&segments);
            for (size_t i = 0; i < seg_count; i++) {
                BreakpointData b = *(const BreakpointData*)Vec_get(&segments, i);

                if (a.net == b.net) {
                    continue;
                }

                Point sect;
                if (segment_intersects(a.ref, b.ref, &sect)) {
                    Intersection intersection = {
                        .a_net = a.net, .a_seg = a.seg,
                        .b_net = b.net, .b_seg = b.seg,
                        .sect = sect
                    };
                    Vec_push(&intersections, &intersection);
                }
            }
        }
    }

    Vec_plain_drop(&segments);
    BinaryHeap_plain_drop(&breakpoints);

    return intersections;
}
