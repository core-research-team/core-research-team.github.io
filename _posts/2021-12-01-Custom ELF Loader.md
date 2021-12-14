---
layout: post
title:  "Custom ELF Loader"
author: "KuroNeko"
comments: true
tags: [methodology, reversing, fuzzing]
---

라온화이트햇 핵심연구팀 원요한

# Custom ELF Loader

이 문서는 dlopen시 Executable ELF (no-pie, relro) 인 경우에 open 불가능함을 우회하여 harness를 작성할 수 있도록 custom elf loader를 구현하고 동작원리를 작성하였습니다

## ELF

> ELF(Executable and Linkable Format)는 실행 파일, 목적 파일, 공유 라이브러리 그리고 코어 덤프를 위한 표준 파일 형식이다. 1999년 86open 프로젝트에 의해 x86 기반 유닉스, 유닉스 계열 시스템들의 표준 바이너리 파일 형식으로 선택되었다.
> 

ELF는 인용된 내용대로 리눅스 시스템에서 사용되는 파일 형식입니다. 해당 파일은 아래와 같은 구조를 가집니다.

![Untitled](/assets/2021-12-01/ELF.png)

Program Header는 섹션에 대한 file, va offset 등으로 구성되어있고, Section header는 해당 섹션의 이름명 offset, 섹션의 offset 등으로 구성되어있습니다. ELF은 섹션 별로 Read/Write/Execute 권한을 설정해줄 수 있으며 특정한 섹션(.DYNAMIC)은 loader에서 dynamic linking을 위해 사용됩니다.

아래에서 이러한 특징을 가진 ELF파일을 사용하기 위해서 loader의 동작 순서에 대해서 살펴보겠습니다.

## Loader

Loader는 아래의 단계를 거쳐 ELF 파일을 실행합니다.

1. ELF 파일 파싱 및 검증
2. 섹션 헤더를 통해 각 섹션의 offset, size, dynamic symbol, address 정보 획득
3. 특정 섹션(dynamic, dynstr, dynsym 등)의 내부 정보 획득
4. 각 섹션에 해당하는 정보를 사용한 동작 수행
    1. .DYNAMIC: DYNSTR 섹션에서 사용할 library 문자열 offset 정보 획득
    2. .DYNSTR: dynamic  섹션에서 얻은 library 문자열 offset을 통한 library 경로 획득
    3. .DYNSYM: Imported Function/Object 등에 대한 정보를 획득
5. RVA을 사용하는 mitigation(ASLR, PIE 등) 적용
6. Lazy binding 적용(옵션이 적용되어있을 경우)
7. 시작점 코드 실행

저는 간단히 harness를 만들기 위해 간단하게 loader를 제작할 예정이므로 파싱 및 검증 과정인 1번과 mitigation 적용 옵션인 5번을 제외한 나머지 과정에 대해서 작성하도록 하겠습니다.

### 1. 섹션 헤더를 통해 각 섹션의 offset, size, dynamic symbol, address 정보 획득

```c
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <elf.h>
#include <string.h>

typedef struct SectionInfo {
    size_t addr;
    size_t size;
    size_t offset;
    char *name;
} SectionInfo;

char *LoadELF(char *image, size_t baseaddr, size_t *retSize, char *libpath[], size_t libpathSize) {
		int fd = open(image, O_RDONLY);
		struct stat st;
		stat(image, &st);
		
		char *elf = (char *)mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
		
		Elf64_Ehdr *ehdr = (Elf64_Ehdr *)elf;
		Elf64_Shdr *shdr = (Elf64_Shdr *)(elf + ehdr->e_shoff);
		Elf64_Shdr *sh_strtab = &shdr[ehdr->e_shstrndx];
		char *sh_strtab_p = elf + sh_strtab->sh_offset;
		
		SectionInfo *sections = (SectionInfo *)malloc(sizeof(SectionInfo) * ehdr->e_shnum);
		
		Elf64_Shdr *iter = shdr;
		size_t size = 0;
		for(int i = 0; i < ehdr->e_shnum; i++) {
		    sections[i].addr = iter->sh_addr;
		    sections[i].size = iter->sh_size;
		    sections[i].offset = iter->sh_offset;
		    sections[i].name = sh_strtab_p + iter->sh_name;
		
		    if(size <= iter->sh_addr) {
		        size = iter->sh_addr + iter->sh_size;
		    }
		
		    iter++;
		}
		// ...
}
```

먼저 위와 같이 ELF 파일을 mmap을 통해 메모리 맵핑해준 이후, ELF Header에 가지고 있는 Section Header offset을 사용하여 Section들의 정보를 위와 같이 획득 합니다. Section 개수 또한 Ehdr에서 가지고 있으므로 for문을 통한 접근을 사용하였습니다.

### 2. 특정 섹션(dynamic, dynstr, dynsym 등)의 내부 정보 획득

```c
char *LoadELF(char *image, size_t baseaddr, size_t *retSize, char *libpath[], size_t libpathSize) {
		// ...
		Elf64_Dyn *dynamic = NULL;
    Elf64_Sym *dynSym = NULL;
    char *dynstr = NULL;
    size_t dynNum = 0;
    size_t dynSymNum = 0;
    for(int i = 0; i < ehdr->e_shnum; i++) {
        if(!strcmp(sections[i].name, ".dynamic")) {
            dynamic = (Elf64_Dyn *)(elf + sections[i].offset);
            dynNum = sections[i].size / sizeof(Elf64_Dyn);
        }
        else if(!strcmp(sections[i].name, ".dynstr")) {
            dynstr = elf + sections[i].offset;
        }
        else if(!strcmp(sections[i].name, ".dynsym")) {
            dynSym = (Elf64_Sym *)(elf + sections[i].offset);
            dynSymNum = sections[i].size / sizeof(Elf64_Sym);
        }
    }
		// ...
}
```

앞서 획득한 섹션 정보를 토대로 차례대로 순회하며 동적 링킹에 필요한 섹션(.dynamic, .dynstr, .dynsym)의 주소, 개수등을 저장합니다.

### 3. 각 섹션에 해당하는 정보를 사용한 동작 수행

```c
char *LoadELF(char *image, size_t baseaddr, size_t *retSize, char *libpath[], size_t libpathSize) {
		// ...
		Node *head = NULL;
    Node *tail = NULL;
    size_t nodeSize = 0;

    Elf64_Dyn *dynIter = dynamic;
    for(int i = 0; i < ehdr->e_shnum; i++) {
        if(dynIter->d_tag == DT_NEEDED) {
            Node *node = (Node *)malloc(sizeof(Node));
            node->d_val = dynIter->d_un.d_val;
            node->next = NULL;
            if(head == NULL) {
                head = tail = node;
            } else {
                tail->next = node;
                tail = node;
            }
            nodeSize++;
        }
        dynIter++;
    }

    handles = malloc(sizeof(size_t) * nodeSize);
    handleNum = nodeSize;

    Node *nodeIter = head;
    for(int p = 0; nodeIter != NULL; nodeIter = nodeIter->next, p++) {
        for(int i = 0; i < libpathSize; i++) {
            char *libname = dynstr + nodeIter->d_val;
            char *ptr = malloc(strlen(libpath[i]) + strlen(libname) + 1);
            sprintf(ptr, "%s/%s", libpath[i], libname);
            
            struct stat st;
            if(stat(ptr, &st) != -1) {
                // puts(ptr);
                handles[p] = dlopen(ptr, RTLD_LAZY | RTLD_NODELETE);
                free(ptr);
                break;
            }

            free(ptr);
        }
    }
		// ...
```

![Untitled](/assets/2021-12-01/ELF%201.png)

.dynamic 섹션은 위의 그림과 같이 `NEEDED` 옵션이 설정되어있다면, library를 로드하기 위한 문자열 offset 값을 저장합니다. 따라서, 실행 파일을 로드하기 위해서는 위와 같은 필요한 library 또한 로드해야합니다. 그러므로 해당 옵션에 해당하는 library 문자열을 획득하여 저장하고, dlopen을 사용해 로드시켜줍니다. 그리고 이후에 Lazy binding 작업을 수행하기 위해서 이 library에 대한 handle을 사용하므로 저장합니다.

### 4. Lazy binding 적용(옵션이 적용되어있을 경우)

```c
char **importedFunctions = NULL;
size_t importedFunctionNum = 0;
void **handles = NULL;
size_t handleNum = 0;

char *gotpltAddr = NULL;
size_t *rtld_ro = NULL;

void *dl_fixup(void *ptr, int index) {
    size_t *addr = (size_t *)gotpltAddr;
    for(int i = 0; i < handleNum; i++) {
        size_t func = (size_t)dlsym(handles[i], importedFunctions[index]);
        if(func) {
            gotpltAddr[index + 3] = func;
            return (void *)func;
        }
    }
}

void __attribute__((naked)) LazyBinding() {
    asm volatile(
        "push   rbx\n"
        "mov    rbx,rsp\n"
        "and    rsp,0xffffffffffffffc0\n"
        "sub    rsp, 0x380\n"
        "mov    QWORD PTR [rsp],rax\n"
        "mov    QWORD PTR [rsp+0x8],rcx\n"
        "mov    QWORD PTR [rsp+0x10],rdx\n"
        "mov    QWORD PTR [rsp+0x18],rsi\n"
        "mov    QWORD PTR [rsp+0x20],rdi\n"
        "mov    QWORD PTR [rsp+0x28],r8\n"
        "mov    QWORD PTR [rsp+0x30],r9\n"
        "mov    eax,0xee\n"
        "xor    edx,edx\n"
        "mov    QWORD PTR [rsp+0x250],rdx\n"
        "mov    QWORD PTR [rsp+0x258],rdx\n"
        "mov    QWORD PTR [rsp+0x260],rdx\n"
        "mov    QWORD PTR [rsp+0x268],rdx\n"
        "mov    QWORD PTR [rsp+0x270],rdx\n"
        "mov    QWORD PTR [rsp+0x278],rdx\n"
        "xsavec [rsp+0x40]\n"
        "mov    rsi,QWORD PTR [rbx+0x10]\n"
        "mov    rdi,QWORD PTR [rbx+0x8]\n"
        "call   dl_fixup\n"
        "mov    r11,rax\n"
        "mov    eax,0xee\n"
        "xor    edx,edx\n"
        "xrstor [rsp+0x40]\n"
        "mov    r9,QWORD PTR [rsp+0x30]\n"
        "mov    r8,QWORD PTR [rsp+0x28]\n"
        "mov    rdi,QWORD PTR [rsp+0x20]\n"
        "mov    rsi,QWORD PTR [rsp+0x18]\n"
        "mov    rdx,QWORD PTR [rsp+0x10]\n"
        "mov    rcx,QWORD PTR [rsp+0x8]\n"
        "mov    rax,QWORD PTR [rsp]\n"
        "mov    rsp,rbx\n"
        "mov    rbx,QWORD PTR [rsp]\n"
        "add    rsp,0x18\n"
        "jmp    r11\n"
    );
}

char *LoadELF(char *image, size_t baseaddr, size_t *retSize, char *libpath[], size_t libpathSize) {
		// ...
		Elf64_Sym *dynSymIter = dynSym;
    for(int i = 0; i < dynSymNum; i++) {
        char *imported = dynstr + dynSymIter->st_name;
        if ((dynSymIter->st_info & STT_FUNC) && dynSymIter->st_shndx == 0) {
            importedFunctionNum++;
        }
        dynSymIter++;
    }

    importedFunctions = (char **)malloc(sizeof(size_t) * importedFunctionNum);

    dynSymIter = dynSym;
    for(int i = 0, k = 0; i < dynSymNum; i++) {
        char *imported = dynstr + dynSymIter->st_name;
        if ((dynSymIter->st_info & STT_FUNC) && dynSymIter->st_shndx == 0) {
            importedFunctions[k] = malloc(strlen(imported) + 1);
            strcpy(importedFunctions[k++], imported);
        }
        dynSymIter++;
    }

    char *virtImage = (char *)mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    for(int i = 0; i < ehdr->e_shnum; i++) {
        if(!sections[i].size || !sections[i].addr)
            continue;

        memcpy(virtImage + sections[i].addr - baseaddr, elf + sections[i].offset, sections[i].size);
        if(!strcmp(sections[i].name, ".got.plt")) {
            gotpltAddr = virtImage + sections[i].addr - baseaddr;
            for(size_t k = 0; k < sections[i].size; k += 8) {
                size_t *ptr = (size_t *)(gotpltAddr + k);
                if(*ptr != 0) {
                    *ptr = *ptr + (size_t)virtImage - (size_t)baseaddr;
                }
                else {
                    *ptr = (size_t)LazyBinding;
                }
            }
        }
    }
		// ...
```

### 5. 시작점 코드 실행

elf는 시작점 코드의 offset이 file header에 존재한다. 따라서 해당 주소를 호출해준다면, 정상적으로 프로그램이 실행되는 것을 확인할 수 있다.

```c
#include <stdio.h>
#include "loader.h"

int main(int argc, char *argv[]) {
    char *libPath[] = {
        "/lib/x86_64-linux-gnu/"
    };
    
    size_t size;
    char *image = LoadELF("./target", 0x00400000, &size, libPath, 1);

    char *argv[] = {
        "./target", (char *)Data, NULL
    };

    ((int (*)(int, char*[]))(image + 0x1231))(2, argv);

    finalize(image, size);
    return 0;
}
```

결과물: [https://github.com/Kur0N3k0/elf-loader](https://github.com/Kur0N3k0/elf-loader/)