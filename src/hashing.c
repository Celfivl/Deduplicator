// hashing.c
#include "hashing.h"
#include <stdlib.h>
#include <stdio.h>  // For file I/O
#include <errno.h>  // For error handling
#include <string.h> // For memcpy
#include <openssl/sha.h> // Include for SHA-256 functions

#define BUFFER_SIZE 4096 // Define a reasonable buffer size
#define SHA256_DIGEST_LENGTH 32 // SHA256 hash length

HashContext* init_hash() {
    HashContext* context = (HashContext*)malloc(sizeof(HashContext));
    if (context == NULL) {
        return NULL;
    }
    if (SHA256_Init(&context->sha256_context) != 1) {
        free(context);
        return NULL;
    }
    return context;
}

void update_hash(HashContext* context, const void* data, size_t size) {
    SHA256_Update(&context->sha256_context, data, size);
}

unsigned char* finalize_hash(HashContext* context) {
    unsigned char* hash = (unsigned char*)malloc(SHA256_DIGEST_LENGTH);
    if (hash == NULL) {
        return NULL;
    }
    SHA256_Final(hash, &context->sha256_context);
    return hash;
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