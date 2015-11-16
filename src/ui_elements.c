#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "ui.h"
#include "ui_ipc.h"
#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui_txtwindow.h"
#include "ui_elements.h"

#include "status.h"

#define PASSWD_WIDTH 35
#define PASSWD_HEIGHT 5
#define PASSWD_XRELPOS (unsigned int)(PASSWD_WIDTH / 2) - (PASSWD_WIDTH / 6)
#define PASSWD_YRELPOS (unsigned int)(PASSWD_HEIGHT / 2) + 1
#define INFOWND_WIDTH 25
#define INFOWND_HEIGHT 1

static struct input *pw_input;
static struct anic *heartbeat;
static struct statusbar *higher, *lower;
static struct txtwindow *infownd;
static char *title = NULL;


static int
lower_statusbar_update(WINDOW *win, struct statusbar *bar)
{
  char *tmp = get_system_stat();
  set_statusbar_text(bar, tmp);
  free(tmp);
  return 0;
}

static int
higher_statusbar_update(WINDOW *win, struct statusbar *bar)
{
  return 0;
}

static int
infownd_update(WINDOW *win, struct txtwindow *tw)
{
  char *tmp = (char*)(tw->userptr);
  size_t len = strlen(tmp);

  if (tw->active) {
    if ( len == 3 ) {
      memset(tmp+1, '\0', 2);
    } else strcat(tmp, ".");
  } else (*tmp) = '.';
  return 0;
}

static int
mq_passwd_send(char *passwd)
{
  int ret;

  ui_ipc_sempost(SEM_IN);
  ret = ui_ipc_msgsend(MQ_PW, passwd);
  return ret;
}

static int
passwd_input_cb(WINDOW *wnd, void *data, int key)
{
  struct input *a = (struct input *) data;
  char ipc_buf[IPC_MQSIZ+1];

  memset(ipc_buf, '\0', IPC_MQSIZ+1);
  wtimeout(stdscr, -1);
  switch (key) {
    case UIKEY_ENTER:
      mq_passwd_send(a->input);
      ui_thrd_suspend();
      clear_input(wnd, a);
      deactivate_input(pw_input);
      ui_ipc_msgrecv(MQ_IF, ipc_buf);
      set_txtwindow_color(infownd, COLOR_PAIR(5), COLOR_PAIR(5));
      set_txtwindow_title(infownd, "BUSY");
      set_txtwindow_text(infownd, ipc_buf);
      set_txtwindow_active(infownd, true);
      ui_thrd_resume();
      sleep(2);
      ui_ipc_msgrecv(MQ_IF, ipc_buf);
      ui_thrd_suspend();
      set_txtwindow_color(infownd, COLOR_PAIR(4), COLOR_PAIR(4) | A_BOLD);
      set_txtwindow_title(infownd, "ERROR");
      set_txtwindow_text(infownd, ipc_buf);
      ui_thrd_resume();
//      while (getch() != '\n') { }
wgetch(stdscr);
      ui_thrd_suspend();
      set_txtwindow_active(infownd, false);
      activate_input(pw_input);
      ui_thrd_resume();
      break;
    case UIKEY_BACKSPACE:
      del_input(wnd, a);
      ui_thrd_force_update();
      break;
    case UIKEY_ESC:
      return DOUI_ERR;
    case UIKEY_DOWN:
    case UIKEY_UP:
    case UIKEY_LEFT:
    case UIKEY_RIGHT:
      break;
    case UIKEY_ACTIVATE:
      break;
    default:
      add_input(wnd, a, key);
      ui_thrd_force_update();
  }
  wtimeout(stdscr, 1000);
  return DOUI_OK;
}

void
init_ui_elements(WINDOW *wnd_main, unsigned int max_x, unsigned int max_y)
{
  asprintf(&title, "/* %s-%s */", PKGNAME, VERSION);
  pw_input = init_input((unsigned int)(max_x / 2)-PASSWD_XRELPOS,
                        (unsigned int)(max_y / 2)-PASSWD_YRELPOS,
                        PASSWD_WIDTH, "PASSWORD: ",
                        IPC_MQSIZ, COLOR_PAIR(3), COLOR_PAIR(2));
  heartbeat = init_anic_default(0, 0, A_BOLD | COLOR_PAIR(1), "[%c]");
  higher = init_statusbar(0, max_x, A_BOLD | COLOR_PAIR(3),
                          higher_statusbar_update);
  lower = init_statusbar(max_y - 1, max_x, COLOR_PAIR(3),
                         lower_statusbar_update);
  infownd = init_txtwindow_centered(INFOWND_WIDTH, INFOWND_HEIGHT,
                                    infownd_update);
  infownd->userptr = calloc(4, sizeof(char));
  (*(char*)(infownd->userptr)) = '.';

  register_input(NULL, pw_input, passwd_input_cb);
  register_statusbar(higher);
  register_statusbar(lower);
  register_anic_default(heartbeat);
  register_txtwindow(infownd);
  activate_input(pw_input);
  set_statusbar_text(higher, title);
}

void
free_ui_elements(void)
{
  unregister_ui_elt(lower);
  unregister_ui_elt(higher);
  unregister_ui_elt(heartbeat);
  unregister_ui_elt(pw_input);
  free_input(pw_input);
  free_anic_default(heartbeat);
  free_statusbar(higher);
  free_statusbar(lower);
  free(infownd->userptr);
  free_txtwindow(infownd);
  free_ui();
  if (title) {
    free(title);
    title = NULL;
  }
}
