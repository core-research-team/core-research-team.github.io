#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include "config.h"
#include "utils.h"

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        printf("Usage: %s [process id] [library full path]\n", argv[0]);
        return -1;
    }

    pid_t pid = atoi(argv[1]);
    const char *lib = argv[2];

    if (!pid) {
        printf("[+] TestInject dlopen(%s)\n", lib);
        void *r = dlopen(lib, RTLD_NOW | RTLD_LOCAL);
        printf("[+] TestInject Result=%p\n", r);
        sleep(-1);
    }

    if (ptrace_attach(pid) == -1) {
        printf("[-] ptrace_attach failed\n");
        return -1;
    }

    long raddr = remote_call(pid, mmap, 6LL, 0LL, 0x400LL, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0LL, 0LL);
    if (!raddr) {
        printf("[-] remote mmap() failed !!\n");
        return -1;
    }

    ptrace_write(pid, (uint8_t*)raddr, (uint8_t*)lib, strlen(lib) + 1);
    if (!remote_call(pid, dlopen, 2, raddr, RTLD_NOW | RTLD_LOCAL))
    {
        printf("[-] remote dlopen() failed !!\n");
        return -1;
    }

    if (remote_call(pid, munmap, 2, raddr, 0x400LL)) {
        printf("[-] remote munmap() failed !!\n");
        return -1;
    }
    
    ptrace_detach(pid);
    printf("[+] injected pid=%d raddr=%p\n", pid, (void *)raddr);
    return 0;
}
