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

direct_t cmdtoch(char c) {
  switch (c) {
    case '^':
      return Up;
    case '>':
      return Left;
    case 'v':
      return Right;
    case '<':
      return Down;
    default:
      printf("Bad cmd\n");
      exit(-1);
  }
}

int push(char *const grid, const int width, const int len, int c_pos,
         direct_t d) {
  if (c_pos % width == 0 || (c_pos + 1) % width || c_pos < width ||
      c_pos >= len - width) {
    return 0;
  }
  int n_pos = c_pos;
  switch (d) {
    case Up:
      n_pos -= width;
      break;
    default:
      break;
  }
}

uint64_t do_cmds(char *const grid, const int g_width, const int g_len,
                 const char *const cmds, const int cmd_len,
                 const int start_pos) {
  int pos = start_pos;
  for (int i = 0; i < cmd_len; i++) {
    direct_t cmd = cmdtoch(cmds[i]);
  }
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
  char *cmds = (char *)malloc(sizeof(char) * (1 << 15));
  int g_len = 0, cmd_len, g_width = -1, start_pos = -1;

  {
    char buff[1024];
    char *ptr = grid;
    int *len = &g_len;
    while (fgets(buff, sizeof(buff), f)) {
      // Assume a full line is always read in
      if (g_width == -1) {
        g_width = strlen(buff) - 1;
      }
      if (buff[0] == '\n') {
        len = &cmd_len;
        ptr = cmds;
        continue;
      }
      memcpy(ptr, buff, g_width);
      ptr += g_width;
      *len += g_width;
    }
  }
  for (int i = 0; i < g_len; i++) {
    if (grid[i] == '@') {
      start_pos = i;
    }
  }

  do_cmds(grid, g_width, g_len, cmds, cmd_len, start_pos);

  free(grid);
  free(cmds);
  fclose(f);
  return 0;
}
