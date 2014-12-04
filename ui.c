#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ncurses.h>
#include <time.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

#include "ui.h"

#define PKGNAME "nask"
#define LOGLEN 128
#define DEFWIN stdscr
#define PWSTR "PASSWORD: "
#define MAXINPUT 40
#define MAXPASS 64
#define MAXTRIES 3
#define SRELPOSY -4
#define PWTIMEOUT 10
#define APPTIMEOUT 600
#define SHTDWN_CMD "echo 'o' >/proc/sysrq-trigger"

#define RET_OK 0
#define RET_FORK -1
#define RET_SHELL 127
#define RET_DENIED 512
#define RET_BUSY 1280


static WINDOW *wnd_main;
static struct nask_ui *nui = NULL;
static pthread_t thrd;
static bool active;
static pthread_cond_t cnd_update = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mtx_update = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t tmretmtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ncbsy = PTHREAD_MUTEX_INITIALIZER;


static void getwmaxyx(WINDOW *wnd, int x, int y, size_t len, int *startyp, int *startxp)
{
  int maxy, maxx;

  getmaxyx(wnd, maxy, maxx);
  *startxp = ( ((int)(maxx/2)) - x - ((int)(len/2)));
  *startyp = ( ((int)(maxy/2)) - y);
}

static void print_rel_to_wnd(WINDOW *wnd, int x, int y, char *text, size_t addwidth)
{
  int startx, starty;
  size_t len = strlen(text);

  getwmaxyx(wnd, x, y, len+addwidth, &starty, &startx);

  pthread_mutex_lock(&ncbsy);
  mvhline(starty-2, startx-2, 0, len+addwidth+3);
  mvhline(starty+2, startx-2, 0, len+addwidth+3);
  mvvline(starty-1, startx-3, 0, 3);
  mvvline(starty-1, startx+len+addwidth+1, 0, 3);
  mvaddch(starty-2, startx-3, ACS_ULCORNER);
  mvaddch(starty+2, startx-3, ACS_LLCORNER);
  mvaddch(starty-2, startx+len+addwidth+1, ACS_URCORNER);
  mvaddch(starty+2, startx+len+addwidth+1, ACS_LRCORNER);
  mvwprintw(wnd, starty, startx, text);
  pthread_mutex_unlock(&ncbsy);
}

static size_t print_pw_status(WINDOW *wnd, char *text)
{
  int startx, starty;
  size_t len = strlen(text);

  curs_set(0);
  getwmaxyx(wnd, 0, SRELPOSY, len+4, &starty, &startx);
  pthread_mutex_lock(&ncbsy);
  attron(A_BLINK);
  mvwprintw(wnd, starty, startx-2, "< ");
  mvwprintw(wnd, starty, startx+len, " >");
  attroff(A_BLINK);
  attron(A_BOLD | COLOR_PAIR(1));
  mvwprintw(wnd, starty, startx, "%s", text);
  attroff(A_BOLD | COLOR_PAIR(1));
  pthread_mutex_unlock(&ncbsy);
  return (strlen(text));
}

static void clear_pw_status(WINDOW *wnd, size_t len)
{
  int startx, starty, curx, cury;
  char buf[len+5];
  if (len <= 0) return;

  attroff(A_BLINK | A_BOLD | COLOR_PAIR(1));
  memset(buf, ' ', len+4);
  buf[len+4] = '\0';

  pthread_mutex_lock(&ncbsy);
  getyx(wnd, cury, curx);
  getwmaxyx(wnd, 0, SRELPOSY, len+4, &starty, &startx);
  mvwprintw(wnd, starty, startx-2, "%s", buf);
  wmove(wnd, cury, curx);
  curs_set(1);
  pthread_mutex_unlock(&ncbsy);
}

void register_ui_elt(ui_callback uicb, void *data)
{
  struct nask_ui *tmp;

  if (nui == NULL) {
    nui = calloc(1, sizeof(struct nask_ui));
    nui->next = NULL;
  }
  tmp = nui;
  while (tmp->next != NULL) {
   tmp = tmp->next;
  }
  tmp->next = calloc(1, sizeof(struct nask_ui));
  tmp->next->ui_elt_cb = uicb;
  tmp->next->do_update = true;
  tmp->next->data = data;
  tmp->next->next = NULL;
}

static void *ui_thrd(void *arg) {
  struct timeval now;
  struct timespec wait;

  pthread_mutex_lock(&mtx_update);
  gettimeofday(&now, NULL);
  wait.tv_sec = now.tv_sec + 1;
  wait.tv_nsec = now.tv_usec;
  while (active == true) {
    pthread_cond_timedwait(&cnd_update, &mtx_update, &wait);
    if (active == false) break;
  }
  pthread_mutex_unlock(&mtx_update);
  return (NULL);
}

int do_update(void) {
  
}

int run_ui_thrd(void) {
  active = true;
  return (pthread_create(&thrd, NULL, &ui_thrd, NULL));
}

int stop_ui_thrd(void) {
  active = false;
  return (pthread_join(thrd, NULL));
}

void init_ui(void)
{
  wnd_main = initscr();
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_WHITE);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);
  raw();
  keypad(DEFWIN, TRUE);
  noecho();
  cbreak();
}

void free_ui(void)
{
  endwin();
  clear();
}

int main(int argc, char **argv)
{
  init_ui();

  if (run_ui_thrd() != 0) {
    exit(EXIT_FAILURE);
  }
  stop_ui_thrd();

  free_ui();
  return (0);
}
