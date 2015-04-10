#ifndef DISPLAY_H
#define DISPLAY_H

#include "netlist.h"

/// Display related functions

/// A color represented in RGB with values from `0` to `255`.
typedef struct {
    uint8_t r, g, b;
} Color;

const Color BLACK,
            WHITE,
            RED,
            GREEN,
            BLUE;

/// A set of colors.
typedef struct {
    Color next;
    uint8_t step;
} ColorSet;

/// Creates a new set of `n` colors.
ColorSet ColorSet_new(size_t size);

/// Removes a color from the set and returns it.
/// If the set is empty, there is no insurance about the returned color.
Color ColorSet_pop(ColorSet* cs);


/// Outputs a postscript file showing the circuit.
void Netlist_to_ps(const Netlist* nl, const char* path);

/// Outputs a postscript file, adding the circuit intersections to the base.
void Netlist_intersections_to_ps(const Vec* intersections, const Netlist* nl,
                                 const char* base_path, const char* file_path);

/// Outputs a postscript file showing the graph.
void Graph_to_ps(const Graph* g, const Netlist* nl, const char* path);

/// Outputs a postscript file showing the solution.
void Solution_to_ps(const BitSet* sol, const Graph* g,
                    const Netlist* nl, const char* path);

#endif // DISPLAY_H
