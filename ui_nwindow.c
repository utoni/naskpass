#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "ui_nwindow.h"


struct txtwindow *
init_txtwindow(unsigned int x, unsigned int y, unsigned int width, unsigned int height, chtype attrs, chtype text_attrs, window_func cb_update)
{
  struct txtwindow *a = calloc(1, sizeof(struct txtwindow));

  a->x = x;
  a->y = y;
  a->width = width;
  a->height = height;
  a->scrollable = false;
  a->title_len = INITIAL_TITLE_LEN;
  a->title = calloc(a->title_len+1, sizeof(char));
  a->text = NULL;
  a->attrs = attrs;
  a->text_attrs = text_attrs;
  a->window_func = cb_update;
  return (a);
}

static void
__free_text(struct txtwindow *a)
{
  if (a->text) {
    if (a->text[0]) {
      free(a->text[0]);
    }
    free(a->text);
    a->text = NULL;
  }
}

void
free_txtwindow(struct txtwindow *a)
{
  __free_text(a);
  if (a->title) {
    free(a->title);
  }
  free(a);
}

static void
print_wnd(struct txtwindow *a)
{
  int i, x = a->x, y = a->y, w = a->width, h = a->height;
  char tmp[a->width+1];

  attron(a->attrs);
  /* print window surface */
  memset(tmp, ' ', a->width);
  tmp[a->width] = '\0';
  for (i = y-1; i < y+h+2; i++)
    mvprintw(i, x, tmp);
  /* print window border */
  mvhline(y-2, x, 0, w);
  mvhline(y+h+2, x, 0, w);
  mvvline(y-1, x-1, 0, h+3);
  mvvline(y-1, x+w, 0, h+3);
  /* print window border edges */
  mvaddch(y-2, x-1, ACS_ULCORNER);
  mvaddch(y+2+h, x-1, ACS_LLCORNER);
  mvaddch(y-2, x+w, ACS_URCORNER);
  mvaddch(y+2+h, x+w, ACS_LRCORNER);
  /* print window title */
  attroff(a->attrs);
  attron(a->text_attrs);
  mvprintw(y-2, (float)x+(float)(w/2)-(float)(a->title_len*2/3), "[ %s ]", a->title);
  /* print windows text */
  i = 0;
  while ( a->text[i] ) {
    mvprintw(y+i, x, a->text[i]);
    i++;
  }
  attroff(a->text_attrs);
}

int
txtwindow_cb(WINDOW *win, void *data, bool timedout)
{
  struct txtwindow *a = (struct txtwindow *) data;

  print_wnd(a);
  return (UICB_OK);
}

void inline
register_txtwindow(struct txtwindow *a)
{
  register_ui_elt(txtwindow_cb, (void *) a, NULL);
}

static size_t
__do_textcpy(char **p_dest, size_t sz_dest, const char *p_src, size_t sz_src)
{
  size_t cursiz = sz_dest;

  if (sz_src > sz_dest) {
    *p_dest = (char *) realloc(*p_dest, (sz_src+1) * sizeof(char));
    cursiz = sz_src;
  }
  memset(*p_dest, '\0', (cursiz+1) * sizeof(char));
  memcpy(*p_dest, p_src, (cursiz+1) * sizeof(char));
  return sz_src;
}

/* seperate a String with NEWLINES into an array */
static char **
__do_textadjust(struct txtwindow *a, char *text)
{
  int i = 0, rows = (int)(strlen(text) / a->width)+1;
  char **adj_text = calloc(rows+1, sizeof(char *));
  char *p_strtok, *tok;
  const char sep[] = "\n";

  if (rows > a->height) goto error;
  p_strtok = strdup(text);
  while ( (tok = strsep(&p_strtok, sep)) && rows-- >= 0 ) {
    if (strlen(tok) > a->width) {
      strcpy(tok+a->width-3, "...");
      *(tok+a->width) = '\0'; 
    }
    adj_text[i] = tok;
    i++;
  }
  return adj_text;
error:
  free(adj_text);
  return NULL;
}

void
set_txtwindow_text(struct txtwindow *a, char *text)
{
  char **fmt_text = __do_textadjust(a, text);

  if (fmt_text) {
    __free_text(a);
    a->text = fmt_text;
  }
}

void
set_txtwindow_title(struct txtwindow *a, const char *title)
{
  a->title_len = __do_textcpy(&a->title, a->title_len, title, strlen(title));
}
