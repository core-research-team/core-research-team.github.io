---
layout: post
title: "Dockerfile, Docker-Compose Project"
author: "DongDongE"
comments: true
tags: [programming, docker]
---

라온화이트햇 핵심연구팀 유선동([dongdonge@d0ngd0nge.xyz](mailto:dongdonge@d0ngd0nge.xyz))


## [Introduce]

Docker-Compose를 사용하여 여러 도커 컨테이너를 통해 환경을 구축시 컨테이너 프로세스 충돌로 인하여 정상 구동이 되지 않아 충돌 문제 관련 연구를 하였습니다.

### [Dockerfile 최적화 및 Docker Container 충돌 문제 연구]

```docker
version: "3.5"

services:
    apache_web:
        image: apache_web
        container_name: apache_web
        restart: always
        depends_on:
            - mysql_db
        links: 
            - mysql_db:mysql_db
        build:
            context: ./apache_web
            dockerfile: Dockerfile
        networks:
            - pentest
        ports: 
            - "127.0.0.1:1127:80"

    mysql_db:
        image: mysql_db
        container_name: mysql_db
        restart: always
        build:
            context: ./mysql_db
            dockerfile: Dockerfile
        environment:
            - MYSQL_ROOT_PASSWORD=@@D@ngD@ngE!
            - MYSQL_DATABASE=db_test
            - MYSQL_USER=DongDongE
            - MYSQL_PASSWORD=D@ngD@ngE
        networks:
            - pentest
        

    flask_web:
        image: flask_web
        container_name: flask_web
        restart: always
        depends_on:
            - mysql_db
            - apache_web
        links: 
            - apache_web:apache_web
        build:
            context: ./apache_web
            dockerfile: Dockerfile
        networks:
            - pentest

    bots:
        image: bots
        container_name: bots
        restart: always
        depends_on:
            - apache_web
            - mysql_db
            - flask_web
        links: 
            - apache_web:apache_web
        build:
            context: ./bots
            dockerfile: Dockerfile
        networks:
            - pentest

networks:
    pentest:
        driver: bridge
```

**Docker-compose.yml - (내부 속성은 더미 사용)**

- `image` : 생성될 이미지 이름 **또는** 사용될 이미지
- `container_name` : 생성될 컨테이너 이름
- `restart: always` : 컨테이너 중단시 자동으로 재시작
- `environment` : 해당 컨테이너에 환경변수 설정
- `depends_on` :  기입된 해당 컨테이너 부터 부팅
- `networks` : 네트워크 생성
- `ports :  "127.0.0.1:1127:80"` : 컨테이너로 포트 포워딩 (127.0.0.1일 경우 로컬에서만 접근 가능)

위 옵션을 토대로 "**networks**"를 사용하여 Docker-Compose.yml에 묶여 있는 컨테이너마다 하나의 망을 제공(각각 기관마다 망을 분리하기 위해) 합니다.

도커 컨테이너가 프로세스(MySQL, Apache)가 비정상적으로 종료시 간혹 컨테이너도 죽는 현상이 있어 컨테이너가 꺼질시 자동으로 켜주기 위해 "**restart: always**"를 사용합니다.

또한 컨테이너 부팅 순서를 `DB` → `Web` → `Flask` → `Bot` 순서로 부팅 되어야 에러가 발생하지 않습니다. 하지만 도커 에서는 컨테이너를 우선순위 지정을 "**depends_on**" 로 지정할 수 있지만 컨테이너가 부팅 되고 해당 프로세스도 정상적으로 올라갔는지는 검사하지 않습니다.  해당 문제를 해결 하기 위해 아래 "**wait-for**" 스크립트를 사용 하였습니다.

[Wait-for-it.sh]

```bash
#!/usr/bin/env bash
# Use this script to test if a given TCP host/port are available

WAITFORIT_cmdname=${0##*/}

echoerr() { if [[ $WAITFORIT_QUIET -ne 1 ]]; then echo "$@" 1>&2; fi }

usage()
{
    cat << USAGE >&2
Usage:
    $WAITFORIT_cmdname host:port [-s] [-t timeout] [-- command args]
    -h HOST | --host=HOST       Host or IP under test
    -p PORT | --port=PORT       TCP port under test
                                Alternatively, you specify the host and port as host:port
    -s | --strict               Only execute subcommand if the test succeeds
    -q | --quiet                Don't output any status messages
    -t TIMEOUT | --timeout=TIMEOUT
                                Timeout in seconds, zero for no timeout
    -- COMMAND ARGS             Execute command with args after the test finishes
USAGE
    exit 1
}

wait_for()
{
    if [[ $WAITFORIT_TIMEOUT -gt 0 ]]; then
        echoerr "$WAITFORIT_cmdname: waiting $WAITFORIT_TIMEOUT seconds for $WAITFORIT_HOST:$WAITFORIT_PORT"
    else
        echoerr "$WAITFORIT_cmdname: waiting for $WAITFORIT_HOST:$WAITFORIT_PORT without a timeout"
    fi
    WAITFORIT_start_ts=$(date +%s)
    while :
    do
        if [[ $WAITFORIT_ISBUSY -eq 1 ]]; then
            nc -z $WAITFORIT_HOST $WAITFORIT_PORT
            WAITFORIT_result=$?
        else
            (echo > /dev/tcp/$WAITFORIT_HOST/$WAITFORIT_PORT) >/dev/null 2>&1
            WAITFORIT_result=$?
        fi
        if [[ $WAITFORIT_result -eq 0 ]]; then
            WAITFORIT_end_ts=$(date +%s)
            echoerr "$WAITFORIT_cmdname: $WAITFORIT_HOST:$WAITFORIT_PORT is available after $((WAITFORIT_end_ts - WAITFORIT_start_ts)) seconds"
            break
        fi
        sleep 1
    done
    return $WAITFORIT_result
}

wait_for_wrapper()
{
    # In order to support SIGINT during timeout: http://unix.stackexchange.com/a/57692
    if [[ $WAITFORIT_QUIET -eq 1 ]]; then
        timeout $WAITFORIT_BUSYTIMEFLAG $WAITFORIT_TIMEOUT $0 --quiet --child --host=$WAITFORIT_HOST --port=$WAITFORIT_PORT --timeout=$WAITFORIT_TIMEOUT &
    else
        timeout $WAITFORIT_BUSYTIMEFLAG $WAITFORIT_TIMEOUT $0 --child --host=$WAITFORIT_HOST --port=$WAITFORIT_PORT --timeout=$WAITFORIT_TIMEOUT &
    fi
    WAITFORIT_PID=$!
    trap "kill -INT -$WAITFORIT_PID" INT
    wait $WAITFORIT_PID
    WAITFORIT_RESULT=$?
    if [[ $WAITFORIT_RESULT -ne 0 ]]; then
        echoerr "$WAITFORIT_cmdname: timeout occurred after waiting $WAITFORIT_TIMEOUT seconds for $WAITFORIT_HOST:$WAITFORIT_PORT"
    fi
    return $WAITFORIT_RESULT
}

# process arguments
while [[ $# -gt 0 ]]
do
    case "$1" in
        *:* )
        WAITFORIT_hostport=(${1//:/ })
        WAITFORIT_HOST=${WAITFORIT_hostport[0]}
        WAITFORIT_PORT=${WAITFORIT_hostport[1]}
        shift 1
        ;;
        --child)
        WAITFORIT_CHILD=1
        shift 1
        ;;
        -q | --quiet)
        WAITFORIT_QUIET=1
        shift 1
        ;;
        -s | --strict)
        WAITFORIT_STRICT=1
        shift 1
        ;;
        -h)
        WAITFORIT_HOST="$2"
        if [[ $WAITFORIT_HOST == "" ]]; then break; fi
        shift 2
        ;;
        --host=*)
        WAITFORIT_HOST="${1#*=}"
        shift 1
        ;;
        -p)
        WAITFORIT_PORT="$2"
        if [[ $WAITFORIT_PORT == "" ]]; then break; fi
        shift 2
        ;;
        --port=*)
        WAITFORIT_PORT="${1#*=}"
        shift 1
        ;;
        -t)
        WAITFORIT_TIMEOUT="$2"
        if [[ $WAITFORIT_TIMEOUT == "" ]]; then break; fi
        shift 2
        ;;
        --timeout=*)
        WAITFORIT_TIMEOUT="${1#*=}"
        shift 1
        ;;
        --)
        shift
        WAITFORIT_CLI=("$@")
        break
        ;;
        --help)
        usage
        ;;
        *)
        echoerr "Unknown argument: $1"
        usage
        ;;
    esac
done

if [[ "$WAITFORIT_HOST" == "" || "$WAITFORIT_PORT" == "" ]]; then
    echoerr "Error: you need to provide a host and port to test."
    usage
fi

WAITFORIT_TIMEOUT=${WAITFORIT_TIMEOUT:-15}
WAITFORIT_STRICT=${WAITFORIT_STRICT:-0}
WAITFORIT_CHILD=${WAITFORIT_CHILD:-0}
WAITFORIT_QUIET=${WAITFORIT_QUIET:-0}

# Check to see if timeout is from busybox?
WAITFORIT_TIMEOUT_PATH=$(type -p timeout)
WAITFORIT_TIMEOUT_PATH=$(realpath $WAITFORIT_TIMEOUT_PATH 2>/dev/null || readlink -f $WAITFORIT_TIMEOUT_PATH)

WAITFORIT_BUSYTIMEFLAG=""
if [[ $WAITFORIT_TIMEOUT_PATH =~ "busybox" ]]; then
    WAITFORIT_ISBUSY=1
    # Check if busybox timeout uses -t flag
    # (recent Alpine versions don't support -t anymore)
    if timeout &>/dev/stdout | grep -q -e '-t '; then
        WAITFORIT_BUSYTIMEFLAG="-t"
    fi
else
    WAITFORIT_ISBUSY=0
fi

if [[ $WAITFORIT_CHILD -gt 0 ]]; then
    wait_for
    WAITFORIT_RESULT=$?
    exit $WAITFORIT_RESULT
else
    if [[ $WAITFORIT_TIMEOUT -gt 0 ]]; then
        wait_for_wrapper
        WAITFORIT_RESULT=$?
    else
        wait_for
        WAITFORIT_RESULT=$?
    fi
fi

if [[ $WAITFORIT_CLI != "" ]]; then
    if [[ $WAITFORIT_RESULT -ne 0 && $WAITFORIT_STRICT -eq 1 ]]; then
        echoerr "$WAITFORIT_cmdname: strict mode, refusing to execute subprocess"
        exit $WAITFORIT_RESULT
    fi
    exec "${WAITFORIT_CLI[@]}"
else
    exit $WAITFORIT_RESULT
fi
```

wait-for-it.sh, [https://github.com/vishnubob/wait-for-it.git](https://github.com/vishnubob/wait-for-it.git)

Docker API 문서에도 해당 문제는 이슈로 등록되어 있었습니다. 현재 검증된 해결방법으로는 "[wait-for-it](https://github.com/vishnubob/wait-for-it.git)" 스크립트 파일 또는 "[dockerize](https://github.com/jwilder/dockerize.git)"  바이너리 파일을 통하여 해당 프로세스가 정상적으로 구동될 때 까지 Sleep Timeout을 설정하여 정상적으로 구동되면 해당 컨테이너를 동작 시킬 수 있습니다.

보다 자세한 사용법은 아래 bot_Dockerfile에서 설명드리겠습니다.

```docker
FROM ubuntu:18.04

RUN useradd -m -c "bot aaaa" -s "/usr/sbin/nologin" raon_bot

WORKDIR /home/raon_bot

COPY ./src/* /home/raon_bot/bot/
COPY ./supervisord.conf /etc/supervisor/conf.d/supervisord.conf
COPY ./wait-for-it.sh /sbin/wait-for-it.sh

RUN apt-get update && \
    # Debug Tool
    # apt-get install --no-install-recommends -y git vim supervisor \
    apt-get install --no-install-recommends -y supervisor \
    python3-minimal \
    python3-pip \
    chmod 700 /sbin/wait-for-it.sh

# Wait for DB Container 활성화후 MySQL Process 대기
CMD [ "/sbin/wait-for-it.sh", "-h", "apache_web", "-p", "80", "--", "/usr/bin/supervisord", "-c", "/etc/supervisor/conf.d/supervisord.conf" ]
```

**bot_Dockerfile**

사용된 인자를 살펴보면 "`/sbin/wait-for-it.sh -h apache_web -p 80 — /usr/bin/supervisord -c '/etc/supervisor/conf.d/supervisord.conf`"를 사용합니다.

즉 host인 "apache_web" 컨테이너의 80번 포트로 통신을 하며, 정상적으로 프로세스가 구동되어 통신이 될 시 "—" 옵션을 통해 Supervisor 프로세스를 구동시키게 됩니다.

또한 Bot Dockerfile를 최적화 작업을 진행하였으며, 최적화를 진행하기전 도커 허브에 올라온 chromedriver 이미지의 용량이 대략 **1.4**GB 정도 불필요한 데이터가 많아 직접 **chromedriver** 이미지를 제작 하였으며 용량은 602MB로 축소 시켰습니다.

```bash
[supervisord]
nodaemon=true
logfile=/home/raon_bot/bot/log/supervisord.log
pidfile=/home/raon_bot/bot/log/supervisord.pid

[program:bot]
command=/usr/bin/python3 /home/raon_bot/bot/bot.py
directory=/home/raon_bot/
user=raon_bot
stdout_logfile=/home/raon_bot/bot/log/bot.log
stderr_logfile=/home/raon_bot/bot/log/bot_error.log
autostart=true
autorestart=true
```

supervisord.conf

현재 컨테이너와 프로세스를 안정성을 높이기 위해 "wait-for-it.sh" → "supervisor" → "container restart" 순서대로 각 단계별 프로세스를 체크하여 프로세스 또는 컨테이너가 비정상 종료시 자동으로 켜주도록 설정하였습니다.