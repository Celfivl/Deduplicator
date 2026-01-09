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
        // 1. Directory Selection
        navigate_directory_selection();
        
        // Confirmation: Exit Program
        if (strlen(final_selected_path) == 0) {
            if (show_confirmation_popup("Exit Deduplicator?")) running = 0;
            continue; 
        }

        // Confirmation: Intended to scan chosen directory
        char confirm_msg[1100];
        snprintf(confirm_msg, sizeof(confirm_msg), "Start scan in: %s?", final_selected_path);
        if (!show_confirmation_popup(confirm_msg)) {
            memset(final_selected_path, 0, sizeof(final_selected_path));
            continue;
        }

        // 2. Scan Phase
        int total = 0;
        MarkedFile *results = find_duplicates(final_selected_path, &total, update_scan_progress);

        // Feature: Handle zero results gracefully
        if (!results || total == 0) {
            show_confirmation_popup("No duplicates found. Press any key...");
            memset(final_selected_path, 0, sizeof(final_selected_path));
            continue;
        }

        // 3. Results Loop
        int ch, cursor = 0, viewing = 1;
        int *marks = calloc(total, sizeof(int));
        keypad(results_window, TRUE);

        while (viewing) {
            show_results_screen(results, total, cursor, marks);
            ch = wgetch(results_window);

            if (ch == KEY_UP && cursor > 0) cursor--;
            else if (ch == KEY_DOWN && cursor < total - 1) cursor++;
            else if (ch == ' ' || ch == 10) {
                if (results[cursor].is_duplicate) marks[cursor] = !marks[cursor];
            }
            else if (ch == 'd' || ch == 'D') {
                // Confirmation: Specifically for the list of files to be deleted
                if (show_confirmation_popup("Confirm deletion of all marked files?")) {
                    // Logic for actual file removal
                    viewing = 0; 
                }
            }
            else if (ch == 'b' || ch == 'B') viewing = 0; 
            else if (ch == 'q' || ch == 'Q' || ch == 27) {
                // Confirmation: Desire to exit program from results screen
                if (show_confirmation_popup("Exit Deduplicator?")) {
                    viewing = 0;
                    running = 0;
                }
            }
        }

        free(marks);
        free_results(results, total);
        memset(final_selected_path, 0, sizeof(final_selected_path)); 
    }

    end_tui();
    return 0;
}