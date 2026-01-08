// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include "tui_interface.h"

int main() {
    // Initialize the TUI
    if (init_tui() != 0) {
        fprintf(stderr, "Failed to initialize TUI\n");
        return 1;
    }

    // 1. Directory Selection (via TUI)
    navigate_directory_selection();

    // 2. Simulate File Scanning & Hashing (replace with actual logic later)
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

    // 3. Results Display & Action Menu (via TUI)
    navigate_results_view();

    // 4. Cleanup
    end_tui();

    return 0;
}