// filesystem.c
#include "filesystem.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

FileEntry* create_file_entry(const char *path) {
    if (path == NULL) {
        return NULL;
    }

    FileEntry *entry = (FileEntry*)malloc(sizeof(FileEntry));
    if (entry == NULL) {
        return NULL;
    }

    entry->path = strdup(path); // Allocate memory and copy the path
    if (entry->path == NULL) {
        free(entry);
        return NULL;
    }

    struct stat file_stat;
    if (stat(path, &file_stat) == 0) {
        entry->size = file_stat.st_size;
        entry->last_modified = file_stat.st_mtime;
    } else {
        // Handle error: could not get file stats
        free(entry->path);
        free(entry);
        return NULL;
    }

    return entry;
}

void free_file_entry(FileEntry *entry) {
    if (entry) {
        free(entry->path); // Free the duplicated path
        free(entry);        // Free the entry itself
    }
}