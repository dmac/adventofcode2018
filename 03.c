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
    int w;
    int h;
} Claim;

Claim parse_claim(char *s) {
    Claim c;
    int matched = sscanf(s, "#%d @ %d,%d: %dx%d", &c.id, &c.x, &c.y, &c.w, &c.h);
    if (matched != 5) {
        fatal("error parsing %s\n", s);
    }
    return c;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("03.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    Claim *claims = NULL;
    for (int i = 0; i < num_lines; i++) {
        arrput(claims, parse_claim(lines[i]));
    }

    int fabric[1000][1000] = {0};
    for (int i = 0; i < arrlen(claims); i++) {
        Claim *c = &claims[i];
        for (int x = c->x; x < c->x + c->w; x++) {
            for (int y = c->y; y < c->y + c->h; y++) {
                fabric[x][y]++;
            }
        }
    }

    int part1 = 0;
    for (int x = 0; x < 1000; x++) {
        for (int y = 0; y < 1000; y++) {
            if (fabric[x][y] >= 2) {
                part1++;
            }
        }
    }
    printf("Part 1: %d\n", part1);

    for (int i = 0; i < arrlen(claims); i++) {
        Claim *c = &claims[i];
        bool single = true;
        for (int x = c->x; x < c->x + c->w; x++) {
            for (int y = c->y; y < c->y + c->h; y++) {
                if (fabric[x][y] != 1) {
                    single = false;
                    goto next;
                }
            }
        }
        if (single) {
            printf("Part 2: %d\n", c->id);
            goto done;
        }
    next:;
    }
done:
    return 0;
}
