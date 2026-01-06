// tui_interface.h
#ifndef TUI_INTERFACE_H
#define TUI_INTERFACE_H

#include <ncurses.h>

extern WINDOW *main_window; // Declared as extern to prevent multiple definitions

int init_tui();
void end_tui();

#endif