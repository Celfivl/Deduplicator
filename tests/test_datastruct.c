// test_datastruct.c
#include "datastruct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_INSERTS 10000
#define NUM_LOOKUPS 10000

// Function to generate a random string of given length
char *generate_random_string(int length) {
    char *str = (char *)malloc(length + 1);
    if (!str) {
        perror("Failed to allocate memory for random string");
        return NULL;
    }

    for (int i = 0; i < length; i++) {
        str[i] = 'a' + rand() % 26; // Generate random lowercase letters
    }
    str[length] = '\0';

    return str;
}

int main() {
    // Initialize random seed
    srand(time(NULL));

    // Create a hash table
    HashTable *hash_table = create_hash_table();
    if (!hash_table) {
        fprintf(stderr, "Failed to create hash table\n");
        return 1;
    }

    // --- Test hash map insertion ---
    printf("Testing hash map insertion...\n");
    clock_t start_insert = clock();

    for (int i = 0; i < NUM_INSERTS; i++) {
        char *hash = generate_random_string(32); // Generate a random hash
        char *filepath = generate_random_string(64); // Generate a random filepath
        insert_hash(hash_table, hash, filepath);
        free(hash);
        free(filepath);
    }

    clock_t end_insert = clock();
    double insert_time = (double)(end_insert - start_insert) / CLOCKS_PER_SEC;
    printf("Inserted %d elements in %f seconds\n", NUM_INSERTS, insert_time);

    // --- Test hash map lookup speed ---
    printf("Testing hash map lookup speed...\n");
    clock_t start_lookup = clock();

    int found_count = 0;
    for (int i = 0; i < NUM_LOOKUPS; i++) {
        char *hash = generate_random_string(32); // Generate a random hash
        FilePathNode *result = lookup_hash(hash_table, hash);
        if (result != NULL) {
            found_count++;
        }
        free(hash);
    }

    clock_t end_lookup = clock();
    double lookup_time = (double)(end_lookup - start_lookup) / CLOCKS_PER_SEC;
    printf("Looked up %d elements in %f seconds\n", NUM_LOOKUPS, lookup_time);
    printf("Found %d elements\n", found_count);

    // Free the hash table
    free_hash_table(hash_table);

    return 0;
}