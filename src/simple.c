#include "netlist.h"
#include "display.h"

int main() {
    const char* path = "netlists/alea0030_030_10_088.net";

    printf("handling `%s` ... ", path);

    char* display_path = change_extension(path, "svg");
    char* intersection_path = change_extension(path, "int");
    char* intersection_display_path = change_extension(path, "int.svg");
    char* graph_display_path = change_extension(path, "graph.svg");
    char* hv_solution_display_path = change_extension(path, "hvsol.svg");

    Netlist netlist = Netlist_from_file(path);
    Netlist_to_svg(&netlist, display_path);

    Vec intersections = Netlist_intersections_avl_sweep(&netlist);
    Netlist_intersections_to_file(&intersections, intersection_path);
    Netlist_intersections_to_svg(&netlist, &intersections,
                                 display_path, intersection_display_path);
    Vec_plain_drop(&intersections);
    Graph graph = Graph_new(&netlist, intersection_path);
    Graph_to_svg(&graph, &netlist, graph_display_path);
    BitSet solution = Graph_hv_solve(&graph, &netlist);
    Solution_to_svg(&solution, &graph, &netlist, hv_solution_display_path);
    Graph_drop(&graph);

    Netlist_drop(&netlist);

    free(hv_solution_display_path);
    free(graph_display_path);
    free(intersection_display_path);
    free(intersection_path);
    free(display_path);

    printf(TERM_GREEN("✓")"\n");

    return EXIT_SUCCESS;
}
