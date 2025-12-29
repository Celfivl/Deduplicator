// datastruct.c
#include "datastruct.h"
#include <stdlib.h>

#define INITIAL_CAPACITY 10

FileList* create_file_list() {
    FileList *list = (FileList*)malloc(sizeof(FileList));
    if (!list) return NULL;

    list->entries = (FileEntry**)malloc(sizeof(FileEntry*) * INITIAL_CAPACITY);
    if (!list->entries) {
        free(list);
        return NULL;
    }

    list->size = 0;
    list->capacity = INITIAL_CAPACITY;
    return list;
}

int add_file_entry(FileList *list, FileEntry *entry) {
    if (!list || !entry) return -1;

    if (list->size == list->capacity) {
        size_t new_capacity = list->capacity * 2;
        FileEntry **new_entries = (FileEntry**)realloc(list->entries, sizeof(FileEntry*) * new_capacity);
        if (!new_entries) return -1;
        
        list->entries = new_entries;
        list->capacity = new_capacity;
    }

    list->entries[list->size++] = entry;
    return 0;
}

void free_file_list(FileList *list) {
    if (list) {
        if (list->entries) {
            for (size_t i = 0; i < list->size; i++) {
                // Calls back to the existing filesystem.c logic
                free_file_entry(list->entries[i]); 
            }
            free(list->entries);
        }
        free(list);
    }
}