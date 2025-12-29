// filesystem.h
// Header for file system scanning module.
#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <sys/types.h>
#include <time.h>
#include <dirent.h>    // Include for directory operations
#include <sys/stat.h>  // Include for file stat operations

// Structure to hold file information.
typedef struct FileEntry {
    char *path;
    off_t size;
    time_t last_modified;
    int is_directory; // 1 if directory, 0 if file
} FileEntry;

FileEntry* create_file_entry(const char *path);
void free_file_entry(FileEntry *entry);

// Dynamic array structure
typedef struct FileList {
    FileEntry **entries; // Array of FileEntry pointers
    size_t size;         // Current number of elements
    size_t capacity;     // Maximum number of elements
} FileList;

// Function to initialize the file list
FileList* create_file_list();

// Function to add a file entry to the list
int add_file_entry(FileList *list, FileEntry *entry);

// Function to remove a file entry from the list
int remove_file_entry(FileList *list, FileEntry *entry);

//Function to free the file list and its entries
void free_file_list(FileList *list);

// New function to scan a directory
FileList* scan_directory(const char *path);

#endif