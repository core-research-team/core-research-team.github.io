#include <dirent.h>
#include <stdbool.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <linux/elf.h>
#include <stdarg.h>
#include "config.h"
#include "utils.h"

#if defined(__aarch64__)
    #define pt_regs    user_pt_regs
    #define uregs      regs
    #define ARM_r0     regs[0]
    #define ARM_lr     regs[30]
    #define ARM_sp     sp
    #define ARM_pc     pc
    #define ARM_cpsr pstate
#endif

static void ptrace_get_regs(pid_t pid, struct pt_regs *regs);
static void ptrace_set_regs(pid_t pid, struct pt_regs *regs);

int ptrace_attach(pid_t pid) {
    if (pid < 0)
        return -1;

    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        perror(NULL);
        return -1;
    }
    waitpid(pid, NULL, WUNTRACED);
    return 0;
}

int ptrace_detach(pid_t pid) {
    if (pid < 0)
        return -1;

    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) < 0) {
        perror(NULL);
        return -1;
    }
    return 0;
}

void ptrace_write(pid_t pid, uint8_t* addr, uint8_t* data, size_t size) {
    const size_t WORD_SIZE = sizeof(long);
    int mod = size % WORD_SIZE;
    int loop_count = size / WORD_SIZE;
    uint8_t* tmp_addr = addr;
    uint8_t* tmp_data = data;

    for (int i = 0; i < loop_count; ++i) {
        ptrace(PTRACE_POKEDATA, pid, tmp_addr, *((long*)tmp_data));
        tmp_addr += WORD_SIZE;
        tmp_data += WORD_SIZE;
    }
    if (mod > 0) {
        long val = ptrace(PTRACE_PEEKDATA, pid, tmp_addr, NULL);
        uint8_t* p = (uint8_t*) &val;
        for (int i = 0; i < mod; ++i) {
            *p = *(tmp_data);
            p++;
            tmp_data++;
        }
        ptrace(PTRACE_POKEDATA, pid, tmp_addr, val);
    }
    printf("[+] ptrace_write:: process(%d) write at %p (size=%zu)\n", pid, addr, size);
}

static void ptrace_get_regs(pid_t pid, struct pt_regs *regs) {
    #if defined(__aarch64__)
        struct {
            void* ufb;
            size_t len;
        } regsvec = { regs, sizeof(struct pt_regs) };

        ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &regsvec);
    #else
        ptrace(PTRACE_GETREGS, pid, NULL, regs);
    #endif
}

static void ptrace_set_regs(pid_t pid, struct pt_regs *regs) {
    #if defined(__aarch64__)
        struct {
            void* ufb;
            size_t len;
        } regsvec = { regs, sizeof(struct pt_regs) };

        ptrace(PTRACE_SETREGSET, pid, NT_PRSTATUS, &regsvec);
    #else
        ptrace(PTRACE_SETREGS, pid, NULL, regs);
    #endif
}


long getmodulebase(pid_t pid, const char* module_name) {
    long base_addr_long = 0;
    if (pid < 0)
        return 0;

    char filename[64];
    char line[512];

    snprintf(filename, 64, "/proc/%d/maps", pid);
    FILE* fp = fopen(filename, "r");
    if (fp) {
        while (fgets(line, 512, fp)) {
            if (strstr(line, module_name)) {
                char* base_addr = strtok(line, "-");
                base_addr_long = strtoul(base_addr, NULL, 16);
                break;
            }
        }
        fclose(fp);
    }
    return base_addr_long;
}

long remote_call(pid_t pid, void *func, size_t argc, ...) {
    #if defined(__aarch64__)
        #define REGS_ARG_NUM        6
    #else
        #define REGS_ARG_NUM        4
    #endif

    char *lib[] = { LIBC_PATH, LINKER_PATH };
    long remote_base, remote_func = 0;
    long offset = 0;
    for (int i = 0; i < sizeof(lib) / sizeof(*lib); i++) {
        long local_base = getmodulebase(getpid(), lib[i]);
        if (!local_base)
            continue;

        if (!offset || (offset > (long)func - local_base && (long)func - local_base > 0)) {
            offset = (long)func - local_base;
            remote_base = getmodulebase(pid, lib[i]);
            remote_func = offset + remote_base;
            
            printf("[*] remote_call:: selected %s offset +%ld\n", lib[i], offset);
        }
    }
    if (!remote_func) {
        printf("[-] remote_call:: remote_func NULL !!\n");
        return 0;
    }

    struct pt_regs regs;
    struct pt_regs backup_regs;
    va_list ap;
    va_start(ap, argc);

    ptrace_get_regs(pid, &regs);
    memcpy(&backup_regs, &regs, sizeof(struct pt_regs));
    for (int i = 0; i < argc && i < REGS_ARG_NUM; i++) {
        regs.uregs[i] = (long)va_arg(ap, long);
    }
    for (int i = REGS_ARG_NUM; i < argc; i++) {
        regs.ARM_sp -= sizeof(long);
        long* data = (long *)va_arg(ap, long *);
        ptrace_write(pid, (uint8_t*)regs.ARM_sp, (uint8_t*)data, sizeof(long));
    }
    va_end(ap);

    regs.ARM_lr = 0;
    regs.ARM_pc = remote_func;
    #if !defined(__aarch64__)
    if (regs.ARM_pc & 1) {
        regs.ARM_pc &= (~1u);
        regs.ARM_cpsr |= CPSR_T_MASK;
    } else {
        regs.ARM_cpsr &= ~CPSR_T_MASK;
    }
    #endif

    ptrace_set_regs(pid, &regs);
    ptrace(PTRACE_CONT, pid, NULL, NULL);

    waitpid(pid, NULL, WUNTRACED);
    ptrace_get_regs(pid, &regs);
    ptrace_set_regs(pid, &backup_regs);
    printf("[+] remote_call:: %p argc=%zu ret=%llx\n", (void *)remote_func, argc, (long long)regs.ARM_r0);
    return regs.ARM_r0;
}