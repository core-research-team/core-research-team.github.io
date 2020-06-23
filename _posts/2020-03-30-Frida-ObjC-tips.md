---
layout: post
title:  "Frida ObjC tips"
author: "kuroneko"
comments: true
tags: [frida]
---


라온화이트햇 핵심연구팀 원요한([kuroneko9505@gmail.com](mailto:kuroneko9505@gmail.com))


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

## 2. MacOS KakaoTalk Bot

위의 개념들을 모두 사용해서 만들어진 맥용 카카오톡 봇이다.

    const baseAddr = Module.findBaseAddress('KakaoTalk');
    const LocoAgent = ObjC.classes.LocoAgent;
    const LocoPacket = ObjC.classes.LocoPacket;
    const LocoPacketHeader = ObjC.classes.LocoPacketHeader;
    const LocoManager = ObjC.classes.LocoManager;
    const LocoV2SLCrypto = ObjC.classes.LocoV2SLCrypto;
    const BCLocoClient = ObjC.classes.BCLocoClient;
    const FCChattingController = ObjC.classes.FCChattingController;
    const NTChatMessage = ObjC.classes.NTChatMessage;
    const NTUser = ObjC.classes.NTUser;
    const LPProperties = ObjC.classes.LPProperties;
    const NSDictionary = ObjC.classes.NSDictionary;
    const NSMutableDictionary = ObjC.classes.NSMutableDictionary;
    const NSString = ObjC.classes.NSString;
    const NSData = ObjC.classes.NSData;
    const NSMutableData = ObjC.classes.NSMutableData;
    const NSMutableURLRequest = ObjC.classes.NSMutableURLRequest;
    const NSMutableURLResponse = ObjC.classes.NSMutableURLResponse;
    const NSURLConnection = ObjC.classes.NSURLConnection;
    const NSError = ObjC.classes.NSError;
    const NSURL = ObjC.classes.NSURL;
    
    function sendMessage(chatid, message, extra) {
    	var packet = LocoPacket.alloc().init();
    	var header = LocoPacketHeader.alloc().init();
    	var body   = NSMutableDictionary.alloc().init();
    	var clog   = NSMutableDictionary.alloc().init();
    
    	var func = body.setObject_forKey_.implementation;
    	var test = new NativeFunction(func, 'void', ['pointer', 'pointer']);
    	var mem = Memory.allocUtf8String("chatId");
    	
    	body.setObject_forKey_(ObjC.classes.NSNumber["+ numberWithInt64:"](chatid["- longLongValue"]()), "chatId");
    	body.setObject_forKey_(extra, "extra");
    	body.setObject_forKey_(message, "msg");
    	body.setObject_forKey_(ObjC.classes.NSNumber["+ numberWithInt64:"](200002501), "msgId");
    	body.setObject_forKey_(ObjC.classes.NSNumber["+ numberWithBool:"](0), "noSeen");
    	body.setObject_forKey_(ObjC.classes.NSNull.alloc().init(), "supplement");
    	body.setObject_forKey_(ObjC.classes.NSNumber["+ numberWithInt32:"](1), "type");
    
    	var bson = new ObjC.Object(body["- BSONData"]());
    	var length = bson["- length"]();
    
    	var method = NSString.stringWithString_("WRITE");
    	
    	header["- setPacketId:"](100000011);
    	header["- setStatusCode:"](0);
    	header["- setMethod:"](method);
    	header["- setBodyType:"](0);
    	header["- setBodyLength:"](parseInt(length));
    
    	packet["- setHeader:"](header);
    	packet["- setBody:"](body);
    
    	var agent = ObjC.chooseSync(LocoAgent)[0];
    	agent["- sendPacket:tag:"](packet, 0x5f5e10b);
    }
    
    function mention(chatid, userid) {
    	sendMessage(chatid, "@test", JSON.stringify({
    		mentions: [
    			{
    				user_id: userid,
    				at: [1],
    				len: 4
    			}
    		]
    	}));
    }
    
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
    
    function nss(str) {
    	return NSString.stringWithString_(str);
    }
    
    function getBotID() {
    	var ctx = ObjC.chooseSync(LPProperties)[0]["- me"]();
    	return parseInt(ctx["- userId"]());
    }
    
    console.log("[*] Bot initialize");
    var ChatRoom = { };
    var BotID = getBotID();
    var BotArg = {
    	chatid: 0,
    	author: "",
    	arg: "",
    };
    console.log("BotID:" + BotID);
    
    var Bot = {
    	prefix: ".",
    	Handler: {},
    	Description: {},
    	AdminHandler: {},
    	UserRequest: {},
    	Limit: {
    		maxcount: 5,
    		minute: 1
    	},
    	addCommand: function(cmd, desc, handler) {
    		if(typeof(handler) === "function") {
    			Bot.Description[Bot.prefix + cmd] = cmd + ": " + desc;
    			Bot.Handler[Bot.prefix + cmd] = handler;
    			return true;
    		}
    		return false;
    	},
    	addAdminCommand: function(cmd, desc, handler) {
    		if(typeof(handler) === "function") {
    			Bot.Description[Bot.prefix + cmd] = cmd + ": " + desc;
    			Bot.AdminHandler[Bot.prefix + cmd] = handler;
    			return true;
    		}
    		return false;
    	},
    	execCommand: function(cmd, arg) {
    		var now = parseInt(Date.now() / 1000);
    
    		if(Bot.UserRequest[arg.authorid] === undefined) {
    			Bot.UserRequest[arg.authorid] = {
    				count: 0,
    				time: now
    			};
    		}
    
    		console.log(arg.authorid + ": " + (now - Bot.UserRequest[arg.authorid].time));
    
    		if(Bot.UserRequest[arg.authorid].count >= Bot.Limit.maxcount) {
    			Bot.UserRequest[arg.authorid].count = Bot.Limit.maxcount;
    			Bot.UserRequest[arg.authorid].time = now + Bot.Limit.minute * 60; // Limit.minute 분 동안 사용못함
    
    			var diff = Bot.UserRequest[arg.authorid].time - now;
    			if(diff > 0){
    				sendMessage(arg.chatid,	diff + "초 후, 다시 시도해주세요.", "{}");
    				return;
    			}
    			Bot.UserRequest[arg.authorid].count = 0;
    		}
    		
    		if(now - Bot.UserRequest[arg.authorid].time < 2) { // 2초 내에 요청하면 카운트 증가
    			Bot.UserRequest[arg.authorid].count++;
    		} else if (now - Bot.UserRequest[arg.authorid].time > 30) { // 30초 이상일 때 카운트 초기화
    			Bot.UserRequest[arg.authorid].count = 0;
    		}
    
    		Bot.UserRequest[arg.authorid].time = now;
    
    		if(Bot.Handler[cmd] !== undefined)
    			return Bot.Handler[cmd](arg);
    		else
    			sendMessage(arg.chatid, "없는 기능입니다.", "{}")
    	},
    	execAdminCommand: function(cmd, arg) {
    		if(Bot.AdminHandler[cmd] !== undefined)
    			return Bot.AdminHandler[cmd](arg);
    	},
    	msg2format: function(msg) {
    		var tmp = msg.split(" ");
    		var cmd = tmp[0];
    		var arg = tmp.splice(1).join(" ").trim().replace(/[\s]+/g, " ");
    		var regex = /^[가-힣0-9a-z !]+$/ig;
    		if(arg !== "" && !regex.test(arg))
    			return undefined;
    		return {
    			cmd: cmd,
    			arg: arg
    		}
    	},
    	isCommand: function(msg) {
    		var re = eval("/^\\" + Bot.prefix + ".+/");
    		return re.test(msg);
    	},
    }
    
    var Http = {
    	get: function(url, param) {
    		var request = NSMutableURLRequest["+ requestWithURL:"](
    			NSURL["+ URLWithString:"](nss(url + "?" + Http.dict2data(param)))
    		);
    
    		request["- setHTTPMethod:"]("GET");
    		var response_ptr = Memory.alloc(0x08);
    		var result = NSURLConnection["+ sendSynchronousRequest:returningResponse:error:"](
    			request, response_ptr, ObjC.classes.NSNull.alloc().init()
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
    		var request = NSMutableURLRequest["+ requestWithURL:"](
    			NSURL["+ URLWithString:"](nss(url))
    		);
    
    		data = Http.dict2data(data)
    		var bdata = nss(data);
    		request["- setHTTPMethod:"]("POST");
    		request["- setValue:forHTTPHeaderField:"](nss("application/x-www-form-urlencoded"), nss("Content-Type"));
    		
    		var ptr = Memory.allocUtf8String(data);
    		request["- setHTTPBody:"](
    			NSData["+ dataWithBytes:length:"](
    				ptr, data.length
    			)
    		);
    		
    		var response_ptr = Memory.alloc(0x08);
    		var result = NSURLConnection["+ sendSynchronousRequest:returningResponse:error:"](
    			request, response_ptr, ObjC.classes.NSNull.alloc().init()
    		);
    
    		var header = new ObjC.Object(response_ptr.readPointer());
    		var body = result.bytes().readCString(result.length());
    
    		request.release();
    		return {
    			header: header,
    			body: body
    		}
    	},
    	dict2data: function(dict) {
    		var keys = Object.keys(dict);
    		var result = [];
    		for(var i = 0; i < keys.length; i++) {
    			var type = typeof(dict[keys[i]]);
    			if(type === "object") {
    				for(var k = 0; k < dict[keys[i]].length; k++) {
    					result.push(keys[i] + "[]=" + dict[keys[i]][k]);
    				}
    			} else {
    				result.push(keys[i] + "=" + encodeURI(dict[keys[i]]));
    			}
    		}
    		return result.join("&");
    	}
    };
    
    Bot.addCommand("help", "명령어 목록을 출력합니다.", function(arg) {
    	var keys = Object.keys(Bot.Handler);
    	var msg = [ "[help]" ];
    	for(var i = 0; i < keys.length; i++)
    		msg.push(Bot.Description[keys[i]]);
    	sendMessage(arg.chatid, msg.join("\n"), "{}");
    });
    
    Bot.addCommand("cat", "print meow :3", function(arg) {
    	sendMessage(arg.chatid, "cat: meow :3", "{}");
    });
    
    Bot.addCommand("echo", "받은 메세지를 전송합니다.", function(arg) {
    	sendMessage(arg.chatid, arg.arg, "{}");
    });
    
    Bot.addCommand("안녕", "인사를 보냅니다.", function(arg) {
    	sendMessage(arg.chatid, arg.author + " 님도 안녕!", "{}");
    });
    
    var bob_menu = {};
    
    Bot.addCommand("메뉴추가", "점심메뉴를 추가합니다.", function(arg) {
    	if(bob_menu[arg.chatid] === undefined)
    		bob_menu[arg.chatid] = [];
    
    	var menu = arg.arg;
    	if(bob_menu[arg.chatid].indexOf(menu) !== -1) {
    		setTimeout(function(){
    			sendMessage(arg.chatid, "이미 추가된 점심메뉴입니다.", "{}");
    		}, 200);
    	} else {
    		bob_menu[arg.chatid].push(menu);
    		setTimeout(function() {
    			sendMessage(arg.chatid, menu + "이(가) 추가되었습니다.", "{}");
    		}, 200);
    	}
    });
    
    Bot.addCommand("메뉴추첨", "추가된 점심메뉴를 추첨합니다.", function(arg) {
    	setTimeout(function(){
    		if(bob_menu[arg.chatid] === undefined)
    			bob_menu[arg.chatid] = [];
    
    		var menu = bob_menu[arg.chatid][Math.floor(Math.random() * bob_menu[arg.chatid].length)];
    		sendMessage(arg.chatid, "[점심메뉴]\n" + menu, "{}");
    	}, 200);
    });
    
    Bot.addCommand("메뉴", "점심메뉴를 출력합니다.", function(arg) {
    	setTimeout(function(){
    		if(bob_menu[arg.chatid] === undefined)
    			bob_menu[arg.chatid] = [];
    
    		var result = ["[점심메뉴]"];
    		if(bob_menu[arg.chatid].length === 0) {
    			sendMessage(arg.chatid, "점심메뉴를 추가해주세요.", "{}");
    			return;
    		}
    
    		for(var i = 0; i < bob_menu[arg.chatid].length; i++)
    			result.push((i + 1) + ". " + bob_menu[arg.chatid][i]);
    		
    		sendMessage(arg.chatid, result.join("\n"), "{}");
    	}, 200);
    });
    
    Bot.addCommand("메뉴제외", "점심메뉴를 제외합니다.", function(arg) {
    	setTimeout(function() {
    		if(bob_menu[arg.chatid] === undefined)
    			bob_menu[arg.chatid] = [];
    
    		var idx = parseInt(arg.arg)
    		if (bob_menu[arg.chatid].length === 0) {
    			sendMessage(arg.chatid, "점심메뉴를 추가해주세요.", "{}");
    			return;
    		}
    		else if(idx < 0 || bob_menu[arg.chatid].length < idx) {
    			sendMessage(arg.chatid, "올바른 번호를 입력해주세요.", "{}");
    			return;
    		}
    
    		var menu = bob_menu[arg.chatid][idx - 1];
    		bob_menu[arg.chatid] = bob_menu[arg.chatid].splice(0, idx - 1).concat(bob_menu[arg.chatid].splice(idx));
    		sendMessage(arg.chatid, menu + "이(가) 제외되었습니다.", "{}")
    	}, 200);
    });
    
    Bot.addCommand("id", "authorid를 출력합니다.", function(arg) {
    	var user = arg.author;
    	var userid = arg.authorid;
    	var passcode = arg.arg;
    
    	setTimeout(function(){
    		sendMessage(arg.chatid, user + "님의 authorid: " + userid, "{}");
    	}, 200);
    });
    
    Bot.addAdminCommand("승인", "요청을 승인합니다.", function(arg) {
    	if(ChatRoom[arg.chatid] === undefined)
    		ChatRoom[arg.chatid] = [];
    
    	for(var i = 0; i < ChatRoom[arg.chatid].length; i++) {
    		var user = ChatRoom[arg.chatid][i].author;
    		var userid = ChatRoom[arg.chatid][i].authorid;
    		var passcode = ChatRoom[arg.chatid][i].arg;
    		setTimeout(function(){
    			if(Ebook.addUser(userid, passcode) === false) {
    				try{
    					var result = Ebook.getToken(userid, passcode);
    				} catch(e) {
    					sendMessage(arg.chatId, "passcode가 틀렸습니다.", "{}");
    					return;
    				}
    			}
    			sendMessage(arg.chatid, user + " 님의 요청이 승인되었습니다.", "{}");
    			delete ChatRoom[arg.chatid][i];
    		}, 200);
    	}
    });
    
    Interceptor.attach(LocoPacket["- packetData"].implementation, {
    	onEnter: function(args) {
    		this.packet = args[0];
    	},
    	onLeave: function(retval) {
    		var locop = new ObjC.Object(this.packet);
    		var body = locop.body();
    		var chatid = body["- objectForKey:"]("chatId");
        if(chatid == null)
          return;
        chatid = nss(chatid);
    		var msg = body["- objectForKey:"]("msg");
    		if(msg === null)
    			return;
    
    		msg = msg.toString();
    		if(msg !== undefined && Bot.isCommand(msg)) {
    			var format = Bot.msg2format(msg);
    			Bot.execAdminCommand(format.cmd, {
    				chatid: chatid,
    				arg: format.arg
    			});
    		}
    	}
    });
    
    Interceptor.attach(LocoPacket["- initWithPacketData:"].implementation, {
    	onLeave: function(retval) {
    		var result = new ObjC.Object(retval);
    		var body = result.body();
    		var author = body["- objectForKey:"]("authorNickname")
    		if(author !== null)
    			author = author.toString();
    		var chatid = body["- objectForKey:"]("chatId");
        if(chatid == null)
          return;
        chatid = nss(chatid);
    		var log = body["- objectForKey:"]("chatLog");
    		if(log == null)
    			return;
    
    		var authorid = parseInt(log["- objectForKey:"]("authorId"));
    		if(authorid == BotID)
    			return;
    
    		var msg = log["- objectForKey:"]("message").toString();
    		if(msg !== undefined && Bot.isCommand(msg)) {
    			var format = Bot.msg2format(msg);
    			if(format === undefined){
    				sendMessage(chatid, "올바른 인자 값 넣어주세요.", "{}");
    				return;
    			}
    
    			Bot.execCommand(format.cmd, {
    				chatid: chatid,
    				author: author,
    				authorid: authorid,
    				arg: format.arg
    			});
    		}
    	}
    });