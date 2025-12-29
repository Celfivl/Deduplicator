// datastruct.h
#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include "filesystem.h"
#include <stddef.h>

// Dynamic list to hold FileEntry pointers
typedef struct FileList {
    FileEntry **entries;
    size_t size;
    size_t capacity;
} FileList;

// Functions to manage FileList
FileList* create_file_list();
int add_file_entry(FileList *list, FileEntry *entry);
void free_file_list(FileList *list);

#endif