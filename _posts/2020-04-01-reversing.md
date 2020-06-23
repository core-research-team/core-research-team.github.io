---
layout: post
title:  "리버싱과 연립일차방정식"
author: "Qbeom"
comments: true
tags: [etc]
---

라온화이트햇 핵심연구팀 최규범([kbchoi@raoncorp.com](mailto:kbchoi@raoncorp.com))

---

- **목차**

    ---

    ---

### 개요

종종 리버싱 문제를 풀 때, 역함수를 만들 수 없고 Z3 Solver 또한 CPU를 잠식한 채 결과를 보여주지 못하는 경우가 있습니다.

그 중 하나인 Dreamhack 워게임의 'multipoint' 문제를 풀면서, 연립일차방정식을 통한 리버싱 문제 풀이 방법론에 대해 정리하고자 합니다.

### 바이너리 분석

```c
int64 __fastcall main(int64 a1, char **a2, char **a3)
{
  int64 result; // rax
  int i; // [rsp+8h] [rbp-218h]
  char buf[256]; // [rsp+10h] [rbp-210h]
  char res[264]; // [rsp+110h] [rbp-110h]
  unsigned int64 v7; // [rsp+218h] [rbp-8h]

  v7 = __readfsqword(0x28u);
  memset(buf, 0, sizeof(buf));
  memset(res, 0, 0x100uLL);
  write(1, "Input : ", 8uLL);
  if ( read(0, buf, 0x100uLL) == 39 )
  {
    for ( i = 0; *(&x0 + i); ++i )
      res[*(&x0 + 4 * i)] = (tb[*(&x2 + 4 * i)] * buf[*(&x1 + 4 * i)] * x3[4 * i]
                            + res[*(&x0 + 4 * i)]) % 251;
    if ( memcmp(res, &goal, 38LL) )
      write(1, "Wrong\n", 6uLL);
    else
      write(1, "Correct\n", 8uLL);
    result = 0LL;
  }
  else
  {
    write(1, "Wrong\n", 6uLL);
    result = 0LL;
  }
  return result;
}
```

multipoint 바이너리의 main 함수입니다.

사용자 입력 값을 38 바이트만큼 받은 후, 반복문에서 특정 연산을 수행합니다.

연산을 수행하여 나온 값 38바이트와 특정 값이 일치해야합니다.

```c
.data:0000000000201020 x0       db  13h                 ; DATA XREF: main+C5↑o
.data:0000000000201020                                         ; main+197↑o ...
.data:0000000000201021 x1       db  20h                 ; DATA XREF: main+F0↑o
.data:0000000000201022 x2       db  21h ; !             ; DATA XREF: main+11B↑o
.data:0000000000201023 x3       db 0DFh, 10h, 1Bh, 11h, 7Ch, 0, 14h, 1Ah, 3Fh, 9, 24h
.data:0000000000201023          db 0Ah, 0F6h, 16h, 11h, 23h, 24h, 3, 0Dh, 1Dh, 0C9h, 24h
.data:0000000000201023          db 22h, 1Fh, 7Ah, 1Fh, 5, 22h, 0B0h, 18h, 9, 1Eh, 54h
.data:0000000000201023          db 23h, 21h, 0Eh, 0CDh, 1, 25h, 12h, 0E4h, 1Fh, 23h, 25h
.data:0000000000201023          db 9Dh, 0, 0Ah, 25h, 8Ah, 4, 21h, 1Dh, 0B7h, 25h, 11h
.data:0000000000201023          db 24h, 0A1h, 14h, 18h, 16h, 0F5h, 0Bh, 24h, 1Dh, 0AFh
.data:0000000000201023          db 9, 1Dh, 17h, 0E1h, 21h, 1Ch, 25h, 41h, 23h, 20h, 22h
```

연산에서 참조하는 특정 값의 이름을 임의로 지정하였습니다.

```c
.data:0000000000201020 x0       db  13h                 ; DATA XREF: main+C5↑o
.data:0000000000201020                                         ; main+197↑o ...
.data:0000000000201021 x1       db  **20h**                 ; DATA XREF: main+F0↑o
.data:0000000000201022 x2       db  21h ; !             ; DATA XREF: main+11B↑o
.data:0000000000201023 x3       db 0DFh, 10h, **1Bh**, 11h, 7Ch, 0, **14h**, 1Ah, 3Fh, 9, **24h**
.data:0000000000201023          db 0Ah, 0F6h, 16h, **11h**, 23h, 24h, 3, 0Dh, 1Dh, 0C9h, 24h
.data:0000000000201023          db 22h, 1Fh, 7Ah, 1Fh, 5, 22h, 0B0h, 18h, 9, 1Eh, 54h
.data:0000000000201023          db 23h, 21h, 0Eh, 0CDh, 1, 25h, 12h, 0E4h, 1Fh, 23h, 25h
.data:0000000000201023          db 9Dh, 0, 0Ah, 25h, 8Ah, 4, 21h, 1Dh, 0B7h, 25h, 11h
.data:0000000000201023          db 24h, 0A1h, 14h, 18h, 16h, 0F5h, 0Bh, 24h, 1Dh, 0AFh
.data:0000000000201023          db 9, 1Dh, 17h, 0E1h, 21h, 1Ch, 25h, 41h, 23h, 20h, 22h
```

각각의 참조 값은 1바이트 값으로, 4바이트 씩 건너가며 참조합니다.

![/assets/8279eef4-9e48-453a-82cd-27a9f6f0944d/8ad51cc3-7c1e-4ecf-8351-79d5855f7dc4.png](/assets/8279eef4-9e48-453a-82cd-27a9f6f0944d/8ad51cc3-7c1e-4ecf-8351-79d5855f7dc4.png)

```c
for ( i = 0; A[i]; ++i )
      res[A[i].idx] = (tb[A[i].x2] * buf[A[i].idx_buf] * A[i].x3 + res[A[i].idx]) % 251;
```

구조체로 만들면 이전보다 깔끔하게 볼 수 있습니다.

A[i].idx, A[i].idx_buf는 당연히 0~37 범위만큼의 값만 존재합니다.

```python
f = open("multipoint", "rb")
data = f.read()
f.close()

data = data[0x1020:]

x0, x1, x2, x3 = [], [], [], []
pos = 0
while data[pos:pos+4] != b"\x00\x00\x00\x00":
    x0.append(data[pos + 0])
    x1.append(data[pos + 1])
    x2.append(data[pos + 2])
    x3.append(data[pos + 3])
    pos += 4
```

위와 같은 코드를 통해 x0 ~ x3까지 값을 파싱할 수 있습니다.

### 식 세우기

```c
for ( i = 0; A[i]; ++i )
      res[A[i].idx] = (tb[A[i].x2] * buf[A[i].idx_buf] * A[i].x3 + res[A[i].idx]) % 251;
```

위 반복문의 코드가 해당 문제의 핵심이며 전부입니다.

$$X_{A_{i0}} = (T_{A_{i2}} \times B_{A_{i1}} \times A_{i3} + X_{A_{i0}})\,\bmod\,251$$

해당 코드를 1:1로 대응하여 식으로 나타내면 위와 같습니다. *(X = res, T = tb, B = buf)*

모듈러 251 연산 내에서 덧셈, 곱셈에 닫혀있습니다.

$$X_{0} = (T_{a} \times B_{b} \times A_{c}\\\quad\quad+ T_{d} \times B_{e} \times A_{f}\\\quad\quad...\\\quad\quad\quad\quad\quad\quad\;\;+ T_{g} \times B_{h} \times A_{i})\,\bmod\,251\\　\Downarrow 　\\X_{0} = P_{0} \times B_{0} + P_{1} \times B_{1} + ... + P_{37} \times B_{37} \pmod{251}$$

예를들면 위처럼 X0에 대한 식으로 정리할 수 있습니다. *(P = T * A)*

P는 이미 주어진 값을 통해 계산할 수 있습니다. 

$$X_{0} = P_{(0,0)} \times B_{0} + P_{(0,1)} \times B_{1} + ... + P_{(0,37)} \times B_{37}\pmod{251}\\X_{1} = P_{(1,0)} \times B_{0} + P_{(1,1)} \times B_{1} + ... + P_{(1,37)} \times B_{37}\pmod{251}\\X_{2} = P_{(2,0)} \times B_{0} + P_{(2,1)} \times B_{1} + ... + P_{(2,37)} \times B_{37}\pmod{251}\\...\\X_{37} = P_{(37,0)} \times B_{0} + P_{(37,1)} \times B_{1} + ... + P_{(37,37)} \times B_{37}\pmod{251}$$

결국 X0 ~ X37은 B에 대한 식으로 나타낼 수 있고, 미지수 38개와 38개의 식이 나옵니다.

### 풀이 시도 - Z3

```python
from z3 import *

f = open("multipoint", "rb")
data = f.read()
f.close()

data = data[0x1020:]

x0, x1, x2, x3 = [], [], [], []
pos = 0
while data[pos:pos+4] != b"\x00\x00\x00\x00":
    x0.append(data[pos + 0])
    x1.append(data[pos + 1])
    x2.append(data[pos + 2])
    x3.append(data[pos + 3])
    pos += 4

tb = [0xEF, 0x61, 0x4A, 0x6C, 0xD6, 0xC4, 0xC2, 0x65, 0xD2, 0x1C, 0xC4, 0xD2, 0x0B, 0x9B,
0x60, 0x4C, 0xEA, 0xDA, 0xD8, 0xB8, 0x2F, 0xAD, 0xBA, 0x19, 0x82, 0xED, 0xF4, 0xB4, 0xD8,
0xD9, 0xA2, 0x95, 0xEA, 0x5B, 0x89, 0xBC, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

goal = [ 0x8D, 0x13, 0xAF, 0xEB, 0x43, 0xDD, 0x88, 0xF5, 0xF2, 0xA3, 0xDC, 0xCC, 0x0D,
0x3C, 0x4C, 0xEB, 0x52, 0xC8, 0x99, 0x7F, 0x84, 0xF4, 0x54, 0x39, 0x4D, 0xC9, 0xE8, 0x18,
0x7A, 0x94, 0xE0, 0x9E, 0x50, 0xA2, 0xC5, 0xC1, 0xAA, 0xB1]

s = Solver()
buf = [ BitVec('x%i' % i, 32) for i in range(38)]

res = []
fom = []
for i in range(38):
    res.append(0)

for i in range(38):
    fom.append([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

cnt = 0
for i in range(len(x0)):
    idx = x0[i]
    idx_buf = x1[i]
    fom[idx][idx_buf] += tb[x2[i]] * x3[i]
    # P = T_x2 * A_x3

for i in range(38):
    for j in range(38):
        fom[i][j] %= 251

for i in range(38):
    for j in range(38):
        res[i] += buf[j] * fom[i][j]
        res[i] %= 251

for i in range(38):
    s.add(res[i] == goal[i])
    s.add(And(buf[i] < 0x80, buf[i] > 0x20))
    
print(s.check())
```

z3를 통해 위의 식을 넣고 풀이 시도합니다. *z3, Python 모두 사용이 미흡하여 코드가 비효율적이다.*

약 1시간을 돌려보아도 답이 나오질 않았습니다.

그 시간동안 해결할 수 없다는 뜻은, z3가 아닌 다른 방법으로 풀어야한다는 뜻이라 생각했습니다.

### Z3의 한계

*'왜 Z3로 해결할 수 없는 것인가?'*

문제를 해결한 이후 이를 납득하기 위해서 간단히 테스트해보았습니다.

```python
from z3 import *
import random
import time

def make_lf(size):
    x = []
    for i in range(size):
        r = random.randint(0, 255)
        x.append(r)
    f = []
    res = []
    for i in range(size):
        tmp = []
        ss = 0
        for j in range(size):
            r = random.randint(0, 255)
            tmp.append(r)

            r *= x[j]
            ss += r

        f.append(tmp)
        res.append(ss)

    return x, f, res

for size in range(4, 37):
    ts = []
    for cnt_run in range(11):
        _, f, res = make_lf(size)
        st = time.time()
        s = Solver()
        x = [ BitVec('x%i' % i, 32) for i in range(size)]

        for i in range(size):
            s.add(And(x[i] >= 0, x[i] < 256))

        for i in range(size):
            tmp = 0
            for j in range(size):
                tmp += (x[j] * f[i][j])

            s.add(res[i] == tmp)

        s.check()
        ts.append(time.time() - st)

    ts.sort()
    print(size, ts[5])
```

N개의 미지수에 대한 N개의 식을 임의 생성(*make_lf 함수*) 후,
Z3로 미지수를 구하는 데 까지 걸리는 시간을 계산하였습니다.

```python
 4 : 0.11500000953674316
 5 : 0.17499995231628418
 6 : 0.3470001220703125
 7 : 0.4549999237060547
 8 : 0.870999813079834
 9 : 1.2239999771118164
10 : 1.4860000610351562
11 : 1.810999870300293
12 : 2.1440000534057617
13 : 2.625
14 : 3.937999963760376
15 : 4.559000015258789
16 : 6.763000011444092
17 : 7.7170000076293945
18 : 15.555999994277954
19 : 23.84500002861023
20 : 56.62400007247925
21 : 80.25499987602234
22 : 50.270999908447266
23 : 161.06699991226196
-------------------------
*결과가 나올 때 까지의 시간이 매번 상이하므로,
N값이 작은 범위(N≤23)에서는 여러 번 계산 후 중간 값을 출력해주었습니다.*
-------------------------
24 : 61.769999980926514
25 : 201.04399991035461
26 : 967.4270000457764
27 : 15411.316999912262
28 : 1857.494999885559
29 : 1452.619000196457
30 : 9330.004999876022
31 : 2545.452000141144
32 : 48023.111999988556
33 : 9613.564999818802
34 : 168636.5529999733
```

***테스트 환경 : i7-6700K(4.00GHz), 1스레드***

결과를 보면 미지수가 늘어날수록 시간이 크게 증가하는 것을 볼 수 있습니다.

34개의 미지수와 식이 있을 경우, 약 2일 정도의 시간이 소요됨을 알 수 있었습니다.

현재 문제에서는 38개의 미지수를 사용하며, FLAG 규칙에서 4개 글자를 알 수 있습니다.

*'미지수가 34개, 약 2일 정도 지나면 나오지 않을까..?'*

 

![/assets/3f446bcf-f775-43b2-8fa1-b3603c501e16/4fe319db-6ae3-44e0-b93b-34a119e3d744.jpg](/assets/3f446bcf-f775-43b2-8fa1-b3603c501e16/4fe319db-6ae3-44e0-b93b-34a119e3d744.jpg)

해당 문제에서는 모듈러를 사용합니다.
모듈러가 Z3의 연산 속도에 어떠한 영향을 미치는지 볼 필요가 있습니다.

```python
from z3 import *
import random
import time

def make_lf_mod(size, mod):
    x = []
    for i in range(size):
        r = random.randint(0, 255) % mod
        x.append(r)

    f = []
    res = []
    for i in range(size):
        tmp = []
        ss = 0
        for j in range(size):
            r = random.randint(0, 255) % mod
            tmp.append(r)
            r *= x[j]
            ss += r
            ss %= mod

        f.append(tmp)
        res.append(ss)
    return x, f, res

mod = 231
for size in range(4, 37):
    ts = []
    for cnt_run in range(1):
        _, f, res = make_lf_mod(size, mod)

        st = time.time()

        s = Solver()
        x = [ BitVec('x%i' % i, 32) for i in range(size)]

        for i in range(size):
            s.add(And(x[i] >= 0, x[i] < 256))

        for i in range(size):
            tmp = 0
            for j in range(size):
                tmp += (x[j] * f[i][j])

            tmp %= mod
            s.add(res[i] == tmp)

        s.check()
        ts.append(time.time() - st)

        if cnt_run == 0:
            print(_)
            print(s.model())

    ts.sort()
    print(size, ts[0])
```

이전 테스트 코드에 모듈러를 추가하여 테스트를 해보았습니다.

```python
 4 : 0.11500000953674316
				    ↓
 4 : 289.9964904785156
```

***테스트 환경 : i7-6700K(4.00GHz), 1스레드***

시작부터 속도가 정말 느려진 것을 확인할 수 있습니다. 

미지수를 34개로 줄였다고 해도, 절대 2일내로는 해결될 것 처럼 보이지 않습니다.

문제 해결 시간이 중요한 CTF에서는 더 더욱 사용하기 어려워 보입니다.

### 풀이 시도 - 연립일차방정식

38개 미지수에 대한 38개 [연립일차방정식](https://ko.wikipedia.org/wiki/%EC%97%B0%EB%A6%BD_%EC%9D%BC%EC%B0%A8_%EB%B0%A9%EC%A0%95%EC%8B%9D)은 [가우소 소거법](https://ko.wikipedia.org/wiki/%EA%B0%80%EC%9A%B0%EC%8A%A4_%EC%86%8C%EA%B1%B0%EB%B2%95)을 통해 해결할 수 있습니다.

*중등 수학 이후로 수학에 손을 대지 않은 수준에서도 이해할만큼, 조금만 찾아봐도 어렵지 않다.*

$$\begin{pmatrix} 239 & 117 & 113 & 160 & ... & 9 \\ 97 & 108 & 72 & 72 & ... & 110 \\ 74 & 80 & 83 & 141 & ... & 106 \\ ... \\ 1 & 31 & 55 & 68 & ... & 131 \end{pmatrix} \times \begin{pmatrix} B_{0} \\ B_{1} \\ B_{2} \\ ... \\ B_{37}  \end{pmatrix} = \begin{pmatrix} 141 \\ 19 \\ 175 \\ ... \\ 177 \end{pmatrix} \pmod{251}$$

연립일차방정식을 모듈러 251인 상태에서 위와 같은 행렬로 나타낼 수 있습니다.

![/assets/ee482697-1d1f-4a8f-9f71-23338c93d733/d3c35d00-67de-47b3-b7cc-1c2f7db40216.png](/assets/ee482697-1d1f-4a8f-9f71-23338c93d733/d3c35d00-67de-47b3-b7cc-1c2f7db40216.png)

**1번 식**을 이용하여, 2번 이후 식의 **첫 번째 미지수**를 없애줍니다.

예를들어 2번 식을 바꾸려면, 1번식에 239를 나눈 후 97을 곱해줍니다.

첫 번째 미지수가 97인 식이 되었고, 그 식을 2번식에서 빼는 방식입니다.

이를 반복하여 마지막 미지수를 구할 수 있고, 후방 대입을 통해 다른 미지수를 구할 수 있게 됩니다.

하지만 문제는 모듈러 251 연산이므로 위의 방법 그대로를 사용할 수 없습니다.

이런 경우에서는 [모듈러 곱셈 역원](https://en.wikipedia.org/wiki/Modular_multiplicative_inverse)을 통해 가우스 소거법을 사용할 수 있습니다.

$$97 \times x \equiv 1\pmod{251}\\　\Downarrow 　\\97 \times x \times 239 \equiv 239\pmod{251}$$

2번 식의 97과 곱하면 1이 되는 수를 찾은 후, 그 수와 239를 2번식에 곱합니다.

그렇게 연산된 식에서 1번식을 빼주면 첫 번째 미지수의 계수가 0이 됩니다.

$$\begin{pmatrix} 239 & 117 & 113 & 160 & ... & 9 \\ 0 & 87 & 23 & 249 & ... & 143 \\ 0 & 0 & 95 & 22 & ... & 126 \\ ... \\ 0 & 0 & 0 & 0 & ... & 225 \end{pmatrix} \times \begin{pmatrix} B_{0} \\ B_{1} \\ B_{2} \\ ... \\ B_{37}  \end{pmatrix} = \begin{pmatrix} 141 \\ 118 \\ 216 \\ ... \\ 13 \end{pmatrix} \pmod{251}$$

모듈러 곱셈 역원과 가우스 소거법을 통해, 위 처럼 정리할 수 있습니다.

$$225 \times B_{37} \equiv 13\pmod{251}\\ (225 \times x \equiv 1 \Rightarrow x = 222)\\ 222 \times 225 \times B_{37} \equiv 222 \times13\pmod{251}\\　\\B_{37} = 125$$

마지막 식을 통해, 마지막 입력 값( '*}*' )을 구했으며, 이는 FLAG의 마지막 글자 규격과도 일치합니다.

이제 후방 대입을 통해 모든 FLAG를 획득하여 문제를 풀 수 있습니다.

### 풀이 소스 코드

```python
def egcd(a, b):
    if a == 0:
        return (b, 0, 1)
    else:
        g, y, x = egcd(b % a, a)
        return (g, x - (b // a) * y, y)

def modinv(a, m):
    g, x, y = egcd(a, m)
    if g != 1:
        print(a,m)
        raise Exception('modular inverse does not exist')
    else:
        return x % m

f = open("multipoint", "rb")
data = f.read()
f.close()

data = data[0x1020:]

x0, x1, x2, x3 = [], [], [], []
pos = 0
while data[pos:pos+4] != b"\x00\x00\x00\x00":
    x0.append(data[pos + 0])
    x1.append(data[pos + 1])
    x2.append(data[pos + 2])
    x3.append(data[pos + 3])
    pos += 4

tb = [0xEF, 0x61, 0x4A, 0x6C, 0xD6, 0xC4, 0xC2, 0x65, 0xD2, 0x1C, 0xC4, 0xD2, 0x0B, 0x9B,
0x60, 0x4C, 0xEA, 0xDA, 0xD8, 0xB8, 0x2F, 0xAD, 0xBA, 0x19, 0x82, 0xED, 0xF4, 0xB4, 0xD8,
0xD9, 0xA2, 0x95, 0xEA, 0x5B, 0x89, 0xBC, 0x66, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]

goal = [ 0x8D, 0x13, 0xAF, 0xEB, 0x43, 0xDD, 0x88, 0xF5, 0xF2, 0xA3, 0xDC, 0xCC, 0x0D,
0x3C, 0x4C, 0xEB, 0x52, 0xC8, 0x99, 0x7F, 0x84, 0xF4, 0x54, 0x39, 0x4D, 0xC9, 0xE8, 0x18,
0x7A, 0x94, 0xE0, 0x9E, 0x50, 0xA2, 0xC5, 0xC1, 0xAA, 0xB1]

buf = []
res = []
fom = []

SIZE_INPUT = 38
MOD = 251
for i in range(SIZE_INPUT):
    res.append(0)
    buf.append(0)

for i in range(SIZE_INPUT):
    fom.append([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

for i in range(len(x0)):
    idx = x0[i]
    idx_buf = x1[i]
    fom[idx][idx_buf] += tb[x2[i]] * x3[i]
"""
    res[*(&x0 + 4 * i)] = (res[*(&x0 + 4 * i)] + tb[*(&x2 + 4 * i)] * x3[4 * i]  * buf[*(&x1 + 4 * i)]) % 251;
"""

for i in range(SIZE_INPUT):
    for j in range(SIZE_INPUT):
        fom[i][j] %= MOD

#GE with modulo
for k in range(SIZE_INPUT-1):
    for i in range(k+1, SIZE_INPUT):
        s = fom[k][k]#239
        t = fom[i][k]#97
        if t == 0:
            continue
        m = modinv(t, MOD)#44 * 97 % 251 => 1 => 1 * 239
        x = (m * s) % MOD
        for j in range(SIZE_INPUT):
            fom[i][j] = (fom[i][j] * x) % MOD
        goal[i] = (goal[i] * x) % MOD

        for j in range(SIZE_INPUT):
            fom[i][j] = (fom[i][j] - fom[k][j]) % MOD
        goal[i] = (goal[i] - goal[k]) % MOD

#Solve GE
for k in range(SIZE_INPUT):
    idx = SIZE_INPUT - k - 1
    t = fom[idx][idx]
    m = modinv(t, MOD)
    for i in range(idx+1, SIZE_INPUT):
        goal[idx] = (goal[idx] - (buf[i] * fom[idx][i])) % MOD
    
    g = goal[idx]
    x = m * g % MOD
    buf[idx] = x
    
flag = ''
for x in buf:
    flag += chr(x)

print(flag)
```