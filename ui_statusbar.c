#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_statusbar.h"


struct statusbar *
init_statusbar(unsigned int y, unsigned int width, chtype attrs, status_func cb_update)
{
  struct statusbar *a = calloc(1, sizeof(struct statusbar));

  a->y = y;
  a->width = width;
  a->text = calloc(a->width, sizeof(char));
  a->attrs = attrs;
  a->status_func = cb_update;
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
  char *tmp;
  unsigned int diff_pos = 0;
  size_t len;

  if (a == NULL) return (UICB_ERR_UNDEF);
  if (timed_out == true) {
    if (a->status_func != NULL) {
      a->status_func(win, a);
    }
  }
  attron(a->attrs);
  len = strnlen(a->text, a->width);
  if (len < a->width) {
    diff_pos = (unsigned int) (a->width - len)/2;
  }
  tmp = (char *) malloc(a->width + 1);
  memset(tmp, ' ', a->width);
  tmp[a->width] = '\0';
  strncpy((tmp + diff_pos), a->text, len);
  if (win != NULL) {
    mvwprintw(win, a->y, 0, tmp);
  } else {
    mvprintw(a->y, 0, tmp);
  }
  free(tmp);
  attroff(a->attrs);
  return (UICB_OK);
}

void
register_statusbar(struct statusbar *a)
{
  register_ui_elt(statusbar_cb, (void *) a, NULL);
}

inline void
set_statusbar_text(struct statusbar *a, const char *text)
{
  size_t len = strlen(text);

  strncpy(a->text, text, (len > a->width ? a->width : len));
}
