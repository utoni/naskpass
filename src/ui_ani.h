#ifndef UI_ANIC_H
#define UI_ANIC_H 1

#include <ncurses.h>


typedef int (*uicb_anic)(WINDOW *, void *, bool);

struct anic_default {
  char state;
  char *fmt;
};

struct anic {
  unsigned int x;
  unsigned int y;
  uicb_anic uicb;
  void *data;
  chtype attrs;
};

struct anic *
init_anic_default(unsigned int x, unsigned int y, chtype attrs, char *fmt);

struct anic *
init_anic(unsigned int x, unsigned int y, chtype attrs, uicb_anic uicb);

void
free_anic_default(struct anic *a);

void
free_anic(struct anic *a);

int
anic_cb(WINDOW *win, void *data, bool timed_out);

void
register_anic(struct anic *a, uicb_anic uicb);

void
register_anic_default(struct anic *a);

#endif
