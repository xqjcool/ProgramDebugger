#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <dlfcn.h>

void test_print();

void hook_print() {
    printf("new hook_print\n");
}

void* get_page_start(void* addr) {
    return (void *)((uintptr_t)addr & ~(getpagesize() - 1));
}

__attribute__((constructor)) void do_hook(void) {
    void *orig = (void *)test_print;
    void *hook = (void *)hook_print;

    printf("[hook.so] Injecting hook: %p → %p\n", orig, hook);

    if (mprotect(get_page_start(orig), getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC) != 0) {
        perror("mprotect failed");
        return;
    }

    unsigned char patch[14] = {
        0x48, 0xB8,                        // mov rax, imm64
        0, 0, 0, 0, 0, 0, 0, 0,            // imm64
        0xFF, 0xE0                         // jmp rax
    };
    memcpy(&patch[2], &hook, 8);

    memcpy(orig, patch, sizeof(patch));

    printf("[hook.so] Hook installed successfully.\n");
}

