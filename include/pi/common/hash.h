#pragma once

/**
 * @file        hash.h
 *
 * @author      Federico Crisitina <federico.cristina@outlook.it>
 *
 * @copyright   Copyright (c) 2025 Federico Cristina
 *
 *              Licensed under the Apache License, Version 2.0 (the "License");
 *              you may not use this file except in compliance with the License.
 *              You may obtain a copy of the License at
 *
 *                  http://www.apache.org/licenses/LICENSE-2.0
 *
 *              Unless required by applicable law or agreed to in writing, software
 *              distributed under the License is distributed on an "AS IS" BASIS,
 *              WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *              See the License for the specific language governing permissions and
 *              limitations under the License.
 *
 * @brief       General-purpose hash table with string keys using separate chaining.
 */

#ifndef _PI_COMMON_HASH_H
#define _PI_COMMON_HASH_H

/* Include PiString definition */
#include "pi/common/string.h"

PI_C_HEADER_BEGIN

/* =---- Hash Table --------------------------------------------= */

#ifndef PI_HASH_DEFAULT_CAPACITY
/**
 * @brief   Default initial capacity for hash tables.
 */
#   define PI_HASH_DEFAULT_CAPACITY 8
#endif

#ifndef PI_HASH_MAX_LOAD
/**
 * @brief   Maximum load factor before growth (0.75 = 75%).
 */
#   define PI_HASH_MAX_LOAD 0.75
#endif

#ifndef PI_HASH_MIN_LOAD
/**
 * @brief   Minimum load factor before shrink (0.25 = 25%).
 */
#   define PI_HASH_MIN_LOAD 0.25
#endif

/**
 * +---- HashValue ------------------------+
 */

/**
 * @brief   Hash table value type.
 *
 *          Can store either a pointer or an integer of pointer size.
 */
typedef union _pi_HashValue
{
    /**
     * @brief   Value as pointer.
     */
    void     *asPtr;
    /**
     * @brief   Value as unsigned integer.
     */
    uintptr_t asInt;
} PiHashValue;

/**
 * +---- HashEntry ------------------------+
 */

/**
 * @brief   Hash table entry for separate chaining.
 *
 *          Each entry forms a node in a singly-linked list (bucket chain).
 */
typedef struct _pi_HashEntry
{
    /**
     * @brief   Key (stored by value, may own char data if clear=1).
     */
    PiString              key;
    /**
     * @brief   Value (pointer or integer).
     */
    PiHashValue           value;
    /**
     * @brief   Next entry in the chain (NULL if end of chain).
     */
    struct _pi_HashEntry *next;
} PiHashEntry;

/**
 * +---- HashTable ------------------------+
 */

/**
 * @brief   Hash table structure using separate chaining.
 *
 *          A general-purpose hash table using PiString keys and PiHashValue.
 *          Collisions are resolved via linked lists (daisy chaining).
 */
typedef struct _pi_HashTable
{
    /**
     * @brief   Array of bucket pointers (each bucket is a linked list head).
     */
    PiHashEntry **buckets;
    /**
     * @brief   Number of buckets.
     */
    uint32_t      capacity;
    /**
     * @brief   Number of entries in the table.
     */
    uint32_t      count;
} PiHashTable;

/**
 * +---- HashTable::Init ------------------+
 */

/**
 * @brief   Initialize hash table with default capacity.
 *
 * @param[out] table  Hash table to initialize.
 *
 * @return  Pointer to initialized hash table (same as input).
 */
PI_Api(PiHashTable *) piInitHashTable(PiHashTable *const table);
/**
 * @brief   Initialize hash table with specified capacity.
 *
 * @param[out] table     Hash table to initialize.
 * @param[in]  capacity  Initial number of buckets (rounded up to power of 2).
 *
 * @return  Pointer to initialized hash table (same as input).
 */
PI_Api(PiHashTable *) piInitHashTableWithCapacity(PiHashTable *const table, const uint32_t capacity);

/**
 * +---- HashTable::Free ------------------+
 */

/**
 * @brief   Free hash table and all owned resources.
 *
 *          Frees all entries and any key char data where clear=1.
 *          Does NOT free values (user responsibility).
 *
 * @param[in,out] table  Hash table to free.
 *
 * @return  NULL (for convenience assignment).
 */
PI_Api(PiHashTable *) piFreeHashTable(PiHashTable *const table);

/**
 * +---- HashTable Operations -------------+
 */

/**
 * @brief   Get value by key.
 *
 * @param[in]  table  Hash table to search.
 * @param[in]  key    Key to look up.
 * @param[out] value  Output parameter for value (may be NULL to check existence).
 *
 * @return  true if key exists, false otherwise.
 */
PI_Api(bool) piHashTableGet(const PiHashTable *const table, const PiString *const key, PiHashValue *const value);

/**
 * @brief   Get value as pointer by key.
 *
 * @param[in]  table  Hash table to search.
 * @param[in]  key    Key to look up.
 *
 * @return  Pointer value if key exists, NULL otherwise.
 */
PI_Api(void *) piHashTableGetPtr(const PiHashTable *const table, const PiString *const key);
/**
 * @brief   Get value as integer by key.
 *
 * @param[in]  table         Hash table to search.
 * @param[in]  key           Key to look up.
 * @param[in]  defaultValue  Value to return if key not found.
 *
 * @return  Integer value if key exists, defaultValue otherwise.
 */
PI_Api(uintptr_t) piHashTableGetInt(const PiHashTable *const table, const PiString *const key, const uintptr_t defaultValue);

/**
 * @brief   Set key-value pair (insert or update).
 *
 *          If key exists, updates value and returns old value.
 *          If key does not exist, inserts new entry.
 *          Takes ownership of key if key.clear=1.
 *
 * @param[in,out] table     Hash table to modify.
 * @param[in]     key       Key to set (struct is copied, char data may be owned).
 * @param[in]     value     Value to associate with key.
 * @param[out]    oldValue  Output parameter for previous value (may be NULL).
 *
 * @return  true if key was already present (update), false if new key (insert).
 */
PI_Api(bool) piHashTableSet(PiHashTable *const table, const PiString *const key, const PiHashValue value, PiHashValue *const oldValue);

/**
 * @brief   Set key to pointer value.
 *
 * @param[in,out] table  Hash table to modify.
 * @param[in]     key    Key to set.
 * @param[in]     value  Pointer value.
 *
 * @return  true if key was already present (update), false if new key (insert).
 */
PI_Api(bool) piHashTableSetPtr(PiHashTable *const table, const PiString *const key, void *const value);
/**
 * @brief   Set key to integer value.
 *
 * @param[in,out] table  Hash table to modify.
 * @param[in]     key    Key to set.
 * @param[in]     value  Integer value.
 *
 * @return  true if key was already present (update), false if new key (insert).
 */
PI_Api(bool) piHashTableSetInt(PiHashTable *const table, const PiString *const key, const uintptr_t value);

/**
 * @brief   Delete entry by key.
 *
 *          Removes the entry and frees key char data if clear=1.
 *          Does NOT free pointer values (user responsibility).
 *
 * @param[in,out] table     Hash table to modify.
 * @param[in]     key       Key to delete.
 * @param[out]    oldValue  Output parameter for removed value (may be NULL).
 *
 * @return  true if key existed and was deleted, false if not found.
 */
PI_Api(bool) piHashTableDelete(PiHashTable *const table, const PiString *const key, PiHashValue *const oldValue);

/**
 * @brief   Check if key exists in table.
 *
 * @param[in] table  Hash table to search.
 * @param[in] key    Key to look up.
 *
 * @return  true if key exists, false otherwise.
 */
PI_Api(bool) piHashTableContains(const PiHashTable *const table, const PiString *const key);

/**
 * +---- HashTable Utilities --------------+
 */

/**
 * @brief   Get number of entries in hash table.
 *
 * @param[in] table  Hash table to query.
 *
 * @return  Number of entries, or 0 if table is NULL.
 */
PI_Api(uint32_t) piHashTableCount(const PiHashTable *const table);
/**
 * @brief   Get current capacity (number of buckets) of hash table.
 *
 * @param[in] table  Hash table to query.
 *
 * @return  Capacity, or 0 if table is NULL.
 */
PI_Api(uint32_t) piHashTableCapacity(const PiHashTable *const table);

/**
 * @brief   Clear all entries from hash table.
 *
 *          Removes all entries, freeing key char data where clear=1.
 *          Does NOT free pointer values. Retains current capacity.
 *
 * @param[in,out] table  Hash table to clear.
 */
PI_Api(void) piHashTableClear(PiHashTable *const table);

/**
 * +---- HashTableIterator ----------------+
 */

/**
 * @brief   Hash table iterator structure.
 */
typedef struct _pi_HashTableIterator
{
    /**
     * @brief   Table being iterated.
     */
    const PiHashTable *table;
    /**
     * @brief   Current bucket index.
     */
    uint32_t           bucketIndex;
    /**
     * @brief   Current entry in the chain.
     */
    PiHashEntry       *current;
} PiHashTableIterator;

/**
 * +---- HashTableIterator::Init ----------+
 */

/**
 * @brief   Initialize iterator for hash table.
 *
 * @param[out] iter   Iterator to initialize.
 * @param[in]  table  Hash table to iterate.
 */
PI_Api(void) piHashTableIterInit(PiHashTableIterator *const iter, const PiHashTable *const table);
/**
 * @brief   Advance iterator to next entry.
 *
 * @param[in,out] iter   Iterator to advance.
 * @param[out]    key    Output parameter for key pointer (may be NULL).
 * @param[out]    value  Output parameter for value (may be NULL).
 *
 * @return  true if entry was found, false if iteration complete.
 */
PI_Api(bool) piHashTableIterNext(PiHashTableIterator *const iter, const PiString **const key, PiHashValue *const value);

/**
 * +---- HashTable Helper Macros ----------+
 */

/**
 * @brief   Create a PiHashValue from a pointer.
 *
 * @param ptr  Pointer value.
 *
 * @return  PiHashValue containing the pointer.
 */
#define PI_HASH_VALUE_PTR(ptr) \
    ((PiHashValue){ .asPtr = (ptr) })
/**
 * @brief   Create a PiHashValue from an integer.
 *
 * @param val  Integer value.
 *
 * @return  PiHashValue containing the integer.
 */
#define PI_HASH_VALUE_INT(val) \
    ((PiHashValue){ .asInt = (uintptr_t)(val) })

/**
 * @brief   Iterate over hash table entries.
 *
 * @param table  Hash table to iterate.
 * @param kVar   Variable name for key pointer (const PiString*).
 * @param vVar   Variable name for value (PiHashValue).
 *
 * @example
 * @code
 *     const PiString *k;
 *     PiHashValue v;
 *     PI_HASH_TABLE_FOREACH(table, k, v) {
 *         printf("Key: %.*s, Value: %p\n", k->count, k->chars, v.asPtr);
 *     }
 * @endcode
 */
#define PI_HASH_TABLE_FOREACH(table, kVar, vVar) \
    for (PiHashTableIterator PI_HASH_ITER__ = {(table), 0, NULL}; \
         piHashTableIterNext(&PI_HASH_ITER__, &(kVar), &(vVar)); )

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
