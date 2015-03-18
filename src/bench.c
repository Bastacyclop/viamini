#include <time.h>

#include "netlist.h"

Vec find_netlists(void);
void vec_str_drop(char** s);

#define measure_exec_time(msg, block)                                   \
    time_mark = clock();                                                \
    block                                                               \
    delta_time = clock() - time_mark;                                   \
    delta_sec = ((double)(delta_time))/CLOCKS_PER_SEC;                  \
    printf(msg": %f s - %d clocks\n", delta_sec, (int)delta_time);

Vec find_netlists() {
    system("find netlists/*.net > netlists.tmp");
    FILE* f = fopen("netlists.tmp", "r");

    Vec files = Vec_new(sizeof(char*));
    char line[255];
    while (fgets(line, 255, f)) {
        cut_at_newline(line);
        char* l = str_clone(line);
        Vec_push(&files, &l);
    }

    fclose(f);
    system("rm netlists.tmp");

    return files;
}

int main() {
    Vec paths = find_netlists();

    clock_t time_mark, delta_time;
    double delta_sec;
    FILE* bench_data = fopen("bench_data", "w");
    fprintf(bench_data, "%zu 0 0 0 0\n", Vec_len(&paths) + 1); // placeholder

    char* path;
    while (Vec_pop(&paths, &path)) {
        printf(" - handling `%s`:\n", path);

        Netlist netlist = Netlist_from_file(path);

        measure_exec_time("   intersections naive",
            Vec intersections = Netlist_intersections_naive(&netlist);
        )
        uint32_t naive_time = (uint32_t)delta_time;
        Vec_plain_drop(&intersections);

        measure_exec_time("   intersections vec sweep",
            Vec intersections2 = Netlist_intersections_vec_sweep(&netlist);
        )
        uint32_t vec_sweep_time = (uint32_t)delta_time;
        Vec_plain_drop(&intersections2);

        measure_exec_time("   intersections list sweep",
            Vec intersections3 = Netlist_intersections_list_sweep(&netlist);
        )
        uint32_t list_sweep_time = (uint32_t)delta_time;
        Vec_plain_drop(&intersections3);

        measure_exec_time("   intersections avl sweep",
            Vec intersections4 = Netlist_intersections_avl_sweep(&netlist);
        )
        uint32_t avl_sweep_time = (uint32_t)delta_time;
        Vec_plain_drop(&intersections4);

        Netlist_drop(&netlist);

        fprintf(bench_data, "%zu %u %u %u %u\n", Vec_len(&paths) + 1,
                naive_time, vec_sweep_time, list_sweep_time, avl_sweep_time);
        puts(TERM_GREEN("   ✓"));
        free(path);
    }

    fputs("0 0 0 0 0\n", bench_data); // placeholder
    fclose(bench_data);

    Vec_plain_drop(&paths);

    return EXIT_SUCCESS;
}
