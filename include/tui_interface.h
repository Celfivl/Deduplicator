// tui_interface.h
#ifndef TUI_INTERFACE_H
#define TUI_INTERFACE_H

#include <ncurses.h>
#include "filesystem.h"

// --- UI-Specific Types ---
typedef struct FileInfo {
    char name[256];
    int is_directory;
    struct FileInfo *next;
} FileInfo;

typedef struct {
    FileInfo *head;
    int count;
} FileInfoList;

// --- Global UI State ---
extern WINDOW *main_window; 
extern WINDOW *dir_window;
extern WINDOW *progress_window;
extern WINDOW *results_window;
extern char final_selected_path[1024];
extern char current_browsing_path[1024];

// --- Core Lifecycle ---
int init_tui();
void end_tui();

// --- Selection & Navigation Logic ---
void populate_directory_list(const char *path);
void show_directory_selection_screen();
void navigate_directory_selection();
void handle_terminal_resize(); // Ensure this is present

// --- Scan Progress Logic ---
void show_scan_progress_screen();
void update_scan_progress(const char *current_process, int percentage, int files_found);
void switch_to_progress_view();
void switch_to_results_view();

// --- Results View Logic ---
void show_results_screen(int cursor, int *marks);    // Added
void navigate_results_view();   // Added

// --- UI Memory & Helper Management ---
int show_confirmation_popup(const char *msg);
void init_file_list(FileInfoList *list);
void add_to_list(FileInfoList *fileList, const char *name, int is_dir);
void free_ui_list(FileInfoList *list);
FileInfo* get_info_at_index(int index);

#endif