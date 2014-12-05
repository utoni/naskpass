#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <ncurses.h>
#include <sys/time.h>

#include "ui.h"
#include "ui_ani.h"


static WINDOW *wnd_main;
static struct nask_ui *nui = NULL;
static pthread_t thrd;
static bool active;
static pthread_cond_t cnd_update = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx_update = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t tmretmtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ncbsy = PTHREAD_MUTEX_INITIALIZER;


void
register_ui_elt(ui_callback uicb, void *data, WINDOW *wnd, chtype attrs)
{
  struct nask_ui *tmp, *new;

  if (nui != NULL) {
    tmp = nui;
    while (tmp->next != NULL) {
     tmp = tmp->next;
    }
  }
  new = calloc(1, sizeof(struct nask_ui));
  new->ui_elt_cb = uicb;
  new->do_update = true;
  new->wnd = wnd;
  new->attrs = attrs;
  new->data = data;
  new->next = NULL;
  if (nui == NULL) {
    nui = new;
    nui->next = NULL;
  } else {
    tmp->next = new;
  }
}

void
unregister_ui_elt(void *data)
{
  struct nask_ui *cur = nui, *next, *before = NULL;

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

static int
do_ui_update(void)
{
  int retval = UICB_OK;
  struct nask_ui *cur = nui;

  while (cur != NULL) {
    if (cur->ui_elt_cb != NULL) {
      attron(cur->attrs);
      cur->ui_elt_cb(cur->wnd, cur->data, cur->do_update);
      attroff(cur->attrs);
    } else {
      retval = UICB_ERR_CB;
    }
    cur = cur->next;
  }
  refresh();
  return (retval);
}

static void *
ui_thrd(void *arg)
{
  struct timeval now;
  struct timespec wait;

  pthread_mutex_lock(&mtx_update);
  gettimeofday(&now, NULL);
  wait.tv_sec = now.tv_sec + 1;
  wait.tv_nsec = now.tv_usec * 1000;
  do_ui_update();
  while (active == true) {
    pthread_cond_timedwait(&cnd_update, &mtx_update, &wait);
    wait.tv_sec += 1;
    if (active == false) break;
    do_ui_update();
  }
  pthread_mutex_unlock(&mtx_update);
  return (NULL);
}

static void
init_ui(void)
{
  wnd_main = initscr();
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_WHITE);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);
  raw();
  keypad(stdscr, TRUE);
  noecho();
  cbreak();
}

static void
free_ui(void)
{
  delwin(wnd_main);
  endwin();
  clear();
}

int
run_ui_thrd(void) {
  init_ui();
  active = true;
  return (pthread_create(&thrd, NULL, &ui_thrd, NULL));
}

int
stop_ui_thrd(void) {
  active = false;
  free_ui();
  return (pthread_join(thrd, NULL));
}

int
main(int argc, char **argv)
{
  struct anic *heartbeat = init_anic(2,2);
  struct anic *a = init_anic(4,4);
  struct anic *b = init_anic(6,6);
  a->state = '-';
  b->state = '\\';

  register_anic(heartbeat, A_BOLD | COLOR_PAIR(3));
  register_anic(a,0); register_anic(b,COLOR_PAIR(1));
  if (run_ui_thrd() != 0) {
    exit(EXIT_FAILURE);
  }
sleep(5);
  stop_ui_thrd();
  unregister_ui_elt(a);
  unregister_ui_elt(heartbeat);
  unregister_ui_elt(b);
  free_anic(heartbeat);
  free_anic(a); free_anic(b);
  return (0);
}
