// datastruct.c
#include "datastruct.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// djb2 hash algorithm
unsigned int calculate_bucket(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    return hash % HASH_TABLE_SIZE;
}

HashTable *create_hash_table() {
    HashTable *ht = malloc(sizeof(HashTable));
    if (!ht) return NULL;

    // calloc ensures all bucket pointers start as NULL
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
    return ht;
}

void insert_hash(HashTable *ht, const char *hash, const char *filepath, int group_id) {
    unsigned int index = calculate_bucket(hash);
    HashEntry *entry = ht->buckets[index];

    // Check if hash already exists in the table
    while (entry) {
        if (strcmp(entry->hash, hash) == 0) {
            FilePathNode *new_node = malloc(sizeof(FilePathNode));
            if (!new_node) return;
            
            new_node->filepath = strdup(filepath);
            new_node->next = entry->file_paths;
            entry->file_paths = new_node;
            
            // Update group_id if a valid one is provided
            if (group_id != 0) entry->group_id = group_id;
            return;
        }
        entry = entry->next;
    }

    // Create new entry if hash not found
    HashEntry *new_entry = malloc(sizeof(HashEntry));
    if (!new_entry) return;

    new_entry->hash = strdup(hash);
    new_entry->group_id = group_id;
    
    FilePathNode *new_node = malloc(sizeof(FilePathNode));
    if (!new_node) {
        free(new_entry->hash);
        free(new_entry);
        return;
    }
    
    new_node->filepath = strdup(filepath);
    new_node->next = NULL;
    
    new_entry->file_paths = new_node;
    new_entry->next = ht->buckets[index];
    ht->buckets[index] = new_entry;
}

HashEntry *get_entry(HashTable *ht, const char *hash) {
    unsigned int index = calculate_bucket(hash);
    HashEntry *entry = ht->buckets[index];
    
    while (entry) {
        if (strcmp(entry->hash, hash) == 0) return entry;
        entry = entry->next;
    }
    return NULL;
}

FilePathNode *lookup_paths(HashTable *ht, const char *hash) {
    HashEntry *entry = get_entry(ht, hash);
    return entry ? entry->file_paths : NULL;
}

void free_hash_table(HashTable *ht) {
    if (!ht) return;

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry *entry = ht->buckets[i];
        while (entry) {
            HashEntry *next_entry = entry->next;
            
            // Free the path list
            FilePathNode *path_node = entry->file_paths;
            while (path_node) {
                FilePathNode *next_path = path_node->next;
                free(path_node->filepath);
                free(path_node);
                path_node = next_path;
            }
            
            free(entry->hash);
            free(entry);
            entry = next_entry;
        }
    }
    free(ht);
}