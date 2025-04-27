#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

#define DLOPEN_PARAMS_NUM	2
#define LIBC_NAME	"libc.so"

static void *get_libc_base(pid_t pid) {
	char path[64] = {0}, line[512] = {0};
	FILE *f = NULL;
	void *addr = NULL;

	if (pid > 0) {
		snprintf(path, sizeof(path), "/proc/%d/maps", pid);
	} else {
		snprintf(path, sizeof(path), "/proc/self/maps");
	}

	f = fopen(path, "r");
	while (fgets(line, sizeof(line), f)) {
		if (strstr(line, LIBC_NAME)) {
			sscanf(line, "%p-", &addr);
			break;
		}
	}
	fclose(f);

	return addr;
}

static void ptrace_attach(pid_t pid)
{
	ptrace(PTRACE_ATTACH, pid, NULL, NULL);
	waitpid(pid, NULL, 0);
}

static void ptrace_detach(pid_t pid, struct user_regs_struct *regs)
{
	ptrace(PTRACE_SETREGS, pid, 0, regs);
	ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

static void ptrace_getregs(pid_t pid, struct user_regs_struct *regs)
{
	ptrace(PTRACE_GETREGS, pid, 0, regs);
}

static void ptrace_pokedata(pid_t pid, void *dst_addr, const void *src_addr, size_t length)
{
	size_t i;

	for (i = 0; i < length; i += sizeof(long)) {
		long word = 0;
		memcpy(&word, src_addr + i, sizeof(long));
		ptrace(PTRACE_POKEDATA, pid, dst_addr + i, word);
	}
}

void call_remote_func(pid_t pid, struct user_regs_struct *regs, void *func, int argc, ...)
{
	va_list ap;
	unsigned long args[6] = {0};
	int i;

	if (argc < 0 || argc > 6) {
		fprintf(stderr, "[-] Only 0 to 6 arguments are supported.\n");
		return;
	}

	va_start(ap, argc);
	for (i = 0; i < argc; i++) {
		args[i] = va_arg(ap, unsigned long);
	}
	va_end(ap);

	regs->rdi = args[0];
	regs->rsi = args[1];
	regs->rdx = args[2];
	regs->rcx = args[3];
	regs->r8  = args[4];
	regs->r9  = args[5];

	regs->rip = (unsigned long)func;
	regs->rax = (unsigned long)func;

	regs->rsp -= 8;
	ptrace(PTRACE_POKEDATA, pid, regs->rsp, 0);

	ptrace(PTRACE_SETREGS, pid, NULL, regs);
	ptrace(PTRACE_CONT, pid, NULL, NULL);
	waitpid(pid, NULL, 0);
}

int main(int argc, char *argv[]) {
	pid_t pid = 0;
	char *so_path = NULL;
	void *local_libc = NULL, *local_dlopen = NULL;
	void *remote_libc = NULL, *remote_dlopen = NULL;
	size_t dlopen_offset = 0;
	void *remote_str = NULL;

	if (argc != 3) {
		printf("Usage: %s <pid> <full-path-to-hook.so>\n", argv[0]);
		return -1;
	}

	pid = atoi(argv[1]);
	so_path = argv[2];

	local_libc = get_libc_base(0);
	local_dlopen = dlsym(RTLD_NEXT, "dlopen");
	dlopen_offset = (char *)local_dlopen - (char *)local_libc;
	printf("[+] Local libc base:%p, dlopen offset: 0x%lx\n", local_libc, dlopen_offset);

	remote_libc =  get_libc_base(pid);
	if (!remote_libc) {
		printf("[-] Failed to locate remote libc base.\n");
		return -1;
	}

	remote_dlopen = (char *)remote_libc + dlopen_offset;
	printf("[+] Remote libc base: %p, dlopen address:%p\n", remote_libc, remote_dlopen);

	printf("[+] Attaching to PID %d...\n", pid);
	ptrace_attach(pid);

	struct user_regs_struct regs, saved;
	ptrace_getregs(pid, &regs);
	saved = regs;

	regs.rsp -= strlen(so_path) + 1;
	regs.rsp &= ~0xF;
	remote_str = (void *)regs.rsp;
	ptrace_pokedata(pid, remote_str, so_path, strlen(so_path) + 1);

	/* call dlopen(so_path, RTLD_NOW|RTLD_GLOBAL) */
	call_remote_func(pid, &regs, remote_dlopen, DLOPEN_PARAMS_NUM, remote_str, RTLD_NOW|RTLD_GLOBAL);

	ptrace_getregs(pid, &regs);
	printf("[+] dlopen() return value in remote: %llx\n", regs.rax);

	ptrace_detach(pid, &saved);

	printf("[+] Injected %s into PID %d\n", so_path, pid);
	return 0;
}


