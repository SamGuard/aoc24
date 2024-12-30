#include <assert.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    Up = 0,
    Right = 1,
    Down = 2,
    Left = 3,
} direct_t;

typedef enum {
    Open,
    Closed,
    Visit,
} node_state_t;

typedef struct {
    uint64_t dist;
    node_state_t s;
    int prev;
} node_t;

int print = 0;

direct_t rot(direct_t d, const int clockwise) {
    if (!clockwise) {
        goto anticlockwise;
    }
    switch (d) {
        case Up:
            return Right;
        case Down:
            return Left;
        case Left:
            return Up;
        case Right:
            return Down;
        default:
            printf("Bad\n");
            exit(-1);
    }
anticlockwise:
    switch (d) {
        case Up:
            return Left;
        case Down:
            return Right;
        case Left:
            return Down;
        case Right:
            return Up;
        default:
            printf("Bad\n");
            exit(-1);
    }
}

int move(int *pos, direct_t d, const int width, const int len) {
    switch (d) {
        case Up: {
            if (*pos - width < 0) {
                goto bad_move;
            }
            *pos -= width;
            break;
        }
        case Down: {
            if (*pos + width >= len) {
                goto bad_move;
            }
            *pos += width;
            break;
        }
        case Left: {
            if (*pos % width == 0) {
                goto bad_move;
            }
            *pos -= 1;
            break;
        }
        case Right: {
            if ((*pos + 1) % width == 0) {
                goto bad_move;
            }
            *pos += 1;
            break;
        }
        default:
            printf("Bad\n");
            exit(-1);
    }
    return 1;
bad_move:
    return 0;
}

// Needed out here for sorting
node_t *nodes;
static int node_sort(const void *p1, const void *p2) {
    const int n1 = *(int *)p1, n2 = *(int *)p2;
    const uint64_t v1 = nodes[n1].dist, v2 = nodes[n2].dist;
    return -1 * ((v1 > v2) - (v1 < v2));
}

uint64_t travel(const char *const grid, const int width, const int len, const int s_pos,
                const int e_pos) {
    int visit_list[256];
    int v_list_len;
    int found = 0;
    nodes = (node_t *)malloc(sizeof(node_t) * len);
    for (int i = 0; i < len; i++) {
        nodes[i].dist = (uint64_t)-1;
        nodes[i].s = Open;
        nodes[i].prev = -1;
    }

    nodes[s_pos].dist = 0;
    visit_list[0] = s_pos;
    v_list_len = 1;

    while (v_list_len > 0) {
        int c_pos = visit_list[--v_list_len];
        node_t *c_node = nodes + c_pos;

        c_node->s = Closed;
        // printf("c_pos=(%d, %d) -> %c\n", c_pos % width, c_pos / width,
        // grid[c_pos]);
        if (grid[c_pos] == '#') {
            continue;
        }

        if (c_pos == e_pos) {
            found = 1;
            continue;
        }

        assert(grid[c_pos] == '.');

        // Left Forward Right
        direct_t c_dir = Up;
        for (int i = 0; i <= 3; i++) {
            int n_pos = c_pos;
            if (move(&n_pos, c_dir, width, len) && nodes[n_pos].s != Closed) {
                node_t *n_node = nodes + n_pos;
                if (c_node->dist + 1 < n_node->dist) {
                    n_node->dist = c_node->dist + 1;
                    n_node->prev = c_pos;
                }
                if (n_node->s == Open) {
                    n_node->s = Visit;
                    visit_list[v_list_len++] = n_pos;
                }
            }
            c_dir = rot(c_dir, 1);
        }

        qsort(visit_list, v_list_len, sizeof(int), node_sort);
        if (v_list_len >= 255) {
            printf("Too smol lol\n");
            exit(-1);
        }
    }
    if (!found) {
        printf("Didnt find the end\n");
        return -1;
    }
    // Part 1
    if (print) {
        char *occ = (char *)calloc(len, sizeof(char));
        int c_pos = e_pos;
        occ[s_pos] = 'O';
        while (c_pos != s_pos) {
            occ[c_pos] = 'O';
            c_pos = nodes[c_pos].prev;
        }
        for (int i = 0; i < len; i++) {
            char c = grid[i];
            if (occ[i]) {
                c = occ[i];
            }
            printf("%c", c);
            if ((i + 1) % width == 0) {
                printf("\n");
            }
        }
        free(occ);
    }

    uint64_t res = nodes[e_pos].dist;
    free(nodes);
    return res;
}

uint64_t part1(char *const grid, const int width, const int len, int *fall_bytes, int num_f_bytes,
               const int s_pos, const int e_pos, const int cap) {
    print = 1;
    for (int i = 0; i < len; i++) {
        grid[i] = '.';
    }
    for (int i = 0; i < cap && i < num_f_bytes; i++) {
        int x = fall_bytes[2 * i], y = fall_bytes[2 * i + 1];
        grid[x + y * width] = '#';
    }

    for (int i = 0; i < len; i++) {
        printf("%c", grid[i]);
        if ((i + 1) % width == 0) {
            printf("\n");
        }
    }
    return travel(grid, width, len, s_pos, e_pos);
}
char *part2(char *const grid, const int width, const int len, int *fall_bytes, int num_f_bytes,
            const int s_pos, const int e_pos) {
    print = 0;
    char *out = (char *)malloc(sizeof(char) * 16);
    int best = num_f_bytes;
    for (int i = 0; i < len; i++) {
        grid[i] = '.';
    }
    printf("Simulating one mo...\n");
    for (int i = 0; i < num_f_bytes; i++) {
        int x = fall_bytes[2 * i], y = fall_bytes[2 * i + 1];
        grid[x + y * width] = '#';
        uint64_t res = travel(grid, width, len, s_pos, e_pos);
        if (res == (uint64_t)-1) {
            best = i;
            break;
        }
    }
    sprintf(out, "%d,%d", fall_bytes[2 * best], fall_bytes[2 * best + 1]);
    return out;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("input pls\n");
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Oopsie daisies");
        return -1;
    }

    char *grid = (char *)malloc(sizeof(char) * (1 << 15));
    int *falling_bytes = (int *)malloc(sizeof(int) * (1 << 15));
    int len = 0, width = -1, start_pos = 0, end_pos = -1, num_falling_bytes = 0, part1_cap = -1;
    char buff[256];
    // Read width
    fgets(buff, sizeof(buff), f);
    width = atoi(buff);
    // Read number of rows to read
    fgets(buff, sizeof(buff), f);
    part1_cap = atoi(buff);

    len = width * width;
    // Start and finish points
    start_pos = 0;
    end_pos = len - 1;
    for (int i = 0; i < len; i++) {
        grid[i] = '.';
    }
    while (fgets(buff, sizeof(buff), f)) {
        char *token = strtok(buff, ",");
        int x, y;
        x = atoi(token);
        y = atoi(strtok(NULL, ","));
        falling_bytes[2 * num_falling_bytes] = x;
        falling_bytes[2 * num_falling_bytes + 1] = y;
        num_falling_bytes++;
    }

    uint64_t part1_total = 0;
    char *part2_ans = NULL;

    part1_total =
        part1(grid, width, len, falling_bytes, num_falling_bytes, start_pos, end_pos, part1_cap);
    part2_ans = part2(grid, width, len, falling_bytes, num_falling_bytes, start_pos, end_pos);

    printf("Part 1 total: %lu\n", part1_total);
    printf("Part 2 Answer: %s\n", part2_ans);

    free(grid);
    free(part2_ans);
    fclose(f);
    return 0;
}
