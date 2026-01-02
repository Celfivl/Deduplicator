#ifndef DEDUPLICATOR_H
#define DEDUPLICATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

typedef struct SizeGroup {
    off_t size;
    char **filepaths;
    size_t count;
    size_t capacity;
    struct SizeGroup *next;
} SizeGroup;

typedef struct DuplicatePair {
    char *file1;
    char *file2;
    struct DuplicatePair *next;
} DuplicatePair;

typedef struct MarkedFile {
    char *path;
    int is_duplicate;  
} MarkedFile;

SizeGroup *group_files_by_size(FileEntry **files, int num_files);
DuplicatePair *find_duplicates(const char *directory_path);
int compare_files(const char *file1, const char *file2);

 // Maps DuplicatePair results back to a flat array of MarkedFile structs.
 // Used for TUI display and selection.
void mark_duplicates(char **all_paths, int num_paths, DuplicatePair *pairs_head, MarkedFile *out_marks);

// Safely frees the linked list of DuplicatePairs and their strdup'd paths.
void free_duplicate_pairs(DuplicatePair *head);

#endif