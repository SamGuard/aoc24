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

int print = 0;

direct_t cmdtoch(char c) {
    switch (c) {
        case '^':
            return Up;
        case '>':
            return Right;
        case 'v':
            return Down;
        case '<':
            return Left;
        default:
            printf("Bad cmd\n");
            exit(-1);
    }
}

int move(int c_pos, int width, direct_t d) {
    switch (d) {
        case Up:
            return c_pos - width;
        case Right:
            return c_pos + 1;
        case Down:
            return c_pos + width;
        case Left:
            return c_pos - 1;
        default:
            printf("Bad move\n");
            exit(-1);
    }
}

int push(char *const grid, const int width, const int len, int c_pos, direct_t d) {
    if (grid[c_pos] == '#') {
        return 0;
    }
    int n_pos = move(c_pos, width, d);

    if (grid[n_pos] == '.') {
        grid[n_pos] = grid[c_pos];
        return 1;
    }
    if (push(grid, width, len, n_pos, d)) {
        grid[n_pos] = grid[c_pos];
        return 1;
    }
    return 0;
}

int push2(char *const grid, const int width, const int len, int c_pos, direct_t d, int dry_run) {
    char c = grid[c_pos], n_c;
    if (c == '#') {
        return 0;
    }
    int n_pos = move(c_pos, width, d), n_pos_2;
    n_c = grid[n_pos];
    if (n_c == '.') {
        if (!dry_run) grid[n_pos] = c;
        return 1;
    }
    if (n_c == '#') {
        return 0;
    }
    assert(n_c == '[' || n_c == ']');
    if (d == Left || d == Right) {
        if (push2(grid, width, len, n_pos, d, dry_run)) {
            if (!dry_run) grid[n_pos] = c;
            return 1;
        }
        return 0;
    }

    if (n_c == '[') {
        n_pos_2 = n_pos + 1;
    } else if (n_c == ']') {
        n_pos_2 = n_pos - 1;
    }
    if (push2(grid, width, len, n_pos, d, 1) && push2(grid, width, len, n_pos_2, d, 1)) {
        if (!dry_run) {
            push2(grid, width, len, n_pos, d, 0);
            push2(grid, width, len, n_pos_2, d, 0);
            grid[n_pos] = c;
            grid[n_pos_2] = '.';
        }
        return 1;
    }
    return 0;
}

uint64_t part1_do_cmds(char *const grid, const int g_width, const int g_len, const char *const cmds,
                       const int cmd_len, const int start_pos) {
    int pos = start_pos;
    for (int i = 0; i < cmd_len; i++) {
        direct_t cmd = cmdtoch(cmds[i]);
        if (push(grid, g_width, g_len, pos, cmd)) {
            int n_pos = move(pos, g_width, cmd);
            grid[n_pos] = grid[pos];
            grid[pos] = '.';
            pos = n_pos;
        }
        if (!print) {
            continue;
        }
        printf("CMD: %c\n", cmds[i]);
        for (int j = 0; j < g_len; j++) {
            printf("%c", grid[j]);
            if ((j + 1) % g_width == 0) {
                printf("\n");
            }
        }
        printf("\n");
    }
    uint64_t res = 0;
    for (int i = 0; i < g_len; i++) {
        if (grid[i] == 'O') {
            res += i % g_width + 100 * (i / g_width);
        }
    }
    return res;
}

uint64_t part2_do_cmds(char *const in_grid, const int in_g_width, const int in_g_len,
                       const char *const cmds, const int cmd_len) {
    char *grid = (char *)malloc(sizeof(char) * (1 << 15));
    const int g_width = 2 * in_g_width, g_len = 2 * in_g_len;
    int pos = -1;

    for (int i = 0; i < in_g_len; i++) {
        char c1, c2;
        switch (in_grid[i]) {
            case '.':
                c1 = c2 = '.';
                break;
            case '#':
                c1 = c2 = '#';
                break;
            case 'O':
                c1 = '[';
                c2 = ']';
                break;
            case '@':
                c1 = '@';
                c2 = '.';
                break;
            default:
                printf("Bad grid %c\n", in_grid[i]);
                exit(-1);
        }
        grid[2 * i] = c1;
        grid[2 * i + 1] = c2;
    }
    printf("Starting grid:\n");
    for (int i = 0; i < g_len; i++) {
        if (grid[i] == '@') {
            pos = i;
        }
        printf("%c", grid[i]);
        if ((i + 1) % g_width == 0) {
            printf("\n");
        }
    }
    printf("\n");

    for (int i = 0; i < cmd_len; i++) {
        direct_t cmd = cmdtoch(cmds[i]);
        if (push2(grid, g_width, g_len, pos, cmd, 0)) {
            int n_pos = move(pos, g_width, cmd);
            grid[n_pos] = grid[pos];
            grid[pos] = '.';
            pos = n_pos;
        }
        if (!print) {
            continue;
        }
        printf("CMD: %c\n", cmds[i]);
        for (int j = 0; j < g_len; j++) {
            printf("%c", grid[j]);
            if ((j + 1) % g_width == 0) {
                printf("\n");
            }
        }
        printf("\n");
    }
    uint64_t res = 0;
    for (int i = 0; i < g_len; i++) {
        if (grid[i] == '[') {
            res += i % g_width + 100 * (i / g_width);
        }
    }
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

    char *grid = (char *)malloc(sizeof(char) * (1 << 15)),
         *gcpy = (char *)malloc(sizeof(char) * (1 << 15));
    char *cmds = (char *)malloc(sizeof(char) * (1 << 15));
    int g_len = 0, cmd_len, g_width = -1, start_pos = -1;

    {
        char buff[1024];
        char *ptr = grid;
        int *len = &g_len, *width = &g_width, cmd_width = -1;
        while (fgets(buff, sizeof(buff), f)) {
            // Assume a full line is always read in
            if (*width == -1) {
                *width = strlen(buff) - 1;
            }
            if (buff[0] == '\n') {
                len = &cmd_len;
                ptr = cmds;
                width = &cmd_width;
                continue;
            }
            memcpy(ptr, buff, *width);
            ptr += *width;
            *len += *width;
        }
    }
    for (int i = 0; i < g_len; i++) {
        if (grid[i] == '@') {
            start_pos = i;
        }
    }

    memcpy(gcpy, grid, sizeof(char) * g_len);

    uint64_t part1_total = 0, part2_total = 0;
    print = 0;
    part1_total = part1_do_cmds(grid, g_width, g_len, cmds, cmd_len, start_pos);
    print = 0;
    part2_total = part2_do_cmds(gcpy, g_width, g_len, cmds, cmd_len);

    printf("Part 1 total: %lu\n", part1_total);
    printf("Part 2 total: %lu\n", part2_total);

    free(grid);
    free(cmds);
    fclose(f);
    return 0;
}
