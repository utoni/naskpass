#ifndef UI_TXTWINDOW_H
#define UI_TXTWINDOW_H 1

#include <ncurses.h>


#define INITIAL_TITLE_LEN 32
#define INITIAL_TEXT_LEN 128

struct txtwindow {
  unsigned int y;
  unsigned int x;
  unsigned int width;
  unsigned int height;
  bool scrollable;
  char *title;
  size_t title_len;
  char *text;
  size_t text_len;
  int (*window_func)(WINDOW *, struct txtwindow *);
  chtype attrs;
  chtype text_attrs;
};

typedef int (*window_func)(WINDOW *, struct txtwindow *);

struct txtwindow *
init_txtwindow(unsigned int, unsigned int y, unsigned int width, unsigned int height, chtype attrs, chtype text_attrs, window_func cb_update);

void
free_txtwindow(struct txtwindow *a);

int
txtwindow_cb(WINDOW *win, void *data, bool timedout);

void
register_txtwindow(struct txtwindow *a);

void
set_txtwindow_text(struct txtwindow *a, char *text);

void
set_txtwindow_scrollable(struct txtwindow *a, bool scrollable); 

void
set_txtwindow_title(struct txtwindow *a, const char *title);

#endif
