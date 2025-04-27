/*
 * Copyright (c) 2025 
 *
 * funchook.c - function hook implementation
 * author: Xing Qingjie <xqjcool@gmail.com>
 * version: 1.0.0
 * history: 04/27/2025	created
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <signal.h>

#define INSTRSIZE	12

/* used to restore the instucts back in signal handler */
static struct restore {
	struct sigaction old_sa;
	void *dst;
	uint8_t backup[32];
} g_restore;

static void restore_sig_handler(int signo)
{
	printf("[%s:%d] receive signal %d\n", __func__, __LINE__, signo);
	memcpy(g_restore.dst, g_restore.backup, INSTRSIZE);
	if (g_restore.old_sa.sa_handler && g_restore.old_sa.sa_handler != SIG_IGN &&
	    g_restore.old_sa.sa_handler != SIG_DFL) {
		g_restore.old_sa.sa_handler(signo);
	}
}

/* may need to add more instruct support */
static size_t get_inst_len(uint8_t *code)
{
	uint8_t opcode = code[0];

	switch (opcode) {
	case 0xC3: // ret
	case 0x90: // nop
		return 1;

	case 0xE8: // call rel32
	case 0xE9: // jmp rel32
	case 0x68: // push imm32
		return 5;

	case 0xEB: // short jmp rel8
		return 2;

	case 0x0F: { // 0F 扩展指令
		uint8_t second = code[1];
		if (second == 0x1F) return 3; // nop dword ptr
		return 6; // 粗略估计复杂0F开头指令
	}

	case 0xF3: { // F3 前缀指令
		if (code[1] == 0x0F && code[2] == 0x1E && code[3] == 0xFA) return 4; // endbr64
		if (code[1] == 0x90) return 2; // pause
		return 1; // 其他F3前缀，保守估计1字节
	}

	default:
	if ((opcode & 0xF8) == 0x50 || (opcode & 0xF8) == 0x58) return 1; // push/pop rX
	if ((opcode & 0xFC) == 0x80) return 4; // add/sub/cmp r/m8, imm8
	if ((opcode & 0xFC) == 0x88) return 3; // mov r/m8, r8
	if (opcode == 0x8B) return 3; // mov r64, r/m64
	return 1; // 保底返回1字节
	}
}

static size_t copy_with_relocate(uint8_t *dst, uint8_t *src, size_t min_size, size_t *src_copied)
{
    size_t copied = 0;     // dst累计写了多少
    size_t src_used = 0;   // src累计读了多少

    while (copied < min_size) {
        uint8_t opcode = src[src_used];
        size_t inst_len = 0;

        if (opcode == 0xE8 || opcode == 0xE9) { // call / jmp rel32
            int32_t rel = *(int32_t *)(src + src_used + 1);
            uintptr_t src_inst_addr = (uintptr_t)(src + src_used);
            uintptr_t target_addr = src_inst_addr + 5 + rel;

            // Build movabs+call/jmp
            dst[copied + 0] = 0x48;
            dst[copied + 1] = 0xB8; // mov rax, imm64
            *(uintptr_t *)(dst + copied + 2) = target_addr;

            if (opcode == 0xE8) {
                dst[copied + 10] = 0xFF;
                dst[copied + 11] = 0xD0; // call rax
            } else {
                dst[copied + 10] = 0xFF;
                dst[copied + 11] = 0xE0; // jmp rax
            }

            src_used += 5;     // 原指令长度5字节
            copied   += 12;    // 新指令长度12字节
        } else {
            inst_len = get_inst_len(src + src_used);
            memcpy(dst + copied, src + src_used, inst_len);

            src_used += inst_len;
            copied   += inst_len;
        }
    }

    if (src_copied) {
        *src_copied = src_used;
    }

    return copied; // 返回写到dst多少字节
}

static void *get_func_addr(const char *func_name)
{
    void *handle = dlopen(NULL, RTLD_NOW);  // NULL = 当前进程自己
    if (!handle) {
        perror("dlopen failed");
        return NULL;
    }
    return dlsym(handle, func_name);
}

static inline int make_func_writable(void *func_addr)
{
	int ret;
	void *page_addr = (void *)((uintptr_t)func_addr & ~(getpagesize() - 1));
	ret = mprotect(page_addr, getpagesize(), PROT_READ | PROT_WRITE | PROT_EXEC);
	if(ret) {
		perror("mprotect failed");
	}
	return ret;
}

int hook_register(const char *func_name, void **pptr_func, 
		void *hook_func, void *stub_func)
{
	unsigned char jump_to_orig[INSTRSIZE] = {
		0x48, 0xB8,                        // mov rax, imm64
		0, 0, 0, 0, 0, 0, 0, 0,            // imm64
		0xFF, 0xE0                         // jmp rax
	};
	unsigned char jump_to_hook[INSTRSIZE] = {
		0x48, 0xB8,                        // mov rax, imm64
		0, 0, 0, 0, 0, 0, 0, 0,            // imm64
		0xFF, 0xE0                         // jmp rax
	};
	void *jump_back;
	size_t inst_size1, inst_size2;
	struct sigaction sa = {0};

	/* get function address */
	*pptr_func = (void *)get_func_addr(func_name);
	if (!*pptr_func) {
		perror("get function address failed");
		return -ENOENT;
	}

	/* backup the instructs */
	memcpy(g_restore.backup, *pptr_func, INSTRSIZE);

	/* make stub function code writable */
	if (make_func_writable(stub_func)) {
		perror("make stub function writable failed");
		return -EINVAL;
	}
	/* copy code to stub func */
	inst_size2 = copy_with_relocate(stub_func, *pptr_func, INSTRSIZE, &inst_size1);
	/* copy jump instructs to stub function */
	jump_back = *pptr_func + inst_size1;
	memcpy(&jump_to_orig[2], &jump_back, 8);
	memcpy(stub_func+inst_size2, jump_to_orig, INSTRSIZE);

	/* make orig function writable */
	if (make_func_writable(*pptr_func)) {
		perror("make original function writable failed");
		return -EINVAL;
	}
	/* copy jump instructs to orig function */
	memcpy(&jump_to_hook[2], &hook_func, 8);
	memcpy(*pptr_func, jump_to_hook, INSTRSIZE);

	/* setup sig handler to restore original function */
	g_restore.dst = *pptr_func;
	sa.sa_handler = restore_sig_handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR2, &sa, &g_restore.old_sa);

	return 0;
}
