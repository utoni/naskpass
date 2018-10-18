#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "ui.h"
#include "ui_ipc.h"
#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui_txtwindow.h"
#include "ui_nask.h"

#include "utils.h"
#include "status.h"

#define APP_TIMEOUT 60
#define APP_TIMEOUT_FMT "%02d"
#define NETUPD_INTERVAL 5
#define NETUPD_STRLEN 128
#define BSTR_LEN 3
#define PASSWD_WIDTH 35
#define PASSWD_HEIGHT 5
#define PASSWD_XRELPOS (unsigned int)(PASSWD_WIDTH / 2) - (PASSWD_WIDTH / 6)
#define PASSWD_YRELPOS (unsigned int)(PASSWD_HEIGHT / 2) + 1
#define INFOWND_WIDTH 25
#define INFOWND_HEIGHT 1
#define INFOWND_XRELPOS (unsigned int)(INFOWND_WIDTH / 2) - (INFOWND_WIDTH / 6)
#define INFOWND_YRELPOS (unsigned int)(INFOWND_HEIGHT / 2) + 1

#define STRLEN(s) (sizeof(s)/sizeof(s[0]))


static struct input *pw_input;
static struct anic *heartbeat;
static struct statusbar *higher, *lower, *netinfo, *uninfo;
static struct txtwindow *busywnd, *errwnd;
static unsigned int atmout = APP_TIMEOUT;
static unsigned int netupd = 0;
static char *title = NULL;
static char *untext = NULL;
static char busy_str[BSTR_LEN+1] = ".\0\0\0";


static int
lower_statusbar_update(WINDOW *win, struct statusbar *bar, bool ui_timeout)
{
  if (ui_timeout == FALSE) return DOUI_OK;
  char *tmp = get_system_stat();
  set_statusbar_text(bar, tmp);
  free(tmp);
  return DOUI_OK;
}

static int
higher_statusbar_update(WINDOW *win, struct statusbar *bar, bool ui_timeout)
{
  return DOUI_OK;
}

static int
netinfo_statusbar_update(WINDOW *win, struct statusbar *bar, bool ui_timeout)
{
  if (ui_timeout == TRUE) {
    if (netupd == 0) {
      netupd = NETUPD_INTERVAL;
      size_t len = 0;
      char buf[NETUPD_STRLEN+1];
      char *dev, *gwIp, *myIp;
      memset(buf, '\0', NETUPD_STRLEN+1);
      if (utGetDefaultGwInfo(&dev, &gwIp) == 0) {
        if (utGetIpFromNetDev(dev, &myIp) == 0) {
#ifdef HAVE_RESOLVE
          char *dom, *srv;
          if (utGetDomainInfo(&dom, &srv) == 0) {
            snprintf(buf, NETUPD_STRLEN, "netdev: %s // address: %s // gateway: %s // dns: %s // domain: %s", dev, myIp, gwIp, srv, dom);
          }
          free(dom);
          free(srv);
#else
          snprintf(buf, NETUPD_STRLEN, "netdev: %s // address: %s // gateway: %s", dev, myIp, gwIp);
#endif
          free(myIp);
        }
        free(dev);
        free(gwIp);
      }
      set_statusbar_text(bar, buf);
    } else {
      netupd--;
    }
  }
  return DOUI_OK;
}

static int
uninfo_statusbar_update(WINDOW *win, struct statusbar *bar, bool ui_timeout)
{
  if (untext == NULL) {
#ifdef HAVE_UNAME
    char *sysop;
    char *sysrelease;
    char *sysmachine;

    if (utGetUnameInfo(&sysop, &sysrelease, &sysmachine) == 0) {
      int ret = asprintf(&untext, "%s v%s (%s)", sysop, sysrelease, sysmachine);
      free(sysop);
      free(sysrelease);
      free(sysmachine);
      if (ret < 0) {
        return UICB_ERR_BUF;
      }
    } else
#endif
    if (asprintf(&untext, "%s", "[unknown kernel]") < 0) {
      return UICB_ERR_BUF;
    }

    set_statusbar_text(bar, untext);
  }
  return DOUI_OK;
}

static int
busywnd_update(WINDOW *win, struct txtwindow *tw, bool ui_timeout)
{
  if (ui_timeout == TRUE && tw->active == TRUE) {
    size_t len = strlen(busy_str);
    if (len == 0) {
      strcat(busy_str, ".");
    } else if (len >= BSTR_LEN) {
      memset(busy_str, '\0', BSTR_LEN+1);
    } else strcat(busy_str, ".");
    mvprintw(tw->y, tw->x + get_txtwindow_textlen(0, tw) + 1, busy_str);
  }
  return DOUI_OK;
}

static void
show_info_wnd(struct txtwindow *wnd, char *_title, char *text, chtype fore, chtype back, bool activate, bool blink)
{
  ui_thrd_suspend();
  set_txtwindow_active(wnd, activate);
  set_txtwindow_blink(wnd, blink);
  set_txtwindow_color(wnd, fore, back);
  set_txtwindow_title(wnd, _title);
  set_txtwindow_text(wnd, text);
  ui_thrd_resume();
  ui_thrd_force_update(false,false);
}

static int
passwd_input_cb(WINDOW *wnd, void *data, int key)
{
  struct input *a = (struct input *) data;
  char ipc_buf[IPC_MQSIZ+1];

  memset(ipc_buf, '\0', IPC_MQSIZ+1);
  switch (key) {
    case UIKEY_ENTER:
      ui_thrd_suspend();
      memset(busy_str, '\0', BSTR_LEN+1);
      ui_ipc_msgsend(MQ_PW, a->input);
      clear_input(wnd, a);
      deactivate_input(pw_input);
      ui_thrd_resume();

      ui_ipc_msgrecv(MQ_IF, ipc_buf, 3);
      show_info_wnd(busywnd, "BUSY", ipc_buf, COLOR_PAIR(5), COLOR_PAIR(5), true, false);
      sleep(3);

      if (ui_ipc_getvalue(SEM_UI) > 0 &&
        ui_ipc_msgrecv(MQ_IF, ipc_buf, 3) > 0)
      {
        show_info_wnd(errwnd, "ERROR", ipc_buf, COLOR_PAIR(4), COLOR_PAIR(4), true, true);
        while (ui_wgetchtest(1500, '\n') != DOUI_KEY) { };
      }

      ui_thrd_suspend();
      set_txtwindow_active(busywnd, false);
      set_txtwindow_active(errwnd, false);
      activate_input(pw_input);
      ui_ipc_msgclear(MQ_IF);
      ui_thrd_resume();
      break;
    case UIKEY_BACKSPACE:
      del_input(wnd, a);
      break;
    case UIKEY_ESC:
      ui_thrd_suspend();
      clear_input(wnd, a);
      deactivate_input(pw_input);
      ui_thrd_resume();
      show_info_wnd(errwnd, "QUIT", "bye bye", COLOR_PAIR(5), COLOR_PAIR(5), true, true);
      sleep(2);
      return DOUI_ERR;
    case UIKEY_DOWN:
    case UIKEY_UP:
    case UIKEY_LEFT:
    case UIKEY_RIGHT:
      break;
    case UIKEY_ACTIVATE:
      break;
    default:
      ui_thrd_suspend();
      add_input(wnd, a, key);
      ui_thrd_resume();
  }
  return DOUI_OK;
}

static void
init_ui_elements(unsigned int max_x, unsigned int max_y)
{
  assert(asprintf(&title, "/* %s-%s */", PKGNAME, VERSION) > 0);
  pw_input  = init_input((unsigned int)(max_x / 2)-PASSWD_XRELPOS,
                         (unsigned int)(max_y / 2)-PASSWD_YRELPOS,
                         PASSWD_WIDTH, "PASSWORD: ",
                         IPC_MQSIZ, COLOR_PAIR(3), COLOR_PAIR(2));
  heartbeat = init_anic_default(0, 0, A_BOLD | COLOR_PAIR(1), "[%c]");
  higher    = init_statusbar(0, max_x, A_BOLD | COLOR_PAIR(3),
                             higher_statusbar_update);
  lower     = init_statusbar(max_y - 1, max_x, COLOR_PAIR(3),
                             lower_statusbar_update);
  netinfo   = init_statusbar(2, max_x, COLOR_PAIR(2),
                             netinfo_statusbar_update);
  uninfo    = init_statusbar(1, max_x, COLOR_PAIR(2),
                             uninfo_statusbar_update);
  busywnd   = init_txtwindow_centered(INFOWND_WIDTH, INFOWND_HEIGHT,
                             busywnd_update);
  errwnd    = init_txtwindow_centered(INFOWND_WIDTH, INFOWND_HEIGHT,
                             NULL);

  register_input(NULL, pw_input, passwd_input_cb);
  register_statusbar(higher);
  register_statusbar(lower);
  register_statusbar(netinfo);
  register_statusbar(uninfo);
  register_anic_default(heartbeat);
  register_txtwindow(busywnd);
  register_txtwindow(errwnd);
  activate_input(pw_input);
  set_statusbar_text(higher, title);
}

static void
free_ui_elements(void)
{
  unregister_ui_elt(lower);
  unregister_ui_elt(higher);
  unregister_ui_elt(netinfo);
  unregister_ui_elt(uninfo);
  unregister_ui_elt(heartbeat);
  unregister_ui_elt(pw_input);
  free_input(pw_input);
  free_anic_default(heartbeat);
  free_statusbar(higher);
  free_statusbar(lower);
  free_statusbar(netinfo);
  free_statusbar(uninfo);
  free_txtwindow(busywnd);
  free_txtwindow(errwnd);
  free_ui();
  if (title) {
    free(title);
    title = NULL;
  }
}

static int
on_update_cb(bool timeout)
{
  if (!timeout) {
    atmout = APP_TIMEOUT;
  } else if (atmout > 0) {
    atmout--;
  } else if (atmout == 0) {
    ui_ipc_semtrywait(SEM_UI);
  }

  if ( ui_ipc_getvalue(SEM_IN) <= 0 ) {
    attron(COLOR_PAIR(4));
    const char msg[] = "Got a piped password ..";
    mvprintw((unsigned int)(ui_get_maxy() / 2)-PASSWD_YRELPOS-4,
             (unsigned int)(ui_get_maxx() / 2)-PASSWD_XRELPOS+(strlen(msg)/2),
             msg);
    attroff(COLOR_PAIR(4));
  }
  return UICB_OK;
}

static int
on_postupdate_cb(bool timeout)
{
  attron(COLOR_PAIR(1));
  mvprintw(0, (unsigned int)(ui_get_maxx() - STRLEN(APP_TIMEOUT_FMT)), "[" APP_TIMEOUT_FMT "]", atmout);
  attroff(COLOR_PAIR(1));
  return UICB_OK;
}

int
do_ui(void)
{
  char key = '\0';
  int ret = DOUI_ERR;

  /* init TUI and UI Elements (input field, status bar, etc) */
  if (init_ui(on_update_cb, on_postupdate_cb))
    init_ui_elements(ui_get_maxx(), ui_get_maxy());
  else
    return DOUI_ERR;

  /* some color definitions */
  init_pair(1, COLOR_RED, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  init_pair(3, COLOR_BLACK, COLOR_WHITE);
  /* TXTwindow */
  init_pair(4, COLOR_YELLOW, COLOR_RED);
  init_pair(5, COLOR_WHITE, COLOR_CYAN);

  ui_thrd_suspend();
  if (run_ui_thrd() != 0) {
    ui_thrd_resume();
    return ret;
  }
  timeout(0);
  ui_thrd_resume();

  while ( ui_ipc_getvalue(SEM_UI) > 0 ) {
    if ( (key = ui_wgetch(10000)) == ERR )
      continue;

    if ( process_key(key) != true ) {
      ui_ipc_semtrywait(SEM_UI);
    }

    ui_thrd_force_update(true,false);
  }
  stop_ui_thrd();
  free_ui_elements();

  return DOUI_OK;
}
