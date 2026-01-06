// test/test_tui.c
#include <ncurses.h>
#include "tui_interface.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

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

    mvprintw(0, 0, "MANUAL MODE: Select a directory to trigger Progress Test.");
    refresh();

    // 1. Manual Navigation Test
    navigate_directory_selection(); 

    // 2. Automated 15B Dynamic Update Test
    // Once selection is confirmed, switch_to_progress_view() has already been called
    const char *test_stages[] = {
        "Traversing subdirectories",
        "Collecting file meta-data",
        "Hashing matched files",
        "Finalizing results"
    };

    for (int i = 0; i < 4; i++) {
        // Simulate a smooth progress bar fill for each stage
        for (int p = 0; p <= 100; p += 10) {
            update_scan_progress(test_stages[i], p, (i + 1) * 125);
            #ifdef _WIN32
            Sleep(50); 
            #endif
        }
    }

    mvprintw(LINES - 1, 2, "15B Test Complete. Press any key to exit.");
    refresh();
    getch();

    end_tui();

    printf("\nManual Session Complete. Path Captured: %s\n", final_selected_path);
    return 0;
}