// datastruct.c
#include "datastruct.h"

unsigned int hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; 
    }
    return hash % HASH_TABLE_SIZE;
}

HashTable *create_hash_table() {
    HashTable *hash_table = (HashTable *)malloc(sizeof(HashTable));
    if (!hash_table) {
        perror("Failed to allocate memory for hash table");
        return NULL;
    }
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table->table[i] = NULL;
    }
    return hash_table;
}

void insert_hash(HashTable *hash_table, const char *hash, const char *filepath) {
    unsigned int index = hash_string(hash);
    HashEntry *current_entry = hash_table->table[index];

    while (current_entry != NULL) {
        if (strcmp(current_entry->hash, hash) == 0) {
            FilePathNode *new_file_path = (FilePathNode *)malloc(sizeof(FilePathNode));
            if (!new_file_path) return;
            new_file_path->filepath = strdup(filepath);
            new_file_path->next = current_entry->file_paths;
            current_entry->file_paths = new_file_path;
            return;
        }
        current_entry = current_entry->next;
    }

    HashEntry *new_entry = (HashEntry *)malloc(sizeof(HashEntry));
    if (!new_entry) return;

    new_entry->hash = strdup(hash);
    new_entry->group_id = 0; // Default ID
    FilePathNode *new_file_path = (FilePathNode *)malloc(sizeof(FilePathNode));
    if (!new_file_path) {
        free(new_entry->hash); free(new_entry); return;
    }
    new_file_path->filepath = strdup(filepath);
    new_file_path->next = NULL;
    new_entry->file_paths = new_file_path;
    new_entry->next = hash_table->table[index];
    hash_table->table[index] = new_entry;
}

void insert_hash_with_id(HashTable *hash_table, const char *hash, const char *filepath, int group_id) {
    insert_hash(hash_table, hash, filepath);
    unsigned int index = hash_string(hash);
    HashEntry *current_entry = hash_table->table[index];
    while (current_entry) {
        if (strcmp(current_entry->hash, hash) == 0) {
            current_entry->group_id = group_id;
            return;
        }
        current_entry = current_entry->next;
    }
}

FilePathNode *lookup_hash(HashTable *hash_table, const char *hash) {
    unsigned int index = hash_string(hash);
    HashEntry *current_entry = hash_table->table[index];
    while (current_entry != NULL) {
        if (strcmp(current_entry->hash, hash) == 0) return current_entry->file_paths;
        current_entry = current_entry->next;
    }
    return NULL;
}

int lookup_group_id(HashTable *hash_table, const char *hash) {
    unsigned int index = hash_string(hash);
    HashEntry *current_entry = hash_table->table[index];
    while (current_entry != NULL) {
        if (strcmp(current_entry->hash, hash) == 0) return current_entry->group_id;
        current_entry = current_entry->next;
    }
    return 0;
}

void free_hash_table(HashTable *hash_table) {
    if (!hash_table) return;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry *current_entry = hash_table->table[i];
        while (current_entry != NULL) {
            HashEntry *next_entry = current_entry->next;
            FilePathNode *current_file_path = current_entry->file_paths;
            while (current_file_path != NULL) {
                FilePathNode *next_file_path = current_file_path->next;
                free(current_file_path->filepath);
                free(current_file_path);
                current_file_path = next_file_path;
            }
            free(current_entry->hash);
            free(current_entry);
            current_entry = next_entry;
        }
    }
    free(hash_table);
}