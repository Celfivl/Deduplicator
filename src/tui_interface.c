// tui_interface.c
#include <ncurses.h>
#include "tui_interface.h"
#include "deduplicator.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#endif

WINDOW *main_window = NULL, *dir_window = NULL, *progress_window = NULL, *results_window = NULL; 
FileInfoList directory_list;
int selected_dir = 0, scroll_offset = 0;
char final_selected_path[1024] = "";

#ifdef _WIN32
char current_browsing_path[1024] = ""; 
#else
char current_browsing_path[1024] = "/";
#endif

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

int init_tui() {
    initscr();
    if (!stdscr) return 1;
    noecho(); cbreak(); curs_set(0); keypad(stdscr, TRUE);
    init_file_list(&directory_list);
    populate_directory_list(current_browsing_path);

    int h_95 = (LINES * 95) / 100, w_95 = (COLS * 95) / 100;
    int start_y = (LINES - h_95) / 2, start_x = (COLS - w_95) / 2;

    dir_window = newwin(h_95, w_95, start_y, start_x);
    results_window = newwin(h_95, w_95, start_y, start_x);

    int ph = 10, pw = 60;
    progress_window = newwin(ph, pw, (LINES - ph) / 2, (COLS - pw) / 2); 

    return 0;
}

void end_tui() {
    free_ui_list(&directory_list);
    if (dir_window) delwin(dir_window);
    if (progress_window) delwin(progress_window); 
    if (results_window) delwin(results_window);
    endwin();
}

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

int show_confirmation_popup(const char *msg) {
    WINDOW *p = newwin(7, 60, (LINES - 7) / 2, (COLS - 60) / 2);
    box(p, 0, 0);
    mvwprintw(p, 2, 2, "%-56.56s", msg);
    mvwprintw(p, 5, 15, "[Y] Confirm / [N] Cancel");
    wrefresh(p);
    int c = wgetch(p);
    delwin(p);
    return (c == 'y' || c == 'Y');
}

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
    mvwprintw(dir_window, h-2, 2, " [ENTER] Open  [SPACE] Select  [Q] Exit ");
    wrefresh(dir_window);
}

void update_scan_progress(const char *current_process, int percentage, int files_found) {
    if (!progress_window) return;
    werase(progress_window);
    box(progress_window, 0, 0);
    mvwprintw(progress_window, 1, 2, "Deduplicator Engine: Active");

    int bar_width = 30, filled = (percentage * bar_width) / 100;
    mvwprintw(progress_window, 3, 2, "Progress: [");
    wattron(progress_window, A_REVERSE);
    for (int i = 0; i < filled; i++) waddch(progress_window, ' ');
    wattroff(progress_window, A_REVERSE);
    for (int i = filled; i < bar_width; i++) waddch(progress_window, '-');
    wprintw(progress_window, "] %d%%", percentage);

    mvwprintw(progress_window, 5, 2, "Step:  %-40s", current_process);
    mvwprintw(progress_window, 7, 2, "Items: %-10d", files_found);
    wrefresh(progress_window);
}

void show_results_screen(MarkedFile *results, int total, int cursor, int *marks) {
    if (!results_window) return;

    int h, w;
    getmaxyx(results_window, h, w);

    werase(results_window);
    box(results_window, 0, 0);
    
    // Header 
    mvwprintw(results_window, 0, 2, " Duplicate Discovery Results ");
    mvwprintw(results_window, 1, 2, " [Space] Mark for Delete | [D] Confirm | [Q] Exit ");

    int current_y = 2; // Starting content row

    for (int i = 0; i < total; i++) {
        // --- Neutral Group Header ---
        if (i == 0 || results[i].group_id != results[i-1].group_id) {
            current_y++; // Add a gap before new group
            wattron(results_window, A_BOLD | COLOR_PAIR(2));
            mvwprintw(results_window, current_y++, 2, "[ Group %d - Matching Content ]", results[i].group_id);
            wattroff(results_window, A_BOLD | COLOR_PAIR(2));
        }

        if (i == cursor) {
            wattron(results_window, A_REVERSE);
        }

        const char *status = marks[i] ? "[X] DELETE" : "[ ]       ";
        
        mvwprintw(results_window, current_y++, 2, "  %s %s", status, results[i].path);

        if (i == cursor) {
            wattroff(results_window, A_REVERSE);
        }

        if (current_y >= h - 2) break;
    }

    mvwprintw(results_window, h - 2, 2, " [SPACE] Toggle  [D] Process  [Q] Back ");

    wnoutrefresh(results_window);
    doupdate();
}

void navigate_directory_selection() {
    int ch;
    while (1) {
        show_directory_selection_screen();
        ch = getch();
        FileInfo *sel = get_info_at_index(selected_dir);

        if (ch == 'q' || ch == 'Q' || ch == 27) {
            final_selected_path[0] = '\0';
            break;
        }
        if (ch == KEY_UP && selected_dir > 0) selected_dir--;
        else if (ch == KEY_DOWN && selected_dir < directory_list.count - 1) selected_dir++;
        else if (ch == ' ' && sel && strcmp(sel->name, "..") != 0) {
            if (!strlen(current_browsing_path)) strcpy(final_selected_path, sel->name);
            else sprintf(final_selected_path, "%s\\%s", current_browsing_path, sel->name);
            break;
        } else if (ch == 10 && sel) {
            update_path(strcmp(sel->name, "..") == 0 ? NULL : sel->name);
            populate_directory_list(current_browsing_path);
            selected_dir = scroll_offset = 0;
        }
    }
}

void init_file_list(FileInfoList *l) { l->head = NULL; l->count = 0; }

void add_to_list(FileInfoList *l, const char *n, int d) {
    FileInfo *node = malloc(sizeof(FileInfo));
    strncpy(node->name, n, 255); node->is_directory = d; node->next = NULL;
    if (!l->head) l->head = node;
    else { FileInfo *tmp = l->head; while (tmp->next) tmp = tmp->next; tmp->next = node; }
    l->count++;
}

void free_ui_list(FileInfoList *l) {
    FileInfo *c = l->head;
    while (c) { FileInfo *n = c->next; free(c); c = n; }
    l->head = NULL; l->count = 0;
}

FileInfo* get_info_at_index(int i) {
    FileInfo *c = directory_list.head;
    while (i-- > 0 && c) c = c->next;
    return c;
}