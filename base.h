#ifndef AOC2018
#define AOC2018

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

typedef char *error;

typedef struct {
    size_t len;
    char *s;
} string;

error file_read_lines(char *filename, char ***lines, int64_t *num_lines) {
    if (lines == NULL) {
        return "input parameter lines is NULL";
    }
    if (num_lines == NULL) {
        return "input parameter num_lines is NULL";
    }
    FILE *f = fopen(filename, "r");
    if (!f) {
        return strerror(errno);
    }

    fseek(f, 0, SEEK_END);
    int64_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *b0 = malloc(size + 1);
    int64_t nread = fread(b0, 1, size, f);
    if (nread != size) {
        return "short read";
    }
    b0[size] = '\0';

    int64_t nlines = 0;
    for (size_t i = 0; i < size; i++) {
        if (b0[i] == '\n') {
            b0[i] = '\0';
            nlines++;
        }
    }

    char **b1 = malloc(nlines * sizeof(*b1) + size + 1);
    memcpy(b1 + nlines, b0, size + 1);
    free(b0);
    b0 = (char *)(b1 + nlines);

    int64_t nline = 0;
    char *line = b0;
    for (size_t i = 0; i < size; i++) {
        if (b0[i] == 0) {
            b1[nline++] = line;
            line = &b0[i + 1];
        }
    }

    *lines = b1;
    if (num_lines) {
        *num_lines = nlines;
    }
    return NULL;
}

#define fatal(...) _fatal(__FILE__, __LINE__, __VA_ARGS__)

void _fatal(char *file, int line, char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "%s:%d: ", file, line);
    vfprintf(stderr, format, args);
    exit(1);
}

#define ARRLEN(a) (sizeof((a)) / sizeof(*(a)))

int compare_strings(const void *s0, const void *s1) {
    return strcmp(*(const char **)s0, *(const char **)s1);
}

void sort_strings(char **strings, long num_strings) {
    qsort(strings, num_strings, sizeof(*strings), compare_strings);
}

string cstr_copy(char *s) {
    string t = (string){.len = strlen(s)};
    t.s = malloc(t.len + 1);
    strcpy(t.s, s);
    return t;
}

#endif
