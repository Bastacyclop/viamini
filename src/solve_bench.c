#include <time.h>

#include "util.h"
#include "netlist.h"

/// Compares the different solving methods.

#define measure_exec_time(msg, block)                                   \
    time_mark = clock();                                                \
    block                                                               \
    delta_time = clock() - time_mark;                                   \
    delta_sec = ((double)(delta_time))/CLOCKS_PER_SEC;                  \
    printf(msg": %f s - %d clocks\n", delta_sec, (int)delta_time);

int main() {
    Vec paths = find("netlists/*.net");

    clock_t time_mark, delta_time;
    double delta_sec;
    FILE* bench_data = fopen("solve_bench/data", "w");
    fprintf(bench_data, "%zu 0 0 0 0 0\n", Vec_len(&paths) + 1); // placeholder

    char* path;
    while (Vec_pop(&paths, &path)) {
        printf(" - handling `%s`:\n", path);

        Netlist netlist = Netlist_from_file(path);

        char* intersection_path = change_extension(path, "int");
        Graph graph = Graph_new(&netlist, intersection_path);
        size_t node_count = Vec_len(&graph.nodes);
        free(intersection_path);

        BitSet solution;

        measure_exec_time("   horizontal/vertical",
            solution = Graph_hv_solve(&graph, &netlist);
        )
        uint32_t hv_time = (uint32_t)delta_time;
        size_t hv_via_count = Solution_via_count(&solution, &graph);
        BitSet_drop(&solution);

        measure_exec_time("   odd cycles",
            solution = Graph_odd_cycle_solve(&graph);
        )
        uint32_t odd_cycle_time = (uint32_t)delta_time;
        size_t odd_cycle_via_count = Solution_via_count(&solution, &graph);
        BitSet_drop(&solution);

        Graph_drop(&graph);
        Netlist_drop(&netlist);

        fprintf(bench_data, "%zu %zu %u %zu %u %zu\n", Vec_len(&paths) + 1, node_count,
                hv_time, hv_via_count, odd_cycle_time, odd_cycle_via_count);
        puts(TERM_GREEN("   ✓"));
        free(path);
    }

    fputs("0 0 0 0 0 0\n", bench_data); // placeholder
    fclose(bench_data);

    Vec_drop(&paths);

    return EXIT_SUCCESS;
}
