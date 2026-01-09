// test_deduplicator.c
#include "deduplicator.h"
#include "filesystem.h"
#include "hashing.h"
#include "datastruct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>

// Create test file with specific size
void create_test_file(const char *filename, size_t size) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) exit(EXIT_FAILURE);
    fseek(fp, size - 1, SEEK_SET);
    fputc('\0', fp);
    fclose(fp);
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
    const char *f1 = "identical1.txt", *f2 = "identical2.txt";
    const char *content = "This is the content of the file.";

    create_file(f1, content);
    create_file(f2, content);

    assert(compare_files(f1, f2) == 1);

    remove(f1); remove(f2);
    printf("PASSED: test_identical_files\n");
}

// Test case for near-identical files
void test_near_identical_files() {
    const char *f1 = "near1.txt", *f2 = "near2.txt";
    const char *c1 = "This is the content of the file.";
    const char *c2 = "This is the content of the file!"; 

    create_file(f1, c1);
    create_file(f2, c2);

    assert(compare_files(f1, f2) == 0);

    remove(f1); remove(f2);
    printf("PASSED: test_near_identical_files\n");
}

// Verify grouping and marking logic
void test_grouping_logic() {
    mkdir("test_logic_dir");
    const char *f1 = "test_logic_dir/f1.txt", *f2 = "test_logic_dir/f2.txt", *f3 = "test_logic_dir/f3.txt";

    create_file(f1, "Duplicate Content");
    create_file(f2, "Unique Content");
    create_file(f3, "Duplicate Content"); // Match f1

    int total = 0;
    MarkedFile *results = find_duplicates("test_logic_dir", &total, NULL);

    int f1_group = -1, f3_group = -2;
    for (int i = 0; i < total; i++) {
        if (strstr(results[i].path, "f1.txt")) {
            f1_group = results[i].group_id;
            assert(results[i].is_duplicate == 0); // First encounter
        }
        if (strstr(results[i].path, "f3.txt")) {
            f3_group = results[i].group_id;
            assert(results[i].is_duplicate == 1); // Second encounter
        }
        if (strstr(results[i].path, "f2.txt")) {
            assert(results[i].group_id == 0); // Unique file
        }
    }

    assert(f1_group == f3_group); // Must be in same group
    assert(f1_group > 0);

    free_results(results, total);
    system("rd /s /q test_logic_dir");
    printf("PASSED: test_grouping_logic\n");
}

int main() {
    const char *f1 = "t1.txt", *f2 = "t2.txt", *f3 = "t3.txt";

    create_test_file(f1, 10);
    create_test_file(f2, 20);
    create_test_file(f3, 10); // Match f1 size

    int total = 0;
    MarkedFile *res = find_duplicates(".", &total, NULL);

    int found_pair = 0;
    for (int i = 0; i < total; i++) {
        for (int j = i + 1; j < total; j++) {
            if (res[i].group_id > 0 && res[i].group_id == res[j].group_id) found_pair = 1;
        }
    }
    assert(found_pair == 1);

    remove(f1); remove(f2); remove(f3);
    free_results(res, total);

    test_identical_files();
    test_near_identical_files();
    test_grouping_logic();

    printf("All tests passed!\n");
    return 0;
}