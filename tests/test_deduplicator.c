// test_deduplicator.c
#include "deduplicator.h"
#include "filesystem.h"
#include "hashing.h"
#include "datastruct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>

// Create test file with specific size
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

// Delete test file
void delete_test_file(const char *filename) {
    if (remove(filename) != 0) {
        perror("Failed to delete test file");
    }
}

// Create file with specific content
int create_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (!fp) return -1; 
    fprintf(fp, "%s", content);
    fclose(fp);
    return 0;
}

// Test case for identical files
void test_identical_files() {
    const char *f1 = "identical1.txt";
    const char *f2 = "identical2.txt";
    const char *content = "This is the content of the file.";

    create_file(f1, content);
    create_file(f2, content);

    assert(compare_files(f1, f2) == 1);

    remove(f1);
    remove(f2);
    printf("PASSED: test_identical_files\n");
}

// Test case for near-identical files
void test_near_identical_files() {
    const char *f1 = "near1.txt";
    const char *f2 = "near2.txt";
    const char *c1 = "This is the content of the file.";
    const char *c2 = "This is the content of the file!"; 

    create_file(f1, c1);
    create_file(f2, c2);

    assert(compare_files(f1, f2) == 0);

    remove(f1);
    remove(f2);
    printf("PASSED: test_near_identical_files\n");
}

// Verify marking logic
void test_mark_duplicates() {
    const char *test_dir = "test_mark_dir";
    mkdir(test_dir);

    char f1[256], f2[256], f3[256], f4[256];
    snprintf(f1, 256, "%s/file1.txt", test_dir);
    snprintf(f2, 256, "%s/file2.txt", test_dir);
    snprintf(f3, 256, "%s/file3.txt", test_dir);
    snprintf(f4, 256, "%s/file4.txt", test_dir);

    create_file(f1, "Alpha");
    create_file(f2, "Beta");
    create_file(f3, "Alpha"); // Match f1
    create_file(f4, "Gamma");

    DuplicatePair *dups = find_duplicates(test_dir);
    
    int num_files = 0;
    FileEntry **entries = scan_directory(test_dir, &num_files);
    
    if (entries) {
        char **paths = malloc(num_files * sizeof(char *));
        MarkedFile *marks = malloc(num_files * sizeof(MarkedFile));
        
        for(int i=0; i < num_files; i++) paths[i] = entries[i]->path;

        mark_duplicates(paths, num_files, dups, marks);

        int found = 0;
        for (int i = 0; i < num_files; i++) {
            if (strstr(marks[i].path, "file1.txt") || strstr(marks[i].path, "file3.txt")) {
                assert(marks[i].is_duplicate == 1);
                if(marks[i].is_duplicate) found++;
            } else {
                assert(marks[i].is_duplicate == 0);
            }
        }
        assert(found == 2);

        free(paths);
        free(marks);
        for (int i = 0; i < num_files; i++) free_file_entry(entries[i]);
        free(entries);
    }

    free_duplicate_pairs(dups);
    system("rd /s /q test_mark_dir");
    printf("PASSED: test_mark_duplicates\n");
}

int main() {
    const char *f1 = "test_f1.txt";
    const char *f2 = "test_f2.txt";
    const char *f3 = "test_f3.txt";
    const char *f4 = "test_f4.txt";

    create_test_file(f1, 10);
    create_test_file(f2, 20);
    create_test_file(f3, 10); // Match f1 size
    create_test_file(f4, 30);

    DuplicatePair *dupes = find_duplicates(".");

    int f1_found = 0, f3_found = 0;
    DuplicatePair *curr = dupes;
    while (curr != NULL) {
        if (strstr(curr->file1, f1) || strstr(curr->file2, f1)) f1_found = 1;
        if (strstr(curr->file1, f3) || strstr(curr->file2, f3)) f3_found = 1;
        curr = curr->next;
    }
    assert(f1_found == 1);
    assert(f3_found == 1);

    delete_test_file(f1);
    delete_test_file(f2);
    delete_test_file(f3);
    delete_test_file(f4);

    free_duplicate_pairs(dupes);

    test_identical_files();
    test_near_identical_files();
    test_mark_duplicates();

    printf("All tests passed!\n");
    return 0;
}