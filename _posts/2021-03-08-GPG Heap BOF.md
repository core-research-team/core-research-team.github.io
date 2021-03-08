---
layout: post
title:  "GPG, Heap BOF in Libgcrypt"
author: "myria"
comments: true
tags: [heap, analysis]
---

라온화이트햇 핵심연구팀 권재승

## 소개

이 문서는 구글 프로젝트 제로팀에서 발견한 Libgcrypt 1.9.0 암호화 라이브러리의 `힙 버퍼 오버플로우`에 대한 설명 및 분석 글입니다. 


### GnuPG 및 Libgcrypt

![/assets/202103/jae_.png](/assets/202103/jae_.png)

> `GnuPG`는 GNU 프로젝트 중 이메일 보안 표준으로 자리잡은 PGP를 대체하는 소프트웨어이다. 여기서 데이터 암호화 및 전자서명을 위해 Libcrypt 암호화 라이브러리를 사용하며, OpenSSL이나 LibreSSL만큼 널리 사용되는 암호화 툴킷은 아니나 Fedora나  Gento와 같은 많은 리눅스 배포판, 맥OS 패키지 관리자인 Homebrew에서 디지털 보안 목적으로 사용되고 있다. Libgcrypt는 기본적인 암호화 모드 및 알고리즘을 지원하며 GnuPG와 분리되어 개발되어 독립적으로 사용 가능하다.


## 환경세팅

Libgcrypt 1.9.0에서 발생한 `힙 오버플로우`를 재현하기 위한 환경세팅 방법입니다. 해당 취약점은 오직 1.9.0버전(released 2021-01-19)에만 영향을 미치기 때문에 정확히 해당 버전을 설치해줄 필요가 있습니다. 

Libgcrypt 1.9.0 환경 구성은 Ubuntu 18.04에서 진행하였으며 아래 스크립트를 사용하면 취약점이 있는 GPG 소프트웨어를 설치할 수 있습니다.

```python
#!/bin/bash
# ---------
# Script to build and install GnuPG 2.2.x	
# Setting libgcrypt-1.9.0

apt-get update
#  libgnutls-dev
apt-get -y install bzip2 make gettext texinfo gnutls-bin libgnutls28-dev build-essential libbz2-dev zlib1g-dev libncurses5-dev libsqlite3-dev libldap2-dev || apt-get -y install libgnutls28-dev bzip2 make gettext texinfo gnutls-bin build-essential libbz2-dev zlib1g-dev libncurses5-dev libsqlite3-dev libldap2-dev
mkdir -p /var/src/gnupg22 && cd /var/src/gnupg22
gpg --list-keys
gpg --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 249B39D24F25E3B6 04376F3EE0856959 2071B08A33BD3F06 8A861B1C7EFD60D9

wget -c https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.41.tar.bz2 && \
wget -c https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.41.tar.bz2.sig && \
wget -c https://gnupg.org/ftp/gcrypt/libgcrypt/libgcrypt-1.9.0_do_not_use.tar.bz2 && \
wget -c https://gnupg.org/ftp/gcrypt/libgcrypt/libgcrypt-1.9.0_do_not_use.tar.bz2.sig && \
wget -c https://gnupg.org/ftp/gcrypt/libassuan/libassuan-2.5.4.tar.bz2 && \
wget -c https://gnupg.org/ftp/gcrypt/libassuan/libassuan-2.5.4.tar.bz2.sig && \
wget -c https://gnupg.org/ftp/gcrypt/libksba/libksba-1.5.0.tar.bz2 && \
wget -c https://gnupg.org/ftp/gcrypt/libksba/libksba-1.5.0.tar.bz2.sig && \
wget -c https://gnupg.org/ftp/gcrypt/npth/npth-1.6.tar.bz2 && \
wget -c https://gnupg.org/ftp/gcrypt/npth/npth-1.6.tar.bz2.sig && \
wget -c https://gnupg.org/ftp/gcrypt/pinentry/pinentry-1.1.1.tar.bz2 && \
wget -c https://gnupg.org/ftp/gcrypt/pinentry/pinentry-1.1.1.tar.bz2.sig && \
wget -c https://gnupg.org/ftp/gcrypt/gnupg/gnupg-2.2.27.tar.bz2 && \
wget -c https://gnupg.org/ftp/gcrypt/gnupg/gnupg-2.2.27.tar.bz2.sig && \
tar -xjf libgpg-error-1.41.tar.bz2 && \
tar -xjf libgcrypt-1.9.0_do_not_use.tar.bz2 && \
tar -xjf libassuan-2.5.4.tar.bz2 && \
tar -xjf libksba-1.5.0.tar.bz2 && \
tar -xjf npth-1.6.tar.bz2 && \
tar -xjf pinentry-1.1.1.tar.bz2 && \
tar -xjf gnupg-2.2.27.tar.bz2 && \
cd libgpg-error-1.41/ && ./configure && make && make install && cd ../ && \
cd libgcrypt-1.9.0 && ./configure && make && make install && cd ../ && \
cd libassuan-2.5.4 && ./configure && make && make install && cd ../ && \
cd libksba-1.5.0 && ./configure && make && make install && cd ../ && \
cd npth-1.6 && ./configure && make && make install && cd ../ && \
cd pinentry-1.1.1 && ./configure --enable-pinentry-curses --disable-pinentry-qt4 && \
make && make install && cd ../ && \
cd gnupg-2.2.27 && ./configure && make && make install && \
echo "/usr/local/lib" > /etc/ld.so.conf.d/gpg2.conf && ldconfig -v && \
echo "Complete!!!"
```

Libgcrypt 1.9.0의 자동 업데이트 및 기존 스크립트에서의 다운을 방지하기 위해 FTP서버의 파일이름이 `libgcrypt-1.9.0_do_not_use`로 변경되어있습니다.

위 스크립트를 통해 설치 및 업데이트를 진행하고 나면 다음과 같이 `gpg --version` 을 통해  취약한 버전이 설치된 것을 확인할 수 있습니다. 

![/assets/202103/jae_1.png](/assets/202103/jae_1.png)


## 취약점 분석

Libcrypt 1.9.0의 블록 버퍼 관리 코드에서 잘못된 가정으로 인해 `힙 버퍼 오버플로우`가 발생하게 됩니다. 공격자는 라이브러리에 특수 제작된 데이터 블록을 전송하여 공격을 유발할 수 있으며, 블럭의 일부 데이터가 단순히 복호화되는 것만으로도 힙 버퍼 오버플로우가 발생합니다. 

libgcrypt의 hash-common.c를 보면 _gcry_md_block_write에서 다음과 같이 블록 버퍼의 점유 공간(`hd->count`)이 블록크기를 초과할 수 없다고 가정합니다.

```c
// libgcrypt/cipher/hash-common.c:150
129 size_t copylen;
...
149	  copylen = inlen;
150	  if (copylen > blocksize - hd->count)
151	    copylen = blocksize - hd->count;
```

그러나 만약 `blocksize`보다 `hd->count`값이 더 크게된다면, 언더플로우가 발생하게 될 것입니다. 이 후, `hd->count` 및 `copylen`은 memcpy의 wrapper함수인 buf_cpy로 전달됩니다.

```c
// libgcrypt/cipher/hash-common.c
152		if (copylen == 0)
153	    break;
154
155	  buf_cpy (&hd->buf[hd->count], inbuf, copylen);
156	  hd->count += copylen;
```

 _gcry_md_block_write에서는 `hd->count`값이 `blocksize`를 넘을수 없다고 가정했지만, libgcrypt의 sha1 코드를 보면, sha1_final을 통해 count값을 blocksize를 넘게끔 설정하는 부분을 찾아볼 수 있습니다.

```c
// libgcrypt/cipher/sha1.c
545		else  /* need one extra block */
546		{
547			hd->bctx.buf[hd->bctx.count++] = 0x80; /* pad character */
548			/* fill pad and next block with zeroes */
549			memset (&hd->bctx.buf[hd->bctx.count], 0, 64 - hd->bctx.count + 56);
550			hd->bctx.count = 64 + 56;
```

여기서 `count`값은 120으로 설정됩니다.  hd 구조체는 다음과 같으므로, count값과 copylen이 클수록 오버플로우가 날 확률도 높아집니다.

```c
/* SHA1 needs 2x64 bytes and SHA-512 needs 128 bytes. */
#define MD_BLOCK_CTX_BUFFER_SIZE 128

typedef struct gcry_md_block_ctx
{
    byte buf[MD_BLOCK_CTX_BUFFER_SIZE];
    MD_NBLOCKS_TYPE nblocks;
    MD_NBLOCKS_TYPE nblocks_high;
    int count;
    unsigned int blocksize_shift;
    _gcry_md_block_write_t bwrite;
} gcry_md_block_ctx_t;
```

이제 sha1_final 이후, `count`값이 120으로 설정되고 _gcry_md_block_write를 호출하게 될 경우, blocksize는 여전히 64이기 때문에 `blocksize - hd->count`는 언더플로우로 copylen을 설정하는 if문을 들어가지 않게 됩니다. 

그렇다면 `buf_cpy`는 전체입력을 버퍼에 복사하게 되고, count값이 120이므로, copylen이 8이상이라면 힙 오버플로우가 발생합니다.

```c
155	  buf_cpy (&hd->buf[hd->count], inbuf, copylen);
```

또한 오버플로우된 버퍼에서 조금 떨어진 곳에 있는 bwrite는 함수 포인터로, _gcry_md_block_write에서 버퍼 복사를 수행한 후 `hd->bwrite(hd, inbuf, inblocks)`를 호출합니다. 때문에 공격자가 오버플로우를 통해 `bwrite`를 덮어 프로그램 흐름을 쉽게 컨트롤할 수 있습니다.
```c
// libgcrypt/cipher/hash-common.c
166		if (inlen >= blocksize)
167    {
168      inblocks = inlen >> blocksize_shift;
169      nburn = hd->bwrite (hd, inbuf, inblocks);
```

이제 [PoC파일](https://bugs.chromium.org/p/project-zero/issues/attachment?aid=486952&signed_aid=hXelt91sJk0bhikNO2DOBA==) 실행 결과, 재현된 취약점을 gdb를 통해 살펴보도록하겠습니다.

gpg에 pwndbg를 붙여 `hash-common.c+150`, `sha1.c+550`에 중단점을 설정하였습니다. 

먼저 아래 부분에서 sha1.c의 550행에 접근하여 `hd->bctx.count = 64 + 56`을 실행하여 count값이 120이 된 것을 확인할 수 있었습니다.

![/assets/202103/jae_2.png](/assets/202103/jae_2.png)

다음으로 `hash-common.c+150`에 중단점이 걸렸을 때, `copylen`, `blocksize`, `hd->count`의 값입니다.

![/assets/202103/jae_3.png](/assets/202103/jae_3.png)

언더플로우로 인해 copylen은 재설정되지않고, 그대로 buf_cpy를 실행하여 복사가 수행되게 됩니다.

![/assets/202103/jae_4.png](/assets/202103/jae_4.png)

![/assets/202103/jae_5.png](/assets/202103/jae_5.png)

이후 다음번 _gcry_md_block_write 호출에서 `hd->bwrite`함수를 호출되게 됩니다.

## 패치

hash-common.c의  _gcry_md_block_write에 다음 코드가 추가되었습니다. `hd->count`의 값이 `blocksize`를 넘지 못하게하여 위와 같은 취약점이 발생하는 것을 방어하였습니다.

![/assets/202103/jae_6.png](/assets/202103/jae_6.png)

## 결론

이 취약점은 트리거를 위해 어떠한 인증(verification)이나 서명(signature)도 필요하지 않기 때문에 심각한 취약점으로 받아들여졌습니다. 현재 익스플로잇을 위한 암호 데이터 블럭의 구성 방법은 90일의 공개 마감(2021-01-29 기준)에 따르기 때문에 그 내용이 알려지지는 않았습니다. 하지만 취약점 자체가 간단하기 때문에 공개된 분석 내용과 PoC 파일을 이용한다면 쉽게 악용할 수 있을 것으로 보입니다. 때문에 영향을 받는 버전인 libgcrypt 1.9.0을 사용하고 있다면 빠른 패치를 해야할 것입니다.
## Reference

- [https://bugs.chromium.org/p/project-zero/issues/detail?id=2145](https://bugs.chromium.org/p/project-zero/issues/detail?id=2145)
- [https://github.com/gpg/libgcrypt/blob/f9d8b5a0369cc94e125d36d9c8864d5cd2eaa1d2/cipher/sha1.c#L550](https://github.com/gpg/libgcrypt/blob/f9d8b5a0369cc94e125d36d9c8864d5cd2eaa1d2/cipher/sha1.c#L550)
- [https://github.com/gpg/libgcrypt/blob/f9d8b5a0369cc94e125d36d9c8864d5cd2eaa1d2/cipher/sha1.c#L550](https://github.com/gpg/libgcrypt/blob/f9d8b5a0369cc94e125d36d9c8864d5cd2eaa1d2/cipher/sha1.c#L550)
- [https://gist.github.com/vt0r/a2f8c0bcb1400131ff51](https://gist.github.com/vt0r/a2f8c0bcb1400131ff51)
- [https://lists.gnupg.org/pipermail/gnupg-announce/2021q1/000456.html](https://lists.gnupg.org/pipermail/gnupg-announce/2021q1/000456.html)
- [https://gist.github.com/vt0r/a2f8c0bcb1400131ff51](https://gist.github.com/vt0r/a2f8c0bcb1400131ff51)
- [https://ko.wikipedia.org/wiki/GNU_프라이버시_가드](https://ko.wikipedia.org/wiki/GNU_%ED%94%84%EB%9D%BC%EC%9D%B4%EB%B2%84%EC%8B%9C_%EA%B0%80%EB%93%9C)
- [https://en.wikipedia.org/wiki/Libgcrypt](https://en.wikipedia.org/wiki/Libgcrypt)