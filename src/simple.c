#include "circuit.h"
#include "output.h"

int main() {
    const char* path = "netlists/alea0030_030_10_088.net";

    printf("handling `%s` ... ", path);

    char* display_path = change_extension(path, "svg");
    char* intersection_path = change_extension(path, "intersection.svg");

    Circuit circuit = Circuit_from_file(path);
    Circuit_to_svg(&circuit, display_path);

    Vec intersections = Circuit_intersections_avl_sweep(&circuit);
    Circuit_intersections_to_svg(&circuit, &intersections,
                                 display_path, intersection_path);
    Vec_plain_drop(&intersections);

    Circuit_drop(&circuit);

    free(display_path);
    free(intersection_path);

    printf(TERM_GREEN("✓")"\n");

    return EXIT_SUCCESS;
}
