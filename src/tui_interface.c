// tui_interface.c
#include <ncurses.h>
#include "tui_interface.h"
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

WINDOW *main_window = NULL; // Define the pointer
WINDOW *dir_window = NULL;

void init_file_list(FileInfoList *list) {
    list->head = NULL;
    list->count = 0;
}

FileInfoList directory_list; // Declare a FileInfoList
int selected_dir = 0;

// Add a global variable to store the result
char final_selected_path[1024] = "";

// Helper function to add a directory to the FileInfoList
void add_to_list(FileInfoList *fileList, const char *name) {
    FileInfo *new_file = (FileInfo *)malloc(sizeof(FileInfo));
    if (new_file == NULL) {
        perror("Memory allocation failed");
        return;
    }
    strncpy(new_file->name, name, sizeof(new_file->name) - 1);
    new_file->name[sizeof(new_file->name) - 1] = '\0';
    new_file->is_directory = 1; // Assuming all entries are directories
    new_file->next = fileList->head;
    fileList->head = new_file;
    fileList->count++;
}

// Helper function to free a FileEntry
void free_file_entry(FileEntry *fileEntry) {
    free(fileEntry);
}

void populate_directory_list(const char *path) {
    init_file_list(&directory_list); // Initialize your TUI linked list
    
    int count = 0;
    // Call the "Untouched" version of scan_directory from filesystem.c
    FileEntry** results = scan_directory(path, &count); 

    if (results != NULL) {
        for (int i = 0; i < count; i++) {
            // Transform the array results into your TUI's linked list
            // Assuming add_to_list is a helper in your tui_interface or a shared header
            add_to_list(&directory_list, results[i]->path); 
            free_file_entry(results[i]); // Clean up the FileEntry as we migrate data
        }
        free(results); // Free the array pointer itself
    }
}

void show_directory_selection_screen() {
    int height = 15;
    int width = 60;
    int start_y = (LINES - height) / 2;
    int start_x = (COLS - width) / 2;

    // Delete the old window if it exists
    if (dir_window != NULL) {
        delwin(dir_window);
    }

    dir_window = newwin(height, width, start_y, start_x);
    box(dir_window, 0, 0);

    mvwprintw(dir_window, 1, 2, "Select Scan Directory:");

    // Display the list of directories
    FileInfo *current = directory_list.head;
    int i = 0;
    while (current != NULL && i < height - 4) { // added i < height - 4 condition
        if (i == selected_dir) {
            wattron(dir_window, A_REVERSE); // Highlight selected directory
            mvwprintw(dir_window, i + 3, 2, current->name);
            wattroff(dir_window, A_REVERSE);
        } else {
            mvwprintw(dir_window, i + 3, 2, current->name);
        }
        current = current->next;
        i++;
    }

    wrefresh(dir_window);
}

void navigate_directory_selection() {
    int ch;
    while((ch = getch()) != 10) { // 10 is Enter
        switch(ch) {
            case KEY_UP:
                if (selected_dir > 0) selected_dir--;
                break;
            case KEY_DOWN:
                if (selected_dir < directory_list.count - 1) selected_dir++;
                break;
        }
        show_directory_selection_screen(); 
    }

    // Capture the name of the selection before the list is freed in end_tui
    FileInfo *current = directory_list.head;
    for(int i = 0; i < selected_dir && current != NULL; i++) {
        current = current->next;
    }
    if (current) {
        strncpy(final_selected_path, current->name, 1023);
    }
}

int init_tui() {
    initscr(); // Initialize ncurses
    cbreak();  // Disable line buffering
    noecho();  // Don't echo user input
    keypad(stdscr, TRUE); // Enable special keys

    // Initialize the physical screen
    refresh();

    // Create windows for the layout (example)
    main_window = newwin(15, 60, 1, 1); 
    
    if (main_window != NULL) {
        box(main_window, 0, 0); // Draw a box around the window
        
        mvwprintw(main_window, 1, 2, " Deduplicator TUI Test ");
        
        wrefresh(main_window);
    }

    populate_directory_list("."); // Populate the directory list
    show_directory_selection_screen();
    navigate_directory_selection();

    return 0;
}

void end_tui() {
    // Free each FileInfo in the list
    FileInfo *current = directory_list.head;
    while (current != NULL) {
        FileInfo *next = current->next;
        free(current);
        current = next;
    }

    // Reset the list
    directory_list.head = NULL;
    directory_list.count = 0;
    
    if (dir_window != NULL) {
        delwin(dir_window);
    }
    if (main_window != NULL) {
        delwin(main_window); // Properly delete the window
    }
    endwin(); // End ncurses
}