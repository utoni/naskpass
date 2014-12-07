#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_input.h"


struct input *
init_input(unsigned int x, unsigned int y, unsigned int width, char *prompt, size_t input_len)
{
  struct input *a = calloc(1, sizeof(struct input));

  a->x = x;
  a->y = y;
  a->width = width;
  a->input = calloc(input_len+1, sizeof(char));
  a->input_max = input_len;
  a->input_len = 0;
  a->input_pos = 0;
  a->prompt = strdup(prompt);
  a->active = false;
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
print_input(WINDOW *win, struct input *a)
{
  char *tmp;
  int i;
  size_t p_len = strlen(a->prompt);

  if (win == NULL) {
    mvprintw(a->y, a->x, a->prompt);
    tmp = calloc(a->width+1, sizeof(char));
    for (i = 0; i < a->width; i++) {
      *(tmp + i) = '_';
    }
    mvprintw(a->y, a->x + p_len, tmp);
    free(tmp);
    mvprintw(a->y, a->x + p_len, a->input);
  }
}

int
post_input_cb(WINDOW *win, void *data, bool needs_update)
{
  struct input *a = (struct input *) data;

  if (a == NULL) return (UICB_ERR_UNDEF);
  if (a->active == true) {
    if (win == NULL) {
      move(a->y, a->x + a->input_pos);
    } else {
      wmove(win, a->y, a->x + a->input_pos);
    }
    return (UICB_CURSOR);
  }
  return (UICB_OK);
}


int
input_cb(WINDOW *win, void *data, bool needs_update)
{
  struct input *a = (struct input *) data;

  if (a == NULL) return (UICB_ERR_UNDEF);
  print_input(win, a);
  return (UICB_OK);
}

void
register_input(WINDOW *win, struct input *a, chtype attr)
{
  register_ui_elt(input_cb, post_input_cb, (void *) a, win, attr);
}
