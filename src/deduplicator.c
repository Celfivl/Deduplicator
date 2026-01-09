// deduplicator.c
#include "deduplicator.h"
#include "hashing.h"
#include "datastruct.h"
#include <string.h>

// Allocates a new metadata bucket for files of a specific size 
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

// Performs a bit-for-bit check to ensure hash matches aren't collisions 
int compare_files(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb"), *f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1); if (f2) fclose(f2);
        return 0;
    }

    int c1, c2;
    do {
        c1 = fgetc(f1); c2 = fgetc(f2);
        if (c1 != c2) { 
            fclose(f1); fclose(f2); 
            return 0; 
        }
    } while (c1 != EOF);

    fclose(f1); fclose(f2);
    return 1;
}

MarkedFile *find_duplicates(const char *path, int *total_out, ProgressCallback cb) {
    int num_files = 0;
    if (cb) cb("Scanning directory...", 5, 0);
    FileEntry **files = scan_directory(path, &num_files);
    if (!files) return NULL;

    // Sort files into buckets by size to narrow search space 
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

    MarkedFile *results = malloc(num_files * sizeof(MarkedFile));
    HashTable *ht = create_hash_table();
    int processed = 0, current_id = 1;

    // Analyze groups: files with unique sizes are skipped automatically 
    for (SizeGroup *g = groups; g; g = g->next) {
        for (size_t i = 0; i < g->count; i++) {
            results[processed].path = g->filepaths[i];
            
            if (g->count < 2) {
                results[processed].group_id = 0;
                results[processed].is_duplicate = 0;
            } else {
                HashContext *ctx = init_hash();
                hash_file(g->filepaths[i], ctx);
                char *hex = finalize_hash(ctx);
                FilePathNode *node = lookup_hash(ht, hex);

                // Assign Group ID if content is identical; otherwise, start new group 
                if (node && compare_files(node->filepath, g->filepaths[i])) {
                    results[processed].group_id = lookup_group_id(ht, hex);
                    results[processed].is_duplicate = 1;
                } else {
                    results[processed].group_id = current_id++;
                    results[processed].is_duplicate = 0;
                    insert_hash_with_id(ht, hex, g->filepaths[i], results[processed].group_id);
                }
                free(hex); free_hash_context(ctx);
            }
            processed++;
            if (cb) cb("Verifying content...", 20 + (processed * 80 / num_files), num_files);
        }
    }

    *total_out = num_files;
    free_hash_table(ht);
    while (groups) {
        SizeGroup *t = groups->next;
        free(groups->filepaths); free(groups);
        groups = t;
    }
    return results;
}

void free_results(MarkedFile *results, int count) {
    for (int i = 0; i < count; i++) free(results[i].path);
    free(results);
}