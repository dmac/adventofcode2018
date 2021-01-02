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
    int year;
    int month;
    int day;
    int hour;
    int minute;

    int id;
    bool begins;
    bool sleeps;
    bool wakes;
} Entry;

Entry parse_entry(char *line, int last_id) {
    Entry e = {0};
    int scanned = sscanf(line, "[%d-%d-%d %d:%d] Guard #%d",
                         &e.year, &e.month, &e.day, &e.hour, &e.minute, &e.id);
    if (scanned < 5) {
        fatal("didn't scan all elements");
    }

    if (e.id == 0) {
        e.id = last_id;
    } else {
        e.begins = true;
    }

    if (strstr(line, "falls asleep")) {
        e.sleeps = true;
    } else if (strstr(line, "wakes up")) {
        e.wakes = true;
    }
    return e;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("04.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }
    sort_strings(lines, num_lines);

    Entry *entries = NULL;
    int last_id = -1;
    for (int i = 0; i < num_lines; i++) {
        Entry e = parse_entry(lines[i], last_id);
        last_id = e.id;
        arrput(entries, e);
    }

    typedef struct {
        int key;       // guard id
        int value[60]; // number of times asleep on minute
    } Schedule;
    Schedule *schedules = NULL;

    for (int i = 0; i < arrlen(entries); i++) {
        Entry e1 = entries[i];
        if (!e1.wakes) {
            continue;
        }
        for (int j = i; j >= 0; j--) {
            Entry e0 = entries[j];
            if (e0.id == e1.id && e0.sleeps) {
                Schedule sched = hmgets(schedules, e0.id);
                sched.key = e0.id;
                for (int k = e0.minute; k < e1.minute; k++) {
                    sched.value[k]++;
                }
                hmputs(schedules, sched);
                goto outer;
            }
        }
    outer:;
    }

    int max = 0;
    int max_id = 0;
    for (int i = 0; i < hmlen(schedules); i++) {
        Schedule sched = schedules[i];
        int sum = 0;
        for (int j = 0; j < ARRLEN(sched.value); j++) {
            sum += sched.value[j];
        }
        if (sum > max) {
            max = sum;
            max_id = sched.key;
        }
    }

    Schedule sched = hmgets(schedules, max_id);
    max = 0;
    int max_idx = 0;
    for (int i = 0; i < ARRLEN(sched.value); i++) {
        int v = sched.value[i];
        if (v > max) {
            max = v;
            max_idx = i;
        }
    }
    printf("Part 1: %d\n", max_id * max_idx);

    max = 0;
    max_id = 0;
    max_idx = 0;

    for (int i = 0; i < hmlen(schedules); i++) {
        Schedule sched = schedules[i];
        for (int j = 0; j < ARRLEN(sched.value); j++) {
            int v = sched.value[j];
            if (v > max) {
                max = v;
                max_id = sched.key;
                max_idx = j;
            }
        }
    }
    printf("Part 2: %d\n", max_id * max_idx);

    return 0;
}
