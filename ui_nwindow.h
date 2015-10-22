#ifndef UI_TXTWINDOW_H
#define UI_TXTWINDOW_H 1

#include <ncurses.h>

#include "ui.h"

#define INITIAL_TITLE_LEN 32

#define set_txtwindow_active(wnd, activate) wnd->active = activate; ui_thrd_force_update()

struct txtwindow {
  unsigned int y;
  unsigned int x;
  unsigned int width;
  unsigned int height;
  bool active;
  char *title;
  size_t title_len;
  char **text;
  int (*window_func)(WINDOW *, struct txtwindow *);
  chtype attrs;
  chtype text_attrs;
  void *userptr;
};

typedef int (*window_func)(WINDOW *, struct txtwindow *);

struct txtwindow *
init_txtwindow(unsigned int, unsigned int y, unsigned int width, unsigned int height, chtype attrs, chtype text_attrs, window_func cb_update);

void
free_txtwindow(struct txtwindow *a);

void
register_txtwindow(struct txtwindow *a);

void
set_txtwindow_text(struct txtwindow *a, char *text);

void
set_txtwindow_title(struct txtwindow *a, const char *title);

#endif
