// tests/test_fs.c
// Test file for the file system scanning module.

#include <assert.h>
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h> // For creating directories
#include <sys/stat.h> // For chmod

void test_create_file_entry() {
    // Test case for create_file_entry
    FileEntry *entry = create_file_entry("test_file.txt");
    assert(entry != NULL);
    assert(strcmp(entry->path, "test_file.txt") == 0);
    free_file_entry(entry);
}

void test_create_file_entry_null_path() {
    // Test case for create_file_entry with NULL path
    FileEntry *entry = create_file_entry(NULL);
    assert(entry == NULL);
}

void test_create_file_entry_nonexistent_file() {
    // Test case for create_file_entry with a non-existent file
    FileEntry *entry = create_file_entry("nonexistent_file.txt");
    // Depending on how you want to handle this, it could return NULL or a valid entry with size 0
    if (errno == ENOENT) {
        //errno set to ENOENT
        assert(entry == NULL);
    }
}

void test_free_file_entry() {
    // Test case for free_file_entry
    FileEntry *entry = create_file_entry("test_file.txt");
    free_file_entry(entry);
    // Add a more robust test to ensure memory is actually freed (optional, OS-dependent)
}

void test_free_file_entry_null_entry() {
    // Test case for free_file_entry with a NULL entry
    free_file_entry(NULL); // Should not crash
}

void create_dummy_directory_structure() {
    // Create a temporary directory structure for testing
    mkdir("test_dir");
    mkdir("test_dir/subdir1");
    mkdir("test_dir/subdir2");

    FILE *fp1 = fopen("test_dir/file1.txt", "w");
    assert(fp1 != NULL);
    fprintf(fp1, "test content 1");
    fclose(fp1);

    FILE *fp2 = fopen("test_dir/subdir1/file2.txt", "w");
    assert(fp2 != NULL);
    fprintf(fp2, "test content 2");
    fclose(fp2);

    FILE *fp3 = fopen("test_dir/subdir2/file3.txt", "w");
    assert(fp3 != NULL);
    fprintf(fp3, "test content 3");
    fclose(fp3);
}

void remove_dummy_directory_structure() {
    // Remove the temporary directory structure
    remove("test_dir/file1.txt");
    remove("test_dir/subdir1/file2.txt");
    remove("test_dir/subdir2/file3.txt");
    rmdir("test_dir/subdir1");
    rmdir("test_dir/subdir2");
    rmdir("test_dir");
}

void test_scan_directory_recursive() {
    // Test case for scan_directory with recursive traversal
    int num_files;
    FileEntry **files;

    create_dummy_directory_structure();

    files = scan_directory("test_dir", &num_files);
    assert(files != NULL);
    assert(num_files == 3); // Expect 3 files

    // Basic check for file names (order may vary)
    int file1_found = 0;
    int file2_found = 0;
    int file3_found = 0;

    for (int i = 0; i < num_files; i++) {
        if (strcmp(files[i]->path, "test_dir/file1.txt") == 0) {
            file1_found = 1;
        } else if (strcmp(files[i]->path, "test_dir/subdir1/file2.txt") == 0) {
            file2_found = 1;
        } else if (strcmp(files[i]->path, "test_dir/subdir2/file3.txt") == 0) {
            file3_found = 1;
        }
        free_file_entry(files[i]); // Free each entry
    }
    assert(file1_found == 1);
    assert(file2_found == 1);
    assert(file3_found == 1);

    free(files); // Free the array of file entries
    remove_dummy_directory_structure();
}


int main() {
    // Create a dummy file for testing
    FILE *fp = fopen("test_file.txt", "w");
    if (fp == NULL) {
        fprintf(stderr, "Could not create test file.\n");
        return 1;
    }
    fprintf(fp, "test content");
    fclose(fp);

    test_create_file_entry();
    test_create_file_entry_null_path();
    test_create_file_entry_nonexistent_file();
    test_free_file_entry();
    test_free_file_entry_null_entry();
    test_scan_directory_recursive();

    remove("test_file.txt"); // Clean up the dummy file

    return 0;
}