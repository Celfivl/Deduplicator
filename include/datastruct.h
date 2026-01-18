// datastruct.h
#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <stddef.h>

// Size chosen for a balance between memory and collision avoidance
#define HASH_TABLE_SIZE 4096 

// Linked list of file paths that share the same hash
typedef struct FilePathNode {
    char *filepath;
    struct FilePathNode *next;
} FilePathNode;

// Entry in the hash table
typedef struct HashEntry {
    char *hash;                     // Key: SHA-256 hex string
    int group_id;                   // Used by TUI to group duplicates
    FilePathNode *file_paths;       // Head of path list
    struct HashEntry *next;         // For hash table collisions
} HashEntry;

// The main Index structure
typedef struct HashTable {
    HashEntry *buckets[HASH_TABLE_SIZE];
} HashTable;

// Lifecycle Management
HashTable *create_hash_table(void);
void free_hash_table(HashTable *ht);

// Core Operations
unsigned int calculate_bucket(const char *hash);
void insert_hash(HashTable *ht, const char *hash, const char *filepath, int group_id);
HashEntry *get_entry(HashTable *ht, const char *hash);

// Utility
FilePathNode *lookup_paths(HashTable *ht, const char *hash);

#endif