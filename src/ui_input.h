#ifndef UI_INPUT_H
#define UI_INPUT_H 1

#include <ncurses.h>


struct input {
  unsigned int x;
  unsigned int y;
  unsigned int width;
  unsigned int cur_pos;
  char *input;
  size_t input_max;
  size_t input_len;
  size_t input_pos;
  char *prompt;
  chtype attrs;
  chtype shadow;
  uicb_input cb_input;
};

struct input *
init_input(unsigned int x, unsigned int y, unsigned int width, char *prompt, size_t input_len, chtype attrs, chtype shadow);

void
free_input(struct input *a);

int
activate_input(WINDOW *win,  struct input *a);

int
add_input(WINDOW *win, struct input *a, int key);

int
del_input(WINDOW *win, struct input *a);

int
clear_input(WINDOW *win, struct input *a);

void
register_input(WINDOW *win, struct input *a, uicb_input ipcb);

void
unregister_input(struct input *a);

#endif
