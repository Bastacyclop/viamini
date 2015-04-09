#include "netlist.h"
#include "display.h"

/// Finds the intersections of the given netlist
/// with the given method and display output.

int main() {
    char file[255];
    ask_str("enter the netlist file name: ", file, 255);
    char* path = str_surround("netlists/", file, ".net");

    char method[20];
    ask_str("choose a method (naive/vec_sweep/list_sweep/avl_sweep): ", method, 20);

    Vec (*compute_intersections)(const Netlist*);
    if (strcmp(method, "naive") == 0) {
        compute_intersections = Netlist_intersections_naive;
    } else if (strcmp(method, "vec_sweep") == 0) {
        compute_intersections = Netlist_intersections_vec_sweep;
    } else if (strcmp(method, "list_sweep") == 0) {
        compute_intersections = Netlist_intersections_list_sweep;
    } else if (strcmp(method, "avl_sweep") == 0) {
        compute_intersections = Netlist_intersections_avl_sweep;
    } else {
        perror("unknown method");
        exit(1);
    }

    char format[4];
    ask_str("choose an output format (ps/svg): ", format, 4);

    char* intersection_path = change_extension(path, "int");
    char* display_path;
    char* intersection_display_path;

    void (*netlist_display)(const Netlist*, const char*);
    void (*intersections_display)(const Netlist*, const Vec*, const char*, const char*);
    if (strcmp(format, "ps") == 0) {
        display_path = change_extension(path, "ps");
        intersection_display_path = change_extension(path, "int.ps");
        netlist_display = Netlist_to_ps;
        intersections_display = Netlist_intersections_to_ps;
    } else if (strcmp(format, "svg") == 0){
        display_path = change_extension(path, "svg");
        intersection_display_path = change_extension(path, "int.svg");
        netlist_display = Netlist_to_svg;
        intersections_display = Netlist_intersections_to_svg;
    } else {
        perror("unknown format");
        exit(1);
    }

    printf("handling `%s` ... ", path);

    Netlist netlist = Netlist_from_file(path);
    netlist_display(&netlist, display_path);

    Vec intersections = compute_intersections(&netlist);
    size_t intersection_count = Vec_len(&intersections);
    Netlist_intersections_to_file(&intersections, intersection_path);
    intersections_display(&netlist, &intersections,
                          display_path, intersection_display_path);
    Vec_plain_drop(&intersections);

    Netlist_drop(&netlist);

    free(intersection_display_path);
    free(display_path);
    free(intersection_path);
    free(path);

    printf(TERM_GREEN("✓")"\n");

    printf("%zu intersections found\n", intersection_count);

    return EXIT_SUCCESS;
}
