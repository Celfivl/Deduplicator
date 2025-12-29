// test_datastruct.c
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "datastruct.h"
#include "filesystem.h"

void test_file_list_management() {
    FileList *list = create_file_list();
    assert(list != NULL);

    const int num_entries = 1000;
    char path_buf[32];

    for (int i = 0; i < num_entries; i++) {
        // Manually allocate to bypass the disk for this unit test
        FileEntry *entry = (FileEntry*)malloc(sizeof(FileEntry));
        assert(entry != NULL);
        
        sprintf(path_buf, "dummy_file_%d", i);
        entry->path = strdup(path_buf);
        entry->size = (off_t)i;
        entry->is_directory = 0;

        assert(add_file_entry(list, entry) == 0);
    }

    assert(list->size == num_entries);
    assert(list->capacity >= num_entries);

    free_file_list(list); 
    printf("Dynamic List Stress Test (1000 entries) Passed!\n");
}

int main() {
    test_file_list_management();
    return 0;
}