#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <ncurses.h>

#define PKGNAME "nask"
#define LOGLEN 128
#define DEFWIN stdscr
#define MAXINPUT 40
#define MAXPASS 64
#define MAXTRIES 3
#define SRELPOSY -4

#define RET_OK 0
#define RET_FORK -1
#define RET_SHELL 127
#define RET_DENIED 512
#define RET_BUSY 1280

static char debug_msg[LOGLEN+1];

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

  mvhline(starty-2, startx-2, 0, len+addwidth+3);
  mvhline(starty+2, startx-2, 0, len+addwidth+3);
  mvvline(starty-1, startx-3, 0, 3);
  mvvline(starty-1, startx+len+addwidth+1, 0, 3);
  mvaddch(starty-2, startx-3, ACS_ULCORNER);
  mvaddch(starty+2, startx-3, ACS_LLCORNER);
  mvaddch(starty-2, startx+len+addwidth+1, ACS_URCORNER);
  mvaddch(starty+2, startx+len+addwidth+1, ACS_LRCORNER);
  mvwprintw(wnd, starty, startx, text);
}

static size_t print_pw_status(WINDOW *wnd, char *text)
{
  int startx, starty;
  size_t len = strlen(text);

  getwmaxyx(wnd, 0, SRELPOSY, len+4, &starty, &startx);
  attron(A_BLINK);
  mvwprintw(wnd, starty, startx-2, "< ");
  mvwprintw(wnd, starty, startx+len, " >");
  attroff(A_BLINK);
  attron(A_BOLD | COLOR_PAIR(1));
  mvwprintw(wnd, starty, startx, "%s", text);
  attroff(A_BOLD | COLOR_PAIR(1));
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

  getyx(wnd, cury, curx);
  getwmaxyx(wnd, 0, SRELPOSY, len+4, &starty, &startx);
  mvwprintw(wnd, starty, startx-2, "%s", buf);
  wmove(wnd, cury, curx);
}

static void usage()
{
  fprintf(stderr, PKGNAME ": [cryptcreate]\n");
}

int main(int argc, char **argv)
{
  WINDOW *win;
  int ret, ch, curx, cury, pidx, iidx, tries = MAXTRIES;
  size_t slen = 0;
  char pass[MAXPASS+1];
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
again:
  clear();
  memset(debug_msg, 0, LOGLEN+1);
  memset(pass, 0, MAXPASS+1);
  pidx = 0;
  iidx = 0;
  print_rel_to_wnd(DEFWIN, 0, 0, "PASSWORD: ", MAXINPUT+5);
  while ( (ch = wgetch(win)) != '\n') {
    getyx(DEFWIN, cury, curx);
    clear_pw_status(DEFWIN, slen);
    if (ch == KEY_BACKSPACE) {
      pass[pidx] = '\0';
      if (pidx <= 0) continue;
      if (iidx == pidx) {
        mvwprintw(DEFWIN, cury, curx - 1, " ");
        mvwprintw(DEFWIN, cury, curx + 2, " ");
        wmove(DEFWIN, cury, curx - 1);
        iidx--;
      }
      pidx--;
    } else if (pidx <= MAXPASS) {
      if (iidx <= MAXINPUT) {
        wprintw(DEFWIN, "*");
        iidx++;
      } else {
        mvwprintw(DEFWIN, cury, curx + 2, ">");
        wmove(DEFWIN, cury, curx);
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
  endwin_and_print_debug();
  return (EXIT_SUCCESS);
}
