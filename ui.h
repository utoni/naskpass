#ifndef UI_H
#define UI_H 1

#include <ncurses.h>


#define UICB_OK		0
#define UICB_ERR_UNDEF	1
#define UICB_ERR_NOP	2
#define UICB_ERR_CB	3

typedef int (*ui_callback)(WINDOW *, void *, bool);

struct nask_ui {
  ui_callback ui_elt_cb;
  bool do_update;
  WINDOW *wnd;
  chtype attrs;
  void *data;
  struct nask_ui *next;
};

void
register_ui_elt(ui_callback uicb, void *data, WINDOW *wnd, chtype attrs);

void
unregister_ui_elt(void *data);

void
set_update(void *ptr_data, bool do_update);

void
init_ui(void);

void
free_ui(void);

int
run_ui_thrd(void);

int
stop_ui_thrd(void);

#endif
