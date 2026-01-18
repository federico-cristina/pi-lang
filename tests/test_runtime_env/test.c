#include "pi/runtime/env.h"

int main(void)
{
    PiEnv env;

    piInitEnv(&env);

    int *a = env.handlers.alloc(&env, sizeof(int) * 50);

    a[40] = 5;

    env.handlers.free(&env, (void *)a);

    piFreeEnv(&env);

    return EXIT_SUCCESS;
}
