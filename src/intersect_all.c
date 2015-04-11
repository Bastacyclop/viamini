#include <pthread.h>

#include "util.h"
#include "netlist.h"
#include "display.h"

/// Finds the intersections of all the netlists.

void handle(char* path);

void handle(char* path) {
    printf("handling `%s`.\n", path);

    char* intersection_path = change_extension(path, "int");

    Netlist netlist = Netlist_from_file(path);
    Vec intersections = Netlist_intersections_avl_sweep(&netlist);
    Netlist_intersections_to_file(&intersections, intersection_path);

    Vec_drop(&intersections);
    Netlist_drop(&netlist);

    free(intersection_path);

    printf("`%s` "TERM_GREEN("✓")"\n", path);
    free(path);
}

int main() {
    Vec paths = find("-S netlists/*.net");
    size_t path_count = Vec_len(&paths);

    Vec threads = Vec_with_capacity(path_count, sizeof(pthread_t));
    char* path;
    while (Vec_pop(&paths, &path)) {
        pthread_t thread;
        if (pthread_create(&thread, NULL, (void* (*)(void*))handle, path)) {
            perror("cannot create new thread");
            exit(1);
        }
        Vec_push(&threads, &thread);
    }

    Vec_drop(&paths);

    pthread_t thread;
    while (Vec_pop(&threads, &thread)) {
        if (pthread_join(thread, NULL) != 0) {
            perror("cannot join thread");
            exit(1);
        }
    }

    Vec_drop(&threads);

    return EXIT_SUCCESS;
}
