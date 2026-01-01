// deduplicator.h
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

// Structure to represent a duplicate file pair
typedef struct DuplicatePair {
    char *file1;
    char *file2;
    struct DuplicatePair *next;
} DuplicatePair;

// Group files by size
SizeGroup *group_files_by_size(FileEntry **files, int num_files);

// Function to find duplicate files in a directory
DuplicatePair *find_duplicates(const char *directory_path);

// Function to compare two files byte by byte
int compare_files(const char *file1, const char *file2);

#endif