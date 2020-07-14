---
layout: post
title:  "MacOS Catalina 10.15.2 Zero-click RCE"
author: "nextline"
comments: true
tags: [mac]
---

라온화이트햇 핵심연구팀 이영주


# 1. Introduction

최근 macOS에서 Zero-click RCE가 발견되었습니다. 현재 macOS 최신버전이 Catalina 10.15.4인데, 취약점이 존재하는 버전이 Catalina 10.15.2 인것을 생각하면 6개월내로 패치하지 않은 모든 MacOS에서 Zero Click RCE가 가능합니다. 따라서 지금 MacOS를 쓰고 계신다면 바로 업데이트 하시는것을 추천드립니다. 취약점은 [https://support.apple.com/en-us/HT210919](https://support.apple.com/en-us/HT210919) 에서 확인할 수 있습니다. 

아래 취약점 분석 글에 나와있는 사진이나 내용은 [http://blogs.360.cn/post/macOS_Bluetoothd_0-click.html](http://blogs.360.cn/post/macOS_Bluetoothd_0-click.html)를 참고하였고 거기에 제가 이 취약점을 이해하면서 봤던 자료들이나 문서들의 내용을 함께 덧붙였습니다.

# 2. Bluetooth

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled.png)

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%201.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%201.png)

먼저 취약점을 이해하기 전에 블루투스에 대해 간략히 소개하겠습니다. 블루투스는 위와 같은 프로토콜 스택을 가지고 있습니다. 프로토콜 스택이란 소포트웨어나 하드웨어가 제대로 동작할 수 있게 하기 위한 프로토콜들들의 집합입니다. 여기서 집중적으로 볼 프로토콜은 L2CAP  (Logical Link Control and Adaption Protocol) 입니다.

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%202.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%202.png)

macOS에서 L2CAP은 IOBluetoothFamily 라는 커널 드라이버에 의해 처리되고, SDP나 BNEP같은 L2CAP의 데이터들은 bluetoothd라는 루트 권한의 프로세스에서 처리합니다.

# 3. CVE-2020-3847

CVE-2020-3847은 리모트 out-of-bounds read 취약점으로 특정 패킷을 보내 heap memory를 leak 해올 수 있습니다. 이 취약점을 트리거 하기 위해서는 두번 연결을 요청해야 합니다.

[ 그림 1 ]

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%203.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%203.png)

[ 그림 2 ]

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%204.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%204.png)

### 첫번째 요청

1. [그림 1] 136번째 줄: pdata가 입력이기 때문에 cont_state에 원하는 값을 넣어줄 수 있습니다. 따라서 0을 입력하여 143번째 에서 is_cont_pkt를 false로 만들 수 있습니다.
2. [그림 2] 270번째 줄: rem_len은 쿼리의 length를 나타내기 때문에 간접적으로 저희가 컨트롤 할 수 있습니다. 따라서 그 값을 0x10으로 가정하겠습니다. 그리고 max_list_len의 경우 패킷에서 2바이트를 읽어와 값을 대입합니다. 그래서 이 값도 컨트롤이 가능합니다. 이 값을 0x10보다 작게 만들어서 else루틴을 타게 합니다.
3. [그림 2] 280번째 줄: 첫번째 과정에서 is_const_pkt를 false로 만들었기 때문에 if문에 내용을 실행합니다. 그 내용은 pSDPServerConn + 16에 rem_len을 넣고 pSDPServerConn+7에 rem_len만큼 할당한 청크를 넣는 내용입니다. 그리고 첫번째 연결을 종료합니다.

### 두번째 요청

1. [그림 1] 153번째 줄: cont_offset은 unsigned int 변수로 패킷에서 4바이트를 읽어옵니다. (직접 컨트롤 가능)
2. [그림 1] 154번째 줄: cont_offset으로 0x12를 넣는다면 아까 0x10을 넣어준 rem_len(pSDPServerConn + 16)에서 cont_offset(0x12)를 unsigned로 빼므로 rem_len은 4294967294가 됩니다. (인티저 오버플로우)
3. [그림 2] 270번째 줄: 그리고 max_list_len이 rem_len(4294967294)보다 작으므로 else분기로 가게됩니다.
4. [그림 2] 278번째 줄: max_list_len은 맘대로 설정할 수 있으므로 프로토콜의 최대 전송 단위인 MTU(672) 값보다 작게 설정해줍니다.
5. [그림 2] 280번째 줄: 마지막으로 is_cont_pkt 또한 맘대로 설정할 수 있으므로 true로 만들어 if 내용을 실행하지 않도록 합니다.
6. 그럼 마지막으로 아래와 같은 코드가 실행됩니다.

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%205.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%205.png)

7. 여기서 cont_offset은 저희가 컨트롤 할 수 있는 값이므로 out-of-bounds read를 할 수 있는데, v74값이 max_list_len이기 때문에 원하는 만큼 결과 버퍼에 저장할 수 있습니다. ServiceAttributeResults가 아까 위에서 rem_len만큼 할당한 chunk (malloc 0x10) 이기 때문에 힙에있는 데이터들을 읽어오는게 가능합니다.

# 4. CVE-2020-3848

CVE-2020-3848은 remote memory corruption 취약점 입니다. 이 취약점은 [SDPClientConnection handleServiceSearchAttributeResponse:length:transactionID:] 함수에 존재합니다. 이 취약점의 경우 위에서 설명한 leak 취약점에 비해 매우 간단합니다.

[그림 1]

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%206.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%206.png)

[그림 2]

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%207.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%207.png)

1. [그림 1] 105번째 줄: v44 값을 패킷에서 원하는대로 설정할 수 있습니다.
2. [그림 1] 116번째 줄: v44 값이 있으면 v43을 true로 셋팅합니다.
3. [그림 2] 149번째 줄: v44 값만큼 memcpy를 하는데, memcpy하는 버퍼가 malloc(0x20)의 static 사이즈 입니다. 하지만 v44는 패킷에서 값을 가져오기 때문에 0xff까지 설정할 수 있고 이로인해 heap overflow가 발생합니다.

# 5. Exploit

일단 현재 exploit은 공개된게 없습니다. 아마 취약점의 파급력 때문이거나 애플에게 허락을 받지 못해 그런것 같습니다. 하지만 이 취약점을 찾은 @jioundai가 exploit을 한 방법에 대해서 간략하게 소개했습니다.

먼저 위 두개의 취약점이 heap 영역에서 발생하는 취약점 이므로 heap feng shui 기법을 사용하였습니다. 하지만 Zero-click으로 exploit을 해야 했기 때문에 할당할 수 있는 heap이 매우 제한적이며 따라서 exploit 확률도 올리기가 쉽지 않았다고 합니다.

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%208.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%208.png)

결국 chunk를 할당할 때, macOS에 많은 SDP 소켓 커넥션을 맺어서 SDPServerConnection 오브젝트를 여러개 할당하였습니다. 그리고 아래와 같은 순서를 통해 최종적으로 exploit을 했다고 합니다.

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%209.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%209.png)

1. 위 그림처럼 heap feng shui 기법을 통해 heap overflow가 발생하는 chunk의 위치를 특정할 수 있게 합니다.
2. 비교적 자유로운 heap leak (0xff)를 통해 heap feng shui가 성공했는지 계속 체크합니다.
3. heap feng shui가 성공했다면 heap overflow 취약점을 이용해 원하는 객체를 덮어 실행 흐름을 변경합니다. (이때 Objective-c's exploit 기법을 썼다는데 아무래도 Objective-c에서 사용할 수 있는 범용적인 exploit 기법이 존재하는 것 같습니다.)

 

![/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%2010.png](/assets/07a6bd10-696c-4843-b1a7-39a2eafbea23/92c099ee-b4f6-4c6d-a974-28623b447859Untitled%2010.png)

마지막으로 한번더 정리하자면 CVE-2020-3847로 heap memory를 leak할때는 공격자가 client 역할을 해야하고, CVE-2020-3848로 overflow 취약점을 트리거 할때는 attacker가 server 역할을 해야합니다. 

# 6. Conclusion

이 취약점과 exploit 과정을 보면서 가장 대단하다고 생각했던 부분이 Zero-click exploit 파트입니다. 물론 취약점 자체도 놀랍지만 그 취약점을 유저인터렉션 없이 exploit하기 위해서 얼마나 많은 기반지식과 숙련도가 필요한지 다시한번 깨닫게 해주었습니다. 

특히나 CVE-2020-3848 취약점을 exploit 한게 가장 놀라웠는데, 이 취약점을 처음 발견했을때는 유저 인터렉션 없이 취약점을 트리거할 수 없었습니다. 취약점을 트리거하기 위해서는 Bluetooth 페어링이 필요해서 Zero-click으로 트리거할 수 없었기 때문입니다. 이 점을 이전에 Bluetooth SDP protocol을 연구하면서 유추한 사실로 다른 기능을 통해 Zero-click RCE을 성공시킨 점이 @jioundai가 그동안 이런 취약점들에 대해 얼마나 많은 연구를 해왔고 Objective-c 리버싱 숙련도가 높다는 사실을 알 수 있습니다. 또한 exploit에 있어서도 macOS의 힙 메커니즘과 Objective-c exploit 기법에 대해 자세히 알고있고 그것을 통해 유저 인터렉션 없는 heap feng shui를 성공시킨것이 매우 인상깊었습니다.

따라서 위 취약점을 분석하면서 파급력 높은 0-day 연구를 진행할때 가져야할 마인드나 공부량에 대해 다시 한번 생각할 수 있었고 Logical 취약점이 발생하는 과정에 대해 익힐 수 있는 좋은 기회가 되었습니다.