/**
 * @file        test.c
 *
 * @brief       Common String Library Test Suite
 *
 * Tests all PiString operations:
 * - Hash computation (FNV-1a)
 * - Initialization (static and owned)
 * - Copy and duplicate
 * - Free
 * - Equality comparison
 * - Lexicographic comparison
 */

#include "pi/common/string.h"
#include "pi/common/memory.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* =---- Hash Function Tests -----------------------------------= */

static void test_hash_empty_string(void)
{
    printf("  Test: Hash empty string\n");

    uint32_t hash = piHashString("", 0);
    assert(hash == PI_FNV_BASIS);
    printf("    Hash of \"\": %u (FNV basis) ✓\n", hash);
}

static void test_hash_simple_string(void)
{
    printf("  Test: Hash simple string\n");

    const char *str = "hello";
    uint32_t hash = piHashString(str, 5);
    assert(hash != PI_FNV_BASIS);
    printf("    Hash of \"hello\": %u ✓\n", hash);
}

static void test_hash_consistency(void)
{
    printf("  Test: Hash consistency\n");

    const char *str = "test";
    uint32_t hash1 = piHashString(str, 4);
    uint32_t hash2 = piHashString(str, 4);
    assert(hash1 == hash2);
    printf("    Hash is consistent: %u == %u ✓\n", hash1, hash2);
}

static void test_hash_different_strings(void)
{
    printf("  Test: Hash different strings\n");

    uint32_t hash1 = piHashString("abc", 3);
    uint32_t hash2 = piHashString("abd", 3);
    assert(hash1 != hash2);
    printf("    \"abc\" (%u) != \"abd\" (%u) ✓\n", hash1, hash2);
}

/* =---- Initialization Tests ----------------------------------= */

static void test_init_string(void)
{
    printf("  Test: Initialize string from static C string\n");

    PiString str;
    piInitString(&str, "hello");

    assert(str.chars != NULL);
    assert(str.count == 5);
    assert(str.clear == 0);
    assert(str.hash == piHashString("hello", 5));
    printf("    chars=\"%.*s\", count=%u, clear=%u ✓\n",
           str.count, str.chars, str.count, str.clear);
}

static void test_init_string_with_length(void)
{
    printf("  Test: Initialize string with explicit length\n");

    PiString str;
    piInitStringWithLength(&str, "hello world", 5);

    assert(str.chars != NULL);
    assert(str.count == 5);
    assert(str.clear == 0);
    printf("    chars=\"%.*s\", count=%u ✓\n", str.count, str.chars, str.count);
}

static void test_init_owned_string(void)
{
    printf("  Test: Initialize owned string\n");

    char *cstr = piMalloc(6);
    strcpy(cstr, "owned");

    PiString str;
    piInitOwnedString(&str, cstr);

    assert(str.chars == cstr);
    assert(str.count == 5);
    assert(str.clear == 1);
    printf("    chars=\"%.*s\", count=%u, clear=%u ✓\n",
           str.count, str.chars, str.count, str.clear);

    piFreeString(&str);
}

static void test_init_owned_string_with_length(void)
{
    printf("  Test: Initialize owned string with length\n");

    char *cstr = piMalloc(12);
    strcpy(cstr, "hello world");

    PiString str;
    piInitOwnedStringWithLenght(&str, cstr, 5);

    assert(str.chars == cstr);
    assert(str.count == 5);
    assert(str.clear == 1);
    printf("    chars=\"%.*s\", count=%u, clear=%u ✓\n",
           str.count, str.chars, str.count, str.clear);

    piFreeString(&str);
}

/* =---- Copy and Duplicate Tests ------------------------------= */

static void test_string_copy(void)
{
    printf("  Test: Shallow copy string\n");

    PiString src;
    piInitString(&src, "source");

    PiString *copy = piStringCopy(&src);

    assert(copy != NULL);
    assert(copy->chars == src.chars);
    assert(copy->count == src.count);
    assert(copy->hash == src.hash);
    printf("    Copy shares same chars pointer ✓\n");

    piFree(copy);
}

static void test_string_copy_to(void)
{
    printf("  Test: Shallow copy to existing destination\n");

    PiString src;
    piInitString(&src, "source");

    PiString dest;
    piStringCopyTo(&dest, &src);

    assert(dest.chars == src.chars);
    assert(dest.count == src.count);
    assert(dest.hash == src.hash);
    printf("    Destination has same data as source ✓\n");
}

static void test_string_dup(void)
{
    printf("  Test: Deep duplicate string\n");

    PiString src;
    piInitString(&src, "source");

    PiString *dup = piStringDup(&src);

    assert(dup != NULL);
    assert(dup->chars != src.chars);
    assert(dup->count == src.count);
    assert(dup->hash == src.hash);
    assert(dup->clear == 1);
    assert(memcmp(dup->chars, src.chars, src.count) == 0);
    printf("    Duplicate has own copy of chars (clear=1) ✓\n");

    piFreeString(dup);
    piFree(dup);
}

static void test_string_dup_to(void)
{
    printf("  Test: Deep duplicate to existing destination\n");

    PiString src;
    piInitString(&src, "source");

    PiString dest;
    piStringDupTo(&dest, &src);

    assert(dest.chars != src.chars);
    assert(dest.count == src.count);
    assert(dest.hash == src.hash);
    assert(dest.clear == 1);
    printf("    Destination has own copy of chars ✓\n");

    piFreeString(&dest);
}

/* =---- Free Tests --------------------------------------------= */

static void test_free_string_null(void)
{
    printf("  Test: Free NULL string (should not crash)\n");

    PiString *result = piFreeString(NULL);
    assert(result == NULL);
    printf("    piFreeString(NULL) returned NULL ✓\n");
}

static void test_free_static_string(void)
{
    printf("  Test: Free static string (clear=0)\n");

    PiString str;
    piInitString(&str, "static");

    piFreeString(&str);

    assert(str.chars == NULL);
    assert(str.count == 0);
    assert(str.clear == 0);
    assert(str.hash == 0);
    printf("    Static string freed (chars set to NULL) ✓\n");
}

static void test_free_owned_string(void)
{
    printf("  Test: Free owned string (clear=1)\n");

    char *cstr = piMalloc(6);
    strcpy(cstr, "owned");

    PiString str;
    piInitOwnedString(&str, cstr);

    piFreeString(&str);

    assert(str.chars == NULL);
    assert(str.count == 0);
    assert(str.clear == 0);
    assert(str.hash == 0);
    printf("    Owned string freed (memory deallocated) ✓\n");
}

/* =---- Equality Tests ----------------------------------------= */

static void test_string_equals_same(void)
{
    printf("  Test: String equals (same content)\n");

    PiString a, b;
    piInitString(&a, "hello");
    piInitString(&b, "hello");

    assert(piStringEquals(&a, &b) == true);
    printf("    \"hello\" == \"hello\" ✓\n");
}

static void test_string_equals_different(void)
{
    printf("  Test: String equals (different content)\n");

    PiString a, b;
    piInitString(&a, "hello");
    piInitString(&b, "world");

    assert(piStringEquals(&a, &b) == false);
    printf("    \"hello\" != \"world\" ✓\n");
}

static void test_string_equals_different_length(void)
{
    printf("  Test: String equals (different length)\n");

    PiString a, b;
    piInitString(&a, "hello");
    piInitString(&b, "hello world");

    assert(piStringEquals(&a, &b) == false);
    printf("    \"hello\" != \"hello world\" ✓\n");
}

static void test_string_equals_ignore_case_same(void)
{
    printf("  Test: String equals ignore case (same)\n");

    PiString a, b;
    piInitString(&a, "Hello");
    piInitString(&b, "hELLO");

    assert(piStringEqualsIgnoreCase(&a, &b) == true);
    printf("    \"Hello\" ==i \"hELLO\" ✓\n");
}

static void test_string_equals_ignore_case_different(void)
{
    printf("  Test: String equals ignore case (different)\n");

    PiString a, b;
    piInitString(&a, "Hello");
    piInitString(&b, "World");

    assert(piStringEqualsIgnoreCase(&a, &b) == false);
    printf("    \"Hello\" !=i \"World\" ✓\n");
}

/* =---- Comparison Tests --------------------------------------= */

static void test_string_compare_equal(void)
{
    printf("  Test: String compare (equal)\n");

    PiString a, b;
    piInitString(&a, "apple");
    piInitString(&b, "apple");

    int cmp = piStringCompare(&a, &b);
    assert(cmp == 0);
    printf("    \"apple\" cmp \"apple\" = %d ✓\n", cmp);
}

static void test_string_compare_less(void)
{
    printf("  Test: String compare (less than)\n");

    PiString a, b;
    piInitString(&a, "apple");
    piInitString(&b, "banana");

    int cmp = piStringCompare(&a, &b);
    assert(cmp < 0);
    printf("    \"apple\" < \"banana\" (%d) ✓\n", cmp);
}

static void test_string_compare_greater(void)
{
    printf("  Test: String compare (greater than)\n");

    PiString a, b;
    piInitString(&a, "banana");
    piInitString(&b, "apple");

    int cmp = piStringCompare(&a, &b);
    assert(cmp > 0);
    printf("    \"banana\" > \"apple\" (%d) ✓\n", cmp);
}

static void test_string_compare_prefix(void)
{
    printf("  Test: String compare (prefix)\n");

    PiString a, b;
    piInitString(&a, "app");
    piInitString(&b, "apple");

    int cmp = piStringCompare(&a, &b);
    assert(cmp < 0);
    printf("    \"app\" < \"apple\" (%d) ✓\n", cmp);
}

static void test_string_compare_ignore_case(void)
{
    printf("  Test: String compare ignore case\n");

    PiString a, b;
    piInitString(&a, "Apple");
    piInitString(&b, "BANANA");

    int cmp = piStringCompareIgnoreCase(&a, &b);
    assert(cmp < 0);
    printf("    \"Apple\" <i \"BANANA\" (%d) ✓\n", cmp);
}

static void test_string_compare_ignore_case_equal(void)
{
    printf("  Test: String compare ignore case (equal)\n");

    PiString a, b;
    piInitString(&a, "HeLLo");
    piInitString(&b, "hEllO");

    int cmp = piStringCompareIgnoreCase(&a, &b);
    assert(cmp == 0);
    printf("    \"HeLLo\" ==i \"hEllO\" (%d) ✓\n", cmp);
}

/* =---- Main --------------------------------------------------= */

int main(void)
{
    printf("Common String Library Test Suite\n");
    printf("=================================\n\n");

    printf("Hash Function Tests:\n");
    test_hash_empty_string();
    test_hash_simple_string();
    test_hash_consistency();
    test_hash_different_strings();

    printf("\nInitialization Tests:\n");
    test_init_string();
    test_init_string_with_length();
    test_init_owned_string();
    test_init_owned_string_with_length();

    printf("\nCopy and Duplicate Tests:\n");
    test_string_copy();
    test_string_copy_to();
    test_string_dup();
    test_string_dup_to();

    printf("\nFree Tests:\n");
    test_free_string_null();
    test_free_static_string();
    test_free_owned_string();

    printf("\nEquality Tests:\n");
    test_string_equals_same();
    test_string_equals_different();
    test_string_equals_different_length();
    test_string_equals_ignore_case_same();
    test_string_equals_ignore_case_different();

    printf("\nComparison Tests:\n");
    test_string_compare_equal();
    test_string_compare_less();
    test_string_compare_greater();
    test_string_compare_prefix();
    test_string_compare_ignore_case();
    test_string_compare_ignore_case_equal();

    printf("\n✓ All string tests passed!\n");
    return 0;
}
