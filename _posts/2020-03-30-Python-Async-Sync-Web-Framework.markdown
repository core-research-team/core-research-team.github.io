---
layout: post
title:  "Python Async & Sync Web Framework"
author: "onestar"
comments: true
tags: [programming]
---

라온화이트햇 핵심연구팀 황정식

# **Python GIL(Global Interpreter Lock)?**

 동기식 웹과 비동기식 웹을 알아보기 전에, Python의 GIL에 대한 개념을 먼저 설명해야 할 것 같습니다. Python에서 GIL은 Global Interpreter Lock의 약자로, **파이썬 인터프리터가 하나의 Thread만 하나의 Byte code로 실행할 수 있도록 하는 것**입니다. GIL은 하나의 Thread에게 모든 자원의 점유를 허락합니다. 

병렬 프로그래밍에서 얘기하는 작업간의 동기화 정도에 따른 분류는 다음과 같이 크게 두 가지로 구분됩니다.

- **coarse-grained lock**
    - 프로세서의 하위 작업 간의 통신에서, 동기화가 자주 일어나지 않음

- **find-grained lock**
    - 프로세서의 하위 작업 간의 통신에서, 동기화가 자주 발생함

파이썬에서 **GIL은 하나의 Thread가 인터프리터의 모든 것을 가져갑**니다. 이는 사실상 **coarse-grained**, 혹은 **embarrassingly parallel(처치 곤한 병렬)**의 형태라고 할 수 있습니다.

이러한 GIL로 인해 Multi-Threading 프로그래밍이 제한되어 있습니다. 다만 Single-Thread 환경에서의 성능은 꽤 보장되는 것으로 보입니다.

# Python Sync Web Framework

Python Flask와 Django의 경우 동기식 웹 프레임워크로써, 위와 같은 형태의 Single-Thread 형태의 코드라고 볼 수 있습니다. 즉, 한 이용자가 웹에 접근하여 특정 처리를 완료하기 전까지, 다른 이용자는 대기 상태가 됩니다.

실제로 python3 [run.py](http://run.py) 와 같이 실행하여 웹을 여러 유저에게 열어두고 접속하도록 하면, 순차적으로 Response를 수행하는 것을 볼 수 있습니다.

이를 개선하기 위해 Python Flask/Django에 웹서버인 Apache2 혹은 Nginx를 연동합니다. 이럴 경우 Thread를 늘려주는 것이 아닌 Process 개수를 늘려, 보다 높은 품질의 웹 어플리케이션을 경험할 수 있도록 해줍니다.

현재 RAON CTF 또한 Python Flask + Apache2를 연동하여 이러한 이슈를 해결하고 있습니다.

# Python Async Web Framework

파이썬에서는 위와 같이 Django, Flask와 같은 웹 프레임워크로도 충분히 개발과 서비스가 가능합니다. 하지만 비동기 방식으로 동작하는 웹 프레임워크들도 활발히 개발 중에 있습니다.

이에 대표적으로는 sanic과 vibora를 들 수 있습니다. 이 두 프레임워크는 앞서 말씀드린 프레임워크보다도 월등한 성능을 보여주며, 현재 카카오를 비롯한 다양한 기업이 이를 이용하여 생산단계에서 이용중인 프레임워크라고 합니다.

![/assets/d560474a-2ffd-44d1-96cf-811a77d78db6/03435ae4-074d-4066-a990-60f915cc9155.png](/assets/d560474a-2ffd-44d1-96cf-811a77d78db6/03435ae4-074d-4066-a990-60f915cc9155.png)

이와 함께 눈여겨볼 수 있는 비동기식 웹 프레임워크로는 asyncio와 tornado가 있습니다.