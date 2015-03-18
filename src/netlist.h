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
} Netlist;

/// Loads a circuit from a file.
Netlist Netlist_from_file(const char* path);

/// Releases the circuit resources.
void Netlist_drop(Netlist* nl);

/// Prints the file representation of the circuit on the terminal.
void Netlist_print(const Netlist* nl);

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
IntersectionVec Netlist_intersections_naive(const Netlist* nl);

/// Finds the circuit intersections by sweeping over the different breakpoints of the x axis.
/// This version uses a vector to manage current horizontal segments.
IntersectionVec Netlist_intersections_vec_sweep(const Netlist* nl);

/// Finds the circuit intersections by sweeping over the different breakpoints of the x axis.
/// This version uses an ordered list to manage current horizontal segments.
IntersectionVec Netlist_intersections_list_sweep(const Netlist* nl);

/// Finds the circuit intersections by sweeping over the different breakpoints of the x axis.
/// This version uses an AVL tree to manage current horizontal segments.
IntersectionVec Netlist_intersections_avl_sweep(const Netlist* nl);

/// Saves the circuit intersections to a file.
void Netlist_intersections_to_file(IntersectionVec* ints, const char* path);

typedef enum {
    POINT_NODE,
    SEGMENT_NODE
} GraphNodeType;

typedef Vec GraphNodeVec;
typedef struct {
    GraphNodeType type;
    void* data;
    GraphNodeVec continuity;
    GraphNodeVec conflict;
} GraphNode;

typedef struct {
    GraphNode* n;
} Graph;

Graph Graph_new(const Netlist* nl, const IntersectionVec* intersections);

#endif // CIRCUIT_H
