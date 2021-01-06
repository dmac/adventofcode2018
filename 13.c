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

typedef enum {
    NONE = ' ',
    NORTH = '^',
    EAST = '>',
    SOUTH = 'v',
    WEST = '<',
} Face;

typedef enum {
    LEFT,
    STRAIGHT,
    RIGHT
} Turn;

typedef struct {
    Face face;
    Turn turn;
} Cart;

typedef struct {
    char track;
    Cart *cart;
    Cart *next;
} Track;

typedef struct {
    Track *tracks;
    Cart *carts;
    int rows;
    int cols;
} Railway;

void draw_railway(Railway *rw) {
    fori(r, rw->rows) {
        fori(c, rw->cols) {
            Track t = rw->tracks[r * rw->cols + c];
            if (t.cart != NULL) {
                printf("%c", t.cart->face);
            } else {
                printf("%c", t.track);
            }
        }
        printf("\n");
    }
}

bool tick(Railway *rw, vec2i *answer, bool allow_collision) {
    fori(r, rw->rows) {
        fori(c, rw->cols) {
            Track *curr = &rw->tracks[r * rw->cols + c];
            int nr = r;
            int nc = c;
            if (curr->cart == NULL) {
                continue;
            }
            switch (curr->cart->face) {
            case NORTH:
                nr--;
                break;
            case SOUTH:
                nr++;
                break;
            case WEST:
                nc--;
                break;
            case EAST:
                nc++;
                break;
            default:
                assert(false);
            }
            Track *next = &rw->tracks[nr * rw->cols + nc];
            if (next->cart != NULL || next->next != NULL) {
                if (allow_collision) {
                    answer->x = nc;
                    answer->y = nr;
                    return true;
                } else {
                    next->cart = NULL;
                    next->next = NULL;
                    continue;
                }
            }
            next->next = curr->cart;
            switch (next->track) {
            case '-':
                break;
            case '|':
                break;
            case '/':
                switch (curr->cart->face) {
                case NORTH:
                    curr->cart->face = EAST;
                    break;
                case SOUTH:
                    curr->cart->face = WEST;
                    break;
                case WEST:
                    curr->cart->face = SOUTH;
                    break;
                case EAST:
                    curr->cart->face = NORTH;
                    break;
                default:
                    assert(false);
                }
                break;
            case '\\':
                switch (curr->cart->face) {
                case NORTH:
                    curr->cart->face = WEST;
                    break;
                case SOUTH:
                    curr->cart->face = EAST;
                    break;
                case WEST:
                    curr->cart->face = NORTH;
                    break;
                case EAST:
                    curr->cart->face = SOUTH;
                    break;
                default:
                    assert(false);
                }
                break;
            case '+':
                switch (curr->cart->turn) {
                case LEFT:
                    switch (curr->cart->face) {
                    case NORTH:
                        curr->cart->face = WEST;
                        break;
                    case SOUTH:
                        curr->cart->face = EAST;
                        break;
                    case WEST:
                        curr->cart->face = SOUTH;
                        break;
                    case EAST:
                        curr->cart->face = NORTH;
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case RIGHT:
                    switch (curr->cart->face) {
                    case NORTH:
                        curr->cart->face = EAST;
                        break;
                    case SOUTH:
                        curr->cart->face = WEST;
                        break;
                    case WEST:
                        curr->cart->face = NORTH;
                        break;
                    case EAST:
                        curr->cart->face = SOUTH;
                        break;
                    default:
                        assert(false);
                    }
                    break;
                case STRAIGHT:
                    break;
                default:
                    assert(false);
                }
                curr->cart->turn = (curr->cart->turn + 1) % 3;
                break;
            case ' ':
                break;
            default:
                assert(false);
            }
        }
    }
    int carts = 0;
    fori(r, rw->rows) {
        fori(c, rw->cols) {
            Track *t = &rw->tracks[r * rw->cols + c];
            t->cart = t->next;
            t->next = NULL;
            if (t->cart != NULL) {
                answer->x = c;
                answer->y = r;
                carts++;
            }
        }
    }
    return !allow_collision && carts == 1;
}

Railway make_railway(char **lines, int64_t num_lines) {
    Railway rw = (Railway){.rows = num_lines};
    forvl(line, lines, num_lines) {
        int len = strlen(line);
        if (len > rw.cols) {
            rw.cols = len;
        }
    }

    rw.tracks = calloc(rw.rows * rw.cols, sizeof(*rw.tracks));
    fori(r, rw.rows) {
        fori(c, rw.cols) {
            Track t = {0};
            char *line = lines[r];
            int len = strlen(line);
            if (c >= len) {
                t.track = ' ';
            } else {
                Cart cart = {.face = NONE};
                switch (line[c]) {
                case NORTH:
                    cart.face = NORTH;
                    cart.turn = LEFT;
                    t.track = '|';
                    break;
                case SOUTH:
                    cart.face = SOUTH;
                    cart.turn = LEFT;
                    t.track = '|';
                    break;
                case WEST:
                    cart.face = WEST;
                    cart.turn = LEFT;
                    t.track = '-';
                    break;
                case EAST:
                    cart.face = EAST;
                    cart.turn = LEFT;
                    t.track = '-';
                    break;
                default:
                    t.track = line[c];
                    break;
                }
                if (cart.face != NONE) {
                    arrput(rw.carts, cart);
                    t.cart = &rw.carts[arrlen(rw.carts) - 1];
                }
            }
            rw.tracks[r * rw.cols + c] = t;
        }
    }
    return rw;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("13.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    Railway rw = make_railway(lines, num_lines);
    for (;;) {
        vec2i collision = {0};
        bool done = tick(&rw, &collision, true);
        if (done) {
            printf("Part 1: %ld,%ld\n", collision.x, collision.y);
            break;
        }
    }

    rw = make_railway(lines, num_lines);
    for (;;) {
        vec2i final = {0};
        bool done = tick(&rw, &final, false);
        if (done) {
            printf("Part 2: %ld,%ld\n", final.x, final.y);
            break;
        }
    }

    return 0;
}
