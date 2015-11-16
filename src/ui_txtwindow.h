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
init_txtwindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height, window_func cb_update);

struct txtwindow *
init_txtwindow_centered(unsigned int width, unsigned int height, window_func cb_update);

void
free_txtwindow(struct txtwindow *a);

void
register_txtwindow(struct txtwindow *a);

void
set_txtwindow_text(struct txtwindow *a, char *text);

void
set_txtwindow_title(struct txtwindow *a, const char *title);

void
set_txtwindow_color(struct txtwindow *a, chtype wnd, chtype txt);

void
set_txtwindow_pos(struct txtwindow *a, unsigned int x, unsigned int y);

#endif
