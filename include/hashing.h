// hashing.h
#ifndef HASHING_H
#define HASHING_H

#include <stddef.h>

typedef struct HashContext HashContext; // Forward declaration

// Initializes the hashing context.
HashContext* init_hash();

// Updates the hash with the given data.
void update_hash(HashContext* context, const void* data, size_t size);

// Finalizes the hash and returns the hash value.
unsigned char* finalize_hash(HashContext* context);

#endif