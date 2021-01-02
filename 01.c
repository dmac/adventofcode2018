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
    error err = file_read_lines("01.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }
    long sum = 0;
    for (int i = 0; i < num_lines; i++) {
        sum += strtol(lines[i], NULL, 10);
    }
    printf("Part 1: %ld\n", sum);

    struct {
        long key;
        bool value;
    } *freqs = NULL;
    sum = 0;
    while (true) {
        for (int i = 0; i < num_lines; i++) {
            sum += strtol(lines[i], NULL, 10);
            if (hmget(freqs, sum)) {
                goto done;
            }
            hmput(freqs, sum, true);
        }
    }
done:
    printf("Part 2: %ld\n", sum);

    return 0;
}
