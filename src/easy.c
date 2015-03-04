#include "circuit.h"
#include "output.h"

int main() {
    const char* path = "netlists/easy.net";

    printf("handling `%s`.\n", path);

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

    printf("`%s` "TERM_GREEN("✓")"\n", path);

    return EXIT_SUCCESS;
}
