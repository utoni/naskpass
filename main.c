#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ncurses.h>
#include <sys/sysinfo.h>

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

static char debug_msg[LOGLEN+1], pass[MAXPASS+1];
static pthread_t thrd;
static char anic = '\0';
static int ptime, atime, pidx, iidx;
pthread_mutex_t tmretmtx = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ncbsy = PTHREAD_MUTEX_INITIALIZER;

static void set_logmsg(char *fmt, ...)
{
  va_list ap;

  memset(debug_msg, 0, LOGLEN+1);
  va_start(ap, fmt);
  vsnprintf(debug_msg, LOGLEN, fmt, ap);
  va_end(ap);
}

static void endwin_and_print_debug(void)
{
  clear();
  endwin();
  if (strnlen(debug_msg, LOGLEN) > 0)
    fprintf(stderr, "%s: %s\n", PKGNAME, debug_msg);
}

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
  pthread_mutex_lock(&ncbsy);
}

static void clear_pw_input(WINDOW *wnd)
{
  clear();
  memset(debug_msg, 0, LOGLEN+1);
  memset(pass, 0, MAXPASS+1);
  pidx = 0;
  iidx = 0;
  curs_set(1);
  print_rel_to_wnd(DEFWIN, 0, 0, PWSTR, MAXINPUT+5);
}

static void usage(void)
{
  fprintf(stderr, PKGNAME ": [cryptcreate]\n");
}

static char animate_heartbeat(char c)
{
  switch (c) {
    default:
    case '|': return ('/');
    case '/': return ('-');
    case '-': return ('\\');
    case '\\': return ('|');
  }
}

static char *get_system_stat(void)
{
  char *retstr = NULL;
  int ncpu;
  struct sysinfo inf;

  if (sysinfo(&inf) == EFAULT) {
    return ("[SYSINFO ERROR]");
  }
  ncpu = get_nprocs();

  if (asprintf(&retstr, "u:%04ld - l:%3.2f,%3.2f,%3.2f - %dcore%s - mem:%lu/%lumb - procs:%02d", 
    inf.uptime, ((float)inf.loads[0]/10000), ((float)inf.loads[1]/10000), ((float)inf.loads[2]/10000),
    ncpu, (ncpu > 1 ? "s" : ""),
    (unsigned long)((inf.freeram/1024)/1024), (unsigned long)((inf.totalram/1024)/1024), inf.procs) == -1) {
    return ("[ASPRINTF ERROR]");
  }
  return (retstr);
}

static void timer_func(void) {
  pthread_mutex_lock(&tmretmtx);
  if ( (ptime -= 1) <= 0) {
    clear_pw_input(DEFWIN);
  }
  if ( (atime -= 1) <= 0 ) {
    set_logmsg("APP TIMEOUT - system('%s')", SHTDWN_CMD);
    endwin_and_print_debug();
    system(SHTDWN_CMD);
  }
  pthread_mutex_unlock(&tmretmtx);
}

static void timer_reset(void) {
  pthread_mutex_lock(&tmretmtx);
  ptime = PWTIMEOUT;
  atime = APPTIMEOUT;
  pthread_mutex_unlock(&tmretmtx);
}

static void print_status_line(void) {
  char *statln;
  int maxx, maxy, curx, cury, i;

  pthread_mutex_lock(&ncbsy);
  statln = get_system_stat();
  getmaxyx(stdscr, maxy, maxx);
  getyx(stdscr, cury, curx);
  attron(A_BOLD | COLOR_PAIR(2));
  for (i = 0; i < maxx; i++) {
    mvaddch(maxy-1, i, ' ');
  }
  attroff(A_BOLD | COLOR_PAIR(2));
  attron(COLOR_PAIR(3));
  mvprintw(maxy-1, 1, "[F1] clearpw %c %s", (anic = animate_heartbeat(anic)), statln);
  attroff(COLOR_PAIR(3));

  wmove(stdscr, cury, curx);
  refresh();
  free(statln);
  pthread_mutex_unlock(&ncbsy);
}

static void *status_thrd(void *arg) {
  timer_reset();
  while (1) {
    sleep(1); // TODO: wait (if gobal variable is set to 1) until main thread sends signal or 1 second is over
    timer_func(); // should be run every second
    print_status_line();
  }
  return (NULL);
}

static int run_status_thrd(void) {
  init_pair(2, COLOR_WHITE, COLOR_WHITE);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);
  return (pthread_create(&thrd, NULL, &status_thrd, NULL));
}

int main(int argc, char **argv)
{
  WINDOW *win;
  int ret, ch, curx, cury, tries = MAXTRIES;
  size_t slen = 0;
  char *cmd = NULL;

  if (argc != 2) {
    usage();
    exit(-1);
  }

  win = initscr();
  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  raw();
  keypad(DEFWIN, TRUE);
  noecho();
  cbreak();
  if (run_status_thrd() != 0) {
    endwin_and_print_debug();
    set_logmsg("Error while running status thread(%d): %s\n", errno, strerror(errno));
    exit(EXIT_FAILURE);
  }
again:
  timer_reset();
  clear_pw_input(DEFWIN);
  print_rel_to_wnd(DEFWIN, 0, 0, PWSTR, MAXINPUT+5);
  print_status_line();
  while ( (ch = wgetch(win)) != '\n') {
    timer_reset();
    getyx(DEFWIN, cury, curx);
    clear_pw_status(DEFWIN, slen);
    if (ch == KEY_F(1)) {
      __fpurge(stdin);
      goto again;
    } else if (ch == KEY_BACKSPACE) {
      pass[pidx] = '\0';
      if (pidx <= 0) continue;
      if (iidx == pidx) {
        pthread_mutex_lock(&ncbsy);
        mvwprintw(DEFWIN, cury, curx - 1, " ");
        mvwprintw(DEFWIN, cury, curx + 2, " ");
        wmove(DEFWIN, cury, curx - 1);
        iidx--;
        pthread_mutex_unlock(&ncbsy);
      }
      pidx--;
    } else if (pidx <= MAXPASS) {
      if (iidx <= MAXINPUT) {
        pthread_mutex_lock(&ncbsy);
        wprintw(DEFWIN, "*");
        iidx++;
        pthread_mutex_unlock(&ncbsy);
      } else {
        pthread_mutex_lock(&ncbsy);
        mvwprintw(DEFWIN, cury, curx + 2, ">");
        wmove(DEFWIN, cury, curx);
        pthread_mutex_unlock(&ncbsy);
      }
      pass[pidx] = ch;
      pidx++;
    } else {
      slen = print_pw_status(DEFWIN, "PASSWD TOO LONG");
      wmove(DEFWIN, cury, curx);
    }
  }

  asprintf(&cmd, "echo '%s' | %s 2>/dev/null >/dev/null", pass, argv[1]);
  switch ((ret = system(cmd))) {
    case RET_OK:
      set_logmsg("success");
      break;
    case RET_BUSY:
      set_logmsg("device is busy");
      break;
    case RET_SHELL:
      set_logmsg("shell error");
      break;
    case RET_FORK:
      set_logmsg("fork error");
      break;
    case RET_DENIED:
      free(cmd);
      cmd = NULL;
      clear_pw_status(DEFWIN, slen);
      slen = print_pw_status(DEFWIN, "ACCESS DENIED");
      tries--;
      wgetch(win);
      if (tries == 0) {
        set_logmsg("ACCESS DENIED: HDD is encrypted, can not boot");
        endwin_and_print_debug();
        break;
      }
      goto again;
    default:
      set_logmsg("unknown error(%d)", ret);
      break;
  }
  free(cmd);
  pthread_kill(thrd, SIGKILL);
  pthread_join(thrd, NULL);
  endwin_and_print_debug();
  return (EXIT_SUCCESS);
}
