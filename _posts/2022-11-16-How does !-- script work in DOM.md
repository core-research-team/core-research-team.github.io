---
layout: post
title:  'How does "<!--<script>" work in DOM?'
author: "moon"
comments: true
tags: [Research]
---

라온화이트햇 핵심연구2팀 문시우


## 개요

어느날 화이트햇 팀 톡방에 다음과 같은 질문이 올라왔다.

![Untitled](/assets/2023-11-16/Untitled.png)

톡방에 올라온 질문 덕에 상기시키는 겸 해당 특이점에 대해 문서로 정리해보았다.

## 특이점

다음 내용을 넣어 html 확장자를 가진 파일로 저장한다.

```html
<script>
alert('hello<!--<script>');
</script>
```

해당 파일을 브라우저에서 열었을 때 예상되는 결과는 “hello<!--<script>” 메시지가 포함된 alert창이 실행되는 것이지만, 막상 열어보면 아무런 액션이 없는 것을 확인할 수 있다. 

그럼 `<!--<script>` 문자열이 포함된 코드가 실행되지 않을 땐 어떻게 작동하는지 살펴보자.

```html
<script>
function foo(){
	alert("<!--<script>");
}
</script>

<script>
alert(1); // 실행 X
</script>
```

`foo`함수를 호출하지 않았기 때문에, 하단에 위치한 `alert(1)`은 실행될 것으로 예상했지만, 해당 문자열이 포함된 함수 호출 여부와는 관계없이 아무 액션도 없는 것을 확인할 수 있다.

(개발자 도구의 콘솔을 확인해보면 아무런 에러도 발생하지 않는다)

![Untitled](/assets/2023-11-16/Untitled%201.png)

이번엔 파일 내용을 다음과 같이 구성하여 실행해보자.

```
<h1>foo</h1>
<img src onerror=alert(1)>
<script>
	alert(2);
	function a(){
		alert("3<!--<script>");
	}
	alert(4);
	alert(5);
</script>
<img src onerror=alert(6)>!-->
<h1>bar</h1>
```

![Untitled](/assets/2023-11-16/Untitled%202.png)

`<!--<script>` 문자열이 포함된 script태그가 나오기 전까지만 렌더링 및 자바스크립트를 실행해주며 맨 아래의 “bar”는 렌더링 조차 되지 않는다.

그럼 `<!--<script>` 문자열이 나온 이후의 코드가 실행되게 하려면 어떻게 해야할까?
script태그 안에서 `-->`로 닫고서야 다음 코드를 정상적으로 실행시킬 수 있었다.

```jsx
<script>
alert(1);
"<!--<script>"

-->
</script>

<script>
alert(2); // 실행됨
</script>
```

또는

```jsx
<script>
alert(1);
"<!--<script>"
</script>

<script>
-->
</script>

<script>
alert(2); // 실행됨
</script>
```

onload, onerror, onfocus와 같은 `on*` attribute에서도 똑같이 작동하는지 살펴보자.

```jsx
<svg onload='alert("test <!--<script> test")'>
```

![Untitled](/assets/2023-11-16/Untitled%203.png)

script태그 밖에선 영향을 미치지 않는다.

정리해보면

1. script태그 안에 `<!--<script>` 문자열이 있다면 렌더링 및 자바스크립트 실행  X
    
    (’, “, `로 감싸져 있는 상태에서도 작동)
    
2. `<!--<script>`가 삽입된 라인을 지나가지 않더라도 실행 X
3. script태그 안에서 `-->` 가 나와야만 페이지가 정상적으로 작동
4. onload, onerror와 같은 on attribute에서는 영향 X

## 이유

이러한 특이점은 왜 발생할까?

**1. 자바스크립트의 주석 문법**

우선 자바스크립트를 포함한 여타 언어들은 다양한 주석 문법을 갖고있다. 
자바스크립트에서 주로 사용되는 주석으로는 `//`, `/**/` 등이 있으며, HTML에서 주석으로 사용하는 `<!--comment-->` 또한 자바스크립트에서 주석으로 사용할 수 있다.

```jsx
<script>
<!--a--> alert(1); // it doesn't work
alert(2); <!-- executed
alert(3); // executed
</script>
```

위의 코드에서  `<!--`이후에 오는 문자들은 `\n`가 나오기 전까지 문법으로 해석되지 않는다.
* 한 줄 주석으로 활용 가능

**2. 브라우저의 HTML 유효성 검사**

브라우저는 HTML Sanitize라는 다음과 같은 특성을 갖는다.

```jsx
<html><head></head><body><iframe></body></html>
```

파일 내용을 위와 같이 구성한 뒤, 브라우저로 열어보면 페이지에 렌더링되는 내용은 다음과 같다.

![Untitled](/assets/2023-11-16/Untitled%204.png)

브라우저는 똑똑하기 때문에, 개발자가 미처 닫지못한 태그의 유효성을 검사하여 HTML 사양에 따라 닫을 태그는 알아서 닫아주고 렌더링 해준다. 따라서 위 예제에서 `</iframe>` 으로 iframe태그를 닫지 않았음에도, 위와 같이 정상적으로 페이지에 표시된 것을 확인할 수 있다. (이러한 브라우저의 태그 처리 과정을 이용해서 XSS를 발생시킬 수 있는데, 이것을 Mutation XSS 줄여서 mXSS라고 부른다)

아래의 HTML은 브라우저가 어떻게 해석하는지 살펴보자.

```html
<iframe>
<script>
foo = "</iframe><script>//"
alert(1)
</script>
```

**예상하는 결과[1]:**

```
<iframe></iframe>
<script>
foo = "</iframe><script>//"
alert(1)
</script>
```

예상되는 결과 중 첫 번째 결과는 `<script>`가 나오기 전 아직 닫히지 않은 iframe 태그를 닫아준 뒤, script태그 안에 있는 `foo = "<!--</iframe><script>//"` 줄은 정상 코드로 인식하고 아래의 `alert(1)`가 실행되는 것이다.

**예상하는 결과[2]:**

```
<iframe>
<script>
foo = "</iframe><script>//"
alert(1)
</script>
</iframe>
```

예상되는 두 번째 결과는 위와 같이 변수 `foo`에는 아무 내용도 담기지 않고, `alert(1)`도 실행되지 않는 것이다.

**실제 결과:**

![Untitled](/assets/2023-11-16/Untitled%205.png)

![Untitled](/assets/2023-11-16/Untitled%206.png)

`alert(1)`는 정상적으로 실행되며, 변수 `foo`에 아무 값도 담기지 않은 것을 확인할 수 있다.
브라우저는 해당 HTML을 다음과 같이 인식한 것이다.

```
<iframe>
<script>
foo = "</iframe><script>//"
alert(1) // 실행됨
</script>
```

이것은 HTML 해석 표준에 따른 구현이기에, 정상적으로 작동한 게 맞다. (크로미움 계열, 심지어 ie9까지 동일하게 동작함)

그렇다면 `<!--<script>` 문자열은 브라우저에서 어떻게 해석될까?

```jsx
<script>"<!--<script>"</script>
<h1>a</h1>
```

위의 내용을 브라우저로 읽었을 때 DOM은 다음과 같이 구성된다.

![Untitled](/assets/2023-11-16/Untitled%207.png)

좀 더 자세히 보자,

![Untitled](/assets/2023-11-16/Untitled%208.png)

script태그를 닫아주는 `</script>`가 2개 있는것을 확인할 수 있다.
즉, `"<!--<script>"`를 단순 자바스크립트의 문자열로 해석한 것이 아닌, Document 파서에서 실제 열려있는 script태그로 해석하여 `</script>`로 한번 닫아주는 과정이 추가되었다. (`<h1>a</h1>` 또한 출력되지 않는다)

이번엔 임의로 `</script>`를 두번 넣어보았다.

```jsx
<script>a="<!--<script>"</script></script>
<h1>a</h1>
```

![Untitled](/assets/2023-11-16/Untitled%209.png)

페이지에 a가 정상적으로 출력되지만 콘솔을 확인해보면 다음과 같은 에러가 발생한다.

![Untitled](/assets/2023-11-16/Untitled%2010.png)

`</script></script>`에서 앞에 있는 `</script>`를 자바스크립트의 정규식 문법(`/(?:)/`)으로 인식하면서 발생한 에러다. 
즉, `a="<!--<script>"</script>`가 정상적으로 스크립트로 인식되면서 자바스크립트로 실행됐다는 의미다.

우리가 알고있는 사실 중

```jsx
<script>alert(1);
```

이렇게 script태그를 닫아주지 않고 브라우저로 넘겨주면

![Untitled](/assets/2023-11-16/Untitled%2011.png)

`<script>alert(1)</script>`가 정상적으로 완성됨에도 불구하고, 스크립트 실행이 안되는 것을 알고있다. 

이는 브라우저가 문서를 파싱하는 과정에서 script Element의 텍스트는 자바스크립트로 실행될 수 있도록 [HTMLScriptRunner](https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/HTMLScriptRunner.cpp)에서 처리하는데, 위 결과에서 `</script>`는 나중에 HTML 유효성 검사에서 추가된 태그라 HTMLScriptRunner로 넘겨주지 않았기 때문이다.

1. `<!--<script>`로 인해 Document 파서에서는 script태그가 한번 더 열린 것으로 인식 (버그)
2. script태그는 열려있지만 닫는 태그가 없기 때문에 브라우저는 이를 HTMLScriptRunner로 처리하지 않음. (자바스크립트 엔진으로 넘겨주지 않음)
3. 해당 script Element와 하단에 있는 Element들이 정상적으로 작동, 렌더링되지 않음

아래와 같다고 볼 수 있다.

```jsx
<script>alert(1)
<h1>a</h1>
```

![Untitled](/assets/2023-11-16/Untitled%2012.png)

참 아이러니한 상황이다, `<!--<script>`를 왜 script태그로 인식하는지는 아래 소스코드를 분석하면 알 수 있을 것이다.

[https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/HTMLDocumentParser.cpp](https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/HTMLDocumentParser.cpp)

정리해보면 해당 특이점은 브라우저에서 HTML을 파싱하고, 자바스크립트를 실행하고, 주석을 처리하고, DOM을 구성하는 **복잡한 처리 과정에서 발생한 버그**다.

이렇게 얕게나마 `<!--<script>` 특이점에 대해 알아보았다.

이 작동에 대해 좀 더 자세히, 논리적으로 알고싶다면 브라우저의 HTML 파싱, 자바스크립트 파싱 및 실행, 주석 우선순위 등 소스코드를 분석해보며 `<!--<script>`문자열을 브라우저가 어떻게 해석해가는지 파악하면 된다.

HTMLDocumentParser.cpp:[https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/HTMLDocumentParser.cpp](https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/HTMLDocumentParser.cpp)

HTMLScriptRunnder.cpp: [https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/HTMLScriptRunner.cpp](https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/HTMLScriptRunner.cpp)

누군가 위의 내용을 참고하여 분석글을 올려주면 좋을 거 같다 :)

## `<!--<script>` 을 이용한 Quotes escape 예시

```html
<script>
	foo = "{Input1}";
</script>
<img src="{Input2}">
```

Input1: `<!--<script>`

Input2: `</script><script>alert`XSS`</script>`

```html
<script>
	foo = "<!-- <script>";
</script>
<img src="</script><script>alert`XSS`</script>">
```

![Untitled](/assets/2023-11-16/Untitled%2013.png)

## 특이점을 이용해서 풀어볼 수 있는 문제

### 1. Midnight CTF 2020 → Crossintheroof

풀이는 [https://github.com/Parveshdhull/CTF-Writeups/blob/master/2020/Midnight Sun CTF 2020 Quals/Crossintheroof.md](https://github.com/Parveshdhull/CTF-Writeups/blob/master/2020/Midnight%20Sun%20CTF%202020%20Quals/Crossintheroof.md)

### 2. Sunrin CTF → Sunrin XSS Sanitizer

이를 이용해서 Sunrin CTF 문제를 만들어보았다.

[ctf-web-prob/2022/SUNRIN_CTF/BABY_XSS at master · munsiwoo/ctf-web-prob](https://github.com/munsiwoo/ctf-web-prob/blob/master/2022/SUNRIN_CTF/BABY_XSS)

## 결론

알아두면 써먹을 곳이 있을 거 같은 이상한 버그를 알아보았다.

여담으로 `<!--<script>` 말고 `<!--<script/` 도 사용할 수 있다.

## 참고

[1]: [https://github.com/Parveshdhull/CTF-Writeups/blob/master/2020/Midnight Sun CTF 2020 Quals/Crossintheroof.md](https://github.com/Parveshdhull/CTF-Writeups/blob/master/2020/Midnight%20Sun%20CTF%202020%20Quals/Crossintheroof.md)

[2]: [https://security.stackexchange.com/questions/229289/effect-of-script-tag-inside-html-comment](https://security.stackexchange.com/questions/229289/effect-of-script-tag-inside-html-comment)

[3]: [https://stackoverflow.com/questions/26850250/do-browsers-automatically-insert-missing-html-tags](https://stackoverflow.com/questions/26850250/do-browsers-automatically-insert-missing-html-tags)

[4]: [https://www.hahwul.com/2019/07/08/xss-payload-for-escaping-string-in/](https://www.hahwul.com/2019/07/08/xss-payload-for-escaping-string-in/)

[5]: [https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/](https://chromium.googlesource.com/chromium/blink/+/b69618018614278cda72077611adc093f460dc57/Source/core/html/parser/HTMLDocumentParser.cpp)
