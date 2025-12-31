#include "deduplicator.h"
#include "filesystem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_GROUP_CAPACITY 4

static SizeGroup *find_group(SizeGroup *head, off_t size) {
    for (SizeGroup *cur = head; cur != NULL; cur = cur->next) {
        if (cur->size == size) return cur;
    }
    return NULL;
}

static SizeGroup *add_group(SizeGroup **head, off_t size) {
    SizeGroup *g = (SizeGroup *)malloc(sizeof(SizeGroup));
    if (!g) {
        perror("malloc SizeGroup");
        exit(EXIT_FAILURE);
    }
    g->size = size;
    g->filepaths = (char **)malloc(INITIAL_GROUP_CAPACITY * sizeof(char *));
    if (!g->filepaths) {
        perror("malloc filepaths");
        exit(EXIT_FAILURE);
    }
    g->count = 0;
    g->capacity = INITIAL_GROUP_CAPACITY;
    g->next = *head;
    *head = g;
    return g;
}

SizeGroup *group_files_by_size(FileEntry **files, int num_files) {
    SizeGroup *head = NULL;
    if (!files || num_files <= 0) return NULL;

    for (int i = 0; i < num_files; i++) {
        FileEntry *f = files[i];
        if (!f) continue;

        SizeGroup *g = find_group(head, f->size);
        if (!g) {
            g = add_group(&head, f->size);
        }

        if (g->count == g->capacity) {
            g->capacity *= 2;
            char **new_paths = (char **)realloc(g->filepaths,
                                                g->capacity * sizeof(char *));
            if (!new_paths) {
                perror("realloc filepaths");
                exit(EXIT_FAILURE);
            }
            g->filepaths = new_paths;
        }

        g->filepaths[g->count] = strdup(f->path);
        if (!g->filepaths[g->count]) {
            perror("strdup path");
            exit(EXIT_FAILURE);
        }
        g->count++;
    }

    return head;
}

SizeGroup *find_matched_files(const char *directory_path) {
    int num_files = 0;
    FileEntry **files = scan_directory(directory_path, &num_files);
    if (!files) return NULL;

    SizeGroup *groups = group_files_by_size(files, num_files);

    // Safe: SizeGroup has its own copies of paths
    for (int i = 0; i < num_files; i++) {
        free_file_entry(files[i]);
    }
    free(files);

    return groups;
}