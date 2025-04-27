#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>
#include "funchook.h"

HOOK_DEFINE(test_print, void, void)
{
	printf("[%s:%d]\n", __func__, __LINE__);
	CALL_ORIG_FUNCION(test_print);
	printf("[%s:%d]\n", __func__, __LINE__);
}

#if 0
void test_print();

void hook_print() {
    printf("new hook_print\n");
}
void* get_page_start(void* addr) {
    return (void *)((uintptr_t)addr & ~(getpagesize() - 1));
}
#endif

__attribute__((constructor)) void do_hook(void) {
    printf("[hook.so] Injecting hook: \n");

    HOOK_REGISTER(test_print);

    printf("[hook.so] Hook installed successfully. *pptr:%p\n", ptr_test_print);
}

