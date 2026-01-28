// tui_interface.h
#ifndef TUI_INTERFACE_H
#define TUI_INTERFACE_H

#include <ncurses.h>
#include "deduplicator.h"

// --- UI Types ---
typedef struct FileInfo {
    char name[256];
    int is_directory;
    struct FileInfo *next;
} FileInfo;

typedef struct {
    FileInfo *head;
    FileInfo *tail;
    int count;
} FileInfoList;

// --- Global UI State ---
extern WINDOW *dir_window, *progress_window, *results_window;
extern char final_selected_path[1024];
extern char current_browsing_path[1024];

// --- Core Lifecycle ---
int init_tui();
void end_tui();

// --- Navigation & Views ---
void populate_directory_list(const char *path);
void show_directory_selection_screen();
void navigate_directory_selection();
void update_scan_progress(const char *step, int percentage, int found);
int navigate_results_view(MarkedFile *res, int total);

// --- Shared Helpers ---
// is_confirm: 1 = [Y/N] prompt, 0 = [ENTER] info only
int show_popup(const char *msg, int is_confirm); 

void init_file_list(FileInfoList *l);
void add_to_list(FileInfoList *l, const char *n, int d);
void free_ui_list(FileInfoList *l);
FileInfo* get_info_at_index(int i);

#endif