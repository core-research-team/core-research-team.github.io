---
layout: post
title:  "Padding Oracle Attack"
author: "onestar"
comments: true
tags: [web]
---

라온화이트햇 핵심연구팀 지한별

- Padding Oracle Attack에 대한 취약점 설명과 관련된 문제풀이
- 참고 자료(오라클 패딩 공격에 대한 이해) : [https://laughfool.tistory.com/31](https://laughfool.tistory.com/31)
- 참고 자료(블럭암호에 대한 이해) : [https://blog.1-star.kr/category/Challenge/Crypto](https://blog.1-star.kr/category/Challenge/Crypto)

---

## Padding Oracle Attack 개요

---

- oracle이란 ?

    : oracle사를 의미하는것이 아니고 암호학에서 사용하는 용어이다. 오라클 패딩은 사용자와 시스템의 복호화 또는 암호화를 진행하는 시스템 중 하나를 의미한다.

- 취약점 설명

    : **패딩이 올바른지 올바르지 않은지에 따라 서버의 응답이 달라질 경우** 이를 통해 공격을 수행 할 수 있다.

- padding이란?

    : 블록 암호화해서 사용하는 것으로, 일정 단위로 블록을 자를때 마지막 블록에 앞 블록과 같은 길이로 만들어주기 위해 남는 곳은 패딩으로 채운다.

    ![/assets/Untitled.png](/assets/Untitled.png)

    - 오라클 패딩의 경우, `5byte`가 남았으면 남은 부분을 `0x05`로 `5byte`를 채우고 `2byte`가 남았으면 남은 부분을 `0x02`로 `2byte`를 채운다.
    - 만약 평문이 `8byte`일 경우 다음 블록을 모두 `0x08`로 `8byte` 를 채운다.

![/assets/Untitled1.png](/assets/Untitled1.png)

- 암호화를 수행하는 과정은 위 그림과 같다.
- `iv`와 `plain text`를 `xor`하여 `Intermediary Value`를 얻고 그 값을 3DES 암호화 방식을 거쳐 암호문으로 나온다.
- 이후 그 암호문을 iv로 사용한다.
- 즉 암호문은, **IV+암호블럭1+암호블럭2**로 이뤄진다.

---

## 블럭암호 - CBC 암호

![/assets/Untitled2.png](/assets/Untitled2.png)

- CBC 모드의 블록 암호 과정이다. 평문 블록 1을 IV로 XOR하고 3DES로 암호화 과정을 거쳐 암호문 블럭이 생성된다. `IV+암호문 블럭`을 이어 암호문을 생성한다.

![/assets/Untitled3.png](/assets/Untitled3.png)

- CBC 암호문을 복호화 하는 과정이다.  암호화 복호화 할 경우 첫 블록이 두번째 블록에 영향 준다.

### CBC 암호 특징

- 평문 블록은 반드시 한단계 앞의 암호문 블록과 XOR을 취하고 나서 암호화 됩니다.

때문에, 만약 평문 블록 1과 2가 같더라도 암호 블록 1과 2는 같을 수 없습니다.

- CBC 모드의 암호문 블록이 1개 파손되었다면, 복호화 했을 때에 평문 블록에 미치는 영향은 2블록에 한정됩니다.

출처: [https://blog.1-star.kr/category/Challenge/Crypto](https://blog.1-star.kr/category/Challenge/Crypto)

### Padding에 따른 응답 차이

1. 어플리케이션에서 올바른 암호화 값을 받았을 경우 - 200 OK

2. 어플리케이션에서 패딩이 올바르지 않은 암호문을 받았을 경우 - 500 Internal Server Error

3. 어플리케이션에서 패딩은 올바르나 잘못된 암호문을 받았을 경우 : 암호화된 값을 평문으로 복호화 하였더니 잘못된 값이 있을 경우 - 에러 메시지

## Padding Oracle Attack을 활용한 문제 풀이

---

![/assets/Untitled4.png](/assets/Untitled4.png)

- [wargame.kr](http://wargame.kr) dun woory about the vase 문제이다.
- 문제에서 알려주고 있듯이 padding oracle 취약점 관련된 문제이다.

![/assets/Untitled5.png](/assets/Untitled5.png)

- guest 아이디/ps가 입력되어있다.

![/assets/Untitled6.png](/assets/Untitled6.png)

- 로그인 하면 admin 세션을 얻으라고 적혀있다.

![/assets/Untitled7.png](/assets/Untitled7.png)

- 세션을 확인해보니 L0g1n에 값이 들어있다.

> g9btM63VW4s%3DZDRjqaKyT6A%3D

![/assets/Untitled8.png](/assets/Untitled8.png)

- 파이썬으로 해당 값을 url decoding 해보면 두부분으로 나누어 base64로 인코딩 되어있는 것을 유추할 수 있다.

> 파트 1 : g9btM63VW4s=

> 파트 2 : ZDRjqaKyT6A=

- 두가지 파트로 나누어진것으로 보아 블록 암호인것으로 생각할 수 있고, 블록 암호에서 복호화 된 값 중 맨 앞에있는 블럭은 IV 이다.
- 즉, 해당 문제에서는 `g9btM63VW4s=` 이 IV 이고. `ZDRjqaKyT6A=` 이 암호문 블록1 이다.

### 문제풀이

---

- 서버 응답은 3가지로 나눌 수 있다.

    **1) 로그인 성공** 

    ![/assets/Untitled9.png](/assets/Untitled9.png)
	
    **2) padding error - 패딩이 맞지 않는 경우**

    ![/assets/Untitled10.png](/assets/Untitled10.png)

    **3) invalid user - 패딩은 맞았으나 복호화해서 확인한 plain text가 잘못된 경우**

---

- 위 3가지 응답을 활용해서 패딩이 잘 처리되었는지, 잘못 처리되었는지 파악할 수 있다.

### 복호화 과정

![/assets/Untitled11.png](/assets/Untitled11.png)

- 복호화 과정을 살펴보면 위 그림과 같다. 암호문을 3DES로 복호화 하여 `Intermediary Value` 값을 얻는다.
- 이후 `IV`와 `Intermediary Value`를 XOR하여 plain 값을 얻는다.

![/assets/Untitled12.png](/assets/Untitled12.png)

- `IV`와 `Intermediary Value` 값을 `XOR` 한 plain 블럭의 마지막 값(패딩 부분)이 `0x01`이 되는 `IV` 마지막 값 찾고, `0x01`과 그 `IV` 마지막 값을 `XOR` 하면 `Intermediary`의 마지막 값을 알 수 있다.
- 동일한 방식으로 `0x01` 부터 `0x08`까지 패딩을 가정하여 `IV` 값을 변경하며 서버로 요청을 수행하고, invalid 응답을 경우의 값을 `iv`로 두고 그때의 패딩 값과 `xor` 하여 `intermediary value` 값을 얻을 수 있고 `intermediary value` 값과 이미 알고있는 `iv` 값을 `xor` 하여 `plain` 값을 얻을 수 있다.
- 이후 `plain` 값을 `admin`을 의미하는 값으로 변경하여 `intermediary value` 값과 `xor` 하여 `iv`를 생성하고 그 값과 기존의 암호문과 합쳐서 쿠키에 넣어 전송한다.

## Exploit code

```python
import base64
from urllib.parse import quote, unquote
import binascii
from urllib3.util.retry import Retry
from requests.adapters import HTTPAdapter
from collections import OrderedDict
import requests
from requests.packages.urllib3.exceptions import InsecureRequestWarning
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

# 페이로드 전송
def send_payload(s, payload):
    # variable initialization
	url = ""
	headers = {}
	params = {}
	data = {}

    # URL setting
	scheme = 'http'
	url = '{}://wargame.kr:8080/dun_worry_about_the_vase/main.php'.format(scheme)

    # headers setting
	headers = OrderedDict()
	headers['Connection'] = 'keep-alive'
	headers['Cache-Control'] = 'max-age=0'
	headers['Upgrade-Insecure-Requests'] = '1'
	headers['User-Agent'] = 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.130 Safari/537.36'
	headers['Accept'] = 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9'
	headers['Accept-Encoding'] = 'gzip, deflate'
	headers['Accept-Language'] = 'ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7'
    # L0g1n 쿠키 변조
	headers['Cookie'] = 'L0g1n={};'.format(payload)
	
    # params setting
	params = OrderedDict()

    # data setting
	data = OrderedDict()

    # send packet
	r = s.get(url, headers=headers, params=params, data=data, verify=False)
	return r.text

# xor 함수
# enumberate는 배열을 인덱스와 값 형태로 나눠줌(ex. 1,22)
# 이후 xor 수행한 값을 bytes 형태로 return 해줌
def xor(data, key):
	output = bytearray()
	for i, ch in enumerate(data):
		output.append(ch ^ key[i % len(key)])
	return bytes(output)

# hex로 변환해주는 함수
# temp = data를 hex로 변환 
# 00 00 00 00 00 00 00 00 <- 형태로 보고 쉽게 표현
# range(0, len(temp), 2) <- 0부터 temp길이만큼 2개씩
# ret 에다가 2개씩 잘라서 넣고 return
def hex_view(data):
	temp = data.hex()
	ret = ""
	for i in range(0, len(temp), 2):
		ret += temp[i:i+2] + " "
	return ret

# cookie 생성하는 함수
# (iv base64 인코딩 -> url인코딩) + cookie에서 iv 값 제외한 부분
def make_cookie(iv, enc):
	return quote(base64.b64encode(iv))+quote(base64.b64encode(enc))

# 초기 값 설정
# inter는 암호화 중간 값으로 byte 형태로 비워둠
# s 는 리퀘스트 전송 위해 session 하나 생성 해둠
cookie="ht8Mmi5LRU4%3DajDlXflQ%2By0%3D"
iv=base64.b64decode(unquote("ht8Mmi5LRU4%3D"))
enc=base64.b64decode(unquote("DajDlXflQ%2By0%3D"))
inter=b''
s = requests.Session()

#현재 IV와 ENC 출력
print("I V => {}".format(hex_view(iv)))
print("ENC => {}".format(hex_view(enc)))

#iv 만드는 과정 1~iv길이+1 까지
for i in range(1,len(iv)+1):
	#iv 시작점 지정 / 1일 경우 맨앞에서부터 뒤에 1글자 빼고 / 2일경우 뒤 2글자 빼고
	start = iv[:len(iv)-i]
	for j in range(0,0xff+1):
		# target = start +(0x00~0xff 중 1개) + xor(inter 뒤집은거, i)
		target = start + bytes([j]) + xor(inter[::-1], bytes([i]))
		cookie = make_cookie(target, enc)
		res = send_payload(s, cookie)
		print(hex_view(target), "=>", cookie)
		print(res)
		if 'padding error' not in res:
			break
	# padding error가 안뜨면 정상이므로 구한 값 j와 현재 패딩 값(1~8중 하나) xor 해서 inter을 파악함
	inter += bytes([i ^ j])
	
	# inter는 뒤부터 구하는 것 이기 때문에 뒤집어서 다시 구해줌
	print(hex_view(inter[::-1]))

# 다 구해진 인터 뒤집어서 리얼 인터로 만듬
# plain 알아냄 inter와 iv xor
inter = inter[::-1]
plain = xor(inter, iv)
print(plain)

# 공격에 쓰일 plain을 적고
# plain의 iv를 구하고(구한 inver와 mod xor하기)
mod = b"admin\x03\x03\x03" 
print(make_cookie(mod_iv, enc))
```