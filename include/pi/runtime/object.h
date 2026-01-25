#pragma once

/**
 * @file        object.h
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
 * @brief       Object system with garbage collection support.
 */

#ifndef _PI_RUNTIME_OBJECT_H
#define _PI_RUNTIME_OBJECT_H

#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- Objects -----------------------------------------------= */

/**
 * +---- ObjectType -----------------------+
 */

/**
 * @brief   Object type enumeration for runtime type identification.
 *
 * This enum defines all possible object types that can be allocated
 * and managed by the garbage collector.
 */
typedef enum _pi_ObjectType
{
    /**
     * @brief   String object type.
     */
    PI_OBJECT_TYPE_STRING,
    /**
     * @brief   Array object type.
     */
    PI_OBJECT_TYPE_ARRAY,
    /**
     * @brief   Native function object type.
     */
    PI_OBJECT_TYPE_NATIVE,
} PiObjectType;

/**
 * +---- ObjectFlags ----------------------+
 */

/**
 * @brief   Object flags for GC and metadata.
 *
 * These flags are used to track object state during garbage collection
 * and to store metadata about object properties.
 */
typedef enum _pi_ObjectFlags
{
    /**
     * @brief   No flags set.
     */
    PI_OBJECT_FLAG_NONE     = 0,
    /**
     * @brief   Mark bit for mark-and-sweep GC.
     */
    PI_OBJECT_FLAG_MARKED   = PI_BitFlag(1),
    /**
     * @brief   Gray list for incremental marking (future).
     */
    PI_OBJECT_FLAG_GRAY     = PI_BitFlag(2),
    /**
     * @brief   Immutable object (optimization).
     */
    PI_OBJECT_FLAG_FROZEN   = PI_BitFlag(3),
    /**
     * @brief   External memory (not GC allocated).
     */
    PI_OBJECT_FLAG_EXTERNAL = PI_BitFlag(4),
} PiObjectFlags;

/**
 * +---- Object ---------------------------+
 */

/**
 * @brief   Base object structure with garbage collection support.
 *
 * All GC-managed objects start with this header. The header contains
 * intrusive linked list pointers for GC tracking, reference count for
 * immediate reclamation, and metadata for type and flags.
 *
 * Total size: 32 bytes (aligned to 8 bytes for NaN-boxing compatibility)
 *
 * Memory layout:
 * - Bytes 0-15:  GC intrusive linked list pointers (gcNext, gcPrev)
 * - Bytes 16-23: Object metadata (type, flags, refCount)
 * - Bytes 24-31: Memory tracking (size, padding)
 */
typedef struct _pi_Object
{
    /* GC intrusive linked list (16 bytes) */
    struct _pi_Object  *gcNext;      /**< Next object in GC list. */
    struct _pi_Object  *gcPrev;      /**< Previous object (doubly-linked for O(1) removal). */

    /* Object metadata (8 bytes) */
    PiObjectType        type;        /**< Object type discriminator (4 bytes). */
    uint16_t            flags;       /**< GC flags + user flags (2 bytes). */
    uint16_t            refCount;    /**< Reference count (2 bytes, max 65535). */

    /* Memory tracking (8 bytes) */
    uint32_t            size;        /**< Total size in bytes (for statistics). */
    uint32_t            padding;     /**< Reserved for alignment. */
} PiObject;

/**
 * +---- StringObject ---------------------+
 */

/**
 * @brief   String object structure.
 *
 * Extends the base PiObject with string-specific fields. The string data
 * is stored inline using a flexible array member, which allows the entire
 * string (header + data) to be allocated in a single memory block.
 *
 * Memory layout:
 * - Bytes 0-31:  Base PiObject header
 * - Bytes 32-39: String metadata (length, hash)
 * - Bytes 40+:   String data (null-terminated)
 */
typedef struct _pi_StringObject
{
    /**
     * @brief   Base object header.
     */
    PiObject  base;
    /**
     * @brief   String length (excluding null terminator).
     */
    uint32_t  length;
    /**
     * @brief   Cached hash code (FNV-1a).
     */
    uint32_t  hash;
    /**
     * @brief   Flexible array member for string data.
     */
    char      data[];
} PiStringObject;

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
