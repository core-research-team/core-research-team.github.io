#include <stdbool.h>
#include <stdio.h>

#ifndef UTILS_H_
#define UTILS_H_
#define CPSR_T_MASK (1u << 5)


#if defined(__aarch64__)
    #define LIBC_PATH      "/system/lib64/libc.so"
    #define LINKER_PATH    "/system/bin/linker64"
    #define LIBDL_PATH    "/system/lib64/libdl.so"
    #define VNDK_LIB_PATH  "/system/lib64/libRS.so"
#else
    #define LIBC_PATH      "/system/lib/libc.so"
    #define LINKER_PATH    "/system/bin/linker64"
    #define LIBDL_PATH    "/system/lib/libdl.so"
    #define VNDK_LIB_PATH  "/system/lib/libRS.so"
#endif

int ptrace_attach(pid_t pid);
int ptrace_detach(pid_t pid);
void ptrace_write(pid_t pid, uint8_t* addr, uint8_t* data, size_t size);
long getmodulebase(pid_t pid, const char* module_name);
long getremotefunc(pid_t remote_pid, const char* module_name, long local_func_name);
long remote_call(pid_t pid, void *func, size_t argc, ...);

#endif