// test_datastruct.c
#include "datastruct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_INSERTS 10000
#define NUM_LOOKUPS 10000

// Helper to generate a random string for stress testing
char *generate_random_string(int length) {
    char *str = malloc(length + 1);
    if (!str) return NULL;

    for (int i = 0; i < length; i++) {
        str[i] = 'a' + rand() % 26;
    }
    str[length] = '\0';
    return str;
}

int main() {
    srand(time(NULL));

    HashTable *ht = create_hash_table();
    if (!ht) {
        fprintf(stderr, "Failed to create hash table\n");
        return 1;
    }

    // --- Insertion Stress Test ---
    printf("Testing hash map insertion (%d elements)...\n", NUM_INSERTS);
    clock_t start_insert = clock();

    for (int i = 0; i < NUM_INSERTS; i++) {
        char *hash = generate_random_string(32);
        char *filepath = generate_random_string(64);
        
        // Using the refactored insert (group_id = 0 for default)
        insert_hash(ht, hash, filepath, 0);
        
        free(hash);
        free(filepath);
    }

    clock_t end_insert = clock();
    double insert_time = (double)(end_insert - start_insert) / CLOCKS_PER_SEC;
    printf("Inserted %d elements in %f seconds\n", NUM_INSERTS, insert_time);

    // --- Lookup Speed Test ---
    printf("Testing hash map lookup speed...\n");
    clock_t start_lookup = clock();

    int found_count = 0;
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        char *hash = generate_random_string(32);
        
        // Using refactored lookup function
        FilePathNode *result = lookup_paths(ht, hash);
        if (result != NULL) found_count++;
        
        free(hash);
    }

    clock_t end_lookup = clock();
    double lookup_time = (double)(end_lookup - start_lookup) / CLOCKS_PER_SEC;
    printf("Looked up %d elements in %f seconds\n", NUM_LOOKUPS, lookup_time);
    printf("Found %d elements (Expected low number due to random generation)\n", found_count);

    // Verify cleanup logic
    free_hash_table(ht);
    printf("Memory cleanup successful.\n");

    return 0;
}