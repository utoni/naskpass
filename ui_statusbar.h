#ifndef UI_STATUSBAR_H
#define UI_STATUSBAR_H 1

#include <ncurses.h>


#define UPDATE_CBDEF int (*update_func)(WINDOW *, struct statusbar *)

struct statusbar {
  unsigned int y;
  unsigned int width;
  char *text;
  UPDATE_CBDEF;
  chtype attrs;
};

typedef UPDATE_CBDEF;

struct statusbar *
init_statusbar(unsigned int y, unsigned int width, chtype attrs, update_func cb_update);

void
free_statusbar(struct statusbar *a);

int
statusbar_cb(WINDOW *win, void *data, bool timed_out);

void
register_statusbar(struct statusbar *a);

void
set_statusbar_text(struct statusbar *a, const char *text);

#endif
