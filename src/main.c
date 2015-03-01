#include <pthread.h>

#include "circuit.h"
#include "output.h"

Vec find_netlists(void);
void handle(char* path);

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

void handle(char* path) {
    printf("handling `%s`.\n", path);

    char* display_path = change_extension(path, "svg");
    char* intersection_path = change_extension(path, "intersection.svg");

    Circuit circuit = Circuit_from_file(path);
    Circuit_to_svg(&circuit, display_path);

    Vec intersections = Circuit_intersections_avl_sweep(&circuit);
    Circuit_intersections_to_svg(&circuit, &intersections,
                                 display_path, intersection_path);
    Vec_plain_drop(&intersections);

    Circuit_drop(&circuit);

    free(display_path);
    free(intersection_path);

    printf("`%s` "TERM_GREEN("✓")"\n", path);
    free(path);
}

int main() {
    Vec paths = find_netlists();
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

    Vec_plain_drop(&paths);

    pthread_t thread;
    while (Vec_pop(&threads, &thread)) {
        if (pthread_join(thread, NULL) != 0) {
            perror("cannot join thread");
            exit(1);
        }
    }

    Vec_plain_drop(&threads);

    return EXIT_SUCCESS;
}
