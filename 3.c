#include <regex.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    char *buff = (char *)malloc(24000 * sizeof(char));
    regex_t mult_regex;

    if (regcomp(&mult_regex, "mul\\([0-9]{1,3},[0-9]{1,3}\\)", REG_EXTENDED)) {
        perror("regex no compile");
        return -1;
    }

    if (argc < 2) {
        printf("input pls\n");
        return -1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Oopsie daisies");
        return -1;
    }

    uint64_t part1_total = 0, part2_total = 0;
    char *s_start = buff, *s_end;
    fread(buff, sizeof(char), 24000, f);
    while (1) {
        s_start = strstr(s_start, "mul(");
        if (s_start == NULL) {
            break;
        }
        s_end = strstr(s_start, ")");
        if (s_end == NULL) {
            break;
        }
        int len = s_end - s_start + 1;
        if (len > 13) {
            s_start++;
            continue;
        }
        char s[14];
        memcpy(s, s_start, len);
        s[len] = '\0';

        int reti = regexec(&mult_regex, s, 0, NULL, 0);
        if (reti == REG_NOMATCH) {
            s_start++;
            continue;
        }
        if (reti) {
            char msgbuf[256];
            regerror(reti, &mult_regex, msgbuf, sizeof(msgbuf));
            printf("Regex match failed: %s\n", msgbuf);
            return -1;
        }

        int n1 = atoi(strtok(s + 4, ",)"));
        int n2 = atoi(strtok(NULL, ",)"));
        // printf("s %s n1 %d n2 %d\n", s, n1, n2);
        part1_total += n1 * n2;

        int do_or_do_not_there_is_no_try = 1;
        for (char *doos = s_start; doos > buff; doos--) {
            if (strncmp("do()", doos, sizeof("do()") - 1) == 0) {
                printf("do %s\n", s);
                do_or_do_not_there_is_no_try = 1;
                break;
            }
            if (strncmp("don't()", doos, sizeof("don't()") - 1) == 0) {
                printf("dont %s\n", s);
                do_or_do_not_there_is_no_try = 0;
                break;
            }
        }
        if (do_or_do_not_there_is_no_try) {
            part2_total += n1 * n2;
        }
        s_start++;
    }

    printf("Part 1 total: %lu\n", part1_total);
    printf("Part 2 total: %lu\n", part2_total);
    fclose(f);
    return 0;
}