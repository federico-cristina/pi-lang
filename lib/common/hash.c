#include "pi/common/hash.h"

#include <string.h>

/* =---- Hash Table --------------------------------------------= */

/**
 * +---- Internal Utilities ---------------+
 */

/**
 * @brief   Round up to next power of 2.
 *
 * @param[in] n  Value to round up.
 *
 * @return  Next power of 2 >= n.
 */
static inline uint32_t pi_NextPowerOf2(uint32_t n)
{
    if (n == 0)
        return 1;

    n--;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;

    n++;

    return n;
}

/**
 * @brief   Compute bucket index from hash.
 *
 * @param[in] hash      Hash value.
 * @param[in] capacity  Number of buckets (must be power of 2).
 *
 * @return  Bucket index.
 */
static inline uint32_t pi_BucketIndex(const uint32_t hash, const uint32_t capacity)
{
    return hash & (capacity - 1);
}

/**
 * +---- Entry Management -----------------+
 */

/**
 * @brief   Allocate and initialize a new entry.
 *
 * @param[in] key    Key for the entry.
 * @param[in] value  Value for the entry.
 *
 * @return  Pointer to new entry, or NULL on failure.
 */
static inline PiHashEntry *pi_AllocHashEntry(const PiString *const key, const PiHashValue value)
{
    PiHashEntry *const entry = PI_New(PiHashEntry);

    if (PI_UNLIKELY(entry == NULL))
        return NULL;

    entry->key   = *key;
    entry->value = value;
    entry->next  = NULL;

    return entry;
}

/**
 * @brief   Free an entry and its owned key data.
 *
 * @param[in] entry  Entry to free.
 */
static inline void pi_FreeHashEntry(PiHashEntry *const entry)
{
    piFreeString(&entry->key);
    piFree(entry);

    return;
}

/**
 * @brief   Free all entries in a bucket chain.
 *
 * @param[in] head  Head of the bucket chain.
 */
static inline void pi_FreeBucketChain(PiHashEntry *head)
{
    while (head != NULL)
    {
        PiHashEntry *const next = head->next;

        pi_FreeHashEntry(head);

        head = next;
    }

    return;
}

/**
 * +---- Find Entry -----------------------+
 */

/**
 * @brief   Find entry by key in bucket chain.
 *
 *          Uses hash comparison as fast-path rejection before
 *          comparing the actual string contents.
 *
 * @param[in]  head  Head of bucket chain.
 * @param[in]  key   Key to search for.
 * @param[out] prev  Previous entry pointer (for deletion), may be NULL.
 *
 * @return  Entry if found, NULL otherwise.
 */
static inline PiHashEntry *pi_FindEntry(PiHashEntry *head, const PiString *const key, PiHashEntry **prev)
{
    const uint32_t keyHash  = key->hash;
    const uint32_t keyCount = key->count;

    PiHashEntry *prevEntry  = NULL;

    while (head != NULL)
    {
        /* Fast path: hash and length mismatch = skip (no function call) */
        if ((head->key.hash == keyHash) &&
            (head->key.count == keyCount) &&
            (memcmp(head->key.chars, key->chars, keyCount) == 0))
        {
            if (prev != NULL)
                *prev = prevEntry;

            return head;
        }

        prevEntry = head;
        head = head->next;
    }

    return NULL;
}

/**
 * +---- Rehash ---------------------------+
 */

/**
 * @brief   Resize hash table to new capacity.
 *
 * @param[in,out] table        Hash table to resize.
 * @param[in]     newCapacity  New bucket count (must be power of 2).
 */
static void pi_HashTableResize(PiHashTable *const table, const uint32_t newCapacity)
{
    const uint32_t oldCapacity = table->capacity;

    PiHashEntry **const newBuckets = PI_NewArray(PiHashEntry *, newCapacity);

    if (PI_UNLIKELY(newBuckets == NULL))
        piFatal("hash table resize failed: out of memory (%u buckets)", newCapacity);

    /* Rehash all entries */
    for (uint32_t i = 0; i < oldCapacity; i++)
    {
        PiHashEntry *entry = table->buckets[i];

        while (entry != NULL)
        {
            PiHashEntry *const next = entry->next;

            const uint32_t newIndex =
                pi_BucketIndex(entry->key.hash, newCapacity);

            entry->next = newBuckets[newIndex];
            newBuckets[newIndex] = entry;

            entry = next;
        }
    }

    piFree(table->buckets);

    table->buckets  = newBuckets;
    table->capacity = newCapacity;

    return;
}

/**
 * @brief   Grow hash table when load factor exceeded.
 *
 * @param[in,out] table  Hash table to grow.
 */
static inline void pi_HashTableGrow(PiHashTable *const table)
{
    pi_HashTableResize(table, table->capacity * 2);

    return;
}

/**
 * @brief   Shrink hash table when load factor too low.
 *
 * @param[in,out] table  Hash table to shrink.
 */
static inline void pi_HashTableShrink(PiHashTable *const table)
{
    const uint32_t newCapacity = table->capacity / 2;

    /* Never shrink below default capacity */
    if (newCapacity >= PI_HASH_DEFAULT_CAPACITY)
        pi_HashTableResize(table, newCapacity);

    return;
}

/**
 * +---- HashTable::Init ------------------+
 */

PI_Api(PiHashTable *) piInitHashTable(PiHashTable *const table)
{
    return piInitHashTableWithCapacity(table, PI_HASH_DEFAULT_CAPACITY);
}

PI_Api(PiHashTable *) piInitHashTableWithCapacity(PiHashTable *const table, const uint32_t capacity)
{
    assert(table != NULL);

    const uint32_t actualCapacity =
        pi_NextPowerOf2(capacity < PI_HASH_DEFAULT_CAPACITY ? PI_HASH_DEFAULT_CAPACITY : capacity);

    table->buckets  = PI_NewArray(PiHashEntry *, actualCapacity);
    table->capacity = actualCapacity;
    table->count    = 0;

    if (PI_UNLIKELY(table->buckets == NULL))
        piFatal("hash table init failed: out of memory (%u buckets)", actualCapacity);

    return table;
}

/**
 * +---- HashTable::Free ------------------+
 */

PI_Api(PiHashTable *) piFreeHashTable(PiHashTable *const table)
{
    if (table == NULL)
        return NULL;

    if (table->buckets != NULL)
    {
        for (uint32_t i = 0; i < table->capacity; i++)
            pi_FreeBucketChain(table->buckets[i]);

        piFree(table->buckets);
    }

    table->buckets  = NULL;
    table->capacity = 0;
    table->count    = 0;

    return NULL;
}

/**
 * +---- HashTable Operations -------------+
 */

PI_Api(bool) piHashTableGet(const PiHashTable *const table, const PiString *const key, PiHashValue *const value)
{
    assert(table != NULL);
    assert(key != NULL);

    if (PI_UNLIKELY(table->count == 0))
        return false;

    const uint32_t index =
        pi_BucketIndex(key->hash, table->capacity);

    PiHashEntry *const entry = pi_FindEntry(table->buckets[index], key, NULL);

    if (entry == NULL)
        return false;

    if (value != NULL)
        *value = entry->value;

    return true;
}

PI_Api(void *) piHashTableGetPtr(const PiHashTable *const table, const PiString *const key)
{
    PiHashValue value;
    return piHashTableGet(table, key, &value) ? value.asPtr : NULL;
}

PI_Api(uintptr_t) piHashTableGetInt(const PiHashTable *const table, const PiString *const key, const uintptr_t defaultValue)
{
    PiHashValue value;
    return piHashTableGet(table, key, &value) ? value.asInt : defaultValue;
}

PI_Api(bool) piHashTableSet(PiHashTable *const table, const PiString *const key, const PiHashValue value, PiHashValue *const oldValue)
{
    assert(table != NULL);
    assert(key != NULL);

    /* Check load factor */
    if (table->count + 1 > (uint32_t)(table->capacity * PI_HASH_MAX_LOAD))
        pi_HashTableGrow(table);

    const uint32_t index =
        pi_BucketIndex(key->hash, table->capacity);

    PiHashEntry *const entry = pi_FindEntry(table->buckets[index], key, NULL);

    if (entry != NULL)
    {
        /* Update existing entry */
        if (oldValue != NULL)
            *oldValue = entry->value;

        piFreeString(&entry->key);

        entry->key   = *key;
        entry->value = value;

        return true;
    }

    /* Insert new entry at head */
    PiHashEntry *const newEntry = pi_AllocHashEntry(key, value);

    if (PI_UNLIKELY(newEntry == NULL))
        piFatal("hash table insert failed: out of memory", NULL);

    newEntry->next = table->buckets[index];
    table->buckets[index] = newEntry;
    table->count++;

    if (oldValue != NULL)
        oldValue->asPtr = NULL;

    return false;
}

PI_Api(bool) piHashTableSetPtr(PiHashTable *const table, const PiString *const key, void *const value)
{
    return piHashTableSet(table, key, PI_HASH_VALUE_PTR(value), NULL);
}

PI_Api(bool) piHashTableSetInt(PiHashTable *const table, const PiString *const key, const uintptr_t value)
{
    return piHashTableSet(table, key, PI_HASH_VALUE_INT(value), NULL);
}

PI_Api(bool) piHashTableDelete(PiHashTable *const table, const PiString *const key, PiHashValue *const oldValue)
{
    assert(table != NULL);
    assert(key != NULL);

    if (PI_UNLIKELY(table->count == 0))
        return false;

    const uint32_t index =
        pi_BucketIndex(key->hash, table->capacity);
    
    PiHashEntry *prev = NULL;
    PiHashEntry *const entry = pi_FindEntry(table->buckets[index], key, &prev);

    if (entry == NULL)
        return false;

    if (oldValue != NULL)
        *oldValue = entry->value;

    /* Unlink from chain */
    if (prev != NULL)
        prev->next = entry->next;
    else
        table->buckets[index] = entry->next;

    pi_FreeHashEntry(entry);

    table->count--;

    /* Check if we should shrink */
    if (table->count < (uint32_t)(table->capacity * PI_HASH_MIN_LOAD))
        pi_HashTableShrink(table);

    return true;
}

PI_Api(bool) piHashTableContains(const PiHashTable *const table, const PiString *const key)
{
    return piHashTableGet(table, key, NULL);
}

/**
 * +---- HashTable Utilities --------------+
 */

PI_Api(uint32_t) piHashTableCount(const PiHashTable *const table)
{
    return table != NULL ? table->count : 0;
}

PI_Api(uint32_t) piHashTableCapacity(const PiHashTable *const table)
{
    return table != NULL ? table->capacity : 0;
}

PI_Api(void) piHashTableClear(PiHashTable *const table)
{
    if (table == NULL || table->buckets == NULL)
        return;

    for (uint32_t i = 0; i < table->capacity; i++)
    {
        pi_FreeBucketChain(table->buckets[i]);
        table->buckets[i] = NULL;
    }

    table->count = 0;
}

/**
 * +---- HashTableIterator ----------------+
 */

PI_Api(void) piHashTableIterInit(PiHashTableIterator *const iter, const PiHashTable *const table)
{
    assert(iter != NULL);

    iter->table       = table;
    iter->bucketIndex = 0;
    iter->current     = NULL;
}

PI_Api(bool) piHashTableIterNext(PiHashTableIterator *const iter, const PiString **const key, PiHashValue *const value)
{
    assert(iter != NULL);

    if (iter->table == NULL || iter->table->buckets == NULL)
        return false;

    /* Advance to next entry in chain */
    if (iter->current != NULL)
        iter->current = iter->current->next;

    /* Find next non-empty bucket */
    while (iter->current == NULL && iter->bucketIndex < iter->table->capacity)
    {
        iter->current = iter->table->buckets[iter->bucketIndex];
        iter->bucketIndex++;
    }

    if (iter->current == NULL)
        return false;

    if (key != NULL)
        *key = &iter->current->key;

    if (value != NULL)
        *value = iter->current->value;

    return true;
}

/* =------------------------------------------------------------= */
