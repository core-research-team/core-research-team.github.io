---
layout: post
title:  "Git Project"
author: "dongdonge"
comments: true
tags: [programming]
---

라온화이트햇 핵심연구팀 유선동


본 자료의 참고 내용은 "**Do it! 지옥에서 온 문서 관리자 깃&깃허브 입문**", "**Git Pro**" 내용을 참고하여, 자주 쓰이는 명령어만 골라 요약  하였습니다.

# 01. Git Version 관리 하기

## 01-1 Git Directory(저장소) 기초

### [저장소 만들기]

    $ git init

Git Directory에 저장소를 만들고 초기화 작업을 수행하면, "**.git**" 숨긴 폴더가 생성됩니다.

Gitlab이나 Github 프로젝트(기존 저장소)에서 Clone을 하였다면 해당 명령을 입력하실 필요는 없습니다. 하지만 프로젝트 저장소에 아무런 소스 코드가 존재 하지 않는다면 해당 명령을 입력하여 뼈대 파일을 만들어 줍니다.

### [기존 프로젝트 저장소 Clone 하기]

![/assets/dong0300.png](/assets/dong0300.png)

Github, Gitlab Project Clone URL를 복사하여 아래 명령어를 통해 Clone 작업을 진행합니다.

    # Clone시 프로젝트명 "Flask_Development_Web" 폴더로 생성되어 저장됩니다.
    $ git clone https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git
    
    # 또는
    
    # Clone URL 뒤 사용자가 디렉터리명을 지정해주면 "web_project" 폴더명으로 생성됩니다.
    $ git clone https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git web_project

기존 저장소(Github, Gitlab)에서 위와 같이 Clone 작업을 진행해야 프로젝트의 히스토리(Commit 이력 등)를 전부 가져옵니다. 추후 기존 저장소가 손상되거나 다른 서버로 이전 작업을 진행할 때 환경 그대로 복구할 수 있습니다.

현재는 HTTPS/HTTP 프로토콜을 사용하지만 외부 Public Project가 아닌 이상 추후 대용량 프로젝트 경우 SSH 프로토콜을 이용하여 Git 작업을 수행할 것을 추천 드립니다.

### [로컬 리모트 저장소 확인하기]

    $ git remote -v
    origin	https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git (fetch)
    origin	https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git (push)

해당 명령어로 현재 프로젝트에 등록된 리모트 저장소의 주소와 프로토콜을 확인할 수 있습니다.

현재 HTTPS 프로토콜을 사용하며, [git.d0ngd0nge.xyz](http://git.d0ngd0nge.xyz) 사이트의 "Flask_Development_web" 프로젝트로 설정되어 있습니다.

현재는 "origin" 이라는 이름의 저장소로 등록되어 있지만, 추후 여러 저장소를 등록할 수 있습니다.

### [외부 리모트 저장소 확인하기]

    $ git remote show origin
    * 리모트 origin
      가져오기 URL: https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git
      푸시  URL: https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git
      HEAD 브랜치: master
      리모트 브랜치:
        dongdonge 추적됨
        master    추적됨
        whitehat  추적됨
      'git pull'에 사용할 로컬 브랜치를 설정:
        dongdonge 병합: 리모트 dongdonge
        master    병합: 리모트 master
      로컬 레퍼런스를 'git push'로 미러링:
        dongdonge에서 dongdonge(으)로 푸시 (정방향 진행 가능)
        master   에서 master   (으)로 푸시 (최신 상태)

외부 리모트 저장소의 구체적인 정보를 확인할 수 있습니다. 해당 명령어로는 외부에서 가져온 모든 리모트 저장소 정보를 출력하기에 로컬로 가져오지 않은 외부 리모트 저장소의 브랜치 목록을 볼 수 있으며, 로컬에 가지고 있는 브랜치 또한 비교할 수 있습니다.

### [리모트 저장소 추가하기]

    # Remote 주소로 ssh 프로토콜을 추가함.
    $ git remote add dongdonge ssh://git@git.d0ngd0nge.xyz:3006/DongDongE/Flask_Development_Web.git
    
    
    $ git remote -v
    dongdonge	ssh://git@git.d0ngd0nge.xyz:3006/DongDongE/Flask_Development_Web.git (fetch)
    dongdonge	ssh://git@git.d0ngd0nge.xyz:3006/DongDongE/Flask_Development_Web.git (push)
    origin	ssh://git@git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git (fetch)
    origin	ssh://git@git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git (push)

"git remote add" 명령어를 통하여 Git remote 주소를 추가 하였습니다. 

    # 저장소 push
    $ git push -u dongdonge master
    
    
    $ git pull dongdonge

방금 새로 추가한 "dongdonge" 리모트 저장소로 Push나 Pool를 진행하고자 한다면 위와 같이 입력하시면 됩니다.

### [리모트 저장소 이름 변경, 삭제하기]

    # 리모트 저장소 이름 변경하기 (dongdonge를 docker로 변경)
    $ git remote rename dongdonge docker
    docker	ssh://git@git.d0ngd0nge.xyz:3006/DongDongE/Flask_Development_Web.git (fetch)
    docker	ssh://git@git.d0ngd0nge.xyz:3006/DongDongE/Flask_Development_Web.git (push)
    origin	ssh://git@git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git (fetch)
    origin	ssh://git@git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git (push)
    
    
    # 리모트 저장소 삭제 하기 (docker 저장소 제거)
    $ git remote rm docker
    origin	ssh://git@git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git (fetch)
    origin	ssh://git@git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git (push)

"**git remote rename <변경 대상 리모트 저장소 이름> <변경하고자 새로운 리모트 저장소 이름**>"

즉, "**dongdonge**" 리모트 저장소 이름이 "**docker**" 저장소 이름으로 변경되었습니다.

"**git remote rm <삭제하려는 리모트 저장소 이름>**" 형태로 사용하여 리모트 저장소를 삭제할 수 있습니다.

## 01-2 Git 작동 원리

### [스테이지와 커밋 원리 이해]

![/assets/dong0301.png](/assets/dong0301.png)

- Working Tree (작업 트리)
사용자 눈에 직접 보이고 사용자가 작업할 코드가 들어있는 디렉터리이며  파일 수정, 저장 등의 작업을 하는 디렉터리
- Stage (스테이지)
Git Version으로 만들 파일이 대기하는 곳이며, 작업 트리에서 파일을 수정후 해당 파일을 버전으로 만들기 위해 스테이지로 넘겨주면 된다.
- Repository (리포지토리)
스테이지에서 대기하고 있던 파일들을 버전으로 만들어 저장하며 ".git" 디렉터리안에 숨은 파일 형태로 존재한다.

![/assets/dong0302.png](/assets/dong0302.png)
"hello.txt" 파일을 수정하고 저장하면 해당 파일은 **작업 트리**에 있게 되며. "git add" 명령어를 통해 수정한 "hello.txt" 파일을 버전으로 만들기 위해 **스테이지**에 넣는다.

![/assets/dong0303.png](/assets/dong0303.png)
"git commit" 명령을 사용하여 새로운 버전이 생성되면서 **스테이지**에 대기하던 "hello.txt" 파일이 모두 저장소에 저장된다.

### [수정/생성 파일 스테이징하기 - git add]

    $ git add hello.txt

"changes to be committed" "new file: hello.txt"의 수식어와 함께 스테이지에 추가된다.

### [스테이지 파일 커밋하기 - git commit]

    $ git commit -m "코맨드 메시지"

스테이지에 해당 파일이 존재한다면 commit 명령어를 통해 버전을 만들 수 있다. 대부분 commit 명령어와 함께 어떤 변경 사항이 있었는지 또는 이슈 사항이 있었는지 간단한 요약을 메시지로 작성한다.
또한 공통 public project시 한글 보단 영문을 작성하여 누구나 알아 볼 수 있게 한다.

### [스테이지/커밋 한번에 처리 - git commit -a -m]

    $ git commit -a -m "코맨드 메시지"
    
    
    $ git commit -am "코맨드 메시지"

"**git add**" 명령어와 "**git commit**" 명령어를 한꺼번에 처리 할 수 있으며, 위 명령어 옵션중 하나를 선택하여 사용하면 된다.

### [Unstaged, 코드 변경 내용 확인 - git diff]

단순히 소스 코드 내용이 변경 되었다면 어떤 부분에 어떤 내용이 변경 되었는지 확인할 수 있습니다.

    $ git status
    현재 브랜치 master
    브랜치가 'origin/master'보다 3개 커밋만큼 앞에 있습니다.
      (로컬에 있는 커밋을 제출하려면 "git push"를 사용하십시오)
    
    커밋하도록 정하지 않은 변경 사항:
      (무엇을 커밋할지 바꾸려면 "git add <파일>..."을 사용하십시오)
      (use "git restore <file>..." to discard changes in working directory)
    	수정함:        README.md
    
    커밋할 변경 사항을 추가하지 않았습니다 ("git add" 및/또는 "git commit -a"를
    사용하십시오)

현재 "**README.md**" 파일이 수정되어 있습니다. "**git diff**" 명령을 사용하기 위해 해당 파일이 스테이징이 되어 있으면 안됩니다.

즉, "**git add**" 명령으로 "**README.md**"를 스테이징 후 "**git diff**"로 변경 내용을 확인 할 수 없습니다.

만약 이미 "**git add**" 명령으로 스테이징을 진행 하였다면 "**git diff —staged**" 명령으로 변경 내용을 확인 할 수 있습니다.

    $ git diff
    diff --git a/README.md b/README.md
    index 92c48d0..e5beddf 100644
    --- a/README.md
    +++ b/README.md
    @@ -1,3 +1,5 @@
    +## DongDongE
    +
     # CTF 테스트
     # Python 3.X로 개발 진행 중

현재 워킹 디렉터리 (현재 작업 디렉터리)와 스테이징에 있는 것을 비교하여 보여 준다.

초록색 부분이 추가 된 것이며, 현재 "**DongDongE**" 문자열이 추가된 걸 볼 수 있습니다.

    $ git add README.md
    
    
    # 스테이징된 "README.md" 파일 변경 내용 확인
    # "--staged" 와 "--cached"는 같은 옵션 이다.
    $ git diff --staged
    diff --git a/README.md b/README.md
    index 92c48d0..e5beddf 100644
    --- a/README.md
    +++ b/README.md
    @@ -1,3 +1,5 @@
    +## DongDongE
    +
     # CTF 테스트
     # Python 3.X로 개발 진행 중

위 옵션(**—staged**)은 스테이징(**git add**)된 파일의 변경 내용을 확인합니다.

즉, 아까와 다르게 저장소에 커밋한 것과 스테이징에 있는 것을 비교합니다.

[요약]
git add 미 사용시, 수정사항 확인 ⇒ **git diff**

git add로 "Staging Area"시 수정사항 확인 ⇒  **git diff —staged** 사용

### [리모트 저장소에 PUSH 하기 - git push -u origin]

    $ git push -u <리모트 저장소 이름> <브랜치>
    
    
    # origin 저장소의 master라는 브랜치에 PUSH 한다.
    $ git push -u origin master

리모트 저장소에 프로젝트를 공유하고 싶을 때 PUSH를 사용하여 코드를 업로드할 수 있습니다.

하지만 해당 프로젝트가 다른 사람들과 공유되어 개발이 된다고 하면, Clone 이후 아무도 저장소에 PUSH를 하지 않았을 때 만 사용할 수 있습니다.

만약 팀원이 수정하여 PUSH를 먼저 하였다면, 팀원이 작업한 내역을 가져와야 하기에 "git pull" 명령어를 사용하여 동기화하고 PUSH 작업을 진행해야 합니다.

### [.gitignore 관리로 특정 파일 제외]

    $ vim .gitignore
    .Ds_store
    ./web_code/*.log
    *.exe
    *.deb
    __pycache__
    *.pyc
    .vscode

"**.gitignore**" 파일은 Git 버전 관리에서 제외할 파일 목록을 작성하여, 프로젝트 안에 불필요한 특정 파일을 제외하고 관리할 수 있다.

즉, log(.log 파일), Mac(.DS_Store), Window(Thumbs.db, .lnk, .cab, desktop.ini) 등 프로젝트에 불필요한 파일들을 제외한다.

패턴 규칙은, 표준 Glob 패턴(정규 표현식)을 사용하며, "#"를 통해 주석을 사용할 수 있습니다.

[*gitignore.io*](https://www.gitignore.io/)  또는 [*Github*](https://github.com/github/gitignore) 에서 자신의 프로젝트 환경에 맞는 언어 및 개발 환경에 맞는 예제를 제공해준다.

---

## 01-3 Git 환경 셋팅

### [Git config 설정]

Git 을 설치하고 Git의 사용 환경을 적절하게 한번만 설정하면 Git을 사용할 때 마다 유지되며, 권한 및 폴더별로 설정이 가능하다.

리눅스 기준 환경

- **/etc/gitconfig**
시스템의 모든 사용자와 모든 저장소에 적용되는 설정값을 설정할 수 있으며, "git config —system" 옵션으로 해당 파일을 읽고 쓸 수 있습니다.
- **~/.gitconfig, ~/.config/git/config**
전체 시스템에 적용하는 대신, 특정 사용자에게만 적용된다. "git config —global" 옵션으로 해당 파일을 읽고 쓸 수 있습니다.
- **.git/config
해당 작업 디렉터리(clone 및 해당 소스코드가 있는 디렉터리)에 있으며, 특정 저장소에만 작용된다.(.git 폴더에 존재)

해당 우선 순위는 역순으로 우선시 된다.**

### [사용자 정보 셋팅 - git config]

Git을 설치한 뒤 해당 작업을 먼저 진행해야 다음 작업을 할 수 있습니다. 

사용자의 닉네임과 이메일을 삽입하여 Git commit시 해당 사용자 정보도 같이 기입됩니다.

    $ git config --global user.name "DongDongE"
    
    
    $ git config --global user.email "DongDongE@d0ngd0nge.xyz"

옵션 중 "**global**" 옵션을 사용하기에 해당 사용자에게만 적용됩니다.
만약 공동 PC 작업 환경이 아니며, 본인만 이용한다면 위와 같이 global 옵션으로 지정하여 프로젝트를 진행하시는게 좋습니다.

하지만 디렉터리별 사용자를 다르게 설정해야 한다면 global 옵션 대신 해당 작업 디렉터리의 ".git/config" 마다 다르게 적용하는게 좋습니다.

### [편집기 설정]

Mac OS 기준

Git은 기본적으로 편집기를 "vim"으로 설정되어 있습니다. 하지만 GUI 환경인 Windows와 Mac OS 환경에서는 다소 불편함이 있어 해당 편집기를 vscode로 변경할 수 있습니다.

    $ git config --global core.editor core.editor=/Applications/Visual\ Studio\ Code.app/Contents/Resources/app/bin/code --wait

위와 같이 Mac OS 환경에 맞춰 기본 편집기를 Vscode로 설정하였습니다. 만약 notepad++, sublime text 등 사용자가 맞는 에디터를 사용하고 싶다면 위와 같이 해당 에디터로 설정 해주시면 됩니다.

### [환경 설정 확인하기]

    $ git config --list
    credential.helper=osxkeychain
    user.name=DongDongE
    user.email=DongDongE@d0ngd0nge.xyz
    core.editor=/Applications/Visual\ Studio\ Code.app/Contents/Resources/app/bin/code --wait
    ...

"git config —list" 명령어를 통하여 현재 설정된 값을 확인할 수 있습니다.

---

## 01-4 Git SSH Key 셋팅

### [SSH Key (Private, Public) 생성하기]

Github와 Gitlab에서는 기본적으로 HTTP/HTTPS로 매번 계정 정보를 입력하여 사용합니다. (귀찮는 사람들에게 SSH Key 등록을 추천 드립니다.)

하지만 SSH Key를 등록함으로 해당 PC에서는 아이디와 패스워드 절차 없이 자유롭게 리모트 저장소에 접근할 수 있습니다.

 Public Key (공개키)는 원격 저장소(Github, Gitlab)에 등록하고,
 Private Key (비밀키)는 해당 PC에 저장하게 됩니다.

* 공개키가 노출 되어도 상관없으나, 비밀키가 노출되면 해장 리모트 저장소에 접근 권한이 생기게 되므로 주의하셔야 합니다. (노출이 된다면 꼭 비밀키를 재생성 하시고 비밀키에 패스워드 설정을 해주십시오.)

    # 공개키와 비밀키 생성하기.
    # Windows 사용자는 git 프로그램의 ssh-keygen.exe 사용하시거나 Xshell 프로그램으로
    # 생성이 가능합니다.
    $ git ssh-keygen -t rsa -b 8192 -C "Macbook_Pro_Key"
    Generating public/private rsa key pair.
    Enter file in which to save the key (/Users/dongdonge/.ssh/id_rsa): ./id_rsa
    Enter passphrase (empty for no passphrase): # Private Key 패스워드 설정
    Enter same passphrase again:
    Your identification has been saved in ./id_rsa.
    Your public key has been saved in ./id_rsa.pub.
    The key fingerprint is:
    SHA256:6j4uOGdcqvz/IiI6XLeZgP82fMePcfHmPEEkYY2ze4g Macbook_Pro_Key
    The key's randomart image is:
    +---[RSA 8192]----+
    |           o+    |
    |          .+ o   |
    |            =    |
    |           . .   |
    |  .     S ..+    |
    | . o ... E ooo   |
    |. oo+o= .. ..o.  |
    |oo+.BOo. o+ +.   |
    |oooB+BB+o... o.  |
    +----[SHA256]-----+
    
    
    
    $ ls -la
    total 48
    drwxr-xr-x   8 dongdonge  staff   256  3 26 14:12 .
    drwxr-xr-x@ 15 dongdonge  staff   480  3 11 14:11 ..
    -rw-------   1 dongdonge  staff  6550  3 26 14:12 id_rsa      # Private Key
    -rw-r--r--   1 dongdonge  staff  1421  3 26 14:12 id_rsa.pub  # Public Key
    
    
    $ cat id_rsa
    -----BEGIN OPENSSH PRIVATE KEY-----
    b3BlbnNzaC1rZXktdjEAAAAACmFlczI1Ni1jdHIAAAAGYmNyeXB0AAAAGAAAABBSI4jlqh
    qyupCxiGpF4lSkAAAAEAAAAAEAAAQXAAAAB3NzaC1yc2EAAAADAQABAAAEAQDMMxcWlJo9
    QmhgMvyjUqfXAbyP05xSueBas8W509o2EA22v4IwLuGR1ODDPniV4JZWegaxxR0Dpb8WFZ
    cKxeI7u4hJP3q46C1JnhxiKuZ4nuQLSWQuCWjWgVgdTJlGCSyoIj2PaZJ3YxHhv2t3n7A1
    +kTm5SdqqAB/fBiLJY8NkNwAllo2kKrXDCboXTFhf/lPX6nSz8fxEB4O...............
    너무 길어서 생략
    -----END OPENSSH PRIVATE KEY-----
    
    
    $ cat id_rsa.pub
    ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAAEAQDMMxcWlJo9QmhgMvyjUqfXAbyP05xSueBa
    s8W509o2EA22v4IwLuGR1ODDPniV4JZWegaxxR0Dpb8WFZcKxeI7u4hJP3q46C1JnhxiKuZ4
    nuQLSWQuCWjWgVgdTJlGCSyoIj2PaZJ3YxHhv2t3n7A1+kTm5SdqqAB/fBiLJY8NkNwAllo2
    kKrXDCboXTFhf/lPX6nSz8fxEB4O1YsGhHS38IVLsY/jYDcRibmi7nblcuzDdFj8kaWFs6vE
    QfFmfAhIoinHKUc7ZoJey0D8G8rEk8akVonU2U0QTxrLkOr1/Og1/26ivavId/XFl9MpYD0S
    AA2LCiW5i8EGq3Lt7ywJonYsrXa.............................................
    너무 길어서 생략
    H8vXr0u6+5In Macbook_Pro_Key

"**ssh-keygen**" 명령어를 사용하여 Private Key와 Public Key를 생성하였습니다.

"**-t**" 옵션을 통해 알고리즘 RSA를 선언하고, "**-b**" 옵션으로 비트수 8192를 설정하며, "**-C**" 옵션으로 주석 메시지를 넣어 줍니다. 주석 메시지는 해당 키가 누가 사용하지는 간편하게 구별하는 메시지를 넣어 주면 좋습니다.

키를 생성할 때 "**Enter passphrase (empty for no passphrase):**"에서 비밀키에 대한 패스워드를 입력 하게 되면 매번 PUSH와 Pull 등 작업을 진행할 때 비밀키에 대한 패스워드를 입력 해야 합니다. 하지만 비밀키에 패스워드를 설정하지 않아도 됩니다. (입력하지 않고 엔터를 누르시면 됩니다.)

### [Private Key To Public Key 생성]

간혹 공개키를 서버에 등록하고 실수로 제거하는 경우 사용할 수 있습니다.

즉. Private Key(비밀키)만 존재하고 Public Key(공개키)가 없는 경우 비밀키로 공개키를 생성할 수 있습니다.

하지만 반대로 공개키를 가지고 비밀키를 생성할 수 없습니다.

    # ssh-keygen -y -f <비밀키 파일명> > <생성될 공개키 파일명>
    $ ssh-keygen -y -f "id_rsa" > "id_rsa.public"
    
    
    $ ls -la
    drwxr-xr-x  11 dongdonge  staff   352  3 26 14:40 .
    drwxr-xr-x@ 15 dongdonge  staff   480  3 11 14:11 ..
    -rw-------   1 dongdonge  staff  6550  3 26 14:12 id_rsa
    -rw-r--r--   1 dongdonge  staff  1405  3 26 14:38 id_rsa.public

### [SSH Private Key, config 등록 (사용자 PC)]

    # 윈도우 사용자
    $ vim C:\Users\<사용자 계정명>\.ssh\config
    Host git.d0ngd0nge.xyz
    		HostName git.d0ngd0nge.xyz
    		User git
    		IdentityFile "D:/Git_Key/id_rsa"
    
    
    
    # Mac OS 사용자
    $ vim ~/.ssh/config
    Host git.d0ngd0nge.xyz
    		HostName git.d0ngd0nge.xyz
    		User git
    		IdentityFile "~/Desktop/Git_Key/id_rsa"
    

각 "**.ssh**" 디렉터리에 "**config**" 파일에 위의 코드를 작성합니다. 만약 config 파일이 존재 하지 않는다면 생성하여 코드를 작성 해주시면 됩니다.

Host는 도메인 별로 지정하여 넣으시면 되고, "**User git**" 부분은 git 그대로 건들지 마시고, **IdentityFile** 부분에서 비밀키 위치를 정해 주시면 됩니다.

"**config**" 파일을 통해 여러 도메인의 비밀키 등록할 수 있으므로 유용합니다.

### [SSH Publick Key, Gitlab에 등록하기]

    # 공개키 파일 복사 하기
    $ cat id_rsa.pub
    ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAAEAQDMMxcWlJo9QmhgMvyjUqfXAbyP05xSueBas8W509o2EA22v4IwLuGR1ODDPniV4JZWegaxxR0Dpb8WFZcKxeI7u4hJP3q46C1JnhxiKuZ4nuQLSWQuCWjWgVgdTJlGCSyoIj2PaZJ3YxHhv2t3n7A1+kTm5SdqqAB/fBiLJY8NkNwAllo2kKrXDCboXTFhf/lPX6nSz8fxEB4O1YsGhHS38IVLsY/jYDcRibmi7nblcuzDdFj8kaWFs6vEQfFmfAhIoinHKUc7ZoJey0D8G8rEk8akVonU2U0QTxrLkOr1/Og1/26ivavId/XFl9MpYD0SAA2LCiW5i8EGq3Lt7ywJonYsrXaBneUA2CxmlIgLA+O8ukaMVskHMtYW1sq7QDH9K4wHIF+FtbfJORb95pksRMrmK7YmL7h1nUTauwnPrkf2wFkQsZ6LktGsSsv3/wHq4JJbMCkggjCkhHzud314k09Jxgivg+gZ5llqTCUXLzIto0exlROyF1LylQA5IaUsYWe/EROhgh9lq3MEPfMcRLA3ln7fq3julG1Ly051oMSDYKZoJ6VfqhII2ekGU99IBCdmRM69zBHPqp4GoFvGEt3+KBmwk3Z41p+4JqFGvBMAYv3Ko1z0gsrXw1Ws+XYeb+/U025HvxmXgpZVbKYtyJet1fKE2vsE9z45lTYct/G0UKGVm5xFR0XczxkMTQwqXjQLvL09aruayDmybbSqORbNAvmAysNOdV0/qo+STv4ui5xM/loAnBBX7e7J6Y206B1YKWoHA67M+gcL4+bGcMNxEIv+NPreuM6v0NF8HU5ZxNCtDM6vY70dVk7hPLXTu8besblAKkDe5toREQNV3mZfZOERbi665tjPvgV9Dyjs2CAGEvxR/alAphJ9NxRL8wxhFL8c3OPht1CcUVvkpju1Es4gc1wQDrOg9H9ReEano1IR45BF8ee2J+qwx2Rhk6btb2M/N4nMCBIxDUsHzRlo2cB1FAbB4Dxa5Bdz81wyvqjK4PP/PTixx/i3NEMcS2jCf/CaPECDrqN7iXgsDW1IhexZCXgLPVJQ7NvXYbwahrGoN72F9yZWKJbDjqWjXoIWklOK4dqUnOTGiCT/PqpparEMJDfPgwXSNAAnu5NqCy2g5d1UCpKtRYT/yH0XxfKLzl1UsBv3q4zSit87h8wBeEIG00+FGuE3f54rFEVMldiN2gP+pb1dmEzDBPPf2SJEQ9dQjD15KkFI1YHE2kSrcedxeW8MPAjl1kQPuWCoSM2YSrisHrlYVn+zOE31f3CgyahjWXFu5PssnBb6hg9b++vcmiiKBunY3SyQIaR8igxLWUZ7W4lj+PZPPyW2lGpnMHedH8vXr0u6+5In Macbook_Pro_Key

Github 또는 Gitlab에 SSH Keys 등록 하기 위해 공개키를 복사합니다.

![/assets/dong0304.png](/assets/dong0304.png)

복사한 공개키는 Gitlab에 붙여 넣어 주고 "Add Key"를 눌러 등록 합니다. 

여러 개의 Publick Key를 등록 할 수 있으므로, 각 다른 PC에 키를 추가적으로 생성하여 적용할 수 있습니다.

---

## 01-5 Git log, Commit 내용 확인

### [git log]

    $ git log
    commit db277747e96bdf126c9a4f1e21920803cf6cba97 (HEAD -> master, origin/master)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 11 17:39:41 2020 +0900
    
        OldTemplate_v09.10 Expert Docker Beta

최근 작성된  Commit hash, Version, 작성자, 날짜, 커밋 메시지, 커밋 로그 등 순서대로 확인 할 수 있습니다.

### [최근 커밋 내용, 수정 내용 확인 - git log -p]

    # -p -3 옵션을 통해 최근 세 개의 결과만 보여준다.
    $ git log -p -3
    commit 748f84c23bde0de966efb277ea6967ac3dcf4ff4 (HEAD -> master, origin/master, origin/HEAD)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Thu Mar 19 14:57:32 2020 +0900
    
        test.py 제거
    
    **diff --git a/test.py b/test.py
    deleted file mode 100644
    index fdabcde..0000000
    --- a/test.py
    +++ /dev/null**
    @@ -1,3 +0,0 @@
    -aa
    -bb
    -CC
    
    commit 1c857b756ba4fe0e13b14df504ce70a29a0b9984
    Merge: 1d67f7e d1dfe69
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 16:53:43 2020 +0900
    
        충돌 수정
    
    commit d1dfe69285b8c40d4ecb72a6909db3d7d16a0abb (dongdonge)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 16:50:52 2020 +0900
    
        dongdonge config
    
    diff --git a/config.py b/config.py
    index e109c75..d163f08 100644
    --- a/config.py
    +++ b/config.py
    @@ -1,7 +1,7 @@
     import os
    
     \#DEBUG                      =   True
    -DEBUG                       =   False
    +DEBUG                       =   dongdonfe_TrueM

해당 명령어를 통하여 "git diff" 처럼 커밋 내용 로그와 수정된 결과도 출력 합니다.

만약 팀 단위 프로젝트로 여러 동료가 커밋을 한다면 해당 명령을 통해 무엇이 변경 되었는지 쉽게 파악할 수 있습니다.

### [git log history 통계 정보 조회]

    $ git log --stat
    commit 748f84c23bde0de966efb277ea6967ac3dcf4ff4 (HEAD -> master, origin/master, origin/HEAD)
    Authoruthor: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Thu Mar 19 14:57:32 2020 +0900
    
        test.py 제거
    
     test.py | 3 ---
     1 file changed, 3 deletions(-)
    
    commit 1c857b756ba4fe0e13b14df504ce70a29a0b9984
    Merge: 1d67f7e d1dfe69
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 16:53:43 2020 +0900
    
        충돌 수정
    
    commit d1dfe69285b8c40d4ecb72a6909db3d7d16a0abb (dongdonge)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 16:50:52 2020 +0900
    
        dongdonge config
    
     config.py | 4 ++--
     1 file changed, 2 insertions(+), 2 deletions(-)
    
    commit 1d67f7ee6a00edf575c49ca6f523c3f225bb6352
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 16:50:01 2020 +0900
    
        master config
    
     config.py | 4 ++--
     1 file changed, 2 insertions(+), 2 deletions(-)

구체적인 변동 사항 대신 어떠한 파일이 변동(수정,삭제,추가 등) 되었는지 간단하게 커밋의 통계 정보를 조회할 수 있습니다.

### [git log oneline으로 조회 하기]

    $ git log --pretty=oneline
    748f84c23bde0de966efb277ea6967ac3dcf4ff4 (HEAD -> master, origin/master, origin/HEAD) test.py
    제거
    1c857b756ba4fe0e13b14df504ce70a29a0b9984 충돌 수정
    d1dfe69285b8c40d4ecb72a6909db3d7d16a0abb (dongdonge) dongdonge config
    1d67f7ee6a00edf575c49ca6f523c3f225bb6352 master config
    070c61922baca9afd67cbdde077a90710de9492d (origin/dongdonge) dongdonge branch test
    
    
    $ git log --oneline
    748f84c (HEAD -> master, origin/master, origin/HEAD) test.py 제거
    1c857b7 충돌 수정
    d1dfe69 (dongdonge) dongdonge config
    1d67f7e master config
    070c619 (origin/dongdonge) dongdonge branch test

각 커밋 내용을 한 라인으로 출력 되며, 많은 커밋 내용을 한번에 보기 쉽습니다.

"**—pretty=oneline**" 옵션은 hash의 모든 값이 출력되는 반면, "**—oneline**" 옵션에서는 부분적으로 hash를 표시합니다.

---

## 01-6 되돌리기

### [Commit 메시지 수정하여 다시 커밋 하기 - git commit —amend]

    $ git commit --amend
    re commit
    # 변경 사항에 대한 커밋 메시지를 입력하십시오. '#' 문자로 시작하는
    # 줄은 무시되고, 메시지를 입력하지 않으면 커밋이 중지됩니다.
    #
    # 시각:      Fri Mar 13 17:29:27 2020 +0900
    #
    # 현재 브랜치 master
    # 현재 브랜치와 'origin/master'이(가) 갈라졌습니다,
    # 다른 커밋이 각각 1개와 1개 있습니다.
    #   (리모트의 브랜치를 현재 브랜치로 병합하려면 "git pull"을 사용하십시오)
    #
    # 추적하지 않는 파일:
    #	123.py
    #
    # 변경 사항 없음

    $ git log
    commit 65444355de6f4e08fce7efbdebff14eb975b7605 (HEAD -> master)
    Merge: 3f6a792 f8d0eb0
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Fri Mar 13 17:29:27 2020 +0900
    
        re commit

일찍 커밋을 하였거나, 파일을 더 추가하거나, 커밋 메시지를 잘못 적어 수정하기 위해 사용됩니다.

### [Working Tree에서 수정한 파일 다시 되돌리기 - git checkout]

    $ cat 123.py
    import json
    
    
    $ vim 123.py
    import json
    import os
    
    print("testtest")

파일을 수정한 뒤 코드가 정상적으로 작동하지 않거나 이상 유무가 발생시 수정된 내용을 취소하고 가장 최신 버전상태로 되돌아 갈 때 사용됩니다.

즉, "git checkout" 명령어를 사용하여 작업 트리(Working Tree)에서 수정한 내용을 손쉽게 취소할 수 있습니다.

위 코드에서 "123.py" 코드에 "import os 문구와 print("testtest")"를 추가한 뒤 취소를 한다면 아래와 같이 명령어를 사용하면 됩니다.

    $ git checkout -- 123.py
    
    
    $ cat 123.py
    import json

주의!!
checkout으로 취소한 내용은 다시 복구할 수 없습니다.

### [스테이징 되돌리기 - git reset HEAD / git restore <file>]

    $ vim 123.py
    abcdefg 
    aaaaaaa # a 6개 문자열 추가
     
    
    $ git add 123.py # 123.py 스테이징
    
    
    $ git status
    현재 브랜치 master
    브랜치가 'origin/master'에 맞게 업데이트된 상태입니다.
    
    커밋할 변경 사항:
      (use "git restore --staged <file>..." to unstage)
    	수정함:        123.py   # 스테이징된 123.py
    
    
    # 스테이지에서 123.py만 내리기
    $ git reset HEAD 123.py
    리셋 뒤에 스테이징하지 않은 변경 사항:
    M	123.py
    
    
    $ git status
    현재 브랜치 master
    브랜치가 'origin/master'에 맞게 업데이트된 상태입니다.
    
    커밋하도록 정하지 않은 변경 사항:
      (무엇을 커밋할지 바꾸려면 "git add <파일>..."을 사용하십시오)
      (use "git restore <file>..." to discard changes in working directory)
    	수정함:        123.py
    
    커밋할 변경 사항을 추가하지 않았습니다 ("git add" 및/또는 "git commit -a"를
    사용하십시오)

"git add"를 통해 스테이징된 파일을 "git reset" 명령어를 통하여 취소할 수 있으며, "**git reset HEAD 123.py**" 명령어를 통해 "123.py" 파일이 스테이지에서 내려진걸 볼 수 있습니다.

"git reset" 명령어 대신 아래 예시처럼 "git restore" 함수를 대신 사용할 수 도 있습니다.

    $ git status
    현재 브랜치 master
    브랜치가 'origin/master'에 맞게 업데이트된 상태입니다.
    
    커밋할 변경 사항:
      (use "git restore --staged <file>..." to unstage)
    	수정함:        123.py   # 스테이징된 123.py
    
    
    # 스테이지에서 123.py만 내리기
    $ git restore README.md
    
    
    $ git status
    현재 브랜치 master
    브랜치가 'origin/master'에 맞게 업데이트된 상태입니다.
    
    커밋하도록 정하지 않은 변경 사항:
      (무엇을 커밋할지 바꾸려면 "git add <파일>..."을 사용하십시오)
      (use "git restore <file>..." to discard changes in working directory)
    	수정함:        123.py
    
    커밋할 변경 사항을 추가하지 않았습니다 ("git add" 및/또는 "git commit -a"를
    사용하십시오)

    $ git status
    현재 브랜치 master
    브랜치가 'origin/master'에 맞게 업데이트된 상태입니다.
    
    커밋할 변경 사항:
      (use "git restore --staged <file>..." to unstage)
    	수정함:        123.py
    	수정함:        run.py
    
    
    # 스테이지 목록에서 전부 내리기
    $ git reset HEAD
    리셋 뒤에 스테이징하지 않은 변경 사항:
    M	123.py
    M	run.py
    
    
    $ git status
    현재 브랜치 master
    브랜치가 'origin/master'에 맞게 업데이트된 상태입니다.
    
    커밋하도록 정하지 않은 변경 사항:
      (무엇을 커밋할지 바꾸려면 "git add <파일>..."을 사용하십시오)
      (use "git restore <file>..." to discard changes in working directory)
    	수정함:        123.py
    	수정함:        run.py
    
    커밋할 변경 사항을 추가하지 않았습니다 ("git add" 및/또는 "git commit -a"를
    사용하십시오)

하지만 스테이지 목록에서 여러 파일을 한번에 내리고 싶다면, 파일명 대신 HEAD 부분 까지만 사용하여 내릴 수 있습니다.

### [최근 커밋으로 되돌리기 - git reset HEAD^]

    $ git add run.py
    
    
    # 커밋 추가
    $ git commit -m "test run.py"
    [master 496ae2b] test run.py
     1 file changed, 1 insertion(+)
    
    
    # 커밋 내용 확인
    $ git log
    commit 496ae2ba0e1c611a85e9d7463c6d4ba922cd0073 (HEAD -> master)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:03:11 2020 +0900
    
        test run.py
    
    
    # 방금 스테이징된 커밋 취소
    $ git reset HEAD^
    리셋 뒤에 스테이징하지 않은 변경 사항:
    M	run.py
    
    
    # 방금 작성된 "test run.py" 커밋 취소된걸 볼 수 있습니다.
    $ git log
    commit b10fa6ba40e5cf67b961803a5169b15b06dbe9f2 (HEAD -> master, origin/master, origin/HEAD)
    Merge: aa68784 73d85c1
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Mon Mar 16 14:35:32 2020 +0900
    
        pool test

### [특정 커밋으로 되돌리기 - git reset commit hash]

    $ git log
    commit 6b9d639fcbef8f1275af03d27c238d677be6839b (HEAD -> master)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:12:30 2020 +0900
    
        test_5
    
    commit 0e0eeccaedd2663c03457eea7cf95cc9738e3bd5
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:12:21 2020 +0900
    
        test_4
    
    commit 54c43e45e39b09ab4099c2adcaa7c75fd2693c24
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:12:12 2020 +0900
    
        test_3
    
    commit 1fb596022cc45d7349b96a68575c949f544ca027
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:11:53 2020 +0900
    
        test_2
    
    commit 52712e7c2848bd6e170813044cd32e2f58f83247
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:11:29 2020 +0900
    
        test_1

(주의!)
해당 명령어는 커밋을 삭제함으로  중요한 데이터가 존재 시 확인 후 작업하시길 바랍니다.

현재 5개의 커밋이 존재하며, "**test_3**" 커밋으로 되돌려 보도록 하겠습니다. 

참고로 "test_3" Commit을 삭제 하는게 아니며 "**test_3**" 이후 커밋을 삭제하고 해당 커밋으로 이동합니다. 

    # "54c43e45e39b09ab4099c2adcaa7c75fd2693c24" hash는 "test_3" 커밋에 대한 hash 입니다.
    $ git reset --hard 54c43e45e39b09ab4099c2adcaa7c75fd2693c24
    HEAD의 현재 위치는 54c43e4입니다 test_3
    
    
    $ git log
    commit 54c43e45e39b09ab4099c2adcaa7c75fd2693c24 (HEAD -> master)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:12:12 2020 +0900
    
        test_3
    
    commit 1fb596022cc45d7349b96a68575c949f544ca027
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:11:53 2020 +0900
    
        test_2
    
    commit 52712e7c2848bd6e170813044cd32e2f58f83247
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:11:29 2020 +0900
    
        test_1

"**test_4**", "**test_5**" 커밋은 삭제되고 "**test_3**" 커밋 환경으로 되돌렸습니다.

### [커밋 삭제하지 않고 되돌리기 - git revert]

커밋을 삭제하고 싶다면 "git reset" 명령어를 사용하지만, 나중에 사용하기 위해 커밋을 되돌리더라도 커밋 내용을 남기기 위해 사용합니다.

    $ git log
    commit 97c5f368ca8f0cf2aa52a87d37fc5fcfa647b980 (HEAD -> master)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 14:02:46 2020 +0900
    
        test_5
    
    commit 8c0f2ff8fae0cb4adaaf8718d19dea30aea7a965
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 14:02:30 2020 +0900
    
        test_4
    
    commit 54c43e45e39b09ab4099c2adcaa7c75fd2693c24
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:12:12 2020 +0900
    
        test_3
    
    commit 1fb596022cc45d7349b96a68575c949f544ca027
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:11:53 2020 +0900
    
        test_2
    
    commit 52712e7c2848bd6e170813044cd32e2f58f83247
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:11:29 2020 +0900
    
        test_1

가장 최근인 "test_5" 커밋을 취소하고 "test_4" 커밋으로 되돌아 가기 위해 "test_5" 커밋 hash를 지정합니다.

    $ git revert 97c5f368ca8f0cf2aa52a87d37fc5fcfa647b980
    aaaaaaaaaa
    
    Revert "test_5"
    
    This reverts commit 97c5f368ca8f0cf2aa52a87d37fc5fcfa647b980.
    
    # 변경 사항에 대한 커밋 메시지를 입력하십시오. '#' 문자로 시작하는
    # 줄은 무시되고, 메시지를 입력하지 않으면 커밋이 중지됩니다.
    #
    # 현재 브랜치 master
    # 브랜치가 'origin/master'보다 6개 커밋만큼 앞에 있습니다.
    #   (로컬에 있는 커밋을 제출하려면 "git push"를 사용하십시오)
    #
    # 커밋할 변경 사항:
    #	수정함:        test.py
    #

git revert 명령어를 통하여 커밋 메시지 추가적으로 남겨둘 수 있습니다. (현재 aaaaaaaaaa를 추가적으로 작성함)

    $ git log
    commit a49d5f028db407ef4f037738609f7dda1ef4d43a
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 14:11:48 2020 +0900
    
        aaaaaaaaaa
    
        Revert "test_5"
    
        This reverts commit 97c5f368ca8f0cf2aa52a87d37fc5fcfa647b980.
    
    commit 97c5f368ca8f0cf2aa52a87d37fc5fcfa647b980
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 14:02:46 2020 +0900
    
        test_5
    
    commit 8c0f2ff8fae0cb4adaaf8718d19dea30aea7a965
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 14:02:30 2020 +0900
    
        test_4
    
    commit 54c43e45e39b09ab4099c2adcaa7c75fd2693c24
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:12:12 2020 +0900
    
        test_3
    
    commit 1fb596022cc45d7349b96a68575c949f544ca027
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:11:53 2020 +0900
    
        test_2
    
    commit 52712e7c2848bd6e170813044cd32e2f58f83247
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 17 12:11:29 2020 +0900
    
        test_1

"test_5"에서 변경했던 이력을 취소하고 새 커밋(Revert "test_5")을 만들었습니다.

---

## 01-7 Tag

태그는 보통 버전 릴리즈할 때 사용되며, 아래와 같이 2가지 태그로 분류 됩니다.

대규모 및 정식 버전으로 릴리즈 후 공개될 프로젝트에서는 많이 사용됩니다.

![/assets/dong0305.png](/assets/dong0305.png)

MS사의 Git Project 중 태그 리스트

### [Lightweight Tag - 일반 태그]

    $ git tag v0.1_web
    
    
    # 생성한 태크 확인 (태그 조회)
    $ git tag
    v0.1_web
    
    
    # 태그 정보 확인
    $ git show v0.1_web
    commit 94328b24e60f7eeec66224b636206a8d3d44b9db (HEAD -> master, tag: v0.1_web, origin/master, origin/HEAD)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 24 12:42:20 2020 +0900
    
        123.py
    
    **diff --git a/123.py b/123.py
    index 52fb42d..3cc2265 100644
    --- a/123.py
    +++ b/123.py**
    @@ -1 +1,2 @@
    -import json
    \ No newline at end of file
    +# test
    +import json

단순하게 태그 버전과 파일 커밋 체크섬만 저장하며, 그외 저장하지 않습니다.

단순한 태그를 원한다면 **Lightweight Tag**를 추천 드립니다.

### [Annotated Tag - 주석 태그]

    # 주석 태그 설정
    $ git tag -a v1.1_db -m "DB Version update"
    
    
    # 방금 생성한 태그 목록 확인 (태그 조회)
    $ git tag
    v0.1_web
    v1.1_db
    
    
    # 태그 내용 확인, 주석 내용과 작성자 아이디와 이메일, 시간을 학인할 수 있습니다.
    $ git show v1.1_db
    tag v1.1_db
    Tagger: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 24 15:34:09 2020 +0900
    
    DB Version update
    
    commit 94328b24e60f7eeec66224b636206a8d3d44b9db (HEAD -> master, tag: v1.1_db, tag: v0.1_web, origin/master, origin/HEAD)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 24 12:42:20 2020 +0900
    ㄴ
        123.py
    
    **diff --git a/123.py b/123.py
    index 52fb42d..3cc2265 100644
    --- a/123.py
    +++ b/123.py**
    @@ -1 +1,2 @@
    -import json
    \ No newline at end of file
    +# test
    +import json

Annotated Tag는 태그를 만든 사람의 아이디 및 이메일, 태그를 만든 날짜, 태그 메시지를 저장합니다.

또한 GPG(GNU Privacy Guard)로 서명할 수도 있습니다.

GPG는 저장소에 아무나 접근하지 못하게 하고 진짜 인가된 해당 사람에게만 커밋을 받기 위해 사용됩니다.

### [나중에 태그 하기]

    $ git log --oneline
    94328b2 (HEAD -> master, tag: v1.1_db, origin/master, origin/HEAD) 123.py
    748f84c test.py 제거
    1c857b7 충돌 수정
    d1dfe69 (dongdonge) dongdonge config
    1d67f7e master config
    070c619 (origin/dongdonge) dongdonge branch test

예전에 작업 했던 커밋 내용을 바탕으로 태그를 작성할 수 있습니다.

우선 "**git log**" 명령어를 통하여 커밋 내용과 해시를 확인하고, "**1c857b7 충돌 수정"의 커밋에 v1.0_db**"으로 태그를 명시 해보도록 하겠습니다. 

    # git tag -a <태그 버전> -m <메시지 내용> <커밋 해시>
    $ git tag -a v1.0_web -m "충돌 수정 코드가 업데이트 되었습니다." 1c857b7
    
    
    # git 태그 목록 확인
    $ git tag
    v1.0_db
    v1.1_db
    
    
    # 생성한 git 태그 상세 내역 확인
    $ git show v1.0_db
    tag v1.0_db
    Tagger: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Tue Mar 24 17:48:50 2020 +0900
    
    충돌 수정 코드가 업데이트 되었습니다.
    
    commit 1c857b756ba4fe0e13b14df504ce70a29a0b9984 (**tag: v1.0_we**b)
    Merge: 1d67f7e d1dfe69
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 16:53:43 2020 +0900
    
        충돌 수정

"git tag -a  <태그 버전> -m <메시지 내용> <커밋 해시>" 형태의 명령어를 사용하여 태그를 명시할 수 있습니다.

### [태그 리모트 저장소에 올리기]

    # "v1.0_db" 태그만 업로드
    $ git push -u origin v1.0_db
    오브젝트 나열하는 중: 1, 완료.
    오브젝트 개수 세는 중: 100% (1/1), 완료.
    오브젝트 쓰는 중: 100% (1/1), 168 bytes | 168.00 KiB/s, 완료.
    Total 1 (delta 0), reused 0 (delta 0)
    To https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git
     * [new tag]         v1.0_db -> v1.0_db
    

태그는 "**git push**" 명령어로 자동으로 태그가 업로드 되지 않습니다. 브랜치와 마찬가지로 사용자가 별도로 리모트 저장소에 직접 **push**를 해야 합니다.

    # 모든 태그 업로드
    $ git push -u origin --tags
    오브젝트 나열하는 중: 1, 완료.
    오브젝트 개수 세는 중: 100% (1/1), 완료.
    오브젝트 쓰는 중: 100% (1/1), 220 bytes | 220.00 KiB/s, 완료.
    Total 1 (delta 0), reused 0 (delta 0)
    To https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git
     * [new tag]         v0.1_web -> v0.1_web
     * [new tag]         v1.0_web -> v1.0_web

---

## 01-8 GPG, 내 작업에 서명하기 [커밋에 대한 무결성 증명]

저장소에 아무나 접근하지 못하게 하고 정상적으로 인가된 사람에게만 커밋을 받을 수 있습니다.

Git에 GPG 서명이 필요한 이유는 "**~/.gitconfig**" 파일에는 커밋을 한 사용자에 대한 정보가 적혀 있습니다.

정보는 아래와 같이 "**사용자명**"와 "**이메일**" 입니다.

    $ git config --list
    user.name=DongDongE
    user.email=DongDongE@d0ngd0nge.xyz

커밋시 위와 같이 사용자에 대한 정보는 필수이지만, GitHub와 Gitlab은 해당 사용자 정보가 맞는지는 검사하지 않습니다. (즉, 검사하는 단계 자체가 없습니다.) 물론 개인 프로젝트일 경우는 상관없으나 공개 프로젝트나 대규모 프로젝트에서는 꼭 GPG 사용을 권고하고 있습니다. 

아래는 관리자 처럼 변경하여 커밋한 내용입니다.

![/assets/dong0306.png](/assets/dong0306.png)

![/assets/dong0307.png](/assets/dong0307.png)

"DongDongE" 사용자를 "Administrator"로 변경하여 커밋시 사용자명이 변경된 걸 확인할 수 있습니다.

### [GPG 설치하기]

GPG는 기본적으로 설치가 되지 않기 때문에 별도로 GPG 모듈을 설치해야 합니다.

아래 링크를 통해 운영체제 별로 설치를 진행하시면 됩니다.

[GPG Installs](https://confluence.atlassian.com/bitbucketserver/using-gpg-keys-913477014.html) 

Windows와 Linux는 GPG 관련 에러가 발생하지 않았지만 Mac OS 에서는 GPG 환경 관련 에러가 발생하여 아래와 같이 설치시 환경 설정 작업을 해야 합니다.

    # Mac OS 기준
    
    $ brew install gpg
    $ brew cask install gpg-suite
    
    

### [GPG Key 생성하기]

    # GPG Key 목록을 확인합니다.
    $ gpg --list-keys
    
    
    # GPG Key 생성하기
    $ gpg --full-gen-key
    gpg (GnuPG) 2.2.20; Copyright (C) 2020 Free Software Foundation, Inc.
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.
    
    Please select what kind of key you want:
       (1) RSA and RSA (default)
       (2) DSA and Elgamal
       (3) DSA (sign only)
       (4) RSA (sign only)
      (14) Existing key from card
    Your selection? **1**
    RSA keys may be between 1024 and 4096 bits long.
    What keysize do you want? (2048) **4096**
    Requested keysize is 4096 bits
    Please specify how long the key should be valid.
             0 = key does not expire
          <n>  = key expires in n days
          <n>w = key expires in n weeks
          <n>m = key expires in n months
          <n>y = key expires in n years
    Key is valid for? (0) **1y**
    Key expires at 토  3/27 12:08:07 2021 KST
    Is this correct? (y/N) **y**
    Real name: **DongDongE**
    Email address: **DongDongE@d0ngd0nge.xyz**
    Comment: **DongDongE**
    You selected this USER-ID:
        "DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>"
    
    Change (N)ame, (C)omment, (E)mail or (O)kay/(Q)uit? **O**
    
    
    									 ┌──────────────────────────────────────────────────────┐
                       │ Please enter the passphrase to                       │
                       │ protect your new key                                 │
                       │                                                      │
                       │ Passphrase: ****____________________________________ │
                       │                                                      │
                       │       <OK>                              <Cancel>     │
                       └──────────────────────────────────────────────────────┘
    
    
    We need to generate a lot of random bytes. It is a good idea to perform
    some other action (type on the keyboard, move the mouse, utilize the
    disks) during the prime generation; this gives the random number
    generator a better chance to gain enough entropy.
    We need to generate a lot of random bytes. It is a good idea to perform
    some other action (type on the keyboard, move the mouse, utilize the
    disks) during the prime generation; this gives the random number
    generator a better chance to gain enough entropy.
    gpg: key C7547F6790E9C3BC marked as ultimately trusted
    gpg: revocation certificate stored as '/Users/dongdonge/.gnupg/openpgp-revocs.d/2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC.rev'
    public and secret key created and signed.
    
    pub   rsa4096 2020-03-27 [SC] [expires: 2021-03-27]
          2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC
    uid                      DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>
    sub   rsa4096 2020-03-27 [E] [expires: 2021-03-27]
    
    
    
    # 생성된 GPG Key 목록 확인
    $ gpg --list-keys
    -----------------------------------
    pub   rsa4096 2020-03-27 [SC] [expires: 2021-03-27]
          2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC
    uid           [ultimate] DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>
    sub   rsa4096 2020-03-27 [E] [expires: 2021-03-27]

GPG Key가 존재 하지 않아서 Key를 하나 생성하였습니다. 

알고리즘은 "**RSA**"를 사용하고 비트 수는 "**4096**"를 선택하였습니다. 또한 GPG 유효 기간을 선택할 수 있으나 안전하게 1년으로 설정하였습니다. (1년마다 새로운 GPG로 갱신하거나, 유효 기간이 없도록 설정할 수 있습니다.)

그리고 마지막으로 GPG Key에 대한 패스워드를 설정하면 마무리 됩니다.

### [GPG Public Key 등록]

    # GPG Key 목록 확인
    $ gpg --list-secret-keys --keyid-format LONG
    /Users/dongdonge/.gnupg/pubring.kbx
    -----------------------------------
    sec   rsa4096/**C7547F6790E9C3BC** 2020-03-27 [SC] [expires: 2021-03-27]
          2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC
    uid                 [ultimate] DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>
    ssb   rsa4096/AF4611FA4E2FCD4F 2020-03-27 [E] [expires: 2021-03-27]
    
    
    # GPG Public Key 복사
    $ gpg --armor --export **C7547F6790E9C3BC**
    -----BEGIN PGP PUBLIC KEY BLOCK-----
    
    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
    ay3hvoHoa94Q5KwhXA3VT0IXMljYMnE=
    =+7x8
    -----END PGP PUBLIC KEY BLOCK-----

위 GPG public key를 복사합니다.

![/assets/dong0308.png](/assets/dong0308.png)

복사한 GPG Public Key를 Gitlab, Github 계정에 Key 목록을 추가 합니다.

### [GPG Key와 Git 연결하기]

GPG Key 생성하여 Public Key를 Github와 Gitlab에 등록 하였다면, 사용을 위해 현재 PC의 Git 설정에 GPG키를 연동해야 합니다. (로컬 PC 에서 GPG Key를 생성 했다고 자동으로 연동되지 않습니다. * GPG와 Git은 서로 다른 프로그램 입니다.)

    # 연동할 GPG Key 검색 (키 생성 당시 이메일로 조회)
    $ gpg --list-secret-keys --keyid-format LONG DongDongE@d0ngd0nge.xyz
    sec   rsa4096/**C7547F6790E9C3BC** 2020-03-27 [SC] [expires: 2021-03-27]
          2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC
    uid                 [ultimate] DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>
    ssb   rsa4096/AF4611FA4E2FCD4F 2020-03-27 [E] [expires: 2021-03-27]
    
    
    # git 설정에 GPG 키 정보 등록한다.
    # 만약 특정 저장소만 (git 폴더별로) 설정하고 싶다면 "--global" 옵션은 빼주시면 됩니다.
    # which gpg 명령으로 git 환경에 gpg 경로를 설정합니다.
    $ g **C7547F6790E9C3BC
    $** git config --global gpg.program $(which gpg)
    
    ****
    $ git config --list
    user.name=DongDongE
    user.email=DongDongE@d0ngd0nge.xyz
    **user.signingkey=C7547F6790E9C3BC**
    core.editor=/Applications/Visual\ Studio\ Code.app/Contents/Resources/app/bin/code --wait

위 명령어를 통해 Git 설정 파일에 GPG Key를 연동하였습니다.

태그에 서명할 때와 커밋에 서명할 때는 "**-S**" 옵션 플래그를 사용하여 서명을 하여야 합니다.

    # Git 태그에 "-s" 옵션으로 태그에 서명
    $ git tag -s v1.0 -m "태그입니다."
    
    
    # Git 커밋시 "-S" 옵션으로 서명
    $ git commit -S -m "커밋입니다."
    ┌────────────────────────────────────────────────────────────────┐
    │ Please enter the passphrase to unlock the OpenPGP secret key:  │
    │ "DongDongE (DongDongE) <dongdonge@d0ngd0nge.xyz>"              │
    │ 4096-bit RSA key, ID **C7547F6790E9C3BC**,                         │
    │ created 2020-03-30.                                            │
    │                                                                │
    │                                                                │
    │ Passphrase: __________________________________________________ │
    │                                                                │
    │         <OK>                                    <Cancel>       │
    └────────────────────────────────────────────────────────────────┘
    [master 0ebf61d] 커밋입니다.
     1 file changed, 2 insertions(+), 1 deletion(-)

커밋 또는 태그시 매번 수동으로 "**-S**", "**-s**" 옵션을 입력해야 하지만 아래 옵션을 통해 자동으로 적용할 수 있으며

Mac OS 사용자는 필수적으로 환경 변수를 별도로 등록해야 에러가 발생 하지 않습니다.

    # 매번 커밋와 태그에 "-S", "-s" 옵션 없어도 항상 서명을 적용하는 옵션입니다. (설정을 추전드립니다.)
    $ git config --global commit.gpgsign true
    
    
    # Mac OS 터미널에서 git 서명시 에러가 발생함으로 아래 옵션으로 터미널 환경변수를 등록합니다.
    $ vim ~/.zshrc      또는     vim ~/.bashrc
    export GPG_TTY=$(tty)
    
    
    # 변경한 환경 변수 적용
    $ source ~/.zshrc   또는    source ~/.bashrc

![/assets/dong0309.png](/assets/dong0309.png)

서명된 태그 

GPG를 통해 정상적으로 서명한다면 위와 같이 "검증됨"으로 표시 되며, 사용자에 대한 무결성을 입증할 수 있습니다.

### [GPG Key 리스트(Public/Private) 조회]

    # Public Key 조회
    $ gpg --list-keys
    /Users/dongdonge/.gnupg/pubring.kbx
    -----------------------------------
    **pub**   rsa4096 2020-03-27 [SC] [expires: 2021-03-27]
          2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC
    uid           [ultimate] DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>
    sub   rsa4096 2020-03-27 [E] [expires: 2021-03-27]
    
    
    
    # Private Key 조회
    $ gpg --list-secret-key
    /Users/dongdonge/.gnupg/pubring.kbx
    -----------------------------------
    **sec**   rsa4096 2020-03-27 [SC] [expires: 2021-03-27]
          2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC
    uid           [ultimate] DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>
    ssb   rsa4096 2020-03-27 [E] [expires: 2021-03-27]

### [GPG Key 삭제하기]

키 삭제는 반드시 Private Key 부터 진행해야 합니다.

    # Private Key 제거
    $ gpg --delete-secret-key 2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC
    gpg (GnuPG) 2.2.20; Copyright (C) 2020 Free Software Foundation, Inc.
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.
    
    
    sec  rsa4096/C7547F6790E9C3BC 2020-03-27 DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>
    
    Delete this key from the keyring? (y/N) **y**
    This is a secret key! - really delete? (y/N) **y**
    
    
    # Public Key 제거
    $ gpg --delete-key 2BEDC9FE6AA9BF14CF678246C7547F6790E9C3BC
    gpg (GnuPG) 2.2.20; Copyright (C) 2020 Free Software Foundation, Inc.
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.
    
    
    pub  rsa4096/C7547F6790E9C3BC 2020-03-27 DongDongE (DongDongE) <DongDongE@d0ngd0nge.xyz>
    
    Delete this key from the keyring? (y/N) **y**
    

---

# 02. Git Branch

Branch 작업은 공통 협업, 하나의 코드로 여러 결과물 도출에 주로 사용하며, 개인적인 프로젝트를 혼자 진행한다면 굳이 사용하지 않아도 됩니다. (추후 병합(Merge) 작업에서 여러 팀원이 같은 동일한 파일을 수정 시 충돌로 수동으로 잡아야 합니다.)

## 02-1 Branch

브랜치는 기존 소스를 그대로 두고, 새로운 소스코드를 더하여 작업을 진행할 수 있습니다.

즉, Branch는 나무가 가지에서 새로운 줄기를 뻗어 나가 여러 갈래로 퍼지는 데이터 흐름을 말합니다.

![/assets/dong0310.png](/assets/dong0310.png)

"브랜치 분기 과정"의 사진처럼 기존 소스코드 "master" Branch에 그대로 유지하면서 기존 파일의 내용을 수정/추가로 Branch에서 뻗어 나와 새로운 Branch를 만들 수 있습니다. (새로운 branch 생성은 분기라고 합니다.)

![/assets/dong0311.png](/assets/dong0311.png)

예를 들어 "A" Branch의 a1와 a2에서 게시판 삭제/수정 페이지를 만들고, "B" Branch의 "b1"에서 게시판 리스트 기능을 만들었다고 가정한다면 "병합 (merge)"를 통해 master 최종 합칠 수 있습니다.

### [새로운 브랜치 생성하기]

    $ git branch dongdonge
    
    
    $ git branch
    dongdonge
    * master
    
    
    # -v 옵션으로 브랜치마다 마지막 커밋 메시지를 확인할 수 있습니다.
    $ git branch -v
    dongdonge 
    * master    94328b2 123.py

"**git branch <브랜치명>**"를 사용하여 생성하고자 하는 브랜치를 적어주며, 현재 "**dongdonge**" 라는 이름의 브랜치를 생성하였습니다.

"**master**" branch 앞에 ***** 표시는 아직 branch가 master로 선택되어 해당 브랜치에서 작업을 하고 있다는 뜻입니다.

    $ git log
    commit c139b8562018f882dba55b6f64e28afd501e319b (HEAD -> master, dongdonge)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 12:45:07 2020 +0900
    
        work_4
    
    commit ad7f249c7c871134a4f9f6aafddbcae7225aa5db
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 12:44:52 2020 +0900
    
        work_3

여태까지는 커밋 로그에서 (HEAD → master)로 표시 되었지만, 이번에 "dongdonge" 브랜치를 추가하면서 (HEAD → master, dongdonge) 표기 되었습니다.

참고로 HEAD는 현재 작업하고 있는 브랜치를 가리키는 포인터 입니다.

### [브랜치 이동하기 - git checkout]

    $ git checkout dongdonge
    
    
    $ git branch
    * dongdonge
      master
    
    
    $ git log
    commit c139b8562018f882dba55b6f64e28afd501e319b (HEAD -> dongdonge, master)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 12:45:07 2020 +0900
    
        work_4
    
    commit ad7f249c7c871134a4f9f6aafddbcae7225aa5db
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 12:44:52 2020 +0900
    
        work_3

브랜치 사이 이동하기 위해 "checkout" 명령을 사용할 수 있습니다. 현재 master 브랜치에서 dongdonge 브랜치로 이동하였습니다.

"git log" 명령어를 통해 (HEAD가 dongdonge) 브랜치를 가리키는걸 볼 수 있습니다.

### [새로운 브랜치에 커밋 하기]

    $ git checkout dongdonge
    
    
    $ vim dongdonge.txt
    DongDongE file 1234
    
    
    $ vim dongdonge_test.txt
    DongDongE_test.txt 1234
    
    
    $ git add .
    
    
    $ git commit -m "dongdonge branch test"
    [dongdonge 070c619] dongdonge branch test
     2 files changed, 2 insertions(+)
     create mode 100644 dongdonge.txt
     create mode 100644 dongdonge_test.txt
    
    
    $ ls -la
    drwxr-xr-x  24 dongdonge  staff   768  3 18 15:24 .
    drwxr-xr-x   4 dongdonge  staff   128  3 16 16:17 ..
    
    
    $ git log --oneline
    070c619 (HEAD -> dongdonge) dongdonge branch test
    c139b85 (master) work_4
    ad7f249 work_3
    e1039f2 work_2
    c444ef8 work_1

"**dongdonge**" branch에서 "**dongdonge.txt**" 파일과 "**dongdonge_test.txt**" 파일을 만들어 스테이징후 커밋을 하였습니다.  해당 브랜치에서 추가적으로 생성 및 수정된 파일은 다른 브랜치에서 동기화가 되지 않습니다. (추후 동기화 및 합치는 작업은 git 병합작업인 merge을 하여야 합니다.)

    $ git checkout master
    'master' 브랜치로 전환합니다
    브랜치가 'origin/master'보다 11개 커밋만큼 앞에 있습니다.
      (로컬에 있는 커밋을 제출하려면 "git push"를 사용하십시오)
    
    
    $ ls -la
    drwxr-xr-x  22 dongdonge  staff   704  3 18 15:22 .
    drwxr-xr-x   4 dongdonge  staff   128  3 16 16:17 ..
    -rw-r--r--   1 dongdonge  staff    20  3 18 15:24 dongdonge.txt
    -rw-r--r--   1 dongdonge  staff    24  3 18 15:24 dongdonge_test.txt
    
    
    $ git log --oneline
    c139b85 (HEAD -> master) work_4
    ad7f249 work_3
    e1039f2 work_2
    c444ef8 work_1

위와 같이 브랜치마다 커밋와 환경을 따로 가지며, checkout을 통해 브랜치 사이 환경을 이동하면서 작업을 진행할 수 있습니다.

### [브랜치 사이의 차이점 확인하기]

브랜치 마다 커밋이 많아 질 경우, 브랜치 사이에 어떠한 차이가 있는지 일일이 확인하기 어려워집니다.

브랜치 이름 사이에 마침표 두 개를 사용하여 차이점을 확인할 수 있습니다.

    $ git log master..dongdonge
    commit 070c61922baca9afd67cbdde077a90710de9492d (HEAD -> dongdonge)
    Author: DongDongE <DongDongE@d0ngd0nge.xyz>
    Date:   Wed Mar 18 15:11:13 2020 +0900
    
        dongdonge branch test

마침표 왼쪽에 있는 브랜치(master)를 기준으로 오른쪽 브랜치(dongdonge)를 기준으로 오른쪽 브랜치와 비교합니다.

위 결과값을 통해 "master" 브랜치에는 없고, "dongdonge" 브랜치에만 존재하는 커밋 내용을 보여줍니다.

---

## 02-2 브랜치 병합하기 - merge

각각 만들어진 브랜치를 하나의 프로젝트로 작업을 합치기 위해 기존 브랜치와 병합을 해야 합니다.

또한 각 브랜치에서 공통적인 파일을 수정했다면 병합시 충돌이 발생하여 해결을 해야 합니다.

    $ git checkout master
    
    
    $ git merge dongdonge
    업데이트 중 c139b85..070c619
    Fast-forward
     dongdonge.txt      | 1 +
     dongdonge_test.txt | 1 +
     2 files changed, 2 insertions(+)
     create mode 100644 dongdonge.txt
     create mode 100644 dongdonge_test.txt

"master" 브랜치로 체크아웃을 진행하고, master 브랜치에 "dongdonge" 브랜치를 가져와 병합 작업을 진행합니다.

### [병합이 끝난 브랜치 삭제하기]

    $ git branch -d dongdonge
    dongdonge 브랜치 삭제 (과거 070c619).
    
    
    # 해당 브랜치를 강제로 삭제한다.
    $ git branch -D dongdonge

브랜치를 병합(merge)한 후 더 이상 사용하지 않는 브랜치는 제거할 수 있습니다.

하지만 제거는 완전히 지워지는 것이 아니라 단순히 감추는 원리이며, 다시 삭제한 브랜치 이름으로 생성한다면 이전내용을 볼 수 있습니다.

또한 로컬 저장소 브랜치를 삭제와 원격 저장소 브랜치를 삭제하는 방법은 다르며, 해당 내용은 "로컬 브랜치"를 삭제합니다.

---

## 02-2 Remote Branch

### [원격 저장소 Branch 조회]

    # git ls-remote <remote>로 사용 합니다.
    $ git ls-remote origin
    From https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git
    94328b24e60f7eeec66224b636206a8d3d44b9db	HEAD
    070c61922baca9afd67cbdde077a90710de9492d	refs/heads/dongdonge
    94328b24e60f7eeec66224b636206a8d3d44b9db	refs/heads/master
    42acdb57d6eb6f2ed1c3339c799b208ab5170112	refs/heads/white
    8c0f2ff8fae0cb4adaaf8718d19dea30aea7a965	refs/keep-around/8c0f2ff8fae0cb4adaaf8718d19dea30aea7a965
    97c5f368ca8f0cf2aa52a87d37fc5fcfa647b980	refs/keep-around/97c5f368ca8f0cf2aa52a87d37fc5fcfa647b980
    db277747e96bdf126c9a4f1e21920803cf6cba97	refs/keep-around/db277747e96bdf126c9a4f1e21920803cf6cba97
    94328b24e60f7eeec66224b636206a8d3d44b9db	refs/tags/v0.1_web
    d41a84cf18603b17aade738467ca232b5f3345c5	refs/tags/v1.0_web
    1c857b756ba4fe0e13b14df504ce70a29a0b9984	refs/tags/v1.0_web^{}
    903b462560025cf594d665e5469e272c4424f46a	refs/tags/v1.1_db
    94328b24e60f7eeec66224b636206a8d3d44b9db	refs/tags/v1.1_db^{}

원격 저장소(Github, Gitlab)서버에서 "**Remote Refs**"를 조회 할 수 있습니다.

위와 같이 브랜치 내역과 태그 내역을 확인 할 수 있습니다.

### [Git 원격 저장소, Remote Branch 가져오기]

Git을 통하여 팀원 또는 그룹 단위로 공유하여 협업 프로젝트를 진행한다면 원격 저장소에 있는 Branch를 로컬 저장소로 가져와야 하는 경우 사용됩니다.

리모트 저장소를 그대로 Clone, Pull 하여도 생성된 Branch는 자동으로 다운로드 및 생성이 되지 않습니다.

사용자가 수동으로 직접 생성된 Branch를 원격 저장소에서 가져와야 합니다.

    # 원격 저장소 갱신 합니다.
    # 만약 새로운 브랜치가 존재시 아래와 같이 출력됩니다.
    $ git remote update
    https://git.d0ngd0nge.xyz:3005/DongDongE/Flask_Development_Web.git URL에서
     * [새로운 브랜치]   white       -> origin/white
    
    
    # (로컬 저장소 브랜치 조회), 현재 로컬 저장소에는 master 브랜치만 존재한다.
    $ git branch
    * master
    
    
    # (원격 저장소 브랜치 조회), 원격 저장소에는 dongdonge, master, white 브랜치 존재한다.
    $ git branch -r
    	origin/HEAD -> origin/master
      origin/dongdonge
      origin/master
      origin/white
    
    
    # 원격 저장소 + 로컬 저장소 조회, (remotes/orgin/브랜치)에서 remotes는 원격 저장소를 의미
    $ git branch -a
    * master
      remotes/origin/HEAD -> origin/master
      remotes/origin/dongdonge
      remotes/origin/master
      remotes/origin/whitehat

"**git branch -r**" 명령을 통해 원격 저장소에 존재하는 브랜치를 조회 합니다.

    # 원격 저장소에 존재하는 'dongdonge' 브랜치를 로컬 저장소에 저장
    # git checkout -t <리모트 저장소/브랜치명>
    $ git checkout --track origin/dongdonge
    'dongdonge' 브랜치가 리모트의 'dongdonge' 브랜치를 ('origin'에서) 따라가도록 설정되었습니다.
    새로 만든 'dongdonge' 브랜치로 전환합니다
    
    
    # 로컬 저장소에 저장된 'dongdonge' 브랜치 확인
    $ git branch
    * dongdonge
      master

'**git checkout —track'** 명령을 사용하여 원격 저장소에 존재하는 브랜치를 로컬 저장소의 브랜치로 checkout 할 수 있습니다.

또한 옵션 명령은 "**—track**" 명령은 줄여서 "**-t**"로 사용 가능합니다.

    # -b 옵션을 사용하여 원격 저장소 브랜치 "white"를 로컬 저장소에서는 "black"으로 저장
    $ git checkout -b black origin/white
    'black' 브랜치가 리모트의 'white' 브랜치를 ('origin'에서) 따라가도록 설정되었습니다.
    새로 만든 'black' 브랜치로 전환합니다
    
    
    $ git branch
    * black
      dongdonge
      master

"**git checkout -b**" 명령을 사용하여 원격 저장소에 존재하는 브랜치 "**white**"를 로컬 저장소로 checkout시 "**black**"으로 저장합니다.  해당 옵션은 아래 사진와 같이 단점이 존재합니다.

![/assets/dong0312.png](/assets/dong0312.png)

만약 팀원와 기존 브랜치명을 이어서 개발을 진행하고자 할 때 "**-b**" 옵션으로 브랜치명을 변경하여 Push를 진행할 시 새로운 브랜치가 생성됩니다.

### [Git 원격 저장소, Remote Branch 삭제]

    # 원격 리모트 업데이트 (브랜치 목록 가져오기)
    $ git remote update
    origin을(를) 가져오는 중
    
    
    # 원격 저장소 브랜치 조회
    $ git branch -r
    	origin/HEAD -> origin/master
      origin/black
      origin/dongdonge
      origin/master
      origin/ppap
      origin/white
    
    
    # 원격 저장소에 존재하는 브랜치중 "back" 브랜치 제거
    $ git push origin --delete black
    - [deleted]         black
    
    
    # 원격 저장소에 존재하는 브랜치중 "ppap" 브랜치 제거
    $ git push origin --delete ppap
    - [deleted]         ppap

해당 브랜치 제거 작업은 "**리모트 원격 저장소**"에 존재하는 브랜치만 제거 하는 작업입니다.

즉, 아래와 같이 "**git push <리모트 저장소> —delete <제거할 브랜치명>**" 명령어를 사용한다면 로컬 저장소의 브랜치는 남아 있고, 원격 저장소의 브랜치만 제거 됩니다.

    # "로컬 저장소"와 "원격 저장소"의 브랜치 목록 조회
    $ git branch -a
    	black     # 로컬 저장소에서는 해당 브랜치가 제거 되지 않았음
      dongdonge
    * master
      ppap      # 로컬 저장소에서는 해당 브랜치가 제거 되지 않았음
      update
      white
      remotes/origin/HEAD -> origin/master
      remotes/origin/dongdonge
      remotes/origin/master
      remotes/origin/white