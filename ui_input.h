#ifndef UI_ANIC_H
#define UI_ANIC_H 1

#include <ncurses.h>

struct input {
  bool box;
  bool shadow;
  char *input;
  size_t input_len;
  char *prompt;
};

struct input *
init_input(bool box, bool shadow, char *prompt, size_t input_len);

void
free_input(struct input *a);

int
input_cb(WINDOW *win, void *data, bool needs_update);

void
register_input(struct input *a, chtype attr);

#endif
