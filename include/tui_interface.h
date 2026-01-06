// tui_interface.h
#ifndef TUI_INTERFACE_H
#define TUI_INTERFACE_H

#include <ncurses.h>
#include "filesystem.h"

// --- UI-Specific Types (Not in filesystem.h) ---

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
extern char final_selected_path[1024];

// --- Core Lifecycle ---
int init_tui();
void end_tui();

// --- Selection Logic ---
void populate_directory_list(const char *path);
void show_directory_selection_screen();
void navigate_directory_selection();

// --- UI Memory Management ---
void init_file_list(FileInfoList *list);
void add_to_list(FileInfoList *fileList, const char *name);

#endif