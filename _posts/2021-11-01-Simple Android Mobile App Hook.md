---
layout: post
title: "Simple Android Mobile App Hook"
author: "vulhack"
comments: true
tags: [frida, android]
---

라온화이트햇 핵심연구팀 김현수

# Simple Android Mobile App Hook

### 1. Summary

2021년 04월 24일 진행되었던 HSPACE CTF THE ZERO 대회의 Android Pengsu Wallet 문제를 출제하면서 문제 풀이 차원에서 Frida 를 이용한 Simple Android Mobile App Hooking 주제로 포스팅을     작성하게 되었습니다.

### 2. Rooting

문제 분석에 앞서, 해당 문제는 `Rooting`을 탐지하는 로직이 존재 합니다. 따라서 간략하게 `Rooting`이 무엇인지 소개 하고자 합니다.  `Rooting`은 모바일 기기에서 구동되는 안드로이드 운영 체제 상에서 최상위 권한 root 권한을 얻음으로 해당 기기의 벤더사에서 걸어 놓은 제약을 해제하는 행위 입니다.  

즉, 정상적인 Android Device 의 경우 root 권한 이 아닌, 일반 사용자의 권한이지만, `Rooting(루팅)`을 할 경우 root(최고권한) 의 사용자가 되는 것 입니다.

사용자는 `Rooting(루팅)`을 통해 소프트웨어, 하드웨어를 사용자가 직접 커스텀 할 수 있고 다양한 권한이 필요한 작업에 대해 제약없이 사용할 수 있습니다. 이와 같은 이유 때문에 다양한 모바일 보안 솔루션에서는 `Rooting(루팅)` 을 감시 하고 사용을 제한하기도 합니다.

iOS Device 에서는 이와 같은 root 권한을 얻는 행위를 `Jail Break(탈옥)` 이라고 불립니다.

### 3. APP Analysis

![Untitled](/assets/2021-11-01/Untitled.png)

NOX 에뮬레이터의 ROOT 켜기 모드를 통해 앱 실행 시, "Detection Rooted or ADB Process!" 를 출력하며 앱이 실행되지 않습니다. 앱 내에서 `Rooting` 을 탐지하고 종료하는 것으로 추측 됩니다.

NOX 에뮬레이터의 ROOT 켜기 모드를 OFF 한 후 앱 실행 시, 아래와 같이 Pengsu 와 함께 보물 상자가 나타납니다. button 을 클릭하면 Pengsu Wallet 의 $ 가 랜덤으로 증가 하고, progress bar 가 상승합니다. 내부 알고리즘에 의해 특정 이벤트 시, $ 가 0으로 초기화 됩니다.

![Untitled](/assets/2021-11-01/Untitled%201.png)

JEB(Android Decompiler) 를 이용해 APK 디컴파일을 시도 합니다. MainActivity, RootCheck 클래스가 존재하는 것을 확인할 수 있습니다.

![Untitled](/assets/2021-11-01/Untitled%202.png)

MainActivity 클래스 내 onClick 메소드를 확인 한 결과, 사용자가 Button 클릭 시, Pengsu Wallet 으로 추측되는 v1 변수가 랜덤 값에 의해 증가합니다. 

하지만 랜덤한 값이 3, 5, 7, 9, 10 일 경우 v1 변수를 0으로 초기화 합니다. 랜덤 값은 1~10 사이의 정수 이고, 확률적으로 v1 의 값을 100까지 증가 하기는 어려울 것으로 보입니다.

해당 값이 100$ 가 될 시, a.a() 메소드를 호출하는 것을 확인할 수 있습니다. Pengsu Wallet 의 값을 게임 내에서 100$ 로 만드는 것은 불가능 합니다. 즉, `Hooking` 을 통해  `Rooting` 을 우회하고 Pengsu Wallet 의 값을 변조해야 합니다.

![Untitled](/assets/2021-11-01/Untitled%203.png)

a() 메소드를 확인한 결과, `Rooting` 탐지를 우회 하고, Pengsu Wallet 의 값이 100$ 일 경우 암호화된 FLAG 를 복호화 하여 Android Log 를 통해 출력하는 것을 확인할 수 있습니다.

```java
public static void a() { 
        a.a = new RootCheck();
        MainActivity v0 = new MainActivity();
        if(a.a.rootCheck()) {
        label_43:
            Log.d("CipherAlgorithm", String.valueOf(a.a.rootCheck()));
            Log.d("CipherAlgorithm", "Please follow the game rules");
            v0.finish();
        }
        else {
            StringBuffer v3 = new StringBuffer();
            try {
                Process v1_1 = Runtime.getRuntime().exec("ps adbd");
                v1_1.waitFor();
                BufferedReader v4 = new BufferedReader(new InputStreamReader(v1_1.getInputStream()));
                while(true) {
                    String v1_2 = v4.readLine();
                    if(v1_2 == null) {
                        break;
                    }

                    v3.append(v1_2 + "\n");
                }
            }
            catch(Exception v1) {
                v1.printStackTrace();
            }

            if(v3.toString().indexOf("bad") == -1) {
                goto label_43;
            }

            Log.d("CipherAlgorithm", "Detection Not Rooted or ADB!");
        }

        String v1_3 = RootCheck.a();
        "hello android world!".getBytes("UTF-8");
        byte[] v0_1 = "hello android world!".getBytes();
        MessageDigest v5 = MessageDigest.getInstance("MD5");
        v5.update("SOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEER".getBytes());
        byte[] v5_1 = v5.digest();
        MessageDigest v6 = MessageDigest.getInstance("SHA-256");
        v6.update(v1_3.getBytes());
        byte[] v6_1 = v6.digest();
        IvParameterSpec v7 = new IvParameterSpec(v5_1);
        SecretKeySpec v5_2 = new SecretKeySpec(v6_1, "AES");
        Cipher v6_2 = Cipher.getInstance("AES/CBC/PKCS5Padding");
        v6_2.init(1, v5_2, v7);
        byte[] v0_2 = v6_2.doFinal(v0_1);
        if(MainActivity.v == 100) {
            Log.d("CipherAlgorithm", " Decrypt Cipher : " + new String(a.d(v1_3, v1_3, Base64.decode("zem8Qf+nUSVM8gsOgKiEeZ+OPRR9EKu76gEjg2eYf4MdXK9wAXrCeTQ9r1CpWcMu".getBytes("UTF-8"), 0)), "UTF-8"));
        }
        else {
            android.os.Process.killProcess(android.os.Process.myPid());
        }

        a.d("SOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEERSOJUBEER", v1_3, v0_2);
    }
```

### 4. Rooting Detection

a() 메소드 에서는 a.a.rootCheck() JNI(Java Native Interface) 함수를 호출하여 `Rooting` 여부를 확인 한 후 반환 값이 TRUE 일 경우 Rooting 된 Device로 간주 하고 finish() 함수를 통해 APP 을 종료 시키는 것을 확인할 수 있습니다.

![Untitled](/assets/2021-11-01/Untitled%204.png)

rootCheck 함수는 JNI(Java Native Interface) C,C++ 을 이용해 구현된 함수 이며, RootCheck 클래스 내에 구현되어 있는 것을 확인할 수 있습니다.

![Untitled](/assets/2021-11-01/Untitled%205.png)

libnative-lib.so 내의 a(), rootCheck() 함수를 분석하기 위해 IDA 를 통해 디컴파일한 결과 입니다. Pengsu apk 의 패키지 명으로 시작되는 함수로 a(), rootCheck() 함수가 존재하는 것을 확인할 수 있습니다.

![Untitled](/assets/2021-11-01/Untitled%206.png)

rootCheck() 함수를 살펴보면 access() 함수를 이용해 `Rooting` 과 관련된 파일이 존재하는지 여부를 체크 한 후 `Rooting`과 관련된 파일이 존재한다면 1을 Return 합니다. `Rooting` 이 되지 않은 Device 의 경우 일반적으로  su 바이너리 파일이 존재하지 않습니다.

![Untitled](/assets/2021-11-01/Untitled%207.png)

a() 함수는 문제가 해결 될 경우, FLAG의 문자열을 복호화 하기 위해 사용되는 AES Private Key 생성 로직 입니다. 

![Untitled](/assets/2021-11-01/Untitled%208.png)

### 5. Hooking with Frida

문제 풀이를 위해 아래와 같이 코드를 작성 할 수 있습니다. 문제 풀이를 위해 필요한 조건은 다음과 같습니다.

1. Rooting Detection Bypass
2. Pengsu Wallet 100 $
3. Get The FLAG

- 1번 조건은 `Rooting`우회를 위해 `Rooting`체크 함수인 rootCheck() JNI 함수의 반환 값을 강제로 0으로 반환 하는 스크립트 입니다.

- 2번 조건은 Pengsu Wallet 변수에 랜덤 값을 할당하여 점수를 부여하기 때문에 Random 클래스 객체의 랜덤 값 타입을 지정하는 메소드 인 nextInt() 메소드의 반환 값을 100으로 반환 하는 스크립트 입니다.

- 3번 조건은 해당 문제에서 FLAG 를 Android Log 를 이용하여 출력하기 때문에 이를 후킹하여 FLAG를 얻을 수 있습니다. Log 클래스의 debug 메소드의 인자를 후킹하는 코드 입니다.

> Frida hook code 1
> 

```jsx
setTimeout(function () {
    setImmediate(function () {
        Java.perform(function () {
						// 1. Rooting Detection Bypass
            const rootCheckPtr = Module.getExportByName('libnative-lib.so', 'Java_com_hspace_pengsu_RootCheck_rootCheck');
            Interceptor.replace(rootCheckPtr, new NativeCallback(function () {
                console.log('[*] Root Check Bypass');
                return 0;
            }, 'int', []));

						// 2. Pengsu Wallet 100 $
            var Random = Java.use("java.util.Random");
            Random.nextInt.overload('int').implementation = function (arg) {
                console.log("[*] Pengsu Wallet 100$ UP !");
                return 100;
            };

						// 3. GET THE FLAG
			      var Log = Java.use("android.util.Log");
			      Log.d.overload("java.lang.String", "java.lang.String").implementation = function(a, b) {
                console.log("[+] Log.d Hook : [" + a.toString() + "] " + b.toString());
                return this.d(a, b);
            };
        });
    });
}, 100);
```

스크립트 실행 후 앱 내에 Pengsu Wallet 의 금액이 100$ 로 변경 되고, `Rooting`이 우회 된 것을 확인할 수 있습니다.

![Untitled](/assets/2021-11-01/Untitled%209.png)

Frida 스크립트가 실행되고 `Rooting` 탐지 우회 후, FLAG 를 획득할 수 있습니다.

![Untitled](/assets/2021-11-01/Untitled%2010.png)

### 6. Hooking with Frida 2

두번째 풀이 방법 입니다. 프로세스  `Rooting` 탐지 시 조건문으로 분기 하여 Log 를 출력 한 후, finish() 함수를 호출 합니다. finish() 함수가 호출되면 앱이 종료 됩니다. finish() 함수 호출을 무력화 한다면 프로세스는 계속 실행 될 것입니다.

![Untitled](/assets/2021-11-01/Untitled%2011.png)

> Frida hook code 2
> 

```jsx
setTimeout(function() {
    setImmediate(function() {
        Java.perform(function() {
            var Activity = Java.use("android.app.Activity");
            Activity.finish.overload('int').implementation = function(arg) {
                console.log(" [*] Finish Function Hook ! ");
            }
            var Random = Java.use("java.util.Random");
            Random.nextInt.overload('int').implementation = function(arg) {
                console.log("[*] Pengsu Wallet 100$ UP !");
                return 100;
            };
            var Log = Java.use("android.util.Log");
            Log.d.overload("java.lang.String", "java.lang.String").implementation = function(a, b) {
                console.log("[+] Log.d Hook : [" + a.toString() + "] " + b.toString());
                return this.d(a, b);
            };
        });
    });
}, 100);
```

Frida 스크립트가 실행되면 `Rooting`이 탐지 되었음에도 불구하고 앱이 종료되지 않고, FLAG 를 획득할 수 있습니다.

![Untitled](/assets/2021-11-01/Untitled%2012.png)

위와 같은 케이스는 Real World 에서도 종종 발견되곤 합니다. 복잡한 `Rooting` 탐지와 관련된 로직을 분석하고, 찾을 필요 없이 `Rooting` 탐지 후, 어떤 이벤트가 발생하는지 알 수 있다면 이와 같은 방법으로 손쉽게 우회할 수 있습니다.