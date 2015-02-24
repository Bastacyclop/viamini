#ifndef OUTPUT_H
#define OUTPUT_H

#include "circuit.h"

/// A color represented in RGB with values from `0` to `255`.
typedef struct {
    uint8_t r, g, b;
} Color;

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


void Circuit_to_ps(const Circuit* c, const char* path);
void Circuit_intersections_to_ps(const Circuit* c, const Vec* intersections,
                                 const char* base_path, const char* file_path);

void Circuit_to_svg(const Circuit* c, const char* path);
void Circuit_intersections_to_svg(const Circuit* c, const Vec* intersections,
                                  const char* base_path, const char* file_path);

#endif // OUTPUT_H
