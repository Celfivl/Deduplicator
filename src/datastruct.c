// datastruct.c
#include "datastruct.h"

// djb2 hash function (slightly modified)
unsigned int hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash % HASH_TABLE_SIZE;
}

// Create a new hash table
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

// Insert a hash and filepath into the hash table
void insert_hash(HashTable *hash_table, const char *hash, const char *filepath) {
    unsigned int index = hash_string(hash);

    // Check if the hash already exists at this index
    HashEntry *current_entry = hash_table->table[index];
    while (current_entry != NULL) {
        if (strcmp(current_entry->hash, hash) == 0) {
            // Hash already exists, add the filepath to the list
            FilePathNode *new_file_path = (FilePathNode *)malloc(sizeof(FilePathNode));
            if (!new_file_path) {
                perror("Failed to allocate memory for file path node");
                return; // Or handle the error appropriately
            }
            new_file_path->filepath = strdup(filepath); // Duplicate the filepath
            new_file_path->next = current_entry->file_paths;
            current_entry->file_paths = new_file_path;
            return;
        }
        current_entry = current_entry->next;
    }

    // Hash doesn't exist, create a new entry
    HashEntry *new_entry = (HashEntry *)malloc(sizeof(HashEntry));
    if (!new_entry) {
        perror("Failed to allocate memory for hash entry");
        return; // Or handle the error appropriately
    }

    new_entry->hash = strdup(hash); // Duplicate the hash
    if (!new_entry->hash) {
        perror("Failed to duplicate hash string");
        free(new_entry);
        return;
    }

    // Create the first file path node
    FilePathNode *new_file_path = (FilePathNode *)malloc(sizeof(FilePathNode));
    if (!new_file_path) {
        perror("Failed to allocate memory for file path node");
        free(new_entry->hash);
        free(new_entry);
        return;
    }
    new_file_path->filepath = strdup(filepath); // Duplicate the filepath
    if (!new_file_path->filepath) {
        perror("Failed to duplicate filepath string");
        free(new_entry->hash);
        free(new_entry);
        free(new_file_path);
        return;
    }
    new_file_path->next = NULL;

    new_entry->file_paths = new_file_path;
    new_entry->next = hash_table->table[index]; // Add to the beginning of the list
    hash_table->table[index] = new_entry;
}

// Lookup a hash in the hash table
FilePathNode *lookup_hash(HashTable *hash_table, const char *hash) {
    unsigned int index = hash_string(hash);
    HashEntry *current_entry = hash_table->table[index];

    while (current_entry != NULL) {
        if (strcmp(current_entry->hash, hash) == 0) {
            // Hash found, return the list of file paths
            return current_entry->file_paths;
        }
        current_entry = current_entry->next;
    }

    // Hash not found
    return NULL;
}

// Free the memory used by the hash table
void free_hash_table(HashTable *hash_table) {
    if (!hash_table) return;

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashEntry *current_entry = hash_table->table[i];
        while (current_entry != NULL) {
            HashEntry *next_entry = current_entry->next;

            // Free the file path list
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