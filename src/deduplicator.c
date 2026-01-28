// deduplicator.c
#include "deduplicator.h"
#include "hashing.h"
#include "datastruct.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// --- Helper: Size Grouping (Restored from Original) ---
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

// --- Helper: Byte Comparison (Restored Safety Check) ---
#define CMP_BUFFER_SIZE (1024 * 1024)

int compare_files(const char *file1, const char *file2) {
    FILE *f1 = fopen(file1, "rb"), *f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1); if (f2) fclose(f2);
        return 0;
    }

    char *buf1 = malloc(CMP_BUFFER_SIZE);
    char *buf2 = malloc(CMP_BUFFER_SIZE);
    
    if (!buf1 || !buf2) {
        if (buf1) free(buf1);
        if (buf2) free(buf2);
        fclose(f1); fclose(f2);
        return 0;
    }

    size_t n1, n2;
    int match = 1;

    while ((n1 = fread(buf1, 1, CMP_BUFFER_SIZE, f1)) > 0) {
        n2 = fread(buf2, 1, CMP_BUFFER_SIZE, f2);
        if (n1 != n2 || memcmp(buf1, buf2, n1) != 0) {
            match = 0;
            break;
        }
    }
    
    free(buf1);
    free(buf2);
    fclose(f1); fclose(f2);
    return match;
}

// --- The Engine ---
MarkedFile *find_duplicates(const char *path, int *total_out, ProgressCallback cb) {
    int num_files = 0;
    if (cb) cb("Scanning directory...", 5, 0);

    // 1. Scan
    FileEntry **files = scan_directory(path, &num_files);
    if (!files || num_files == 0) { *total_out = 0; return NULL; }

    // 2. Group by Size (The First Filter)
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

    // 3. Hash & Match (The "Deferred Add" Logic)
    HashTable *ht = create_hash_table();
    
    // Results container
    int capacity = 16;
    int count = 0;
    MarkedFile *results = malloc(capacity * sizeof(MarkedFile));
    
    int current_group_id = 1;
    int processed = 0;

    unsigned char *group_emitted = calloc(num_files + 1, sizeof(unsigned char));

    for (SizeGroup *g = groups; g; g = g->next) {
        if (g->count < 2) {
            processed += g->count;
            continue;
        }

        for (size_t i = 0; i < g->count; i++) {
            char *current_path = g->filepaths[i];
            
            HashContext *ctx = init_hash();
            if (hash_file(current_path, ctx) != 0) {
                free_hash_context(ctx);
                continue; // Skip inaccessible files
            }
            char *hex = finalize_hash(ctx);
            
            // Check for existing match
            HashEntry *entry = get_entry(ht, hex);
            
            if (entry) {
                // Potential Duplicate Found!
                // Verify with byte comparison to be safe
                if (compare_files(entry->file_paths->filepath, current_path)) {
                    
                    int gid = entry->group_id;

                    // Expand results if needed (we might add 2 items here)
                    if (count + 2 >= capacity) {
                        capacity *= 2;
                        results = realloc(results, capacity * sizeof(MarkedFile));
                    }

                    // CRITICAL LOGIC: Has the "Original" file been added to results yet?
                    if (group_emitted[gid] == 0) {
                        results[count].path = strdup(entry->file_paths->filepath);
                        results[count].group_id = gid;
                        results[count].is_duplicate = 0;
                        count++;
                        
                        group_emitted[gid] = 1;
                    }

                    results[count].path = strdup(current_path);
                    results[count].group_id = gid;
                    results[count].is_duplicate = 1;
                    count++;

                } else {
                    int new_gid = current_group_id++;
                    insert_hash(ht, hex, current_path, new_gid);
                }
            } else {
                insert_hash(ht, hex, current_path, current_group_id++);
            }

            free(hex);
            free_hash_context(ctx);
            processed++;
            if (cb) cb("Comparing...", (processed * 100) / num_files, num_files);
        }
    }

    // Cleanup
    free(group_emitted);
    free_hash_table(ht);
    while (groups) {
        SizeGroup *t = groups->next;
        for(size_t i = 0; i < groups->count; i++) free(groups->filepaths[i]);
        free(groups->filepaths); 
        free(groups);
        groups = t;
    }

    if (count == 0) {
        free(results);
        *total_out = 0;
        return NULL;
    }

    *total_out = count;
    return results;
}

void free_results(MarkedFile *results, int count) {
    if (!results) return;
    for (int i = 0; i < count; i++) {
        if (results[i].path) free(results[i].path);
    }
    free(results);
}

void execute_deletion(MarkedFile *results, int *marks, int total) {
    if (!results || !marks) return;
    for (int i = 0; i < total; i++) {
        if (marks[i] == 1) {
            remove(results[i].path);
        }
    }
}