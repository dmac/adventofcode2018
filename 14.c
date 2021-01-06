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
    error err = file_read_lines("14.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }
    long target = strtol(lines[0], NULL, 10);
    long *recipes = NULL;
    arrput(recipes, 3);
    arrput(recipes, 7);
    long elf0 = 0;
    long elf1 = 1;

    long *input = NULL;
    for (long t = target; t > 0; t /= 10) {
        arrput(input, t % 10);
    }
    for (long i = arrlen(input) - 1; i >= 0; i--) {
        arrput(input, input[i]);
    }
    for (long i = 0, len = arrlen(input); i < len / 2; i++) {
        arrdel(input, 0);
    }

    long part2 = 0;
    long p2i = 0;
    for (; arrlen(recipes) < target + 10 || part2 == 0;) {
        long sum = recipes[elf0] + recipes[elf1];
        long *next = NULL;
        if (sum == 0) {
            arrput(next, sum);
        }
        for (; sum > 0; sum /= 10) {
            arrput(next, sum % 10);
        }
        for (long i = arrlen(next) - 1; i >= 0; i--) {
            arrput(recipes, next[i]);
        }
        arrfree(next);
        elf0 = (elf0 + recipes[elf0] + 1) % arrlen(recipes);
        elf1 = (elf1 + recipes[elf1] + 1) % arrlen(recipes);

        if (part2 == 0) {
            for (; p2i < arrlen(recipes) - arrlen(input); p2i++) {
                fori(i, arrlen(input)) {
                    if (recipes[p2i + i] != input[i]) {
                        goto outer;
                    }
                }
                part2 = p2i;
            outer:;
            }
        }
    }
    printf("Part 1: ");
    for (long i = target; i < target + 10; i++) {
        printf("%ld", recipes[i]);
    }
    printf("\n");

    printf("Part 2: %ld\n", part2);

    return 0;
}
