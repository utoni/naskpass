#include <stdlib.h>
#include <string.h>

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
  char *tmp;
  unsigned int diff_pos = 0;
  size_t width;

  if (a == NULL) return (UICB_ERR_UNDEF);
  attron(a->attrs);
  if (win != NULL) {
    width = getmaxx(win);
  } else {
    width = getmaxx(stdscr);
  }
  if (a->width < width) {
    diff_pos = (unsigned int) (width - a->width)/2;
  }
  tmp = (char *) malloc(width + 1);
  memset(tmp, ' ', width);
  tmp[width] = '\0';
  strncpy((tmp + diff_pos), a->text, a->width);
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
  strncpy(a->text, text, a->width);
}
