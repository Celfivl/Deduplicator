#include "filesystem.h"
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h> // For printing errors
#include <errno.h> // For error handling

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
        entry->is_directory = S_ISDIR(file_stat.st_mode); // Set is_directory flag
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

// Helper function for recursive directory scanning
FileEntry** scan_directory_recursive(const char *path, int *num_files) {
    DIR *dir;
    struct dirent *entry;
    FileEntry **file_list = NULL;
    int count = 0;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        *num_files = 0;
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue; // Skip current and parent directories
        }

        char full_path[1024]; // Adjust size as needed
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            fprintf(stderr, "Error getting file status for %s: %s\n", full_path, strerror(errno));
            continue; // Skip to the next entry
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Recursively scan subdirectory
            int sub_num_files = 0;
            FileEntry **sub_files = scan_directory_recursive(full_path, &sub_num_files);
            if (sub_files != NULL) {
                // Append sub_files to file_list
                for (int i = 0; i < sub_num_files; i++) {
                     file_list = (FileEntry**)realloc(file_list, sizeof(FileEntry*) * (count + 1));
                     if (file_list == NULL) {
                        perror("realloc");
                        // Free previously allocated entries
                        for (int j = 0; j < count; j++) {
                            free_file_entry(file_list[j]);
                        }
                        for (int j = 0; j < sub_num_files; j++) {
                            free_file_entry(sub_files[j]);
                        }
                        free(file_list);
                        free(sub_files);
                        closedir(dir);
                        *num_files = 0;
                        return NULL;
                    }
                    file_list[count] = sub_files[i];
                    count++;
                }
                free(sub_files); // Free the array, but not the entries
            }
        } else {
            // Allocate memory for the new file entry
            file_list = (FileEntry**)realloc(file_list, sizeof(FileEntry*) * (count + 1));
            if (file_list == NULL) {
                perror("realloc");
                // Free previously allocated entries
                for (int i = 0; i < count; i++) {
                    free_file_entry(file_list[i]);
                }
                free(file_list);
                closedir(dir);
                *num_files = 0;
                return NULL;
            }

            file_list[count] = create_file_entry(full_path);
            if (file_list[count] == NULL) {
                perror("create_file_entry");
                // Free previously allocated entries
                for (int i = 0; i < count; i++) {
                    free_file_entry(file_list[i]);
                }
                free(file_list);
                closedir(dir);
                *num_files = 0;
                return NULL;
            }
            count++;
        }
    }

    closedir(dir);
    *num_files = count;
    return file_list;
}

//Main entrypoint for directory scanning.
FileEntry** scan_directory(const char *path, int *num_files) {
    return scan_directory_recursive(path, num_files);
}