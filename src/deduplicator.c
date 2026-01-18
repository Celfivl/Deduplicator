// deduplicator.c
#include "deduplicator.h"
#include "hashing.h"
#include "datastruct.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Helper: Buckets files by size to avoid hashing unique files
static SizeGroup *add_group(SizeGroup **head, off_t size) {
    SizeGroup *g = malloc(sizeof(SizeGroup));
    if (!g) return NULL;

    g->size = size;
    g->count = 0;
    g->capacity = 4;
    g->filepaths = malloc(g->capacity * sizeof(char *));
    g->next = *head;
    *head = g;
    return g;
}

// Optimized 8KB buffer comparison: The "Final Court of Appeals" for hash collisions
int compare_files(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb"), *f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1); if (f2) fclose(f2);
        return 0;
    }

    char buf1[8192], buf2[8192];
    size_t n1, n2;
    int match = 1;

    while ((n1 = fread(buf1, 1, sizeof(buf1), f1)) > 0) {
        n2 = fread(buf2, 1, sizeof(buf2), f2);
        if (n1 != n2 || memcmp(buf1, buf2, n1) != 0) {
            match = 0;
            break;
        }
    }

    fclose(f1); fclose(f2);
    return match;
}

// The core engine: Orchestrates Scan -> Size Grouping -> Hashing -> Comparison
MarkedFile *find_duplicates(const char *path, int *total_out, ProgressCallback cb) {
    int num_files = 0;
    if (cb) cb("Scanning directory...", 5, 0);
    
    // Phase 1: Initial FS Scan
    FileEntry **files = scan_directory(path, &num_files);
    if (!files) return NULL;

    // Phase 2: Group by Size to isolate candidate duplicates
    SizeGroup *groups = NULL;
    for (int i = 0; i < num_files; i++) {
        SizeGroup *g = groups;
        while (g && g->size != files[i]->size) g = g->next;
        if (!g) g = add_group(&groups, files[i]->size);
        
        if (g->count == g->capacity) {
            g->capacity *= 2;
            g->filepaths = realloc(g->filepaths, g->capacity * sizeof(char *));
        }
        g->filepaths[g->count++] = strdup(files[i]->path);
        free_file_entry(files[i]);
    }
    free(files);

    // Phase 3: Hash and Compare
    MarkedFile *results = malloc(num_files * sizeof(MarkedFile));
    HashTable *ht = create_hash_table();
    int processed = 0, current_id = 1;

    for (SizeGroup *g = groups; g; g = g->next) {
        for (size_t i = 0; i < g->count; i++) {
            results[processed].path = strdup(g->filepaths[i]);
            
            if (g->count < 2) {
                results[processed].group_id = 0;
                results[processed].is_duplicate = 0;
            } else {
                HashContext *ctx = init_hash();
                hash_file(g->filepaths[i], ctx);
                char *hex = finalize_hash(ctx);
                
                HashEntry *entry = get_entry(ht, hex);

                if (entry && compare_files(entry->file_paths->filepath, g->filepaths[i])) {
                    results[processed].group_id = entry->group_id;
                    results[processed].is_duplicate = 1;
                } else {
                    results[processed].group_id = current_id++;
                    results[processed].is_duplicate = 0;
                    insert_hash(ht, hex, g->filepaths[i], results[processed].group_id);
                }
                free(hex); 
                free_hash_context(ctx);
            }
            processed++;
            if (cb) cb("Comparing contents...", 20 + (processed * 80 / num_files), num_files);
        }
    }

    // Phase 4: Teardown metadata
    *total_out = num_files;
    free_hash_table(ht);
    while (groups) {
        SizeGroup *t = groups->next;
        for(size_t i = 0; i < groups->count; i++) free(groups->filepaths[i]);
        free(groups->filepaths); 
        free(groups);
        groups = t;
    }
    return results;
}

// Cleanup: Releases the results array used by the TUI
void free_results(MarkedFile *results, int count) {
    if (!results) return;
    for (int i = 0; i < count; i++) {
        if (results[i].path) free(results[i].path);
    }
    free(results);
}

// Deletion Engine: Permanently removes files marked in the TUI
void execute_deletion(MarkedFile *results, int *marks, int total) {
    if (!results || !marks) return;
    for (int i = 0; i < total; i++) {
        if (marks[i] == 1) {
            remove(results[i].path);
        }
    }
}