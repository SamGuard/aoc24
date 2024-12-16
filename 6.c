#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>
#include <string.h>

typedef enum {
    Up, Down, Left, Right,
} direct_t;

int print = 0;

uint64_t travel(const char *const grid, const int width, const int len, int g_pos, uint64_t max_steps) {
    char *occ = (char *)calloc(len, sizeof(char));
    direct_t d = Up;
    uint64_t step = 0;

    occ[g_pos] = 'X';

    while (step < max_steps) {
        switch (d) {
        case Up: {
            if (g_pos - width < 0) {
                goto finish;
            }
            g_pos -= width;
            break;
        }
        case Down: {
            if (g_pos + width >= len) {
                goto finish;
            }
            g_pos += width;
            break;
        }
        case Left: {
            if (g_pos % width == 0) {
                goto finish;
            }
            g_pos -= 1;
            break;
        }
        case Right: {
            if ((g_pos + 1) % width == 0) {
                goto finish;
            }
            g_pos += 1;
            break;
        }
        default:
            printf("Bad\n");
            exit(-1);
        }

        if (grid[g_pos] == '.') {
            step++;
            occ[g_pos] = 'X';
            continue;
        }
        if (grid[g_pos] != '#') {
            printf("bad\n");
            exit(-1);
        }

        occ[g_pos] = '#';

        switch (d) {
        case Up:
            g_pos += width;
            d = Right;
            break;
        case Down:
            g_pos -= width;
            d = Left;
            break;
        case Left:
            g_pos += 1;
            d = Up;
            break;
        case Right:
            g_pos -= 1;
            d = Down;
            break;
        default:
            printf("Bad\n");
            exit(-1);
        }
    }
finish:
    if (step == max_steps) {
        return 0;
    }
    uint64_t total = 0;
    for (int i = 0; i < len; i++) {
        if (occ[i] == '\0') {
            if (print) printf(".");
        }
        else {
            if (occ[i] == 'X') {
                total++;
            }
            if (print) printf("%c", occ[i]);
        }
        if ((i + 1) % width == 0) {
            if (print) printf("\n");
        }
    }
    free(occ);
    return total;
}

uint64_t iter(const char *const grid, const int width, const int len, int g_pos) {
    char *cpy = (char *)malloc(len * sizeof(char));
    memcpy(cpy, grid, len * sizeof (char));
    uint64_t total = 0;
    printf("Running iterations...\n");
    for (int i = 0; i < len; i++) {
        if (i == g_pos || cpy[i] == '#') {
            continue;
        }
        if (cpy[i] != '.') {
            printf("AHHHHH %c\n", cpy[i]);
            exit(-1);
        }
        cpy[i] = '#';
        if (travel(cpy, width, len, g_pos, 1 << 20) == 0) {
            total++;
        }
        cpy[i] = '.';
        // printf("Progress: %d/%d\n", i, len);
    }
    free(cpy);
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
    int len = 0, width = -1, g_pos = -1;

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
        if (grid[i] == '^') {
            g_pos = i;
            grid[i] = '.';
            break;
        }
    }

    if (g_pos == -1) {
        exit(-1);
    }

    uint64_t part1_total = 0, part2_total = 0;
    print = 1;
    part1_total = travel(grid, width, len, g_pos, 1 << 16);
    print = 0;
    part2_total = iter(grid, width, len, g_pos);
    printf("Part 1 Total: %lu\n", part1_total);
    printf("Part 2 Total: %lu\n", part2_total);

    fclose(f);
    free(grid);
    return 0;
}

/*
    for (int i = 0; i < len; i++) {
        printf("%c", grid[i]);
        if ((i + 1) % width == 0) {
            printf("\n");
        }
    }
*/