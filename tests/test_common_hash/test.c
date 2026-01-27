/**
 * @file        test.c
 *
 * @brief       Common Hash Table Library Test Suite
 *
 * Tests all PiHashTable operations:
 * - Initialization and cleanup
 * - Insert, get, delete
 * - Pointer and integer value variants
 * - Automatic growth
 * - Iteration
 * - Key ownership
 */

#include "pi/common/hash.h"
#include "pi/common/memory.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* =---- Initialization Tests ----------------------------------= */

static void test_init_hash_table(void)
{
    printf("  Test: Initialize hash table with default capacity\n");

    PiHashTable table;
    piInitHashTable(&table);

    assert(table.buckets != NULL);
    assert(table.capacity == PI_HASH_DEFAULT_CAPACITY);
    assert(table.count == 0);
    printf("    capacity=%u, count=%u ✓\n", table.capacity, table.count);

    piFreeHashTable(&table);
}

static void test_init_hash_table_with_capacity(void)
{
    printf("  Test: Initialize hash table with custom capacity\n");

    PiHashTable table;
    piInitHashTableWithCapacity(&table, 32);

    assert(table.buckets != NULL);
    assert(table.capacity >= 32);
    assert(table.count == 0);
    printf("    capacity=%u (requested 32), count=%u ✓\n", table.capacity, table.count);

    piFreeHashTable(&table);
}

static void test_free_null_hash_table(void)
{
    printf("  Test: Free NULL hash table (should not crash)\n");

    PiHashTable *result = piFreeHashTable(NULL);
    assert(result == NULL);
    printf("    piFreeHashTable(NULL) returned NULL ✓\n");
}

/* =---- Basic Operations Tests --------------------------------= */

static void test_set_and_get(void)
{
    printf("  Test: Set and get value\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key;
    piInitString(&key, "testkey");

    int testValue = 42;
    piHashTableSetPtr(&table, &key, &testValue);

    PiHashValue value;
    bool found = piHashTableGet(&table, &key, &value);

    assert(found == true);
    assert(value.asPtr == &testValue);
    assert(*(int *)value.asPtr == 42);
    printf("    Set \"testkey\" -> 42, retrieved %d ✓\n", *(int *)value.asPtr);

    piFreeHashTable(&table);
}

static void test_get_ptr(void)
{
    printf("  Test: Get pointer value\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key;
    piInitString(&key, "ptrkey");

    int testValue = 100;
    piHashTableSetPtr(&table, &key, &testValue);

    void *ptr = piHashTableGetPtr(&table, &key);

    assert(ptr == &testValue);
    printf("    Retrieved pointer value ✓\n");

    piFreeHashTable(&table);
}

static void test_get_int(void)
{
    printf("  Test: Set and get integer value\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key;
    piInitString(&key, "intkey");

    piHashTableSetInt(&table, &key, 12345);

    uintptr_t value = piHashTableGetInt(&table, &key, 0);

    assert(value == 12345);
    printf("    Set \"intkey\" -> 12345, retrieved %llu ✓\n", (unsigned long long)value);

    piFreeHashTable(&table);
}

static void test_get_int_default(void)
{
    printf("  Test: Get integer with default value (key not found)\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key;
    piInitString(&key, "nonexistent");

    uintptr_t value = piHashTableGetInt(&table, &key, 999);

    assert(value == 999);
    printf("    Default value returned: %llu ✓\n", (unsigned long long)value);

    piFreeHashTable(&table);
}

static void test_get_nonexistent(void)
{
    printf("  Test: Get nonexistent key\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key;
    piInitString(&key, "nonexistent");

    PiHashValue value;
    bool found = piHashTableGet(&table, &key, &value);

    assert(found == false);
    printf("    Key not found ✓\n");

    piFreeHashTable(&table);
}

static void test_contains(void)
{
    printf("  Test: Contains key\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key1, key2;
    piInitString(&key1, "exists");
    piInitString(&key2, "notexists");

    piHashTableSetInt(&table, &key1, 1);

    assert(piHashTableContains(&table, &key1) == true);
    assert(piHashTableContains(&table, &key2) == false);
    printf("    \"exists\" found, \"notexists\" not found ✓\n");

    piFreeHashTable(&table);
}

static void test_update_value(void)
{
    printf("  Test: Update existing key value\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key;
    piInitString(&key, "updatekey");

    piHashTableSetInt(&table, &key, 100);
    assert(piHashTableGetInt(&table, &key, 0) == 100);

    PiHashValue oldValue;
    bool wasPresent = piHashTableSet(&table, &key, PI_HASH_VALUE_INT(200), &oldValue);

    assert(wasPresent == true);
    assert(oldValue.asInt == 100);
    assert(piHashTableGetInt(&table, &key, 0) == 200);
    printf("    Updated 100 -> 200, old value preserved ✓\n");

    piFreeHashTable(&table);
}

/* =---- Delete Tests ------------------------------------------= */

static void test_delete_existing(void)
{
    printf("  Test: Delete existing key\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key;
    piInitString(&key, "deletekey");

    piHashTableSetInt(&table, &key, 42);
    assert(table.count == 1);

    PiHashValue oldValue;
    bool deleted = piHashTableDelete(&table, &key, &oldValue);

    assert(deleted == true);
    assert(oldValue.asInt == 42);
    assert(table.count == 0);
    assert(piHashTableContains(&table, &key) == false);
    printf("    Deleted \"deletekey\", old value was 42 ✓\n");

    piFreeHashTable(&table);
}

static void test_delete_nonexistent(void)
{
    printf("  Test: Delete nonexistent key\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString key;
    piInitString(&key, "nonexistent");

    bool deleted = piHashTableDelete(&table, &key, NULL);

    assert(deleted == false);
    printf("    Delete returned false (key not found) ✓\n");

    piFreeHashTable(&table);
}

/* =---- Multiple Keys and Growth Tests ------------------------= */

static void test_multiple_keys(void)
{
    printf("  Test: Multiple keys\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString keys[5];
    piInitString(&keys[0], "one");
    piInitString(&keys[1], "two");
    piInitString(&keys[2], "three");
    piInitString(&keys[3], "four");
    piInitString(&keys[4], "five");

    for (int i = 0; i < 5; i++)
        piHashTableSetInt(&table, &keys[i], (uintptr_t)(i + 1));

    assert(table.count == 5);

    for (int i = 0; i < 5; i++)
    {
        uintptr_t val = piHashTableGetInt(&table, &keys[i], 0);
        assert(val == (uintptr_t)(i + 1));
    }

    printf("    Inserted and retrieved 5 keys ✓\n");

    piFreeHashTable(&table);
}

static void test_automatic_growth(void)
{
    printf("  Test: Automatic growth\n");

    PiHashTable table;
    piInitHashTableWithCapacity(&table, 8);

    uint32_t initialCapacity = table.capacity;
    char keyBuf[32];

    /* Insert enough entries to trigger growth */
    for (int i = 0; i < 20; i++)
    {
        sprintf(keyBuf, "key%d", i);
        PiString key;
        piInitString(&key, keyBuf);
        piHashTableSetInt(&table, &key, (uintptr_t)i);
    }

    assert(table.count == 20);
    assert(table.capacity > initialCapacity);
    printf("    Capacity grew from %u to %u (20 entries) ✓\n", initialCapacity, table.capacity);

    /* Verify all entries are still accessible */
    for (int i = 0; i < 20; i++)
    {
        sprintf(keyBuf, "key%d", i);
        PiString key;
        piInitString(&key, keyBuf);
        assert(piHashTableGetInt(&table, &key, 999) == (uintptr_t)i);
    }

    printf("    All entries accessible after growth ✓\n");

    piFreeHashTable(&table);
}

/* =---- Utilities Tests ---------------------------------------= */

static void test_count_and_capacity(void)
{
    printf("  Test: Count and capacity\n");

    PiHashTable table;
    piInitHashTable(&table);

    assert(piHashTableCount(&table) == 0);
    assert(piHashTableCapacity(&table) == PI_HASH_DEFAULT_CAPACITY);

    PiString key;
    piInitString(&key, "key");
    piHashTableSetInt(&table, &key, 1);

    assert(piHashTableCount(&table) == 1);
    printf("    count=%u, capacity=%u ✓\n", piHashTableCount(&table), piHashTableCapacity(&table));

    piFreeHashTable(&table);
}

static void test_clear(void)
{
    printf("  Test: Clear hash table\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString keys[3];
    piInitString(&keys[0], "a");
    piInitString(&keys[1], "b");
    piInitString(&keys[2], "c");

    for (int i = 0; i < 3; i++)
        piHashTableSetInt(&table, &keys[i], (uintptr_t)i);

    assert(table.count == 3);

    piHashTableClear(&table);

    assert(table.count == 0);
    assert(table.capacity == PI_HASH_DEFAULT_CAPACITY);

    for (int i = 0; i < 3; i++)
        assert(piHashTableContains(&table, &keys[i]) == false);

    printf("    Cleared 3 entries, capacity retained ✓\n");

    piFreeHashTable(&table);
}

/* =---- Iteration Tests ---------------------------------------= */

static void test_iteration(void)
{
    printf("  Test: Iteration\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString keys[3];
    piInitString(&keys[0], "first");
    piInitString(&keys[1], "second");
    piInitString(&keys[2], "third");

    piHashTableSetInt(&table, &keys[0], 1);
    piHashTableSetInt(&table, &keys[1], 2);
    piHashTableSetInt(&table, &keys[2], 3);

    PiHashTableIterator iter;
    piHashTableIterInit(&iter, &table);

    const PiString *key;
    PiHashValue value;
    int count = 0;
    uintptr_t sum = 0;

    while (piHashTableIterNext(&iter, &key, &value))
    {
        count++;
        sum += value.asInt;
    }

    assert(count == 3);
    assert(sum == 6);
    printf("    Iterated %d entries, sum=%llu ✓\n", count, (unsigned long long)sum);

    piFreeHashTable(&table);
}

static void test_iteration_empty(void)
{
    printf("  Test: Iteration on empty table\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiHashTableIterator iter;
    piHashTableIterInit(&iter, &table);

    const PiString *key;
    PiHashValue value;
    int count = 0;

    while (piHashTableIterNext(&iter, &key, &value))
        count++;

    assert(count == 0);
    printf("    Iterated 0 entries ✓\n");

    piFreeHashTable(&table);
}

static void test_foreach_macro(void)
{
    printf("  Test: PI_HASH_TABLE_FOREACH macro\n");

    PiHashTable table;
    piInitHashTable(&table);

    PiString keys[3];
    piInitString(&keys[0], "x");
    piInitString(&keys[1], "y");
    piInitString(&keys[2], "z");

    piHashTableSetInt(&table, &keys[0], 10);
    piHashTableSetInt(&table, &keys[1], 20);
    piHashTableSetInt(&table, &keys[2], 30);

    const PiString *k;
    PiHashValue v;
    int count = 0;
    uintptr_t sum = 0;

    PI_HASH_TABLE_FOREACH(&table, k, v)
    {
        count++;
        sum += v.asInt;
    }

    assert(count == 3);
    assert(sum == 60);
    printf("    Foreach: %d entries, sum=%llu ✓\n", count, (unsigned long long)sum);

    piFreeHashTable(&table);
}

/* =---- Key Ownership Tests -----------------------------------= */

static void test_owned_key(void)
{
    printf("  Test: Owned key (clear=1)\n");

    PiHashTable table;
    piInitHashTable(&table);

    /* Create an owned string key */
    char *cstr = piMalloc(10);
    strcpy(cstr, "ownedkey");

    PiString key;
    piInitOwnedString(&key, cstr);

    piHashTableSetInt(&table, &key, 999);

    assert(piHashTableContains(&table, &key) == true);
    assert(piHashTableGetInt(&table, &key, 0) == 999);
    printf("    Owned key inserted and retrieved ✓\n");

    /* Free table should also free the owned key's chars */
    piFreeHashTable(&table);
    printf("    Table freed (owned key memory released) ✓\n");
}

static void test_collision_handling(void)
{
    printf("  Test: Collision handling (separate chaining)\n");

    PiHashTable table;
    piInitHashTableWithCapacity(&table, 4);

    /* Insert many keys to force collisions */
    char keyBuf[32];
    for (int i = 0; i < 16; i++)
    {
        sprintf(keyBuf, "collision_key_%d", i);
        PiString key;
        piInitString(&key, keyBuf);
        piHashTableSetInt(&table, &key, (uintptr_t)i);
    }

    assert(table.count == 16);

    /* Verify all are retrievable */
    for (int i = 0; i < 16; i++)
    {
        sprintf(keyBuf, "collision_key_%d", i);
        PiString key;
        piInitString(&key, keyBuf);
        uintptr_t val = piHashTableGetInt(&table, &key, 999);
        assert(val == (uintptr_t)i);
    }

    printf("    16 keys with potential collisions handled correctly ✓\n");

    piFreeHashTable(&table);
}

/* =---- Main --------------------------------------------------= */

int main(void)
{
    printf("Common Hash Table Library Test Suite\n");
    printf("====================================\n\n");

    printf("Initialization Tests:\n");
    test_init_hash_table();
    test_init_hash_table_with_capacity();
    test_free_null_hash_table();

    printf("\nBasic Operations Tests:\n");
    test_set_and_get();
    test_get_ptr();
    test_get_int();
    test_get_int_default();
    test_get_nonexistent();
    test_contains();
    test_update_value();

    printf("\nDelete Tests:\n");
    test_delete_existing();
    test_delete_nonexistent();

    printf("\nMultiple Keys and Growth Tests:\n");
    test_multiple_keys();
    test_automatic_growth();

    printf("\nUtilities Tests:\n");
    test_count_and_capacity();
    test_clear();

    printf("\nIteration Tests:\n");
    test_iteration();
    test_iteration_empty();
    test_foreach_macro();

    printf("\nKey Ownership Tests:\n");
    test_owned_key();
    test_collision_handling();

    printf("\n✓ All hash table tests passed!\n");
    return 0;
}
