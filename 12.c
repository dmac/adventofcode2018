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
    int key;
    bool value;
} Garden;

typedef struct {
    bool b[5];
} bool5;

typedef struct {
    bool5 key;
    bool value;
} Rule;

typedef struct {
    Garden *garden;
    Garden *scratch;
    Rule *rules;
} Sim;

void fill_region(Garden *garden, int idx, bool5 *region) {
    fori(i, 5) {
        region->b[i] = hmget(garden, idx - 2 + i);
    }
}

void step(Sim *sim) {
    sim->scratch = NULL;
    bool5 region;
    memzero(&region, sizeof(region));
    int min = INT_MAX;
    int max = 0;
    forpl(g, sim->garden, hmlen(sim->garden)) {
        if (g->value && g->key < min) {
            min = g->key;
        }
        if (g->value && g->key > max) {
            max = g->key;
        }
        fill_region(sim->garden, g->key, &region);
        hmput(sim->scratch, g->key, hmget(sim->rules, region));
    }
    for (int i = min - 2; i < min; i++) {
        fill_region(sim->garden, i, &region);
        hmput(sim->scratch, i, hmget(sim->rules, region));
    }
    for (int i = max + 1; i <= max + 2; i++) {
        fill_region(sim->garden, i, &region);
        hmput(sim->scratch, i, hmget(sim->rules, region));
    }
    hmfree(sim->garden);
    sim->garden = sim->scratch;
    sim->scratch = NULL;
}

int garden_sum(Garden *garden) {
    int sum = 0;
    forpl(g, garden, hmlen(garden)) {
        if (g->value) {
            sum += g->key;
        }
    }
    return sum;
}

int garden_min(Garden *garden) {
    int min = INT_MAX;
    forpl(g, garden, hmlen(garden)) {
        if (g->value && g->key < min) {
            min = g->key;
        }
    }
    return min;
}

char *garden_string(Garden *garden) {
    assert(hmlen(garden) > 0);
    int min = INT_MAX;
    int max = 0;
    forpl(g, garden, hmlen(garden)) {
        if (g->value && g->key < min) {
            min = g->key;
        }
        if (g->value && g->key > max) {
            max = g->key;
        }
    }
    int shift = -min;
    int size = max + shift + 1;
    assert(size > 0);
    char *buf = malloc(size + 1);
    fori(i, size) {
        if (hmget(garden, i - shift)) {
            buf[i] = '#';
        } else {
            buf[i] = '.';
        }
    }
    buf[size] = '\0';
    return buf;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("12.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    Garden *garden1 = NULL;
    Garden *garden2 = NULL;
    int i = 0;
    for (char *p = strchr(lines[0], ':') + 2; *p != '\0'; p++) {
        hmput(garden1, i, *p == '#');
        hmput(garden2, i, *p == '#');
        i++;
    }

    Rule *rules = NULL;
    for (int i = 2; i < num_lines; i++) {
        Rule rule;
        memzero(&rule, sizeof(rule));
        fori(j, 5) {
            rule.key.b[j] = lines[i][j] == '#';
        }
        char *p = strchr(lines[i], '>') + 2;
        rule.value = *p == '#';
        hmputs(rules, rule);
    }

    Sim sim = {
        .garden = garden1,
        .rules = rules,
    };

    fori(i, 20) {
        step(&sim);
    }
    printf("Part 1: %d\n", garden_sum(sim.garden));

    sim.garden = garden2;
    char *prev = garden_string(sim.garden);
    int prev_sum = 0;
    for (int i = 1; i <= 1000; i++) {
        step(&sim);
        char *curr = garden_string(sim.garden);
        int sum = garden_sum(sim.garden);
        if (strcmp(prev, curr) == 0) {
            printf("Part 2: %ld\n", sum + (50000000000L - i) * (sum - prev_sum));
            return 0;
        }
        free(prev);
        prev = curr;
        prev_sum = sum;
    }

    return 0;
}
