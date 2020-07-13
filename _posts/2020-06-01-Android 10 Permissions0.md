---
layout: post
title: "Android 10 Permissions"
author: "KuroNeko"
comments: true
tags: [android,mobile,analysis]
---

라온화이트햇 핵심연구팀 원요한([kuroneko9505@gmail.com](mailto:kuroneko9505@gmail.com))


## TL;DR

이 문서는 Android 10 버전부터 적용된 PermissionController의 동작 흐름을 따라가며, 원리를 파악하기 위해 작성되었습니다.

## Android Permission

안드로이드는 사용자로부터 퍼미션을 승인/거부 후, 해당 기능을 사용할 수 있습니다. 보통 퍼미션 요청을 보낼 때, 아래와 같은 코드를 사용해 사용자로부터 승인 여부를 기다립니다.

```kotlin
package com.example.testapp

import android.Manifest
import android.os.Bundle
import com.google.android.material.snackbar.Snackbar
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat

import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
    }
    
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        setSupportActionBar(toolbar)

        requestPermissions(arrayOf(
            Manifest.permission.CAMERA
        ), 1000)

    }

}
```

승인 여부는 해당 activity 클래스의 `onRequestPermissionsResult`함수를 통해 알 수 있습니다. 이 때 요청된 권한을 승인할 경우, 어떻게 처리가 되는지 살펴보겠습니다.

먼저, 구글링을 통해 Android 10 환경에서 permission을 어떤 구조로  동작하는 지 알 수 있었습니다.

![/assets/won60.png](/assets/won60.png)

설치된 사용자 앱에서 퍼미션 요청을 하게 되면, Android 10 부터는 `PermissionController 앱`, 그 이전 버전까지는 `Package Installer 앱`이 담당하여 System server로 전달하여 처리하는 것으로 보입니다. PermissionController는 다음과 같은 작업을 합니다.

![/assets/won61.png](/assets/won61.png)

위의 기능들은 PackageInstaller의 일부 기능 이였는데, Android 10으로 넘어오면서 분리된 앱으로 구성되어 있다는 것을 확인할 수 있었습니다. 대략적으로 큰 그림을 확인하였으니,  유저 앱에서  PermissionController로 요청할 때 사용되는 `requestPermissions` 의 코드를 확인하기 위해 해당 [링크](https://android.googlesource.com/platform/frameworks/support/+/4f9d8c4/v4/java/android/support/v4/app/ActivityCompat.java#308)를 참조하였습니다.

```java
// ActivityCompat
public static void requestPermissions(final @NonNull Activity activity,
      final @NonNull String[] permissions, final int requestCode) {
  // TODO: Change to comparison against API 23 once we have it defined.
  if (Build.VERSION.CODENAME.equals("MNC") || Build.VERSION.SDK_INT > 22) {
      ActivityCompatApi23.requestPermissions(activity, permissions, requestCode);
  } else if (activity instanceof OnRequestPermissionsResultCallback) {
      Handler handler = new Handler(Looper.getMainLooper());
      handler.post(new Runnable() {
          @Override
          public void run() {
              final int[] grantResults = new int[permissions.length];
              Arrays.fill(grantResults, PackageManager.PERMISSION_GRANTED);
              ((OnRequestPermissionsResultCallback) activity).onRequestPermissionsResult(
                      requestCode, permissions, grantResults);
          }
      });
  }
}
```

현재 SDK의 버전이 23 이상일 경우, `ActivityCompatApi23.requestPermissions` 를 호출하는 것을 볼 수 있습니다. 따라서 아래의 [코드](https://chromium.googlesource.com/android_tools/+/refs/heads/master/sdk/sources/android-25/android/support/v4/app/ActivityCompatApi23.java#39)를 살펴보아야 합니다.

```java
// ActivityCompatApi23
public static void requestPermissions(Activity activity, String[] permissions,
        int requestCode) {
    if (activity instanceof RequestPermissionsRequestCodeValidator) {
        ((RequestPermissionsRequestCodeValidator) activity)
                .validateRequestPermissionsRequestCode(requestCode);
    }
    activity.requestPermissions(permissions, requestCode);
}
```

해당 코드에서는 Activity클래스의 requestPermissions을 호출하는 것을 볼 수 있는데, 해당 [코드](https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/app/Activity.java#5094)는 다음과 같습니다.

```java
// Activity
public final void requestPermissions(@NonNull String[] permissions, int requestCode) {
    if (requestCode < 0) {
        throw new IllegalArgumentException("requestCode should be >= 0");
    }
    if (mHasCurrentPermissionsRequest) {
        Log.w(TAG, "Can request only one set of permissions at a time");
        // Dispatch the callback with empty arrays which means a cancellation.
        onRequestPermissionsResult(requestCode, new String[0], new int[0]);
        return;
    }
    Intent intent = getPackageManager().buildRequestPermissionsIntent(permissions);
    startActivityForResult(REQUEST_PERMISSIONS_WHO_PREFIX, intent, requestCode, null);
    mHasCurrentPermissionsRequest = true;
}
```

위의 코드에서 볼 수 있듯, 요청한 퍼미션을 Packagemanager클래스의 buildRequestPermissionsIntent함수를 통해 Intent로 전처리 작업을 거친 이후 Intent broadcast를 합니다. 해당 intent를 처리할 수 있는 앱이 받아서 처리한다는 이야기인데, 그 앱이 바로 PermissionController입니다. 따라서 해당 [링크](https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/AndroidManifest.xml#82)를 통해 `Intent-filter`를 살펴보았습니다.

```xml
<!-- PermissionController AndroidManifest.xml -->
<activity android:name="com.android.packageinstaller.permission.ui.GrantPermissionsActivity"
        android:configChanges="keyboardHidden|screenSize"
        android:excludeFromRecents="true"
        android:theme="@style/GrantPermissions"
        android:visibleToInstantApps="true"
        android:inheritShowWhenLocked="true">
    <intent-filter android:priority="1">
        <action android:name="android.content.pm.action.REQUEST_PERMISSIONS" />
        <category android:name="android.intent.category.DEFAULT" />
    </intent-filter>
</activity>
```

위의 intent-filter에서 퍼미션 요청을 받아서 처리하도록 되어있으므로, 해당 Activity 코드를 [아래](https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/src/com/android/packageinstaller/permission/ui/GrantPermissionsActivity.java#304)에서 살펴볼 수 있습니다.

```java
// GrantPermissionsActivity.onCreate ...
mCallingUid = callingPackageInfo.applicationInfo.uid;
UserHandle userHandle = UserHandle.getUserHandleForUid(mCallingUid);
if (DeviceUtils.isTelevision(this)) {
    mViewHandler = new com.android.packageinstaller.permission.ui.television
            .GrantPermissionsViewHandlerImpl(this,
            mCallingPackage).setResultListener(this);
} else if (DeviceUtils.isWear(this)) {
    mViewHandler = new GrantPermissionsWatchViewHandler(this).setResultListener(this);
} else if (DeviceUtils.isAuto(this)) {
    mViewHandler = new GrantPermissionsAutoViewHandler(this, mCallingPackage, userHandle)
            .setResultListener(this);
} else {
    mViewHandler = new com.android.packageinstaller.permission.ui.handheld
            .GrantPermissionsViewHandlerImpl(this, mCallingPackage, userHandle)
            .setResultListener(this);
}
// ...
```

 위의 코드에서 보이듯, GrantPermissionActivity를 호출한 패키지의 정보를 찾아 uid를 가져온 이후, 기기의 종류에 따라서 요청하는 핸들러가 달라지게 됩니다. 이후에 퍼미션 별 그룹화를 진행한 뒤, 사용자의 퍼미션 승인 여부의 결과를 처리하기 위해 `onPermissionGrantResult` 가 호출됩니다.

```java
public void onPermissionGrantResult(String name,
        @GrantPermissionsViewHandler.Result int result) {
    logGrantPermissionActivityButtons(name, result);
    GroupState foregroundGroupState = getForegroundGroupState(name);
    GroupState backgroundGroupState = getBackgroundGroupState(name);
    // ...
    switch (result) {
        case GRANTED_ALWAYS :
            if (foregroundGroupState != null) {
                onPermissionGrantResultSingleState(foregroundGroupState, true, false);
            }
            if (backgroundGroupState != null) {
                onPermissionGrantResultSingleState(backgroundGroupState, true, false);
            }
            break;
        case GRANTED_FOREGROUND_ONLY :
            if (foregroundGroupState != null) {
                onPermissionGrantResultSingleState(foregroundGroupState, true, false);
            }
            if (backgroundGroupState != null) {
                onPermissionGrantResultSingleState(backgroundGroupState, false, false);
            }
            break;
        case DENIED :
            if (foregroundGroupState != null) {
                onPermissionGrantResultSingleState(foregroundGroupState, false, false);
            }
            if (backgroundGroupState != null) {
                onPermissionGrantResultSingleState(backgroundGroupState, false, false);
            }
            break;
        case DENIED_DO_NOT_ASK_AGAIN :
            if (foregroundGroupState != null) {
                onPermissionGrantResultSingleState(foregroundGroupState, false, true);
            }
            if (backgroundGroupState != null) {
                onPermissionGrantResultSingleState(backgroundGroupState, false, true);
            }
            break;
    }
    if (!showNextPermissionGroupGrantRequest()) {
        setResultAndFinish();
    }
}
```

사용자가 선택한 결과에 의해 분기된 이후, 승인 또는 거부한 퍼미션 그룹의 결과를 `onPermissionGrantResultSingleState` 함수를 호출하여 처리하는 것을 볼 수 있습니다. 해당하는 함수는 다음과 같습니다.

```java
private void onPermissionGrantResultSingleState(GroupState groupState, boolean granted,
        boolean doNotAskAgain) {
    if (groupState != null && groupState.mGroup != null
            && groupState.mState == GroupState.STATE_UNKNOWN) {
        if (granted) {
            groupState.mGroup.grantRuntimePermissions(doNotAskAgain,
                    groupState.affectedPermissions);
            groupState.mState = GroupState.STATE_ALLOWED;
            reportRequestResult(groupState.affectedPermissions,
                    PERMISSION_GRANT_REQUEST_RESULT_REPORTED__RESULT__USER_GRANTED);
        } else {
            groupState.mGroup.revokeRuntimePermissions(doNotAskAgain,
                    groupState.affectedPermissions);
            groupState.mState = GroupState.STATE_DENIED;
            reportRequestResult(groupState.affectedPermissions, doNotAskAgain
                    ?
                    PERMISSION_GRANT_REQUEST_RESULT_REPORTED__RESULT__USER_DENIED_WITH_PREJUDICE
                    : PERMISSION_GRANT_REQUEST_RESULT_REPORTED__RESULT__USER_DENIED);
        }
    }
}
```

승인 또는 거부한 퍼미션을 각각 RuntimePermission을 설정해주는 것을 볼 수 있습니다. 해당 함수를  따라가게 되면, 이전에 처리한 퍼미션인지 또는 앱에서 설정된 퍼미션인지 등등 여러 조건들에 대해 검사를 진행합니다. 모든 조건을 거친 이후, `persistChanges` 함수를 호출하게 됩니다.

```java
void persistChanges(boolean mayKillBecauseOfAppOpsChange) {
    int uid = mPackageInfo.applicationInfo.uid;

    int numPermissions = mPermissions.size();
    boolean shouldKillApp = false;

    for (int i = 0; i < numPermissions; i++) {
        Permission permission = mPermissions.valueAt(i);

        if (!permission.isSystemFixed()) {
            if (permission.isGranted()) {
                mPackageManager.grantRuntimePermission(mPackageInfo.packageName,
                        permission.getName(), mUserHandle);
            } else {
                boolean isCurrentlyGranted = mContext.checkPermission(permission.getName(), -1,
                        uid) == PERMISSION_GRANTED;

                if (isCurrentlyGranted) {
                    mPackageManager.revokeRuntimePermission(mPackageInfo.packageName,
                            permission.getName(), mUserHandle);
                }
            }
        }
				// ...
		}
}
```

위의 함수에서는 PackageManager를 사용해 Runtime Permission을 변경합니다. PackageManager는  `ApplicationPackageManager` 클래스입니다.

```java
@Override
public void grantRuntimePermission(String packageName, String permissionName,
        UserHandle user) {
    try {
        mPM.grantRuntimePermission(packageName, permissionName, user.getIdentifier());
    } catch (RemoteException e) {
        throw e.rethrowFromSystemServer();
    }
}

@Override public void grantRuntimePermission(java.lang.String packageName, java.lang.String permissionName, int userId) throws android.os.RemoteException
{
  android.os.Parcel _data = android.os.Parcel.obtain();
  android.os.Parcel _reply = android.os.Parcel.obtain();
  try {
    _data.writeInterfaceToken(DESCRIPTOR);
    _data.writeString(packageName);
    _data.writeString(permissionName);
    _data.writeInt(userId);
    boolean _status = mRemote.transact(Stub.TRANSACTION_grantRuntimePermission, _data, _reply, 0);
    if (!_status && getDefaultImpl() != null) {
      getDefaultImpl().grantRuntimePermission(packageName, permissionName, userId);
      return;
    }
    _reply.readException();
  }
  finally {
    _reply.recycle();
    _data.recycle();
  }
}
```

위의 코드에서 볼 수 있듯, 인자로 받은 값들을 모두 직렬화한 이후 transact를 진행하게 됩니다. 이때 IPC통신으로 인자를 전달 후, 결과를 받아 이상이 없을 경우 정상적으로 반환되게 됩니다.  transact에 사용되는 `mRemote는 Service Binder`입니다. 해당 Binder를 통해 transact를 하게 되면 onTransact함수가 호출되며, 아래의 함수에서 처리하게 됩니다.

```java
// (PackageManagerService extends IPackageManager.Stub).onTransact
@Override
public boolean onTransact(int code, Parcel data, Parcel reply, int flags)
        throws RemoteException {
    try {
        return super.onTransact(code, data, reply, flags);
    } catch (RuntimeException e) {
        if (!(e instanceof SecurityException) && !(e instanceof IllegalArgumentException)) {
            Slog.wtf(TAG, "Package Manager Crash", e);
        }
        throw e;
    }
}

// IPackageManager.Stub.onTransact
@Override public boolean onTransact(int code, android.os.Parcel data, android.os.Parcel reply, int flags) throws android.os.RemoteException
{
  java.lang.String descriptor = DESCRIPTOR;
  switch (code)
  {
    case TRANSACTION_grantRuntimePermission:
    {
      data.enforceInterface(descriptor);
      java.lang.String _arg0;
      _arg0 = data.readString();
      java.lang.String _arg1;
      _arg1 = data.readString();
      int _arg2;
      _arg2 = data.readInt();
      this.grantRuntimePermission(_arg0, _arg1, _arg2);
      reply.writeNoException();
      return true;
    }
    case TRANSACTION_revokeRuntimePermission:
    {
      data.enforceInterface(descriptor);
      java.lang.String _arg0;
      _arg0 = data.readString();
      java.lang.String _arg1;
      _arg1 = data.readString();
      int _arg2;
      _arg2 = data.readInt();
      this.revokeRuntimePermission(_arg0, _arg1, _arg2);
      reply.writeNoException();
      return true;
    }
  }
}
```

Parcel을 역직렬화한 이후 this.grantRuntimePermission의 인자값으로 사용합니다. 해당 함수는 `PackageManagerService.grantRuntimePermission` 이며 아래와 같은 코드로 구성되어 있습니다.

```java
@Override
public void grantRuntimePermission(String packageName, String permName, final int userId) {
    boolean overridePolicy = (checkUidPermission(
            Manifest.permission.ADJUST_RUNTIME_PERMISSIONS_POLICY, Binder.getCallingUid())
            == PackageManager.PERMISSION_GRANTED);

    mPermissionManager.grantRuntimePermission(permName, packageName, overridePolicy,
            getCallingUid(), userId, mPermissionCallback);
}
```

먼저 요청한 Package의 uid를 구한 이후, 퍼미션 정책을 수정할 수 있는지에 대해서 검증을 진행합니다. 이후에 아래와 같이 `mPermissionManager.grantRuntimePermission` 함수를 통해 퍼미션을 승인합니다. 

```java
// mPermissionManager.grantRuntimePermission
// ...
final int result = permissionsState.grantRuntimePermission(bp, userId);
switch (result) {
    case PERMISSION_OPERATION_FAILURE: {
        return;
    }

    case PermissionsState.PERMISSION_OPERATION_SUCCESS_GIDS_CHANGED: {
        if (callback != null) {
            callback.onGidsChanged(UserHandle.getAppId(pkg.applicationInfo.uid), userId);
        }
    }
    break;
}
// ...
```

## Reference

**#0 permission request flow**

- [https://source.android.com/devices/tech/config](https://source.android.com/devices/tech/config)

**#1 ActivityCompat.requestPermissions**

- [https://android.googlesource.com/platform/frameworks/support/+/4f9d8c4/v4/java/android/support/v4/app/ActivityCompat.java#308](https://android.googlesource.com/platform/frameworks/support/+/4f9d8c4/v4/java/android/support/v4/app/ActivityCompat.java#308)

**#2 ActivityCompatApi23.requestPermissions**

- [https://chromium.googlesource.com/android_tools/+/refs/heads/master/sdk/sources/android-25/android/support/v4/app/ActivityCompatApi23.java#39](https://chromium.googlesource.com/android_tools/+/refs/heads/master/sdk/sources/android-25/android/support/v4/app/ActivityCompatApi23.java#39)

**#3 Activity.requestPermissions**

- [https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/app/Activity.java#5094](https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/app/Activity.java#5094)

**#4 PermissionController AndroidManifest.xml**

- [https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/AndroidManifest.xml#82](https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/AndroidManifest.xml#82)

**#5 GrantPermissionActivity.onCreate**

- [https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/src/com/android/packageinstaller/permission/ui/GrantPermissionsActivity.java#304](https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/src/com/android/packageinstaller/permission/ui/GrantPermissionsActivity.java#304)

**#6 GrantPermissionActivity.onPermissionGrantResult**

- [https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/src/com/android/packageinstaller/permission/ui/GrantPermissionsActivity.java#726](https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/src/com/android/packageinstaller/permission/ui/GrantPermissionsActivity.java#726)

**#7 GrantPermissionActivity.onPermissionGrantResultSingleState**

- [https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/src/com/android/packageinstaller/permission/ui/GrantPermissionsActivity.java#810](https://android.googlesource.com/platform/packages/apps/PackageInstaller/+/refs/heads/master/src/com/android/packageinstaller/permission/ui/GrantPermissionsActivity.java#810)

**#8 persistChanges**

- [https://cs.android.com/android/platform/superproject/+/master:packages/apps/PermissionController/src/com/android/packageinstaller/permission/model/AppPermissionGroup.java;l=1221?q=persistChanges &ss=android%2Fplatform%2Fsuperproject](https://cs.android.com/android/platform/superproject/+/master:packages/apps/PermissionController/src/com/android/packageinstaller/permission/model/AppPermissionGroup.java;l=1221?q=persistChanges%20&ss=android%2Fplatform%2Fsuperproject)

#9 **ApplicationPackageMangaer.grantRuntimePermission**

- [https://cs.android.com/android/platform/superproject/+/master:out/soong/.intermediates/frameworks/base/framework-minus-apex/android_common/xref28/srcjars.xref/frameworks/base/core/java/android/content/pm/IPackageManager.java;drc=master;bpv=1;bpt=1;l=5500](https://cs.android.com/android/platform/superproject/+/master:out/soong/.intermediates/frameworks/base/framework-minus-apex/android_common/xref28/srcjars.xref/frameworks/base/core/java/android/content/pm/IPackageManager.java;drc=master;bpv=1;bpt=1;l=5500)

**#10 (PackageManagerService extends IPackageManager.Stub).onTransact**

- [https://cs.android.com/android/platform/superproject/+/master:frameworks/base/services/core/java/com/android/server/pm/PackageManagerService.java;l=3937?q=extends IPackageManager.Stub&ss=android%2Fplatform%2Fsuperproject](https://cs.android.com/android/platform/superproject/+/master:frameworks/base/services/core/java/com/android/server/pm/PackageManagerService.java;l=3937?q=extends%20IPackageManager.Stub&ss=android%2Fplatform%2Fsuperproject)

**#11 IPackageManager.Stub**

- [https://cs.android.com/android/platform/superproject/+/master:out/soong/.intermediates/frameworks/base/framework-minus-apex/android_common/xref28/srcjars.xref/frameworks/base/core/java/android/content/pm/IPackageManager.java;l=5500;drc=master;bpv=1;bpt=1](https://cs.android.com/android/platform/superproject/+/master:out/soong/.intermediates/frameworks/base/framework-minus-apex/android_common/xref28/srcjars.xref/frameworks/base/core/java/android/content/pm/IPackageManager.java;l=5500;drc=master;bpv=1;bpt=1)

**#12 PackageManagerService.grantRuntimePermission**

- [https://cs.android.com/android/platform/superproject/+/master:frameworks/base/services/core/java/com/android/server/pm/PackageManagerService.java;l=5655;drc=master;bpv=1;bpt=1](https://cs.android.com/android/platform/superproject/+/master:frameworks/base/services/core/java/com/android/server/pm/PackageManagerService.java;l=5655;drc=master;bpv=1;bpt=1)

**#13 mPermissionManager.grantRuntimePermission**

- [https://cs.android.com/android/platform/superproject/+/master:frameworks/base/services/core/java/com/android/server/pm/permission/PermissionManagerService.java;l=2187;drc=master;bpv=1;bpt=1?q=PermissionManagerServiceInternal &ss=android%2Fplatform%2Fsuperproject](https://cs.android.com/android/platform/superproject/+/master:frameworks/base/services/core/java/com/android/server/pm/permission/PermissionManagerService.java;l=2187;drc=master;bpv=1;bpt=1?q=PermissionManagerServiceInternal%20&ss=android%2Fplatform%2Fsuperproject)