#ifndef CIRCUIT_H
#define CIRCUIT_H

#include "vec.h"
#include "bit_set.h"

/// Netlist related functions

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

/// Loads a netlist from a file.
Netlist Netlist_from_file(const char* path);

/// Releases the netlist resources.
void Netlist_drop(Netlist* nl);

/// Prints the file representation of the netlist on the terminal.
void Netlist_print(const Netlist* nl);

/// Returns the total number of segments of the netlist.
size_t Netlist_segment_count(const Netlist* nl);

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

/// Finds the netlist intersections by comparing the segments of different nets two by two.
IntersectionVec Netlist_intersections_naive(const Netlist* nl);

/// Finds the netlist intersections by sweeping over the different breakpoints of the x axis.
/// This version uses a vector to manage current horizontal segments.
IntersectionVec Netlist_intersections_vec_sweep(const Netlist* nl);

/// Finds the netlist intersections by sweeping over the different breakpoints of the x axis.
/// This version uses an ordered list to manage current horizontal segments.
IntersectionVec Netlist_intersections_list_sweep(const Netlist* nl);

/// Finds the netlist intersections by sweeping over the different breakpoints of the x axis.
/// This version uses an AVL tree to manage current horizontal segments.
IntersectionVec Netlist_intersections_avl_sweep(const Netlist* nl);

/// Saves the netlist intersections to a file.
void Netlist_intersections_to_file(IntersectionVec* ints, const char* path);

typedef Vec GraphEdgeVec;
typedef struct {
    size_t u;
    size_t v;
} GraphEdge;

typedef enum {
    POINT_NODE,
    SEGMENT_NODE
} GraphNodeType;

typedef Vec GraphNodeVec;
typedef struct {
    GraphNodeType type;
    union {
        const Point* point;
        const Segment* segment;
    };
    GraphEdgeVec continuity;
    GraphEdgeVec conflict;
} GraphNode;

typedef struct {
    GraphNodeVec nodes;
    Vec net_offsets;
} Graph;

/// Creates the graph associated to the netlist and its intersections.
Graph Graph_new(const Netlist* nl, const char* int_path);

/// Releases the graph resources.
void Graph_drop(Graph* g);

/// Solves the problem by putting every horizontal segments on a face
///                           and -----  vertical  ----------- the other.
BitSet Graph_hv_solve(const Graph* g, const Netlist* nl);

/// Solves the problem by finding odd cycles.
BitSet Graph_odd_cycle_solve(const Graph* g);

/// Returns the number of vias required by the solution.
size_t Solution_via_count(const BitSet* solution, const Graph* g);

#endif // CIRCUIT_H
