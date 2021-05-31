---
layout: post
title: "Universal Android Debugging Detection and Bypass"
author: "vulhack"
comments: true
tags: [frida, android, pentest]
---

라온화이트햇 핵심연구팀 김현수

## 1. 개요

해당 문서는 **Android Application** 분석을 위해 **불가피** 하게 PC ↔  Android Device 간의 USB연결을 필요로 하는 경우, USB Debug 모드를 허용해야 하는 경우 **Android Application** 에서 `Detection` 하는 방법과 `Bypass` 방법에 대해 기술합니다.

### 1.1 **USB Connection**

USB 연결 상태가 되어 있고 `USB Debuge Mode` 을 허용하는 경우에는 `USB Debugging` 을 수행할수 있습니다. 이때문에 **강력한 보안** 이 요구되는 **금융앱** 이나 **모바일 보안 솔루션** 에서는 USB 연결  상태를 확인합니다.

### 1.2 **USB Debugging**

`USB Debugging` 모드가 활성화 되어 있는 경우 `ADB` 같은 `Device Manager` 를 통해 `log` 확인, `debug` 등 **Android Application**에서 다양한 정보를 획득 할 수 있기때문에 **강력한 보안**이 요구되는 **금융앱**이나 **모바일 보안 솔루션** 에서는 위와 같은 설정 사항을 확인합니다.

## 2. **ADB**

`ADB` 는 Android Device 나 Android Amulator 를 연결해주는 `명령줄 도구` 입니다. `ADB` 를 활용하여 분석하려고 하는  **Android Appplication** 에 대한 다양한 정보를 획득할 수 있습니다. 이때문에 **강력한 보안** 이 요구되는 **금융앱** 이나 **모바일 보안 솔루션** 에서는 `ADB`  프로세스  활성화 여부 등을 확인합니다.

즉, 보안이 요구되는 **Android Appplication** 의 경우  USB 연결이 되어있거나, `USB Debugging`  모드가 **활성화** 되어 있다면 이를 탐지하고 종료하거나 **Android Appplication** 기능에 제한을 두는 등의 조치를 취하게 됩니다. 이를 **Android Appplication** 에서 어떻게 탐지하고 어떻게 우회할 것인지에 대해 살펴 보겠습니다.

**Android Device USB Connect**

※ 물리적인 USB 케이블 연결 이외에도 Wifi를 이용한 무선 연결 및 네트워크를 통한 연결이 가능합니다.

![/assets/2021-05-01/Android_1.png](/assets/2021-05-01/Android_1.png)

`Android Device` 에서 `PC` 와 `USB` 연결을 하는 경우 위와 같이 "**접근 권한 허용**" 을 통해 `Android Device` 내에 접근하여 `Application Debugging`, `Application Logging` 등 다양한 작업이 가능합니다.

![/assets/2021-05-01/Android_2.png](/assets/2021-05-01/Android_2.png)

`Android Device` 를 통해 `개발자 옵션` 에 접근하면 `USB Debugging`  모드를 **활성화** 할 수 있습니다. 

![/assets/2021-05-01/Android_3.png](/assets/2021-05-01/Android_3.png)

`USB Debugging` 을 활성화 하지 않고 Frida 를 attach 할 경우 위와 같이 "Waiting for USB device to appear" 를 출력하고 Frida와의 연결이 되지 않습니다.

### **2.1 adb**

![/assets/2021-05-01/Android_4.png](/assets/2021-05-01/Android_4.png)

USB 연결이 완료되면 위와 같이 `adb devices`  명령을 통해 PC 와 연결된 `Android Device`를 확인할 수 있습니다.

### **2.2 adb shell**

![/assets/2021-05-01/Android_5.png](/assets/2021-05-01/Android_5.png)

정상적으로 `Android Device` 목록이 나타나면 `adb shell` 을 통해 `Android Device` 내 장치내에 접근하여 다양한 명령 수행과 `Frida` 를 이용한 `Hooking` 이 가능합니다. 

### **2.3 logcat**

![/assets/2021-05-01/Android_6.png](/assets/2021-05-01/Android_6.png)

`logcat` 명령을 통해 앱 분석에 필요한 다양한 정보를 획득할 수 있습니다.

위와 같이 `USB Debugging` 을 통해 `Android Application` 분석과 `Frida` 와 같은 도구를 활용할 수 있습니다. 물론 `USB 연결`을 하지 않고 네트워크를 통해 `Android Application` 분석과 `Frida` 활용이 가능합니다.

해당 문서에서는 불가피 하게 `USB연결`을 사용해야 하는 상황을 가정합니다. `USB Debugging` 모드 **활성화** 여부를 `Detection` 하는 `Android API` 와 **구현** 에 대해 알아보겠습니다.

## 3. **Android USB Debugging 탐지 구현**

안드로이드 앱에서 시스템 기본 설정이 포함 된 보안 시스템 설정 여부를 확인하는 **Class** 는 다음과 같습니다.

**API 16이하를 사용하는 경우에는  `public static final class Settings.Secure`**

**API 17이상을 사용하는 경우에는  `public static final class Settings.Global`**

**Java Class**

```java
/* API 16이하 */
public static final class Settings.Secure
extends Settings.NameValueTable

java.lang.Object
   ↳	android.provider.Settings.NameValueTable
 	   ↳	android.provider.Settings.Secure

/* API 17이상 */
public static final class Settings.Global
extends Settings.NameValueTable

java.lang.Object
   ↳	android.provider.Settings.NameValueTable
 	   ↳	android.provider.Settings.Global
```

`Secure`, `Global` 클래스에 정의되어 있는 상수 값 `ADB_ENABLE` 을 통해 `USB Debugging` 모드 **활성화** 여부를 확인할 수 있습니다.

자세한 내용은 안드로이드 개발자 공식 홈페이지에 정의되어 있습니다.

[https://developer.android.com/reference/android/provider/Settings.Global](https://developer.android.com/reference/android/provider/Settings.Global) 

[https://developer.android.com/reference/android/provider/Settings.Secure](https://developer.android.com/reference/android/provider/Settings.Secure)

`Android Device` 에 설정 되어 있는 값을 확인하기 위한 상수 표는 아래와 같습니다. 

```java
/*
상수 list
*/

String	ADB_ENABLED
USB를 통한 ADB 활성화 여부.

String	AIRPLANE_MODE_ON
비행기 모드가 켜져 있는지 여부.

String	AIRPLANE_MODE_RADIOS
비행기 모드가 켜져있을 때 비활성화해야하는 쉼표로 구분 된 라디오 목록입니다.

String	ALWAYS_FINISH_ACTIVITIES
0이 아닌 경우 활동 관리자는 더 이상 필요하지 않은 즉시 활동 및 프로세스를 적극적으로 완료합니다.

String	ANIMATOR_DURATION_SCALE
Animator 기반 애니메이션의 배율입니다.

String	APPLY_RAMPING_RINGER
수신 전화 벨소리에 램핑 벨소리를 적용할지 여부.

String	AUTO_TIME
사용자가 네트워크 (NITZ)에서 자동으로 가져 오는 날짜, 시간 및 시간대를 선호하는지 여부를 지정하는 값입니다.

String	AUTO_TIME_ZONE
사용자가 네트워크 (NITZ)에서 시간대를 자동으로 가져 오는 것을 선호하는지 여부를 지정하는 값입니다.

String	BLUETOOTH_ON
블루투스 활성화 / 비활성화 여부 0 = 비활성화.

String	BOOT_COUNT
기기가 API 레벨 24를 실행하기 시작한 이후의 부팅 수입니다.

String	CONTACT_METADATA_SYNC_ENABLED
연락처 메타 데이터 동기화 활성화 여부 값 1-활성화, 0-비활성화

String	DATA_ROAMING
데이터 로밍 사용 여부입니다.

String	DEBUG_APP
디버깅 할 애플리케이션 패키지의 이름입니다.

String	DEVELOPMENT_SETTINGS_ENABLED
사용자가 개발 설정을 활성화했는지 여부.

String	DEVICE_NAME
장치 이름

String	DEVICE_PROVISIONED
장치가 프로비저닝되었는지 여부 (0 = 거짓, 1 = 참).

String	HTTP_PROXY
글로벌 http 프록시의 호스트 이름 및 포트.

String	INSTALL_NON_MARKET_APPS
이 상수는 API 레벨 21에서 더 이상 사용되지 않습니다. Settings.Secure.INSTALL_NON_MARKET_APPS대신 사용하십시오.

String	MODE_RINGER
벨소리 모드.

String	NETWORK_PREFERENCE
사용할 네트워크에 대한 사용자 기본 설정입니다.

String	RADIO_BLUETOOTH
블루투스 라디오를 지정하기 위해 AIRPLANE_MODE_RADIOS에서 사용하는 상수입니다.

String	RADIO_CELL
셀룰러 라디오를 지정하기 위해 AIRPLANE_MODE_RADIOS에서 사용하는 상수입니다.

String	RADIO_NFC
NFC 라디오를 지정하기 위해 AIRPLANE_MODE_RADIOS에서 사용하는 상수입니다.

String	RADIO_WIFI
AIRPLANE_MODE_RADIOS에서 Wi-Fi 라디오를 지정하기 위해 사용하는 상수입니다.

String	SHOW_PROCESSES
이 상수는 API 레벨 25에서 더 이상 사용되지 않습니다.이 기능은에서 더 이상 사용할 수 없습니다 Build.VERSION_CODES.N_MR1.

String	STAY_ON_WHILE_PLUGGED_IN
장치가 연결되어있는 동안 장치를 계속 켜져 있는지 여부.

String	TRANSITION_ANIMATION_SCALE
활동 전환 애니메이션에 대한 확장 요인.

String	USB_MASS_STORAGE_ENABLED
USB 대용량 저장소 활성화

String	USE_GOOGLE_MAIL
이 설정을 (무엇이든) 설정하면 기기의 Gmail에 대한 모든 참조가 Google Mail로 변경되어야합니다.

String	WAIT_FOR_DEBUGGER
1이면 DEBUG_APP를 시작할 때 사용자 코드를 시작하기 전에 디버거를 기다립니다.

String	WIFI_DEVICE_OWNER_CONFIGS_LOCKDOWN
이 설정은 기기 소유자 앱에서 생성 한 Wi-Fi 구성을 잠 가야하는지 여부를 제어합니다 (즉, 설정 앱이 아닌 기기 소유자 앱에서만 수정 또는 제거 가능).

String	WIFI_MAX_DHCP_RETRY_COUNT
DHCP에서 IP 주소를 가져 오지 못한 액세스 포인트에 대한 연결을 재 시도하는 최대 횟수입니다.

String	WIFI_MOBILE_DATA_TRANSITION_WAKELOCK_TIMEOUT_MS
Wi-Fi 연결이 끊어진 후 모바일 데이터 연결이 설정 될 때까지 대기하는 동안 wakelock을 유지하는 최대 시간 (밀리 초)입니다.

String	WIFI_ON
Wi-Fi가 켜져 있어야하는지 여부.

String	WIFI_WATCHDOG_ON
Wi-Fi 감시가 활성화되었는지 여부.

String	WINDOW_ANIMATION_SCALE
일반 창 애니메이션의 배율입니다.
```

위와 같이 정의되어 있는 상수 값을 가져와  `활성화` 여부 등을 확인할 수 있습니다.

![/assets/2021-05-01/Android_7.png](/assets/2021-05-01/Android_7.png)

`ADB_ENABLE` 상수의 단일 보안 설정 값을 가져오는 방법은 아래의 Method 를 통해 가져올 수 있습니다.

**Java Method**

```java
public static int getInt (ContentResolver cr, String name, int def)
public static int getInt (ContentResolver cr, String name)

public static float getFloat(ContentResolver cr, String name, float def)
public static float getFloat(ContentResolver cr, String name)

public static long getLong (ContentResolver cr, String name)
public static long getLong (ContentResolver cr, String name, long def)

public static String getString (ContentResolver resolver, String name)

public static Uri getUriFor (String name)
```

Overloading Method 중 2개의 인자를 받는 Method 는 기본 값을 사용하지 않습니다. **'def'** 이 설정되지 않았거나 문자열 값이 반환 형과 다른 경우 "**SettingNotSettingNotFoundException"** 을 발생 시킵니다. 

Overloading Method  중 3개의 인자를 받는 Method 는 기본 값을 지정하여 **유용한 반환 값이 아닌 경우** **지정한 값(int def)을 반환** 합니다.

API 버전에 따른 `USB Debugging` 모드를  탐지하는 방법에 대해 소개하겠습니다.

### 3.1 **API 16이하를 사용하는 경우**

**`Settings.Secure Class`  사용**

```java
/* 
API16 이상 USB Debug 활성화 상태 탐지
public static final class Settings.Secure
*/
if (Build.VERSION.SDK_INT <= 16) {
    Log.d(TAG, "Settings.Secure 사용 ");

    int adbCheck_int = Settings.Secure.getInt(getContentResolver(), Settings.Secure.ADB_ENABLED, 0);
    float adbCheck_float = Settings.Secure.getFloat(getContentResolver(), Settings.Secure.ADB_ENABLED, 0);
    long adbCheck_long = Settings.Secure.getLong(getContentResolver(), Settings.Secure.ADB_ENABLED, 0);
    String adbCheck_String = Settings.Secure.getString(getContentResolver(), Settings.Secure.ADB_ENABLED);

    Log.d(TAG, "adbCheck : " + adbCheck_int);
    Log.d(TAG, "adbCheck_float : " + adbCheck_float);
    Log.d(TAG, "adbCheck_long : " + adbCheck_long);
    Log.d(TAG, "adbCheck_String : " + adbCheck_String);

    if (adbCheck_int != 0 || adbCheck_float != 0 || adbCheck_long != 0 || !adbCheck_String.equals("0")) {
        Toast.makeText(this.getApplicationContext(), "USB 디버깅 모드가 탐지 되었습니다.", Toast.LENGTH_SHORT).show();
        Toast.makeText(this.getApplicationContext(), "int : " + adbCheck_int + " float : " + adbCheck_float + " long : " + adbCheck_long + " string : " + adbCheck_String, Toast.LENGTH_SHORT).show();
    } 
    else {
        Toast.makeText(this.getApplicationContext(), "USB 디버깅 모드가 탐지되지 않았습니다.", Toast.LENGTH_SHORT).show();
    }
}

/*Debug Log 
	D/ADBCheck: adbCheck_int : 1
	D/ADBCheck: adbCheck_float : 1.0
	D/ADBCheck: adbCheck_long : 1
	D/ADBCheck: adbCheck_String : 1
*/
```

### 3.2 **API 17이상을 사용하는 경우**

**`Settings.Global Class` 사용**

```java
/* 
API17 이상 USB Debug 활성화 상태 탐지
public static final class Settings.Global
*/

if (Build.VERSION.SDK_INT >= 17) { //API Level 17 이상일 경우 Global
    Log.d(TAG, "Settings.Global 사용 ");

    int adbCheck_int = Settings.Global.getInt(getContentResolver(), Settings.Global.ADB_ENABLED, 0);
    float adbCheck_float = Settings.Global.getFloat(getContentResolver(), Settings.Global.ADB_ENABLED, 0);
    long adbCheck_long = Settings.Global.getLong(getContentResolver(), Settings.Global.ADB_ENABLED, 0);
    String adbCheck_String = Settings.Global.getString(getContentResolver(), Settings.Global.ADB_ENABLED);

    Log.d(TAG, "adbCheck_int : " + adbCheck_int);
    Log.d(TAG, "adbCheck_float : " + adbCheck_float);
    Log.d(TAG, "adbCheck_long : " + adbCheck_long);
    Log.d(TAG, "adbCheck_String : " + adbCheck_String);

    if (adbCheck_int != 0 || adbCheck_float != 0 || adbCheck_long != 0 || !adbCheck_String.equals("0")) {
        Toast.makeText(this.getApplicationContext(), "USB 디버깅 모드가 탐지 되었습니다.", Toast.LENGTH_SHORT).show();
        Toast.makeText(this.getApplicationContext(), "int : " + adbCheck_int + " float : " + adbCheck_float + " long : " + adbCheck_long + " string : " + adbCheck_String, Toast.LENGTH_SHORT).show();
    } 
    else {
        Toast.makeText(this.getApplicationContext(), "USB 디버깅 모드가 탐지되지 않았습니다.", Toast.LENGTH_SHORT).show();
    }
}

/*Debug Log 
	D/ADBCheck: adbCheck_int : 1
	D/ADBCheck: adbCheck_float : 1.0
	D/ADBCheck: adbCheck_long : 1
	D/ADBCheck: adbCheck_String : 1
*/
```

### 3.3 **USB Debugging 모드를 탐지하는 `usbDebugCheck()` 함수 구현**

```java
public boolean **usbDebugCheck()**{
        Log.d(TAG,"SDK Version : " + Build.VERSION.SDK_INT);

        if(Build.VERSION.SDK_INT <= 16){
            Log.d(TAG,"Settings.Secure 사용 ");

            int adbCheck_int = Settings.Secure.getInt(getContentResolver(), Settings.Secure.ADB_ENABLED, 0);
            float adbCheck_float = Settings.Secure.getFloat(getContentResolver(), Settings.Secure.ADB_ENABLED, 0);
            long adbCheck_long = Settings.Secure.getLong(getContentResolver(), Settings.Secure.ADB_ENABLED, 0);
            String adbCheck_String = Settings.Secure.getString(getContentResolver(), Settings.Secure.ADB_ENABLED);

            Log.d(TAG,"adbCheck : " + adbCheck_int);
            Log.d(TAG,"adbCheck_float : " + adbCheck_float);
            Log.d(TAG,"adbCheck_long : " + adbCheck_long);
            Log.d(TAG,"adbCheck_String : " + adbCheck_String);

            if(adbCheck_int!= 0 || adbCheck_float != 0 || adbCheck_long != 0 || !adbCheck_String.equals("0")){
                Toast.makeText(this.getApplicationContext(),"USB 디버깅 모드가 탐지 되었습니다.", Toast.LENGTH_SHORT).show();
                Toast.makeText(this.getApplicationContext(),"int : "+ adbCheck_int + " float : " + adbCheck_float + " long : " + adbCheck_long + " string : "  + adbCheck_String , Toast.LENGTH_SHORT).show();
                return true;
            }
            else {
                Toast.makeText(this.getApplicationContext(),"USB 디버깅 모드가 탐지되지 않았습니다.", Toast.LENGTH_SHORT).show();
                return false;
            }
        }
        else if(Build.VERSION.SDK_INT >= 17){
            Log.d(TAG,"Settings.Global 사용 ");

            int adbCheck_int = Settings.Global.getInt(getContentResolver(), Settings.Global.ADB_ENABLED, 0);
            float adbCheck_float = Settings.Global.getFloat(getContentResolver(), Settings.Global.ADB_ENABLED, 0);
            long adbCheck_long = Settings.Global.getLong(getContentResolver(), Settings.Global.ADB_ENABLED, 0);
            String adbCheck_String = Settings.Global.getString(getContentResolver(), Settings.Global.ADB_ENABLED);

            Log.d(TAG,"adbCheck_int : " + adbCheck_int);
            Log.d(TAG,"adbCheck_float : " + adbCheck_float);
            Log.d(TAG,"adbCheck_long : " + adbCheck_long);
            Log.d(TAG,"adbCheck_String : " + adbCheck_String);

            if(adbCheck_int != 0 || adbCheck_float != 0 || adbCheck_long != 0 || !adbCheck_String.equals("0")){
                Toast.makeText(this.getApplicationContext(),"USB 디버깅 모드가 탐지 되었습니다.", Toast.LENGTH_SHORT).show();
                Toast.makeText(this.getApplicationContext(),"int : "+ adbCheck_int + " float : " + adbCheck_float + " long : " + adbCheck_long + " string : "  + adbCheck_String , Toast.LENGTH_SHORT).show();
                return true;
            }
            else{
                Toast.makeText(this.getApplicationContext(),"USB 디버깅 모드가 탐지되지 않았습니다.", Toast.LENGTH_SHORT).show();
                return false;
            }
        }
        return false;
    }

/*Debug Log
	D/ADBCheck: SDK Version : 29
	D/ADBCheck: Settings.Global 사용 
	D/ADBCheck: adbCheck_int : 1
	D/ADBCheck: adbCheck_float : 1.0
	D/ADBCheck: adbCheck_long : 1
	D/ADBCheck: adbCheck_String : 1
	D/ADBCheck: result : true
*/
```

위의 코드와 같이 `USB Debugging` 모드를 탐지하기 위해 `Secure`, `Global` 등 `Class` 와 `getint`, `getString` 등의 `Method` 를 통해 `ADB_ENABLED` 의 상수 값을 판단 하여 `USB Debugging` 모드를 탐지 합니다.

아래의 내용은 `USB Debugging` **활성화** 여부 탐지를 테스트 하는 간단한 예제 입니다.

![/assets/2021-05-01/Android_8.png](/assets/2021-05-01/Android_8.png)

실제 `Android Device` (Galaxy S9)를 이용하여 `USB debugging` 모드를 **활성화** 합니다. 

![/assets/2021-05-01/Android_9.png](/assets/2021-05-01/Android_9.png)

앱 실행후에 정상적으로 "**USB  디버깅 모드가 탐지 되었습니다**" 라는 Toast 메세지가 출력됩니다. 

![/assets/2021-05-01/Android_10.png](/assets/2021-05-01/Android_10.png)

이후 각 getInt(), getFloat(), getLong(), getString() Method가 실행되고 `USB debugging` **활성화** 여부 탐지에 대한 return 값 이 출력됩니다.

![/assets/2021-05-01/Android_11.png](/assets/2021-05-01/Android_11.png)

다음은 `USB debugging` 모드를 **비활성화** 하고 앱을 실행 합니다.

![/assets/2021-05-01/Android_12.png](/assets/2021-05-01/Android_12.png)

`USB debugging` 모드를 **비활성화** 하였기 때문에 정상적으로 "**USB 디버깅 모드가 탐지되지 않았습니다**" Toast 메세지가 출력됩니다. 

## 4. **Android USB Debugging 탐지 우회**

앞서 `Android USB Debugging 탐지 구현` 에서 `USB debugging` 모드를 탐지하는 간단한 예를 살펴 보았습니다. 이제 이를 우회하는 Frida 스크립트에 대해 알아 보겠습니다.

**Java Method**

```java
public static int getInt (ContentResolver cr, String name, int def)
public static int getInt (ContentResolver cr, String name)

public static float getFloat(ContentResolver cr, String name, float def)
public static float getFloat(ContentResolver cr, String name)

public static long getLong (ContentResolver cr, String name)
public static long getLong (ContentResolver cr, String name, long def)

public static String getString (ContentResolver resolver, String name)
```

가장 간단한 방법은 위의 Method 를 후킹 하는 것 입니다. 위의 Method 를 통해 `ADB_ENABLE` 상수 값의 상태를 가져와서 `USB debugging` 모드 **활성화 여부** 를 판단하기 때문입니다.

앞서 `Android USB Debugging 탐지 구현` 에서 의 예제는 Overloading Method 의 인자가 3개인 Method 만 사용 하였지만, 실제 분석하고자 하는 `Android Application` 은 어떠한 Method 로 구현되어 있는지 예측할 수 없습니다.  

따라서 본 문서에서는 `ADB_ENABLE` 상수 값을 판별하는 모든 Method(Overloading Method를 포함) 후킹 스크립트를 작성합니다.

### 4.1 Universal **Android Debug mode bypass with Frida.js**

```jsx
Java.perform(function() {
    var androidSettings = ['adb_enabled'];
    var sdkVersion = Java.use('android.os.Build$VERSION');
    console.log("SDK Version : " + sdkVersion.SDK_INT.value);

    if (sdkVersion.SDK_INT.value <= 16) {
        var settingSecure = Java.use('android.provider.Settings$Secure');
        
        settingSecure.getInt.overload('android.content.ContentResolver', 'java.lang.String').implementation = function(cr, name) {
            if (name == androidSettings[0]) {
                console.log('[+]Secure.getInt(cr, name) Bypassed');
                return 0;
            }
            var ret = this.getInt(cr, name);
            return ret;
        }

        settingSecure.getInt.overload('android.content.ContentResolver', 'java.lang.String', 'int').implementation = function(cr, name, def) {
            if (name == (androidSettings[0])) {
                console.log('[+]Secure.getInt(cr, name, def) Bypassed');
                return 0;
            }
            var ret = this.getInt(cr, name, def);
            return ret;
        }

        settingSecure.getFloat.overload('android.content.ContentResolver', 'java.lang.String').implementation = function(cr, name) {
            if (name == androidSettings[0]) {
                console.log('[+]Secure.getFloat(cr, name) Bypassed');
                return 0;
            }
            var ret = this.getFloat(cr, name)
            return ret;
        }

        settingSecure.getFloat.overload('android.content.ContentResolver', 'java.lang.String', 'float').implementation = function(cr, name, def) {
            if (name == androidSettings[0]) {
                console.log('[+]Secure.getFloat(cr, name, def) Bypassed');
                return 0;
            }
            var ret = this.getFloat(cr, name, def);
            return ret;
        }

        settingSecure.getLong.overload('android.content.ContentResolver', 'java.lang.String').implementation = function(cr, name) {
            if (name == androidSettings[0]) {
                console.log('[+]Secure.getLong(cr, name) Bypassed');
                return 0;
            }
            var ret = this.getLong(cr, name)
            return ret;
        }

        settingSecure.getLong.overload('android.content.ContentResolver', 'java.lang.String', 'long').implementation = function(cr, name, def) {
            if (name == androidSettings[0]) {
                console.log('[+]Secure.getLong(cr, name, def) Bypassed');
                return 0;
            }
            var ret = this.getLong(cr, name, def);
            return ret;
        }

        settingSecure.getString.overload('android.content.ContentResolver', 'java.lang.String').implementation = function(cr, name) {
            if (name == androidSettings[0]) {
                var stringClass = Java.use("java.lang.String");
                var stringInstance = stringClass.$new("0");

                console.log('[+]Secure.getString(cr, name) Bypassed');
                return stringInstance;
            }
            var ret = this.getString(cr, name);
            return ret;
        }
    }

    /* API17이상 Settings.Global Hook */
    if (sdkVersion.SDK_INT.value >= 17) {
        var settingGlobal = Java.use('android.provider.Settings$Global');

        settingGlobal.getInt.overload('android.content.ContentResolver', 'java.lang.String').implementation = function(cr, name) {
            if (name == androidSettings[0]) {
                console.log('[+]Global.getInt(cr, name) Bypassed');
                return 0;
            }
            var ret = this.getInt(cr, name);
            return ret;
        }

        settingGlobal.getInt.overload('android.content.ContentResolver', 'java.lang.String', 'int').implementation = function(cr, name, def) {
            if (name == (androidSettings[0])) {
                console.log('[+]Global.getInt(cr, name, def) Bypassed');
                return 0;
            }
            var ret = this.getInt(cr, name, def);
            return ret;
        }

        settingGlobal.getFloat.overload('android.content.ContentResolver', 'java.lang.String').implementation = function(cr, name) {
            if (name == androidSettings[0]) {
                console.log('[+]Global.getFloat(cr, name) Bypassed');
                return 0;
            }
            var ret = this.getFloat(cr, name);
            return ret;
        }

        settingGlobal.getFloat.overload('android.content.ContentResolver', 'java.lang.String', 'float').implementation = function(cr, name, def) {
            if (name == androidSettings[0]) {
                console.log('[+]Global.getFloat(cr, name, def) Bypassed');
                return 0;
            }
            var ret = this.getFloat(cr, name, def);
            return ret;
        }

        settingGlobal.getLong.overload('android.content.ContentResolver', 'java.lang.String').implementation = function(cr, name) {
            if (name == androidSettings[0]) {
                console.log('[+]Global.getLong(cr, name) Bypassed');
                return 0;
            }
            var ret = this.getLong(cr, name)
            return ret;
        }

        settingGlobal.getLong.overload('android.content.ContentResolver', 'java.lang.String', 'long').implementation = function(cr, name, def) {
            if (name == androidSettings[0]) {
                console.log('[+]Global.getLong(cr, name, def) Bypassed');
                return 0;
            }
            var ret = this.getLong(cr, name, def);
            return ret;
        }
        
        settingGlobal.getString.overload('android.content.ContentResolver', 'java.lang.String').implementation = function(cr, name) {
            if (name == androidSettings[0]) {
                var stringClass = Java.use("java.lang.String");
                var stringInstance = stringClass.$new("0");

                console.log('[+]Global.getString(cr, name) Bypassed');
                return stringInstance;
            }
            var ret = this.getString(cr, name);
            return ret;
        }
    }
});
```

다소 무식한 방법입니다. 하지만 Universal 한 Code 로 충분하리라 생각됩니다. 개발자가 어떠한 Method를 통해 `ADB_ENABLE` 활성화 여부를 체크 할지 모르기 때문입니다. 

**실행 결과**

![/assets/2021-05-01/Android_13.png](/assets/2021-05-01/Android_13.png)

정상적으로 스크립트가 실행되는 것을 확인할 수 있습니다.

![/assets/2021-05-01/Android_12.png](/assets/2021-05-01/Android_12.png)

스크립트가 실행된 후, 정상적으로 `USB debugging` 활성화 모드가 **우회** 되는것을 확인할 수 있습니다.

## 5. 마치며

위와 같은 방법으로 `USB Debugging` 모드를 탐지(구현)하고, 우회 하는 방법에 대해 알아보았습니다. 유용하게 쓰일지는 모르겠으나, 조금이나마 도움이 되었으면 하는 바램입니다. 긴 글 읽어주셔서 감사합니다.