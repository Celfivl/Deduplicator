// filesystem.c
#include "filesystem.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h> // Required for PATH_MAX

FileEntry* create_file_entry(const char *path) {
    if (!path) return NULL;

    FileEntry *entry = malloc(sizeof(FileEntry));
    if (!entry) return NULL;

    entry->path = strdup(path);
    if (!entry->path) {
        free(entry);
        return NULL;
    }

    struct stat file_stat;
    if (stat(path, &file_stat) == 0) {
        entry->size = file_stat.st_size;
        entry->last_modified = file_stat.st_mtime;
        entry->is_directory = S_ISDIR(file_stat.st_mode);
    } else {
        // Cleanup if file stats are inaccessible (e.g., permission denied)
        free(entry->path);
        free(entry);
        return NULL;
    }

    return entry;
}

void free_file_entry(FileEntry *entry) {
    if (entry) {
        free(entry->path);
        free(entry);
    }
}

FileEntry** scan_directory_recursive(const char *path, int *num_files) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        *num_files = 0;
        return NULL;
    }

    struct dirent *entry;
    FileEntry **file_list = NULL;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        // Prevent infinite recursion by skipping self and parent references
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        FileEntry *new_entry = create_file_entry(full_path);
        if (!new_entry) continue;

        if (new_entry->is_directory) {
            int sub_num = 0;
            FileEntry **sub_files = scan_directory_recursive(full_path, &sub_num);
            
            if (sub_files) {
                // Expand main list to accommodate found sub-directory files
                FileEntry **tmp = realloc(file_list, sizeof(FileEntry*) * (count + sub_num));
                if (!tmp) {
                    for (int i = 0; i < sub_num; i++) free_file_entry(sub_files[i]);
                    free(sub_files);
                    goto cleanup_error;
                }
                file_list = tmp;
                
                // Transfer ownership of pointers from sub_files array to file_list
                for (int i = 0; i < sub_num; i++) file_list[count++] = sub_files[i];
                free(sub_files); // Free temporary container, not the entries themselves
            }
            free_file_entry(new_entry); // Directory entries are scanned, not stored
        } else {
            FileEntry **tmp = realloc(file_list, sizeof(FileEntry*) * (count + 1));
            if (!tmp) goto cleanup_error;
            
            file_list = tmp;
            file_list[count++] = new_entry;
        }
    }

    closedir(dir);
    *num_files = count;
    return file_list;

cleanup_error:
    // Centralized cleanup to prevent memory leaks on allocation failure
    perror("realloc/mem_error");
    for (int i = 0; i < count; i++) free_file_entry(file_list[i]);
    free(file_list);
    closedir(dir);
    *num_files = 0;
    return NULL;
}

FileEntry** scan_directory(const char *path, int *num_files) {
    return scan_directory_recursive(path, num_files);
}