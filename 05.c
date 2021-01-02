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

int expand(string s, int i) {
    int deleted = 0;
    int j = i + 1;
    for (; i >= 0 && j < s.len;) {
        if (s.s[i] != s.s[j] && toupper(s.s[i]) == toupper(s.s[j])) {
            deleted += 2;
            s.s[i] = ' ';
            s.s[j] = ' ';
        } else {
            break;
        }
        for (i = i - 1; s.s[i] == ' '; i--)
            ;
        for (j = j + 1; s.s[j] == ' '; j++)
            ;
    }
    return deleted;
}

int simulate(string s) {
    int newlen = s.len;
    for (int i = 0; i < s.len - 1; i++) {
        int c0 = s.s[i];
        int c1 = s.s[i + 1];
        if (c0 != c1 && toupper(c0) == toupper(c1)) {
            int deleted = expand(s, i);
            i += deleted / 2;
            newlen -= deleted;
        }
    }
    return newlen;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("05.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    string line = cstr_copy(lines[0]);
    printf("Part 1: %d\n", simulate(line));

    line = cstr_copy(lines[0]);
    int64_t min = INT64_MAX;
    for (int c = 'a'; c <= 'z'; c++) {
        int len = simulate(remove_all(line, c));
        if (len < min) {
            min = len;
        }
    }
    printf("Part 2: %ld\n", min);

    return 0;
}
