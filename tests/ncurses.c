#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <assert.h>


static WINDOW *wnd;

int
main(void)
{
  assert( (wnd = initscr()) != NULL );
  assert( start_color() == 0 );
  assert( init_pair(1, COLOR_BLACK, COLOR_WHITE) == 0 );
  assert( raw() == 0 );
  assert( keypad(wnd, TRUE) == 0 );
  assert( noecho() == 0 );
  assert( cbreak() == 0 );
  assert( printw("TESTEST") == 0 );
  assert( refresh() == 0 );
  assert( clear() == 0 );
  assert( delwin(wnd) == 0 );
  assert( endwin() == 0 );
  return 0;
}
