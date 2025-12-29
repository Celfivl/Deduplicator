// hashing.c
#include "hashing.h"
#include <stdlib.h>
#include <stdio.h>   // For file I/O
#include <errno.h>   // For error handling
#include <string.h>  // For memcpy
#include <openssl/evp.h> // Include for EVP functions

#define BUFFER_SIZE 4096 // Define a reasonable buffer size
#define SHA256_DIGEST_LENGTH 32 // SHA256 hash length

struct HashContext {
    EVP_MD_CTX* md_context;
    const EVP_MD* md;
};

HashContext* init_hash() {
    HashContext* context = (HashContext*)malloc(sizeof(HashContext));
    if (context == NULL) {
        return NULL;
    }

    context->md = EVP_MD_fetch(NULL, "SHA256", NULL);
    if (context->md == NULL) {
        free(context);
        return NULL;
    }

    context->md_context = EVP_MD_CTX_create();
    if (context->md_context == NULL) {
        EVP_MD_free((EVP_MD*)context->md);
        free(context);
        return NULL;
    }

    if (EVP_DigestInit_ex2(context->md_context, context->md, NULL) != 1) {
        EVP_MD_CTX_destroy(context->md_context);
        EVP_MD_free((EVP_MD*)context->md);
        free(context);
        return NULL;
    }

    return context;
}

void update_hash(HashContext* context, const void* data, size_t size) {
    EVP_DigestUpdate(context->md_context, data, size);
}

unsigned char* finalize_hash(HashContext* context) {
    unsigned char* hash = (unsigned char*)malloc(SHA256_DIGEST_LENGTH);
    if (hash == NULL) return NULL;

    unsigned int hash_len;
    // Finalize the digest
    if (EVP_DigestFinal_ex(context->md_context, hash, &hash_len) != 1) {
        free(hash);
        return NULL;
    }
    return hash;
}

void free_hash_context(HashContext* context) {
    if (context) {
        if (context->md_context) EVP_MD_CTX_destroy(context->md_context);
        if (context->md) EVP_MD_free((EVP_MD*)context->md);
        free(context);
    }
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