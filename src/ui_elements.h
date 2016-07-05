#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H 1

#include "config.h"


void
show_info_wnd(struct txtwindow *wnd, char *title, char *text, chtype fore, chtype back, bool activate, bool blink);

void
init_ui_elements(WINDOW *wnd_main, unsigned int max_x, unsigned int max_y);

void
free_ui_elements(void);

#endif
