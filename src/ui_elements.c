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
#define BSTR_LEN 3

static struct input *pw_input;
static struct anic *heartbeat;
static struct statusbar *higher, *lower;
static struct txtwindow *infownd;
static struct tctwindow *errwnd;
static char *title = NULL;
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
infownd_update(WINDOW *win, struct txtwindow *tw, bool ui_timeout)
{
  if (ui_timeout == TRUE && tw->active == TRUE) {
    size_t len = strlen(busy_str);
    if (len > BSTR_LEN) {
      memset(busy_str, '\0', BSTR_LEN+1);
      busy_str[0] = '.';
    } else strcat(busy_str, ".");
  }
  return DOUI_OK;
}

void
show_info_wnd(struct txtwindow *wnd, char *title, char *text, chtype fore, chtype back, bool activate, bool blink)
{
  ui_thrd_suspend();
  deactivate_input(pw_input);
  set_txtwindow_active(wnd, activate);
  set_txtwindow_blink(wnd, blink);
  set_txtwindow_color(wnd, fore, back);
  set_txtwindow_title(wnd, title);
  set_txtwindow_text(wnd, text);
  ui_thrd_resume();
  ui_thrd_force_update(false);
}

static int
passwd_input_cb(WINDOW *wnd, void *data, int key)
{
  struct input *a = (struct input *) data;
  char ipc_buf[IPC_MQSIZ+1];

  memset(ipc_buf, '\0', IPC_MQSIZ+1);
  switch (key) {
    case UIKEY_ENTER:
      ui_ipc_msgsend(MQ_PW, a->input);

      ui_thrd_suspend();
      clear_input(wnd, a);
      deactivate_input(pw_input);
      ui_thrd_resume();
      ui_thrd_force_update(false);

      ui_ipc_msgrecv(MQ_IF, ipc_buf);
      show_info_wnd(infownd, "BUSY", ipc_buf, COLOR_PAIR(5), COLOR_PAIR(5), true, false);

      sleep(2);

      if (ui_ipc_msgcount(MQ_IF) > 0) {
        ui_ipc_msgrecv(MQ_IF, ipc_buf);
        show_info_wnd(infownd, "ERROR", ipc_buf, COLOR_PAIR(4), COLOR_PAIR(4), true, true);
        while (ui_wgetch(1500) != '\n') { };
      }

      ui_thrd_suspend();
      set_txtwindow_active(infownd, false);
      activate_input(pw_input);
      ui_thrd_resume();
      ui_thrd_force_update(true);

      ui_ipc_sempost(SEM_IN);
      break;
    case UIKEY_BACKSPACE:
      del_input(wnd, a);
      break;
    case UIKEY_ESC:
      show_info_wnd(infownd, "BUSY", "bye bye", COLOR_PAIR(5), COLOR_PAIR(5), true, true);
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
      add_input(wnd, a, key);
  }
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
  free_txtwindow(infownd);
  free_ui();
  if (title) {
    free(title);
    title = NULL;
  }
}
