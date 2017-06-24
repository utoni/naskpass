#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_ani.h"

#define ANIC_INITSTATE '|'


struct anic *
init_anic_default(unsigned int x, unsigned int y, chtype attrs, char *fmt)
{
  struct anic *a = init_anic(x, y, attrs, anic_cb);
  struct anic_default *b = calloc(1, sizeof(struct anic_default));

  a->data = (void *) b;
  b->state = ANIC_INITSTATE;
  if (fmt != NULL) {
    b->fmt = strdup(fmt);
  }
  return (a);
}

struct anic *
init_anic(unsigned int x, unsigned int y, chtype attrs, uicb_anic uicb)
{
  struct anic *a = calloc(1, sizeof(struct anic));

  a->x = x;
  a->y = y;
  a->uicb = uicb;
  a->attrs = attrs;
  return (a);
}

void
free_anic(struct anic *a)
{
  free(a);
}

void
free_anic_default(struct anic *a)
{
  struct anic_default *b;

  if (a->data != NULL) {
    b = (struct anic_default *) a->data;
    free(b->fmt);
    free(b);
  }
  free_anic(a);
}

int
anic_cb(WINDOW *win, void *data, bool timed_out)
{
  struct anic *a = (struct anic *) data;
  struct anic_default *b;
  char *tmp;
  int retval = UICB_OK;

  if (a == NULL) return (UICB_ERR_UNDEF);
  b = (struct anic_default *) a->data;
  if (timed_out == true) {
    switch (b->state) {
      default:
      case '|': b->state = '/'; break;
      case '/': b->state = '-'; break;
      case '-': b->state = '\\'; break;
      case '\\': b->state = '|'; break;
    }
  }
  attron(a->attrs);
  if (b->fmt != NULL) {
    if (asprintf(&tmp, b->fmt, b->state) <= 0) {
      retval = UICB_ERR_BUF;
    }
  } else {
    if (asprintf(&tmp, "%c", b->state) < 0)
      retval = UICB_ERR_BUF;
  }
  if (win != NULL) {
    mvwprintw(win, a->y, a->x, tmp);
  } else {
    mvprintw(a->y, a->x, tmp);
  }
  free(tmp);
  attroff(a->attrs);
  return (retval);
}

void
register_anic(struct anic *a, uicb_anic uicb)
{
  struct ui_callbacks cbs;
  cbs.ui_element = uicb;
  cbs.ui_input = NULL;
  register_ui_elt(&cbs, (void *) a, NULL);
}

void
register_anic_default(struct anic *a)
{
  register_anic(a, anic_cb);
}
