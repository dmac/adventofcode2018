#if 0
set -e
src="$(basename $0)"
exe="$(echo $src | cut -f1 -d.)"
trap "rm -f $exe" EXIT
gcc -std=gnu99 -Wall -Werror -pedantic -O2 -g -o $exe $src
./$exe
exit $?
#endif

#include "base.h"

string step(string s) {
    for (int i = 0; i < s.len - 1; i++) {
        int c0 = s.s[i];
        int c1 = s.s[i + 1];
        if (c0 != c1 && toupper(c0) == toupper(c1)) {
            string t = (string){.len = s.len - 2};
            t.s = malloc(t.len + 1);
            memcpy(t.s, s.s, i);
            memcpy(t.s + i, s.s + i + 2, t.len - i);
            t.s[t.len] = '\0';
            return t;
        }
    }
    return s;
}

string step_all(string line) {
    for (;;) {
        string next = step(line);
        if (line.len == next.len) {
            return line;
        }
        line = next;
    }
}

string remove_all(string s, int c) {
    string t = (string){.len = s.len};
    t.s = malloc(t.len + 1);
    strcpy(t.s, s.s);
    for (int i = 0; i < t.len;) {
        if (t.s[i] == tolower(c) || t.s[i] == toupper(c)) {
            memmove(&t.s[i], &t.s[i + 1], t.len - i);
            t.len--;
            continue;
        }
        i++;
    }
    return t;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("05.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    string line = (string){
        .len = strlen(lines[0]),
        .s = lines[0],
    };

    string final = step_all(line);
    printf("Part 1: %zu\n", final.len);

    int64_t min = INT64_MAX;
    for (int c = 'a'; c <= 'z'; c++) {
        string s = step_all(remove_all(line, c));
        if (s.len < min) {
            min = s.len;
        }
    }
    printf("Part 2: %ld\n", min);

    return 0;
}
