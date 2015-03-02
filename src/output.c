#include <math.h>

#include "output.h"

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
Transform compute_ps_transform(const AABB* aabb);

static
void svg_begin_color(FILE* f, Color c);
static
void svg_end_color(FILE* f);
static
void svg_draw_line(FILE* f, Point a, Point b, const Transform* t);
static
void svg_draw_point(FILE* f, Point p, const Transform* t);
static
Transform compute_svg_transform(const AABB* aabb);

ColorSet ColorSet_new(size_t size) {
    // number of combinations is 'pow((255/step), 3)'
    // we want it to be 'size + 2' (no black and white), hence the formula:
    uint8_t step = (uint8_t)(255. / powf((float)(size + 2), 1./3.));
    return (ColorSet) {
        .next = { 0, 0, step }, // start b at step because no white
        .step = step
    };
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

Color ColorSet_pop(ColorSet* cs) {
    Color c = cs->next;
    ColorSet_step(cs);
    return c;
}

void Circuit_to_ps(const Circuit* c, const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) {
        perror("cannot open postscript file");
        exit(1);
    }

    size_t net_count = Vec_len(&c->nets);
    ColorSet colors = ColorSet_new(net_count);
    Transform trans = compute_ps_transform(&c->aabb);

    for (size_t i = 0; i < net_count; i++) {
        const Net* net = Vec_get(&c->nets, i);
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

#define x_trans(X) t->scale*(float)(X+t->translate.x)
#define y_trans(Y) t->scale*(float)(Y+t->translate.y)

void ps_draw_line(FILE* f, Point a, Point b, const Transform* t) {
    fprintf(f, "%f %f moveto\n", x_trans(a.x), y_trans(a.y));
    fprintf(f, "%f %f lineto\n", x_trans(b.x), y_trans(b.y));
    fputs("stroke\n", f);
}

void ps_draw_point(FILE* f, Point p, const Transform* t) {
    fprintf(f, "%f %f 1.5 0 360 arc\n", x_trans(p.x), y_trans(p.y));
    fputs("fill\n", f);
}

#undef x_trans
#undef y_trans

Transform compute_ps_transform(const AABB* aabb) {
    return (Transform) {
        .translate = { -aabb->inf.x, -aabb->inf.y },
        .scale = 600. / (float)(aabb->sup.x - aabb->inf.x)
    };
}

void Circuit_intersections_to_ps(const Circuit* c, const Vec* intersections,
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

    Transform trans = compute_ps_transform(&c->aabb);
    ps_set_color(f, (Color) { 0, 0, 0 });

    for (size_t i = 0; i < Vec_len(intersections); i++) {
        const Intersection* intersection = Vec_get(intersections, i);
        ps_draw_point(f, intersection->sect, &trans);
    }

    fclose(f);
}

void Circuit_to_svg(const Circuit* c, const char* path) {
    FILE* f = fopen(path, "w");
    if (!f) {
        perror("cannot open svg file");
        exit(1);
    }

    fputs("<svg version=\"1.2\" baseProfile=\"tiny\" xmlns=\"http://www.w3.org/2000/svg\">\n", f);

    size_t net_count = Vec_len(&c->nets);
    ColorSet colors = ColorSet_new(net_count);
    Transform trans = compute_svg_transform(&c->aabb);

    for (size_t i = 0; i < net_count; i++) {
        const Net* net = Vec_get(&c->nets, i);
        size_t point_count = Vec_len(&net->points);
        size_t segment_count = Vec_len(&net->segments);

        svg_begin_color(f, ColorSet_pop(&colors));

        for (size_t s = 0; s < segment_count; s++) {
            const Segment* segment = Vec_get(&net->segments, s);
            const Point* beg = Vec_get(&net->points, segment->beg);
            const Point* end = Vec_get(&net->points, segment->end);
            svg_draw_line(f, *beg, *end, &trans);
        }

        for (size_t p = 0; p < point_count; p++) {
            const Point* point = Vec_get(&net->points, p);
            svg_draw_point(f, *point, &trans);
        }

        svg_end_color(f);
    }

    fclose(f);
}

void svg_begin_color(FILE* f, Color c) {
    fprintf(f, "<g stroke=\"rgb(%u, %u, %u)\" fill=\"rgb(%u, %u, %u)\" >\n",
                            c.r, c.g, c.b,           c.r, c.g, c.b);
}

void svg_end_color(FILE* f) {
    fputs("</g>\n", f);
}

#define x_trans(X) t->scale*(float)(X+t->translate.x)
#define y_trans(Y) t->scale*(float)(Y+t->translate.y)

void svg_draw_line(FILE* f, Point a, Point b, const Transform* t) {
    fprintf(f, "<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" />\n",
            x_trans(a.x), y_trans(a.y), x_trans(b.x), y_trans(b.y));
}

void svg_draw_point(FILE* f, Point p, const Transform* t) {
    fprintf(f, "<circle cx=\"%f\" cy=\"%f\" r=\"1.\" />\n",
            x_trans(p.x), y_trans(p.y));
}

#undef x_trans
#undef y_trans

Transform compute_svg_transform(const AABB* aabb) {
    float scale_w = 600. / (float)(aabb->sup.x - aabb->inf.x);
    float scale_h = 600. / (float)(aabb->sup.y - aabb->inf.y);
    return (Transform) {
        .translate = { -aabb->inf.x, -aabb->inf.y },
        .scale = float_max(scale_w, scale_h)
    };
}

void Circuit_intersections_to_svg(const Circuit* c, const Vec* intersections,
                                  const char* base_path, const char* file_path) {
    FILE* base = fopen(base_path, "r");
    if (!base) {
        perror("cannot open base svg file");
        exit(1);
    }
    FILE* f = fopen(file_path, "w");
    if (!f) {
        perror("cannot open svg file");
        exit(1);
    }

    char line[255];
    while (fgets(line, 255, base)) {
        fputs(line, f);
    }

    fclose(base);

    Transform trans = compute_svg_transform(&c->aabb);
    svg_begin_color(f, (Color) { 0, 0, 0 });

    for (size_t i = 0; i < Vec_len(intersections); i++) {
        const Intersection* intersection = Vec_get(intersections, i);
        svg_draw_point(f, intersection->sect, &trans);
    }

    svg_end_color(f);

    fclose(f);
}
