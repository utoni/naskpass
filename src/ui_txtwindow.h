#ifndef UI_TXTWINDOW_H
#define UI_TXTWINDOW_H 1

#include <ncurses.h>

#include "ui.h"

#define INITIAL_TITLE_LEN 32

#define set_txtwindow_active(wnd, activate) wnd->active = activate;
#define set_txtwindow_blink(wnd, blink) wnd->title_blink = blink;

struct txtwindow;

typedef int (*window_func)(WINDOW *, struct txtwindow *, bool);

struct txtwindow {
  unsigned int y;
  unsigned int x;
  unsigned int width;
  unsigned int height;
  bool active;
  char *title;
  bool title_blink;
  size_t title_len;
  char **text;
  window_func window_func;
  chtype attrs;
  chtype text_attrs;
};


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

void
set_txtwindow_dim(struct txtwindow *a, unsigned int w, unsigned int h);

#endif
