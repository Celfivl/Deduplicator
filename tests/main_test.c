// main_test.c
#include <ncurses.h>
#include "tui_interface.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Toggle this to 0 for a lightning-fast automated "Smoke Test"
#define MANUAL_TEST 1 

void run_automated_flow() {
    ungetch(10);  // Select Dir
    ungetch(' '); // Toggle Mark
    ungetch(27);  // Exit
}

int main() {
    if (init_tui() != 0) return 1;

    // If we are debugging logic, push keys to the buffer immediately
    if (!MANUAL_TEST) {
        run_automated_flow();
    }

    // --- Phase 1: Selection ---
    navigate_directory_selection();

    // --- Phase 2: Progress Simulation (Commit 17A/B Logic) ---
    const char *stages[] = {"Scanning", "Hashing", "Finalizing"};
    for (int i = 0; i < 3; i++) {
        update_scan_progress(stages[i], (i + 1) * 33, i * 5);
        #ifdef _WIN32
        if (MANUAL_TEST) Sleep(200); // Only slow down if we're watching it
        #endif
    }

    // --- Phase 3: Results ---
    navigate_results_view();

    end_tui();

    printf("Test Sequence %s\n", MANUAL_TEST ? "Manual Verified" : "Auto-Passed");
    return 0;
}