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
    int before[4];
    int prog[4];
    int after[4];
} Sample;

Sample *parse_samples(char **lines, int64_t num_lines, int64_t *line_num) {
    Sample *samples = NULL;
    char *prev = NULL;
    Sample s = {0};
    fori(ln, num_lines) {
        char *line = lines[ln];
        if (line[0] == 'B') {
            sscanf(line, "Before: [%d, %d, %d, %d]",
                   &s.before[0], &s.before[1], &s.before[2], &s.before[3]);
        } else if (line[0] == 'A') {
            sscanf(line, "After: [%d, %d, %d, %d]",
                   &s.after[0], &s.after[1], &s.after[2], &s.after[3]);
        } else if (isdigit(line[0])) {
            sscanf(line, "%d %d %d %d",
                   &s.prog[0], &s.prog[1], &s.prog[2], &s.prog[3]);
        } else {
            assert(strlen(line) == 0);
            if (prev != NULL && strlen(prev) == 0) {
                *line_num = ln;
                return samples;
            }
            arrput(samples, s);
            memzero(&s, sizeof(s));
        }
        prev = line;
    }
    arrput(samples, s);
    return samples;
}

int *parse_prog(char **lines, int64_t num_lines, int64_t prog_line) {
    int *prog = NULL;
    for (int64_t i = prog_line; i < num_lines; i++) {
        char *line = lines[i];
        if (strlen(line) == 0) {
            continue;
        }
        for (char *next = NULL; next == NULL || *next != '\0';) {
            int n = (int)strtol(line, &next, 10);
            arrput(prog, n);
            line = next;
        }
    }
    return prog;
}

typedef void (*instruction)(int r[4], int a, int b, int c);

void addr(int r[4], int a, int b, int c) {
    r[c] = r[a] + r[b];
}

void addi(int r[4], int a, int b, int c) {
    r[c] = r[a] + b;
}

void mulr(int r[4], int a, int b, int c) {
    r[c] = r[a] * r[b];
}

void muli(int r[4], int a, int b, int c) {
    r[c] = r[a] * b;
}

void banr(int r[4], int a, int b, int c) {
    r[c] = r[a] & r[b];
}

void bani(int r[4], int a, int b, int c) {
    r[c] = r[a] & b;
}

void borr(int r[4], int a, int b, int c) {
    r[c] = r[a] | r[b];
}

void bori(int r[4], int a, int b, int c) {
    r[c] = r[a] | b;
}

void setr(int r[4], int a, int b, int c) {
    r[c] = r[a];
}

void seti(int r[4], int a, int b, int c) {
    r[c] = a;
}

void gtir(int r[4], int a, int b, int c) {
    r[c] = a > r[b] ? 1 : 0;
}

void gtri(int r[4], int a, int b, int c) {
    r[c] = r[a] > b ? 1 : 0;
}

void gtrr(int r[4], int a, int b, int c) {
    r[c] = r[a] > r[b] ? 1 : 0;
}

void eqir(int r[4], int a, int b, int c) {
    r[c] = a == r[b] ? 1 : 0;
}

void eqri(int r[4], int a, int b, int c) {
    r[c] = r[a] == b ? 1 : 0;
}

void eqrr(int r[4], int a, int b, int c) {
    r[c] = r[a] == r[b] ? 1 : 0;
}

instruction instructions[] = {
    addr,
    addi,
    mulr,
    muli,
    banr,
    bani,
    borr,
    bori,
    setr,
    seti,
    gtir,
    gtri,
    gtrr,
    eqir,
    eqri,
    eqrr,
};

long check_opcodes(Sample *s) {
    long sum = 0;
    int r[4] = {0};
    forv(inst, instructions) {
        fori(i, 4) {
            r[i] = s->before[i];
        }
        inst(r, s->prog[1], s->prog[2], s->prog[3]);
        bool match = true;
        fori(i, 4) {
            if (r[i] != s->after[i]) {
                match = false;
            }
        }
        if (match) {
            sum++;
        }
    }
    return sum;
}

void determine_opcodes(Sample *samples, int *opcodes) {
    int stride = ARRLEN(instructions);
    bool *possible = calloc(stride * stride, sizeof(*possible));
    int r[4] = {0};
    forpl(s, samples, arrlen(samples)) {
        fori(idx, ARRLEN(instructions)) {
            instruction inst = instructions[idx];
            fori(i, 4) {
                r[i] = s->before[i];
            }
            inst(r, s->prog[1], s->prog[2], s->prog[3]);
            bool match = true;
            fori(i, 4) {
                if (r[i] != s->after[i]) {
                    match = false;
                }
            }
            if (match) {
                possible[s->prog[0] * stride + idx] = true;
            }
        }
    }
    for (int found = 0; found < ARRLEN(instructions); found++) {
        fori(opcode, ARRLEN(instructions)) {
            int n = 0;
            int ans = 0;
            fori(idx, ARRLEN(instructions)) {
                if (possible[opcode * stride + idx]) {
                    n++;
                    ans = idx;
                }
            }
            if (n == 1) {
                opcodes[opcode] = ans;
                fori(op, ARRLEN(instructions)) {
                    possible[op * stride + ans] = false;
                }
                goto outer;
            }
        }
        fatal("no answer found\n");
    outer:;
    }
    free(possible);
}

int run_prog(int *prog, int *opcodes) {
    int r[4];
    for (int i = 0; i < arrlen(prog); i += 4) {
        instruction inst = instructions[opcodes[prog[i]]];
        inst(r, prog[i + 1], prog[i + 2], prog[i + 3]);
    }
    return r[0];
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("16.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    int64_t prog_line = 0;
    Sample *samples = parse_samples(lines, num_lines, &prog_line);
    int *prog = parse_prog(lines, num_lines, prog_line);
    long part1 = 0;
    forpl(sample, samples, arrlen(samples)) {
        long n = check_opcodes(sample);
        if (n >= 3) {
            part1++;
        }
    }
    printf("Part 1: %ld\n", part1);

    int opcodes[ARRLEN(instructions)] = {0};
    determine_opcodes(samples, opcodes);
    int part2 = run_prog(prog, opcodes);
    printf("Part 2: %d\n", part2);

    return 0;
}
