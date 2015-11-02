#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ncurses.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>

#include "ui.h"
#include "ui_ipc.h"
#include "ui_elements.h"
#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui_nwindow.h"

#include "status.h"
#include "config.h"

#define APP_TIMEOUT 60
#define APP_TIMEOUT_FMT "%02d"
#define PASSWD_WIDTH 35
#define PASSWD_HEIGHT 5
#define PASSWD_XRELPOS (unsigned int)(PASSWD_WIDTH / 2) - (PASSWD_WIDTH / 6)
#define PASSWD_YRELPOS (unsigned int)(PASSWD_HEIGHT / 2) + 1
#define INFOWND_WIDTH 25
#define INFOWND_HEIGHT 3
#define INFOWND_XRELPOS (unsigned int)(INFOWND_WIDTH / 2) - (INFOWND_WIDTH / 6)
#define INFOWND_YRELPOS (unsigned int)(INFOWND_HEIGHT / 2) + 1

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))


static unsigned int max_x, max_y;
static WINDOW *wnd_main;
static struct nask_ui /* simple linked list to all GUI objects */ *nui = NULL,
                      /* simple linked list to all INPUT objects */ *nin = NULL, /* current active input */ *active = NULL;
static pthread_t thrd;
static unsigned int atmout = APP_TIMEOUT;
static pthread_cond_t cnd_update = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx_update = PTHREAD_MUTEX_INITIALIZER;


static void
register_basic(enum ui_type utype, union ui_data uicb, void *data, WINDOW *wnd)
{
  struct nask_ui *tmp, *new, *ui = NULL;

  switch (utype) {
    case UI_ELEMENT:
      ui = nui;
      break;
    case UI_INPUT:
      ui = nin;
      break;
  }
  new = calloc(1, sizeof(struct nask_ui));
  new->type = utype;
  new->callback = uicb;
  new->wnd = wnd;
  new->data = data;
  new->next = NULL;
  if (ui == NULL) {
    ui = new;
    ui->next = NULL;
    switch (utype) {
      case UI_ELEMENT:
        nui = ui;
        break;
      case UI_INPUT:
        nin = ui;
        break;
    }
  } else {
    tmp = ui;
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    tmp->next = new;
  }
}

void
register_ui_elt(ui_callback uicb, void *data, WINDOW *wnd)
{
  union ui_data cb;
  cb.ui_element = uicb;
  register_basic(UI_ELEMENT, cb, data, wnd);
}

void
register_ui_input(ui_input_callback ipcb, void *data, WINDOW *wnd)
{
  union ui_data cb;
  cb.ui_input = ipcb;
  register_basic(UI_INPUT, cb, data, wnd);
}

static void
unregister_basic(enum ui_type utype, void *data)
{
  struct nask_ui *cur, *next, *before = NULL, **ui = NULL;

  switch (utype) {
    case UI_ELEMENT:
      ui = &nui;
      break;
    case UI_INPUT:
      ui = &nin;
      break;
  }
  cur = *ui;
  while (cur != NULL) {
    next = cur->next;
    if (cur->data != NULL && cur->data == data) {
      free(cur);
      if (before != NULL) {
        before->next = next;
      } else {
        *ui = next;
      }
    }
    before = cur;
    cur = next;
  }
}

void
unregister_ui_elt(void *data)
{
  unregister_basic(UI_ELEMENT, data);
}

void
unregister_ui_input(void *data)
{
  unregister_basic(UI_INPUT, data);
}

int
activate_ui_input(void *data)
{
  struct nask_ui *cur = nin;

  if (cur == NULL || data == NULL) return DOUI_NINIT;
  while ( cur != NULL ) {
    if ( cur == data && cur->type == UI_INPUT ) {
      if ( cur->callback.ui_input(cur->wnd, data, UIKEY_ACTIVATE) == DOUI_OK ) {
        active = cur;
        return DOUI_OK;
      }
    }
    cur = cur->next;
  }
  return DOUI_ERR;
}

static bool
process_key(char key)
{
  atmout = APP_TIMEOUT;
  if ( active != NULL ) {
printf("XXXXXXXX\n");
    return ( active->callback.ui_input(active->wnd, active->data, key) == DOUI_OK ? true : false );
  }
  return false;
}

static int
do_ui_update(bool timed_out)
{
  int retval = UICB_OK;
  int curx = getcurx(wnd_main);
  int cury = getcury(wnd_main);
  struct nask_ui *cur = nui;

  /* call all draw callback's */
  erase();
  while (cur != NULL) {
    if (cur->type == UI_ELEMENT && cur->callback.ui_element != NULL) {
      cur->callback.ui_element(cur->wnd, cur->data, timed_out);
      doupdate();
    } else {
      retval = UICB_ERR_CB;
    }
    cur = cur->next;
  }
  /* TODO: Maybe export to an extra module? */
  attron(COLOR_PAIR(1));
  mvprintw(0, max_x - STRLEN(APP_TIMEOUT_FMT), "[" APP_TIMEOUT_FMT "]", atmout);
  attroff(COLOR_PAIR(1));
  /* EoT (End of Todo) */
  wmove(wnd_main, cury, curx);
  wrefresh(wnd_main);
  return (retval);
}

static void *
ui_thrd(void *arg)
{
  int cnd_ret;
  struct timeval now;
  struct timespec wait;

  gettimeofday(&now, NULL);
  wait.tv_sec = now.tv_sec + UILOOP_TIMEOUT;
  wait.tv_nsec = now.tv_usec * 1000;
  do_ui_update(true);
  ui_ipc_sempost(SEM_RD);
  while ( ui_ipc_getvalue(SEM_UI) > 0 ) {
    pthread_mutex_lock(&mtx_update);
    cnd_ret = pthread_cond_timedwait(&cnd_update, &mtx_update, &wait);
    if (--atmout == 0) ui_ipc_semtrywait(SEM_UI);
    do_ui_update( (cnd_ret == ETIMEDOUT ? true : false) );
    if (cnd_ret == ETIMEDOUT) {
      wait.tv_sec += UILOOP_TIMEOUT;
    }
    pthread_mutex_unlock(&mtx_update);
  }
  pthread_mutex_unlock(&mtx_update);
  return (NULL);
}

void
ui_thrd_force_update(void)
{
  pthread_mutex_lock(&mtx_update);
  pthread_cond_signal(&cnd_update);
  pthread_mutex_unlock(&mtx_update);
}

WINDOW *
init_ui(void)
{
  wnd_main = initscr();
  max_x = getmaxx(wnd_main);
  max_y = getmaxy(wnd_main);
  start_color();
  init_pair(1, COLOR_RED, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);
  init_pair(4, COLOR_YELLOW, COLOR_RED);
  raw();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
  return (wnd_main);
}

void
free_ui(void)
{
  delwin(wnd_main);
  endwin();
  clear();
  printf(" \033[2J\n");
}

static int
run_ui_thrd(void) {
  return (pthread_create(&thrd, NULL, &ui_thrd, NULL));
}

static int
stop_ui_thrd(void)
{
  return (pthread_join(thrd, NULL));
}

int
do_ui(void)
{
  char key = '\0';
  char *title = NULL;
  int ret = DOUI_ERR;

  asprintf(&title, "/* %s-%s */", PKGNAME, VERSION);
  /* init TUI and UI Elements (input field, status bar, etc) */
  init_ui();
  init_ui_elements(wnd_main, max_x, max_y);

  if (run_ui_thrd() != 0) {
    goto error;
  }
  ui_ipc_semwait(SEM_RD);
  wtimeout(wnd_main, 10);
  while ( ui_ipc_getvalue(SEM_UI) > 0 ) {
    if ((key = wgetch(wnd_main)) == '\0') {
      break;
    }
    if (key == -1) {
      continue;
    }
    if ( process_key(key) != true ) {
      break;
    } else printf("XXXXX\n");
    do_ui_update(false);
  }
  ui_thrd_force_update();
  stop_ui_thrd();
  free_ui_elements();

  ret = DOUI_OK;
error:
  if (title) free(title);
  title = NULL;
  return ret;
}

