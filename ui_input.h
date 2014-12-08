#ifndef UI_INPUT_H
#define UI_INPUT_H 1

#include <ncurses.h>


struct input {
  unsigned int x;
  unsigned int y;
  unsigned int width;
  unsigned int cur_pos;
#ifdef NCURSES_WIDECHAR
  wchar_t *input;
#else
  char *input;
#endif
  size_t input_max;
  size_t input_len;
  size_t input_pos;
#ifdef NCURSES_WIDECHAR
  wchar_t *prompt;
#else
  char *prompt;
#endif
  chtype attrs;
  chtype shadow;
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
input_cb(WINDOW *win, void *data, bool timed_out);

void
register_input(WINDOW *win, struct input *a);

#endif
