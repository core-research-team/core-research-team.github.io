---
layout: post
title: "Use external native library for Android"
author: "gpsfly"
comments: true
tags: [mobile, android]
---

라온화이트햇 핵심연구팀 김동민

# 개요

외부에서 추출 및 획득한 Android Native Library(.so) 파일을 분석하기 위해서는 IDA 및 frida 등 분석도구들을 활용하여 분석하는 경우가 일반적입니다. 하지만 본 연구에서는 기존 분석도구를 사용하지 않고 JNI(Java Native Interface)의 특성을 활용한 APK 컴파일을 통해 해당 Native Library 를 분석하고자 합니다.

# 기존 분석방안

- 공통

일반적으로 Android Native Library 를 추출하기 위해서는 먼저 해당 APK를 먼저 획득해야 합니다. 플레이 스토어 및 웹사이트를 통해 APK를 다운로드 받고 리패키징 도구인 apktool 을 통해 압축을 해제 합니다.

다음으로, 압축을 해제한 APK의 lib(s) 디렉토리를 확인해보면 다수의 Native Library(.so) 파일들이 존재하는 것을 확인할 수 있습니다. 이 중 분석하고자하는 파일을 선택합니다.

 

이후, 아래 두가지의 분석도구를 통해 분석합니다.

- IDA Pro(정적분석)

해당 Native Library(.so)는 Android NDK 를 통해 C/C++ 언어로 개발되었기 때문에 IDA Pro 를 통하여 정적분석이 가능합니다. 그리고 해당 Native Library 는 (Java 로 개발된)Dex 내의 JNI(Java Native Interface)를 통해 호출 되어지는데 아래와 같은 규칙성을 가집니다. 따라서 해당 함수들을 통해 전체적인 흐름을 파악할 수 있습니다.

`Java_{패키지명}_{메소드명}`

![/assets/2021-04-01/Use external native library for Android/Untitled.png](/assets/2021-04-01/Use external native library for Android/Untitled.png)

- frida(동적분석)

다음으로 동적분석 도구인 frida 를 사용하여 분석하는데, frida는 DBI(Dynamic Binary Instrumentation) 및 Stalker의 일종입니다. 해당 도구는 특히 후킹에 최적화 되어있어 동적분석에 아주 유용한데 아래와 같은 후킹 스크립트(Javascript)를 통해 분석하고자 하는 메소드를 후킹하여 인자 정보 및 리턴 정보를 획득하여 분석에 큰 도움을 줄 수 있습니다.

```jsx
Interceptor.attach(Module.findExportByName("libtest_native.so", "Java_com_package_name_stringFromJNI"), {
	onEnter: function (args) {
		console.log(JSON.stringify(this.context));
	}, onLeave: function (retval) {
		console.log(retval.readCString());
	}
}
```

# APK 컴파일을 통한 행위 분석

필자는 APK를 개발하고 컴파일하여 추출한 Native Library(.so)를 분석하기 위해 아래와 같이 준비하고 진행하였습니다.

1. 분석한 Native Library 준비
2. Android Studio 설치
3. strings 바이너리
4. 단말기 준비(or 에뮬레이터)

먼저, Android Studio를 설치하고 첫 실행 이후 하단 Configure 의 SDK Manager를 클릭하면 [Android SDK]-[SDK Tools]에서 Android SDK Platform-Tools 를 체크하여 설치해 줍니다.

![/assets/2021-04-01/Use external native library for Android/Untitled%201.png](/assets/2021-04-01/Use external native library for Android/Untitled%201.png)

다음으로, 분석할 Native Library를 strings를 통해 패키지명 및 메소드명을 찾습니다.

```bash
# strings libtest_native.so|grep ^Java
Java_com_aeonlucid_nativetesting_MainActivity_stringFromJNI
```

위에서 획득한 패키지명을 통해 같은 이름의 프로젝트를 생성합니다.

- 패키지명 : com.aeonlucid.nativetesting
- 호출한지점 : MainActivity
- 메소드명 : stringFromJNI

이후, [MainActivity.java](http://mainactivity.java) 에서 Native Library 를 로드하고 JNI 를 선언하여 onCreate 안에서 호출합니다. 필자는 activity에 txt_result라는 TextView를 생성하여 실행결과를 해당 TextView로 보여줍니다. 소스코드는 아래와 같습니다.

```java
package com.aeonlucid.nativetesting;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {
    private TextView txt_result;

    static {
        System.loadLibrary("test_native");
    }

    public native String stringFromJNI();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        txt_result = (TextView)findViewById(R.id.txt_result);

        txt_result.setText(stringFromJNI());
    }
}
```

다음으로, 추출한 Native Library를 해당 프로젝트 내에 복사해야하는데 필자는 {project_path}/main/libs/armeabi-v7a 디렉토리를 생성하고 해당 디렉토리 내에 복사하였습니다. (armeabi-v7a 디렉토리 명은 필자가 분석하려는 Native Library가  arm32bit로 빌드되어 있었기 때문입니다)

![/assets/2021-04-01/Use external native library for Android/Untitled%202.png](/assets/2021-04-01/Use external native library for Android/Untitled%202.png)

마지막으로 build.gradle을 수정해야하는데 각각의 의미는 아래와 같습니다.

```java
plugins {
    id 'com.android.application'
}

android {
    compileSdkVersion 30
    buildToolsVersion "30.0.2"

    defaultConfig {
        applicationId "com.aeonlucid.nativetesting"
        minSdkVersion 28
        targetSdkVersion 30
        versionCode 1
        versionName "1.0"

        testInstrumentationRunner "androidx.test.runner.AndroidJUnitRunner"

				// [start edit point]----------------------------------------------------
        ndk{
            abiFilters "armeabi-v7a" // arm32 bit
            moduleName "test_native" // native library name
        }
        sourceSets.main{
            jni.srcDirs = []
            jniLibs.srcDir "src/main/libs" // native library path
        }
				// [end edit point]----------------------------------------------------
    }

    buildTypes {
        release {
            minifyEnabled false
            proguardFiles getDefaultProguardFile('proguard-android-optimize.txt'), 'proguard-rules.pro'
        }
    }
    compileOptions {
        sourceCompatibility JavaVersion.VERSION_1_8
        targetCompatibility JavaVersion.VERSION_1_8
    }
}

dependencies {

    implementation 'androidx.appcompat:appcompat:1.2.0'
    implementation 'com.google.android.material:material:1.3.0'
    implementation 'androidx.constraintlayout:constraintlayout:2.0.4'
    testImplementation 'junit:junit:4.+'
    androidTestImplementation 'androidx.test.ext:junit:1.1.2'
    androidTestImplementation 'androidx.test.espresso:espresso-core:3.3.0'
}
```

# 결과

APK를 빌드 이후 실행결과는 아래와 같습니다.

![/assets/2021-04-01/Use external native library for Android/Untitled%203.png](/assets/2021-04-01/Use external native library for Android/Untitled%203.png)

위 그림과 같이 해당 Native Library에서 실행된 결과가 화면에 노출(`Hello from C++`)되는 것을 확인할 수 있습니다. 또한 TextView 출력 뿐만 아니라 Log 등을 통해 인자정보 및 리턴정보를 여러가지 방안으로 분석할 수 있습니다.

# 마치며

해당 연구를 통해 기존의 분석도구가 아닌 일종의 편법(?)으로 분석하는 방안을 제시 하였는데, 해당 방법으로도 분석을 진행할 순 있지만 연구진행 도중 인자를 필요로하는 메소드의 경우 인자 타입과 같은 정보를 기존의 분석도구를 통해 확인 해야했습니다. 따라서 기존의 분석도구와 함께 사용한다면 원하는 루틴을 더욱 빠르게 분석할 수 있습니다.