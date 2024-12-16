#include <stdint-gcc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REP_LEN 256

int f_is_safe(int *reports, int levels) {
  int is_increasing =
      reports[0]<reports[1], is_decreasing = reports[0]> reports[1],
      is_safe = 1;
  for (int i = 0; i < levels - 1; i++) {
    if (is_increasing && reports[i] > reports[i + 1]) {
      is_safe = 0;
      break;
    }
    if (is_decreasing && reports[i] < reports[i + 1]) {
      is_safe = 0;
      break;
    }
    int diff = abs(reports[i] - reports[i + 1]);
    if (diff < 1 || diff > 3) {
      is_safe = 0;
      break;
    }
  }
  return is_safe;
}

int dampen(int *reports, int levels) {
  // Assume report isnt safe
  int tmp_reports[REP_LEN];
  for (int i = 0; i < levels; i++) {
    // Copy all elems in reports into tmp_reports
    // Apart from one element
    int count = 0;
    for (int j = 0; j < levels; j++) {
      if (i == j) {
        continue;
      }
      tmp_reports[count++] = reports[j];
    }
    if (f_is_safe(tmp_reports, levels - 1)) {
      return 1;
    }
  }
  return 0;
}

int main(int argc, char *argv[]) {
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

  int reports[REP_LEN];
  uint64_t part1_total = 0, part2_total = 0;
  while (fgets(buff, sizeof(buff), f) != NULL) {
    char *token = NULL, *savptr;
    int levels = 0;
    // Remove \n from str
    buff[strlen(buff) - 1] = '\0';
    while ((token = strtok_r(token == NULL ? buff : NULL, " ", &savptr)) !=
           NULL) {
      printf("%s ", token);
      reports[levels++] = atoi(token);
    }
    printf("\n");
    int is_safe = f_is_safe(reports, levels);
    printf("Part1: is safe? %s\n", is_safe ? "yes" : "no");
    part1_total += is_safe ? 1 : 0;

    // Part 2
    if (is_safe) {
      // No need to do this as we already know its safe
      part2_total++;
      printf("Part1: is safe? yes\n");
      continue;
    }
    part2_total += dampen(reports, levels) ? 1 : 0;
    printf("Part2: is safe? %s\n", is_safe ? "yes" : "no");
  }
  printf("Part 1 total: %lu\n", part1_total);
  printf("Part 2 total: %lu\n", part2_total);
  fclose(f);
  return 0;
}