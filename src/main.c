// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tui_interface.h"
#include "deduplicator.h"

int main() {
    if (init_tui() != 0) return 1;

    int running = 1;
    while (running) {
        // --- STAGE 1: Directory Traversal/Selection ---
        navigate_directory_selection();
        
        if (strlen(final_selected_path) == 0) {
            if (show_popup("Exit Deduplicator?", 1)) running = 0;
            continue; 
        }

        // --- STAGE 2: Confirmation ---
        char confirm_msg[1100];
        snprintf(confirm_msg, sizeof(confirm_msg), "Start scan in: %.50s?", final_selected_path);
        
        if (!show_popup(confirm_msg, 1)) {
            memset(final_selected_path, 0, sizeof(final_selected_path));
            continue;
        }

        // --- STAGE 3: Workflow/Progress Bar ---
        int total = 0;
        MarkedFile *results = find_duplicates(final_selected_path, &total, update_scan_progress);

        if (!results || total == 0) {
            show_popup("No duplicates found.", 0);
            if (results) free_results(results, total);
            memset(final_selected_path, 0, sizeof(final_selected_path));
            continue;
        }

        // --- STAGE 4: Results/Selection & STAGE 5-6: Action/Verification ---
        navigate_results_view(results, total);

        // --- STAGE 7: Return to Directory Selection ---
        free_results(results, total);
        memset(final_selected_path, 0, sizeof(final_selected_path)); 
    }

    end_tui();
    return 0;
}