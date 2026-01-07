// test/test_tui.c
#include <ncurses.h>
#include "tui_interface.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Simulates keyboard input
void simulate_input(int key, int num_times) {
    for (int i = 0; i < num_times; i++) {
        ungetch(key);
    }
}

// Unit Test: Logic only, assumes TUI is already active
int test_results_transition_logic() {
    // Manually trigger the transition condition
    update_scan_progress("Finalizing results", 100, 123);

    if (results_window == NULL) {
        return 1;
    }

    // Verify responsiveness to input (simulation)
    simulate_input(KEY_DOWN, 2);
    simulate_input(KEY_UP, 1);
    simulate_input(10, 1); // Enter

    return 0;
}

int main() {
    // 1. Setup
    if (init_tui() != 0) {
        fprintf(stderr, "Failed to initialize TUI\n");
        return 1;
    }

    // 2. Manual Directory Selection Phase
    mvprintw(0, 0, "MANUAL MODE: Select a directory to trigger Progress Test.");
    refresh();
    navigate_directory_selection(); 

    // 3. Automated Progress Phase (15B Verification)
    const char *test_stages[] = {
        "Traversing subdirectories",
        "Collecting file meta-data",
        "Hashing matched files",
        "Finalizing results"
    };

    for (int i = 0; i < 4; i++) {
        for (int p = 0; p <= 100; p += 20) {
            update_scan_progress(test_stages[i], p, (i + 1) * 125);
            #ifdef _WIN32
            Sleep(100); 
            #endif
        }
    }

    // 4. Automated Results Transition (16B Verification)
    // Note: The loop above triggers the transition on the last iteration.
    // Now we enter the interaction loop to satisfy 16A requirements.
    navigate_results_view(); 

    // 5. Cleanup
    mvprintw(LINES - 1, 2, "16B Test Complete. Press any key to exit.");
    refresh();
    getch();

    end_tui();

    printf("\nTest Session Complete.\nTarget Path: %s\n", final_selected_path);
    return 0;
}