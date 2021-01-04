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

typedef struct Node Node;
struct Node {
    int num_nodes;
    int num_meta;
    Node **nodes;
    int *meta;
};

Node *parse_node(int vals[], int *idx) {
    int num_nodes = vals[(*idx)++];
    int num_meta = vals[(*idx)++];
    Node *n = calloc(1, sizeof(*n));
    n->num_nodes = num_nodes;
    n->num_meta = num_meta;
    fori(i, num_nodes) {
        arrput(n->nodes, parse_node(vals, idx));
    }
    fori(i, num_meta) {
        arrput(n->meta, vals[(*idx)++]);
    }
    return n;
}

Node *parse(char *s) {
    char *next = s;
    int *vals = NULL;
    for (; *next != '\0';) {
        long n = strtol(s, &next, 10);
        arrput(vals, n);
        s = next;
    }
    int idx = 0;
    Node *root = parse_node(vals, &idx);
    arrfree(vals);
    return root;
}

int sum_meta(Node *n) {
    int sum = 0;
    forvl(v, n->meta, n->num_meta) {
        sum += v;
    }
    forvl(m, n->nodes, n->num_nodes) {
        sum += sum_meta(m);
    }
    return sum;
}

int value_of(Node *n) {
    int sum = 0;
    if (n->num_nodes == 0) {
        forvl(v, n->meta, n->num_meta) {
            sum += v;
        }
        return sum;
    }
    forvl(v, n->meta, n->num_meta) {
        v--;
        if (v >= 0 && v < n->num_nodes) {
            Node *m = n->nodes[v];
            sum += value_of(m);
        }
    }
    return sum;
}

int main(void) {
    char **lines = NULL;
    int64_t num_lines = 0;
    error err = file_read_lines("08.input.txt", &lines, &num_lines);
    if (err != NULL) {
        fatal("%s\n", err);
    }

    Node *root = parse(lines[0]);
    int meta = sum_meta(root);
    printf("Part 1: %d\n", meta);

    int value = value_of(root);
    printf("Part 2: %d\n", value);

    return 0;
}
