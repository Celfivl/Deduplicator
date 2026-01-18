// test_tui.c
#include <ncurses.h>
#include "tui_interface.h"
#include "deduplicator.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

void simulate_input(int key, int num_times) {
    for (int i = 0; i < num_times; i++) {
        ungetch(key);
    }
}

int main() {
    if (init_tui() != 0) {
        fprintf(stderr, "Failed to initialize TUI\n");
        return 1;
    }

    mvprintw(0, 0, "MANUAL MODE: Select a directory to trigger Progress Test.");
    refresh();
    navigate_directory_selection(); 

    const char *test_stages[] = {
        "Traversing subdirectories",
        "Collecting file meta-data",
        "Hashing matched files",
        "Finalizing results"
    };

    for (int i = 0; i < 4; i++) {
        for (int p = 0; p <= 100; p += 25) {
            update_scan_progress(test_stages[i], p, (i + 1) * 125);
            #ifdef _WIN32
            Sleep(100); 
            #endif
        }
    }

    /* Mock data to test 18A Group-ID rendering */
    int mock_total = 4;
    MarkedFile mock_results[4] = {
        { .path = "C:\\Data\\file_a.txt", .group_id = 1, .is_duplicate = 0 },
        { .path = "C:\\Backup\\file_a_copy.txt", .group_id = 1, .is_duplicate = 1 },
        { .path = "C:\\Photos\\image.png", .group_id = 2, .is_duplicate = 0 },
        { .path = "C:\\Old\\image_backup.png", .group_id = 2, .is_duplicate = 1 }
    };

    /* Verify transition to interactive results review */
    navigate_results_view(mock_results, mock_total); 

    mvprintw(LINES - 1, 2, "18A Test Complete. Press any key to exit.");
    refresh();
    getch();

    end_tui();

    printf("\nTest Session Complete.\nTarget Path: %s\n", final_selected_path);
    return 0;
}