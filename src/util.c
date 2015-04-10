#include "util.h"

void cut_at_newline(char* s) {
    char* pos = strchr(s, '\n');
    if (pos) { *pos = '\0'; }
}

char* change_extension(const char* path, const char* ext) {
    char* sep = strchr(path, '.');
    assert(sep);
    size_t name_len = sep - path;
    size_t ext_len = strlen(ext);

    // +1 for `.` and +1 for `\0`
    char* res = malloc((name_len + 1 + ext_len + 1)*sizeof(char));
    memcpy(res, path, name_len);
    res[name_len] = '.';
    memcpy(res + name_len + 1, ext, ext_len + 1);
    return res;
}

StrVec find(const char* pattern) {
    char* cmd = str_surround("ls ", pattern, " > find.tmp");
    system(cmd);
    free(cmd);
    FILE* f = fopen("find.tmp", "r");

    Vec lines = Vec_new(sizeof(char*));
    char line[255];
    while (fgets(line, 255, f)) {
        cut_at_newline(line);
        char* l = str_clone(line);
        Vec_push(&lines, &l);
    }

    fclose(f);
    system("rm find.tmp");

    return lines;
}

void ask(const char* msg, const char* format, void* ptr) {
    char input[255];
    LOOP {
        ask_str(msg, input, 255);
        if (sscanf(input, format, ptr) == 1) break;
    }
}

void ask_str(const char* msg, char* s, size_t max_len) {
    fputs(msg, stdout);
    fgets(s, max_len, stdin);
    cut_at_newline(s);
}

char* str_clone(const char* s) {
    size_t len = strlen(s);
    char* clone = malloc((len + 1)*sizeof(char)); // + 1 for '\0'
    assert_alloc(clone);
    memcpy(clone, s, (len + 1));
    return clone;
}

char* str_surround(const char* prefix, const char* s, const char* suffix) {
    size_t pre_len = strlen(prefix);
    size_t s_len = strlen(s);
    size_t suf_len = strlen(suffix);

    char* res = malloc(pre_len + s_len + suf_len + 1); // + 1 for '\0'
    memcpy(res, prefix, pre_len);
    memcpy(res + pre_len, s, s_len);
    memcpy(res + pre_len + s_len, suffix, suf_len + 1);

    return res;
}
