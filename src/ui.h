#ifndef UI_H
#define UI_H 1

#include <ncurses.h>
#include <stdint.h>

#define UICB_OK		0
#define UICB_ERR_UNDEF	1
#define UICB_ERR_CB	2
#define UICB_ERR_BUF	3

#define DOUI_OK		0
#define DOUI_ERR	4
#define DOUI_TMOUT	5
#define DOUI_NINIT	6

#define UILOOP_TIMEOUT	1

#define UIKEY_ACTIVATE	0
#define UIKEY_ENTER	10
#define UIKEY_BACKSPACE	7
#define UIKEY_ESC	27
#define UIKEY_DOWN	2
#define UIKEY_UP	3
#define UIKEY_LEFT	4
#define UIKEY_RIGHT	5


typedef int (*uicb_base)(WINDOW *, void *, bool);
typedef int (*uicb_input)(WINDOW *, void *, int);


struct ui_callbacks {
  uicb_base ui_element;
  uicb_input ui_input;
};

struct nask_ui {
  struct ui_callbacks cbs;
  WINDOW *wnd;
  void *data;
  struct nask_ui *next;
};

void
register_ui_elt(struct ui_callbacks *cbs, void *data, WINDOW *wnd);

void
unregister_ui_elt(void *data);

unsigned int
ui_get_maxx(void);

unsigned int
ui_get_maxy(void);

void
ui_set_cur(unsigned int x, unsigned int y);

unsigned int
ui_get_curx(void);

unsigned int
ui_get_cury(void);

int
activate_ui_input(void *data);

int
deactivate_ui_input(void *data);

void
ui_thrd_force_update(bool force_all);

void
ui_thrd_suspend(void);

void
ui_thrd_resume(void);

WINDOW *
init_ui(void);

void
free_ui(void);

char ui_wgetch(int timeout);

int
do_ui(void);

#endif
