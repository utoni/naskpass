#ifndef UI_INPUT_H
#define UI_INPUT_H 1

#include <ncurses.h>


struct input {
  unsigned int x;
  unsigned int y;
  unsigned int width;
  char *input;
  size_t input_max;
  size_t input_len;
  size_t input_pos;
  char *prompt;
  bool active;
};

struct input *
init_input(unsigned int x, unsigned int y, unsigned int width, char *prompt, size_t input_len);

void
free_input(struct input *a);

int
input_cb(WINDOW *win, void *data, bool needs_update);

void
register_input(WINDOW *win, struct input *a, chtype attr);

#endif
