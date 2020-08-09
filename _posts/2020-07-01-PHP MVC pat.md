---
layout: post 
title: "PHP MVC 패턴"
author: "DongDongE"
comments: true
tags: [programming]
---

라온화이트햇 핵심연구팀 유선동

## [Introduce]

프로그래밍 설계 과정에서 MVC 패턴을 사용하면 빠르고 쉽고 편하게 관리를 할 수 있어, PHP 언어 기반 MVC 패턴을 연구해보았습니다. PHP MVC 패턴이 적용된 공개된 오픈 소스 프레임 워크는 "코드 이그나이터", "라라벨" 등으로 크게 분류할 수 있으나 해당 프레임워크를 사용하기 전 직접 프레임워크를 제작하여 원리를 구현해보고 싶어 연구를 진행하게 되었습니다.

- 본 연구는 "모의 웹 취약점 진단 실습용  웹 프레임워크"를 개발하기 위해 진행하였으며, 공개된 프레임워크는 기본적으로 시큐어 코딩이 탑재되어 있어 직접 프레임워크를 제작하여 구현하게 됨.

- 보다 구체적인 원리는 추후에 gitlab 통하여 코드를 공개하도록 하겠습니다. (현재 Private으로 진행)

## [MVC Pattern]

---

- MVC의 약자는 "Model", "View", "controller"이며 소프트웨어 디자인 패턴이라고 합니다.
- Model
→ "어떤 걸 할지 정의", 데이터 처리(Ex. 로그인, 검증),  로직 수행
- View
→ 사용자에게 보여주는 화면 (시각적인 표현을 담당하며, Ex HTML, CSS)
- Controller
→ 사용자의 요청을 받아 "어떻게 처리 할지 정의", Model 호출

즉, 사용자가 로그인 페이지에 로그인을 시도한다고 하면
"Controller"는 로그인 페이지의 로직을 "Model"에서 가져와 로그인 인증 로직을 수행하고, 성공 또는 실패에 맞는 "View"를 가져와 시각적으로 사용자에게 표현한다.

## [MVC Pattern - 적용전]

---

![/assets/dong0700.png](/assets/dong0700.png)

본 사진 속 코드는 "모의 웹 취약점 진단  실습용 솔루션"이며 MVC 미적용된 코드이다.

우선 사진 속만 봐도 게시판이 추가될 때마다 3개의 파일(list, down,write)가 추가되므로 지저분하게 보인다.

즉 10개의 게시판이 추가된다면 (30개의 php 파일이 생성되며, N*3이 된다.)

## [MVC Pattern - 적용 전 (코드편)]

---

```php
<?php include "./head.php"; ?>

<?php

if (!isset($_SESSION['id']))
{
	echo "<script>alert('Only logged-in Users can.');</script>";
	echo "<script>history.back();</script>";
	exit();
}

if (isset($_GET['no']))
{

	$no = $_GET['no'];

	include "./conn.php";

	$sql = "UPDATE QnA_board SET hit = hit + 1 WHERE BINARY no = $no";
	mysql_query($sql, $conn);

	$sql = "SELECT * FROM QnA_board WHERE BINARY no = $no";
	$result = mysql_query($sql, $conn) or die(mysql_error());
	$row = mysql_fetch_assoc($result);
	
	$nickname = $row['nickname'];
	$sql = "SELECT id, about, photo FROM User WHERE BINARY nickname = '$nickname'";
	$result = mysql_query($sql, $conn) or die(mysql_error());
	$user_row = mysql_fetch_assoc($result);

	mysql_close($conn);

?>

 ..... 생략

<div class="content">
	<div class="container-fluid">
		<div class="row">
			<div class="col-md-12">
				<div class="card">
					<div class="header">
						<h4 class="title">Q&amp;A Board</h4>
						<p class="category">1:1, Only logged in users.</p>
					</div>
					<div class="content table-responsive table-full-width">
						<table class="table table-striped">
							<thead>
								<tr>
									<th>Number</th>
									<th>Title</th>
									<th>Writer</th>
									<th>Date</th>
									<th>views</th>
								</tr>
							</thead>
							<tbody>

..... 생략

```

[QnA 게시판]

```php
<?php include "./head.php"; ?>

<?php
if (isset($_GET['no'])) {

    $no = $_GET['no'];

    include "./conn.php";

    $sql = "UPDATE free_board SET hit = hit + 1 WHERE BINARY no = $no";
    mysql_query($sql, $conn) or die(mysql_error());

    $sql = "SELECT * FROM free_board WHERE BINARY no = $no";
    $result = mysql_query($sql, $conn) or die(mysql_error());
    $row = mysql_fetch_assoc($result);

    $nickname = $row['nickname'];
    $sql = "SELECT id, about, photo FROM User WHERE BINARY nickname = '$nickname'";
    $result = mysql_query($sql, $conn) or die(mysql_error());
    $user_row = mysql_fetch_assoc($result);
    mysql_close($conn);

..... 생략

<div class="content">
        <div class="container-fluid">
            <div class="row">
                <div class="col-lg-4 col-md-5">
                    <div class="card card-user">
                        <div class="image">
                            <img src="assets/img/background.jpg" alt="..."/>
                        </div>
                        <div class="content">
                            <div class="author">
                                <img class="avatar border-white" src="assets/img/no-profile-male-img.gif" alt="..."/>
                                <h4 class="title"><?php echo $row['nickname']; ?><br/>
                                    <a href="#">
                                        <small>@<?php echo $row['ip']; ?></small>
                                    </a>
                                </h4>
                            </div>
                            <p class="description text-center">
                                Free_Board
                            </p>
                        </div>

                    </div>
                </div>
                <div class="col-lg-8 col-md-7">
                    <div class="card">
                        <div class="header">

..... 생략
```

[자유 게시판]

두 개의 게시판 "QnA"와 "자유 게시판"를 보면 쿼리문과 로직이 대부분 비슷하며 쿼리 같은 경우는 테이블 명이 다르고 그 외 칼럼 개수 등 중첩된다.

만약 게시판의 칼럼이 각자 하나씩 추가된다면 게시판 종류별 php 3개의 코드를 하나씩 복잡하게 수정을 해줘야 하는 단점이 있다. 하지만 MVC 코드를 사용하여 위와 같은 불편한 점을 해결할 수 있다.

## [MVC Pattern - 적용 후 ]

---

![/assets/dong0701.png](/assets/dong0701.png)

MVC 패턴이 적용된 PHP이며 Web Root Directory에 설치될 구조이며, 아래에 tree 구조로 전체를 나열하였습니다.

```bash
.
├── DB.sql
├── READEME.md
├── application
│   ├── **controller**
│   │   ├── board.php
│   │   ├── core.php
│   │   ├── main.php
│   │   ├── new_test.php
│   │   ├── profile.php
│   │   ├── route.php
│   │   ├── sign.php
│   │   └── test
│   │       ├── test.php
│   │       └── test2.php
│   ├── **lib**
│   │   ├── autoload.php
│   │   ├── config.php
│   │   ├── db_pdo.php
│   │   └── init.php
│   ├── **model**
│   │   ├── alert.php
│   │   ├── boards.php
│   │   ├── login.php
│   │   ├── model
│   │   │   └── aaa.php
│   │   └── mypage.php
│   └── **view**
│       ├── _template
│       │   ├── footer.php
│       │   └── head.php
│       ├── board
│       │   ├── bob_board
│       │   │   ├── bob_board.php
│       │   │   ├── bob_board_edit.php
│       │   │   ├── bob_board_read.php
│       │   │   └── bob_board_write.php
│       │   ├── free_board.php
│       │   ├── free_board_edit.php
│       │   ├── free_board_read.php
│       │   └── free_board_write.php
│       ├── main
│       │   └── main.php
│       ├── profile
│       │   ├── profile.php
│       │   └── profile_edit.php
│       └── sign
│           ├── signin.php
│           └── signup.php
└── public
    ├── index.php
    ├── raon
    │   └── index.php
    ├── robots.txt
    ├── static
    │   ├── css
    │   │   ├── argon-design-system.css
    │   │   ├── font-awesome.css
    │   │   ├── nucleo-icons.css
    │   │   └── nucleo-svg.css
    │   ├── fonts
    │   │   ├── FontAwesome.otf
    │   │   ├── fontawesome-webfont.eot
    │   │   ├── fontawesome-webfont.svg
    │   │   ├── fontawesome-webfont.ttf
    │   │   ├── fontawesome-webfont.woff
    │   │   ├── fontawesome-webfont.woff2
    │   │   ├── nucleo-icons.eot
    │   │   ├── nucleo-icons.svg
    │   │   ├── nucleo-icons.ttf
    │   │   ├── nucleo-icons.woff
    │   │   └── nucleo-icons.woff2
    │   ├── img
    │   │   ├── brand
    │   │   │   └── blue.png
    │   │   ├── profile
    │   │   │   ├── mohamed.jpg
    │   │   │   ├── no-profile.jpeg
    │   │   │   └── 프로필.jpeg
    │   │   └── raon.png
    │   ├── index.php
    │   ├── js
    │   │   ├── argon-design-system.min.js
    │   │   ├── core
    │   │   │   ├── bootstrap.min.js
    │   │   │   ├── jquery.min.js
    │   │   │   └── popper.min.js
    │   │   ├── notify.js
    │   │   └── plugins
    │   │       ├── bootstrap-datepicker.min.js
    │   │       ├── bootstrap-switch.js
    │   │       ├── chartjs.min.js
    │   │       ├── datetimepicker.js
    │   │       ├── jquery.sharrre.min.js
    │   │       ├── moment.min.js
    │   │       ├── nouislider.min.js
    │   │       └── perfect-scrollbar.jquery.min.js
    │   ├── test.html
    │   └── test.js
    └── upload
        ├── bob_board
        ├── free_board
        └── profile

28 directories, 74 files
```

위 디렉터리 구조를 보면 빨간색으로 표시된 부분이 MVC 패턴대로 분리된 영역이며, "lib" 디렉토리에는 전반적으로 사용되는 설정 값이 들어 있도록 하였습니다.

예를 들어 DB Connect info 또는 Web Route 설정 값이 들어 있도록 하였다.

## [MVC Pattern - 적용 후 (코드) ]

---

```bash
RewriteEngine On
RewriteRule ^$ public/ [L]
RewriteRule ^(.*)$ public/$1 [L,QSA]
```

[/.**htaccess**]

```bash
Options -Multiviews
RewriteEngine On

RewriteCond %{REQUEST_URI} !(raon|upload|static|css)
RewriteCond %{REQUEST_URI} !(\.css|\.js|\.png|\.jpg|\.gif|robots\.txt)$ [NC]

RewriteCond %{REQUEST_FILENAME} !-f [OR]
RewriteCond %{REQUEST_FILENAME} !-d [OR]

RewriteRule ^(.+)$ index.php?url=$1 [QSA,L]
```

[**/public/.htaccess**]

MVC 패턴에서는 대부분 Apache 경우 rewrite를 사용하여 URL에 맞게 rewrite를 진행하도록 htaccess를 사용한다. 해당 문법을 예시로 설명하자면  **http://<IP>/main**으로 접속 시 rewrite 모듈에서 "**/public/index.php?url=main**"으로 변환되어 접속된다. 

```php
<?php
    session_start();
    require_once $_SERVER['DOCUMENT_ROOT']."/application/lib/config.php";
    require_once $_SERVER['DOCUMENT_ROOT']."/application/lib/autoload.php";

    require_once $_SERVER["DOCUMENT_ROOT"] . "/application/lib/db_pdo.php";
    require_once $_SERVER['DOCUMENT_ROOT']."/application/controller/route.php";
    require_once $_SERVER['DOCUMENT_ROOT']."/application/lib/init.php";
?>
```

**[/public/index.php]**

해당 index 페이지는 require를 사용하여 MVC 패턴이 적용된 코드 또는 라이브러리를 불러오도록 작성하였다.

```php
<?php

spl_autoload_register(function($className) {
    
    $allow_class_dir = array(
        "controller/"
    );

    $className = str_replace("\\", "/", $className);

    /* test/test2 */
    // echo '<h3>class path : '. $className.'</h3>';
    
    foreach($allow_class_dir as $dir) {
        if (file_exists(__DIR__."/../".$dir.$className.".php")) {
            require_once __DIR__."/../".$dir.$className.".php";
            return;
        }
    }
    throw new Exception('Can not load class : ' . $className);
});

?>
```

[/**application/lib/autoload.php**]

spl_autoload_register 함수는 (PHP 5 >= 5.1.0, PHP 7)이상 버전에서 지원하며, 역할은 인자로 전달받은 파일명속 클래스는 자동으로 불러와 주는 역할을 한다.

즉, **/application/controller/**의 존재하는 "**board.php"**, "**core.php**", "**main.php**", "**profile.php**" 등 해당 디렉터리에 존재하는 php 파일 속 클래스는 전부 가져와 로드 한다.

```php
<?php

class Router {
    private         $_request_uri;
    private         $_request;
    private         $_routes        = [];
    private         $_default_page  = "main";

    /* 생성자 */
    public function __construct($request = "/") {
        $this->_request_uri = $_SERVER["REQUEST_URI"];
        $this->_request     = $request;
    }

    /* 소멸자 */
    public function __destruct() {
        $this->_request_uri = NULL;
        $this->_request     = NULL;
    }

    public function test() {

        print_r($this->_routes);
    }

    /* 라우팅 매칭 추가 함수 */
    public function addRoute($routeName, $method, $routeExpression, $routeClass, $routeFunction): bool {
        /* 소멸자로 소멸되지 않고 메모리에 존재할 시 비정상 오류 이므로 강제 종료 */
        if ($this->routeExists($routeName)) {
            throw new LogicException($routeName."\n 메모리에 중복 존재로 에러 발생");
            exit();
        }
        else {
            $routeName = empty($routeName) || $routeName == "/" ? $this->$_default_page : $routeName;
            
            $this->_routes[$routeName] = 
            [
                "routeName"         =>  $routeName,
                "method"            =>  $method,
                "routeExpression"   =>  $routeExpression,
                "routeClass"        =>  $routeClass,
                "routeFunction"     =>  $routeFunction
            ];
            
            return true;
        }
        return false;
    }

    public function notPage() {
        die("잘못된 페이지 입니다.");
    }

    /* destruct 소멸자를 통하여 제대로 소멸이 안되어 메모리에 데이터가 남아 있는지 체크 함수 */
    private function routeExists($routeName): bool {
        /* 해당 배열 키(배열 명)이 존재시 1, 실패시 0 */
        return array_key_exists($routeName, $this->_routes);
    }

    /* 요청 URI 파라미터 분리 */
    private function getRequestParameter(): array {
        /*
            $this->_request_uri = $_SERVER["REQUEST_URI"];
            $this->_request     = $request; / 
            [1] => test
             [2] => d
        */
        return array_filter(
            explode($this->_request, $this->_request_uri)
        );
    }

    private function getRequestMethod(): string {
        return addSlashes(stripSlashes(strip_tags($_SERVER["REQUEST_METHOD"])));
    }

    /* 모든 배열 출력 */
    public function showCurrentRoute(): array {
        /*
        [test] => Array                     (배열 명) 
            (
            [routeName]         =>  test             (배열 이름) 중복 금지
            [method]            =>  GET              (메소드)
            [routeExpression]   =>  test/{id}        (링크 구분자)
            [routeClass]        =>  test             (클래스 명)
            [routeFunction]     =>  test_fun         (함수 명)
            )
        */

        return $this->_routes[$this->getRequestParameter()[1] ?? $this->_default_page] ?? $this->notPage();
    }

    private function getRouteParameter(): array {
        /*
            [0] => test
            [1] => {id}
        */
        return array_filter(
            explode($this->_request, $this->showCurrentRoute()["routeExpression"])
        );
    }

    private function getRouteParams(): array {
        /*
            [2] => 123
        */
        return array_diff($this->getRequestParameter(), $this->getRouteParameter());
    }

    private function getExpressionParams(): array {
        /*
            [0] => id
        */
        preg_match_all('/\{(.*?)\}/', $this->showCurrentRoute()["routeExpression"], $routeParams);
        return $routeParams[1];
    }

    public function combinedParameter(): array {

        /* no 파라미터 인자가 존재시  */
        $no_reg = $this->getExpressionParams();
        if ( (end($no_reg) === "no")  ) {
            $getRouteParams_cnt = $this->getRouteParams();
            $getExpressionParams = $this->getExpressionParams();

            if ( (count($getRouteParams_cnt) !== count($getExpressionParams)) && (count($getRouteParams_cnt)+1 == count($getExpressionParams)) ) {
                array_push($getRouteParams_cnt, NULL);
    
                return array_combine($getExpressionParams, $getRouteParams_cnt);
            }
        }        
        if ( count($this->getRouteParams()) !== count($this->getExpressionParams()) ) {
            $this->notPage();
        }

        return array_combine($this->getExpressionParams(), $this->getRouteParams());
    }

    public function execute() {
        $current_route = $this->showCurrentRoute();

        /* 요청한 HTTP Method가 다를 경우 */
        if ( $current_route["method"] !== $this->getRequestMethod()) {
            $this->notPage();
        } else {
            /* 요청한 파라미터 갯수 체크 로직 */
            $arg = $this->combinedParameter();

            $func_test = new $current_route["routeClass"];
            call_user_func_array(array($func_test, $current_route["routeFunction"]), [(object)$arg, $current_route["routeName"]]);
        }

    }

}

?>
```

[**application/controller/route.php**]

```php
<?php

$route = new Router();
$method = strtoupper($_SERVER["REQUEST_METHOD"]);

if ($method === "GET") {
    /*               URL    Method  URL 규칙       클래스           함수*/
    $route->addRoute("main", "GET", "/", "main", "index");
    $route->addRoute("signup", "GET", "signup", "sign", "signup_index");
    $route->addRoute("signin", "GET", "signin", "sign", "signin_index");
    $route->addRoute("signout", "GET", "signout", "sign", "signout");
    $route->addRoute("profile", "GET", "profile/{mode}", "profile", "profile_index");
    $route->addRoute("profile_photo", "GET", "profile_photo/{no}", "profile", "profile_photo_out");
    $route->addRoute("board", "GET", "board/{category}/{mode}/{no}", "board", "board_index");
    
}elseif ($method === "POST") {
    $route->addRoute("signup", "POST", "signup", "sign", "signup");
    $route->addRoute("signin", "POST", "signin", "sign", "signin");
    $route->addRoute("profile", "POST", "profile/edit", "profile", "profile_edit");
    $route->addRoute("board", "POST", "board/{category}/{mode}/{no}", "board", "board_index");
}
else {
    $route->notPage();
    exit();
}

$route->execute();

?>
```

[**/application/lib/init.php**]

init.php에서 URL 경로 라우팅 정보를 기입하여 **route.php에서 해당 URL에 검증하여 정상적인 URL이면 해당 클래스와 함수를 가져와 로직을 실행한다.**

## [MVC Pattern - 적용 후 (로그인 페이지) ]

---

```php
<?php

/* 서브 폴더가 없으므로 namespase register; 선언은 하지 않는다. */
class sign extends \core {

..... 생략

/* 로그인 인덱스 페이지 로직 */
    public function signin_index($data, $title) {
        $login = $this->model("login");
        $alert = $this->model("alert");
        
        /* 이미 로그인 되어 있으면 해당 서비스 접근 이용 불가 */
        $data_array = array(
            "title"=>$title,
            "login"=>!$login->isLoginIn_Location(0)
        );

        $this->view("sign/signin", $data_array);
        
    }

    /* 로그인 로직 */
    public function signin($data, $title) {
        $login = $this->model("login");
        $alert = $this->model("alert");

        $res_login = !$login->isLoginIn_Location(0);

        if ($login->signin()) {
            /* 이미 로그인 되어 있으면 해당 서비스 접근 이용 불가 */
            $data_array = array(
                "title"=>$title,
                "login"=>$res_login
            );
            alert::alert_notify("로그인에 성공하였습니다. {$alert->xss_check($_SESSION['id'])}님 반갑습니다.", "success", "/");
        }else {
            alert::alert_notify("아이디 또는 패스워드를 재확인 해주십시오.", "error");
        }

        exit();
    }

..... 생략
}
```

**[/controller/sign.php]**

sign controller는 회원가입/로그인을 내부적으로 어떻게 처리할지 정의되어 있다.

현재 "http://<ip>/signup"으로 접속하면 route.php를 통하여 sign.php의 sign 클래스를 불러와 signin_index 함수를 실행시킨다. 해당 함수는 단순히 어떤 걸 처리하기보단 사용자가 로그인할 수 있도록 Controller가 View(로그인 화면)를 사용자에게 전달한다.

```php
<?php require_once _VIEW . "_template/head.php"; ?>
<section class="section section-shaped section-lg" style="height: 100vh;">
    <div class="shape shape-style-1 bg-gradient-default">
      <span></span>
      <span></span>
      <span></span>
      <span></span>
      <span></span>
      <span></span>
      <span></span>
      <span></span>
    </div>
    <div class="container pt-lg-9">
      <div class="row justify-content-center">
        <div class="col-lg-5">
          <div class="card bg-secondary shadow border-0">
            <div class="card-body px-lg-5 py-lg-5">
              <div class="text-center text-muted mb-4">
                <h2 style="font-weight: 700;">Log In</h2>
              </div>
              <form role="form" method="POST" action="/signin">
                <div class="form-group mb-3">
                  <div class="input-group input-group-alternative">
                    <div class="input-group-prepend">
                      <span class="input-group-text"><i class="ni ni-email-83"></i></span>
                    </div>
                    <input class="form-control" placeholder="ID" type="text" name="id" required>
                  </div>
                </div>
                <div class="form-group focused">
                  <div class="input-group input-group-alternative">
                    <div class="input-group-prepend">
                      <span class="input-group-text"><i class="ni ni-lock-circle-open"></i></span>
                    </div>
                    <input class="form-control" placeholder="Password" type="password" name="password" required>
                  </div>
                </div>
                <div class="text-center">
                  <input type="submit" class="btn btn-primary my-4" name="submit" value="로그인">
                </div>
              </form>
            </div>
          </div>
          <div class="row mt-3">
            <div class="col-6">
              <a href="#" class="text-light"><small>Forgot password?</small></a>
            </div>
            <div class="col-6 text-right">
              <a href="#" class="text-light"><small>Create new account</small></a>
            </div>
          </div>
        </div>
      </div>
    </div>
  </section>
<?php require_once _VIEW . "_template/footer.php"; ?>
```

**[/application/view/sign/signin.php]**

로그인 페이지를 담당하는 View 영역의 signin.php 코드입니다.

그 외 회원가입, 게시판 글 등록 등 로직은 위와 같이 방식으로 나눠 처리를 진행하면 됩니다.

MVC 패턴을 통하여 중복되는 코드를 최소화를 시킬 수 있었으나 해당 코드를 전부 설명하기엔 너무 길어 추후에 gitlab에 주석과 함께 공개할 예정입니다.