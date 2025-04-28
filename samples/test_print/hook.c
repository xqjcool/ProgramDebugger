/* headers needed for patched function */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>
/* 1. header for HOOK_DEFINE,HOOK_REGISTER,HOOK_CALL_ORIG_FUNCTION */
#include "funchook.h"

/* macro definitions */

/* structure definitions */

/* global params */

/* functions needed by patched function */

/* 2. define the hook function group */
HOOK_DEFINE(test_print, void, long rnd)
{
	int changed = rnd % 100;
	printf("[%s:%d] DEBUG passed rnd:%ld, we change it to %d\n", __func__, __LINE__, rnd, changed);
	/* 3. call the original function */
	CALL_ORIG_FUNCION(test_print, changed);
	printf("[%s:%d] DEBUG changed rnd:%d\n", __func__, __LINE__, changed);
}

/* constructor function */
__attribute__((constructor)) void do_hook(void) {
    printf("[hook.so] Injecting hook: \n");

    /* 4. register the hook fuction group */
    HOOK_REGISTER(test_print);

    printf("[hook.so] Hook installed successfully.\n");
}

/* 5. use 'kill -SIGUSR2 <pid>' to restore the target function if you want */
