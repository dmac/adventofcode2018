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

int64_t power_level(int64_t serial, int64_t x, int64_t y) {
    int64_t rack_id = x + 10;
    int64_t power = rack_id * y;
    power += serial;
    power *= rack_id;
    power /= 100;
    power %= 10;
    power -= 5;
    return power;
}

vec2i max_power_square(int serial, int window, int64_t *power) {
    vec2i max_coord = {0};
    int64_t max_power = 0;
    for (int y = 1; y < 300 - window; y++) {
        for (int x = 1; x <= 300 - window; x++) {
            int64_t power = 0;
            for (int dy = 0; dy < window; dy++) {
                for (int dx = 0; dx < window; dx++) {
                    power += power_level(serial, x + dx, y + dy);
                }
            }
            if (power > max_power) {
                max_power = power;
                max_coord = (vec2i){x, y};
            }
        }
    }
    if (power != NULL) {
        *power = max_power;
    }
    return max_coord;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("11.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    int serial = strtol(lines[0], NULL, 10);

    vec2i part1 = max_power_square(serial, 3, NULL);
    printf("Part 1: %ld,%ld\n", part1.x, part1.y);

    vec2i part2 = {0};
    int max_window = 0;
    int64_t max_power = 0;
    int64_t power = 0;
    for (int window = 1; window <= 300; window++) {
        vec2i coord = max_power_square(serial, window, &power);
        if (power > max_power) {
            max_power = power;
            max_window = window;
            part2 = coord;
        }
    }
    printf("Part 2: %ld,%ld,%d\n", part2.x, part2.y, max_window);

    return 0;
}
