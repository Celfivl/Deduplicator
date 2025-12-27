// tests/test_fs.c
// Test file for the file system scanning module.

#include <assert.h>
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

    remove("test_file.txt"); // Clean up the dummy file

    return 0;
}