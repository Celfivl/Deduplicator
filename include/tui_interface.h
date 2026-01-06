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
extern WINDOW *progress_window; // Added progress_window
extern char final_selected_path[1024];
extern char current_browsing_path[1024];

// --- Core Lifecycle ---
int init_tui();
void end_tui();

// --- Selection & Navigation Logic ---
void populate_directory_list(const char *path);
void show_directory_selection_screen();
void navigate_directory_selection();
void handle_terminal_resize();

// --- Scan Progress Logic ---
void show_scan_progress_screen(); // Added show_scan_progress_screen
void update_scan_progress(const char *file_name, int percentage, const char *message); // Added update_scan_progress
void switch_to_progress_view(); // Added switch_to_progress_view

// --- UI Memory & Helper Management ---
void init_file_list(FileInfoList *list);
void add_to_list(FileInfoList *fileList, const char *name, int is_dir);
void free_ui_list(FileInfoList *list);
FileInfo* get_info_at_index(int index);

#endif