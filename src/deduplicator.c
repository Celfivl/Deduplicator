// deduplicator.c
#include "deduplicator.h"
#include "filesystem.h"
#include "hashing.h"
#include "datastruct.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_GROUP_CAPACITY 4

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

                char *hash_res = (char *)finalize_hash(ctx);
                if (!hash_res) {
                    free_hash_context(ctx);
                    continue;
                }

                // Lookup the hash in the hash table
                FilePathNode *existing_file_paths = lookup_hash(hash_table, hash_res);

                if (existing_file_paths != NULL) {
                    // Hash collision! Compare the files byte by byte with all existing files with the same hash
                    FilePathNode *current_existing_file = existing_file_paths;
                    while(current_existing_file != NULL){
                         if (compare_files(filepath, current_existing_file->filepath)) {
                            // Files are identical, create a DuplicatePair
                            DuplicatePair *new_pair = (DuplicatePair *)malloc(sizeof(DuplicatePair));
                            if (!new_pair) {
                                perror("malloc DuplicatePair");
                                exit(EXIT_FAILURE);
                            }
                            new_pair->file1 = strdup(filepath);
                            new_pair->file2 = strdup(current_existing_file->filepath);
                            new_pair->next = duplicate_list;
                            duplicate_list = new_pair;
                            break;
                        }
                        current_existing_file = current_existing_file->next;
                    }
                } else {
                    // Hash not found, insert it into the hash table
                    insert_hash(hash_table, hash_res, filepath);
                }

                // Cleanup per-file hashing resources
                free(hash_res); 
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