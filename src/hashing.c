// hashing.c
#include "hashing.h"
#include <stdlib.h>
#include <stdio.h>   // For file I/O
#include <errno.h>   // For error handling
#include <string.h>  // For memcpy
#include <openssl/evp.h> // Include for EVP functions

#define BUFFER_SIZE 4096 // Define a reasonable buffer size

HashContext* init_hash() {
    HashContext* context = (HashContext*)malloc(sizeof(HashContext));
    if (context == NULL) return NULL;

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

char* finalize_hash(HashContext* context) {
    unsigned char bin_hash[HASH_DIGEST_LENGTH];
    unsigned int hash_len;

    // Finalize the digest
    if (EVP_DigestFinal_ex(context->md_context, bin_hash, &hash_len) != 1) {
        return NULL;
    }

    // Convert binary hash to hex string (2 chars per byte + null terminator)
    char* hex_str = malloc(HASH_DIGEST_LENGTH * 2 + 1);
    if (hex_str == NULL) return NULL;

    for (int i = 0; i < HASH_DIGEST_LENGTH; i++) {
        sprintf(&hex_str[i * 2], "%02x", bin_hash[i]);
    }
    hex_str[HASH_DIGEST_LENGTH * 2] = '\0';

    return hex_str;
}

void free_hash_context(HashContext* context) {
    if (context) {
        if (context->md_context) EVP_MD_CTX_destroy(context->md_context);
        if (context->md) EVP_MD_free((EVP_MD*)context->md);
        free(context);
    }
}

// Resets the hash context for re-use
int reset_hash(HashContext* context) {
    if (!context || !context->md_context || !context->md) return -1;
    return (EVP_DigestInit_ex(context->md_context, context->md, NULL) == 1) ? 0 : -1;
}

// New function to hash a file
int hash_file(const char* filepath, HashContext* context) {
    if (reset_hash(context) != 0) return -1;

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