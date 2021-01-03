#if 0
set -e
src="$(basename $0)"
exe="$(echo $src | cut -f1 -d.)"
trap "rm -f $exe" EXIT
gcc -std=gnu99 -Wall -Werror -pedantic -Wno-unused-value -O2 -g -o $exe $src
./$exe
exit $?
#endif

#include "base.h"

// #define NWORKERS 2
// #define TIMEPAD 0
#define NWORKERS 5
#define TIMEPAD 60

typedef struct {
    char name;
    char *deps;
    int time;
    bool claimed;
} Step;

typedef struct {
    char step;
    char dep;
} Req;

typedef struct {
    char key;
    Step value;
} StepKV;

int steptime(char name) {
    return name - 'A' + 1 + TIMEPAD;
}

Req parse_req(char *line) {
    Req r;
    int n = sscanf(line, "Step %c must be finished before step %c can begin.", &r.dep, &r.step);
    if (n != 2) {
        fatal("parse error\n");
    }
    return r;
}

StepKV *parse_steps(char **lines, int64_t num_lines) {
    StepKV *steps = NULL;
    forvl(line, lines, num_lines) {
        Req r = parse_req(line);
        int idx = hmgeti(steps, r.step);
        if (idx == -1) {
            Step s = (Step){
                .name = r.step,
                .time = steptime(r.step),
            };
            hmput(steps, r.step, s);
            idx = hmgeti(steps, r.step);
        }
        arrput(steps[idx].value.deps, r.dep);

        idx = hmgeti(steps, r.dep);
        if (idx == -1) {
            Step s = (Step){
                .name = r.dep,
                .time = steptime(r.dep),
            };
            hmput(steps, r.dep, s);
        }
    }
    return steps;
}

void print_step(Step *s) {
    printf("%c requires [", s->name);
    fori(i, arrlen(s->deps)) {
        if (i > 0) {
            printf(" ");
        }
        printf("%c", s->deps[i]);
    }
    printf("]\n");
}

char *do_part_1(StepKV *steps) {
    char *part1 = NULL;
loop:;
    char *ready = NULL;
    forvl(kv, steps, hmlen(steps)) {
        if (arrlen(kv.value.deps) == 0) {
            arrput(ready, kv.key);
        }
    }
    if (arrlen(ready) > 0) {
        sort_chars(ready, arrlen(ready));
        char rdy = ready[0];
        arrput(part1, rdy);
        hmdel(steps, rdy);
        forvl(kv, steps, hmlen(steps)) {
            char *deps = kv.value.deps;
            fori(i, arrlen(deps)) {
                if (deps[i] == rdy) {
                    arrdel(deps, i);
                }
            }
        }
        goto loop;
    }
    arrput(part1, '\0');
    return part1;
}

int compare_steps(const void *v0, const void *v1) {
    Step *s0 = *(Step **)v0;
    Step *s1 = *(Step **)v1;
    return (s0->name - s1->name) - (s1->name - s0->name);
}

Step **get_ready_steps(StepKV *steps) {
    Step **ready = NULL;
    forpl(kv, steps, hmlen(steps)) {
        Step *step = &kv->value;
        if (arrlen(step->deps) == 0 && !step->claimed) {
            arrput(ready, step);
        }
    }
    qsort(ready, arrlen(ready), sizeof(*ready), compare_steps);
    return ready;
}

void assign_idle_workers(Step *workers[NWORKERS], Step **ready) {
    forvl(step, ready, arrlen(ready)) {
        fori(i, NWORKERS) {
            if (workers[i] == NULL) {
                workers[i] = step;
                step->claimed = true;
                goto outer;
            }
        }
    outer:;
    }
}

void mark_step_complete(StepKV *steps, Step *step) {
    forvl(kv, steps, hmlen(steps)) {
        char *deps = kv.value.deps;
        fori(i, arrlen(deps)) {
            if (deps[i] == step->name) {
                arrdel(deps, i);
            }
        }
    }
}

int do_part_2(StepKV *steps) {
    Step *workers[NWORKERS] = {0};
    for (int second = 0;; second++) {
        Step **ready = get_ready_steps(steps);
        bool done = true;
        forvl(kv, steps, hmlen(steps)) {
            if (kv.value.time > 0) {
                done = false;
            }
        }
        if (done) {
            return second;
        }
        assign_idle_workers(workers, ready);
        fori(i, NWORKERS) {
            Step *step = workers[i];
            if (step == NULL) {
                continue;
            }
            step->time--;
            if (step->time == 0) {
                mark_step_complete(steps, step);
                workers[i] = NULL;
            }
        }
    }
    return 0;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("07.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    StepKV *steps = parse_steps(lines, num_lines);
    char *part1 = do_part_1(steps);
    printf("Part 1: %s\n", part1);

    steps = parse_steps(lines, num_lines);
    int part2 = do_part_2(steps);
    printf("Part 2: %d\n", part2);

    return 0;
}
