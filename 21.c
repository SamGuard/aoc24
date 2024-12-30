#include <assert.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const int width, len, miss_index;
} keypad_t;

typedef struct {
    char *s;
    uint64_t s_len, s_size;
} out_str_t;

keypad_t key_door = {3, 12, 9}, key_dir = {3, 6, 0};

int print = 1;

int door_num_to_index(int num) {
    switch (num) {
        case -1:  // Press Accept
            return 11;
        case 0:
            return 10;
        case 1:
        case 2:
        case 3:
            return 6 + num - 1;
        case 4:
        case 5:
        case 6:
            return 3 + num - 4;
        case 7:
        case 8:
        case 9:
            return 0 + num - 7;
        default:
            printf("Bad number %d\n", num);
            exit(-1);
    }
}

int dir_cmd_to_index(char cmd) {
    switch (cmd) {
        case '^':
            return 1;
        case 'A':
            return 2;
        case '<':
            return 3;
        case 'v':
            return 4;
        case '>':
            return 5;
        default:
            printf("Bad cmd %c\n", cmd);
            exit(-1);
    }
}

int num_dir_keypads = 0;
int dir_push(const char cmd, const int level, const int start_index, uint64_t * const out_size) {
    const int width = key_dir.width;

    assert(num_dir_keypads != 0);
    if (level == num_dir_keypads) {
        if (print) {
            printf("Adding to str: %c\n", cmd);
        }
        (*out_size)++;
        return start_index;
    }

    if (print) {
        printf("Dir level %d wants %c\n", level, cmd);
    }
    // Always start at Accept button
    const int tgt_i = dir_cmd_to_index(cmd);
    const int dx = (tgt_i % width) - start_index % width,
              dy = (tgt_i / width) - start_index / width, next_level = level + 1;

    int c_pos = 2;  // Pos of the robot hand
    for (int i = 0; i < abs(dy); i++) {
        char cmd = 'v';
        if (dy < 0) {
            cmd = '^';
        }
        c_pos = dir_push(cmd, next_level, c_pos, out_size);
    }

    for (int i = 0; i < abs(dx); i++) {
        char cmd = '>';
        if (dx < 0) {
            cmd = '<';
        }
        c_pos = dir_push(cmd, next_level, c_pos, out_size);
    }
    dir_push('A', next_level, c_pos, out_size);

    return tgt_i;
}

int door_push(const int num, const int start_index, uint64_t *const out_size) {
    int width = key_door.width;

    if (print) {
        printf("Door wants %d\n", num);
    }

    // Always start at Accept button
    const int tgt_i = door_num_to_index(num);
    const int dx = (tgt_i % width) - start_index % width,
              dy = (tgt_i / width) - start_index / width;

    { // Pls dont look at this mess
        int diffs[] = {0, 0};
        char cmds[] = {'\0','\0'};
        uint64_t tmp_out[2];
        for (int i = 0; i < 2; i++) {
            tmp_out[i] = *out_size;
            if (i == 0) {
                diffs[0] = dx;
                diffs[1] = dy;
                cmds[0] = dx > 0 ? '>' : '<';
                cmds[1] = dy > 0 ? 'v' : '^';
                // Invalid route
                if (start_index + diffs[0] == key_door.miss_index &&
                    start_index / width == key_door.miss_index / width) {
                    tmp_out[i] = -1;
                    continue;
                }
            } else {
                diffs[0] = dy;
                diffs[1] = dx;
                cmds[0] = dy > 0 ? 'v' : '^';
                cmds[1] = dx > 0 ? '>' : '<';
                // Invalid route
                if (start_index + diffs[0] * width == key_door.miss_index) {
                    tmp_out[i] = -1;
                    continue;
                }
            }

            int c_pos = 2;
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < abs(diffs[j]); k++) {
                    c_pos = dir_push(cmds[j], 0, c_pos, tmp_out + i);
                }
            }
            dir_push('A', 0, c_pos, tmp_out + i);
        }

        if (tmp_out[0] < tmp_out[1]) {
            *out_size = tmp_out[0];
        } else {
            *out_size = tmp_out[1];
        }
        assert(*out_size != (uint64_t)-1);
    }

    return tgt_i;
}

uint64_t do_code(const char *const seq, int num_seq) {
    int c_pos = 11;

    uint64_t res = 0;

    for (int i = 0; i < num_seq; i++) {
        printf("%c", seq[i]);
        int num = seq[i] - '0';
        if (seq[i] == 'A') {
            num = -1;
        }
        c_pos = door_push(num, c_pos, &res);
    }
    return res;
}

uint64_t part1(const char *const all_seqs, int seq_len) {
    print = 0;
    uint64_t total = 0;
    num_dir_keypads = 2;
    for (int i = 0; i < seq_len; i += 4) {
        uint64_t n, res;
        res = do_code(all_seqs + i, 4);
        // Convert to number
        {
            char str[] = {all_seqs[i + 0], all_seqs[i + 1], all_seqs[i + 2], '\0'};
            n = (uint64_t)atoi(str);
        }
        printf("%lu * %lu\n", res, n);

        total += res * n;
    }

    printf("= %lu\n", total);
    return total;
}

uint64_t part2(const char *const all_seqs, int seq_len) {
    print = 0;
    uint64_t total = 0;
    num_dir_keypads = 25;
    for (int i = 0; i < seq_len; i += 4) {
        uint64_t n, res;
        res = do_code(all_seqs + i, 4);
        // Convert to number
        {
            char str[] = {all_seqs[i + 0], all_seqs[i + 1], all_seqs[i + 2], '\0'};
            n = (uint64_t)atoi(str);
        }
        printf("%lu * %lu\n", res, n);

        total += res * n;
    }

    printf("= %lu\n", total);
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

    char seqs[256];
    int len_seqs = 0;

    {
        char buff[256];
        while (fgets(buff, sizeof(buff), f) != NULL) {
            memcpy(seqs + len_seqs, buff, sizeof(char) * 4);
            len_seqs += 4;
        }
    }

    uint64_t part1_total, part2_total;
    part1_total = part1(seqs, len_seqs);
    part2_total = part2(seqs, len_seqs);

    printf("Part 1 total: %lu\n", part1_total);
    printf("Part 2 total: %lu\n", part2_total);

    fclose(f);
    return 0;
}