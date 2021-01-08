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
    int id;
    int x;
    int y;
} Coord;

typedef struct {
    Coord key;
    bool value;
} CoordSet;

typedef struct {
    int len;
    int *distances;
} GridInfo;

typedef struct {
    int w;
    int h;
    GridInfo *grid;
} Grid;

Coord parse_coord(char *s) {
    static int id = 1;
    Coord c = (Coord){.id = id};
    id++;
    int n = sscanf(s, "%d, %d", &c.x, &c.y);
    if (n != 2) {
        fatal("unexpected input\n");
    }
    return c;
}

int idx(Grid *grid, int x, int y) {
    return y * grid->w + x;
}

bool in_bounds(Grid *grid, Coord *coord) {
    return coord->x >= 0 && coord->x < grid->w &&
           coord->y >= 0 && coord->y < grid->h;
}

void fill_for_point(Grid *grid, Coord *coord) {
    bool *seen = calloc(grid->w * grid->h, sizeof(*seen));
    Coord *queue = NULL;
    Coord init;
    init = (Coord){.x = coord->x, .y = coord->y};
    seen[init.y * grid->w + init.x] = true;
    arrput(queue, init);
    int head = 0;
    for (; arrlen(queue) - head > 0;) {
        Coord next = queue[head];
        head++;
        grid->grid[idx(grid, next.x, next.y)].distances[coord->id] = next.id;
        struct {
            int x;
            int y;
        } diffs[] = {
            {-1, 0},
            {1, 0},
            {0, -1},
            {0, 1},
        };
        for (int i = 0; i < ARRLEN(diffs); i++) {
            Coord neighbor;
            neighbor = (Coord){
                .id = next.id + 1,
                .x = next.x + diffs[i].x,
                .y = next.y + diffs[i].y,
            };
            int seenidx = neighbor.y * grid->w + neighbor.x;
            if (in_bounds(grid, &neighbor) && !seen[seenidx]) {
                seen[seenidx] = true;
                arrput(queue, neighbor);
            }
        }
    }
}

int id_of_min_distance(GridInfo *gi) {
    int min = INT_MAX;
    int min_id = 0;
    for (int i = 1; i < gi->len; i++) {
        if (gi->distances[i] < min) {
            min = gi->distances[i];
            min_id = i;
        }
    }
    for (int i = 1; i < gi->len; i++) {
        if (gi->distances[i] == min && i != min_id) {
            return 0;
        }
    }
    return min_id;
}

int sum_distances(GridInfo *gi) {
    int sum = 0;
    for (int i = 1; i < gi->len; i++) {
        sum += gi->distances[i];
    }
    return sum;
}

int area_of(Grid *grid, Coord *p) {
    struct {
        int x;
        int y;
    } limits[] = {
        {0, p->y},
        {grid->w - 1, p->y},
        {p->x, 0},
        {p->x, grid->h - 1},
    };
    for (int i = 0; i < ARRLEN(limits); i++) {
        GridInfo *gi = &grid->grid[idx(grid, limits[i].x, limits[i].y)];
        if (id_of_min_distance(gi) == p->id) {
            return 0;
        }
    }
    int area = 0;
    for (int i = 0; i < grid->w * grid->h; i++) {
        GridInfo *gi = &grid->grid[i];
        int curr = id_of_min_distance(gi);
        if (curr == p->id) {
            area++;
        }
    }
    return area;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("06.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    Coord *coords = NULL;
    for (int i = 0; i < num_lines; i++) {
        arrput(coords, parse_coord(lines[i]));
    }

    int maxx = 0;
    int maxy = 0;
    for (int i = 0; i < arrlen(coords); i++) {
        Coord c = coords[i];
        if (c.x > maxx) {
            maxx = c.x;
        }
        if (c.y > maxy) {
            maxy = c.y;
        }
    }

    Grid grid = (Grid){
        .w = maxx + 1,
        .h = maxy + 1,
    };
    grid.grid = calloc(sizeof(*grid.grid), grid.w * grid.h);
    int *distances = calloc(sizeof(int), grid.w * grid.h * (arrlen(coords) + 1));
    for (int i = 0; i < grid.w * grid.h; i++) {
        GridInfo *gi = &grid.grid[i];
        gi->len = arrlen(coords) + 1;
        gi->distances = distances + i * (arrlen(coords) + 1);
    }

    for (int i = 0; i < arrlen(coords); i++) {
        fill_for_point(&grid, &coords[i]);
    }
    int largest = 0;
    for (int i = 0; i < arrlen(coords); i++) {
        int area = area_of(&grid, &coords[i]);
        if (area > largest) {
            largest = area;
        }
    }

    int within = 0;
    for (int i = 0; i < grid.w * grid.h; i++) {
        GridInfo *gi = &grid.grid[i];
        int sum = sum_distances(gi);
        if (sum < 10000) {
            within++;
        }
    }

    printf("Part 1: %d\n", largest);
    printf("Part 2: %d\n", within);
    return 0;
}
