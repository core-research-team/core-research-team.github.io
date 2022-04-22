---
layout: post
title: "Android 환경 Frida 탐지"
author: "vulhack"
comments: true
tags: [Android, Frida]
---

라온화이트햇 핵심연구팀 김현수

## 1. 개요

모바일 취약점 진단 시, Frida 의 활용도와 의존도는 매우 높은 편입니다. 공격자는 Frida를 이용하여 코드 흐름을 변조하고 원하는 악의적인 행위(Intergrity Bypass, SSL Pinning, Rooting Bypass, Frida Bypass, Business Logic Tampering)를 가능하게 합니다. 

금융앱이나 모바일 앱 보안 솔루션 ****의 경우 Frida의 탐지는 필수적이며 Frida 의 사용을 제한합니다. 어떻게 Frida 를 탐지할 수 있는지 알아보겠습니다.

## 2. Frida?

![Untitled](/assets/2022-04-01/Untitled.png)

Frida 는Ole가 개발한 DBI(Dynamic Binary Instrumentation) 프레임워크 입니다. Frida 는 Windows, macOS, GNU/Linux , iOS, Android 환경에서 동작이 가능하며 제공되는 Frida API(Javascript, C, Swift) 를 이용하여 공격자가 원하는 코드를 작성하여 후킹, 함수추적 등의 행위가 가능합니다.

## 3. Frida Detection

### **3.1 Frida Binary 탐지**

```c
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <android/log.h>

#define  LOG_TAG  "FRIDA_DETECT"

int is_frida_binary() {
   DIR            *dir_info;
   struct dirent  *dir_entry;
	 char *ptr;

   dir_info = opendir("/data/local/tmp");              
   if ( NULL != dir_info)
   {
      while(dir_entry = readdir(dir_info)){ 
         ptr = strstr(dir_entry->d_name, "frida");
				 if(ptr != NULL){
					  __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "FRIDA DETECTION [1]: Exits File : %s", dir_entry->d_name);
				 }
      }
      closedir(dir_info);
   }
	 return 0;
}
int main(){
	is_frida_binary();
  return 0;
}
```

> **Code Description**
> 

![Untitled](/assets/2022-04-01/Untitled%201.png)

코드 테스팅 환경은 Galaxy S9 디바이스에 진행되었으며, /data/local/tmp 경로에 frida 관련 binary 가 존재합니다.

![Untitled](/assets/2022-04-01/Untitled%202.png)

코드 실행 시 frida 관련 파일이 탐지되는것을 확인할 수 있습니다. 

Android Device 에서 Frida 실행 시, 일반적으로 /data/local/tmp 디렉터리 내에서 frida-server 를 실행하게 됩니다. 해당 코드는 Frida Server 파일이 존재하는지 확인합니다. 위와 같은 방법은 파일 명 과 경로를 변경하는 것 만으로 간단히 우회 될 수 있습니다.

### **3.2 Frida D-Bus 탐지**

```c
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>

#define  LOG_TAG  "FRIDA_DETECT"

int is_frida_dbus() {
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    inet_aton("127.0.0.1", &(sa.sin_addr));
    int sock;
    int fd;
    char res[7];
    int num_found;
    int ret;
    int i;
    while (1) {
        /*
         * 1: Frida Server Detection.
         */
        for(i = 20000 ; i <= 65535 ; i++) {
			
            sock = socket(AF_INET , SOCK_STREAM , 0);
            sa.sin_port = htons(i);

            if (connect(sock , (struct sockaddr*)&sa , sizeof sa) != -1) {
                memset(res, 0 , 7);

                send(sock, "\x00", 1, 0);
                send(sock, "AUTH\r\n", 6, 0);

                usleep(100); // Give it some time to answer

                if ((ret = recv(sock, res, 6, MSG_DONTWAIT)) != -1) {
                    if (strcmp(res, "REJECT") == 0) {
                        __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "FRIDA DETECTED [2] - frida server running on port %d!", i);
                    }
                }
            }

            close(sock);
        }
	}
}
int main(){
	is_frida_dbus();
	return 0;
}
```

> **Code Description**
> 

Frida 는 D-Bus 프로토콜을 사용하여 통신하기 때문에, Android Device 에서 오픈된 Port 에 대해 D-Bus 인증 메시지를 보내어 Frida Server 의 실행 여부를 확인 할 수 있습니다. 

### **3.3 Frida Server 탐지**

Frida 서버는 일반적으로 [localhost](http://localhost) 의 27042 Port 를 사용하기 때문에, Frida Server 를 확인하여 Frida 의 실행 여부를 확인할 수 있습니다.

```c
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>

#define  LOG_TAG  "FRIDA_DETECT"

int is_frida_server_listening() {
    struct sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(27042);
    inet_aton("127.0.0.1", &(sa.sin_addr));
    int sock = socket(AF_INET , SOCK_STREAM , 0);
    if (connect(sock , (struct sockaddr*)&sa , sizeof sa) != -1) {
				__android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "FRIDA DETECTION [3]: Server connection successful 27042\n");
      return 1;
    }
    return 0;
}
int main(){
	is_frida_server_listening();
  return 0;
}
```

> **Code Description**
> 

![Untitled](/assets/2022-04-01/Untitled%203.png)

netstat 명령을 통해 네트워크 상태를 확인한 결과 Frida 서버의 Default Port 인 27042 Port 를 통해 ESTABLISHED(연결) 상태로 frida-server 가 동작 중인 것을 확인할 수 있습니다. 

![Untitled](/assets/2022-04-01/Untitled%204.png)

[localhost](http://localhost):27042(Frida Server) 에 대한 연결 요청 결과 정상적으로 연결된 것을 확인할 수 있습니다.

```bash
./frida-server -l 0.0.0.0:7777
```

위의 탐지 방법은 Frida Server 의 Default Port 번호를 변경하는 것으로 우회할 수 있습니다.

### **3.4 Frida library 탐지**

Frida Runtime 에 의해 메모리에 Mapping 되는 라이브러리를 확인하여 Frida 라이브러리를 확인하는 방법 입니다.

```c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>

#define  LOG_TAG  "FRIDA_DETECT"

int is_frida_library() {
	char line[512];
	FILE* fp;
	fp = fopen("/proc/self/maps", "r");
	if (fp) {
	    while (fgets(line, 512, fp)) {
	        if (strstr(line, "frida")) {
            __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "FRIDA DETECTION [4]: /proc/self/maps : %s\n",line);
	        }
	    }
	    fclose(fp);
	    } else {
	       /* Error opening /proc/self/maps. If this happens, something is off. */
	    }
	}
  return 0;
}
int main(){
	is_frida_library();
  return 0;
}
```

> **Code Description**
> 

![Untitled](/assets/2022-04-01/Untitled%205.png)

Frida 를 분석 대상 Application 에 inject 하게 되면 위와 같이 [Frida-agent-64.so](http://Frida-agent-64.so) 라이브러리가 메모리에 매핑되는것을 확인할 수 있습니다. 

![Untitled](/assets/2022-04-01/Untitled%206.png)

코드를 실행하게 되면, 위와 같이 Frida 라이브러리가 탐지된 것을 확인할 수 있습니다. 위의 탐지 방법은 Frida 라이브러리의 이름을 변경하여 우회가 가능합니다.

### **3.5 Frida Segment 탐지**

frida-gadget 및 frida-agent의 모든 버전에 있는 "LIBFRIDA" 문자열을 검색합니다. 다음 코드는 '/proc/self/maps' 에 있는 모든 실행 가능한 세그먼트를 스캔 합니다.

```c
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <android/log.h>

#define  LOG_TAG  "FRIDA_DETECT"
#define MAX_LINE 512
static char keyword[] = "LIBFRIDA";

int find_mem_string(unsigned long start, unsigned long end, char *bytes, unsigned int len) {

    char *pmem = (char*)start;
    int matched = 0;

    while ((unsigned long)pmem < (end - len)) {

        if(*pmem == bytes[0]) {

            matched = 1;
            char *p = pmem + 1;

            while (*p == bytes[matched] && (unsigned long)p < end) {
                matched ++;
                p ++;
            }

            if (matched >= len) {
                return 1;
            }
        }

        pmem ++;

    }
    return 0;
}
int scan_executable_segments(char * map) {
    char buf[512];
    unsigned long start, end;

    sscanf(map, "%lx-%lx %s", &start, &end, buf);

    if (buf[2] == 'x') {
        return (find_mem_string(start, end, (char*)keyword, 8) == 1);
    } else {
        return 0;
    }
}
int read_one_line(int fd, char *buf, unsigned int max_len) {
    char b;
    ssize_t ret;
    ssize_t bytes_read = 0;

    memset(buf, 0, max_len);

    do {
        ret = read(fd, &b, 1);

        if (ret != 1) {
            if (bytes_read == 0) {
                // error or EOF
                return -1;
            } else {
                return bytes_read;
            }
        }

        if (b == '\n') {
                return bytes_read;
        }

        *(buf++) = b;
        bytes_read += 1;

    } while (bytes_read < max_len - 1);

    return bytes_read;
}
int main() {
		char map[MAX_LINE];
		int num_found;
		int fd;
        if ((fd = openat("AT_FDCWD", "/proc/self/maps", O_RDONLY, 0)) >= 0) {

            while ((read_one_line(fd, map, MAX_LINE)) > 0) {
                if (scan_executable_segments(map) == 1) {
                    num_found++;
                }
            }

            if (num_found > 1) {
                __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, "FRIDA DETECTED [5] - suspect string found in memory!");
            }

        } else {
				 __android_log_print(ANDROID_LOG_VERBOSE, APPNAME,("Error opening /proc/self/maps. That's usually a bad sign.");

        }

        sleep(3);
}
```

> **Code Description**
> 

Frida의 라이브러리에서 발견되는 frida-agent의 모든 버전에 있는 "LIBFRIDA" 문자열을 검색합니다. libc 내장함수 후킹을 방지하기 위해, openat() 과 같은 함수를 inline assembly, syscall 등으로 대체할 수 있습니다. 또한 여러 아티팩트 스캔을 통해 효율을 높일 수 있습니다.

### **3.6 기타 탐지 방법**

> Process 탐지
> 

![Untitled](/assets/2022-04-01/Untitled%207.png)

frida-server 가 구동하게 되면, 위와 같이 Process 확인을 통해 frida-server 가 구동중인지 여부를 확인할 수 있습니다.  

> Network Connection 탐지
> 

![Untitled](/assets/2022-04-01/Untitled%208.png)

frida-server 를 사용하는 경우 원격 프로세스에 주입 된 frida-agent 간의 통신은 파이프를 통해 frida-server와 통신합니다. 연결된 파이프 확인을 통해 frida-server 가 구동 중인지 여부를 확인할 수 있습니다. 

## 4. 결론

Frida 를 detect 하는 다양한 방법에 대해 알아보았습니다. 흔히 알려져 있고 실제 민감한 Application에서도 위와 같은 방법을 사용하곤 합니다. Frida 를 탐지하지만 Frida 를 이용해 탐지를 우회하는(?) 방법이 수없이 존재합니다. 

libc 내장함수를 사용한 Frida, rooted, jailbreak 의 탐지 방법은 쉽게 구현되지만 쉽게 우회될수 있습니다. 다양한 Frida 탐지 루틴을 적용하고 inline assembly, syscall 등을 사용하여 공격자가 Frida 를 이용하여 우회 하는 것을 어렵게 하는 것이 도움이 될 것으로 생각됩니다.

## 5. 참조

[https://github.com/darvincisec/DetectFrida/blob/master/app/src/main/c/native-lib.c](https://github.com/darvincisec/DetectFrida/blob/master/app/src/main/c/native-lib.c)

[https://github.com/muellerberndt/frida-detection/blob/master/AntiFrida/app/src/main/cpp/native-lib.cpp](https://github.com/muellerberndt/frida-detection/blob/master/AntiFrida/app/src/main/cpp/native-lib.cpp)

[https://www.romainthomas.fr/post/20-09-r2con-obfuscated-whitebox-part1/](https://www.romainthomas.fr/post/20-09-r2con-obfuscated-whitebox-part1/)

[https://darvincitech.wordpress.com/2019/12/23/detect-frida-for-android/](https://darvincitech.wordpress.com/2019/12/23/detect-frida-for-android/)

[https://mobile-security.gitbook.io/mobile-security-testing-guide/android-testing-guide/0x05j-testing-resiliency-against-reverse-engineering#bypassing-debugger-detection](https://mobile-security.gitbook.io/mobile-security-testing-guide/android-testing-guide/0x05j-testing-resiliency-against-reverse-engineering#bypassing-debugger-detection)