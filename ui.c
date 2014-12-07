#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <ncurses.h>
#include <sys/time.h>

#include "ui.h"
#include "ui_ani.h"
#include "ui_input.h"


static WINDOW *wnd_main;
static struct nask_ui *nui = NULL;
static pthread_t thrd;
static bool active;
static pthread_cond_t cnd_update = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx_update = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_cb = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mtx_busy = PTHREAD_MUTEX_INITIALIZER;
static sem_t sem_rdy;


void
register_ui_elt(ui_callback uicb, void *data, WINDOW *wnd)
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
  new->wnd = wnd;
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
do_ui_update(bool timed_out)
{
  int retval = UICB_OK;
  int curx = getcurx(wnd_main);
  int cury = getcury(wnd_main);
  struct nask_ui *cur = nui;

  /* call all draw callback's */
  erase();
  while (cur != NULL) {
    if (cur->ui_elt_cb != NULL) {
      pthread_mutex_lock(&mtx_cb);
      cur->ui_elt_cb(cur->wnd, cur->data, timed_out);
      doupdate();
      pthread_mutex_unlock(&mtx_cb);
    } else {
      retval = UICB_ERR_CB;
    }
    cur = cur->next;
  }
  wmove(wnd_main, cury, curx);
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
  wait.tv_sec = now.tv_sec + UILOOP_TIMEOUT;
  wait.tv_nsec = now.tv_usec * 1000;
  do_ui_update(true);
  sem_post(&sem_rdy);
  while (active == true) {
    pthread_mutex_unlock(&mtx_busy);
    pthread_cond_timedwait(&cnd_update, &mtx_update, &wait);
    wait.tv_sec += UILOOP_TIMEOUT;
    pthread_mutex_lock(&mtx_busy);
    if (active == false) break;
    do_ui_update(true);
  }
  pthread_mutex_unlock(&mtx_busy);
  pthread_mutex_unlock(&mtx_update);
  return (NULL);
}

void
ui_thrd_force_update(void)
{
  pthread_cond_signal(&cnd_update);
}

void
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

void
free_ui(void)
{
  delwin(wnd_main);
  endwin();
  clear();
  printf(" \033[2J");
}

int
run_ui_thrd(void) {
  pthread_mutex_lock(&mtx_busy);
  active = true;
  pthread_mutex_unlock(&mtx_busy);
  return (pthread_create(&thrd, NULL, &ui_thrd, NULL));
}

int
stop_ui_thrd(void) {
  pthread_mutex_lock(&mtx_busy);
  active = false;
  pthread_mutex_unlock(&mtx_busy);
  return (pthread_join(thrd, NULL));
}

static bool
process_key(int key, struct input *a, WINDOW *win)
{
  bool retval = true;

  pthread_mutex_lock(&mtx_busy);
  switch (key) {
    case UIKEY_ENTER:
      break;
    case UIKEY_BACKSPACE:
      del_input(win, a);
      break;
    case UIKEY_ESC:
      retval = active = false;
      ui_thrd_force_update();
      break;
    case UIKEY_DOWN:
    case UIKEY_UP:
    case UIKEY_LEFT:
    case UIKEY_RIGHT:
      break;
    default:
      add_input(win, a, key);
  }
  //mvprintw(0,0,"*%d*", key);
  pthread_mutex_unlock(&mtx_busy);
  return (retval);
}

int
main(int argc, char **argv)
{
  struct input *pw_input = init_input(1,7,20,"PASSWORD",128,COLOR_PAIR(3));
  struct anic *heartbeat = init_anic(2,2,A_BOLD | COLOR_PAIR(3));
  struct anic *a = init_anic(4,4,0);
  struct anic *b = init_anic(6,6,COLOR_PAIR(1));
  a->state = '-';
  b->state = '\\';
  char key = '\0';

  if (sem_init(&sem_rdy, 0, 0) == -1) {
    perror("init semaphore");
    exit(1);
  }
  init_ui();
  register_anic(heartbeat);
  register_anic(a); register_anic(b);
  register_input(NULL, pw_input);
  activate_input(wnd_main, pw_input);
  if (run_ui_thrd() != 0) {
    exit(EXIT_FAILURE);
  }
  sem_wait(&sem_rdy);
  while ((key = wgetch(wnd_main)) != '\0' && process_key(key, pw_input, wnd_main) == true) {
    pthread_mutex_lock(&mtx_busy);
    do_ui_update(false);
    activate_input(wnd_main, pw_input);
    pthread_mutex_unlock(&mtx_busy);
  }
  stop_ui_thrd();
  unregister_ui_elt(a);
  unregister_ui_elt(heartbeat);
  unregister_ui_elt(b);
  unregister_ui_elt(pw_input);
  free_input(pw_input);
  free_anic(heartbeat);
  free_anic(a); free_anic(b);
  free_ui();
  return (0);
}
