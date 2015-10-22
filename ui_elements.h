#ifndef UI_ELEMENTS_H
#define UI_ELEMENTS_H 1

#include "ui.h"
#include "ui_ani.h"
#include "ui_input.h"
#include "ui_statusbar.h"
#include "ui_nwindow.h"

#include "status.h"
#include "config.h"


void
init_ui_elements(WINDOW *wnd_main, unsigned int max_x, unsigned int max_y);

void
free_ui_elements(void);

#endif
