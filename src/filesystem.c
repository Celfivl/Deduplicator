#include "filesystem.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>   // For printing errors
#include <errno.h>   // For error handling

#define INITIAL_CAPACITY 10 // Initial capacity of the dynamic array

FileEntry* create_file_entry(const char *path) {
    if (path == NULL) {
        return NULL;
    }

    FileEntry *entry = (FileEntry*)malloc(sizeof(FileEntry));
    if (entry == NULL) {
        return NULL;
    }

    entry->path = strdup(path); // Allocate memory and copy the path
    if (entry->path == NULL) {
        free(entry);
        return NULL;
    }

    struct stat file_stat;
    if (stat(path, &file_stat) == 0) {
        entry->size = file_stat.st_size;
        entry->last_modified = file_stat.st_mtime;
        entry->is_directory = S_ISDIR(file_stat.st_mode); // Set is_directory flag
    } else {
        // Handle error: could not get file stats
        free(entry->path);
        free(entry);
        return NULL;
    }

    return entry;
}

void free_file_entry(FileEntry *entry) {
    if (entry) {
        free(entry->path); // Free the duplicated path
        free(entry);        // Free the entry itself
    }
}

// Updated Helper: No return value, just takes the list pointer
void scan_directory_recursive(const char *path, FileList *list) {
    DIR *dir = opendir(path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) continue;

        if (S_ISDIR(statbuf.st_mode)) {
            scan_directory_recursive(full_path, list); // Recursive call passes same list
        } else {
            FileEntry *fe = create_file_entry(full_path);
            if (fe) {
                add_file_entry(list, fe); // Uses your efficient doubling logic!
            }
        }
    }
    closedir(dir);
}

// Updated Main Entrypoint
FileList* scan_directory(const char *path) {
    FileList *list = create_file_list(); // Capacity starts at 10
    if (!list) return NULL;

    scan_directory_recursive(path, list);
    return list;
}

// Function to initialize the file list
FileList* create_file_list() {
    FileList *list = (FileList*)malloc(sizeof(FileList));
    if (list == NULL) {
        return NULL;
    }

    list->entries = (FileEntry**)malloc(sizeof(FileEntry*) * INITIAL_CAPACITY);
    if (list->entries == NULL) {
        free(list);
        return NULL;
    }

    list->size = 0;
    list->capacity = INITIAL_CAPACITY;
    return list;
}

// Function to add a file entry to the list
int add_file_entry(FileList *list, FileEntry *entry) {
    if (list == NULL || entry == NULL) {
        return -1;
    }

    if (list->size == list->capacity) {
        // Need to resize the array
        size_t new_capacity = list->capacity * 2;
        FileEntry **new_entries = (FileEntry**)realloc(list->entries, sizeof(FileEntry*) * new_capacity);
        if (new_entries == NULL) {
            return -1; // Reallocation failed
        }
        list->entries = new_entries;
        list->capacity = new_capacity;
    }

    list->entries[list->size] = entry;
    list->size++;
    return 0;
}

// Function to remove a file entry from the list
int remove_file_entry(FileList *list, FileEntry *entry) {
    if (list == NULL || entry == NULL) {
        return -1;
    }

    // Find the entry in the list
    size_t index = -1;
    for (size_t i = 0; i < list->size; i++) {
        if (list->entries[i] == entry) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return -1; // Entry not found
    }

    // Shift the remaining elements to fill the gap
    for (size_t i = index; i < list->size - 1; i++) {
        list->entries[i] = list->entries[i + 1];
    }

    list->size--;
    return 0;
}

void free_file_list(FileList *list) {
    if (list) {
        if (list->entries) {
            // New: Loop through and free each individual entry
            for (size_t i = 0; i < list->size; i++) {
                free_file_entry(list->entries[i]);
            }
            free(list->entries);
        }
        free(list);
    }
}