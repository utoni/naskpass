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

#include "ui.h"
#include "ui_ipc.h"
#include "ui_elements.h"
#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui_txtwindow.h"

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
static unsigned int cur_x, cur_y;
static WINDOW *wnd_main;
static struct nask_ui /* simple linked list to all UI objects */ *nui = NULL,
                      /* current active input */ *active = NULL;
static pthread_t thrd;
static unsigned int atmout = APP_TIMEOUT;
static pthread_cond_t cnd_update = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx_update = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_busy = PTHREAD_MUTEX_INITIALIZER;


void
register_ui_elt(struct ui_callbacks *cbs, void *data, WINDOW *wnd)
{
  struct nask_ui *tmp, *new;

  new = calloc(1, sizeof(struct nask_ui));
  new->cbs = *cbs;
  new->wnd = wnd;
  new->data = data;
  new->next = NULL;
  if (nui == NULL) {
    nui = new;
    nui->next = NULL;
  } else {
    tmp = nui;
    while (tmp->next != NULL) {
      tmp = tmp->next;
    }
    tmp->next = new;
  }
}

void
unregister_ui_elt(void *data)
{
  struct nask_ui *cur, *next, *before = NULL;

  cur = nui;
  while (cur != NULL) {
    next = cur->next;
    if (cur->data != NULL && cur->data == data) {
      free(cur);
      if (before != NULL) {
        before->next = next;
      } else {
        nui = next;
      }
    }
    before = cur;
    cur = next;
  }
}

unsigned int
ui_get_maxx(void)
{
  return max_x;
}

unsigned int
ui_get_maxy(void)
{
  return max_y;
}

void
ui_set_cur(unsigned int x, unsigned int y)
{
  cur_x = x;
  cur_y = y;
}

unsigned int
ui_get_curx(void)
{
  return (cur_x);
}

unsigned int
ui_get_cury(void)
{
  return (cur_y);
}

int
activate_ui_input(void *data)
{
  int ret = DOUI_ERR;
  struct nask_ui *cur = nui;

  if (cur == NULL || data == NULL) return DOUI_NINIT;
  while ( cur->data != NULL ) {
    if ( cur->data == data ) {
      if ( cur->cbs.ui_input != NULL && cur->cbs.ui_input(cur->wnd, data, UIKEY_ACTIVATE) == DOUI_OK ) {
        active = cur;
        ret = DOUI_OK;
        break;
      }
    }
    cur = cur->next;
  }
  return ret;
}

int
deactivate_ui_input(void *data)
{
  int ret = DOUI_ERR;

  if (active != NULL && data == active->data) {
    active = NULL;
    ret = DOUI_OK;
  }
  return ret;
}

static bool
process_key(char key)
{
  bool ret = false;

  if ( active != NULL ) {
    ret = ( active->cbs.ui_input(active->wnd, active->data, key) == DOUI_OK ? true : false );
  }
  return ret;
}

static int
do_ui_update(bool timed_out)
{
  int retval = UICB_OK;
  struct nask_ui *cur = nui;

  /* call all draw callback's */
  erase();
  if (timed_out == TRUE && atmout > 0) {
    atmout--;
  } else atmout = APP_TIMEOUT;
  while (cur != NULL) {
    if (cur->cbs.ui_element != NULL) {
      cur->cbs.ui_element(cur->wnd, cur->data, timed_out);
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
  wmove(wnd_main, cur_y, cur_x);
  wrefresh(wnd_main);
  return (retval);
}

static void *
ui_thrd(void *arg)
{
  int cnd_ret;
  struct timespec now;
  struct timespec wait;

  //gettimeofday(&now, NULL);
  clock_gettime(CLOCK_REALTIME, &now);
  wait.tv_sec = now.tv_sec + UILOOP_TIMEOUT;
  wait.tv_nsec = now.tv_nsec * 1000;
  do_ui_update(true);
  ui_ipc_sempost(SEM_RD);
  pthread_mutex_lock(&mtx_update);
  while ( ui_ipc_getvalue(SEM_UI) > 0 ) {
    cnd_ret = pthread_cond_timedwait(&cnd_update, &mtx_update, &wait);
    pthread_mutex_lock(&mtx_busy);
    do_ui_update( (cnd_ret == ETIMEDOUT ? true : false) );
    pthread_mutex_unlock(&mtx_busy);
    if (cnd_ret == ETIMEDOUT) {
      wait.tv_sec += UILOOP_TIMEOUT;
    }
  }
  pthread_mutex_unlock(&mtx_update);
  return (NULL);
}

void
ui_thrd_force_update(void)
{
  pthread_mutex_lock(&mtx_busy);
  pthread_cond_signal(&cnd_update);
  pthread_mutex_unlock(&mtx_busy);
}

void
ui_thrd_suspend(void)
{
  ui_thrd_force_update();
  pthread_mutex_lock(&mtx_busy);
}

void
ui_thrd_resume(void)
{
  pthread_mutex_unlock(&mtx_busy);
  ui_thrd_force_update();
}

WINDOW *
init_ui(void)
{
  wnd_main = initscr();
  max_x = getmaxx(wnd_main);
  max_y = getmaxy(wnd_main);
  cur_x = getcurx(wnd_main);
  cur_y = getcury(wnd_main);
  start_color();
  init_pair(1, COLOR_RED, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);
  /* TXTwindow */
  init_pair(4, COLOR_YELLOW, COLOR_RED);
  init_pair(5, COLOR_WHITE, COLOR_CYAN);
  /* EoF TXTwindow */
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
  int ret = DOUI_ERR;

  /* init TUI and UI Elements (input field, status bar, etc) */
  init_ui();
  init_ui_elements(wnd_main, max_x, max_y);

  pthread_mutex_lock(&mtx_update);
  if (run_ui_thrd() != 0) {
    return ret;
  }
  ui_ipc_semwait(SEM_RD);
  pthread_mutex_unlock(&mtx_update);
  wtimeout(stdscr, 1000);
  while ( ui_ipc_getvalue(SEM_UI) > 0 ) {
    if ((key = wgetch(wnd_main)) == ERR) {
      continue;
    }
    if ( process_key(key) != true ) {
      ui_ipc_semtrywait(SEM_UI);
    }
    ui_thrd_force_update();
  }
  stop_ui_thrd();
  free_ui_elements();

  return DOUI_OK;
}

