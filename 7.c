#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int part = 0;
int guess(uint64_t tgt, uint64_t acc, const uint64_t *const ins, int num_ins) {
    if (acc > tgt) {
        return 0;
    }
    if (num_ins == 0) {
        return tgt == acc;
    }

    // Only need to do one if its true
    if (guess(tgt, acc + ins[0], ins + 1, num_ins - 1)) {
        return 1;
    }
    if (guess(tgt, acc * ins[0], ins + 1, num_ins - 1)) {
        return 1;
    }

    if (part == 1) {
        return 0;
    }

    char s[128], *end = NULL;
    sprintf(s, "%lu%lu", acc, ins[0]);
    if (guess(tgt, strtoull(s, &end, 10), ins + 1, num_ins - 1)) {
        return 1;
    }
    return 0;
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

    char buff[256];
    uint64_t part1_total = 0, part2_total = 0;
    while (fgets(buff, sizeof(buff), f)) {
        uint64_t tgt, ins[32];
        int num_ins = 0;
        char *token = strtok(buff, ":"), *end = NULL;
        tgt = strtoull(token, &end, 10);
        while ((token = strtok(NULL, " ")) != NULL) {
            ins[num_ins++] = atoi(token);
        }
        part = 1;
        if (guess(tgt, ins[0], ins + 1, num_ins - 1)) {
            part1_total += tgt;
        }
        part = 2;
        if (guess(tgt, ins[0], ins + 1, num_ins - 1)) {
            part2_total += tgt;
        }
    }

    printf("Part 1 total: %lu\n", part1_total);
    printf("Part 2 total: %lu\n", part2_total);

    fclose(f);
    return 0;
}