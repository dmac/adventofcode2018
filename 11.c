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

typedef struct {
    vec2i corner;
    int size;
    int power;
} Window;

int power_level(int serial, int x, int y) {
    int rack_id = x + 10;
    int power = rack_id * y;
    power += serial;
    power *= rack_id;
    power /= 100;
    power %= 10;
    power -= 5;
    return power;
}

vec2i do_part1(int serial, int window) {
    vec2i max_coord = {0};
    int max_power = 0;
    for (int y = 1; y < 300 - window; y++) {
        for (int x = 1; x <= 300 - window; x++) {
            int power = 0;
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
    return max_coord;
}

Window do_part2(int serial) {
    struct {
        Window key; // key.power = 0;
        Window value;
    } *windows = NULL;

    Window max_window = {0};

    for (int window = 1; window <= 300; window++) {
        for (int y = 301 - window; y >= 1; y--) {
            for (int x = 301 - window; x >= 1; x--) {
                Window k = (Window){
                    .corner = (vec2i){.x = x + 1, .y = y + 1},
                    .size = window - 1,
                };
                Window base = hmget(windows, k);
                int power = base.power;
                for (int x0 = x; x0 < x + window; x0++) {
                    power += power_level(serial, x0, y);
                }
                for (int y0 = y + 1; y0 < y + window; y0++) {
                    power += power_level(serial, x, y0);
                }
                k = (Window){
                    .corner.x = x,
                    .corner.y = y,
                    .size = window,
                };
                Window v = k;
                v.power = power;
                hmput(windows, k, v);
                if (v.power > max_window.power) {
                    max_window = v;
                }
            }
        }
    }
    return max_window;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("11.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    int serial = strtol(lines[0], NULL, 10);

    vec2i part1 = do_part1(serial, 3);
    printf("Part 1: %ld,%ld\n", part1.x, part1.y);

    Window part2 = do_part2(serial);
    printf("Part 2: %ld,%ld,%d\n", part2.corner.x, part2.corner.y, part2.size);

    return 0;
}
