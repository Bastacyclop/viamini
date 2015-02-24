#include "circuit.h"
#include "output.h"

Vec find_netlists(void);
char* change_extension(const char* path, const char* ext);
void vec_str_drop(char** s);

#define TERM_GREEN(str) "\x1B[32m"str"\033[0m"

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

char* change_extension(const char* path, const char* ext) {
    char* sep = strchr(path, '.');
    assert(sep);
    size_t name_len = sep - path;
    size_t ext_len = strlen(ext);
    // +1 for `.` and +1 for `\0`
    char* res = malloc((name_len+1+ext_len+1)*sizeof(char));
    memcpy(res, path, name_len);
    res[name_len] = '.';
    memcpy(res+name_len+1, ext, ext_len+1);
    return res;
}

void vec_str_drop(char** s) {
    free(*s);
}

int main() {
    char* test = change_extension("test.net", "ps");
    assert(strcmp(test, "test.ps") == 0);
    free(test);

    Vec files = find_netlists();
    size_t files_count = Vec_len(&files);

    for (size_t i = 0; i < files_count; i++) {
        const char* file = *(const char* const*)Vec_get(&files, i);
        printf(" - handling `%s` ... ", file); fflush(stdout);

        char* display_file = change_extension(file, "svg");
        char* intersection_file = change_extension(file, "intersection.svg");
        char* intersection2_file = change_extension(file, "intersection2.svg");

        Circuit circuit = Circuit_from_file(file);
        Circuit_to_svg(&circuit, display_file);

        /*
        Vec intersections = Circuit_intersections_naive(&circuit);
        Circuit_intersections_to_svg(&circuit, &intersections,
                                     display_file, intersection_file);
        Vec_plain_drop(&intersections);
        */

        Vec intersections2 = Circuit_intersections_sweep(&circuit);
        Circuit_intersections_to_svg(&circuit, &intersections2,
                                     display_file, intersection2_file);
        Vec_plain_drop(&intersections2);

        Circuit_drop(&circuit);

        free(display_file);
        free(intersection_file);
        free(intersection2_file);

        puts(TERM_GREEN("✓"));
    }

    Vec_drop(&files, (void (*)(void*))vec_str_drop);

    return 0;
}
