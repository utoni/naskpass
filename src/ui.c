#include <stdio.h>
#include <unistd.h>
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


static unsigned int max_x, max_y;
static unsigned int cur_x, cur_y;
static WINDOW *wnd_main = NULL;
static struct nask_ui /* simple linked list to all UI objects */ *nui = NULL,
                      /* current active input */ *active = NULL;
static uicb_update update_callback = NULL;
static uicb_update postupdate_callback = NULL;
static pthread_t thrd;
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

static int
do_ui_update(bool timed_out)
{
  int retval = UICB_OK;
  struct nask_ui *cur = nui;

  /* call all draw callback's */
  erase();

  if (update_callback)
    if (update_callback(timed_out) != UICB_OK)
      return UICB_ERR_CB;

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

  if (postupdate_callback)
    if (postupdate_callback(timed_out) != UICB_OK)
      return UICB_ERR_CB;

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
      do_ui_update(false);
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
init_ui(uicb_update on_update_callback, uicb_update on_postupdate_callback)
{
  if (on_update_callback)
    update_callback = on_update_callback;
  if (on_postupdate_callback)
    postupdate_callback = on_postupdate_callback;
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
