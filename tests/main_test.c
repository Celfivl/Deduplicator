#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define GetCurrentDir _getcwd
    #define ACCESS _access
    #define F_OK 0
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
    #define ACCESS access
#endif

#include "tui_interface.h"
#include "deduplicator.h"

#define TEST_DIR "v1_test_env"

// --- Helper: Check if file exists on disk ---
int file_exists(const char *path) {
    return (ACCESS(path, F_OK) == 0);
}

void setup() {
    // Clean old runs
#ifdef _WIN32
    system("if exist " TEST_DIR " rd /s /q " TEST_DIR " >nul 2>&1");
    CreateDirectoryA(TEST_DIR, NULL);
#else
    system("rm -rf " TEST_DIR " > /dev/null 2>&1");
    mkdir(TEST_DIR);
#endif

    // Create Dummy Files
    char p1[256], p2[256];
    snprintf(p1, 256, "%s/original.dat", TEST_DIR);
    snprintf(p2, 256, "%s/duplicate.dat", TEST_DIR);
    
    FILE *f1 = fopen(p1, "wb"); if(f1) { fputs("CONTENT_A", f1); fclose(f1); }
    FILE *f2 = fopen(p2, "wb"); if(f2) { fputs("CONTENT_A", f2); fclose(f2); }
}

int main() {
    setup();
    
    // Auto-detect current path for browsing
    char cwd[1024];
    if (GetCurrentDir(cwd, sizeof(cwd)) != NULL) {
        strncpy(current_browsing_path, cwd, 1023);
    }

    if (init_tui() != 0) return 1;

    /* --- PHASE 1: DIRECTORY SELECTION --- */
    // INSTRUCTIONS: Navigate to 'v1_test_env' and press SPACE, then ENTER
    navigate_directory_selection();

    /* --- PHASE 2: ENGINE EXECUTION --- */
    int total = 0;
    MarkedFile *results = find_duplicates(final_selected_path, &total, update_scan_progress);

    /* --- PHASE 3: INTERACTIVE TUI (The Refactor Fix) --- */
    // We use the ACTUAL production function.
    // INSTRUCTIONS: Mark the duplicate (red) with SPACE, then press D to delete.
    if (results && total > 0) {
        navigate_results_view(results, total);
    }

    end_tui();

    /* --- VERIFICATION PHASE --- */
    printf("\n--- INTEGRATION TEST REPORT ---\n");
    
    char original[256], duplicate[256];
    snprintf(original, 256, "%s/original.dat", TEST_DIR);
    snprintf(duplicate, 256, "%s/duplicate.dat", TEST_DIR);

    int orig_exists = file_exists(original);
    int dupe_exists = file_exists(duplicate);

    if (orig_exists && !dupe_exists) {
        printf("[SUCCESS] Original file preserved, duplicate deleted.\n");
    } else if (!orig_exists) {
        printf("[FAIL] Original file was deleted!\n");
    } else if (dupe_exists) {
        printf("[FAIL] Duplicate file still exists (Did you press 'D'?)\n");
    }

    // Cleanup Memory (Disk cleanup is skipped so you can inspect results)
    free_results(results, total);
    
    return 0;
}