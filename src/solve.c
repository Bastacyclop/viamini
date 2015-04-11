#include "util.h"
#include "netlist.h"
#include "display.h"

/// Solves the given netlist.

BitSet odd_cycle_solve_wrapper(const Graph* g, const Netlist* nl);

BitSet odd_cycle_solve_wrapper(const Graph* g, const Netlist* nl) {
    (void) nl;
    return Graph_odd_cycle_solve(g);
}

int main() {
    char file[255];
    ask_str("enter the netlist file name: ", file, 255);
    char* path = str_surround("netlists/", file, ".net");

    char method[20];
    ask_str("choose a method (hv/odd_cycle): ", method, 20);

    BitSet (*solve)(const Graph*, const Netlist*);
    if (strcmp(method, "hv") == 0) {
        solve = Graph_hv_solve;
    } else if (strcmp(method, "odd_cycle") == 0) {
        solve = odd_cycle_solve_wrapper;
    } else {
        perror("unknown method");
        exit(1);
    }

    char* intersection_path = change_extension(path, "int");
    char* graph_display_path = change_extension(path, "graph.ps");
    char* solution_display_path = change_extension(path, "sol.ps");

    printf("handling `%s` ... ", path);

    Netlist netlist = Netlist_from_file(path);
    Graph graph = Graph_new(&netlist, intersection_path);
    Graph_to_ps(&graph, &netlist, graph_display_path);
    BitSet solution = solve(&graph, &netlist);
    Solution_to_ps(&solution, &graph, &netlist, solution_display_path);

    BitSet_drop(&solution);
    Graph_drop(&graph);
    Netlist_drop(&netlist);

    free(solution_display_path);
    free(graph_display_path);
    free(intersection_path);
    free(path);

    printf(TERM_GREEN("✓")"\n");

    return EXIT_SUCCESS;
}
