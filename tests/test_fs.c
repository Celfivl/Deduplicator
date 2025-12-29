// tests/test_fs.c
// Test file for the file system scanning module.

#include <assert.h>
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>   // For creating directories
#include <sys/stat.h> // For stat

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
    
    create_dummy_directory_structure();

    FileList *file_list = scan_directory("test_dir");
    assert(file_list != NULL);
    
    // CHANGE: You created 3 files (file1, file2, file3)
    // Directories are skipped by your logic, so size should be 3.
    assert(file_list->size == 3);

    // Cleanup
    free_file_list(file_list); // This now safely frees the list AND the 3 entries
    remove_dummy_directory_structure();
}

void test_metadata_retrieval() {
    // Test case for metadata retrieval (file size and type)

    // Test file metadata
    const char *test_file_path = "test_metadata.txt";
    const char *test_file_content = "This is a test file.";
    FILE *fp = fopen(test_file_path, "w");
    assert(fp != NULL);
    fprintf(fp, test_file_content);
    fclose(fp);

    FileEntry *file_entry = create_file_entry(test_file_path);
    assert(file_entry != NULL);
    assert(file_entry->size == strlen(test_file_content));
    assert(file_entry->is_directory == 0); // Ensure it's a file
    free_file_entry(file_entry);
    remove(test_file_path);

    // Test directory metadata
    const char *test_dir_path = "test_metadata_dir";
    mkdir(test_dir_path);

    FileEntry *dir_entry = create_file_entry(test_dir_path);
    assert(dir_entry != NULL);
    assert(dir_entry->is_directory == 1); // Ensure it's a directory
    free_file_entry(dir_entry);
    rmdir(test_dir_path);
}

void test_file_list_management() {
    FileList *list = create_file_list();
    assert(list != NULL);

    const int num_entries = 1000;
    FileEntry *entries[num_entries];
    char path[32];

    for (int i = 0; i < num_entries; i++) {
        // We bypass create_file_entry because the files don't exist on disk.
        // We manually allocate to test the LIST logic specifically.
        entries[i] = (FileEntry*)malloc(sizeof(FileEntry));
        assert(entries[i] != NULL);
        
        sprintf(path, "dummy_file_%d", i);
        entries[i]->path = strdup(path);
        entries[i]->size = (off_t)i;
        entries[i]->is_directory = 0;

        assert(add_file_entry(list, entries[i]) == 0);
    }

    assert(list->size == num_entries);
    
    // This will trigger reallocations from 10 -> 20 -> 40 -> ... -> 1280
    assert(list->capacity >= num_entries);

    // free_file_list will now iterate and free all 1000 entries 
    // and their strdup'd paths.
    free_file_list(list);
    
    printf("Dynamic List Stress Test (1000 entries) Passed!\n");
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
    test_metadata_retrieval();
    test_file_list_management();

    remove("test_file.txt"); // Clean up the dummy file

    return 0;
}