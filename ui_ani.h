#ifndef UI_ANIC_H
#define UI_ANIC_H 1

#include <ncurses.h>


struct anic {
  unsigned int x;
  unsigned int y;
  char state;
  chtype attrs;
};

struct anic *
init_anic(unsigned int x, unsigned int y, chtype attrs);

void
free_anic(struct anic *a);

int
anic_cb(WINDOW *win, void *data, bool timed_out);

void
register_anic(struct anic *a);

#endif
