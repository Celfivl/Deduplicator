#include "deduplicator.h"
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

// Helper function to create a test file with a specific size
void create_test_file(const char *filename, size_t size) {
    FILE *fp = fopen(filename, "wb");
    if (fp == NULL) {
        perror("Failed to create test file");
        exit(EXIT_FAILURE);
    }
    fseek(fp, size - 1, SEEK_SET);
    fputc('\0', fp);
    fclose(fp);
}

// Helper function to delete a test file
void delete_test_file(const char *filename) {
    if (remove(filename) != 0) {
        perror("Failed to delete test file");
    }
}

int main() {
    // Define test file names and sizes
    const char *file1 = "test_file_1.txt";
    const char *file2 = "test_file_2.txt";
    const char *file3 = "test_file_3.txt";
    const char *file4 = "test_file_4.txt";
    size_t size1 = 10;
    size_t size2 = 20;
    size_t size3 = 10; // Same size as file1
    size_t size4 = 30;

    // Create test files
    create_test_file(file1, size1);
    create_test_file(file2, size2);
    create_test_file(file3, size3);
    create_test_file(file4, size4);

    // Call find_matched_files
    SizeGroup *groups = find_matched_files("."); // Current directory

    // Verify that the returned SizeGroup list is correctly structured
    SizeGroup *current_group = groups;
    while (current_group != NULL) {
        // Verify that all files in the group have the same size
        for (size_t i = 1; i < current_group->count; i++) {
            struct stat st;
            stat(current_group->filepaths[i], &st);
            assert(st.st_size == current_group->size);
        }
        current_group = current_group->next;
    }

    //Verify that file1 and file3 are grouped together (same size)
    int file1_found = 0;
    int file3_found = 0;
    current_group = groups;
    while (current_group != NULL) {
    if (current_group->size == size1) {
        for (size_t i = 0; i < current_group->count; i++) {
            // Use strstr to account for "./" or absolute path prefixes
            if (strstr(current_group->filepaths[i], file1) != NULL) {
                file1_found = 1;
            }
            if (strstr(current_group->filepaths[i], file3) != NULL) {
                file3_found = 1;
            }
        }
    }
         current_group = current_group->next;
    }
    assert(file1_found == 1);
    assert(file3_found ==1);

    // Clean up test files
    delete_test_file(file1);
    delete_test_file(file2);
    delete_test_file(file3);
    delete_test_file(file4);

    // Free the SizeGroup list
    while (groups != NULL) {
        SizeGroup *next = groups->next;
        for (size_t i = 0; i < groups->count; i++) {
            free(groups->filepaths[i]); // Free the duplicated filepaths
        }
        free(groups->filepaths);
        free(groups);
        groups = next;
    }

    printf("All tests passed!\n");
    return 0;
}