#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>
#include "funchook.h"

HOOK_DEFINE(test_print, void, long rnd)
{
	int changed = rnd % 100;
	printf("[%s:%d] passed rnd:%ld\n", __func__, __LINE__, rnd);
	CALL_ORIG_FUNCION(test_print, changed);
	printf("[%s:%d] changed rnd:%d\n", __func__, __LINE__, changed);
}

__attribute__((constructor)) void do_hook(void) {
    printf("[hook.so] Injecting hook: \n");

    HOOK_REGISTER(test_print);

    printf("[hook.so] Hook installed successfully.\n");
}

