#if 0
set -e
src="$(basename $0)"
exe="$(echo $src | cut -f1 -d.)"
trap "rm -f $exe" EXIT
gcc -std=gnu99 -Wall -Werror -pedantic -O3 -g -o $exe $src
./$exe
exit $?
#endif

#include "base.h"

typedef enum {
    ELF = 'E',
    GOBLIN = 'G',
} UnitType;

typedef struct Tile Tile;

typedef struct {
    UnitType type;
    long health;
    long attack;
    Tile *tile;
} Unit;

struct Tile {
    long row;
    long col;
    Unit *unit;
    bool wall;
};

typedef struct {
    Tile *tile;
    long dist;
} TileDistance;

typedef struct {
    long w;
    long h;
    Tile *tiles;
    Unit *units;
} World;

World build_world(char **lines, long num_lines, long elf_attack) {
    World w = {
        .w = strlen(lines[0]),
        .h = num_lines,
    };
    long num_units = 0;
    forvl(line, lines, num_lines) {
        forvl(ch, line, strlen(line)) {
            if (ch == ELF || ch == GOBLIN) {
                num_units++;
            }
        }
    }
    arrsetcap(w.units, num_units);
    w.tiles = calloc(w.w * w.h, sizeof(*w.tiles));
    fori(r, num_lines) {
        char *line = lines[r];
        long len = strlen(line);
        assert(w.w == len);
        fori(c, len) {
            w.tiles[r * w.w + c] = (Tile){
                .wall = line[c] == '#',
                .row = r,
                .col = c,
            };
            if (line[c] == ELF || line[c] == GOBLIN) {
                Unit u = (Unit){
                    .health = 200,
                };
                switch (line[c]) {
                case ELF:
                    u.type = ELF;
                    u.attack = elf_attack;
                    break;
                case GOBLIN:
                    u.type = GOBLIN;
                    u.attack = 3;
                    break;
                default:
                    assert(false);
                }
                arrput(w.units, u);
                Tile *tp = &w.tiles[r * w.w + c];
                Unit *up = &w.units[arrlen(w.units) - 1];
                tp->unit = up;
                up->tile = tp;

            } else {
                assert(line[c] == '.' || line[c] == '#');
            }
        }
    }
    return w;
}

void print_world(World *w) {
    fori(r, w->h) {
        fori(c, w->w) {
            Tile t = w->tiles[r * w->w + c];
            if (t.unit != NULL) {
                printf("%c", t.unit->type);
            } else if (t.wall) {
                printf("#");
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

bool in_bounds(World *w, long row, long col) {
    return row >= 0 && row < w->h &&
           col >= 0 && col < w->w;
}

Tile **determine_targets(World *w, Tile *t) {
    Tile **targets = NULL;
    forpl(tt, w->tiles, w->w * w->h) {
        if (tt->unit != NULL && t->unit->type != tt->unit->type) {
            arrput(targets, tt);
        }
    }
    return targets;
}

bool is_open(Tile *t) {
    return !t->wall && !t->unit;
}

Tile **tiles_in_range(World *w, Tile *t) {
    Tile **ts = NULL;
    if (t->row > 0) {
        Tile *tt = &w->tiles[(t->row - 1) * w->w + t->col];
        arrput(ts, tt);
    }
    if (t->col > 0) {
        Tile *tt = &w->tiles[t->row * w->w + (t->col - 1)];
        arrput(ts, tt);
    }
    if (t->col < w->w - 1) {
        Tile *tt = &w->tiles[t->row * w->w + (t->col + 1)];
        arrput(ts, tt);
    }
    if (t->row < w->h - 1) {
        Tile *tt = &w->tiles[(t->row + 1) * w->w + t->col];
        arrput(ts, tt);
    }
    return ts;
}

long compute_distance(World *w, Tile *t0, Tile *t1) {
    TileDistance *queue = NULL;
    long head = 0;
    struct {
        Tile *key;
        bool value;
    } *seen = NULL;
    TileDistance first = (TileDistance){
        .tile = t0,
        .dist = 0,
    };
    long distance = -1;
    arrput(queue, first);
    hmput(seen, t0, true);
    for (; arrlen(queue) - head > 0;) {
        TileDistance td = queue[head++];
        if (td.tile == t1) {
            distance = td.dist;
            goto end;
        }
        Tile **in_range = tiles_in_range(w, td.tile);
        forvl(nt, in_range, arrlen(in_range)) {
            if (hmget(seen, nt)) {
                continue;
            }
            hmput(seen, nt, true);
            if (is_open(nt)) {
                TileDistance ntd = (TileDistance){
                    .tile = nt,
                    .dist = td.dist + 1,
                };
                arrput(queue, ntd);
            }
        }
        arrfree(in_range);
    }
end:
    arrfree(queue);
    hmfree(seen);
    return distance;
}

int compare_tile_distance(const void *v0, const void *v1) {
    const TileDistance *td0 = v0;
    const TileDistance *td1 = v1;
    if (td0->dist != td1->dist) {
        return td0->dist - td1->dist;
    }
    if (td0->tile->row != td1->tile->row) {
        return td0->tile->row - td1->tile->row;
    }
    return td0->tile->col - td1->tile->col;
}

bool tick_tile(World *w, Tile *t) {
    if (t->unit == NULL) {
        return false;
    }
    bool done = false;
    Tile **targets = determine_targets(w, t);
    Tile **to_attack = NULL;
    Tile **to_move = NULL;
    if (arrlen(targets) == 0) {
        done = true;
        goto end;
    }
    forvl(target, targets, arrlen(targets)) {
        Tile **in_range = tiles_in_range(w, target);
        forvl(ir, in_range, arrlen(in_range)) {
            if (ir->row == t->row && ir->col == t->col) {
                arrput(to_attack, target);
            } else if (is_open(ir)) {
                arrput(to_move, ir);
            }
        }
        arrfree(in_range);
    }
    if (arrlen(to_move) == 0 && arrlen(to_attack) == 0) {
        goto end;
    }
    if (arrlen(to_attack) == 0) {
        // Move
        TileDistance *distances = NULL;
        forvl(tm, to_move, arrlen(to_move)) {
            TileDistance td = (TileDistance){
                .tile = tm,
                .dist = compute_distance(w, t, tm),
            };
            if (td.dist >= 0) {
                arrput(distances, td);
            }
        }
        if (arrlen(distances) == 0) {
            goto end;
        }
        qsort(distances, arrlen(distances), sizeof(*distances), compare_tile_distance);
        TileDistance chosen = distances[0];
        assert(chosen.dist > 0);
        TileDistance *paths = NULL;
        Tile **adjacent = tiles_in_range(w, t);
        forvl(pt, adjacent, arrlen(adjacent)) {
            if (is_open(pt)) {
                TileDistance ptd = (TileDistance){
                    .tile = pt,
                    .dist = compute_distance(w, pt, chosen.tile),
                };
                if (ptd.dist >= 0) {
                    arrput(paths, ptd);
                }
            }
        }
        assert(arrlen(paths) > 0);
        qsort(paths, arrlen(paths), sizeof(*paths), compare_tile_distance);
        assert(paths[0].dist >= 0);
        Tile *path = paths[0].tile;
        Unit *u = t->unit;
        t->unit = NULL;
        path->unit = u;
        u->tile = path;
        t = path;
        arrfree(distances);
        arrfree(paths);
        arrfree(adjacent);
    }
    // Attack
    Tile *attack = NULL;
    Tile **in_range = tiles_in_range(w, t);
    forvl(pt, in_range, arrlen(in_range)) {
        if (pt->unit != NULL && pt->unit->type != t->unit->type) {
            if (attack == NULL || pt->unit->health < attack->unit->health) {
                attack = pt;
            }
        }
    }
    arrfree(in_range);
    if (attack == NULL) {
        goto end;
    }
    attack->unit->health -= t->unit->attack;
    // printf("%ld,%ld hits %ld,%ld HP %ld -> %ld\n",
    //        t->row, t->col, attack->row, attack->col,
    //        attack->unit->health + t->unit->attack, attack->unit->health);
    if (attack->unit->health <= 0) {
        attack->unit->tile = NULL;
        attack->unit = NULL;
    }
end:
    arrfree(targets);
    arrfree(to_attack);
    arrfree(to_move);
    return done;
}

bool tick(World *w) {
    Unit **order = NULL;
    fori(r, w->h) {
        fori(c, w->w) {
            Tile *t = &w->tiles[r * w->w + c];
            if (t->unit) {
                arrput(order, t->unit);
            }
        }
    }
    bool done = false;
    forvl(unit, order, arrlen(order)) {
        if (unit->tile) {
            if (tick_tile(w, unit->tile)) {
                done = true;
                goto end;
            }
        }
    }
end:
    arrfree(order);
    return done;
}

long total_hp(World *w) {
    long total = 0;
    forpl(t, w->tiles, w->w * w->h) {
        if (t->unit) {
            total += t->unit->health;
        }
    }
    return total;
}

bool all_elves_alive(World *w) {
    long total_elves = 0;
    long alive_elves = 0;
    forpl(unit, w->units, arrlen(w->units)) {
        if (unit->type == ELF) {
            total_elves++;
        }
    }
    forpl(tile, w->tiles, w->w * w->h) {
        if (tile->unit && tile->unit->type == ELF) {
            alive_elves++;
        }
    }
    return total_elves == alive_elves;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("15.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    World w = build_world(lines, num_lines, 3);
    for (long i = 0;; i++) {
        bool done = tick(&w);
        if (done) {
            long hp = total_hp(&w);
            printf("Part 1: %ld\n", i * hp);
            break;
        }
    }

    for (long elf_attack = 4;; elf_attack++) {
        World w = build_world(lines, num_lines, elf_attack);
        for (long i = 0;; i++) {
            bool done = tick(&w);
            if (done) {
                if (all_elves_alive(&w)) {
                    long hp = total_hp(&w);
                    printf("Part 2: %ld\n", i * hp);
                    return 0;
                }
                goto outer;
            }
        }
    outer:;
    }

    return 0;
}
