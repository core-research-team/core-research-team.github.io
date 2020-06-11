---
layout: post
title:  "WebSocket Based Tunneling Service(with wstunnel)"
author: "gpsfly"
comments: true
tags: [web, network]
---

> 2020.05
라온화이트햇 핵심연구팀 김동민 (dmkim@raoncorp.com)

## 개요

---

 일반적으로 기업내에서 나가는 패킷은 대부분 차단하고 있으며, 80, 443 포트와 같은 웹 서비스 관련 패킷만 허용하는 경우가 대부분입니다. 따라서 ssh 등 외부서버에 붙어서 작업을 해야 되거나, 외부 메신저 사용 등이 불가합니다.

 이를 우회하기 위해 웹 서비스 관련 프로토콜 http(s)을 이용하여 터널링 작업을 수행할 수 있습니다. 그러나 http(s) 프로토콜은 비동기 프로토콜이므로 터널링 서버에서 주기적으로 클라이언트에게 현재 생존여부(?)를 확인해야 합니다. 따라서 터널링 서버로 사용하기에 불필요한 리소스 낭비와 비효율적인 퍼포먼스로 인해 부적합합니다. 이를 개선하기 위해 해당 문서는 웹 소켓(ws)을 통한 터널링 서비스 구축을 소개합니다. 또한 SSL/TLS 를 적용한 wss 서비스도 지원합니다.

해당 내용은 아래와 같은 예외적인 상황일 경우 유용하게 사용될 수 있습니다.

- 80, 443 포트 외 모든 포트를 차단할 경우
- 또한, 80, 443 포트로 나가는(Out-Bound) 패킷을 검사하는 경우(웹 프로토콜 규약 및 헤더 검사)
- ICMP, DNS 등 다른 터널링을 못하는 경우

## wstunnel  구축

---

### Enviroment

- Server : ubuntu 18.04 / Client : Windows 10 x64 or OSX x64
- NodeJS v8.10.0
- Nginx/1.14.0

1. wstunnel 설치

 wstunnel 은 nodejs 패키지 매니저인 npm을 통해 아래와 같이 쉽게 설치할 수 있습니다. 서버와 클라이언트 모두 wstunnel 을 설치합니다.

```bash
npm install wstunnel -g
```

![/assets/6e839579-c42b-43df-bfa6-2fdb1de64931/1ab5e18c-9800-475b-b7e5-71759e9d3114.png](/assets/6e839579-c42b-43df-bfa6-2fdb1de64931/1ab5e18c-9800-475b-b7e5-71759e9d3114.png)

2. 터널링 서버(외부)

다음 nginx 와 같은 웹 서버를 설치해 줍니다.

```bash
apt install nginx
```

 또한, 아래와 같이 nginx의 설정파일(ex. `/etc/nginx/sites-available/default`) location 필드에 해당내용을 추가해 줍니다.

```bash
# ...
	location / {
		proxy_pass http://127.0.0.1:8080;
		proxy_http_version 1.1;
		proxy_set_header Upgrade $http_upgrade;
		proxy_set_header Connection "upgrade";
		proxy_set_header        Host            $host;
		proxy_set_header        X-Real-IP       $remote_addr;
		proxy_set_header        X-Forwarded-For $proxy_add_x_forwarded_for;
		proxy_set_header        X-Forwarded-Proto $scheme;
	}
# ...
```

 아래는 필자의 개인서버 설정파일 (SSL/TLS 적용 포함) 입니다. 추가적으로 SSL/TLS 적용에 관한 내용은 주제와 관련이 없기 때문에 생략합니다. 필요할경우, letsencrypt 및 certbot 관련 내용을 검색하시면 됩니다.

```bash
server {
	listen 80 ;
	listen [::]:80 ;

	root /var/www/html;

	index index.html index.htm index.nginx-debian.html;
  server_name kt.int80.kr; # managed by Certbot

	location / {
		try_files $uri $uri/ =404;

		proxy_pass http://127.0.0.1:8080;
		proxy_http_version 1.1;
		proxy_set_header Upgrade $http_upgrade;
		proxy_set_header Connection "upgrade";
		proxy_set_header        Host            $host;
		proxy_set_header        X-Real-IP       $remote_addr;
		proxy_set_header        X-Forwarded-For $proxy_add_x_forwarded_for;
		proxy_set_header        X-Forwarded-Proto $scheme;
	}

  listen [::]:443 ssl ipv6only=on; # managed by Certbot
  listen 443 ssl; # managed by Certbot
  ssl_certificate /etc/letsencrypt/live/kt.int80.kr/fullchain.pem; # managed by Certbot
  ssl_certificate_key /etc/letsencrypt/live/kt.int80.kr/privkey.pem; # managed by Certbot
  include /etc/letsencrypt/options-ssl-nginx.conf; # managed by Certbot
  ssl_dhparam /etc/letsencrypt/ssl-dhparams.pem; # managed by Certbot
}
```

이제 아래와 같은 명령을 통해 nginx 및 터널링 서비스를 시작할 수 있습니다.

```bash
# Start Web service (nginx)
service nginx start

# Start tunneling service (wstunnel)
wstunnel -s 0.0.0.0:8080
# SSL/TLS를 사용하지만, 인증서 검증을 무시하고 싶을경우 -c 옵션 추가
# wstunnel -s 0.0.0.0:8080 -c
```

3. 터널링 클라이언트

 클라이언트도 마찬가지로 서버와 같이 wstunnel 을 설치합니다. 이후, 아래와 같은 명령을 통해 터널링 작업을 수행합니다.

- wstunnel 설치

```bash
npm install wstunnel -g
```

- 터널링 작업(로컬포트 127.0.0.1:1234와 원격포트 u20.int80.kr:22 바인딩)

```bash
wstunnel -t 127.0.0.1:1234:u20.int80.kr:22 ws://kt.int80.kr

# using SSL/TLS
# wstunnel -t 127.0.0.1:1234:u20.int80.kr:22 wss://kt.int80.kr
```

이후 로컬에 오픈 된 1234에 ssh 로 붙어 테스트 작업을 수행할 수 있습니다.

[Terminal 1]

![/assets/0665527e-a0c4-4865-94a2-b0f56d2e555c/75d0eaa9-6666-4f0d-92ad-1616ca8834ca.png](/assets/0665527e-a0c4-4865-94a2-b0f56d2e555c/75d0eaa9-6666-4f0d-92ad-1616ca8834ca.png)

[Terminal 2]

![/assets/c2d51fc1-7893-4ede-b49c-57f149f69188/9088c33e-fb9e-4c29-bedc-b7c8e3a35b99.png](/assets/c2d51fc1-7893-4ede-b49c-57f149f69188/9088c33e-fb9e-4c29-bedc-b7c8e3a35b99.png)

## 카카오톡 메신저 사용하기

---

 해당 터널링 서비스를 통해 외부 포트를 사용할 수 있지만, 카카오톡과 같은 외부 메신저를 사용하기 위해서는 추가적인 작업이 필요합니다.

 카카오톡의 경우 아래 고급설정을 통해 http 프록시 서버를 설정할 수 있습니다. 해당 기능과 터널링 서비스를 적절히 조합하면 카카오톡 패킷 또한 우회할 수 있습니다.

![/assets/78a13ca6-dec5-41be-94bc-25fd65ff7595/3ff89916-36ed-445d-931d-e6ebcc95e62a.png](/assets/78a13ca6-dec5-41be-94bc-25fd65ff7595/3ff89916-36ed-445d-931d-e6ebcc95e62a.png)

 먼저 외부 터널링 서비스와 함께 프록시 서버가 필요합니다. 프록시 서버는 직접 구축해서 사용해도 되고 공개된 프록시 서버를 사용해도 됩니다. 필자는 시놀로지에서 제공하는 프록시 서버를 사용하였습니다. (프록시 호스트 : nas.int80.kr, 프록시 포트 : 3128)

![/assets/f849a4db-e666-46fd-9d39-0b66dded08fb/4a7472b6-f258-456b-9a6c-8175b2625434.png](/assets/f849a4db-e666-46fd-9d39-0b66dded08fb/4a7472b6-f258-456b-9a6c-8175b2625434.png)

 이후 마찬가지로 wstunnel을 통해 로컬 포트(`1234`)와 외부 호스트(`nas.int80.kr`) 및 포트(`3128`)로 바인딩 해줍니다.

```bash
wstunnel -t 127.0.0.1:1234:nas.int80.kr:3128 wss://kt.int80.kr
```

 마지막으로, 카카오톡의 http 프록시 기능을 이용하여 로그인 할 경우 다음과 같이 정상적으로 작동하는 것을 확인할 수 있습니다.

![/assets/bcaf849d-7b00-42db-9230-a81b1675c8df/6d1bc139-cb33-405d-88bb-6a1eb434f867.png](/assets/bcaf849d-7b00-42db-9230-a81b1675c8df/6d1bc139-cb33-405d-88bb-6a1eb434f867.png)

## Reference

---

- [https://github.com/mhzed/wstunnel](https://github.com/mhzed/wstunnel)
- [https://www.npmjs.com/package/wstunnel](https://www.npmjs.com/package/wstunnel)