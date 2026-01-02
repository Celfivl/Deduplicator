// deduplicator.c
#include "deduplicator.h"
#include "filesystem.h"
#include "hashing.h"
#include "datastruct.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_GROUP_CAPACITY 4

// 32-byte digest -> 65-byte hex string (64 chars + '\0')
static char *digest_to_hex(const unsigned char *digest) {
    char *hex = malloc(SHA256_DIGEST_LENGTH * 2 + 1);
    if (!hex) return NULL;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hex + (i * 2), "%02x", digest[i]);
    }
    hex[SHA256_DIGEST_LENGTH * 2] = '\0';
    return hex;
}

static SizeGroup *find_group(SizeGroup *head, off_t size) {
    for (SizeGroup *cur = head; cur != NULL; cur = cur->next) {
        if (cur->size == size) return cur;
    }
    return NULL;
}

static SizeGroup *add_group(SizeGroup **head, off_t size) {
    SizeGroup *g = (SizeGroup *)malloc(sizeof(SizeGroup));
    if (!g) {
        perror("malloc SizeGroup");
        exit(EXIT_FAILURE);
    }
    g->size = size;
    g->filepaths = (char **)malloc(INITIAL_GROUP_CAPACITY * sizeof(char *));
    if (!g->filepaths) {
        perror("malloc filepaths");
        exit(EXIT_FAILURE);
    }
    g->count = 0;
    g->capacity = INITIAL_GROUP_CAPACITY;
    g->next = *head;
    *head = g;
    return g;
}

SizeGroup *group_files_by_size(FileEntry **files, int num_files) {
    SizeGroup *head = NULL;
    if (!files || num_files <= 0) return NULL;

    for (int i = 0; i < num_files; i++) {
        FileEntry *f = files[i];
        if (!f) continue;

        SizeGroup *g = find_group(head, f->size);
        if (!g) {
            g = add_group(&head, f->size);
        }

        if (g->count == g->capacity) {
            g->capacity *= 2;
            char **new_paths = (char **)realloc(g->filepaths,
                                                g->capacity * sizeof(char *));
            if (!new_paths) {
                perror("realloc filepaths");
                exit(EXIT_FAILURE);
            }
            g->filepaths = new_paths;
        }

        g->filepaths[g->count] = strdup(f->path);
        if (!g->filepaths[g->count]) {
            perror("strdup path");
            exit(EXIT_FAILURE);
        }
        g->count++;
    }

    return head;
}

// Function to compare two files byte by byte
int compare_files(const char *file1, const char *file2) {
    FILE *fp1 = fopen(file1, "rb");
    if (!fp1) {
        perror("fopen file1");
        return 0;
    }

    FILE *fp2 = fopen(file2, "rb");
    if (!fp2) {
        perror("fopen file2");
        fclose(fp1);
        return 0;
    }

    int ch1, ch2;
    while ((ch1 = fgetc(fp1)) != EOF && (ch2 = fgetc(fp2)) != EOF) {
        if (ch1 != ch2) {
            fclose(fp1);
            fclose(fp2);
            return 0; // Files are different
        }
    }

    if (fgetc(fp1) != EOF || fgetc(fp2) != EOF) {
        // One file is longer than the other
        fclose(fp1);
        fclose(fp2);
        return 0;
    }

    fclose(fp1);
    fclose(fp2);
    return 1; // Files are identical
}


DuplicatePair *find_duplicates(const char *directory_path) {
    int num_files = 0;
    FileEntry **files = scan_directory(directory_path, &num_files);
    if (!files) return NULL;

    SizeGroup *groups = group_files_by_size(files, num_files);

    for (int i = 0; i < num_files; i++) {
        free_file_entry(files[i]);
    }
    free(files);

    // Create a hash table
    HashTable *hash_table = create_hash_table();
    if (hash_table == NULL) {
        fprintf(stderr, "Error: Could not create hash table.\n");
        // Free the SizeGroup list
        while (groups != NULL) {
            SizeGroup *next = groups->next;
            for (size_t i = 0; i < groups->count; i++) {
                free(groups->filepaths[i]); // Free the duplicated filepaths
            }
            free(groups->filepaths);
            free(groups);
            groups = next;
        }
        return NULL;
    }

    // Now hash the files and compare byte by byte if needed
    DuplicatePair *duplicate_list = NULL;
    SizeGroup *current_group = groups;
    while (current_group != NULL) {
        if (current_group->count > 1) {
            // Hash all files in the group and add them to the hash table
            for (size_t i = 0; i < current_group->count; i++) {
                char *filepath = current_group->filepaths[i];
                
                // CORRECTED SECTION: Use frozen hashing.h context lifecycle
                HashContext *ctx = init_hash();
                if (!ctx) continue;

                if (hash_file(filepath, ctx) != 0) { // Assuming 0 is success for hash_file
                    fprintf(stderr, "Error: Could not hash file %s\n", filepath);
                    free_hash_context(ctx);
                    continue; 
                }

                unsigned char *digest = finalize_hash(ctx);
                if (!digest) {
                    free_hash_context(ctx);
                    continue;
            }

                char *hash_hex = digest_to_hex(digest);
                free(digest);
                if (!hash_hex) {
                    free_hash_context(ctx);
                    continue;
                }
                // Lookup the hash in the hash table
                FilePathNode *existing_file_paths = lookup_hash(hash_table, hash_hex);

                if (existing_file_paths != NULL) {
                // Add to duplicate list
                DuplicatePair *new_pair = malloc(sizeof(DuplicatePair));
                if (new_pair) {
                    new_pair->file1 = strdup(existing_file_paths->filepath);
                    new_pair->file2 = strdup(filepath);
                    new_pair->next = duplicate_list;
                    duplicate_list = new_pair;
                }   
                // use `hash_hex` only as key; don't free until after lookup/insert
                } else {
                    insert_hash(hash_table, hash_hex, filepath);
                }

                // After using `hash_hex`
                free(hash_hex);
                free_hash_context(ctx);
                }
            }
        current_group = current_group->next;
    }

    // Free the SizeGroup list structures, but NOT the filepaths.
    while (groups != NULL) {
        SizeGroup *next = groups->next;
        for (size_t i = 0; i < groups->count; i++) {
            free(groups->filepaths[i]); // This frees the strdup from group_files_by_size
        }
        free(groups->filepaths);
        free(groups);
        groups = next;
    }

    free_hash_table(hash_table);

    // This list contains the final strdup copies. 
    return duplicate_list;
}

void mark_duplicates(char **all_paths, int num_paths,
                     DuplicatePair *pairs_head, MarkedFile *out_marks) {
    if (!all_paths || !out_marks) return;

    HashTable *dup_lookup = create_hash_table();
    if (!dup_lookup) {
        fprintf(stderr, "Error: could not create dup lookup table\n");
        return;
    }

    // Build lookup from DuplicatePair list
    for (DuplicatePair *p = pairs_head; p != NULL; p = p->next) {
        if (p->file1) insert_hash(dup_lookup, p->file1, "dup");
        if (p->file2) insert_hash(dup_lookup, p->file2, "dup");
    }

    // Mark each path
    for (int i = 0; i < num_paths; i++) {
    out_marks[i].path = all_paths[i];

    if (!all_paths[i]) {
        out_marks[i].is_duplicate = 0;
        continue;
    }

    if (lookup_hash(dup_lookup, all_paths[i]) != NULL) {
        out_marks[i].is_duplicate = 1;
    } else {
        out_marks[i].is_duplicate = 0;
    }
}

    free_hash_table(dup_lookup);
}

void free_duplicate_pairs(DuplicatePair *head) {
    DuplicatePair *current = head;
    while (current != NULL) {
        DuplicatePair *next = current->next;
        
        // Free the strings we strdup'd in find_duplicates
        if (current->file1) free(current->file1);
        if (current->file2) free(current->file2);
        
        // Free the node itself
        free(current);
        
        current = next;
    }
}