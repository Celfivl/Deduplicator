#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tui_interface.h"
#include "deduplicator.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* Mock engine runner to simulate the progress bar and results generation */
MarkedFile* simulate_engine_run(const char *path, int *count) {
    const char *stages[] = {"Scanning", "Hashing", "Comparing", "Finalizing"};
    *count = 4;
    
    for (int i = 0; i < 4; i++) {
        for (int p = 0; p <= 100; p += 25) {
            update_scan_progress(stages[i], p, (i + 1) * 10);
            #ifdef _WIN32
            Sleep(100);
            #endif
        }
    }

    MarkedFile *mock = malloc(sizeof(MarkedFile) * (*count));
    mock[0] = (MarkedFile){strdup("C:\\Mock\\file1.exe"), 1, 0};
    mock[1] = (MarkedFile){strdup("C:\\Mock\\file1_copy.exe"), 1, 1};
    mock[2] = (MarkedFile){strdup("C:\\Mock\\data.bin"), 2, 0};
    mock[3] = (MarkedFile){strdup("C:\\Mock\\data_old.bin"), 2, 1};
    return mock;
}

int main() {
    if (init_tui() != 0) return 1;

    while (1) {
        // 1. Path Selection Phase
        navigate_directory_selection();

        // If final_selected_path is empty, user pressed 'Q' to quit the app
        if (strlen(final_selected_path) == 0) break;

        // 2. Simulation Phase (Engine Push)
        int result_count = 0;
        MarkedFile *results = simulate_engine_run(final_selected_path, &result_count);

        // 3. Results/Deletion Phase
        // This function must return control once 'D' or 'Q' is pressed
        navigate_results_view(results, result_count);

        // 4. Cleanup & Reset for next "Run"
        for (int i = 0; i < result_count; i++) free(results[i].path);
        free(results);
        
        // Clear global path so the browser starts fresh
        memset(final_selected_path, 0, sizeof(final_selected_path));
    }

    end_tui();
    printf("Test Suite Clean Exit.\n");
    return 0;
}