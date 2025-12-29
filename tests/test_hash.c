#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h> // Include for EVP_MAX_MD_SIZE
#include "hashing.h"

#define TEST_FILE_NAME "test_hash_file.txt"
#define SHA256_DIGEST_LENGTH 32 // SHA256 hash length

// Test vectors
#define TEST_VECTOR1_INPUT ""
#define TEST_VECTOR1_SHA256 "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
#define TEST_VECTOR2_INPUT "abc"
#define TEST_VECTOR2_SHA256 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
#define TEST_VECTOR3_INPUT "message digest"
#define TEST_VECTOR3_SHA256 "f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650"

void hex_to_bytes(const char* hex_string, unsigned char* byte_array) {
    // Only loop for the 32 bytes of a SHA-256 hash
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sscanf(hex_string + 2 * i, "%2hhx", &byte_array[i]);
    }
}

void test_hash_binary_data() {
    const char* bin_filename = "test_binary.dat";
    // Using a 3-byte string to avoid any alignment/padding confusion
    unsigned char bin_data[3] = { 'a', 'b', 'c' };
    size_t bin_size = 3; 
    
    FILE* fp = fopen(bin_filename, "wb");
    assert(fp != NULL);
    fwrite(bin_data, 1, bin_size, fp);
    fclose(fp);

    HashContext* context = init_hash();
    assert(hash_file(bin_filename, context) == 0);
    unsigned char* hash = finalize_hash(context);

    // This is the known hash for "abc" (TEST_VECTOR2)
    const char* expected_hex = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    unsigned char expected_bytes[32];
    hex_to_bytes(expected_hex, expected_bytes);

    printf("Generated: ");
    for(int i = 0; i < 32; i++) printf("%02x", hash[i]);
    printf("\nExpected:  %s\n", expected_hex);

    assert(memcmp(hash, expected_bytes, 32) == 0);

    free_hash_context(context);
    free(hash);
    remove(bin_filename);
}

void test_hash_string(const char* input, const char* expected_sha256) {
    HashContext* context = init_hash();
    assert(context != NULL);

    update_hash(context, input, strlen(input));
    unsigned char* hash = finalize_hash(context);
    assert(hash != NULL);

    unsigned char expected_bytes[SHA256_DIGEST_LENGTH];
    hex_to_bytes(expected_sha256, expected_bytes);

    assert(memcmp(hash, expected_bytes, SHA256_DIGEST_LENGTH) == 0);

    free_hash_context(context);
    free(hash);
}

void test_hash_file_with_known_vector(const char* input, const char* expected_sha256) {
    FILE* fp = fopen(TEST_FILE_NAME, "w");
    assert(fp != NULL);
    fprintf(fp, "%s", input);
    fclose(fp);

    HashContext* context = init_hash();
    assert(hash_file(TEST_FILE_NAME, context) == 0);

    unsigned char* hash = finalize_hash(context);
    unsigned char expected_bytes[SHA256_DIGEST_LENGTH];
    hex_to_bytes(expected_sha256, expected_bytes);

    assert(memcmp(hash, expected_bytes, SHA256_DIGEST_LENGTH) == 0);

    free_hash_context(context);
    free(hash);
    remove(TEST_FILE_NAME);
}

int main() {
    test_hash_binary_data();
    test_hash_string(TEST_VECTOR1_INPUT, TEST_VECTOR1_SHA256);
    test_hash_string(TEST_VECTOR2_INPUT, TEST_VECTOR2_SHA256);
    test_hash_string(TEST_VECTOR3_INPUT, TEST_VECTOR3_SHA256);
    test_hash_file_with_known_vector(TEST_VECTOR2_INPUT, TEST_VECTOR2_SHA256);

    printf("All SHA-256 Test Vectors Passed Successfully!\n");
    return 0;
}