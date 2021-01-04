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

typedef struct Marble Marble;
struct Marble {
    int v;
    Marble *prev;
    Marble *next;
};

typedef struct {
    int nplayers;
    int nmarbles;
    long *players;
    Marble *marbles;
} Game;

long max_score(Game *game) {
    long max = 0;
    forvl(score, game->players, game->nplayers) {
        if (score > max) {
            max = score;
        }
    }
    return max;
}

void simulate(Game *game) {
    Marble *curr = &game->marbles[0];
    curr->prev = curr;
    curr->next = curr;
    int player = 0;
    fori(i, game->nmarbles) {
        Marble *m = &game->marbles[i];
        m->v = i;
        if (m->v == 0) {
            continue;
        }

        if (m->v % 23 != 0) {
            m->next = curr->next->next;
            m->prev = curr->next;
            m->next->prev = m;
            m->prev->next = m;
            curr = m;
            continue;
        }

        game->players[player] += m->v;
        fori(_, 7) {
            curr = curr->prev;
        }
        game->players[player] += curr->v;
        curr->prev->next = curr->next;
        curr->next->prev = curr->prev;
        curr = curr->next;

        player = (player + 1) % game->nplayers;
    }
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("09.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    Game *game = calloc(1, sizeof(*game));
    if (sscanf(lines[0], "%d players; last marble is worth %d points", &game->nplayers, &game->nmarbles) != 2) {
        fatal("bad input\n");
    }
    game->nmarbles++;
    game->players = calloc(game->nplayers, sizeof(*game->players));
    game->marbles = calloc(game->nmarbles, sizeof(*game->marbles));
    simulate(game);
    long score = max_score(game);
    printf("Part 1: %ld\n", score);

    forpl(p, game->players, game->nplayers) {
        *p = 0;
    }
    game->nmarbles *= 100;
    game->marbles = realloc(game->marbles, game->nmarbles * sizeof(*game->marbles));
    simulate(game);
    score = max_score(game);
    printf("Part 2: %ld\n", score);

    return 0;
}
