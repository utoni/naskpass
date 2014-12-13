#include <stdlib.h>

#include "ui.h"
#include "ui_statusbar.h"


struct statusbar *
init_statusbar(unsigned int y, unsigned int width, chtype attrs)
{
  struct statusbar *a = calloc(1, sizeof(struct statusbar));

  a->y = y;
  a->width = width;
  a->text = calloc(width, sizeof(char));
  a->attrs = attrs;
  return (a);
}

void
free_statusbar(struct statusbar *a)
{
  if (a->text) {
    free(a->text);
  }
  free(a);
}

int
statusbar_cb(WINDOW *win, void *data, bool timed_out)
{
  struct statusbar *a = (struct statusbar *) data;

  if (a == NULL) return (UICB_ERR_UNDEF);
  if (timed_out == true) {
  }
  attron(a->attrs);
  if (win != NULL) {
  } else {
  }
  attroff(a->attrs);
  return (UICB_OK);
}

void
register_statusbar(struct statusbar *a)
{
  register_ui_elt(statusbar_cb, (void *) a, NULL);
}

void
set_statusbar_text(struct statusbar *a, const char *text)
{

}
