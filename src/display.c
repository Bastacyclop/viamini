#include <math.h>

#include "display.h"

const Color BLACK = {   0,   0,   0 };
const Color WHITE = { 255, 255, 255 };
const Color RED   = { 255,   0,   0 };
const Color GREEN = {   0, 255,   0 };
const Color BLUE  = {   0,   0, 255 };

static
void ColorSet_step(ColorSet* cs);

typedef struct {
    Vector translate;
    float scale;
} Transform;

static
void ps_set_color(FILE* f, Color c);
static
void ps_draw_line(FILE* f, Point a, Point b, const Transform* t);
static
void ps_draw_point(FILE* f, Point p, const Transform* t);
static
void ps_draw_intersection(FILE* f, Point p, const Transform* t);
static
void ps_draw_hidden_line(FILE* f, Point a, Point b, const Transform* t);
static
void ps_init_text(FILE* f);
static
void ps_draw_text(FILE*f, const char* text, int x, int y);

static
Transform compute_ps_transform(const AABB* aabb);

static
Point middle_of(const Point* a, const Point* b);

ColorSet ColorSet_new(size_t size) {
    // number of combinations is 'pow((255/step), 3)'
    // we want it to be 'size + 2' (no black and white), hence the formula:
    uint8_t step = (uint8_t)(255. / powf((float)(size + 2), 1./3.));
    return (ColorSet) {
        .next = { 0, 0, step }, // start b at step because no white
        .step = step
    };
}

Color ColorSet_pop(ColorSet* cs) {
    Color c = cs->next;
    ColorSet_step(cs);
    return c;
}

void ColorSet_step(ColorSet* cs) {
    uint8_t step = cs->step;
    cs->next.b += step;
    if (cs->next.b < step) {
        cs->next.g += step;
        if (cs->next.g < step) {
            cs->next.r += step;
        }
    }
}

void Netlist_to_ps(const Netlist* nl, const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) {
        perror("cannot open postscript file");
        exit(1);
    }

    size_t net_count = Vec_len(&nl->nets);
    ColorSet colors = ColorSet_new(net_count);
    Transform trans = compute_ps_transform(&nl->aabb);

    for (size_t i = 0; i < net_count; i++) {
        const Net* net = Vec_get(&nl->nets, i);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);

        ps_set_color(f, ColorSet_pop(&colors));

        for (size_t s = 0; s < segment_count; s++) {
            const Segment* segment = Vec_get(&net->segments, s);
            const Point* beg = Vec_get(&net->points, segment->beg);
            const Point* end = Vec_get(&net->points, segment->end);
            ps_draw_line(f, *beg, *end, &trans);
        }

        for (size_t p = 0; p < point_count; p++) {
            const Point* point = Vec_get(&net->points, p);
            ps_draw_point(f, *point, &trans);
        }
    }

    fclose(f);
}

void ps_set_color(FILE* f, Color c) {
    #define to_ps(x) (float)(x)/255.
    fprintf(f, "%f %f %f setrgbcolor\n", to_ps(c.r), to_ps(c.g), to_ps(c.b));
    #undef to_ps
}

#define x_trans(X) (t->scale*(float)(X+t->translate.x))
#define y_trans(Y) (t->scale*(float)(Y+t->translate.y))

void ps_draw_line(FILE* f, Point a, Point b, const Transform* t) {
    fprintf(f, "%f %f moveto\n", x_trans(a.x), y_trans(a.y));
    fprintf(f, "%f %f lineto\n", x_trans(b.x), y_trans(b.y));
    fputs("stroke\n", f);
}

void ps_draw_point(FILE* f, Point p, const Transform* t) {
    fprintf(f, "%f %f 1.5 0 360 arc\n", x_trans(p.x), y_trans(p.y));
    fputs("fill\n", f);
}

void ps_draw_intersection(FILE* f, Point p, const Transform* t) {
    float x = x_trans(p.x);
    float y = y_trans(p.y);
    float r = 1.5;
    fprintf(f, "%f %f moveto\n", x - r, y - r);
    fprintf(f, "%f %f lineto\n", x + r, y + r);
    fputs("stroke\n", f);
    fprintf(f, "%f %f moveto\n", x - r, y + r);
    fprintf(f, "%f %f lineto\n", x + r, y - r);
    fputs("stroke\n", f);
}

#undef x_trans
#undef y_trans

void ps_draw_hidden_line(FILE* f, Point a, Point b, const Transform* t) {
    fprintf(f, "save\n");
    fprintf(f, "[5 5] 0 setdash\n");
    ps_draw_line(f, a, b, t);
    fprintf(f, "restore\n");
}

void ps_init_text(FILE* f) {
    fputs("/Times-Roman findfont\n"
          "20 scalefont\n"
          "setfont\n", f);
}

void ps_draw_text(FILE* f, const char* text, int x, int y) {
    fputs("newpath\n", f);
    fprintf(f, "%d %d moveto\n", x, y);
    fprintf(f, "(%s) show\n", text);
}


Transform compute_ps_transform(const AABB* aabb) {
    float scale_w = 600. / (float)(aabb->sup.x - aabb->inf.x);
    float scale_h = 600. / (float)(aabb->sup.y - aabb->inf.y);
    return (Transform) {
        .translate = { -aabb->inf.x, -aabb->inf.y },
        .scale = float_min(scale_w, scale_h)
    };
}

void Netlist_intersections_to_ps(const Vec* intersections, const Netlist* nl,
                                 const char* base_path, const char* file_path) {
    FILE* base = fopen(base_path, "r");
    if (!base) {
        perror("cannot open base postscript file");
        exit(1);
    }
    FILE* f = fopen(file_path, "w");
    if (!f) {
        perror("cannot open postscript file");
        exit(1);
    }

    char line[255];
    while (fgets(line, 255, base)) {
        fputs(line, f);
    }

    fclose(base);

    Transform trans = compute_ps_transform(&nl->aabb);
    ps_set_color(f, RED);

    size_t intersection_count = Vec_len(intersections);
    for (size_t i = 0; i < intersection_count; i++) {
        const Intersection* intersection = Vec_get(intersections, i);
        ps_draw_intersection(f, intersection->point, &trans);
    }

    ps_init_text(f);
    char text[100];
    snprintf(text, 100, "%zu intersections", intersection_count);
    ps_draw_text(f, text, 20, 595);

    fclose(f);
}

void Graph_to_ps(const Graph* g, const Netlist* nl, const char* path) {
    FILE* f = fopen(path, "w");

    size_t net_count = Vec_len(&nl->nets);
    ColorSet colors = ColorSet_new(net_count);
    Transform trans = compute_ps_transform(&nl->aabb);

    size_t offset = 0;
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);

        ps_set_color(f, ColorSet_pop(&colors));

        for (size_t i = offset; i < (offset + point_count); i++) {
            const GraphNode* node = Vec_get(&g->nodes, i);
            ps_draw_point(f, *node->point, &trans);

            size_t continuity_count = Vec_len(&node->continuity);
            for (size_t c = 0; c < continuity_count; c++) {
                const GraphEdge* edge = Vec_get(&node->continuity, c);
                const GraphNode* continuity = Vec_get(&g->nodes, edge->v);
                const Point* c_beg = Vec_get(&net->points, continuity->segment->beg);
                const Point* c_end = Vec_get(&net->points, continuity->segment->end);
                ps_draw_line(f, *node->point, middle_of(c_beg, c_end), &trans);
            }
        }
        offset += point_count;

        for (size_t i = offset; i < (offset + segment_count); i++) {
            const GraphNode* node = Vec_get(&g->nodes, i);
            const Point* beg = Vec_get(&net->points, node->segment->beg);
            const Point* end = Vec_get(&net->points, node->segment->end);
            Point mid = middle_of(beg, end);
            ps_draw_point(f, mid, &trans);
        }
        offset += segment_count;
    }

    ps_set_color(f, BLACK);

    offset = 0;
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);
        offset += point_count;

        for (size_t i = offset; i < (offset + segment_count); i++) {
            const GraphNode* node = Vec_get(&g->nodes, i);
            const Point* beg = Vec_get(&net->points, node->segment->beg);
            const Point* end = Vec_get(&net->points, node->segment->end);
            Point mid = middle_of(beg, end);

            size_t conflict_count = Vec_len(&node->conflict);
            for (size_t c = 0; c < conflict_count; c++) {
                const GraphEdge* edge = Vec_get(&node->conflict, c);
                const GraphNode* conflict = Vec_get(&g->nodes, edge->v);
                size_t c_n = 0;
                for (size_t j = 0; j < net_count; j++) {
                    if (edge->v > *(const size_t*)Vec_get(&g->net_offsets, j)) {
                        c_n = j;
                    } else {
                        break;
                    }
                }
                const Net* c_net = Vec_get(&nl->nets, c_n);
                const Point* c_beg = Vec_get(&c_net->points,
                                             conflict->segment->beg);
                const Point* c_end = Vec_get(&c_net->points,
                                             conflict->segment->end);
                ps_draw_line(f, mid, middle_of(c_beg, c_end), &trans);
            }
        }
        offset += segment_count;
    }

    fclose(f);
}

Point middle_of(const Point* a, const Point* b) {
    return (Point) {
        .x = (a->x + b->x) /2,
        .y = (a->y + b->y) /2
    };
}

void Solution_to_ps(const BitSet* sol, const Graph* g,
                    const Netlist* nl, const char* path) {
    FILE* f = fopen(path, "w");

    size_t net_count = Vec_len(&nl->nets);
    ColorSet colors = ColorSet_new(net_count);
    Transform trans = compute_ps_transform(&nl->aabb);

    size_t offset = 0;
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);

        ps_set_color(f, ColorSet_pop(&colors));

        for (size_t i = offset; i < (offset + point_count); i++) {
            const GraphNode* node = Vec_get(&g->nodes, i);
            ps_draw_point(f, *node->point, &trans);

            size_t continuity_count = Vec_len(&node->continuity);
            for (size_t c = 0; c < continuity_count; c++) {
                const GraphEdge* edge = Vec_get(&node->continuity, c);
                const GraphNode* continuity = Vec_get(&g->nodes, edge->v);
                const Point* c_beg = Vec_get(&net->points, continuity->segment->beg);
                const Point* c_end = Vec_get(&net->points, continuity->segment->end);
                if (BitSet_contains(sol, edge->v)) {
                    ps_draw_hidden_line(f, *node->point, middle_of(c_beg, c_end),
                                        &trans);
                } else {
                    ps_draw_line(f, *node->point, middle_of(c_beg, c_end), &trans);
                }
            }
        }
        offset += point_count;

        for (size_t i = offset; i < (offset + segment_count); i++) {
            const GraphNode* node = Vec_get(&g->nodes, i);
            const Point* beg = Vec_get(&net->points, node->segment->beg);
            const Point* end = Vec_get(&net->points, node->segment->end);
            ps_draw_point(f, middle_of(beg, end), &trans);
        }
        offset += segment_count;
    }

    ps_set_color(f, BLACK);

    size_t via_count = 0;

    offset = 0;
    for (size_t n = 0; n < net_count; n++) {
        const Net* net = Vec_get(&nl->nets, n);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);

        for (size_t i = offset; i < (offset + point_count); i++) {
            if (BitSet_contains(sol, i)) {
                const GraphNode* node = Vec_get(&g->nodes, i);
                ps_draw_point(f, *node->point, &trans);
                via_count++;
            }
        }
        offset += point_count + segment_count;
    }

    ps_init_text(f);
    char text[100];
    snprintf(text, 100, "%zu vias", via_count);
    ps_draw_text(f, text, 20, 595);

    fclose(f);
}
