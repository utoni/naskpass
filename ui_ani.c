#include <stdlib.h>

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
anic_cb(WINDOW *win, void *data)
{
  struct anic *a = (struct anic *) data;

  switch (a->state) {
    default:
    case '|': return ('/');
    case '/': return ('-');
    case '-': return ('\\');
    case '\\': return ('|');
  }
}
