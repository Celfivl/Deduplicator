// hashing.c
#include "hashing.h"
#include <stdlib.h>
#include <stdio.h> // For file I/O
#include <errno.h> // For error handling

#define BUFFER_SIZE 4096 // Define a reasonable buffer size

// Placeholder for the actual hashing context
struct HashContext {
    // Add necessary fields for your hashing algorithm
};

HashContext* init_hash() {
    HashContext* context = (HashContext*)malloc(sizeof(HashContext));
    // Initialize the context here
    return context;
}

void update_hash(HashContext* context, const void* data, size_t size) {
    // Update the hash with the given data
}

unsigned char* finalize_hash(HashContext* context) {
    // Finalize the hash and return the hash value
    return NULL; // Placeholder
}

// New function to hash a file
int hash_file(const char* filepath, HashContext* context) {
    FILE* file = fopen(filepath, "rb"); // Open in binary mode
    if (file == NULL) {
        perror("fopen");
        return -1; // Indicate failure
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        update_hash(context, buffer, bytesRead);
    }

    if (ferror(file)) {
        perror("fread");
        fclose(file);
        return -1; // Indicate failure
    }

    fclose(file);
    return 0; // Indicate success
}