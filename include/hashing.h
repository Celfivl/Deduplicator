// hashing.h
#ifndef HASHING_H
#define HASHING_H

#include <stddef.h>
#include <openssl/evp.h> // Include for EVP functions

typedef struct HashContext HashContext; // Forward declaration

struct HashContext {
    EVP_MD_CTX* md_context;
    const EVP_MD* md;
};

// Initializes the hashing context.
HashContext* init_hash();

// Updates the hash with the given data.
void update_hash(HashContext* context, const void* data, size_t size);

// Finalizes the hash and returns the hash value.
unsigned char* finalize_hash(HashContext* context);

//Frees the hash context
void free_hash_context(HashContext* context);

// Hashes a file
int hash_file(const char* filepath, HashContext* context);

#endif