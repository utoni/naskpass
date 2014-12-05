#include <stdlib.h>

#include "ui.h"
#include "ui_ani.h"

#define ANIC_INITSTATE '\0'


struct anic *
init_anic(unsigned int x, unsigned int y)
{
  struct anic *a = calloc(1, sizeof(struct anic));

  a->x = x;
  a->y = y;
  a->state = ANIC_INITSTATE;
  return (a);
}

void
free_anic(struct anic *a)
{
  free(a);
}

int
anic_cb(WINDOW *win, void *data, bool needs_update)
{
  struct anic *a = (struct anic *) data;

  if (a == NULL) return (UICB_ERR_UNDEF);
  switch (a->state) {
    default:
    case '|': a->state = '/'; break;
    case '/': a->state = '-'; break;
    case '-': a->state = '\\'; break;
    case '\\': a->state = '|'; break;
  }
  if (needs_update == true) {
    if (win != NULL) {
      mvwaddch(win, a->y, a->x, a->state);
    } else {
      mvaddch(a->y, a->x, a->state);
    }
  } else return (UICB_ERR_NOP);
  return (UICB_OK);
}

void
register_anic(struct anic *a, chtype attr)
{
  register_ui_elt(anic_cb, (void *) a, NULL, attr);
}
