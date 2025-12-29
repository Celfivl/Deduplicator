// hashing.h
#ifndef HASHING_H
#define HASHING_H

#include <stddef.h>
#include <openssl/sha.h> // Include for SHA-256 functions

typedef struct HashContext HashContext; // Forward declaration

struct HashContext {
    SHA256_CTX sha256_context;
};

// Initializes the hashing context.
HashContext* init_hash();

// Updates the hash with the given data.
void update_hash(HashContext* context, const void* data, size_t size);

// Finalizes the hash and returns the hash value.
unsigned char* finalize_hash(HashContext* context);

// Hashes a file
int hash_file(const char* filepath, HashContext* context);

#endif