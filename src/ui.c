#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <ncurses.h>
#include <sys/time.h>

#include "ui.h"
#include "ui_ipc.h"
#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui_txtwindow.h"
#include "ui_nask.h"

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
static WINDOW *wnd_main = NULL;
static struct nask_ui /* simple linked list to all UI objects */ *nui = NULL,
                      /* current active input */ *active = NULL;
static pthread_t thrd;
static int atmout = APP_TIMEOUT;
static pthread_cond_t cnd_update = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx_update = PTHREAD_MUTEX_INITIALIZER;


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
      cur = NULL;
      if (before != NULL) {
        before->next = next;
      } else {
        nui = next;
      }
    }
    if (cur != NULL)
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

bool
process_key(char key)
{
  bool ret = false;

  if ( active != NULL ) {
    ret = ( active->cbs.ui_input(active->wnd, active->data, key) == DOUI_OK ? true : false );
  }
  return ret;
}

int
do_ui_update(bool timed_out)
{
  int retval = UICB_OK;
  struct nask_ui *cur = nui;

  /* call all draw callback's */
  erase();
  if (!timed_out) {
    atmout = APP_TIMEOUT;
  } else if (atmout > 0) {
    atmout--;
  } else if (atmout == 0) {
    ui_ipc_semtrywait(SEM_UI);
  }
  while (cur != NULL) {
    if (cur->cbs.ui_element != NULL) {
      if ( cur->cbs.ui_element(cur->wnd, cur->data, timed_out) != UICB_OK)
        retval = UICB_ERR_CB;
      doupdate();
    } else {
      retval = UICB_ERR_UNDEF;
      break;
    }
    cur = cur->next;
  }
  /* TODO: Maybe export to an extra module? */
  attron(COLOR_PAIR(1));
  mvprintw(0, max_x - STRLEN(APP_TIMEOUT_FMT), "[" APP_TIMEOUT_FMT "]", atmout);
  attroff(COLOR_PAIR(1));
  /* EoT (End of Todo) */
  move(cur_y, cur_x);
  refresh();
  return (retval);
}

static int
ui_cond_timedwait(pthread_cond_t* cnd, pthread_mutex_t* mtx, int timeInMs)
{
  struct timeval tv;
  struct timespec ts;

  gettimeofday(&tv, NULL);
  ts.tv_sec   = time(NULL) + timeInMs/1000;
  ts.tv_nsec  = tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000);
  ts.tv_sec  += ts.tv_nsec / (1000 * 1000 * 1000);
  ts.tv_nsec %= (1000 * 1000 * 1000);

  return pthread_cond_timedwait(cnd, mtx, &ts);
}

static void *
ui_thrd(void *arg)
{
  int cnd_ret;

  pthread_mutex_lock(&mtx_update);
  do_ui_update(false);

  while ( ui_ipc_getvalue(SEM_UI) > 0 ) {
    cnd_ret = ui_cond_timedwait(&cnd_update, &mtx_update, UILOOP_TIMEOUT);
    if (cnd_ret == 0) {
      do_ui_update(true);
    } else if (cnd_ret == ETIMEDOUT) {
      do_ui_update(true);
    }
  }
  pthread_mutex_unlock(&mtx_update);
  return (NULL);
}

void
ui_thrd_force_update(bool force_all, bool timedout)
{
  pthread_mutex_lock(&mtx_update);
  if (force_all)
    touchwin(wnd_main);
 if (timedout) 
    pthread_cond_signal(&cnd_update);
  else
    do_ui_update(false);
  pthread_mutex_unlock(&mtx_update);
}

void
ui_thrd_suspend(void)
{
  pthread_mutex_lock(&mtx_update);
}

void
ui_thrd_resume(void)
{
  pthread_mutex_unlock(&mtx_update);
}

int
run_ui_thrd(void) {
  return (pthread_create(&thrd, NULL, &ui_thrd, NULL));
}

int
stop_ui_thrd(void)
{
  return (pthread_join(thrd, NULL));
}

int ui_wgetchtest(int timeout, char testchar)
{
  int retval = DOUI_OK;
  usleep(timeout/2);
  pthread_mutex_lock(&mtx_update);
  if ( ui_ipc_getvalue(SEM_UI) <= 0 ) {
    retval = DOUI_KEY;
  } else if ( wgetch(stdscr) == testchar ) {
    retval = DOUI_KEY;
  }
  pthread_mutex_unlock(&mtx_update);
  usleep(timeout/2);
  return retval;
}

char ui_wgetch(int timeout)
{
  char key;
  usleep(timeout/2);
  pthread_mutex_lock(&mtx_update);
  if ( ui_ipc_getvalue(SEM_UI) <= 0 ) {
    key = ERR;
  } else {
    key = wgetch(stdscr);
  }
  pthread_mutex_unlock(&mtx_update);
  usleep(timeout/2);
  return key;
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
  raw();
  keypad(stdscr, TRUE);
  noecho();
  nodelay(stdscr, TRUE);
  cbreak();
  set_escdelay(25);
  return wnd_main;
}

void
free_ui(void)
{
  delwin(wnd_main);
  endwin();
  clear();
  printf(" \033[2J\n");
}
