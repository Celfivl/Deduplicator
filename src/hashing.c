// hashing.c
#include "hashing.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <openssl/evp.h>

#define BUFFER_SIZE 8192 // Define a reasonable buffer size

HashContext* init_hash() {
    HashContext* context = malloc(sizeof(HashContext));
    if (!context) return NULL;

    context->md = EVP_MD_fetch(NULL, "SHA256", NULL);
    context->md_context = EVP_MD_CTX_create();

    // Check if OpenSSL allocation or fetching failed
    if (!context->md || !context->md_context || 
        EVP_DigestInit_ex2(context->md_context, context->md, NULL) != 1) {
        free_hash_context(context);
        return NULL;
    }

    return context;
}

void update_hash(HashContext* context, const void* data, size_t size) {
    if (context && context->md_context) {
        EVP_DigestUpdate(context->md_context, data, size);
    }
}

char* finalize_hash(HashContext* context) {
    unsigned char bin_hash[HASH_DIGEST_LENGTH];
    unsigned int hash_len;

    if (EVP_DigestFinal_ex(context->md_context, bin_hash, &hash_len) != 1) {
        return NULL;
    }

    char* hex_str = malloc(HASH_HEX_SIZE);
    if (!hex_str) return NULL;

    // Efficient hex conversion
    for (int i = 0; i < HASH_DIGEST_LENGTH; i++) {
        static const char hex_digits[] = "0123456789abcdef";
        hex_str[i * 2] = hex_digits[(bin_hash[i] >> 4) & 0x0F];
        hex_str[i * 2 + 1] = hex_digits[bin_hash[i] & 0x0F];
    }
    hex_str[64] = '\0';

    return hex_str;
}

void free_hash_context(HashContext* context) {
    if (context) {
        if (context->md_context) EVP_MD_CTX_destroy(context->md_context);
        if (context->md) EVP_MD_free((EVP_MD*)context->md);
        free(context);
    }
}

int reset_hash(HashContext* context) {
    if (!context || !context->md_context || !context->md) return -1;
    // Standard Init allows context reuse without re-fetching the algorithm
    return (EVP_DigestInit_ex(context->md_context, context->md, NULL) == 1) ? 0 : -1;
}

int hash_file(const char* filepath, HashContext* context) {
    if (reset_hash(context) != 0) return -1;

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        perror("fopen");
        return -1;
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        update_hash(context, buffer, bytesRead);
    }

    int result = ferror(file) ? -1 : 0;
    if (result == -1) perror("fread");

    fclose(file);
    return result;
}