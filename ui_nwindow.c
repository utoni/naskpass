#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_nwindow.h"


struct txtwindow *
init_txtwindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height, chtype attrs, window_func cb_update)
{
  struct txtwindow *a = calloc(1, sizeof(struct txtwindow));

  a->x = x;
  a->y = y;
  a->width = width;
  a->height = height;
  a->scrollable = false;
  a->title_len = INITIAL_TITLE_LEN;
  a->text_len = INITIAL_TEXT_LEN;
  a->title = calloc(a->title_len, sizeof(char));
  a->text = calloc(a->text_len, sizeof(char));
  a->attrs = attrs;
  a->window_func = cb_update;
  return (a);
}

void
free_txtwindow(struct txtwindow *a)
{
  if (a->text) {
    free(a->text);
  }
  if (a->title) {
    free(a->title);
  }
  free(a);
}

int
txtwindow_cb(WINDOW *win, void *data, bool timedout)
{
  struct txtwindow *a = (struct txtwindow *) data;
  return (UICB_OK);
}

void inline
register_txtwindow(struct txtwindow *a)
{
  register_ui_elt(txtwindow_cb, (void *) a, NULL);
}

static inline size_t
__do_textcpy(char **p_dest, size_t sz_dest, const char *p_src, size_t sz_src)
{
  if (sz_src > sz_dest) {
    *p_dest = (char *) realloc(*p_dest, sz_dest * sizeof(char));
  }
  memset(*p_dest, '\0', sz_dest);
  return sz_dest;
}

void
set_txtwindow_text(struct txtwindow *a, const char *text)
{
  size_t len = strlen(text);

  if (len > a->text_len) {
    a->text_len = __do_textcpy(&a->text, a->text_len, text, len);
  }
}
