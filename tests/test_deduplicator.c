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

// Helper function to create a file with specific content
void create_file(const char *filename, const char *content) {
    FILE *fp = fopen(filename, "w");
    assert(fp != NULL);
    fputs(content, fp);
    fclose(fp);
}

// Test case for identical files
void test_identical_files() {
    const char *filename1 = "identical1.txt";
    const char *filename2 = "identical2.txt";
    const char *content = "This is the content of the file.";

    create_file(filename1, content);
    create_file(filename2, content);

    assert(compare_files(filename1, filename2) == 1);

    remove(filename1);
    remove(filename2);

    printf("PASSED: test_identical_files\n");
}

// Test case for near-identical files
void test_near_identical_files() {
    const char *filename1 = "near1.txt";
    const char *filename2 = "near2.txt";
    const char *content1 = "This is the content of the file.";
    const char *content2 = "This is the content of the file!"; // One byte difference

    create_file(filename1, content1);
    create_file(filename2, content2);

    assert(compare_files(filename1, filename2) == 0);

    remove(filename1);
    remove(filename2);

    printf("PASSED: test_near_identical_files\n");
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

    // Call find_duplicates instead of find_matched_files
    DuplicatePair *dupes = find_duplicates(".");

    // Verify that file1 and file3 are in the duplicate list
    int file1_found = 0;
    int file3_found = 0;
    DuplicatePair *curr_pair = dupes;
    while (curr_pair != NULL) {
        if (strstr(curr_pair->file1, file1) || strstr(curr_pair->file2, file1)) file1_found = 1;
        if (strstr(curr_pair->file1, file3) || strstr(curr_pair->file2, file3)) file3_found = 1;
        curr_pair = curr_pair->next;
    }
    assert(file1_found == 1);
    assert(file3_found == 1);

    // Clean up test files
    delete_test_file(file1);
    delete_test_file(file2);
    delete_test_file(file3);
    delete_test_file(file4);

     // Free the DuplicatePair list
    while (dupes != NULL) {
        DuplicatePair *next = dupes->next;
        free(dupes->file1);
        free(dupes->file2);
        free(dupes);
        dupes = next;
    }


    // Run the new tests
    test_identical_files();
    test_near_identical_files();

    printf("All tests passed!\n");
    return 0;
}