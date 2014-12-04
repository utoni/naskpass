#ifndef UI_H
#define UI_H 1

#include <ncurses.h>

struct anic {
  unsigned int x;
  unsigned int y;
  char state;
};

struct anic *
init_anic(unsigned int x, unsigned int y);

void
free_anic(struct anic *a);

int
anic_cb(WINDOW *win, void *data);

#endif
