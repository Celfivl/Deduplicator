// main_test.c
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define GetCurrentDir _getcwd
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define GetCurrentDir getcwd
#endif

#include "tui_interface.h"
#include "deduplicator.h"

#define TEST_DIR "v1_test_env"
#define MANUAL_MODE 1 

extern FileInfoList directory_list;
extern int selected_dir;
extern char current_browsing_path[1024];

void setup();
void teardown();

int main() {
    char cwd[1024];
    if (GetCurrentDir(cwd, sizeof(cwd)) != NULL) {
        strncpy(current_browsing_path, cwd, 1023);
    }

    setup();
    if (init_tui() != 0) return 1;

    // Enable arrow keys for the main screen during selection
    keypad(stdscr, TRUE);

    // --- Phase 1: Selection ---
    navigate_directory_selection();

    // --- Phase 2: Engine ---
    int total = 0;
    MarkedFile *results = find_duplicates(final_selected_path, &total, update_scan_progress);

    // --- Phase 3: Results (No Assumptions Mode) ---
    int *marks = calloc(total, sizeof(int)); // All initialized to 0 (No file is pre-marked)
    int cursor = 0, viewing = 1;

    // Ensure the results window specifically accepts arrow keys
    if (results_window) keypad(results_window, TRUE);

    while (viewing) {
        show_results_screen(results, total, cursor, marks);
        int ch = wgetch(results_window);

        // Explicitly handle arrow keys and standard WASD/JK alternatives
        switch(ch) {
            case KEY_UP:
            case 'k':
            case 'w':
                if (cursor > 0) cursor--;
                break;
            case KEY_DOWN:
            case 'j':
            case 's':
                if (cursor < total - 1) cursor++;
                break;
            case ' ':
                // User explicitly decides to mark this file for deletion
                marks[cursor] = !marks[cursor];
                break;
            case 'd':
            case 'D':
                if (show_confirmation_popup("Confirm deletion of SELECTED files?")) {
                    // Only here would we call the actual file removal logic
                    viewing = 0;
                }
                break;
            case 'q':
            case 'Q':
            case 27: // ESC
                if (show_confirmation_popup("Exit without deleting?")) viewing = 0;
                break;
        }
    }

    end_tui();
    
    printf("\n--- SESSION SUMMARY ---\n");
    int delete_count = 0;
    for(int i = 0; i < total; i++) if(marks[i]) delete_count++;
    printf("User selected %d files for removal.\n", delete_count);

    free(marks);
    free_results(results, total);
    teardown();
    return 0;
}

// --- Setup/Teardown Logic ---
void teardown() {
#ifdef _WIN32
    system("if exist " TEST_DIR " rd /s /q " TEST_DIR " >nul 2>&1");
#else
    system("rm -rf " TEST_DIR " > /dev/null 2>&1");
#endif
}

void setup() {
    teardown();
#ifdef _WIN32
    CreateDirectoryA(TEST_DIR, NULL);
#else
    mkdir(TEST_DIR);
#endif
    char p1[256], p2[256];
    snprintf(p1, 256, "%s/original.dat", TEST_DIR);
    snprintf(p2, 256, "%s/duplicate.dat", TEST_DIR);
    FILE *f1 = fopen(p1, "wb"); if(f1) { fputs("DUPE", f1); fclose(f1); }
    FILE *f2 = fopen(p2, "wb"); if(f2) { fputs("DUPE", f2); fclose(f2); }
}