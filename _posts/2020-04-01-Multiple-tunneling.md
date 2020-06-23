---
layout: post
title:  "Multiple tunneling"
author: "kuroneko"
comments: true
tags: [etc]
---

라온화이트햇 핵심연구팀 원요한([kuroneko9505@gmail.com](mailto:kuroneko9505@gmail.com))

## 1. ssh tunneling

ssh tunneling이란, 특정 포트에 대한 통신을 ssh 서버를 거쳐 원하는 `호스트:주소`로 통신할 수 있게 만들어주는 기능입니다. 보통 사용하는 tunneling은 다음과 같습니다.

- local tunneling

![/assets/98de0849-6162-4319-8a1f-5fdf3d76a594/fb5665cc-9e68-4ada-ad97-e21c11aa41ec.png](/assets/98de0849-6162-4319-8a1f-5fdf3d76a594/fb5665cc-9e68-4ada-ad97-e21c11aa41ec.png)

local tunneling의 경우, `ssh client가 로컬에 특정 포트를 바인딩`하고 해당 포트로 통신을 하게 되면 ssh server를 거쳐 터널링에서 설정한 서버와 통신하게 됩니다.

local tunneling을 위한 옵션은 `-L` 입니다. 해당 옵션의 포맷은 다음과 같습니다.

```bash
-L [local_bind]:[external_server]
```

[Local Tunneling option 예시](https://www.notion.so/05e3f65825164c7599efcefdf5a02e50)

```bash
ssh -N -L 1122:localhost:4444 -p2222 -i nekopkr.pem root@nekop.kr
```

- remote tunneling

![/assets/e89bc9e2-7bb0-427a-81ee-3d804cf6f3e4/ae58d625-cac4-4d82-8475-3da522f17ab6.png](/assets/e89bc9e2-7bb0-427a-81ee-3d804cf6f3e4/ae58d625-cac4-4d82-8475-3da522f17ab6.png)

remote tunneling의 경우, `ssh server가 외부로 특정 포트를 바인딩`하고 Host A에서 Host B의 열린 포트로 접속해 터널링에서 설정한 서버와 통신하게 됩니다.

remote tunneling을 위한 옵션은 `-R` 입니다. 해당 옵션의 포맷은 다음과 같습니다.

```bash
-R [server_bind]:[external_server]
```

[Remote Tunneling option 예시](https://www.notion.so/46ff01f183ec407991b20d4b09ef9c7d)

```bash
ssh -N -R 1122:localhost:4444 -p2222 -i nekopkr.pem root@nekop.kr
```

- dynamic tunneling

local/remote tunnling의 경우는 옵션으로 설정한 `external_server` 로만 통신이 가능하게 됩니다. 하지만 dynamic tunneling을 사용하게되면 로컬에서 바인딩한 포트로 통신할 경우, 원하는 서버와의 통신이 가능하게 됩니다.

![/assets/eeb0424a-1260-424b-adef-5274b28b267c/4b4b3d1c-1019-4118-9818-34b554fb1466.png](/assets/eeb0424a-1260-424b-adef-5274b28b267c/4b4b3d1c-1019-4118-9818-34b554fb1466.png)

dynamic tunneling을 위한 옵션은 `-D` 입니다. 해당 옵션으로는 바인딩할 로컬포트를 입력받고 해당 포트로 통신할 경우, 서버를 거쳐 통신하고자하는 서버와 연결할 수 있게 됩니다.

```bash
ssh -D 1122 -C -q -N -p 2222 -i server.key root@nekop.kr
ssh -D 0.0.0.0:1122 -C -q -N -p 2222 -i server.key root@nekop.kr
```

## 2. 제약적인 환경 우회

내부 방화벽과 같은 제약적인 환경에서 분석을 진행할 때, 외부로 접속을 하기 어려운 경우가 있습니다. ssh 포트가 방화벽에서 허가된 경우, 보다 편한 분석 환경을 위해서 아래와 같이 ssh 로컬 터널링을 이용할 수 있습니다.

```bash
 ssh -N -L 1122:localhost:4444 -p2222 -i nekopkr.pem root@nekop.kr
```

위와 같이 특정 호스트:포트에 접근이 필요할 경우, `[localhost:1122](http://localhost:1122)` 로 바인딩해서 서버를 거쳐 통신할 수 있습니다. 하지만 단일 호스트에 대해서만 처리 가능하게 됩니다. 이는 ssh 동적 터널링으로 해결할 수 있습니다.

```bash
ssh -D 1122 -C -q -N -p 2222 -i server.key root@nekop.kr
```

`-D` 옵션을 통해 로컬에 바인딩할 포트를 인자로 넣고 서버를 통해 통신하도록 할 수 있습니다. 즉, socks 프록시와 동일한 효과를 가져올 수 있었습니다.

## 3. Fiddler socks proxy 적용

Fiddler는 여러 기능들을 제공하는 유용한 웹 프록시 툴입니다. http[s] 패킷을 살펴볼 수 있는 inspector, 특정 rule에 따른 자동 응답 기능 등 많은 기능을 제공합니다. 또한, socks프록시 환경을 제공합니다. 하지만 아래와 같이 ManualProxy 설정에서 socks에 대한 예시가 빠져있는 것을 볼 수 있습니다.

![/assets/36e43ffb-ba66-4a1c-8db7-77283485ef4c/13d62c69-48ef-4a07-a1cf-a4445e0581f7.png](/assets/36e43ffb-ba66-4a1c-8db7-77283485ef4c/13d62c69-48ef-4a07-a1cf-a4445e0581f7.png)

`socks=nekop.kr:1234` 와 같이 대략적으로 게싱해서 넣어봐도 정상적으로 되지 않는 것을 확인하였습니다. 다른 방법을 찾던 와중, Fiddler script에서 session의 `X-OverrideGateway` 라는 옵션이 있는 것을 보게 되었고 해당 옵션에 아래와 같이 설정하여 socks 프록시로 통신하는 것을 볼 수 있었습니다.

![/assets/82a62464-46a3-4e5e-8605-3ee351d2837d/c07cffa6-a385-4c14-aba3-40db352df762.png](/assets/82a62464-46a3-4e5e-8605-3ee351d2837d/c07cffa6-a385-4c14-aba3-40db352df762.png)

따라서, 크롬 브라우저의 프록시서버를 Fiddler 프록시(127.0.0.1:8888)로 설정하고 ssh 동적 터널링 포트로 위와 같이 `X-OverrideGateway` 를 설정해주면 어떤 웹사이트에 접속해도 정상적으로 통신하는 것을 확인할 수 있습니다.

## 4. aws를 활용한 ip ban 우회

aws의 lightsail을 사용하면 instance마다 static-ip를 붙힐 수 있게 됩니다. 따라서 proxy서버로 사용할 instance를 생성한 이후, static-ip를 생성합니다. 그리고 메인 서버에서 proxy서버로 dynamic tunneling을 해줍니다. ip ban이 될 때마다 static-ip를 생성하고 지우는 방법으로 보다 빠른 ip 우회가 가능합니다.

```bash
# fiddler to main-server
ssh -D 1122 -C -q -N -p 2222 -i proxy.key [username]@[main-server]

# main-server:4444 to proxy-server
ssh -D 0.0.0.0:4444 -C -q -N -p 2222 -i proxy.key [username]@[proxy_static-ip]
```

## 5. openvpn을 활용한 ip ban 우회

aws IP대역이 ban되었을 경우, openvpn을 사용해서 ip우회할 수 있습니다. 환경은 `aws`에서 진행하였습니다.

먼저 openvpn클라이언트를 설치합니다.

```bash
apt-get install openvpn
```

이후, `vpngate`([https://www.vpngate.net/en/](https://www.vpngate.net/en/))에서 openvpn 서버 연결 설정을 다운로드 받습니다.

다운로드 받은 ovpn으로 openvpn을 실행하면, 모든 패킷이 vpn서버를 거쳐서 가게 됩니다. 따라서 ssh연결이 먹통이 되는 경우가 생깁니다. 이를 방지하기 위해서 ovpn파일에 `route-nopull` 을 추가합니다. 이 옵션은 모든 패킷이 거쳐가지 않도록 합니다. 위의 옵션을 활성화했을 경우, 아래와 같은 명령어를 입력해서 직접 라우팅을 해야합니다.

```bash
echo "20 pentest_tunnel" >> /etc/iproute2/rt_tables

sysctl -w net.ipv4.ip_forward=1
sysctl -w net.ipv4.conf.all.rp_filter=2
sysctl -w net.ipv4.conf.tun1.rp_filter=2

ip route add 0.0.0.0/0 dev tun0 table 20
ip rule add from all fwmark 20 table 20
ip route flush cache

iptables -A PREROUTING -t mangle -p tcp --dport 80 -j MARK --set-mark 20
iptables -A OUTPUT -t mangle -p tcp --dport 80 -j MARK --set-mark 20
iptables -A PREROUTING -t mangle -p tcp --dport 443 -j MARK --set-mark 20
iptables -A OUTPUT -t mangle -p tcp --dport 443 -j MARK --set-mark 20
iptables -t nat -A POSTROUTING -o tun0 -j MASQUERADE
```

위와 같이 라우팅 테이블 및 포워딩 설정을 하게 되면, 외부로 나가는 80/443 포트만 vpn서버를 거쳐서 가게 됩니다. 따라서 ip ban을 보다 빠르게 우회할 수 있습니다.

위의 라우팅 스크립트는 openvpn 클라이언트를 실행할 때 마다 설정해야 합니다. 하지만, 위의 스크립트를 지속적으로 실행하면 iptables 라우팅이 계속 쌓이게 되므로 두번째 실행부터는 아래의 스크립트를 사용해야 합니다.

```bash
sysctl -w net.ipv4.conf.tun0.rp_filter=2

ip route add 0.0.0.0/0 dev tun0 table 20
ip route flush cache

iptables -t nat -A POSTROUTING -o tun0 -j MASQUERADE
```