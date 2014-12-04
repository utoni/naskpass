#ifndef UI_H
#define UI_H 1

#include <ncurses.h>


#define UICB_OK		0
#define UICB_ERR_UNDEF	1

typedef int (*ui_callback)(WINDOW *, void *);

struct nask_ui {
  ui_callback ui_elt_cb;
  bool do_update;
  void *data;
  struct nask_ui *next;
};

void
register_ui_elt(ui_callback uicb, void *data);

void
unregister_ui_elt(void *data);

#endif
