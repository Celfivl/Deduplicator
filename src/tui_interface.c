// tui_interface.c
#include <ncurses.h>
#include "tui_interface.h"

WINDOW *main_window = NULL; // Define the pointer

int init_tui() {
    initscr(); // Initialize ncurses
    cbreak();  // Disable line buffering
    noecho();  // Don't echo user input
    keypad(stdscr, TRUE); // Enable special keys

    // Initialize the physical screen
    refresh();

    // Create windows for the layout (example)
    main_window = newwin(LINES - 2, COLS - 2, 1, 1);
    if (main_window != NULL) {
        box(main_window, 0, 0); // Draw a box around the window
        wrefresh(main_window);
    }

    return 0;
}

void end_tui() {
    if (main_window != NULL) {
        delwin(main_window); // Properly delete the window
    }
    endwin(); // End ncurses
}