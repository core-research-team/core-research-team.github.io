---
layout: post
title: "Windows 환경에서  WEB Proxy(Burp Suite, Fiddler) 탐지 및 구현"
author: "vulhack"
comments: true
tags: [windows, analysis]
---

라온화이트햇 핵심연구팀 김현수

# Windows 환경에서  WEB Proxy(Burp Suite, Fiddler) 탐지 및 구현

## 1. Summary

웹 프록시(Web Proxy) 도구를 이용해 금융, 증권사 등의 웹 홈페이지 취약점 진단 시, 웹 Proxy(Web Proxy) 를 탐지하는 경우가 증가하고 있습니다. 이는 어떻게 탐지하고 구현되는지, Case Study 를 통해 알아보는 시간을 가져보려고 합니다.

## 2. WEB Proxy?

웹 프록시(Web Proxy) 는 클라이언트(Client)가 자신(Proxy) 를 통해서 서버(Server) 와 의 웹 프로토콜(http, https) 통신에 대한 서비스에 간접적으로 접속하여 클라이언트와 서버 사이의 중계기로서 대리 통신의 역할을 수행해주는 응용프로그램 입니다. 

웹 프록시(Web Proxy) 에는 대표적으로 Fiddler, Burp Suite 등이 있습니다. 해당 프로그램을 이용해 서버(Server) 와 클라이언트(Client) 간의 웹 패킷을 위/변조 할 수 있습니다. 

흔히 웹 해킹(Web Hacking) 에 쓰이는 도구이며 패킷 위변조, 자동화 공격 등 웹 프록시(Web Proxy) 를 통해 다양한 공격을 수행 할 수 있습니다. 

## 3. Window API 사용

웹 프록시(Web Proxy) 를 탐지할 수 있는 수많은 방법과 함수, API 가 존재하지만 그 중 아래 3가지 Windows API 를 통해 웹 프록시(Web Proxy) 프로그램을 탐지하는 방법을 알아보겠습니다. 

해당 API를 사용하여 구현한 이유는 상용 보안 솔루션 등에서 취약점 분석 도구 탐지 시, 간혹 '**Window Class**' 이름 과, '**Window Caption**' 이름은 조작이 힘든 점 등의 이유로 다음과 같이 '**Window Class**', '**Window Caption**' 이름 등을 통해 프로그램 작동 여부를 확인 하기 때문입니다. 

**※ Window Class, Window Caption 예시**

![Untitled](/assets/2021-12-01/Untitled.png)

**Burp Suite Class Name** : SunAwtFrame

**Burp Suite Caption Name** :  Burp Suite Professional v2020.12.1 - Temporary Project - hyeonsu

**Fiddler Class Name** : WindowsForms10.Window.8.app.0.13965fa-r6_ad1

**Fiddler Caption Name** : Progress Telerik Fiddler Web Debugger

**※ 사용할 Windows API**

- FindWindowExA
- FindWindowExW
- EnumWindows

 

FindWindowExA(), FindWindowExW() 함수는 FindWindowEx() 함수에서 기능이 확장된 함수 입니다. FindWindow() 함수는 Main Window 만 검색이 가능하여, 자식 Window 검색이 불가 합니다. 범용적으로 사용하기 위해 FindWindowExA(), FindWindowExW() 함수를 사용하였습니다.

**※ 연관된 Windows API**

- FindWindow
- FindWindowEx

FindWindowEx() 함수는 '**Window Class**' 의 이름이나, '**Window Caption**' 이름으로 Window의 핸들 값을 얻는 함수 입니다. 

FindWindow() 함수와의 다른 점은 FindWindowEx() 함수는 특정 Window 에 포함된 자식 Window 를 탐색할 때 사용 됩니다.

FindWindowExA() 함수와  FindWindowExW() 함수의 차이는 함수의 매개변수 'String Type' 과 관련이 있습니다.  

FindWindowExA() 함수는  윈도우 클래스의 매개변수가 LPCSTR (Const String Type) 멀티바이트이고, FindWindowExW() 함수의 클래스 명의 매개변수는 LPCWSTR(Wide String Type) 유니코드 입니다.

> **FindWindowExA**
> 

`FindWindowExA()` 함수는 Windows 32API 함수로, '**Window Class**' 이름 및 'Window' 이름이 지정된 문자열과 일치하는 Window에 대한 핸들을 검색하는 함수입니다. 

```cpp
HWND FindWindowExA(
  HWND   hWndParent,     //자식 창을 검색 할 부모 창에 대한 핸들.
  HWND   hWndChildAfter, //자식 창에 대한 핸들.
  LPCSTR lpszClass,      //'Window Class' 이름(멀티바이트).
  LPCSTR lpszWindow      //창 이름 매개 변수가 NULL 이면 모든 창에서 검색.
);
```

> **FindWindowExW**
> 

`FindWindowExW()` 함수는 Windows 32API 함수로, '**Window Class**' 이름 및 'Window' 이름이 지정된 문자열과 일치하는 Window에 대한 핸들을 검색하는 함수입니다.

```cpp
HWND FindWindowExW(
  HWND   hWndParent,      //자식 창을 검색 할 부모 창에 대한 핸들.
  HWND   hWndChildAfter,  //자식 창에 대한 핸들.
  LPCWSTR lpszClass,      //'Window Class' 이름(유니코드).
  LPCWSTR lpszWindow      //창 이름 매개변수가 NULL 이면 모든 창에서 검색.
);
```

> **EnumWindows**
> 

각 창에 핸들을 차례로 전달하여 애플리케이션 정의 콜백 함수에 전달하여 화면의 모든 최상위 창을 열거합니다. `EnumWindows()` 함수는 마지막 최상위 창이 열거되거나 콜백 함수가 FALSE를 반환 할 때까지 계속 됩니다.

```cpp
HWND FindWindowExW(
  HWND   hWndParent,      //자식 창을 검색 할 부모 창에 대한 핸들.
  HWND   hWndChildAfter,  //자식 창에 대한 핸들.
  LPCWSTR lpszClass,      //창 클래스 이름을 지정. 
  LPCWSTR lpszWindow      //창 이름 매개변수가 NULL 이면 모든 창에서 검색.
);
```

## 4. Burp Suite Detection

Microsoft Spy++ 을 통해 "Burp Suite" 의 '**Window Class**' 이름을 확인할 수 있습니다. 

![Untitled](/assets/2021-12-01/Untitled%201.png)

Microsoft Spy++ 를 통해 Burp Suite 의 '**Window Class**' 이름을 확인하였습니다. "Burp Suite" 의 '**Window Class**' 이름은 Java GUI 플랫폼인 "SunAwtFrame" 입니다.

Burp Suite 의 '**Window Class**' 이름 인, "SunAwtFrame" 는, Java GUI 플랫폼으로 타 프로그램에서 사용하였을 수도 있습니다. 따라서 해당 정보 만 가지고 "Burp Suite" 의 실행 여부를 확인할 수는 없습니다.

### 4-1. **C, C++ Source Example 01**

```cpp
/*
   FindWindowExA(), FindWindowExW() functions Burp Suite Web Proxy detection
*/
#include <Windows.h> 
#include <iostream> 
using namespace std;

bool findClassname(LPCSTR m_classname, LPCWSTR u_classname) {

    HWND hw1 = FindWindowExA(NULL, NULL, m_classname, NULL); 
    HWND hw2 = FindWindowExW(NULL, NULL, u_classname, NULL);

    if (hw1 != NULL || hw2 != NULL) {
        cout << "Find SunAwtFrame Class Name." << endl;
        return true;
    } 
    else {
        cout << "Not detected." << endl;
        return false;
    }
}
int main() {
    bool result = findClassname("SunAwtFrame",L"SunAwtFrame");
}
```

FindWindowExA() 함수와  FindWindowExW() 함수를 통해 "SunAwtFrame" 을 사용하는 '**Window Class**' 이름을 식별할 수 있습니다. 이는 "Burp Suite" 의 실행 가능성을 열어둘 수 있습니다.

### 4-2. **C, C++ Source Output 01**

![Untitled](/assets/2021-12-01/Untitled%202.png)

"Burp Suite" 에서 사용되는 "**Window Class**" 이름인 "SunAwtFrame" 을 탐지한 것을 확인할 수 있습니다.

### 4-3. **C, C++ Source Example 02**

```cpp
/* 
   WEB Proxy detector using enumWindows() function
*/
#include <Windows.h> 
#include <tchar.h> 
#include <iostream> 
using namespace std;
int CALLBACK findProxy(HWND hwnd, LPARAM param) {
    TCHAR name[256];
    TCHAR title[256];
    GetClassName(hwnd, name, 256);
    GetWindowText(hwnd, title, 256);
    if (IsWindowVisible(hwnd) == FALSE) {
        return 1;
    }
    if (GetWindowTextLength(hwnd) == 0) {
        return 1;
    }
    if (GetParent(hwnd) != 0) {
        return 1;
    }
    if(strstr(title,"Burp Suite") != NULL){
    	  cout << "Find Burp Suite !" << endl;	
	     
		}
	  _tprintf(_T("HWND : %x, %s - %s\n"), hwnd, name, title);
    return 1;	 
}
int main() {
    EnumWindows(findProxy, 0);
}
```

조금 전, FindWindowExA() 함수와  FindWindowExW() 함수를 통해 "SunAwtFrame" 을 사용하는 '**Window Class**' 이름을 식별할 수 있었습니다. 

이번에는 EnumWindows() 함수를 사용하여 Window 창의 모든 문자열을 열거하여 모든 Window 의 "**Window Caption**" 이름 중, "Burp Suite" 가 존재하는지 확인해보겠습니다.

### 4-4. **C, C++ Source Output 02**

![Untitled](/assets/2021-12-01/Untitled%203.png)

수많은 "Window Handle" 값과 "**Window Caption**" 이름이 출력 되고, 그 중 "Burp Suite" 의 Caption 이름이 존재하는 것을 확인할 수 있습니다.

### 4-5. **Simple** Burp Suite Detector **Source Example**

```cpp
/*
   Simple Brup Suite Detector
*/
#include <Windows.h> 
#include <iostream> 
#include <tchar.h> 
using namespace std;
bool findClassname(LPCSTR m_classname, LPCWSTR u_classname) {
    HWND hw1;
    HWND hw2;

    hw1 = FindWindowExA(NULL, NULL, m_classname, NULL); 
    hw2 = FindWindowExW(NULL, NULL, u_classname, NULL); 

    if (hw1 != NULL || hw2 != NULL) {
        cout << "[*] Find SunAwtFrame Class Name." << endl;
        return true;
    } 
    else {
        cout << "Not detected." << endl;
        return false;
    }
}
int CALLBACK findProxy(HWND hwnd, LPARAM param) {
    TCHAR name[256];
    TCHAR title[256];
    GetClassName(hwnd, name, 256);
    GetWindowText(hwnd, title, 256);
    if (IsWindowVisible(hwnd) == FALSE) {
        return 1;
    }
    if (GetWindowTextLength(hwnd) == 0) {
        return 1;
    }
    if (GetParent(hwnd) != 0) {
        return 1;
    }
    if(strstr(title,"Burp Suite") != NULL){
    	cout << "[*] Find Burp Suite !" << endl;	
    	_tprintf(_T("HWND : %x, %s - %s\n"), hwnd, name, title);
	}
    return 1;
}

int main() {
    bool result = findClassname("SunAwtFrame",L"SunAwtFrame");
    if(result){
    	EnumWindows(findProxy,0);             
	}
}
```

FindWindowExA() / FindWindowExW() / EnumWindows() 함수를 통해  WEB Proxy 프로그램("Burp Suite") 를 탐지할 수 있습니다.

### 4-6. **Simple** Burp Suite Detector **Source Output**

![Untitled](/assets/2021-12-01/Untitled%204.png)

WEB Proxy 프로그램 인 "Burp Suite" 가 탐지 된 것을 확인할 수 있습니다.

## 5. Fiddler Detection

Microsoft Spy++ 을 통해 "Fiddler" 의 '**Window Class**' 이름을 확인할 수 있습니다.

![Untitled](/assets/2021-12-01/Untitled%205.png)

Microsoft Spy++ 를 통해 "Fiddler" 의 '**Window Class**' 이름을 확인하였습니다. "Fiddler" 의 '**Class Name**' 이름은 Winforms(C#,VB.NET) 에서 .NET에 의해 개발된 것으로 추측되는 "WindowsForms10.Window.8.app.0.13965fa_r6_ad1" 입니다. 

"Fiddler" 의 '**Window Class**' 이름 인, "WindowsForms10.Window.8.app.0.13965fa_r6_ad1" 는, Winforms(C#,VB.NET)  플랫폼으로 개발 된 것으로 추측되고, 유일한 값 이 아닙니다. 따라서 해당 정보만 가지고 "Fiddler" 사용 여부를 확인할 수 없습니다.

### 5-1. **C, C++ Source Example 01**

```cpp
/*
   FindWindowExA(), FindWindowExW() functions Fiddler Web Proxy detection
*/
#include <Windows.h> 
#include <iostream> 
using namespace std;

bool findClassname(LPCSTR m_classname, LPCWSTR u_classname) {

    HWND hw1 = FindWindowExA(NULL, NULL, m_classname, NULL); 
    HWND hw2 = FindWindowExW(NULL, NULL, u_classname, NULL);

    if (hw1 != NULL || hw2 != NULL) {
        cout << "Find Fiddler Class Name." << endl;
        return true;
    } 
    else {
        cout << "Not detected." << endl;
        return false;
    }
}
int main() {
    bool result = findClassname("WindowsForms10.Window.8.app.0.13965fa_r6_ad1",L"WindowsForms10.Window.8.app.0.13965fa_r6_ad1");
}
```

FindWindowExA() 함수와  FindWindowExW() 함수를 통해 "SunAwtFrame" 을 사용하는 '**Window Class**' 이름을 식별할 수 있습니다. 이는 "Fiddler" 의 실행 가능성을 열어둘 수 있습니다.

이번에는 EnumWindows() 함수를 사용하여 Window 창의 모든 문자열을 열거하여 모든 Window 의 '**Window Caption'** 이름 중, "Fiddler" 가 존재하는지 확인할 수 있습니다.

### 5-2. **C, C++ Source Output 01**

![Untitled](/assets/2021-12-01/Untitled%206.png)

"Fiddler" 에서 사용되는 '**Window Class'**이름인 "WindowsForms10.Window.8.app.0.13965fa_r6_ad1" 을 탐지한 것을 확인할 수 있습니다.

### 5-3. **C, C++ Source Example 02**

```cpp
/* 
   Fiddler WEB Proxy detector using enumWindows() function
*/
#include <Windows.h> 
#include <tchar.h> 
#include <iostream> 
using namespace std;
int CALLBACK findProxy(HWND hwnd, LPARAM param) {
    TCHAR name[256];
    TCHAR title[256];
    GetClassName(hwnd, name, 256);
    GetWindowText(hwnd, title, 256);
    if (IsWindowVisible(hwnd) == FALSE) {
        return 1;
    }
    if (GetWindowTextLength(hwnd) == 0) {
        return 1;
    }
    if (GetParent(hwnd) != 0) {
        return 1;
    }
    if(strstr(title,"Fiddler") != NULL){
    	  cout << "Find Fiddler !" << endl;	
	     
		}
	  _tprintf(_T("HWND : %x, %s - %s\n"), hwnd, name, title);
    return 1;	 
}
int main() {
    EnumWindows(findProxy, 0);
}
```

FindWindowEx, FindWindowExW 함수를 통해 "WindowsForms10.Window.8.app.0.13965fa_r6_ad1" 를 사용하는 '**Window Class**' 이름을 식별할 수 있었습니다. 

이번에는 EnumWindows() 함수를 사용하여 Window 창의 모든 문자열을 열거하여 모든 Window 의 "**Window Caption**" 이름 중, "Fiddler" 가 존재하는지 확인해보겠습니다.

### 5-4. **C, C++ Source Output 02**

![Untitled](/assets/2021-12-01/Untitled%207.png)

수많은 "Window Handle" 값과 "**Window Caption**" 이름이 출력 되고, 그 중 "Fiddler" 의 Caption 이름이 존재하는 것을 확인할 수 있습니다.

### 5-5. **Simple Fiddler** Detector **Source Example**

```cpp
/*
   Simple Fiddler Detector
*/
#include <Windows.h> 
#include <iostream> 
#include <tchar.h> 
using namespace std;
bool findClassname(LPCSTR m_classname, LPCWSTR u_classname) {
    HWND hw1;
    HWND hw2;

    hw1 = FindWindowExA(NULL, NULL, m_classname, NULL); 
    hw2 = FindWindowExW(NULL, NULL, u_classname, NULL); 

    if (hw1 != NULL || hw2 != NULL) {
        cout << "[*] Find WindowsForms10.Window.8.app.0.13965fa_r6_ad1 Class Name." << endl;
        return true;
    } 
    else {
        cout << "Not detected." << endl;
        return false;
    }
}
int CALLBACK findProxy(HWND hwnd, LPARAM param) {
    TCHAR name[256];
    TCHAR title[256];
    GetClassName(hwnd, name, 256);
    GetWindowText(hwnd, title, 256);
    if (IsWindowVisible(hwnd) == FALSE) {
        return 1;
    }
    if (GetWindowTextLength(hwnd) == 0) {
        return 1;
    }
    if (GetParent(hwnd) != 0) {
        return 1;
    }
    if(strstr(title,"Fiddler") != NULL){
    	cout << "[*] Find Fiddler !" << endl;	
    	_tprintf(_T("HWND : %x, %s - %s\n"), hwnd, name, title);
   	}
    return 1;
}

int main() {
    bool result = findClassname("WindowsForms10.Window.8.app.0.13965fa_r6_ad1",L"WindowsForms10.Window.8.app.0.13965fa_r6_ad1");
    if(result){
    	EnumWindows(findProxy,0);             
	  }
}
```

FindWindowExA() / FindWindowExW() / EnumWindows() 함수를 통해  WEB Proxy 프로그램("Fiddler") 를 탐지할 수 있습니다.

### 5-6. **Simple Fiddler** Detector **Source Output**

![Untitled](/assets/2021-12-01/Untitled%208.png)

WEB Proxy 프로그램 인 "Fiddler" 가 탐지 된 것을 확인할 수 있습니다.

## 6. 마치며

위와 같은 방법으로 Windows 환경에서 WEB Proxy(Burp Suite, Fiddler) 를 탐지하고, 구현하는 방법에 대해 알아보았습니다. 

WEB Proxy 탐지 방법은 여러 CASE 중 일부 Window API 를 이용해 구현 된 것 이며, CASE Study 차원 에서 구현되었습니다. 조금이나마 도움이 되었으면 하는 바램입니다. 긴 글 읽어주셔서 감사합니다.