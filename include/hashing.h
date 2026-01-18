// hashing.h
#ifndef HASHING_H
#define HASHING_H

#include <stddef.h>
#include <openssl/evp.h>

// SHA-256 digest size in bytes and resulting hex string length
#define HASH_DIGEST_LENGTH 32
#define HASH_HEX_SIZE 65

// Manages the OpenSSL Message Digest context
typedef struct HashContext {
    EVP_MD_CTX* md_context;
    const EVP_MD* md;
} HashContext;

// Allocation and Lifecycle
HashContext* init_hash(void);
void free_hash_context(HashContext* context);
int reset_hash(HashContext* context);

// Core Hashing Operations
void update_hash(HashContext* context, const void* data, size_t size);
char* finalize_hash(HashContext* context); // Returns a malloc'd hex string

// High-level File Wrapper
int hash_file(const char* filepath, HashContext* context);

#endif