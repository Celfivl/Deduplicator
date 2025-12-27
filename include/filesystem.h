// filesystem.h
// Header for file system scanning module.
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/types.h>
#include <time.h>
#include <dirent.h> // Include for directory operations
#include <sys/stat.h> // Include for file stat operations

// Structure to hold file information.
typedef struct FileEntry {
    char *path;
    off_t size;
    time_t last_modified;
    int is_directory; // 1 if directory, 0 if file
} FileEntry;

FileEntry* create_file_entry(const char *path);
void free_file_entry(FileEntry *entry);

// New function to scan a directory
FileEntry** scan_directory(const char *path, int *num_files);

#endif