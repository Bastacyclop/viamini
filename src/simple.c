#include "netlist.h"
#include "display.h"

int main() {
    const char* path = "netlists/alea0030_030_10_088.net";

    printf("handling `%s` ... ", path);

    char* display_path = change_extension(path, "svg");
    char* intersection_display_path = change_extension(path, "int.svg");

    Netlist netlist = Netlist_from_file(path);
    Netlist_to_svg(&netlist, display_path);

    Vec intersections = Netlist_intersections_list_sweep(&netlist);
    Netlist_intersections_to_svg(&netlist, &intersections,
                                 display_path, intersection_display_path);
    Vec_plain_drop(&intersections);

    Netlist_drop(&netlist);

    free(display_path);
    free(intersection_display_path);

    printf(TERM_GREEN("✓")"\n");

    return EXIT_SUCCESS;
}
