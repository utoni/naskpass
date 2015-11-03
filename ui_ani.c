#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_ani.h"

#define ANIC_INITSTATE '|'


struct anic *
init_anic(unsigned int x, unsigned int y, chtype attrs, char *fmt)
{
  struct anic *a = calloc(1, sizeof(struct anic));

  a->x = x;
  a->y = y;
  a->state = ANIC_INITSTATE;
  a->attrs = attrs;
  if (fmt != NULL) {
    a->fmt = strdup(fmt);
  }
  return (a);
}

void
free_anic(struct anic *a)
{
  if (a->fmt != NULL) {
    free(a->fmt);
  }
  free(a);
}

int
anic_cb(WINDOW *win, void *data, bool timed_out)
{
  struct anic *a = (struct anic *) data;
  char *tmp;
  int retval = UICB_OK;

  if (a == NULL) return (UICB_ERR_UNDEF);
  if (timed_out == true) {
    switch (a->state) {
      default:
      case '|': a->state = '/'; break;
      case '/': a->state = '-'; break;
      case '-': a->state = '\\'; break;
      case '\\': a->state = '|'; break;
    }
  }
  attron(a->attrs);
  if (a->fmt != NULL) {
    if (asprintf(&tmp, a->fmt, a->state) <= 0) {
      retval = UICB_ERR_BUF;
    }
  } else {
    asprintf(&tmp, "%c", a->state);
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
register_anic(struct anic *a)
{
  struct ui_callbacks cbs;
  cbs.ui_element = anic_cb;
  cbs.ui_input = NULL;
  register_ui_elt(&cbs, (void *) a, NULL);
}
