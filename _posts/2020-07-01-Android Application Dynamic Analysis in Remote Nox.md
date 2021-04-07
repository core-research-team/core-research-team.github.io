---
layout: post
title: "Android Application Dynamic Analysis in Remote NoxPlayer"
author: "jeong.su"
comments: true
tags: [android, frida]
---

라온화이트햇 핵심연구팀 최정수

# NoxPlayer?

Nox App Player는 PC에서 모바일 게임을 비롯한 모바일 Application들을 실행할 수 있게 하는 Emulator 입니다.

# Android App Dynamic Analysis with NoxPlayer

Android Application을 동적으로 분석하고자 할 때 Android OS의 Device가 없는 경우 NoxPlayer를 사용하여 분석 할 수 있습니다. ADB, Frida, gdb를 모두 사용할 수 있습니다. 각각의 사용 방법은 아래와 같습니다.

## 1. ADB

### # ADB Connect

![/assets/2020-07-01/soo0700.png](/assets/2020-07-01/soo0700.png)

NoxPlayer의 Device에 ADB를 사용하고자 할 때 Device를 실행한 상태에서  `adb devices` 명령어를 사용하면 위와 같이 devices를 찾을 수 없습니다.

NoxPlayer의 Device는 **ADB 접속을 USB 통신이 아닌 Network 통신**을 합니다.

따라서 **Network 통신을 위한 ADB Port**를 찾아야합니다.

![/assets/2020-07-01/soo0701.png](/assets/2020-07-01/soo0701.png)

NoxPlayer의 Device들의 **Process 이름은 `NoxVMHandle.exe`**입니다.

`NoxVMHandle.exe`가 LISTENING 하고 있는 Port를 찾아보니 위와 같습니다.

![/assets/2020-07-01/soo0702.png](/assets/2020-07-01/soo0702.png)

LISTENING 하고 있는 Port를 1개씩 `adb connect 127.0.0.1:{PORT}` 하고 나서 `adb devices` 명령어로 devices 리스트를 확인 해본 결과 `62001` 이 ADB를 접속할 수 있는 Port 였습니다.

![/assets/2020-07-01/soo0703.png](/assets/2020-07-01/soo0703.png)

`adb connect`를 여러번 했기 때문에 여러 device가 등록되어 있고 adb에 `-s` 옵션을 통해서 원하는 device를 선택해야합니다. 또는 `adb kill-server` 명령어를 통해서 연결된 모든 device를 초기화 시킬 수 있습니다.

### # ADB Connect with Multi Device

NoxPlayer의 경우 멀티 앱플레이어 기능을 통해 여러 Device를 동시에 실행 할 수 있습니다. 여러 Device를 동시에 실행하고 있을 경우 각각의 ADB Connect를 위한 Port는 아래와 같습니다.

![/assets/2020-07-01/soo0704.png](/assets/2020-07-01/soo0704.png)

LISTENING 하고 있는 Port 중에서 위 과정을 반복한 결과 `62025`가 다른 Device의 ADB 접속 Port 입니다.

![/assets/2020-07-01/soo0705.png](/assets/2020-07-01/soo0705.png)

LISTENING 하고 있는 Port 중에서 위 과정을 반복한 결과 `62026`가 다른 Device의 ADB 접속 Port 입니다.

따라서 **NoxPlayer에서 ADB를 Connect 하기 위한 PORT**는 아래와 같습니다.

- **127.0.0.1:62001**
- **127.0.0.1:62025**
- **127.0.0.1:62026**
- **127.0.0.1:62027**
- **127.0.0.1:62028**

## 2. Frida

NoxPlayer의 Device에서도 일반적인 Android OS의 Device처럼 frida-server를 설치하는 방법으로 Frida를 사용할 수 있습니다.

frida-server는 [https://github.com/frida/frida/releases](https://github.com/frida/frida/releases) 에서 다운로드 가능하며 NoxPlayer의 경우 x86 기반의 에뮬레이터이기 때문에

`frida-server-{version}-android-x86`

를 다운로드 받아서 ADB를 통해 NoxPlayer의 Device에 아래와 같이 넣어준뒤 실행합니다.

```bash
# frida-server download & push
curl -L -O https://github.com/frida/frida/releases/download/12.11.6/frida-server-12.11.6-android-x86.xz
xz -d frida-server-12.11.6-android-x86.xz
adb -s 127.0.0.1:62001 push frida-server-12.11.6-android-x86 /data/local/tmp/

# frida-server chmod & run
cd /data/local/tmp/
chmod 755 frida-server-12.8.7-android-x86
./frida-server-12.8.7-android-x86
```

## 3. GDB

NoxPlayer에서도 gdb-server를 이용하여 gdb를 통한 디버깅을 할 수 있습니다. 

[https://chromium.googlesource.com/android_tools/+/refs/heads/2403/ndk/prebuilt/android-x86/gdbserver](https://chromium.googlesource.com/android_tools/+/refs/heads/2403/ndk/prebuilt/android-x86/gdbserver)에서 x86 gdbserver를 다운로드 받은 뒤 실행합니다.

```bash
# gdbserver download & push
curl -L -O https://chromium.googlesource.com/android_tools/+archive/refs/heads/2403/ndk/prebuilt/android-x86/gdbserver.tar.gz
tar -xvf gdbserver.tar.gz
adb -s 127.0.0.1:62001 push gdbserver /data/local/tmp/

# gdbserver chmod & run
cd /data/local/tmp/
chmod 755 gdbserver 
./gdbserver 0.0.0.0:62002 --attach [PID]
```

# Android App Dynamic Analysis with Remote NoxPlayer

NoxPlayer가 분석 환경에서 설치되지 않은 경우 Server에 NoxPlayer를 설치하고 Remote 환경에서 접속하여 분석할 수 있습니다. 따라서 Androd OS의 기기가 없으며 NoxPlayer를 사용할 수 없는 환경에서도 Server에 설치된 NoxPlayer를 이용하여 안드로이드 어플리케이션을 동적으로 분석 할 수 있습니다.

최종적으로 Frida Server, ADB, GDB Server를 사용할 PORT는 아래와 같습니다.

- 62000 - Frida Server
- 62001 - Nox ADB
- 62002 - GDB Server

## 1. **Windows 환경의 NoxPlayer가 설치된 Server**

### # Reverse Tunneling

NoxPlayer는 ADB 를 127.0.0.1을 LISTENING 하고 있기 때문에 리버스 터널링을 필요로 합니다. 따라서 NoxPlayer가 설치된 서버에서는 아래와 같이 NoxPlayer의 ADB port를 Reverse Tunneling 해주면 됩니다.

```bash
# NoxPlayer ADB PORT Reverse Tunneling
ssh -N jeon95u@jeong.su -R 0.0.0.0:62001:127.0.0.1:62001
```

## 2. 분석 PC

### 1. ADB

분석을 진행할 PC에서는 리버스 터널링된 서버와 포트를 통해 서버에서 NoxPlayer가 실행중인 Device의 ADB에 접속할 수 있습니다.

```bash
# ADB kill server
adb kill-server

# ADB remote connect & port forwarding (frida-server, gdbserver)
adb connect jeong.su:62001
adb forward tcp:62000 tcp:62000
adb forward tcp:62002 tcp:62002
adb shell
```

### 2. Frida

Frida가 서버에 있는 NoxPlayer에 설치되어 있지 않는 경우 위와 똑같은 방법으로 설치를 해줍니다. 설치가 되어 있다면 아래와 같이 frida-server를 실행 시켜줍니다.

```bash
mv /data/local/tmp/
./frida-server-12.8.7-android-x86 -l 0.0.0.0:62000
```

`1. ADB`에서 adb forward 명령어를 사용하여 분석 PC에 62000을 접속하게 되면 ADB를 타고 리모트에 있는 NoxPlayer의 Device의 62000을 포워딩 해줍니다.

따라서 분석 PC에서 바로 아래와 같이 frida 사용을 할 수 있습니다.

```bash
frida -H 127.0.0.1:62000
frida-ps -H 127.0.0.1:62000

frida -H 127.0.0.1:62000 com.example.app -l test.js
```

### 3. GDB

GDB가 서버에 있는 NoxPlayer에 설치되어 있지 않는 경우 위와 똑같은 방법으로 설치를 해줍니다. 설치가 되어 있다면 아래와 같이 gdbserver를 실행 시켜줍니다.

```bash
mv /data/local/tmp
./gdbserver 0.0.0.0:62002 --attach [PID]
```

`1. ADB`에서 adb forward 명령어를 사용하여 분석 PC에 62002을 접속하게 되면 ADB를 타고 리모트에 있는 NoxPlayer의 Device의 62002을 포워딩 해줍니다.

따라서 분석 PC에서 바로 아래와 같이 gdb 사용을 할 수 있습니다.

```bash
gdb
target remote 127.0.0.1:62002
```

# 마무리 & 추가 연구 과제

iOS 환경의 스마트폰을 사용하는 경우 급하게 Android 어플리케이션을 동적으로 분석하고자 할 때 요긴하게 쓰이고자 작성된 문서입니다.

위 방법으로는 ADB, Frida, GDB 등 간단하게 CLI 환경에서 할 수 있는 것만 사용 가능하며 특정 Activity 실행 등은 대부분 Activity Manager를 통해 ADB shell에서 호출이 가능하지만 그 외 GUI를 사용해야하는 경우는 추가 연구가 필요할 것 으로 생각됩니다.