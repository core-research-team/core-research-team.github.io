---
layout: post
title: "Flutter Reversing on Android"
author: "KuroNeko"
comments: true
tags: [mobile, reversing]
---

라온화이트햇 핵심연구팀 원요한

## 개요

multi-platform을 지원하는 flutter 앱 분석 방법 및 한계점에 대해서 작성되었습니다.

### 1. Flutter

flutter란 dart언어를 사용하며 multi-platform 을 지원하는 native 앱 개발 프레임워크입니다. 내부적으로 아래와 같은 구조를 가지고 있습니다.

![/assets/2021-05-01/flutter_0.png](/assets/2021-05-01/flutter_0.png)

주로 dart는 UI를 구현하는데 사용되고, 추가적인 http 요청 및 SharedPreference와 같은 부분은 Flutter Engine에서 구현된 부분을 사용하도록 앱을 작성합니다. 이 때, Framework는 Dart VM으로 구현되어 있고 내부에서 Engine에 존재하는 함수를 호출하도록 되어있습니다. 이러한 구조때문에 `Flutter는 System 인증서를 신뢰하지 않는 경우가 다수 존재`합니다. 따라서 외부로 http/https 요청을 하는 경우, 경우에 따라서는 리패키징이 필요할 수 있습니다.

### 2. 모드 별 특징

flutter는 다른 앱들과 마찬가지로 debug/release 모드로 빌드를 진행할 수 있습니다. 또한 모드별 차이점이 존재합니다. 각 모드별 특징은 아래와 같습니다.

1. Debug mode
    - Dart로 구현된 VM코드가 앱 내 `assets/flutter_assets/kernel_blob.bin` 파일로 따로 분리되어있다.
    - 내부에서 사용하는 문자열 또한 `assets/flutter_assets/` 하위 경로에 아래의 그림과 같이 분리되어있다.

        ![/assets/2021-05-01/flutter_1.png](/assets/2021-05-01/flutter_1.png)

    - `strings kernel_blob.bin` 을 하면 내부에 구현된 소스코드를 확인할 수 있다.

        ![/assets/2021-05-01/flutter_2.png](/assets/2021-05-01/flutter_2.png)

2. Release mode
    - DartVM이 앱 내 `lib/libapp.so` 파일로 isolate_snapshot_data, kernel_blob.bin, vm_snapshot_data이 합쳐져 있다.

        ![/assets/2021-05-01/flutter_3.png](/assets/2021-05-01/flutter_3.png)

        ![/assets/2021-05-01/flutter_4.png](/assets/2021-05-01/flutter_4.png)

### 3. 분석방법

1. Debug mode

    디버그 모드에서는 위에서 볼 수 있듯이 `kernel_blob.bin 문자열을 얻어와 소스코드를 확인`할 수 있습니다. 시스템 인증서를 사용하지 않을 수 있으므로 릴리즈 모드 같이 분석에 어려움을 겪을 수 있습니다. 따라서, 리패키징을 통한 요청 서버 변경이 필요합니다. 이는 릴리즈 모드에서 http/2 프록시를 다룰때 기술하겠습니다.

2. Release mode

    릴리즈 모드는 앞서 설명한 대로 vm코드와 vm에서 사용하는 데이터들을 라이브러리 파일로 생성해 실행합니다. 그러므로 분석을 위한 방법으로 동적 디버깅과 flutter engine의 소스코드를 변경 후 동작 흐름을 파악하는 등, 여러 방식으로 어떤 함수가 외부 요청에 사용되는 지 알 수 있습니다. 아래부터 flutter engine 컴파일 방법과 ssl verify 우회, http/2 proxy에 대해서 기술하겠습니다.

    1. flutter engine compile & replace
        1. [libapp.so](http://libapp.so) class dump

            Release 모드에서는 vm코드와 사용되는 문자열 등의 데이터들을 하나의 라이브러리 파일로 생성후, 로드해서 실행하는 방식으로 구현되어있습니다. 따라서, 내부에서 사용되는 클래스 명을 얻어낼 방법이 존재할 수 있음을 짐작할 수 있는데, `Doldrums`라는 오픈소스가 존재했습니다.

            ![/assets/2021-05-01/flutter_5.png](/assets/2021-05-01/flutter_5.png)

            ![/assets/2021-05-01/flutter_6.png](/assets/2021-05-01/flutter_6.png)

            따라서, apktool로 앱을 디컴파일 한 이후, `./lib/libapp.so` 파일을 위의 그림과 같이 입력해주면 아래의 그림과 같은 dart 를 확인할 수 있습니다.

            ![/assets/2021-05-01/flutter_7.png](/assets/2021-05-01/flutter_7.png)

            거의 모든 클래스 정보를 획득할 수 있으므로, frida를 통해 후킹을 쉽게 접근할 수 있습니다. 하지만 어느정도의 동적 디버깅을 통해 내부 함수를 후킹하는 등의 작업이 필요할 수 있습니다.

        2. Compile Flutter engine

            앱을 분석하기 위해서는 로깅되는 정보가 많아야 함수의 기능을 유추 및 접근성이 용이해집니다. 따라서, flutter 앱 내부에서 사용되는 libflutter.so의 버전을 알아내 해당 버전의 flutter engine의 코드를 직접 수정 후 재컴파일하는 등의 방식으로 많은 양의 로그를 만들 수 있습니다. 따라서 아래의 작업을 통해 flutter engine을 컴파일할 수 있습니다.

            - [https://github.com/flutter/engine](https://github.com/Kur0N3k0/engine) fork

                ![/assets/2021-05-01/flutter_8.png](/assets/2021-05-01/flutter_8.png)

            - download depot_tools & set env

                ![/assets/2021-05-01/flutter_9.png](/assets/2021-05-01/flutter_9.png)

            - write .gclient

                ```python
                # mkdir engine; cd engine
                # write .gclient
                solutions = [
                  {
                    "managed": False,
                    "name": "src/flutter",
                    "url": "https://github.com/<username>/engine.git",
                    "custom_deps": {},
                    "deps_file": "DEPS",
                    "safesync_url": "",
                  },
                ]
                ```

            - gclient sync
            - cd src; ./build/install-build-deps-android.sh
            - ./flutter/tools/gn --no-goma --android --unoptimized --android-cpu=arm64

            1. SSL verify disable
                - boringssl session_verify_cert_chain disable

                    flutter는 내부적으로 boringssl을 사용해 인증서를 검증합니다. cert chain을 검증하는 함수를 찾아 아래의 코드같이 수정한 이후 컴파일 후 리패키징을 해준다면 정상적으로 프록시를 사용할 수 있습니다.

                    ```cpp
                    static bool ssl_crypto_x509_session_verify_cert_chain(SSL_SESSION *session,
                                                                          SSL_HANDSHAKE *hs,
                                                                          uint8_t *out_alert) {
                      ERR_clear_error();
                      return true;
                    }
                    ```

    2. SSL verify disable (frida hooking)

        verify_cert_chain 함수를 후킹하기 위해서는 boringssl을 사용하는 libflutter.so를 후킹해야합니다. 하지만 위의 코드에서 볼 수 있듯이 `static` 으로 구현되어있기때문에 함수 주소를 찾기위해서 `내부적으로 사용하는 문자열을 검색(ssl_client, ssl_server`해서 따라가는 방법으로 원본 함수를 구할 수 있었습니다.

        ![/assets/2021-05-01/flutter_10.png](/assets/2021-05-01/flutter_10.png)

        ![/assets/2021-05-01/flutter_11.png](/assets/2021-05-01/flutter_11.png)

        위와 같이 함수를 찾았으므로, 해당 함수를 후킹하기 위해서 위에서 구현한 코드와 같은 동작을 하도록 frida script를 작성하면 아래와 같이 작성할 수 있습니다.

        ```jsx
        const session_verify_cert_chain_offset = 0x5873D4
        const ERR_clear_error_offset = 0x54599C

        Java.perform(function() {
        		const ERR_clear_error_addr = flutter.add(ERR_clear_error_offset)
            const ERR_clear_error = new NativeFunction(ERR_clear_error_addr, 'void', [])

            Interceptor.replace(flutter.add(session_verify_cert_chain_offset), new NativeCallback(function(a,b,c) {
                console.log("clear error & return 0x01")
                ERR_clear_error()
                return 0x01
            }, "int", ["pointer", "pointer", "pointer"]))
        })

        // or

        Java.perform(function() {
        		Interceptor.attach(flutter.add(session_verify_cert_chain_offset), {
                onEnter: function(args) {
                    console.log("session_verify_cert_chain")
                },
                onLeave: function(ret) {
                    console.log("ret:", ret, " => ", 0x01)
                    ret.replace(0x01)
                }
            })
        })
        ```

    3. http/2 proxy

        앞선 방법으로 프록시가 정상적으로 잡히지 않거나, grpc와 같은 까다로운 조건의 환경에서 프록시를 구성해야한다면 http2 프록시 서버를 구현하는 것이 좋습니다. http2는 https 통신을 하도록 권장하고 있는데, 리패키징을 통해 서버 주소를 프록시 주소로 변경한다면, 아래와 같은 코드를 작성해 e2e http2 프록시환경을 구성해야합니다.

        ```jsx
        const spdy = require("spdy")
        const fs = require("fs")
        const express = require("express")
        const proxy = require("http2-proxy")

        const app = express()

        const options = {
            key: fs.readFileSync('/etc/nginx/ssl/ssl.key'),
            cert: fs.readFileSync('/etc/nginx/ssl/fullchain.cer'),
            spdy: {
                protocols: ['h2']
            }
        }

        app.all("/*", async (req, res) => {
            let bytesWritten = 0

            console.log(req.hostname, req.url)

            const hostname = "example.com"
            const port = 443
            const protocol = port == 443 ? "https" : "http"

            proxy.web(req, res, {
                hostname: hostname,
                port: port,
                protocol: protocol,
                onRes: async (req, res, proxyRes) => {
                    function setHeaders () {
                      if (!bytesWritten) {
                        res.statusCode = proxyRes.statusCode
                        for (const [ key, value ] of Object.entries(proxyRes.headers)) {
                          res.setHeader(key, value)
                        }
                      }
                    }

                    let data = ''
                    proxyRes
                        .on('data', buf => {
                            setHeaders()
                            data += buf.toString('hex')
                            bytesWritten += buf.length
                            if (!res.write(buf)) {
                                proxyRes.pause()
                            }
                        })
                        .on('end', () => {
                            setHeaders()
                            console.log('[req]', req.url)
                            console.log(req.headers)
                            console.log(req.rawBody.toString('hex'))
                            console.log("[res]", req.url)
                            console.log(data)
                            res.addTrailers(proxyRes.trailers)
                            res.end()
                        })
                        .on('close', () => {
                            res.off('drain', onDrain)
                        })

                    res.on('drain', onDrain)

                    function onDrain() {
                        proxyRes.resume()
                    }
                }
            })
        })

        spdy
            .createServer(options, app)
            .listen(443)
        ```

### 3. 한계점

Flutter는 멀티 플랫폼을 지원하기 위한 aarch64 instruction을 사용하지만 ui 및 내부 구현을 전부 심볼이 살아있지 않으므로 앱을 분석하는데에 동적분석은 필수로 적용되어야 합니다. 또한 경우에 따라서 리패키징이 필요하므로 flutter에서 무결성 검증을 하는 로직이 존재한다면 이를 우회해야할 수 있습니다.

### Reference

1. [https://tinyhack.com/2021/03/07/reversing-a-flutter-app-by-recompiling-flutter-engine/](https://tinyhack.com/2021/03/07/reversing-a-flutter-app-by-recompiling-flutter-engine/)
2. [https://github.com/flutter/flutter/wiki/Setting-up-the-Engine-development-environment](https://github.com/flutter/flutter/wiki/Setting-up-the-Engine-development-environment)
3. [https://github.com/flutter/flutter/wiki/Compiling-the-engine](https://github.com/flutter/flutter/wiki/Compiling-the-engine)
4. [https://github.com/rscloura/Doldrums](https://github.com/rscloura/Doldrums)