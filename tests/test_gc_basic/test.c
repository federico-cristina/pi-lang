/**
 * @file        test_gc_basic.c
 *
 * @brief       Basic GC functionality test.
 *
 * Tests the updated piGcRelease API with immediate collection.
 */

#include "pi/runtime/gc.h"
#include "pi/runtime/env.h"
#include "pi/runtime/string.h"

#include <stdio.h>
#include <assert.h>

int main(void)
{
    printf("Testing GC with updated piGcRelease API...\n");

    PiEnv env;

    piInitEnv(&env);

    /* Test 1: Basic allocation and release */
    printf("  Test 1: Basic allocation and release\n");

    PiStringObject *str1 = piStringNew(&env, "Hello", 5);

    assert(str1 != NULL);
    assert(str1->base.refCount == 1);
    assert(env.gc.stats.objectCount == 1);

    printf("    - Allocated string: refCount=%d, objectCount=%zu\n",
           str1->base.refCount, env.gc.stats.objectCount);

    /* Test 2: Retain increases refcount */
    printf("  Test 2: Retain increases refcount\n");
    piGcRetain(&str1->base);

    assert(str1->base.refCount == 2);

    printf("    - After retain: refCount=%d\n", str1->base.refCount);

    /* Test 3: First release decrements but doesn't free */
    printf("  Test 3: First release decrements\n");
    piGcRelease(&env, &str1->base);

    assert(str1->base.refCount == 1);
    assert(env.gc.stats.objectCount == 1); /* Still alive */

    printf("    - After first release: refCount=%d, objectCount=%zu\n",
           str1->base.refCount, env.gc.stats.objectCount);

    /* Test 4: Second release frees immediately */
    printf("  Test 4: Second release frees immediately\n");
    piGcRelease(&env, &str1->base);

    assert(env.gc.stats.objectCount == 0); /* Freed */
    assert(env.gc.stats.objectsFreed == 1);

    printf("    - After second release: objectCount=%zu, objectsFreed=%zu\n",
           env.gc.stats.objectCount, env.gc.stats.objectsFreed);

    /* Test 5: Multiple objects */
    printf("  Test 5: Multiple objects\n");

    PiStringObject *str2 = piStringNew(&env, "World", 5);
    PiStringObject *str3 = piStringNew(&env, "Test", 4);

    assert(env.gc.stats.objectCount == 2);

    printf("    - Allocated 2 strings: objectCount=%zu\n", env.gc.stats.objectCount);

    piGcRelease(&env, &str2->base);
    piGcRelease(&env, &str3->base);

    assert(env.gc.stats.objectCount == 0);
    assert(env.gc.stats.objectsFreed == 3); /* Total freed including str1 */

    printf("    - After releasing both: objectCount=%zu, objectsFreed=%zu\n",
           env.gc.stats.objectCount, env.gc.stats.objectsFreed);

    piFreeEnv(&env);

    printf("All tests passed!\n");

    return 0;
}
