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
    direct_t dir;
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
uint64_t travel(const char *const grid, const int width, const int len,
                const int s_pos, const int e_pos, const direct_t s_dir) {
    printf("spos = %d; epos = %d\n", s_pos, e_pos);
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
    nodes[s_pos].dir = s_dir;
    visit_list[0] = s_pos;
    v_list_len = 1;

    while (v_list_len > 0) {
        int c_pos = visit_list[--v_list_len];
        node_t *c_node = nodes + c_pos;
        direct_t c_dir = c_node->dir;

        c_node->s = Closed;
        // printf("c_pos=(%d, %d) -> %c\n", c_pos % width, c_pos / width,
        // grid[c_pos]);
        if (grid[c_pos] == '#') {
            continue;
        }

        if (grid[c_pos] == 'E') {
            found = 1;
            continue;
        }

        assert(grid[c_pos] == '.' || grid[c_pos] == 'S');

        {
            int n_pos = c_pos;
            // Forward
            if (move(&n_pos, c_dir, width, len) && nodes[n_pos].s != Closed) {
                node_t *n_node = nodes + n_pos;
                if (c_node->dist + 1 < n_node->dist) {
                    n_node->dist = c_node->dist + 1;
                    n_node->dir = c_dir;
                    n_node->prev = c_pos;
                }
                if (n_node->s == Open) {
                    n_node->s = Visit;
                    visit_list[v_list_len++] = n_pos;
                }
            }
        }
        // Left Right
        for (int i = 0; i <= 1; i++) {
            int n_pos = c_pos;
            direct_t n_dir = rot(c_dir, i);
            if (move(&n_pos, n_dir, width, len) && nodes[n_pos].s != Closed) {
                node_t *n_node = nodes + n_pos;
                if (c_node->dist + 1001 < n_node->dist) {
                    n_node->dist = c_node->dist + 1001;
                    n_node->dir = n_dir;
                    n_node->prev = c_pos;
                }
                if (n_node->s == Open) {
                    n_node->s = Visit;
                    visit_list[v_list_len++] = n_pos;
                }
            }
        }

        qsort(visit_list, v_list_len, sizeof(int), node_sort);
        if (v_list_len >= 255) {
            printf("Too smol lol\n");
            exit(-1);
        }
    }
    printf("\nDone!\n");
    if (!found) {
        return -1;
    }
    // Part 1
    {
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

uint64_t travel2(const char *const grid, const int width, const int len,
                 const int s_pos, const int e_pos, const direct_t s_dir) {
    printf("spos = %d; epos = %d\n", s_pos, e_pos);
    uint64_t res = 0;
    int visit_list[4096];
    int v_list_len;
    int found = 0;
    const int node_len = 4 * len;
    nodes = (node_t *)malloc(sizeof(node_t) * node_len);
    for (int i = 0; i < node_len; i++) {
        nodes[i].dist = (uint64_t)-1;
        nodes[i].s = Open;
        nodes[i].prev = -1;
        nodes[i].dir = i / len;
    }

    nodes[s_pos + len * s_dir].dist = 0;
    nodes[s_pos + len * s_dir].dir = s_dir;
    visit_list[0] = s_pos + len * s_dir;
    v_list_len = 1;

    while (v_list_len > 0) {
        const int c_index = visit_list[--v_list_len];
        const int g_index = c_index % len;
        node_t *c_node = nodes + c_index;
        direct_t c_dir = c_node->dir;

        // printf("c_pos=%d -> (%d, %d, %d) -> %c\n", c_index, g_index % width,
        // g_index / width, c_index / len, grid[g_index]);
        c_node->s = Closed;
        if (grid[g_index] == '#') {
            continue;
        }

        if (grid[g_index] == 'E') {
            found = 1;
            continue;
        }

        assert(grid[g_index] == '.' || grid[g_index] == 'S');

        {
            int n_pos = g_index;
            // Forward
            if (move(&n_pos, c_dir, width, len) &&
                nodes[n_pos + len * c_dir].s != Closed) {
                int n_index = n_pos + len * c_dir;
                node_t *n_node = nodes + n_index;
                if (c_node->dist + 1 < n_node->dist) {
                    n_node->dist = c_node->dist + 1;
                    n_node->dir = c_dir;
                    n_node->prev = c_index;
                }
                if (n_node->s == Open) {
                    n_node->s = Visit;
                    visit_list[v_list_len++] = n_index;
                }
            }
        }
        // Left Right
        for (int i = 0; i <= 1; i++) {
            int n_pos = g_index;
            direct_t n_dir = rot(c_dir, i);
            int n_index = n_pos + len * n_dir;
            node_t *n_node = nodes + n_index;
            // printf("LR %d %d -> cdir: %d ndir: %d dist: %lu\n", n_index,
            // n_node->s, c_dir, n_dir, n_node->dist);
            if (n_node->s == Closed) {
                continue;
            }
            if (c_node->dist + 1000 < n_node->dist) {
                n_node->dist = c_node->dist + 1000;
                n_node->dir = n_dir;
                n_node->prev = c_index;
            }
            if (n_node->s == Open) {
                n_node->s = Visit;
                visit_list[v_list_len++] = n_index;
            }
        }

        qsort(visit_list, v_list_len, sizeof(int), node_sort);
        if (v_list_len >= 4096) {
            printf("Too smol lol\n");
            exit(-1);
        }
    }
    printf("\nDone!\n");
    if (!found) {
        return -1;
    }
    printf("\n\n------\n\n");
    {
        char *occ = (char *)calloc(len, sizeof(char));
        int pos_list[4096], pos_len = 0;
        {
            uint64_t end_min = -1;
            for (int i = 0; i < 4; i++) {
                if (nodes[e_pos + i * len].dist < end_min) {
                    end_min = nodes[e_pos + i * len].dist;
                }
            }
            for (int i = 0; i < 4; i++) {
                if (nodes[e_pos + i * len].dist == end_min) {
                    pos_list[pos_len++] = e_pos + i * len;
                }
            }
        }
        while (pos_len > 0) {
            int p = pos_list[--pos_len];
            occ[p % len] = 'O';
            int n_pos = p % len;
            direct_t c_dir = p / len;
            move(&n_pos, rot(rot(c_dir, 1), 1), width, len);
            int neigh[] = {n_pos + len * c_dir,
                           ((p - len) + node_len) % node_len,
                           (p + len) % node_len};
            uint64_t offset[] = {1, 1000, 1000};
            int len_neigh = sizeof(neigh) / sizeof(int);

            for (int j = 0; j < len_neigh; j++) {
                int c_index = neigh[j];
                int g_index = c_index % len;
                node_t *n_node = &nodes[c_index];
                if (n_node->dist < nodes[p].dist && grid[g_index] != '#' &&
                    n_node->dist + offset[j] == nodes[p].dist) {
                    pos_list[pos_len++] = c_index;
                }
            }
            for (int i = 0; i < pos_len; i++) {
                printf("c_pos=%d -> (%d, %d, %d) |||| ", pos_list[i],
                       (pos_list[i] % len) % width, (pos_list[i] % len) / width,
                       pos_list[i] / len);
            }
            printf("\n");
        }
        for (int i = 0; i < len; i++) {
            char c = grid[i];
            if (occ[i] == 'O') {
                c = 'O';
                res++;
            }
            printf("%c", c);
            if ((i + 1) % width == 0) {
                printf("\n");
            }
        }
        free(occ);
    }

    free(nodes);
    return res;
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
    part1_total = travel(grid, width, len, start_pos, end_pos, Right);
    part2_total = travel2(grid, width, len, start_pos, end_pos, Right);

    printf("Part 1 total: %lu\n", part1_total);
    printf("Part 2 total: %lu\n", part2_total);

    free(grid);
    fclose(f);
    return 0;
}