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

/// Loads a circuit from a file.
Circuit Circuit_from_file(const char* path);

/// Releases the circuit resources.
void Circuit_drop(Circuit* c);

/// Prints the file representation of the circuit on the terminal.
void Circuit_print(const Circuit* c);

typedef struct {
    size_t net;
    size_t seg;
} SegmentLoc;

typedef Vec IntersectionVec;
typedef struct {
    SegmentLoc a;
    SegmentLoc b;
    Point sect;
} Intersection;

/// Finds the circuit intersections by comparing the segments of different nets two by two.
IntersectionVec Circuit_intersections_naive(const Circuit* c);

/// Finds the circuit intersections by sweeping over the different breakpoints of the x axis.
/// This version uses a vector to manage current horizontal segments.
IntersectionVec Circuit_intersections_vec_sweep(const Circuit* c);

/// Finds the circuit intersections by sweeping over the different breakpoints of the x axis.
/// This version uses an ordered list to manage current horizontal segments.
IntersectionVec Circuit_intersections_list_sweep(const Circuit* c);

/// Finds the circuit intersections by sweeping over the different breakpoints of the x axis.
/// This version uses an AVL tree to manage current horizontal segments.
IntersectionVec Circuit_intersections_avl_sweep(const Circuit* c);

typedef enum {
    POINT_NODE,
    SEGMENT_NODE
} GraphNodeType;

typedef struct GraphNode GraphNode;
typedef Vec GraphNodeVec;
struct GraphNode {
    GraphNodeType type;
    void* data;
    GraphNodeVec continuity;
    GraphNodeVec conflict;
};

typedef struct {
    GraphNode* n;
} Graph;

Graph Graph_new(const Circuit* c, const IntersectionVec* inters);

#endif // CIRCUIT_H
