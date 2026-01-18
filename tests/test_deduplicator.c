// test_deduplicator.c
#include "deduplicator.h"
#include "filesystem.h"
#include "hashing.h"
#include "datastruct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Create file with specific content
int create_file(const char *path, const char *content) {
    FILE *fp = fopen(path, "w");
    if (!fp) return -1; 
    fprintf(fp, "%s", content);
    fclose(fp);
    return 0;
}

void test_identical_files() {
    const char *f1 = "identical1.txt", *f2 = "identical2.txt";
    const char *content = "This is the content of the file.";

    create_file(f1, content);
    create_file(f2, content);

    // Should return 1 (Match)
    assert(compare_files(f1, f2) == 1);

    remove(f1); remove(f2);
    printf("PASSED: test_identical_files\n");
}

void test_near_identical_files() {
    const char *f1 = "near1.txt", *f2 = "near2.txt";
    const char *c1 = "This is the content of the file.";
    const char *c2 = "This is the content of the file!"; 

    create_file(f1, c1);
    create_file(f2, c2);

    // Should return 0 (Mismatch)
    assert(compare_files(f1, f2) == 0);

    remove(f1); remove(f2);
    printf("PASSED: test_near_identical_files\n");
}

void test_grouping_logic() {
    // Cross-platform directory setup
#ifdef _WIN32
    system("if not exist test_logic_dir mkdir test_logic_dir");
#else
    system("mkdir -p test_logic_dir");
#endif

    const char *f1 = "test_logic_dir/f1.txt";
    const char *f2 = "test_logic_dir/f2.txt";
    const char *f3 = "test_logic_dir/f3.txt";

    create_file(f1, "Duplicate Content");
    create_file(f2, "Unique Content");
    create_file(f3, "Duplicate Content"); 

    int total = 0;
    MarkedFile *results = find_duplicates("test_logic_dir", &total, NULL);

    int f1_group = -1, f3_group = -2;
    for (int i = 0; i < total; i++) {
        if (strstr(results[i].path, "f1.txt")) {
            f1_group = results[i].group_id;
            // First file encountered with this hash is the 'Anchor' (is_duplicate = 0)
            assert(results[i].is_duplicate == 0); 
        }
        if (strstr(results[i].path, "f3.txt")) {
            f3_group = results[i].group_id;
            // Subsequent file with same hash is a 'Duplicate' (is_duplicate = 1)
            assert(results[i].is_duplicate == 1); 
        }
        if (strstr(results[i].path, "f2.txt")) {
            // Unique content shouldn't be grouped
            assert(results[i].group_id == 0); 
        }
    }

    assert(f1_group == f3_group); 
    assert(f1_group > 0);

    free_results(results, total);
    
#ifdef _WIN32
    system("rd /s /q test_logic_dir");
#else
    system("rm -rf test_logic_dir");
#endif
    printf("PASSED: test_grouping_logic\n");
}

int main() {
    test_identical_files();
    test_near_identical_files();
    test_grouping_logic();

    printf("\nAll Deduplicator Engine Tests Passed!\n");
    return 0;
}