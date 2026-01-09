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
            if (show_confirmation_popup("Exit Deduplicator?")) running = 0;
            continue; 
        }

        // --- STAGE 2: Confirmation ---
        char confirm_msg[1100];
        snprintf(confirm_msg, sizeof(confirm_msg), "Start scan in: %s?", final_selected_path);
        if (!show_confirmation_popup(confirm_msg)) {
            memset(final_selected_path, 0, sizeof(final_selected_path));
            continue;
        }

        // --- STAGE 3: Workflow/Progress Bar ---
        int total = 0;
        MarkedFile *results = find_duplicates(final_selected_path, &total, update_scan_progress);

        if (!results || total == 0) {
            show_confirmation_popup("No duplicates found. Press any key...");
            memset(final_selected_path, 0, sizeof(final_selected_path));
            if (results) free_results(results, total);
            continue;
        }

        // --- STAGE 4: Results/Selection ---
        int ch, cursor = 0, viewing = 1;
        int *marks = calloc(total, sizeof(int));
        keypad(results_window, TRUE);

        while (viewing) {
            show_results_screen(results, total, cursor, marks);
            ch = wgetch(results_window);

            if (ch == KEY_UP && cursor > 0) cursor--;
            else if (ch == KEY_DOWN && cursor < total - 1) cursor++;
            else if (ch == ' ' || ch == 10) {
                marks[cursor] = !marks[cursor]; // Toggle selection
            }
            else if (ch == 'd' || ch == 'D') {
                // --- STAGE 5: Confirmation (Action) ---
                if (show_confirmation_popup("Confirm deletion of all marked files?")) {
                    // --- STAGE 6: Verification of Action ---
                    execute_deletion(results, marks, total);
                    show_confirmation_popup("Action Verified: Files Deleted. Press any key...");
                    viewing = 0; // Break Results Loop
                }
            }
            else if (ch == 'b' || ch == 'B') {
                viewing = 0; // Return to selection
            }
            else if (ch == 'q' || ch == 'Q' || ch == 27) {
                if (show_confirmation_popup("Exit Deduplicator?")) {
                    viewing = 0;
                    running = 0;
                }
            }
        }

        // --- STAGE 7: Return to Directory Selection ---
        free(marks);
        free_results(results, total);
        memset(final_selected_path, 0, sizeof(final_selected_path)); 
    }

    end_tui();
    return 0;
}