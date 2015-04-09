#include "netlist.h"
#include "display.h"

/// Solves the given netlist.

int main() {
    char file[255];
    ask_str("enter the netlist file name: ", file, 255);
    char* path = str_surround("netlists/", file, ".net");

    char* intersection_path = change_extension(path, "int");
    char* solution_display_path = change_extension(path, "sol.svg");

    printf("handling `%s` ... ", path);

    Netlist netlist = Netlist_from_file(path);
    Graph graph = Graph_new(&netlist, intersection_path);
    BitSet solution = Graph_odd_cycle_solve(&graph);
    Solution_to_svg(&solution, &graph, &netlist, solution_display_path);

    BitSet_drop(&solution);
    Graph_drop(&graph);
    Netlist_drop(&netlist);

    free(solution_display_path);
    free(intersection_path);
    free(path);

    printf(TERM_GREEN("✓")"\n");

    return EXIT_SUCCESS;
}
