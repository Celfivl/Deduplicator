// filesystem.h
// Header for file system scanning module.
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/types.h>

// Structure to hold file information.
typedef struct FileEntry {
    char *path;             
    off_t size;             
    time_t last_modified;   
} FileEntry;

FileEntry* create_file_entry(const char *path);
void free_file_entry(FileEntry *entry);

#endif