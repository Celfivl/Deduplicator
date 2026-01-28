// tui_interface.c
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "tui_interface.h"

#ifdef _WIN32
#include <windows.h>
#endif

// --- Global UI State ---
WINDOW *dir_window = NULL, *progress_window = NULL, *results_window = NULL;
FileInfoList directory_list;
int selected_dir = 0, scroll_offset = 0;
char final_selected_path[1024] = "";

#ifdef _WIN32
char current_browsing_path[1024] = "";
#else
char current_browsing_path[1024] = "/";
#endif

// --- Helper: Frame & Branding ---
static void draw_window_frame(WINDOW *win, const char *title, const char *legend) {
    int h, w;
    getmaxyx(win, h, w);
    werase(win);
    box(win, 0, 0);
    
    if (title) mvwprintw(win, 0, 2, " %s ", title);
    if (legend) mvwprintw(win, h - 2, 2, " %s ", legend);

    const char *footer = "Copyright 2026 G. Melancon JR";
    int footer_len = (int)strlen(footer);
    
    if (w > footer_len + 4) {
        attron(A_DIM);
        mvwprintw(win, h - 1, w - footer_len - 2, "%s", footer);
        attroff(A_DIM);
    }
}

// --- Helper: Path Management ---
static void update_path(const char* addition) {
    if (!addition) {
        char *last = strrchr(current_browsing_path, '\\');
        if (!last) last = strrchr(current_browsing_path, '/');
        if (last) {
            if (last == current_browsing_path + 2 && current_browsing_path[1] == ':') current_browsing_path[0] = '\0';
            else *last = '\0';
        } else current_browsing_path[0] = '\0';
    } else {
        size_t len = strlen(current_browsing_path);
        if (len > 0 && current_browsing_path[len-1] != '\\' && current_browsing_path[len-1] != '/') 
            strcat(current_browsing_path, "\\");
        strcat(current_browsing_path, addition);
    }
}

// --- Unified Popup Logic ---
int show_popup(const char *msg, int is_confirm) {
    WINDOW *p = newwin(7, 60, (LINES - 7) / 2, (COLS - 60) / 2);
    box(p, 0, 0);
    mvwprintw(p, 2, 2, "%.54s", msg);
    
    if (is_confirm) mvwprintw(p, 4, 15, "[Y] Confirm / [N] Cancel");
    else mvwprintw(p, 4, 15, "Press [ENTER] to continue...");
    
    wrefresh(p);
    int ret = 0, ch;
    
    while(1) {
        ch = wgetch(p);
        if (is_confirm) {
            if (ch == 'y' || ch == 'Y') { ret = 1; break; }
            if (ch == 'n' || ch == 'N' || ch == 27) { ret = 0; break; }
        } else {
            if (ch == 10 || ch == 13 || ch == ' ') break; 
        }
    }
    delwin(p);
    return ret;
}

int init_tui() {
    initscr();
    if (!stdscr) return 1;
    noecho(); cbreak(); curs_set(0); keypad(stdscr, TRUE);
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    refresh(); 

    int h = (LINES * 95) / 100, w = (COLS * 95) / 100;
    int sy = (LINES - h) / 2, sx = (COLS - w) / 2;

    dir_window = newwin(h, w, sy, sx); keypad(dir_window, TRUE);
    results_window = newwin(h, w, sy, sx); keypad(results_window, TRUE);
    progress_window = newwin(10, 60, (LINES - 10) / 2, (COLS - 60) / 2);

    init_file_list(&directory_list);
    populate_directory_list(current_browsing_path);
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

void show_directory_selection_screen() {
    int h, w; getmaxyx(dir_window, h, w);
    int max = h - 4;
    if (selected_dir < scroll_offset) scroll_offset = selected_dir;
    else if (selected_dir >= scroll_offset + max) scroll_offset = selected_dir - max + 1;

    draw_window_frame(dir_window, current_browsing_path, "[ENTER] Open  [SPACE] Select  [Q] Exit");

    FileInfo *curr = directory_list.head;
    for (int i = 0; i < scroll_offset && curr; i++) curr = curr->next;
    for (int i = 0; i < max && curr; i++) {
        if (scroll_offset + i == selected_dir) wattron(dir_window, A_REVERSE);
        mvwprintw(dir_window, i + 2, 2, " [DIR] %-50.50s", curr->name);
        wattroff(dir_window, A_REVERSE);
        curr = curr->next;
    }
    wrefresh(dir_window);
}

void navigate_directory_selection() {
    while (1) {
        show_directory_selection_screen();
        int ch = wgetch(dir_window);
        FileInfo *sel = get_info_at_index(selected_dir);

        if (ch == 'q' || ch == 'Q') { final_selected_path[0] = '\0'; break; }
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

void update_scan_progress(const char *step, int percentage, int found) {
    draw_window_frame(progress_window, "Engine Status", NULL);
    int bar_width = 30, filled = (percentage * bar_width) / 100;
    mvwprintw(progress_window, 2, 2, "Step: %-20s", step);
    mvwprintw(progress_window, 4, 2, "Progress: [");
    wattron(progress_window, COLOR_PAIR(1));
    for (int i = 0; i < filled; i++) waddch(progress_window, '#');
    wattroff(progress_window, COLOR_PAIR(1));
    for (int i = filled; i < bar_width; i++) waddch(progress_window, '.');
    wprintw(progress_window, "] %d%%", percentage);
    mvwprintw(progress_window, 6, 2, "Files: %d", found);
    wrefresh(progress_window);
}

void show_results_screen(MarkedFile *res, int total, int cursor, int *marks, int scroll_offset) {
    int h, w; getmaxyx(results_window, h, w);
    draw_window_frame(results_window, "Analysis Results", "[SPACE] Mark  [D] Delete  [Q] Back");
    
    int max_visible_lines = h - 4;
    int y = 2;

    for (int i = scroll_offset; i < total; i++) {
        if (y >= h - 2) break;

        if (i == 0 || res[i].group_id != res[i-1].group_id) {
            if (y >= h - 2) break;
            wattron(results_window, COLOR_PAIR(2) | A_BOLD);
            mvwprintw(results_window, y++, 2, "--- Group %d ---", res[i].group_id);
            wattroff(results_window, COLOR_PAIR(2) | A_BOLD);
        }

        if (y >= h - 2) break;

        if (i == cursor) wattron(results_window, A_REVERSE);
        mvwprintw(results_window, y++, 2, " [%c] %s", marks[i] ? 'X' : ' ', res[i].path);
        if (i == cursor) wattroff(results_window, A_REVERSE);
    }
    wrefresh(results_window);
}

int navigate_results_view(MarkedFile *res, int total) {
    int cursor = 0, ret_code = 0;
    int result_scroll = 0;
    int *marks = calloc(total, sizeof(int));

    while (1) {
        int h, w; getmaxyx(results_window, h, w);
        int max_visible = h - 5;

        if (cursor < result_scroll) result_scroll = cursor;
        else if (cursor >= result_scroll + max_visible) result_scroll = cursor - max_visible + 1;

        show_results_screen(res, total, cursor, marks, result_scroll);
        
        int ch = wgetch(results_window);

        if (ch == 'q' || ch == 'Q') { ret_code = 0; break; }
        if (ch == KEY_UP && cursor > 0) cursor--;
        if (ch == KEY_DOWN && cursor < total - 1) cursor++;
        if (ch == ' ') marks[cursor] = !marks[cursor];
        
        if (ch == 'd' || ch == 'D') {
            if (show_popup("Delete marked files?", 1)) {
                for(int i=0; i<total; i++) if(marks[i]) remove(res[i].path);
                show_popup("Deletion Complete.", 0);
                ret_code = 1; 
                break;
            }
        }
    }
    free(marks);
    return ret_code;
}

// --- List Helpers ---
void init_file_list(FileInfoList *l) { l->head = NULL; l->tail = NULL; l->count = 0; }
void add_to_list(FileInfoList *l, const char *n, int d) {
    FileInfo *node = malloc(sizeof(FileInfo));
    strncpy(node->name, n, 255); node->is_directory = d; node->next = NULL;
    if (l->tail) l->tail->next = node; else l->head = node;
    l->tail = node; l->count++;
}
void free_ui_list(FileInfoList *l) {
    FileInfo *c = l->head;
    while (c) { FileInfo *n = c->next; free(c); c = n; }
    l->head = l->tail = NULL; l->count = 0;
}
FileInfo* get_info_at_index(int i) {
    FileInfo *c = directory_list.head;
    while (i-- > 0 && c) c = c->next;
    return c;
}