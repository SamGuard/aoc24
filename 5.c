#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int l, r;
} rule_t;

#define MAX_RULES 2048

// Needed out here for sorting
rule_t rules[MAX_RULES];
int num_rules = 0;

rule_t *find_rule(const int l, const int r) {
  for (int i = 0; i < num_rules; i++) {
    if (rules[i].l == l && rules[i].r == r) {
      return rules + i;
    }
  }
  return NULL;
}

static int page_sort(const void *p1, const void *p2) {
  int a, b;
  a = *(int *)p1;
  b = *(int *)p2;

  rule_t *rule1 = find_rule(a, b), *rule2 = find_rule(b, a);
  if (rule1 && rule2) {
    printf("Opposing rules!?!? %d %d\n", a, b);
    exit(-1);
  }
  if (rule1) {
    return -1;
  }
  if (rule2) {
    return 1;
  }
  printf("No rule for these pages!?!?! %d %d\n", a, b);
  return 0;
}

int main(int argc, char *argv[]) {
  uint64_t part1_total, part2_total;
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

  rule_t *c_rule;
  while (1) {
    if (!fgets(buff, sizeof(buff), f)) {
      // Shouldnt end up here :(
      perror("Something had gone really wrong");
      return -1;
    }
    if (buff[0] == '\n') {
      break;
    }
    c_rule = &rules[num_rules++];
    sscanf(buff, "%d|%d\n", &c_rule->l, &c_rule->r);
  }

  while (1) {
    if (!fgets(buff, sizeof(buff), f)) {
      // EOF
      break;
    }
    char *token = strtok(buff, ",");
    int pages[256], tmp_pages[256];
    int num_pages = 0;
    do {
      pages[num_pages++] = atoi(token);
    } while ((token = strtok(NULL, ",")) != NULL);

    memcpy(tmp_pages, pages, sizeof(pages));
    qsort(tmp_pages, num_pages, sizeof(int), page_sort);
    int is_correct = 1;
    for (int i = 0; i < num_pages; i++) {
      printf("%d ", pages[i]);
      if (pages[i] != tmp_pages[i]) {
        is_correct = 0;
        break;
      }
    }
    if (is_correct) {
      printf("<-- Is Correct!");
      part1_total += pages[num_pages / 2];
    } else {
      part2_total += tmp_pages[num_pages / 2];
      printf("<-- Not Correct");
    }
    printf("\n");
  }
  fclose(f);
  printf("Part 1 Total: %lu\n", part1_total);
  printf("Part 2 Total: %lu\n", part2_total);
  return 0;
}