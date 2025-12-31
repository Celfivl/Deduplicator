#ifndef DEDUPLICATOR_H
#define DEDUPLICATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h" // Include the FileList and FileEntry structures

typedef struct SizeGroup {
    off_t size;
    char **filepaths;   // SizeGroup owns these (via strdup)
    size_t count;
    size_t capacity;
    struct SizeGroup *next;
} SizeGroup;

// Group files by size
SizeGroup *group_files_by_size(FileEntry **files, int num_files);

// Convenience wrapper for a directory path
SizeGroup *find_matched_files(const char *directory_path);

#endif