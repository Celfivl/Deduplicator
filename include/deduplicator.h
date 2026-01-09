// deduplicator.h
#ifndef DEDUPLICATOR_H
#define DEDUPLICATOR_H

#include <stdio.h>
#include <stdlib.h>
#include "filesystem.h"

// Callback for TUI: message, percentage, and total count 
typedef void (*ProgressCallback)(const char *stage, int percent, int found);

typedef struct {
    char *path;
    int group_id;     // Non-zero ID for matched sets 
    int is_duplicate; // 0 = Original/Anchor, 1 = Redundant copy 
} MarkedFile;

typedef struct SizeGroup {
    off_t size;
    char **filepaths;
    size_t count;
    size_t capacity;
    struct SizeGroup *next;
} SizeGroup;

int compare_files(const char *file1, const char *file2);

// Core logic entry point: Returns flat array for TUI display 
MarkedFile *find_duplicates(const char *path, int *total_out, ProgressCallback cb);
void free_results(MarkedFile *results, int count);

#endif