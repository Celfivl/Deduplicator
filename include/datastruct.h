// datastruct.h
#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 1024 

typedef struct FilePathNode {
    char *filepath;
    struct FilePathNode *next;
} FilePathNode;

typedef struct HashEntry {
    char *hash;                     // Key: file hash
    int group_id;                   // Added for TUI grouping
    FilePathNode *file_paths;       // Value: list of file paths with this hash
    struct HashEntry *next;         // For collision handling
} HashEntry;

typedef struct HashTable {
    HashEntry *table[HASH_TABLE_SIZE];
} HashTable;

HashTable *create_hash_table();
unsigned int hash_string(const char *str);
void insert_hash(HashTable *hash_table, const char *hash, const char *filepath);
void insert_hash_with_id(HashTable *hash_table, const char *hash, const char *filepath, int group_id);
FilePathNode *lookup_hash(HashTable *hash_table, const char *hash);
int lookup_group_id(HashTable *hash_table, const char *hash);
void free_hash_table(HashTable *hash_table);

#endif