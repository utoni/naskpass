#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_input.h"


struct input *
init_input(unsigned int x, unsigned int y, unsigned int width, char *prompt, size_t input_len, chtype attrs)
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

  attron(a->attrs);
  if (win == NULL) {
    mvprintw(a->y, a->x, a->prompt);
    tmp = calloc(a->width+1, sizeof(char));
    for (i = 0; i < a->width; i++) {
      *(tmp + i) = '_';
    }
    mvprintw(a->y, a->x + p_len, tmp);
    free(tmp);
  } else {
  }
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
  return (UICB_OK);
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
  //mvwprintw(win, 10, 1, "w:%d,cp:%d,im:%lu,il:%lu,ip:%lu,s:%s", a->width, a->cur_pos, a->input_max, a->input_len, a->input_pos, a->input);
  return (UICB_OK);
}

int
del_input(WINDOW *win, struct input *a)
{
  if (a == NULL) return (UICB_ERR_UNDEF);
  memmove((a->input + a->input_pos - 1), (a->input + a->input_pos), a->input_max - a->input_pos);
  --a->input_len;
  if (a->input_pos == a->input_len) {
    --a->input_pos;
  }
  print_input(win, a);
  mvwprintw(win, 10, 1, "w:%d,cp:%d,im:%lu,il:%lu,ip:%lu,s:%s", a->width, a->cur_pos, a->input_max, a->input_len, a->input_pos, a->input);
  
  return (UICB_OK);
}

int
input_cb(WINDOW *win, void *data, bool timed_out)
{
  struct input *a = (struct input *) data;

  if (a == NULL) return (UICB_ERR_UNDEF);
  if (timed_out == true) {
    print_input(win, a);
  }
  return (UICB_OK);
}

void
register_input(WINDOW *win, struct input *a)
{
  register_ui_elt(input_cb, (void *) a, win);
}
