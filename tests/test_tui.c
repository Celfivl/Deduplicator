// test/test_tui.c
#include <ncurses.h>
#include "tui_interface.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Function to simulate keyboard input
void simulate_input(int key, int num_times) {
    for (int i = 0; i < num_times; i++) {
        ungetch(key);
    }
}

int test_directory_selection() {
    int result = 0;
    char expected_path[1024] = "";

    // Initialize the TUI
    init_tui();

    // Simulate navigating to the second directory
    simulate_input(KEY_DOWN, 1);

    // Select the directory (simulate pressing Enter)
    simulate_input(10, 1);

    // Verify that the selected path is correct
    strcpy(expected_path, "test"); //Assuming "test" is the second directory
    if (strcmp(final_selected_path, expected_path) != 0) {
        printf("ERROR: Selected path is incorrect. Expected '%s', got '%s'\n", expected_path, final_selected_path);
        result = 1;
    }

    // Clean up and exit
    end_tui();
    return result;
}

int test_window_initialization() {
    // Basic test to ensure the main window initializes
    init_tui();

    if (main_window == NULL && stdscr == NULL) {
        end_tui();
        fprintf(stderr, "ERROR: Main window is NULL\n");
        return 1;
    }

    end_tui();
    return 0;
}

int main() {
    init_tui();

    mvprintw(0, 0, "MANUAL MODE ACTIVE: If screen is blank, press any key.");
    refresh();

    navigate_directory_selection();

    end_tui();

    printf("\nManual Session Complete. Path Captured: %s\n", final_selected_path);
    return 0;
}