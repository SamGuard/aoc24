#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>
#include <string.h>

int r_input(char *g, FILE *f, int *width) {
    char buff[256];
    int r_len;
    char *ptr = g;
    *width = -1;
    while ((r_len = fread(buff, sizeof(char), sizeof(buff), f)) != 0) {
        for (int i = 0; i < r_len; i++) {
            if (buff[i] == '\n') {
                if (*width == -1) {
                    *width = i;
                }
                continue;
            }
            *(ptr++) = buff[i];
        }
    }
    return ptr - g;
}

int part1_traverse(const char *const grid, const int g_width, const int g_len, const int dx, const int dy, int pos, int c_index) {
    static char word[] = "XMAS";
    for (int i = 0; i < c_index; i++) {
        printf(" ");
    }
    printf("%d %d %c\n", pos % g_width, pos / g_width, word[c_index]);
    if (word[c_index] == '\0') {
        printf("WAHOOO\n");
        return 1;
    }

    int total = 0;
    char c = word[c_index];

    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            if (x == 0 && y == 0) {
                continue;
            }
            int x_pos = pos % g_width,
                y_pos = pos / g_width, ndx, ndy;
            if (dx == 0 && dy == 0) {
                ndx = x; ndy = y;
            }
            else {
                ndx = dx; ndy = dy;
            }
            x_pos += ndx;
            y_pos += ndy;
            if (x_pos < 0 || x_pos >= g_width || y_pos < 0 || y_pos * g_width >= g_len) {
                continue;
            }
            if (grid[x_pos + y_pos * g_width] != c) {
                continue;
            }
            total += part1_traverse(grid, g_width, g_len, ndx, ndy, x_pos + y_pos * g_width, c_index + 1);
            if (dx != 0 || dy != 0) {
                return total;
            }
        }
    }

    return total;
}

#define P2MATCH(PATT, OUT) \
{ \
    OUT = 1; \
    for (int x = -1; x <= 1; x++) { \
        for (int y = -1; y <= 1; y++) { \
            int m_pos = x + 1 + (y + 1) * 3; \
            int x_pos = pos % g_width + x, \
                y_pos = pos / g_width + y; \
            if (x_pos < 0 || x_pos >= g_width || y_pos < 0 || y_pos * g_width >= g_len) { \
                OUT = 0; \
                continue; \
            } \
            if (PATT[m_pos] == '.') { \
                continue; \
            } \
            if (PATT[m_pos] != grid[x_pos + y_pos * g_width]) { \
                OUT = 0; \
                continue; \
            } \
        } \
    } \
}

int part2_traverse(const char *const grid, const int g_width, const int g_len, int pos) {
    /*
    M.S
    .A.
    M.S

    M.M
    .A.
    S.S

    S.S
    .A.
    M.M

    S.M
    .A.
    S.M
    */

    static const char m1[] = "M.S.A.M.S",
        m2[] = "M.M.A.S.S",
        m3[] = "S.S.A.M.M",
        m4[] = "S.M.A.S.M";

    int m1_match, m2_match, m3_match, m4_match;
    P2MATCH(m1, m1_match);
    P2MATCH(m2, m2_match);
    P2MATCH(m3, m3_match);
    P2MATCH(m4, m4_match);

    return m1_match + m2_match + m3_match + m4_match;
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
    if (!grid) {
        perror("fek");
    }

    int width;
    int len = r_input(grid, f, &width);
    fclose(f);

    uint64_t part1_total = 0, part2_total = 0;
    for (int i = 0; i < len; i++) {
        if (grid[i] != 'X') {
            continue;
        }
        part1_total += part1_traverse(grid, width, len, 0, 0, i, 1);
    }
    printf("Part 1 Total: %lu\n", part1_total);
    for (int i = 0; i < len; i++) {
        part2_total += part2_traverse(grid, width, len, i);
    }
    printf("Part 2 Total: %lu\n", part2_total);
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