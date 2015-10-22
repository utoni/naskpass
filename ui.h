#ifndef UI_H
#define UI_H 1

#include <ncurses.h>
#include <stdint.h>

#define UICB_OK		0
#define UICB_ERR_UNDEF	1
#define UICB_ERR_CB	2
#define UICB_ERR_BUF	3

#define DOUI_OK		0
#define DOUI_ERR	1
#define DOUI_TMOUT	2
#define DOUI_PASSWD	3

#define UILOOP_TIMEOUT	1

#define UIKEY_ENTER	10
#define UIKEY_BACKSPACE	7
#define UIKEY_ESC	27
#define UIKEY_DOWN	2
#define UIKEY_UP	3
#define UIKEY_LEFT	4
#define UIKEY_RIGHT	5


typedef int (*ui_callback)(WINDOW *, void *, bool);
typedef int (*ui_input_callback)(WINDOW *, void *, int);


union ui_type {
  ui_callback ui_element;
  ui_input_callback ui_input;
};

struct nask_ui {
  union ui_type type;
  WINDOW *wnd;
  void *data;
  struct nask_ui *next;
};

void
register_ui_elt(ui_callback uicb, void *data, WINDOW *wnd);

void
register_ui_input(ui_input_callback ipcb, void *data, WINDOW *wnd);

void
unregister_ui_elt(void *data);

void
ui_thrd_force_update(void);

WINDOW *
init_ui(void);

void
free_ui(void);

int
do_ui(void);

#endif
