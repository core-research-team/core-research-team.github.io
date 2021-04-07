---
layout: post
title: "이해하기 쉬운 웹 보안 모델 이야기 1 (SOP, CORS)"
author: "zairo"
comments: true
tags: [web]
---

라온화이트햇 핵심연구팀 박의성

## 1. 개요

본 문서는 웹 보안 모델인 SOP(Same-Origin Policy)와 CORS(Cross-Origin Resource Sharing)에 대해 서술한 문서입니다.  많은 웹 개발자 분들과 보안 연구원 분들을 귀찮고 힘들게(?) 하는 정책이자, 웹 상에서 취약점으로 인해 정보가 탈취되거나 악의적인 행위가 수행되지 않도록 막아주는 중요한 정책 입니다. 많은 분들께 도움이 되었으면 하는 마음에 최대한 쉽게 이해하실 수 있도록 정리하였습니다.

## 2. 웹 브라우저의 역사

우리가 일상생활에서 웹을 사용할 때, 일반적으로 웹 브라우저라는 프로그램을 사용하여 다양한 웹 서버를 서핑하게 됩니다. 우리가 일상에서 당연하듯이 쓰는 웹 브라우저는 이 글을 쓰는 시점으로부터 약 30년 정도의 역사를 가지고 있습니다. 지금으로부터 대략 30년전으로 거슬러 올라간 1990년에 팀 버너스 리가 HTML와 웹 서버, 웹 브라우저를 최초로 발명하였다고 알려져 있습니다.

1990년대, 웹 브라우저가 처음 만들어지던 초기에는 단순하게 정적인 HTML만을 렌더링하여 사용하였습니다. Netscape 기업의 설립자 마크 로웰 앤드리슨이 웹 브라우저 내에서 이미지, 플러그인 등의 요소를 쉽게 조합할 수 있는 글루 언어(glue language)가 필요하다고 생각하였고 1995년 JavaScript가 도입된 Netscape Navigator 2.0가 출시되었습니다. 브라우저에 JavaScript가 도입된 이후, Netscape Navigator 2.02 버전에서 Same-Origin Policy(이하 SOP)가 등장하였습니다. 

![/assets/2021-04-01/1_Netscape.png](/assets/2021-04-01/1_Netscape.png)

**[웹 브라우저 Netscape Navigator 2.0.2]**

([https://upload.wikimedia.org/wikipedia/commons/thumb/6/69/Netscape_Navigator_2_Screenshot.png/220px-Netscape_Navigator_2_Screenshot.png](https://upload.wikimedia.org/wikipedia/commons/thumb/6/69/Netscape_Navigator_2_Screenshot.png/220px-Netscape_Navigator_2_Screenshot.png))

## 3. DOM(Document Object Model)

웹 브라우저에 JavaScript가 도입된 이후 SOP가 추가된 것을 보면 둘 간의 밀접한 연관이 있을 것이라고 예측 할 수 있습니다. 여기에서 웹 브라우저에서 JavaScript를 이용하여 무엇이 가능하게 되었길래 그 직후에 SOP가 추가 되었을지 의문을 가져볼 수 있습니다.

웹 브라우저에서 JavaScript를 도입한 이유는 웹 브라우저 내에서 이미지, 플러그인 등의 요소를 쉽게 접근하고 조합할 수 있도록 하기 위해서 입니다. 따라서 JavaScript를 이용하여 브라우저 내부의 문서 객체인 Document Object Model(이하 DOM)에 접근할 수 있도록 개발되었습니다. DOM이란 HTML 문서의 객체 기반 표현 방식 입니다. 쉽게 설명드리면 JavaScript를 통해 HTML로 표현된 객체에 접근하여 변경, 제거 등의 작업을 수행할 수 있도록 해주는 것이 DOM 입니다.

예를 들어 JavaScript에서 아래와 같이 form 내부의 input 태그 value 속성 값을 가져오려고 할 때, `document.test_form.test_input.value` 와 같이 접근하는 것이 DOM을 이용한 것이라고 할 수 있습니다.

```html
<html>
	<head>
		<title>test</title>
	</head>
	<body>
		<form name="test_form">
			<input name="test_input" value="test_value">
		</form>
		<script>
			alert(document.test_form.test_input.value);
		</script>
	</body>
</html>
```

**[DOM 예시]** 

![/assets/2021-04-01/2_DOM.png](/assets/2021-04-01/2_DOM.png)

**[DOM 예시 실행화면]**

## 4. AJAX(Asynchronous JavaScript and XML)

지금까지 웹 브라우저 역사와 DOM 까지 알아봤습니다. 이제 브라우저에 JavaScript가 등장한 이후 생겨나게 된 기술인 Asynchronous JavaScript and XML(이하 AJAX)에 대해서 알아보도록 하겠습니다.

페이지가 로드 된 이후 페이지에서 JavaScript를 이용하여 비동기식으로 서버와 상호작용하는 것을 AJAX라고 합니다. AJAX는 페이지 전체를 로딩하지 않고도 비동기식으로 필요한 부분만 동적으로 처리하거나 로딩할 수 있는 장점 때문에 웹 개발자들에게 많은 사랑을 받고 있습니다.

AJAX를 사용한 간단한 예시를 통해 AJAX에 대해 알려드리겠습니다. 아래 예시는 AJAX를 이용하여 test.html의 값을 불러와 로드하는 예제입니다.

```jsx
<h1>It's works!</h1>
```

**[test.html]**

```jsx
<html>
	<head>
		<title>test</title>
	</head>
	<body>
		<div id="result">
		</div>
		<script>
			httpRequest = new XMLHttpRequest();
			httpRequest.onreadystatechange = function() {
			    if(this.status == 200 && this.readyState == this.DONE) {
			        document.getElementById("result").innerHTML = httpRequest.responseText;
			    }
			};
			httpRequest.open('GET', '/test.html');
			httpRequest.send();
		</script>
	</body>
</html>
```

**[index.html]**

![/assets/2021-04-01/3_AJAX.png](/assets/2021-04-01/3_AJAX.png)

**[index.html 실행 화면]**

XMLHttpRequest 객체를 이용하여 JavaScript에서 비동기식으로 이미 로드된 페이지가 아닌 다른 페이지의 응답 값을 동적으로 로드할 수 있는 것을 확인하였습니다.

아마도 여기까지 이해하셨다면 개발시 `service.example.com` 와 `api.example.com` 같이 프론트엔드와 백엔드 기능을 수행하는 API 서버를 별도로 분리 구축해두고 프론트엔드에서 해당 API를 호출해서 사용하면 편리할 것이라고 생각하실 겁니다.

하지만, 이런 상황에서 AJAX를 이용하여 현재 접속한 페이지의 `service.example.com`도메인이 아닌 `api.example.com`와 같은 타 도메인 리소스에 요청하여 응답 값을 가져올 경우, SOP와 CORS 정책에 따라 아래의 에러와 직면하게 될 것입니다.

```jsx
(index):1 Access to XMLHttpRequest at 'http://api.example.com/' from origin 'http://service.example.com' has been blocked by CORS policy: No 'Access-Control-Allow-Origin' header is present on the requested resource.
```

![/assets/2021-04-01/4_Error.png](/assets/2021-04-01/4_Error.png)

실제 개발자분들이 큰 규모의 개발 프로젝트에 투입되어 파트를 나눠 개발하다보면 각각 다른 팀에서 여러 서버를 분담하여 개발하게 되면서 추후 기능을 합칠 때 이 SOP, CORS 에러를 매우 빈번하게 겪게 됩니다. 아마 해당 문제에 직면해서 문제를 해결하기 위해 이 글을 읽고 계시는 분들도 있으실거라 생각됩니다. 

## 5. Same-Origin Policy(SOP)

그럼 이제부터 본격적으로 SOP와 CORS가 무엇인지 알아보도록 하겠습니다.  SOP란 직역하자면 동일 출처 정책이란 뜻으로 동일한 출처의 리소스만 상호작용을 허용하는 정책입니다. 역으로 설명드리면 A 출처에서 온 문서가 B 출처에서 가져온 리소스와 상호작용하는 것을 차단하는 보안 정책입니다. 두 URL의 프로토콜, 호스트, 포트가 모두 같아야 동일한 출처로 인정되며 웹 사이트를 샌드박스화 하여 잠재적인 보안 위협으로부터 보호해주는 정책 입니다.

![/assets/2021-04-01/5_SOP.png](/assets/2021-04-01/5_SOP.png)

개발시 해당 문제를 자주 겪다 보면 귀찮음도 생기고 왜 다른 출처간의 상호작용을 차단하는 건지 의문점을 가질 수 있습니다. 만약 다른 출처간의 상호작용을 차단하지 않는다면 어떤 일이 발생할 수 있는지 확인해보겠습니다.

![/assets/2021-04-01/6_SOP.png](/assets/2021-04-01/6_SOP.png)

SOP가 없는 상황에서 악의적인 JavaScript가 포함되어있는 페이지에 접속하는 상황을 가정하여 설명드리겠습니다. 사용자가 악성페이지에 접속하여 악의적인 JavaScript가 실행되면 사용자가 모르는 사이에 포털사이트인 `potal.example.com`에 임의의 요청을 보내고 그 응답 값을 해커의 서버로 재차 보내 사용자의 소중한 개인정보가 탈취되는 것입니다. 더 나아가서는 JavaScript를 이용하여 사용자가 접속중인 내부망의 아이피와 포트를 스캐닝 하거나, 해커가 사용자 브라우저를 프록시처럼 사용할 수도 있을 것 입니다. 

위의 몇 가지 사례를 통해 SOP가 존재하는 이유에 대해서 간접적으로 알아보았습니다. 따라서 이런 경우가 발생할 가능성을 사전에 방지하기 위해 SOP가 존재한다고 할 수 있습니다.

## 6. Cross-Origin Resource Sharing(CORS)

보안도 중요하지만 개발을 하다 보면 개발 시 기능상 어쩔 수 없이 다른 출처간의 상호작용을 해야 하는 케이스가 존재합니다. 이런 경우를 대비 하기 위해 SOP의 예외 정책으로 Cross-Origin Resource Sharing(이하 CORS)라는 정책을 마련해두었습니다. 따라서 CORS을 이용하면 SOP의 제약을 받지 않게 됩니다. 

먼저 CORS가 어떤 방식으로 동작하는지 확인해보겠습니다.  `service.example.com` 사이트에서는   `api.example.com` 사이트의 리소스를 가져오기 위해 아래와 같은 JavaScript를 구성하였습니다.

```jsx
<html>
	<head>
		<title>test</title>
	</head>
	<body>
		<div id="result">
		</div>
		<script>
			httpRequest = new XMLHttpRequest();
			httpRequest.onreadystatechange = function() {
			    if(this.status == 200 && this.readyState == this.DONE) {
			        document.getElementById("result").innerHTML = httpRequest.responseText;
			    }
			};
			httpRequest.open('GET', 'http://api.example.com/api/v1/test');
			httpRequest.send();
		</script>
	</body>
</html>
```

위와 같이 `service.example.com` 사이트에서 `api.example.com`사이트의 리소스를 요청할 경우에 전송되는 HTTP 패킷에서는 `service.example.com` 사이트의 프로토콜, 호스트, 포트 정보가 Origin 헤더를 통해 전송됩니다.

```html
GET /api/v1/test HTTP/1.1
Host: api.example.com
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/88.0.4324.190 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Language: en-us,en;q=0.5
Accept-Encoding: gzip,deflate
Connection: keep-alive
Origin: https://service.example.com
```

서버는 요청된 Origin 헤더를 보고 어느 사이트를 통해 요청이 전송됐는지 확인할 수 있습니다. 해당 요청에 대한 응답 값은 아래와 같이 전송할 경우, 정상적으로 `api.example.com`사이트에서 데이터를 불러와 로드할 수 있습니다.

```html
HTTP/1.1 200 OK
Date: Mon, 01 Dec 2008 00:23:53 GMT
Server: Apache/2
Access-Control-Allow-Origin: *
Keep-Alive: timeout=2, max=100
Connection: Keep-Alive
Transfer-Encoding: chunked
Content-Type: application/xml

[…Data…]
```

서버는 Access-Control-Allow-Origin 헤더를 통해 응답하여야 합니다. 위 응답 패킷에서의 `Access-Control-Allow-Origin: *` 은 모든 도메인에서 접근을 허용한다는 의미로 만약, 특정 도메인만 허용하고 싶다면 `Access-Control-Allow-Origin: https://service.example.com`로 응답하면 `https://service.example.com`의 요청에 대해서만 리소스에 대한 접근을 허용합니다. 이해하기 쉽게 표현하면 서버에 `Access-Control-Allow-Origin`헤더를 통해 퍼가기 가능 대상 목록을 미리 선언해둔다고 생각하면 될 것 같습니다.

```jsx
(index):1 Access to XMLHttpRequest at 'http://api.example.com/' from origin 'http://service.example.com' has been blocked by CORS policy: No 'Access-Control-Allow-Origin' header is present on the requested resource.
```

![/assets/2021-04-01/4_ERROR.png](/assets/2021-04-01/4_ERROR.png)

따라서 위의 에러와 직면한 분들은 응답을 불러오려고 하는 서버의 응답 헤더에 `Access-Control-Allow-Origin`헤더를 추가하여 문제를 해결하실 수 있습니다.

## 7. Same-Origin Policy를 우회하려다가 발생할 수 있는 취약점

SOP는 동일한 출처가 아닌 다른 도메인의 리소스를 차단하는 정책이므로 간혹 개발 기능에 따라 다른 도메인의 리소스와 상호작용을 해야할 경우가 있습니다. 위에서 알아본 SOP와 CORS 정책에 따라서 정상적으로 로드하기 위해서는 리소스를 가져올 서버에 직접 접근하여 서버측 소스코드에 응답 시 `Access-Control-Allow-Origin` 헤더를 추가하도록 해야 하는 단점이 있습니다.

하지만, 업무상 타 팀에서 개발하거나 운영하고 있는 서버라면 타 팀에 협조를 구해야 하고 해당 서버에 권한이 없는 경우, 프로젝트 마감이 임박한 경우 등에는 현실적으로 CORS를 적용하기 어려운 상황이 됩니다.

이런 경우 마감 기한에 쫓기는 불쌍한(?) 개발자 분들은 어쩔 수 없이 개발 기능 상 타 도메인에서 응답을 받아오기 위해 서버에서 curl이나 HttpURLConnection, urllib, requests 등의 HTTP 모듈을 사용하여 서버에서 직접 응답 값을 가져오도록 구현하게 됩니다. 구현할 때 파라미터에 대한 적절한 검증을 거치지 않는다면 Command Injection 또는 SSRF(Server-side Request Forgery) 취약점이 발생하게 됩니다. 특히 SSRF(Server-side Request Forgery) 취약점이 발생할 가능성이 매우 높습니다.

SSRF(Server-side Request Forgery) 취약점은 공격자가 서버측에서 요청하는 도메인을 변조하여 공격자가 원하는 서버로 요청을 하도록 하여 잘못된 응답 값을 가져가도록 하거나, 요청을 변경 또는 내부망 스캐닝 등을 수행할 수 있습니다.

따라서 여건이 된다면 SOP와 CORS 정책을 최대한 사용하는 것이 좋고 어쩔 수 없이 서버를 통해 요청을 하고 응답 값을 가져와 수행하는 기능 개발시에는 파라미터 검증에 더 주의를 기울여야 할 것으로 생각됩니다.

## 8. 마치며

많은 웹 개발자 분들과 보안 연구원 분들이 SOP(Same-Origin Policy)와 CORS(Cross-Origin Resource Sharing)에 대해 이해하고 보다 안전한 서비스를 만드는 것에 제 글이 조금이나마 도움이 되었으면 좋겠습니다. 다음시간에는 Content Security Policy, 일명 CSP라고 불리는 정책에 대해 알아보겠습니다. 긴 글 읽어주셔서 감사합니다. 

## 9. 참고자료

[https://en.wikipedia.org/wiki/Same-origin_policy](https://en.wikipedia.org/wiki/Same-origin_policy)

[https://developers.google.com/web/fundamentals/security/csp?hl=ko](https://developers.google.com/web/fundamentals/security/csp?hl=ko)

[https://ko.wikipedia.org/wiki/자바스크립트](https://ko.wikipedia.org/wiki/%EC%9E%90%EB%B0%94%EC%8A%A4%ED%81%AC%EB%A6%BD%ED%8A%B8)

[https://ko.wikipedia.org/wiki/웹_브라우저](https://ko.wikipedia.org/wiki/%EC%9B%B9_%EB%B8%8C%EB%9D%BC%EC%9A%B0%EC%A0%80)

[https://ko.wikipedia.org/wiki/HTML](https://ko.wikipedia.org/wiki/HTML)

[https://ko.wikipedia.org/wiki/마크_앤드리슨](https://ko.wikipedia.org/wiki/%EB%A7%88%ED%81%AC_%EC%95%A4%EB%93%9C%EB%A6%AC%EC%8A%A8)

[https://en.wikipedia.org/wiki/Same-origin_policy](https://en.wikipedia.org/wiki/Same-origin_policy)

[https://blog.lgcns.com/1165](https://blog.lgcns.com/1165)

[https://wit.nts-corp.com/2019/02/14/5522](https://wit.nts-corp.com/2019/02/14/5522)

[https://velog.io/@yejinh/CORS-4tk536f0db](https://velog.io/@yejinh/CORS-4tk536f0db)

[https://developer.mozilla.org/ko/docs/Web/HTTP/CORS](https://developer.mozilla.org/ko/docs/Web/HTTP/CORS)

[https://developer.mozilla.org/ko/docs/Web/Guide/AJAX/Getting_Started](https://developer.mozilla.org/ko/docs/Web/Guide/AJAX/Getting_Started)

[https://developer.mozilla.org/ko/docs/Web/Security/Same-origin_policy](https://developer.mozilla.org/ko/docs/Web/Security/Same-origin_policy)

[https://evan-moon.github.io/2020/05/21/about-cors/](https://evan-moon.github.io/2020/05/21/about-cors/)