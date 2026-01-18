// deduplicator.h
#ifndef DEDUPLICATOR_H
#define DEDUPLICATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> // Ensure off_t is defined for all compilers
#include "filesystem.h"

// Callback for TUI: message, percentage, and total count 
typedef void (*ProgressCallback)(const char *stage, int percent, int found);

// Represents a file entry to be displayed and acted upon in the TUI
typedef struct {
    char *path;
    int group_id;     // 0 for unique, non-zero for sets of duplicates 
    int is_duplicate; // 0 = First instance found, 1 = Redundant copy 
} MarkedFile;

// Internal bucket for grouping files of identical size before hashing
typedef struct SizeGroup {
    off_t size;       // File size in bytes
    char **filepaths;
    size_t count;
    size_t capacity;
    struct SizeGroup *next;
} SizeGroup;

// Core Logic
int compare_files(const char *file1, const char *file2);
MarkedFile *find_duplicates(const char *path, int *total_out, ProgressCallback cb);
void execute_deletion(MarkedFile *results, int *marks, int total);
void free_results(MarkedFile *results, int count);

#endif