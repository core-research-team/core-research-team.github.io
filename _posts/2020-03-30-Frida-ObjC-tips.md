---
layout: post
title:  "Frida ObjC tips"
author: "kuroneko"
comments: true
tags: [frida]
---

라온화이트햇 핵심연구팀 원요한

## 0. ObjC 특징

- IOS/MacOS 환경에서 주로 사용되며 컴파일 시, Mach-O 포맷을 따른다.
- Receiver / Selector 를 사용하는 queueing기반 실행 구조

    특정 클래스의 함수를 실행하기 위해서는 아래와 같은 포맷을 따라야 한다.

    `[classname method: argument]`

    아래는 그 예시이다.

        int main() {
        	@autoreleasepool {
        		NSString  *str = [NSString stringWithString:@"stringWithString"];
        	}
        }

- autoreleasepool 내에서 생성된 객체는 shared reference가 소멸될 때, 제거된다.
- 함수를 호출하게 되면 queue에 task가 쌓이게 되며, 차례대로 실행되어 처리된다.
- 현재 실행 중인 task의 시간이 정해져 있으며, 해당 시간을 넘어가게 되면 프로그램이 강제 종료된다.

## 1. Frida with ObjC

### 1.1 Class get/alloc/init

    // get class
    ObjC.classes.classname
    ObjC.classes.NSString
    
    // class alloc & init
    ObjC.classes.LocoAgent.alloc().init()
    
    // get/call class method
    // selector naming
    // foo["- function:"] or foo["+ function:"]
    // bar["- variable"] or bar["+ variable"]
    ObjC.classes.classname["function or variable name"]
    ObjC.classes.NSString["- stringWithString_:"]("hello")
    ObjC.classes.NSString.stringWithString_("hello")
    ObjC.classes.Test["- abc"]()
    
    // get class name, methods
    ObjC.classes.$classname
    ObjC.classes.$methods
    
    // choose class
    var LocoAgent = ObjC.chooseSync(ObjC.classes.LocoAgent)
    
    // ptr to class
    Interceptor.attach(ptr(1234), {
    	onEnter: function(args) {
    		var locoagent = new ObjC.Object(args[0]);
    	}
    })

ObjC는 요청받은 클래스의 함수를 실행하기 위해서 string ref가 존재해야 하며, Apple의 정책상 앱의 난독화는 금지 되어있다. 이때문에 ObjC로 짜여진 바이너리의 분석을 하기 위해서 IDA로 확인해보면 심볼이 살아있음을 알 수 있다. 그러므로 위와 같이 원하는 `클래스의 생성, 할당, 초기화, 함수 호출`을 하는데 비교적 자유롭게 가능하다.

### 1.2 ByteArray Helper

    function getUInt64(dataview, byteOffset, littleEndian) {
    	// split 64-bit number into two 32-bit (4-byte) parts
    	const left =  dataview.getUint32(byteOffset, littleEndian);
    	const right = dataview.getUint32(byteOffset+4, littleEndian);
    
    	// combine the two 32-bit values
    	const combined = littleEndian? left + 2**32*right : 2**32*left + right;
    
    	if (!Number.isSafeInteger(combined))
    		console.warn(combined, 'exceeds MAX_SAFE_INTEGER. Precision may be lost');
    
    	return combined;
    }
    
    function u16(bytearray) {
    	if(bytearray.byteLength != 2) 
    		throw "u16 unpack size is not 2"
    	var dv = new DataView(bytearray);
    	return dv.getUInt16();
    }
    
    function u32(bytearray) {
    	if(bytearray.byteLength != 4)
    		throw "u32 unpack size is not 4"
    	var dv = new DataView(bytearray);
    	return dv.getUInt32();
    }
    
    function u64(bytearray) {
    	if(bytearray.byteLength != 8)
    		throw "u64 unpack size is not 8"
    	var dv = new DataView(bytearray);
    	return getUInt64(dv, 0, 1);
    }

frida를 사용해 분석하면 python의 pack/unpack기능이 필요하게 되는데, unpack은 위와 같이 DataView를 사용하여 작성한 unpack함수를 사용하면 된다. pack을 할 경우, 간단히 And연산으로 구현하면 되므로 굳이 코드로 나타내지 않았다.

### 1.3 NSMutableDictionary

    var body = NSMutableDictionary.alloc.init()
    
    var value = 123;
    body.setObject_forKey_(value, "key");
    
    // NSDictionary is BSON supported!
    var bson = new ObjC.Object(body["- BSONData"]());
    var length = bson["- length"]();
    
    // javascript nubmer is always double..
    // But, NSNumber is useful to casting each types.
    // ref. https://developer.apple.com/documentation/foundation/nsnumber/1551470-numberwithint?language=objc
    body.setObject_forKey_(ObjC.classes.NSNumber["+ numberWithInt64:"](1234), "key");

ObjC에서는 Dictionary type을 지원한다. 보통 ObjC에서는 NSDictionary를 사용해서 처리하지만, 이는 할당과 동시에 초기화를 해줘야하는 문제가 존재한다. 그러므로 NSMutableDictionary를 사용하여 원하는 key, value를 작성한 뒤, NSDictionary로 비명시적 타입 캐스팅되도록 유도해서 사용하면 된다.

### 1.4 ObjC Type Helper

    function Int64_helper(value) {
    	return ObjC.classes.NSNumber["+ numberWithInt64:"](value);
    }
    
    function Int32_helper(value) {
    	return ObjC.classes.NSNumber["+ numberWithInt32:"](value);
    }
    
    function Int16_helper(value) {
    	return ObjC.classes.NSNumber["+ numberWithInt16:"](value);
    }
    
    function Float_helper(value) {
    	return ObjC.classes.NSNumber["+ numberWithFloat:"](value);
    }
    
    function Boolean_helper(value) {
    	return ObjC.classes.NSNumber["+ numberWithBool:"](value);
    }
    
    function UInt64_helper(value) {
    	return ObjC.classes.NSNumber["+ numberWithUInt64:"](value);
    }
    
    function UInt32_helper(value) {
    	return ObjC.classes.NSNumber["+ numberWithUInt32:"](value);
    }
    
    function UInt16_helper(value) {
    	return ObjC.classes.NSNumber["+ numberWithUInt16:"](value);
    }

frida는 javascript엔진으로 compile된 코드를 사용해서 실행하게 된다. 이는 javascript에서 수를 관리하는 방법을 따른다는 말이 된다. 즉,  smi의 범위(2^31) 밖의 수를 Double형으로 관리하는 javascript의 특징으로 인해 범위 밖의 Int형 값을 원하는 함수의 인자로 넣고 싶을 때 문제가 발생하게 된다. 이를 우회하기 위해서, NSNumber 클래스의 함수들을 이용하면 된다. NSNumber는 각 타입별로 `boxing/unboxing을 지원`하며, 해당 클래스를 인자로 넘길 경우 unbox하여 인자에 맞는 타입으로 들어가게 된다.

### 1.5 Interceptor

    const LocoAgent = ObjC.classes.LocoAgent;
    Interceptor.attach(LocoAgent["- sendPacket:tag:"].implementation, {
    	onEnter: function(args) {}
    });

원하는 함수를 후킹하기 위해서는, 함수 주소를 인자 값으로 넘겨야 한다. 이는 위와 같이 원하는 함수의 원형을 얻은 이후, implementation을 속성으로 주소 값을 전달하면 된다.

### 1.6. Bypass Anti-Debugging

    int start(int a1)
    {
      ...
      v1 = a1;
      v2 = dlopen(0LL, 0xA);
      v3 = v2;
      v4 = dlsym(v2, "ptrace");
      ((void (__fastcall *)(signed __int64, _QWORD, _QWORD, _QWORD))v4)(31LL, 0LL, 0LL, 0LL);
      dlclose(v3);
      ...
      return 0LL;
    }

위의 코드를 보면, ptrace와 인자 값으로 `PT_DENY_ATTACH(31)` 이 설정되어 있는 것을 볼 수 있다. 이 값은 맥용으로 ptrace를 포팅하면서 생긴 request다. 이 값을 설정하면 디버거의 attach를 방지하여 동적 디버깅을 어렵게 만들어준다. 하지만 이는 Ubuntu에서는 LD_PRELOAD, Mac에서는 `DYLD_INSERT_LIBRARIES` 를 사용해서 인젝션 할 수 있고, 후킹이 가능하게 된다. 아래는 라이브러리 코드 및 컴파일, 인젝션 명령어이다.

    // gcc -dynamiclib hook.c -o hook.dylib
    // DYLD_INSERT_LIBRARIES=hook.dylib /Application/KakaoTalk.app/Contents/MacOS/KakaoTalk
    #include <stdio.h>
    #include <stdlib.h>
    #include <dlfcn.h>
    
    typedef long int (*org_ptrace)(int, pid_t, void *, void *);
    
    long int ptrace(int request, pid_t pid, void *addr, void *data) {
        printf("ptrace[request]: %d\n", request);
        if(request == 31)
            return 0;
    
        org_ptrace pt = (org_ptrace)dlsym(RTLD_NEXT, "ptrace");
        puts("org_ptrace call");
        return pt(request, pid, addr, data);
    }

위의 코드에 있는 주석대로 실행하면, 인젝션과 동시에 안티 디버깅이 우회되어 frida가 정상적으로 붙는 것을 볼 수 있다. 아래의 링크는 Mac환경에서 안티 디버깅 기법들이 정리된 곳이다.

Ref. [https://mobile-security.gitbook.io/mobile-security-testing-guide/ios-testing-guide/0x06j-testing-resiliency-against-reverse-engineering](https://mobile-security.gitbook.io/mobile-security-testing-guide/ios-testing-guide/0x06j-testing-resiliency-against-reverse-engineering)

### 1.7 HTTP request

    const NSMutableURLRequest = ObjC.classes.NSMutableURLRequest;
    const NSURLConnection = ObjC.classes.NSURLConnection;
    const NSURL = ObjC.classes.NSURL;
    const NSNull = ObjC.classes.NSNull;
    
    function nss(str) {
    	return NSString.stringWithString_(str);
    }
    
    var Http = {
    	get: function(url, param) {
    		var request = NSMutableURLRequest["+ requestWithURL:"](
    			NSURL["+ URLWithString:"](nss(url + "?" + Http.dict2data(param)))
    		);
    		
    		request["- setHTTPMethod:"]("GET");
    		var response_ptr = Memory.alloc(0x08);
    		var result = NSURLConnection["+ sendSynchronousRequest:returningResponse:error:"](
    			request, response_ptr, NSNull.alloc().init()
    		);
    
    		var header = new ObjC.Object(response_ptr.readPointer());
    		var body = result.bytes().readCString(result.length());
    
    		request.release();
    		return {
    			header: header,
    			body: body
    		}
    	},
    	post: function(url, data) {
    		if(typeof(data) === "object")
    			data = Http.dict2data(data);
    		
    		
    	},
    	dict2data: function(dict) {
    		var keys = Object.keys(dict);
    		var result = [];
    		for(var i = 0; i < keys.length; i++) {
    			result.push(keys[i] + "=" + escape(dict[keys[i]]));
    		}
    		return result;
    	}
    };
    
    Http.get("https://nekop.kr", {
    	cat: "nyan"
    });
    
    Http.post("https://nekop.kr", {
    	cat: "/etc/passwd"
    });

ObjC는 여러가지 기능들을 제공한다. 위의 코드는 frida로 기능을 추가하기 위해서 HTTP request가 필요한 경우 사용할 수 있는 코드이다. 위의 코드를 기반으로 헤더 수정 및 여러 값들을 조작할 수 있으므로 원하는 기능을 추가하여 프로그램을 확장시킬 수 있다.

