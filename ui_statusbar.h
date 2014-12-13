#ifndef UI_STATUSBAR_H
#define UI_STATUSBAR_H 1

#include <ncurses.h>


struct statusbar {
  unsigned int y;
  unsigned int width;
  char *text;
  chtype attrs;
};

struct statusbar *
init_statusbar(unsigned int y, unsigned int width, chtype attrs);

void
free_statusbar(struct statusbar *a);

int
statusbar_cb(WINDOW *win, void *data, bool timed_out);

void
register_statusbar(struct statusbar *a);

void
set_statusbar_text(struct statusbar *a, const char *text);

#endif
