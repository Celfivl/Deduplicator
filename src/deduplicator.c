// deduplicator.c
#include "deduplicator.h"
#include "filesystem.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// Structure to store file information (size and path)
typedef struct FileInfo {
    char *filepath;
    off_t size;
    struct FileInfo *next;
} FileInfo;

// Function to find files with matched sizes in a directory
FilePair *find_matched_files(const char *directory_path) {
    // 1. Get all files in the directory
    FilePair *matched_file_list = NULL;
    char **all_files = get_all_files(directory_path);
    if (all_files == NULL) {
        fprintf(stderr, "Error: Could not get file list.\n");
        return NULL;
    }

    // Create a linked list to store file information
    FileInfo *file_list = NULL;
    for (int i = 0; all_files[i] != NULL; i++) {
        struct stat file_stat;
        if (stat(all_files[i], &file_stat) == 0) {
            FileInfo *new_file_info = (FileInfo *)malloc(sizeof(FileInfo));
            if (!new_file_info) {
                perror("Failed to allocate memory for file info");
                exit(EXIT_FAILURE); // Handle memory allocation failure
            }
            new_file_info->filepath = strdup(all_files[i]);
            new_file_info->size = file_stat.st_size;
            new_file_info->next = file_list;
            file_list = new_file_info;
        } else {
            fprintf(stderr, "Error: Could not get file size for %s\n", all_files[i]);
            // Handle the error, maybe skip the file
        }
        free(all_files[i]); // Free the filepath obtained from get_all_files
    }
    free(all_files); // Free the array itself

    // Iterate through the file list and find pairs with matching sizes
    FileInfo *current_file = file_list;
    while (current_file != NULL) {
        FileInfo *inner_file = current_file->next;
        while (inner_file != NULL) {
            if (current_file->size == inner_file->size) {
                // Found a pair with matching sizes
                FilePair *new_pair = (FilePair *)malloc(sizeof(FilePair));
                if (!new_pair) {
                    perror("Failed to allocate memory for file pair");
                    exit(EXIT_FAILURE);
                }
                new_pair->file1 = strdup(current_file->filepath);
                new_pair->file2 = strdup(inner_file->filepath);
                new_pair->next = matched_file_list;
                matched_file_list = new_pair;
            }
            inner_file = inner_file->next;
        }
        current_file = current_file->next;
    }

    // Free the file list
    current_file = file_list;
    while (current_file != NULL) {
        FileInfo *next = current_file->next;
        free(current_file->filepath);
        free(current_file);
        current_file = next;
    }

    return matched_file_list;
}