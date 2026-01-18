// test_fs.c
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

// Cross-platform directory creation helper
void make_dir(const char* path) {
#ifdef _WIN32
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "if not exist \"%s\" mkdir \"%s\"", path, path);
    system(cmd);
#else
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "mkdir -p %s", path);
    system(cmd);
#endif
}

void create_dummy_file(const char* path, const char* content) {
    FILE* fp = fopen(path, "w");
    if (!fp) {
        printf("Error: Could not open %s for writing. Ensure directory exists.\n", path);
        assert(fp != NULL);
    }
    fprintf(fp, "%s", content);
    fclose(fp);
}

void test_basic_scan() {
    // Setup environment
    make_dir("test_dir");
    make_dir("test_dir/sub_dir");
    
    create_dummy_file("test_dir/file1.txt", "content1");
    create_dummy_file("test_dir/sub_dir/file2.txt", "content22");

    int num_files = 0;
    FileEntry** files = scan_directory("test_dir", &num_files);

    // Diagnostic output
    if (num_files == 0) {
        printf("Error: Scanner found 0 files. Check path recursion logic.\n");
    }

    assert(files != NULL);
    assert(num_files == 2); 

    bool found_file2 = false;
    for (int i = 0; i < num_files; i++) {
        if (strstr(files[i]->path, "file2.txt")) {
            found_file2 = true;
            assert(files[i]->size == 9);
        }
    }
    assert(found_file2);

    // Memory Cleanup
    for (int i = 0; i < num_files; i++) free_file_entry(files[i]);
    free(files);

    printf("Filesystem Scanner Test: Passed\n");
}

int main() {
    test_basic_scan();
    return 0;
}