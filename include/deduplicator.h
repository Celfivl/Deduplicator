// deduplicator.h
#ifndef DEDUPLICATOR_H
#define DEDUPLICATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to represent a duplicate file pair (place holder, not used yet)
typedef struct DuplicatePair {
    char *file1;
    char *file2;
    struct DuplicatePair *next;
} DuplicatePair;

// Structure to represent a file pair
typedef struct FilePair {
    char *file1;
    char *file2;
    struct FilePair *next;
} FilePair;

// Function to find files with matched sizes in a directory
FilePair *find_matched_files(const char *directory_path);

#endif