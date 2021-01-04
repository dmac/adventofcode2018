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
    vec2i pos;
    vec2i vel;
} Point;

typedef struct {
    vec2i key;   // pos
    vec2i value; // vel
} PointKV;

Point parse_point(char *line) {
    Point p = {0};
    int n = sscanf(line, "position=< %ld, %ld> velocity=< %ld, %ld>",
                   &p.pos.x, &p.pos.y, &p.vel.x, &p.vel.y);
    if (n != 4) {
        fatal("parse error\n");
    }
    return p;
}

bool print_grid(Point *points, int maxx, int maxy) {
    vec2i min = {.x = INT64_MAX, .y = INT64_MAX};
    vec2i max = {.x = 0, .y = 0};
    forpl(point, points, arrlen(points)) {
        if (point->pos.x < min.x) {
            min.x = point->pos.x;
        }
        if (point->pos.y < min.y) {
            min.y = point->pos.y;
        }
        if (point->pos.x > max.x) {
            max.x = point->pos.x;
        }
        if (point->pos.y > max.y) {
            max.y = point->pos.y;
        }
    }

    vec2i shift = {.x = -min.x, .y = -min.y};
    vec2i dims = vec2i_add(max, shift);
    dims.x++;
    dims.y++;

    if (dims.x > maxx || dims.y > maxy) {
        return false;
    }
    printf("Part 1:\n");

    char *grid = calloc(dims.x * dims.y, sizeof(*grid));
    memset(grid, '.', dims.x * dims.y * sizeof(*grid));
    forpl(point, points, arrlen(points)) {
        vec2i pos = vec2i_add(point->pos, shift);
        grid[pos.x + dims.x * pos.y] = '#';
    }

    fori(i, dims.x * dims.y) {
        if (i > 0 && i % dims.x == 0) {
            printf("\n");
        }
        printf("%c", grid[i]);
    }
    printf("\n");

    free(grid);
    return true;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("10.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    Point *points = NULL;
    forvl(line, lines, num_lines) {
        arrput(points, parse_point(line));
    }

    for (int i = 1;; i++) {
        forpl(point, points, arrlen(points)) {
            point->pos = vec2i_add(point->pos, point->vel);
        }
        if (print_grid(points, 70, 70)) {
            printf("Part 2: %d\n", i);
            break;
        }
    }
    return 0;
}
