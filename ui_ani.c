#include <stdlib.h>

#include "ui.h"
#include "ui_ani.h"

#define ANIC_INITSTATE '|'


struct anic *
init_anic(unsigned int x, unsigned int y, chtype attrs)
{
  struct anic *a = calloc(1, sizeof(struct anic));

  a->x = x;
  a->y = y;
  a->state = ANIC_INITSTATE;
  a->attrs = attrs;
  return (a);
}

void
free_anic(struct anic *a)
{
  free(a);
}

int
anic_cb(WINDOW *win, void *data, bool needs_update, bool timed_out)
{
  struct anic *a = (struct anic *) data;

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
  if (needs_update == true) {
    attron(a->attrs);
    if (win != NULL) {
      mvwaddch(win, a->y, a->x, a->state);
    } else {
      mvaddch(a->y, a->x, a->state);
    }
    attroff(a->attrs);
  } else return (UICB_ERR_NOP);
  return (UICB_OK);
}

void
register_anic(struct anic *a)
{
  register_ui_elt(anic_cb, (void *) a, NULL);
}
