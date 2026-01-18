// filesystem.h
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/types.h>
#include <time.h>
#include <stdbool.h>

// Represents a file, its metadata, and its unique content identifier
typedef struct FileEntry {
    char *path;
    off_t size;
    time_t last_modified;
    bool is_directory;
} FileEntry;

FileEntry* create_file_entry(const char *path);
void free_file_entry(FileEntry *entry);

// Returns a dynamically allocated array of FileEntry pointers
FileEntry** scan_directory(const char *path, int *num_files);

#endif