// test_hash.c
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashing.h"

// Standard NIST SHA-256 Test Vectors
#define TEST_VECTOR1_INPUT ""
#define TEST_VECTOR1_SHA256 "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
#define TEST_VECTOR2_INPUT "abc"
#define TEST_VECTOR2_SHA256 "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
#define TEST_VECTOR3_INPUT "message digest"
#define TEST_VECTOR3_SHA256 "f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650"

#define TEST_FILE "hash_test_temp.txt"

void test_string_hashing() {
    HashContext* ctx = init_hash();
    assert(ctx != NULL);

    // Test Vector 2: "abc"
    update_hash(ctx, TEST_VECTOR2_INPUT, strlen(TEST_VECTOR2_INPUT));
    char* result = finalize_hash(ctx);
    
    assert(result != NULL);
    assert(strcmp(result, TEST_VECTOR2_SHA256) == 0);
    
    free(result);
    free_hash_context(ctx);
    printf("String Hashing Test: Passed\n");
}

void test_file_hashing() {
    // Create a temporary file with known content
    FILE* fp = fopen(TEST_FILE, "wb");
    assert(fp != NULL);
    fprintf(fp, "%s", TEST_VECTOR3_INPUT);
    fclose(fp);

    HashContext* ctx = init_hash();
    assert(hash_file(TEST_FILE, ctx) == 0);
    
    char* result = finalize_hash(ctx);
    assert(result != NULL);
    assert(strcmp(result, TEST_VECTOR3_SHA256) == 0);

    free(result);
    free_hash_context(ctx);
    remove(TEST_FILE);
    printf("File Hashing Test: Passed\n");
}

void test_context_reset() {
    HashContext* ctx = init_hash();
    
    // Hash first string
    update_hash(ctx, "abc", 3);
    char* res1 = finalize_hash(ctx);
    free(res1);

    // Reset and hash second string to ensure no carry-over
    assert(reset_hash(ctx) == 0);
    update_hash(ctx, TEST_VECTOR3_INPUT, strlen(TEST_VECTOR3_INPUT));
    char* res2 = finalize_hash(ctx);
    
    assert(strcmp(res2, TEST_VECTOR3_SHA256) == 0);

    free(res2);
    free_hash_context(ctx);
    printf("Context Reset Test: Passed\n");
}

int main() {
    test_string_hashing();
    test_file_hashing();
    test_context_reset();
    
    printf("\nAll Hashing Engine Tests Passed Successfully!\n");
    return 0;
}