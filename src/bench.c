#include <time.h>

#include "circuit.h"
#include "output.h"

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
    fprintf(bench_data, "%zu 0 0 0\n", Vec_len(&paths)); // placeholder

    char* path;
    while (Vec_pop(&paths, &path)) {
        printf(" - handling `%s`:\n", path);

        Circuit circuit = Circuit_from_file(path);

        measure_exec_time("   intersections naive",
            Vec intersections = Circuit_intersections_naive(&circuit);
        )
        uint32_t naive_time = (uint32_t)delta_time;
        Vec_plain_drop(&intersections);

        measure_exec_time("   intersections sweep",
            Vec intersections2 = Circuit_intersections_sweep(&circuit);
        )
        uint32_t sweep_time = (uint32_t)delta_time;
        Vec_plain_drop(&intersections2);

        measure_exec_time("   intersections avl sweep",
            Vec intersections3 = Circuit_intersections_avl_sweep(&circuit);
        )
        uint32_t avl_sweep_time = (uint32_t)delta_time;
        Vec_plain_drop(&intersections3);

        Circuit_drop(&circuit);

        fprintf(bench_data, "%zu %u %u %u\n", Vec_len(&paths),
                naive_time, sweep_time, avl_sweep_time);
        puts(TERM_GREEN("   ✓"));
        free(path);
    }

    fputs("0 0 0 0\n", bench_data); // placeholder
    fclose(bench_data);

    Vec_plain_drop(&paths);

    return EXIT_SUCCESS;
}
