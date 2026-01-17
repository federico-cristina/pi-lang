#include "pi/runtime/env.h"

int main(void)
{
    pi_InitSystem();

    PiEnv env;

    piInitEnv(&env);

    int *a = env.handlers.alloc(&env, sizeof(int) * 50);

    a[40] = 5;

    env.handlers.free(&env, (void *)a);

    piFreeEnv(&env);

    pi_FreeSystem();

    return;
}
