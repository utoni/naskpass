#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_input.h"


struct input *
init_input(unsigned int x, unsigned int y, unsigned int width, char *prompt, size_t input_len, chtype attrs, chtype shadow)
{
  struct input *a = calloc(1, sizeof(struct input));

  a->x = x;
  a->y = y;
  a->width = width;
  a->cur_pos = 0;
  a->input = calloc(input_len+1, sizeof(char));
  a->input_max = input_len;
  a->input_len = 0;
  a->input_pos = 0;
  a->prompt = strdup(prompt);
  a->attrs = attrs;
  a->shadow = shadow;
  return (a);
}

void
free_input(struct input *a)
{
  if (a->input != NULL) {
    free(a->input);
  }
  free(a->prompt);
  free(a);
}

static void
print_wnd(size_t addwidth, struct input *a)
{
  int i, x = a->x, y = a->y;
  size_t relwidth = addwidth*2, len = strlen(a->prompt) + a->width;
  char tmp[len+relwidth+1];

  attron(a->attrs);
  memset(tmp, ' ', len+relwidth);
  tmp[len+relwidth] = '\0';
  for (i = -1; i <= 1; i++)
    mvprintw(y+i, x-addwidth, tmp);

  mvhline(y-2, x-addwidth, 0, len+relwidth);
  mvhline(y+2, x-addwidth, 0, len+relwidth);
  mvvline(y-1, x-addwidth-1, 0, 3);
  mvvline(y-1, x+len+addwidth, 0, 3);
  mvaddch(y-2, x-addwidth-1, ACS_ULCORNER);
  mvaddch(y+2, x-addwidth-1, ACS_LLCORNER);
  mvaddch(y-2, x+len+addwidth, ACS_URCORNER);
  mvaddch(y+2, x+len+addwidth, ACS_LRCORNER);
  attroff(a->attrs);

  attron(a->shadow);
  for (i = x-addwidth+1; i < x+len+relwidth; i++)
    mvaddch(y+3, i, ACS_CKBOARD);
  for (i = -1; i < 3; i++) {
    mvaddch(y+i, x+len+relwidth-2, ACS_CKBOARD);
    mvaddch(y+i, x+len+relwidth-1, ACS_CKBOARD);
  }
  attroff(a->shadow);
}

static void
print_input_text(WINDOW *win, struct input *a)
{
  size_t start = 0;
  size_t p_len = strlen(a->prompt);

  char tmp[a->width + 1];
  memset(tmp, '\0', a->width + 1);
  if (a->input_pos >= a->width) {
    start = a->input_pos - a->width;
  }

  strncpy(tmp, (char *)(a->input + start), a->width);
  int i;
  for (i = 0; i < strlen(tmp); i++) {
    tmp[i] = '*';
  }

  if (win == NULL) {
    mvprintw(a->y, a->x + p_len, "%s", tmp);
  } else {
    mvwprintw(win, a->y, a->x + p_len, "%s", tmp);
  }
}

static void
print_input(WINDOW *win, struct input *a)
{
  char *tmp;
  int i;
  size_t p_len = strlen(a->prompt);

  print_wnd(3, a);
  attron(a->attrs);
  if (win) {
    mvwprintw(win, a->y, a->x, a->prompt);
  } else {
    mvprintw(a->y, a->x, a->prompt);
  }
  tmp = calloc(a->width+1, sizeof(char));
  for (i = 0; i < a->width; i++) {
    *(tmp + i) = '_';
  }
  if (win) {
    mvwprintw(win, a->y, a->x + p_len, tmp);
  } else {
    mvprintw(a->y, a->x + p_len, tmp);
  }
  free(tmp);
  print_input_text(win, a);
  attroff(a->attrs);
}

int
activate_input(WINDOW *win, struct input *a)
{
  if (a == NULL) return (UICB_ERR_UNDEF);
  size_t p_len = strlen(a->prompt);
  if (win == NULL) {
    move(a->y, a->x + p_len + a->cur_pos);
  } else {
    wmove(win, a->y, a->x + p_len + a->cur_pos);
  }
  return (activate_ui_input( (void *) a )); 
}

int
add_input(WINDOW *win, struct input *a, int key)
{
  if (a == NULL) return (UICB_ERR_UNDEF);
  if (a->input_len >= a->input_max) return (UICB_ERR_BUF);
  *(a->input + a->input_pos) = (char) key;
  ++a->input_pos;
  ++a->input_len;
  a->cur_pos = (a->cur_pos+1 < a->width ? a->cur_pos+1 : a->cur_pos);
  print_input(win, a);
  return (UICB_OK);
}

int
del_input(WINDOW *win, struct input *a)
{
  if (a == NULL) return (UICB_ERR_UNDEF);
  if (a->input_len == 0) return (UICB_ERR_BUF);
  memmove((a->input + a->input_pos - 1), (a->input + a->input_pos), a->input_max - a->input_pos);
  --a->input_len;
  *(a->input + a->input_len) = '\0';
  if (a->input_pos-1 == a->input_len) {
    --a->input_pos;
  }
  if (a->cur_pos+1 < a->width && a->cur_pos > 0) {
    --a->cur_pos;
  } else if (a->cur_pos-1 == a->input_pos) {
    --a->cur_pos;
  }
  mvwprintw(win, a->y, a->x + a->cur_pos + strlen(a->prompt), "_");
  print_input(win, a);
  return (UICB_OK);
}

static int
input_cb(WINDOW *win, void *data, bool timed_out)
{
  struct input *a = (struct input *) data;

  if (a == NULL) return (UICB_ERR_UNDEF);
  if (win != NULL && is_wintouched(win) == false) return (UICB_OK);
  print_input(win, a);
  return (UICB_OK);
}

void
register_input(WINDOW *win, struct input *a, uicb_input ipcb)
{
  struct ui_callbacks cbs;
  cbs.ui_element = input_cb;
  cbs.ui_input = ipcb;
  register_ui_elt(&cbs, (void *) a, win);
}

