#include <time.h>

#include "circuit.h"
#include "output.h"

Vec find_netlists(void);
char* change_extension(const char* path, const char* ext);
void vec_str_drop(char** s);

#define TERM_GREEN(str) "\x1B[32m"str"\033[0m"

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

void vec_str_drop(char** s) {
    free(*s);
}

int main() {
    Vec files = find_netlists();
    size_t files_count = Vec_len(&files);

    clock_t time_mark, delta_time;
    double delta_sec;
    FILE* bench_data = fopen("bench_data", "w");
    fputs("0 0 0\n", bench_data); // placeholder

    for (size_t i = 0; i < files_count; i++) {
        const char* file = *(const char* const*)Vec_get(&files, i);
        printf(" - handling `%s`:\n", file);

        Circuit circuit = Circuit_from_file(file);

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

        Circuit_drop(&circuit);

        fprintf(bench_data, "%zu %u %u\n", i+1, naive_time, sweep_time);
        puts(TERM_GREEN("  ✓"));
    }

    fprintf(bench_data, "%zu 0 0\n", files_count); // placeholder
    fclose(bench_data);

    Vec_drop(&files, (void (*)(void*))vec_str_drop);

    return 0;
}
