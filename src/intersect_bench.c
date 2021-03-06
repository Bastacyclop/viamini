#include <time.h>

#include "util.h"
#include "netlist.h"

/// Compares the different intersection finding methods.

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
    FILE* bench_data = fopen("intersect_bench/data", "w");
    fprintf(bench_data, "%zu 0 0 0 0 0\n", Vec_len(&paths) + 1); // placeholder

    char* path;
    while (Vec_pop(&paths, &path)) {
        printf(" - handling `%s`:\n", path);

        Netlist netlist = Netlist_from_file(path);
        size_t seg_count = Netlist_segment_count(&netlist);

        Vec intersections;

        measure_exec_time("   naive",
            intersections = Netlist_intersections_naive(&netlist);
        )
        uint32_t naive_time = (uint32_t)delta_time;
        Vec_drop(&intersections);

        measure_exec_time("   vec sweep",
            intersections = Netlist_intersections_vec_sweep(&netlist);
        )
        uint32_t vec_sweep_time = (uint32_t)delta_time;
        Vec_drop(&intersections);

        measure_exec_time("   list sweep",
            intersections = Netlist_intersections_list_sweep(&netlist);
        )
        uint32_t list_sweep_time = (uint32_t)delta_time;
        Vec_drop(&intersections);

        measure_exec_time("   avl sweep",
            intersections = Netlist_intersections_avl_sweep(&netlist);
        )
        uint32_t avl_sweep_time = (uint32_t)delta_time;
        Vec_drop(&intersections);

        Netlist_drop(&netlist);

        fprintf(bench_data, "%zu %zu %u %u %u %u\n", Vec_len(&paths) + 1, seg_count,
                naive_time, vec_sweep_time, list_sweep_time, avl_sweep_time);
        puts(TERM_GREEN("   ✓"));
        free(path);
    }

    fputs("0 0 0 0 0 0\n", bench_data); // placeholder
    fclose(bench_data);

    Vec_drop(&paths);

    return EXIT_SUCCESS;
}
