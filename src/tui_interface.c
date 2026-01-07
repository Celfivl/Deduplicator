// tui_interface.c
#include <ncurses.h>
#include "tui_interface.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

// --- Global UI State ---
WINDOW *main_window = NULL, *dir_window = NULL, *progress_window = NULL, *results_window = NULL; // Added results_window
FileInfoList directory_list;
int selected_dir = 0, scroll_offset = 0;
char final_selected_path[1024] = "";

#ifdef _WIN32
char current_browsing_path[1024] = ""; 
#else
char current_browsing_path[1024] = "/";
#endif

// --- Helper Functions ---
// Updates the current browsing path
static void update_path(const char* addition) {
    if (!addition) {
        char *last = strrchr(current_browsing_path, '\\');
        if (!last) last = strrchr(current_browsing_path, '/');
        if (last) {
            if (last == current_browsing_path + 2 && current_browsing_path[1] == ':') current_browsing_path[0] = '\0';
            else *last = '\0';
        } else current_browsing_path[0] = '\0';
    } else {
        if (strlen(current_browsing_path) == 0) strcpy(current_browsing_path, addition);
        else {
            size_t len = strlen(current_browsing_path);
            if (current_browsing_path[len-1] != '\\' && current_browsing_path[len-1] != '/') strcat(current_browsing_path, "\\");
            strcat(current_browsing_path, addition);
        }
    }
}

// --- Core Lifecycle ---
// Initializes the TUI
int init_tui() {
    initscr();
    if (!stdscr) return 1;
    noecho(); cbreak(); curs_set(0); keypad(stdscr, TRUE);
    init_file_list(&directory_list);
    populate_directory_list(current_browsing_path);

    // Create windows
    int dir_height = 18, dir_width = 70;
    dir_window = newwin(dir_height, dir_width, (LINES - dir_height) / 2, (COLS - dir_width) / 2);

    int progress_height = 10, progress_width = 60;
    progress_window = newwin(progress_height, progress_width, (LINES - progress_height) / 2, (COLS - progress_width) / 2); // Create progress window

     int results_height = 20, results_width = 80;
    results_window = newwin(results_height, results_width, (LINES - results_height) / 2, (COLS - results_width) / 2);

    
    if(progress_window == NULL)
    	show_directory_selection_screen();

    return 0;
}

// Ends the TUI
void end_tui() {
    free_ui_list(&directory_list);
    if (dir_window) delwin(dir_window);
    if (progress_window) delwin(progress_window); // Delete progress window
    if (results_window) delwin(results_window);
    endwin();
}

// --- Selection Logic ---
// Populates the directory list
void populate_directory_list(const char *path) {
    free_ui_list(&directory_list);
#ifdef _WIN32
    if (strlen(path) == 0) {
        char drives[256], *d = drives;
        GetLogicalDriveStringsA(sizeof(drives), drives);
        while (*d) { add_to_list(&directory_list, d, 1); d += strlen(d) + 1; }
        return;
    }
    add_to_list(&directory_list, "..", 1);
    char s[1024]; sprintf(s, "%s\\*", path);
    WIN32_FIND_DATAA fd; HANDLE h = FindFirstFileA(s, &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (fd.cFileName[0] == '.' && (fd.cFileName[1] == '\0' || fd.cFileName[1] == '.')) continue;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) add_to_list(&directory_list, fd.cFileName, 1);
        } while (FindNextFileA(h, &fd));
        FindClose(h);
    }
#endif
}

// Shows a confirmation popup
int show_confirmation_popup(const char *path) {
    WINDOW *p = newwin(7, 60, (LINES - 7) / 2, (COLS - 60) / 2);
    box(p, 0, 0);
    mvwprintw(p, 1, 2, "Confirm Selection:");
    mvwprintw(p, 3, 2, "Path: %-54.54s", path);
    mvwprintw(p, 5, 15, "[Y] Confirm / [N] Cancel");
    wrefresh(p);
    int c = wgetch(p);
    delwin(p);
    return (c == 'y' || c == 'Y');
}

// Shows the directory selection screen
    void show_directory_selection_screen() {
    int h, w; getmaxyx(dir_window, h, w);
    int max = h - 4;
    if (selected_dir < scroll_offset) scroll_offset = selected_dir;
    else if (selected_dir >= scroll_offset + max) scroll_offset = selected_dir - max + 1;

    werase(dir_window);
    box(dir_window, 0, 0);
    mvwprintw(dir_window, 0, 2, " Path: %s ", strlen(current_browsing_path) ? current_browsing_path : "System");
    
    FileInfo *curr = directory_list.head;
    for (int i = 0; i < scroll_offset && curr; i++) curr = curr->next;
    for (int i = 0; i < max && curr; i++) {
        if (scroll_offset + i == selected_dir) wattron(dir_window, A_REVERSE);
        mvwprintw(dir_window, i + 2, 2, " [DIR] %-60.60s ", curr->name);
        wattroff(dir_window, A_REVERSE);
        curr = curr->next;
    }
    wnoutrefresh(stdscr); wnoutrefresh(dir_window); doupdate();
}

//Shows the scan progress screen
void show_scan_progress_screen() {
    werase(progress_window);
    box(progress_window, 0, 0);
    mvwprintw(progress_window, 1, 2, "Scan Progress:");
    mvwprintw(progress_window, 3, 2, "Progress: [----------] 0%%");
    mvwprintw(progress_window, 5, 2, "Scanning: ");
    wrefresh(progress_window);
}

// Updates the scan progress screen with process-stage focus
void update_scan_progress(const char *current_process, int percentage, int files_found) {
    if (!progress_window) return;

    werase(progress_window);
    box(progress_window, 0, 0);
    mvwprintw(progress_window, 1, 2, "Deduplicator Engine: Active");

    // Progress bar logic
    int bar_width = 30;
    int filled = (percentage * bar_width) / 100;
    mvwprintw(progress_window, 3, 2, "Progress: [");
    wattron(progress_window, A_REVERSE);
    for (int i = 0; i < filled; i++) waddch(progress_window, ' ');
    wattroff(progress_window, A_REVERSE);
    for (int i = filled; i < bar_width; i++) waddch(progress_window, '-');
    wprintw(progress_window, "] %d%%", percentage);

    // Display the specific process step and count
    mvwprintw(progress_window, 5, 2, "Step:  %-40s", current_process);
    mvwprintw(progress_window, 7, 2, "Items: %-10d", files_found);

    wrefresh(progress_window);

    // Switch to results screen when scan is complete
    if (percentage == 100 && strcmp(current_process, "Finalizing results") == 0) {
        switch_to_results_view();
    }
}

//Shows the duplicate file results screen
void show_results_screen() {
     werase(results_window);
    box(results_window, 0, 0);
    mvwprintw(results_window, 1, 2, "Duplicate File Sets:");
    mvwprintw(results_window, 3, 2, "Results will be here ");

    wrefresh(results_window);
}

//Switches to the scan progress view
void switch_to_progress_view() {
    // Hide the directory selection window
    if(dir_window != NULL){
        wbkgd(dir_window, COLOR_PAIR(0)); // Reset background color
        wclear(dir_window);
        wrefresh(dir_window);
        delwin(dir_window);
        dir_window = NULL;
    }

    // Show the scan progress window
    show_scan_progress_screen();
}

// Switches to the results view
void switch_to_results_view() {
    // Hide the progress window
    if (progress_window != NULL) {
        wbkgd(progress_window, COLOR_PAIR(0));
        wclear(progress_window);
        wrefresh(progress_window);
        delwin(progress_window);
        progress_window = NULL;
    }

    // Show the results window
    show_results_screen();
}

// Handles directory selection navigation
void navigate_directory_selection() {
    int ch;
    while ((ch = getch()) != 27) {
        FileInfo *sel = get_info_at_index(selected_dir);
        if (ch == KEY_UP && selected_dir > 0) selected_dir--;
        else if (ch == KEY_DOWN && selected_dir < directory_list.count - 1) selected_dir++;
        else if (ch == 32 && sel && strcmp(sel->name, "..") != 0) {
            char t[1024];
            if (!strlen(current_browsing_path)) strcpy(t, sel->name);
            else sprintf(t, "%s\\%s", current_browsing_path, sel->name);
            if (show_confirmation_popup(t)) { 
                strcpy(final_selected_path, t);
                switch_to_progress_view(); // Switch to progress view
                break;
            }
        } else if (ch == 10 && sel && dir_window != NULL) {
            update_path(strcmp(sel->name, "..") == 0 ? NULL : sel->name);
            populate_directory_list(current_browsing_path);
            selected_dir = scroll_offset = 0;
            show_directory_selection_screen();
        } else if (ch == KEY_RESIZE) {
            handle_terminal_resize();
             mvwin(dir_window, (LINES - 18) / 2, (COLS - 70) / 2);
             mvwin(progress_window, (LINES - 10) / 2, (COLS - 60) / 2);
        }
        if(dir_window != NULL)
        	show_directory_selection_screen();
    }
}

// Handles terminal resize
void handle_terminal_resize() { 
    resizeterm(LINES, COLS); 
    clear(); 
    refresh();
}

// --- UI Memory Management ---
// Initializes the file list
void init_file_list(FileInfoList *l) { l->head = NULL; l->count = 0; }

// Adds a file to the list
void add_to_list(FileInfoList *l, const char *n, int d) {
    FileInfo *node = malloc(sizeof(FileInfo));
    strncpy(node->name, n, 255); node->is_directory = d; node->next = NULL;
    if (!l->head) l->head = node;
    else { FileInfo *tmp = l->head; while (tmp->next) tmp = tmp->next; tmp->next = node; }
    l->count++;
}

// Frees the UI list
void free_ui_list(FileInfoList *l) {
    FileInfo *c = l->head;
    while (c) { FileInfo *n = c->next; free(c); c = n; }
    l->head = NULL; l->count = 0;
}

// Gets the file info at a given index
FileInfo* get_info_at_index(int i) {
    FileInfo *c = directory_list.head;
    while (i-- > 0 && c) c = c->next;
    return c;
}