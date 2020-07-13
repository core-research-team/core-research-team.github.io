---
layout: post
title: "Web Encode/Decode Code"
author: "DongDongE"
comments: true
tags: [programming, tools]
---

라온화이트햇 핵심연구팀 유선동([dongdonge@d0ngd0nge.xyz](mailto:dongdonge@d0ngd0nge.xyz))


## [Introduce]

---

CTF 문제를 풀 때 와 개발할 때 각종 인/디코딩 및 해시화를 한 번에 볼 수 있는 게 있으면 좋을 것 같아. 고민 끝에 누구나 이용할 수 있도록 Web Language 제작하였습니다. (본 코드는 Mac OS / Linux OS의 Docker 플랫폼 자동화 기반으로 제작되었으며, 모든 코드는 Gitlab 프로젝트를 참고 하시면 됩니다.)

![/assets/encode.gif](/assets/encode.gif)

## [Code Langugage]

---

- 현재 지원되는 언어는 PHP로 기반 되어 있습니다.

    → 깃랩에 올라온 PHP 코드/도커파일이 디버깅 모드 및 Docker Container에 디버깅 관련 프로그램이 설치 되어 있습니다. 만약 외부에서 접속 하고자 한다면 해당 기능을 Off 해주시거나 제거 해주시면 됩니다.

- 추후 Python Flask도 제작될 예정입니다.

## [Support En/Decode + Hash]

---

- **HASH**: md2, md4, md5, sha1, sha256, sha384, sha512, NTLMHash, ripemd128, ripemd160, ripemd256, ripemd320ripemd320**, whirlpool, tiger128_3, tiger160_3, tiger192_3, tiger128_4, tiger160_4, tiger192_4, snefru, gost, adler32, crc32, crc32b, haval128_3, haval160_3, haval192_3, haval224_3, haval256_3, haval128_4, haval160_4, haval192_4, haval224_4, haval256_4, haval128_5, haval160_5, haval192_5, haval224_5, haval256_5**
- **Encode/Decode** : Base64, URL, HTML, HTML_Entities, Rot13,

**Conversion** :  String To Hex, Hex To String, String To Binary, Binary To String

## [Installing]

---

1. PHP 플랫폼일 경우 `Apache`와 `PHP`가 설치가 되어 있어야 하나 없는 경우 `Docker` 및 `Docker-compose`로 자동으로 컨테이너화 가능하므로, 꼭 사전에 설치를 하셔야 합니다. ([Docker install Link](https://docs.docker.com/get-docker/))
2. [https://git.d0ngd0nge.xyz:3005/dongdonge_public_project/web_encode](https://git.d0ngd0nge.xyz:3005/dongdonge_public_project/web_encode) 에서 제작된 코드를 Git Clone으로 다운 받아 원 클릭으로 손쉽게 도커화 준비하기
3. (Mac or Linux 일 경우) `./compose_start.sh`로 도커라이징
4. [http://localhost:1129/index.php](http://localhost:1129/index.php) 접속

## [PHP/HTML Source Example]

---

```jsx
<html>
    <head>
        <meta charset="utf-8">
        <title>test</title>
        <style type="text/css">
        textarea { width: 100%; }
        input { width: 100%; }
       
        </style>
    </head>
    <body>
        <textarea id="Encode" placeholder="Encode" onkeyup="DongDongE('encode')" autofocus=""></textarea><br>
        <br>
        <div id="encode_res"></div>
        <br><br><br><br>
        <textarea id="Decode" placeholder="Decode" onkeyup="DongDongE('decode')"></textarea><br>
        <br>
        <div id="decode_res"></div>

        <script src="./jquery-3.5.1.min.js"></script>
        <script>
            function DongDongE(mode) {
                if(mode == "encode") {
                    var url = "index.php";
                    var data = {data: $("#Encode").val(), code:"encode"};
                    var result = "#encode_res";
                }
                else {
                    var url = "index.php";
                    var data = {data: $("#Decode").val(), code:"decode"};
                    var result = "#decode_res";
                }
            
                $.post(url, data, function(response) {
                    $(result).html(response);
                });
            }
            
        </script>
    </body>
    </html>
```

**index.html**

사용자가 입력을 할때 마다 즉시 전송하여 인/디코딩 및 해시화를 하여 결과값을 돌려 받습니다. 이때 jquery를 사용하여 인자를 POST로 요청합니다.

```php
<?php
 
    require_once "./func.php";

    if (isset($_POST["data"]) && isset($_POST["code"])) {
        $data   = $_POST["data"];
        $code   = $_POST["code"];   /* encode | decode */
    }
    else {
        require_once "./index.html";
        exit();
    }

    $raw    = FALSE;    /* Default False */
    
    $encode =   array("md2", "md4", "md5", 
                    "sha1", "sha256", "sha384", "sha512",
                    "NTLMHash", 
                    "ripemd128", "ripemd160", "ripemd256", "ripemd320",
                    "whirlpool",
                    "tiger128_3", "tiger160_3", "tiger192_3", "tiger128_4", "tiger160_4", "tiger192_4",
                    "snefru", "gost", "adler32",
                    "crc32", "crc32b",
                    "haval128_3", "haval160_3", "haval192_3", "haval224_3", "haval256_3",
                    "haval128_4", "haval160_4", "haval192_4", "haval224_4", "haval256_4",
                    "haval128_5", "haval160_5", "haval192_5", "haval224_5", "haval256_5",
                    "base64_en", "url_en", "html_en", "html_entities", "rot13_en"
                );

    $connvert = array("str_2_hex", "hex_2_str", "str_2_hex_2", "str_2_hex_3", "str_2_bin", "bin_2_str"
                );

    $decode = array("base64_de", "url_de", "html_de", "rot13_de");

    /* hash start - 자주 사용된 */
    $md2                = hash("md2",            $data, $raw);       // 32
    $md4                = hash("md4",            $data, $raw);       // 32
    $md5                = hash("md5",            $data, $raw);       // 32
    $sha1               = hash("sha1",           $data, $raw);       // 40
    $sha256             = hash("sha256",         $data, $raw);       // 64
    $sha384             = hash("sha384",         $data, $raw);       // 96
    $sha512             = hash("sha512",         $data, $raw);       // 128

    /* hash start - 자주 사용되지 않는 */
    $NTLMHash           = @NTLMHash($data);
    $ripemd128          = hash("ripemd128",      $data, $raw);       // 32
    $ripemd160          = hash("ripemd160",      $data, $raw);       // 40
    $ripemd256          = hash("ripemd256",      $data, $raw);       // 64
    $ripemd320          = hash("ripemd320",      $data, $raw);       // 80
    $whirlpool          = hash("whirlpool",      $data, $raw);       // 128
    $tiger128_3         = hash("tiger128,3",     $data, $raw);      // 32
    $tiger160_3         = hash("tiger160,3",     $data, $raw);      // 40
    $tiger192_3         = hash("tiger192,3",     $data, $raw);      // 48
    $tiger128_4         = hash("tiger128,4",     $data, $raw);      // 32
    $tiger160_4         = hash("tiger160,4",     $data, $raw);      // 40
    $tiger192_4         = hash("tiger192,4",     $data, $raw);      // 48
    $snefru             = hash("snefru",         $data, $raw);      // 64
    $gost               = hash("gost",           $data, $raw);      // 64
    $adler32            = hash("adler32",        $data, $raw);      // 8
    $crc32              = hash("crc32",          $data, $raw);      // 8
    $crc32b             = hash("crc32b",         $data, $raw);      // 8
    $haval128_3         = hash("haval128,3",     $data, $raw);      // 32
    $haval160_3         = hash("haval160,3",     $data, $raw);      // 40
    $haval192_3         = hash("haval192,3",     $data, $raw);      // 48
    $haval224_3         = hash("haval224,3",     $data, $raw);      // 56
    $haval256_3         = hash("haval256,3",     $data, $raw);      // 64
    $haval128_4         = hash("haval128,4",     $data, $raw);      // 32
    $haval160_4         = hash("haval160,4",     $data, $raw);      // 40
    $haval192_4         = hash("haval192,4",     $data, $raw);      // 48
    $haval224_4         = hash("haval224,4",     $data, $raw);      // 56
    $haval256_4         = hash("haval256,4",     $data, $raw);      // 64
    $haval128_5         = hash("haval128,5",     $data, $raw);      // 32
    $haval160_5         = hash("haval160,5",     $data, $raw);      // 40
    $haval192_5         = hash("haval192,5",     $data, $raw);      // 48
    $haval224_5         = hash("haval224,5",     $data, $raw);      // 56
    $haval256_5         = hash("haval256,5",     $data, $raw);      // 64
    /* hash end */
.............................. 너무 길어서 생략 (전체 코드는 Gitlab에서 참조 해주세요!.)
```

**index.php**

[PHP ALL Source Gitlab Link](https://git.d0ngd0nge.xyz:3005/dongdonge_public_project/web_encode/blob/master/src/index.php) 

PHP 자체에서 지원해주는 En/Decode 및 hash는 Default Function를 사용했지만 지원하지 않는 함수는 별도의 func.php 파일로 사용자 정의 함수를 생성하여 로직을 구현 하였습니다.

## [Dockerfile Example]

---

```docker
# Server version: Apache/2.4.38 (Debian)
#       Server built:   2019-10-15T19:53:42
# PHP 7.2.29 (cli) (built: Mar 31 2020 19:37:48) ( NTS )
#       Copyright (c) 1997-2018 The PHP Group
#       Zend Engine v3.2.0, Copyright (c) 1998-2018 Zend Technologies

FROM php:7.2-apache

LABEL DongDongE="DongDongE@d0ngd0nge.xyz"

ARG apache_pwd=/etc/apache2
ARG php_ini=/usr/local/etc/php

COPY entrypoint.sh /sbin/entrypoint.sh

WORKDIR /var/log/apache2/

RUN apt-get update && \
    apt-get install git wget unzip net-tools vim -y && \
    docker-php-ext-install mysqli && \
    cp ${php_ini}/php.ini-production ${php_ini}/php.ini && \
    sed -ri 's/#ServerName www.example.com/ServerName localhost/g' ${apache_pwd}/sites-available/000-default.conf && \ 
    sed -ri 's/#AddDefaultCharset UTF-8/AddDefaultCharset UTF-8/g' ${apache_pwd}/conf-available/charset.conf && \
    sed -ri 's/short_open_tag = Off/short_open_tag = On/g' ${php_ini}/php.ini && \
    sed -ri 's/display_errors = Off/display_errors = On/g' ${php_ini}/php.ini && \
    rm -rf /var/log/apache2/* && \
    touch access.log error.log other_vhosts_access.log && \
    chown root:root /var/www/html -R && \
    chown root:root /var/log/apache2 -R && \
    chmod 700 /var/log/apache2 -R && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*s && \
    chmod 700 /sbin/entrypoint.sh

WORKDIR /var/www/html/

EXPOSE 80

CMD [ "/sbin/entrypoint.sh" ]
```

**Dockerfile Sample**

도커 이미지는 PHP-7.2를 사용하였으며, 커스텀 마이징을 위해  Apache Conf 설정 변경 및 최적화를 셋팅하였습니다.

## [Docker-compose.yml]

---

```docker
version: "3"

services:
    web_encode:
        image: web_encode
        container_name: web_encode
        restart: always
        volumes: 
            - ./src:/var/www/html/
        build:
            context: ./
            dockerfile: ./Dockerfile
        networks:
            - web_encode
        ports: 
            - "127.0.0.1:1129:80"

        
networks:
    web_encode:
        driver: bridge
        ipam:
            config:
                - subnet: 192.168.0.8/24
```

**docker-compose.yml**

포트는 내부 로컬 호스트에서 1129번 포트로 접속 가능하게 설정 하였습니다. 외부에서 접속을 하고자 한다면 "1129:80"으로 수정하시면 됩니다.

## [Delete]

---

```php
$ sudo sh ./compose_down.sh
```