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

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("02.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    int exactly2 = 0;
    int exactly3 = 0;
    for (int i = 0; i < num_lines; i++) {
        int counts[26] = {0};
        for (int j = 0; j < strlen(lines[i]); j++) {
            counts[lines[i][j] - 'a']++;
        }
        bool did2 = false;
        bool did3 = false;
        for (int j = 0; j < ARRLEN(counts); j++) {
            if (counts[j] == 2 && !did2) {
                exactly2++;
                did2 = true;
            }
            if (counts[j] == 3 && !did3) {
                exactly3++;
                did3 = true;
            }
        }
    }

    printf("Part 1: %d\n", exactly2 * exactly3);

    for (int i = 0; i < num_lines - 1; i++) {
        for (int j = i + 1; j < num_lines; j++) {
            int diffs = 0;
            for (int ix = 0; ix < strlen(lines[i]); ix++) {
                if (lines[i][ix] != lines[j][ix]) {
                    diffs++;
                }
            }
            if (diffs == 1) {
                printf("Part 2: ");
                for (int ix = 0; ix < strlen(lines[i]); ix++) {
                    if (lines[i][ix] == lines[j][ix]) {
                        printf("%c", lines[i][ix]);
                    }
                }
                printf("\n");
            }
        }
    }

    return 0;
}
