#include <stdio.h>
#include <stdlib.h>
#include <stdint-gcc.h>

#define LIST_SIZE 1024

static int cmp_num(const void *p1, const void *p2) {
    return *(const int *)p1 > *(const int *)p2;
}

int main(int argc, char *argv[]) {
    int l_list[LIST_SIZE], r_list[LIST_SIZE];
    char buff[256];

    if (argc < 2) {
        printf("input pls\n");
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Oopsie daisies");
        return -1;
    }

    int row = 0;
    while (fgets(buff, sizeof(buff), f) != NULL) {
        sscanf(buff, "%d %d", l_list + row, r_list + row);
        row++;
    }

    qsort(l_list, row, sizeof(int), cmp_num);
    qsort(r_list, row, sizeof(int), cmp_num);

    // part 1
    uint64_t total = 0;
    for (int i = 0; i < row; i++) {
        total += abs(l_list[i] - r_list[i]);
        printf("%d - %d -> %d\n", l_list[i], r_list[i], abs(l_list[i] - r_list[i]));
    }
    printf("Part 1 Total: %lu\n", total);

    // part 2
    total = 0;
    for (int i = 0; i < row; i++) {
        uint64_t occur = 0;
        int x = l_list[i], y;
        for (int j = 0; j < row; j++) {
            y = r_list[j];
            if (x == y) {
                occur++;
            }
        }
        total += occur * x;
    }
    printf("Part 2 Total: %lu\n", total);
    fclose(f);
    return 0;
}