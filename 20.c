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

int print = 1;

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
                const int e_pos, const int portal_in, const int portal_out) {
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
        if (grid[c_pos] == '#') {
            continue;
        }

        if (grid[c_pos] == 'E') {
            found = 1;
            continue;
        }

        assert(grid[c_pos] == '.' || grid[c_pos] == 'S');

        direct_t n_dir = Up;
        do {
            int n_pos = c_pos;
            if (move(&n_pos, n_dir, width, len) && nodes[n_pos].s != Closed) {
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
            n_dir = rot(n_dir, 1);
        } while (n_dir != Up);

        if (c_pos == portal_in && nodes[portal_out].s != Closed) {
            node_t *n_node = nodes + portal_out;
            int d;
            {
                // Manhattan distance
                int dx, dy;
                dx = abs((portal_out % width) - (portal_in % width));
                dy = abs((portal_out / width) - (portal_in / width));
                d = dx + dy;
            }
            if (c_node->dist + d < n_node->dist) {
                n_node->dist = c_node->dist + d;
                n_node->prev = c_pos;
            }
            if (n_node->s == Open) {
                n_node->s = Visit;
                visit_list[v_list_len++] = portal_out;
            }
        }

        qsort(visit_list, v_list_len, sizeof(int), node_sort);
        if (v_list_len >= 255) {
            printf("Too smol lol\n");
            exit(-1);
        }
    }
    if (!found) {
        return -1;
    }

    if (print) {
        char *occ = (char *)calloc(len, sizeof(char));
        int c_pos = e_pos;
        while (c_pos != s_pos) {
            occ[c_pos] = 'X';
            c_pos = nodes[c_pos].prev;
            printf("%d ", c_pos);
        }
        printf("\n");
        for (int i = 0; i < len; i++) {
            char c = grid[i];
            if (occ[i] == 'X') {
                c = 'X';
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

uint64_t cheat(const char *const grid, const int width, const int len, const int s_pos,
               const int e_pos, const int c_t) {
    uint64_t total = 0, base = travel(grid, width, len, s_pos, e_pos, 0, 1);
    printf("Base distance -> %lu\n", base);
    printf("Finding all other paths...\n");

    print = 0;
    for (int i = 0; i < len; i++) {
        if (grid[i] != '.' && grid[i] != 'S') {
            continue;
        }

        if (i % 1000 == 0) {
            printf("%d%%\n", (100 * i) / len);
        }

        int portal_in = i, portal_out;
        int pix = portal_in % width, piy = portal_in / width;
        // pix -> portal in x  | piy -> portal in y
        // pox -> portal out y | poy -> portal out y
        for (int pox = pix - c_t; pox <= pix + c_t; pox++) {
            for (int poy = piy - c_t; poy <= piy + c_t; poy++) {
                // Portal to yourself doesnt make sense
                if (pix == pox && piy == poy) {
                    continue;
                }
                if (pox < 0 || poy < 0 || pox >= width || poy >= len / width) {
                    continue;
                }
                {
                    int d = abs(pix - pox) + abs(piy - poy);
                    if (d > c_t || d <= 1) {
                        continue;
                    }
                }
                portal_out = pox + width * poy;
                if (portal_out < 0 || portal_out >= len) {
                    continue;
                }
                if (grid[portal_out] != '.' && grid[portal_out] != 'E') {
                    continue;
                }

                uint64_t res = travel(grid, width, len, s_pos, e_pos, portal_in, portal_out);
                if (res < base) {
                    // len < 1000 is to switch logic between example and actual
                    if (len < 1000) {
                        printf("in=(%d,%d) out=(%d, %d) | Cheat found, saving %lu\n", pix, piy, pox,
                               poy, base - res);
                    }
                    if (base - res >= 100 || (len < 1000 && (base - res >= 50 || c_t == 2))) {
                        total += 1;
                    }
                }
            }
        }
    }

    return total;
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
    int len = 0, width = -1, start_pos = -1, end_pos = -1;

    {
        char buff[256];
        char *ptr = grid;
        while (fgets(buff, sizeof(buff), f)) {
            // Assume a full line is always read in
            if (width == -1) {
                width = strlen(buff) - 1;
            }
            memcpy(ptr, buff, width);
            ptr += width;
            len += width;
        }
    }

    for (int i = 0; i < len; i++) {
        if (grid[i] == 'S') {
            start_pos = i;
            continue;
        }
        if (grid[i] == 'E') {
            end_pos = i;
        }
    }

    if (start_pos == -1) {
        exit(-1);
    }

    uint64_t part1_total = 0, part2_total = 0;
    print = 1;
    part1_total = cheat(grid, width, len, start_pos, end_pos, 2);
    part2_total = cheat(grid, width, len, start_pos, end_pos, 20);

    printf("Part 1 total: %lu\n", part1_total);
    printf("Part 2 total: %lu\n", part2_total);

    free(grid);
    fclose(f);
    return 0;
}