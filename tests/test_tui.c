// test_tui.c
#include <ncurses.h>
#include "tui_interface.h"
#include <assert.h>
#include <stdio.h>

int main() {
    init_tui();

    if (main_window == NULL) {
        end_tui();
        fprintf(stderr, "ERROR: Main window is NULL\n");
        return 1;
    }

    mvwprintw(main_window, 1, 2, "Test: Window Initialization");
    mvwprintw(main_window, 2, 2, "Status: main_window created successfully");
    mvwprintw(main_window, 4, 2, "Press any key to finalize test 13B...");
    
    // Push the changes to the physical screen
    wrefresh(main_window);

    // This prevents the screen from closing instantly
    getch();

    end_tui();

    printf("PASSED: test_tui (Visual verification complete)\n");
    return 0;
}