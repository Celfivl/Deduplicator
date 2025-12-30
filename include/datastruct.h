// datastruct.h
#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 1024 // You can adjust the size as needed

// Structure to hold a file path
typedef struct FilePathNode {
    char *filepath;
    struct FilePathNode *next;
} FilePathNode;

// Structure for a hash map entry
typedef struct HashEntry {
    char *hash;                     // Key: file hash
    FilePathNode *file_paths;       // Value: list of file paths with this hash
    struct HashEntry *next;         // For collision handling (separate chaining)
} HashEntry;

// Hash map structure
typedef struct HashTable {
    HashEntry *table[HASH_TABLE_SIZE];
} HashTable;

// Function prototypes
HashTable *create_hash_table();
unsigned int hash_string(const char *str);
void insert_hash(HashTable *hash_table, const char *hash, const char *filepath);
FilePathNode *lookup_hash(HashTable *hash_table, const char *hash);
void free_hash_table(HashTable *hash_table);

#endif