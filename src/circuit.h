#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "vec.h"

typedef Vec PointVec;
typedef struct {
    int32_t x;
    int32_t y;
} Point, Vector;

typedef Vec SegmentVec;
typedef struct {
    size_t beg;
    size_t end;
} Segment;

typedef Vec NetVec;
typedef struct {
    PointVec points;
    SegmentVec segments;
} Net;

typedef struct {
    Point inf;
    Point sup;
} AABB;

typedef struct {
    NetVec nets;
    AABB aabb;
} Circuit;

Circuit Circuit_from_file(const char* path);
void Circuit_drop(Circuit* c);
void Circuit_print(const Circuit* c);

typedef Vec IntersectionVec;
typedef struct {
    size_t a_net;
    size_t a_seg;
    size_t b_net;
    size_t b_seg;
    Point sect;
} Intersection;

IntersectionVec Circuit_intersections_naive(const Circuit* c);
IntersectionVec Circuit_intersections_sweep(const Circuit* c);

#endif // CIRCUIT_H
