#include <assert.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CACHE_LEN 128

typedef enum {
    White,
    Blue,
    Black,
    Red,
    Green,
} col_t;

typedef struct {
    col_t c[32];
    int len;
} towel_t;

typedef struct {
    int index;
    uint64_t res;
} cache_entry_t;

col_t c_to_c(char c) {
    switch (c) {
        case 'w':
            return White;
        case 'u':
            return Blue;
        case 'b':
            return Black;
        case 'r':
            return Red;
        case 'g':
            return Green;
        default:
            printf("Bad colour :(\n");
            exit(-1);
    }
}

cache_entry_t *cache;
int num_cache = 0, cache_size = 0;

cache_entry_t *find_cache_ent(const int col_index) {
    if (!cache) {
        return NULL;
    }
    for (int i = 0; i < num_cache; i++) {
        cache_entry_t *c = cache + i;
        if (c->index == col_index) {
            return c;
        }
    }
    return NULL;
}

void add_cache_ent(const int col_index, const uint64_t res) {
    if (cache == NULL || cache_size == 0 || num_cache == cache_size) {
        if (cache_size == 0 || cache == NULL) {
            free(cache);  // Incase already defined
            cache = NULL;
            cache_size = 1 << 15;
            num_cache = 0;
        } else {
            cache_size = (int)(cache_size * 1.5f);
            if (cache_size > 1 << 29) {
                printf("Cache getting too big\n");
                exit(-1);
            }
        }
        cache = (cache_entry_t *)realloc(cache, sizeof(cache_entry_t) * cache_size);
        if (!cache) {
            printf("Error allocing new memory for cache\n");
            exit(-1);
        }
    }

    cache_entry_t *c = cache + (num_cache++);
    c->index = col_index;
    c->res = res;
}

uint64_t search(int col_index, const towel_t *const tools, const int num_tools,
                const col_t *const cols, const int num_cols) {
    uint64_t res = 0;
    if (col_index == num_cols) {
        return 1;
    }
    cache_entry_t *c;
    if (col_index < MAX_CACHE_LEN && (c = find_cache_ent(col_index)) != NULL) {
        return c->res;
    }

    for (int i = 0; i < num_tools; i++) {
        const towel_t *const t = tools + i;
        if (col_index - 1 + t->len >= num_cols) {
            continue;
        }
        int does_match = 1;
        for (int j = 0; j < t->len; j++) {
            if (t->c[j] != cols[col_index + j]) {
                does_match = 0;
                break;
            }
        }
        if (!does_match) {
            continue;
        }
        res += search(col_index + t->len, tools, num_tools, cols, num_cols);
    }
    if (col_index < MAX_CACHE_LEN) {
        add_cache_ent(col_index, res);
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

    towel_t tools[1024];
    int num_tools = 0;
    {
        size_t buff_sz = sizeof(char) * (1 << 15);
        char *buff = (char *)malloc(buff_sz);

        fgets(buff, buff_sz, f);
        char *token = strtok(buff, ", ");
        do {
            towel_t *t = tools + num_tools;
            t->len = 0;
            for (int i = 0; token[i] != '\0' && token[i] != '\n'; i++) {
                t->c[i] = c_to_c(token[i]);
                t->len++;
            }
            num_tools++;
        } while ((token = strtok(NULL, ", ")) != NULL);
        free(buff);
    }
    {
        uint64_t part1_total = 0, part2_total = 0;
        char buff[256];
        while (fgets(buff, sizeof(buff), f) != NULL) {
            col_t tgt[128];
            int num_cols = 0;
            if (buff[0] == '\0' || buff[0] == '\n') {
                continue;
            }
            for (int i = 0; buff[i] != '\0' && buff[i] != '\n'; i++) {
                printf("%c", buff[i]);
                tgt[num_cols++] = c_to_c(buff[i]);
            }
            printf(" ---> ");
            free(cache);
            cache = NULL;
            num_cache = 0;
            uint64_t res = search(0, tools, num_tools, tgt, num_cols);
            if (res) {
                printf("Match!\n");
                part1_total++;
                part2_total += res;
            } else {
                printf("Not Match\n");
            }
        }

        printf("Part 1 total: %lu\n", part1_total);
        printf("Part 2 total: %lu\n", part2_total);
    }

    fclose(f);
    return 0;
}