---
layout: post
title: "Vulnerabilities of Flask"
author: "kkamikoon"
comments: true
tags: [programming]
toc: true
---

라온화이트햇 핵심연구팀 황정식

 Flask에서 발생할 수 있는 취약점은 크게 참조한 모듈(Module), 렌더링 방법, 운용 방법 등에서 일어날 수 있습니다. 단, **운용 방법**의 경우 Debug=True 옵션, Thread 및 Process 관리 등, **취약점이 일어나기 위한 전제 조건이 매우 까다롭기 때문**에 **이에 대한 방법은 생략**하겠습니다. 이번 보고서에서는 모듈 이용, 렌더링 방법에 대한 취약점이 발생할 수 있는 원인과 이에 대한 대응 방법을 정리해보았습니다.

# 1. Vulnerabilities of Modules

 Flask에서 이용하는 모듈은 Flask 자체적인 모듈 뿐만이 아니라 서비스를 개발하기 위해 사용된 모듈이 있습니다.  대표적으로 SQL과 통신하기 위해 사용된 sqlite, pymysql, sqlalchemy 등이 있습니다. 이와 같은 모듈을 이용하는 방법에 따라 취약점이 발생할 수 있는데, 먼저 SQL Injection 발생이 가능한 코드를 살펴보도록 하겠습니다.

## 1.1. SQL Connection Module

- **PyMySQL(or SQLite), 잘못된 인자값 전달 방법**

```python
from app.models import db

#...

query = "SELECT * FROM user WHERE uid='%s' AND pw='%s'" % ( uid, pw )
db.execute(query)

query = "SELECT * FROM user WHERE uid='{}' AND pw='{}'".format( uid, pw )
db.execute(query)

query = "SELECT * FROM user WHERE uid='" + uid + " AND pw='" + pw + "'"
db.execute(query)

query = f"SELECT * FROM user WHERE uid='{uid}' AND pw='{pw}'"
db.execute(query)
```

- **SQLAlchemy(2.x) 잘못된 인자값 전달 방법**

```python
from app.models import db

#...

query = "SELECT * FROM user WHERE uid='%s' AND pw='%s'" % ( uid, pw )
db.session.execute(query)

query = "SELECT * FROM user WHERE uid='{}' AND pw='{}'".format( uid, pw )
db.session.execute(query)

query = "SELECT * FROM user WHERE uid='" + uid + " AND pw='" + pw + "'"
db.session.execute(query)

query = f"SELECT * FROM user WHERE uid='{uid}' AND pw='{pw}'"
db.session.execute(query)
```

 위와 같은 방법으로 query를 넘겨주게 되면 Escapable 하기 때문에 SQL Injection이 발생할 수 있습니다. 따라서 위와 같은 방법은 권장되지 않으며, 다음과 같은 방법으로 코드를 작성하는 것이 바람직합니다.

- **PyMySQL(or SQLite), 안전한 인자값 전달 방법(1)**

```python
from app.models import db
from pymysql    import escape_string
#...

# using pymysql.escape_string()
uid = escape_string(uid)
pw  = escape_string(pw)

query = "SELECT * FROM user WHERE uid='%s' AND pw='%s'" % ( uid, pw )
db.execute(query)

query = "SELECT * FROM user WHERE uid='{}' AND pw='{}'".format( uid, pw )
db.execute(query)

query = "SELECT * FROM user WHERE uid='" + uid + " AND pw='" + pw + "'"
db.execute(query)

query = f"SELECT * FROM user WHERE uid='{uid}' AND pw='{pw}'"
db.execute(query)
```

- **PyMySQL(or SQLite), 안전한 인자값 전달 방법(2)**

```python
from app.models import db

query = "SELECT * FROM user WHERE uid='%(uid)s' AND pw='%(pw)s'"
args  = {'uid' : uid, 'pw' : pw}
db.execute(query, args)
```

- **SQLAlchemy, 안전한 인자값 전달 방법**

```python
from app.models import db, Users

# SQLAlchmey 권장
Users.query.filter_by(Users.uid=uid, Users.pw=pw).all()

# db.session.execute()를 사용해야 할 경우
query = "SELECT * FROM user WHERE uid='%(uid)s' AND pw='%(pw)s'"
args  = {'uid' : uid, 'pw' : pw}
db.session.execute(query, args)
```

## 1.2. Session Module

 Flask에서 제공되는 기본 세션은 JWT 형태와 유사한 형태입니다. JWT와 유사한 형태이므로 Header와 Payload, Signature로 구분되는데, Signature를 제외한 값들은 Base64로 인코딩 되어 있습니다. 따라서 간단하게 Decoding이 가능하여 세션에 저장된 값을 복호화 할 수 있습니다.

![/assets/sik0700.png](/assets/sik0700.png)

 Session 복호화의 경우 위험한 취약점은 아니지만, 세션에 비밀번호나 이용자의 중요 정보를 삽입하여 사용할 경우 정보가 노출될 수 있습니다. 때문에 위와 같이 필요한 최소한의 정보만 사용하는 것이 바람직합니다.

 단, 위와 같은 세션 값은 따로 session timeout과 같은 설정이 되어 있지 않아 언제든 timeout 값을 따로 설정해주지 않을 시 탈취되어 이용될 수 있는 가능성이 있습니다. 따라서 Flask에서는 보다 안전한 세션 관리를 위해 RedisSessionInterface, FileSystemSessionInterface, MongoDBSessionInterface, SqlAlchemySessionInterface 등을 제공합니다. 기본적인 세션 인터페이스를 이용하는 것보다 이들을 겸하여 함께 사용하는 것을 권장합니다.

- **Redis Session Class Example**

```python
# http://egloos.zum.com/mcchae/v/11184587
from uuid import uuid4
from datetime import datetime, timedelta

from flask.sessions import SessionInterface, SessionMixin
from werkzeug.datastructures import CallbackDict
from flask import Flask, session, url_for, redirect

import redis
import cPickle

#########################################################################################
# This is a session object. It is nothing more than a dict with some extra methods
class RedisSession(CallbackDict, SessionMixin):
   def __init__(self, initial=None, sid=None):
      CallbackDict.__init__(self, initial)
      self.sid = sid
      self.modified = False

#########################################################################################
# Session interface is responsible for handling logic related to sessions
# i.e. storing, saving, etc
class RedisSessionInterface(SessionInterface):
   # Init connection
   def __init__(self, host='localhost', port=6379, db=0, timeout=3600):
      self.store = redis.StrictRedis(host=host, port=port, db=db)
      self.timeout = timeout

   def open_session(self, app, request):
      # Get session id from the cookie
      sid = request.cookies.get(app.session_cookie_name)

      # If id is given (session was created)
      if sid:
         # Try to load a session from Redisdb
         stored_session = None
         ssstr = self.store.get(sid)
         if ssstr:
            stored_session = cPickle.loads(ssstr)
         if stored_session:
            # Check if the session isn't expired
            if stored_session.get('expiration') > datetime.utcnow():
               return RedisSession(initial=stored_session['data'],
                              sid=stored_session['sid'])
      
      # If there was no session or it was expired...
      # Generate a random id and create an empty session
      sid = str(uuid4())
      return RedisSession(sid=sid)

   def save_session(self, app, session, response):
      domain = self.get_cookie_domain(app)

      # We're requested to delete the session
      if not session:
         response.delete_cookie(app.session_cookie_name, domain=domain)
         return

      # Refresh the session expiration time
      # First, use get_expiration_time from SessionInterface
      # If it fails, add 1 hour to current time
      if self.get_expiration_time(app, session):
         expiration = self.get_expiration_time(app, session)
      else:
         expiration = datetime.utcnow() + timedelta(hours=1)

      # Update the Redis document, where sid equals to session.sid
      ssd = {
         'sid': session.sid,
         'data': session,
         'expiration': expiration
      }
      ssstr = cPickle.dumps(ssd)
      self.store.setex(session.sid, self.timeout, ssstr)

      # Refresh the cookie
      response.set_cookie(app.session_cookie_name, session.sid,
                     expires=self.get_expiration_time(app, session),
                     httponly=True, domain=domain)

#########################################################################################
# Create an application
app = Flask(__name__)
app.session_interface = RedisSessionInterface()

@app.route("/")
def index():
   session.permanent = False
   if not 'refreshed'in session:
      session['refreshed'] = 0

   text = "You refreshed the page %d times" % ( session['refreshed'] )
   text += '<br/><a href="/kill">Reset</a>'
   text += '<br/>dbname="%s"' % session.get('dbname','NULL')
   session['refreshed'] = session['refreshed'] + 1
   return text

@app.route("/dbset/<dbname>")
def dbset(dbname):
   session['dbname']=dbname
   return redirect(url_for('index'))

@app.route("/kill")
def kill():
   session.clear()
   return redirect(url_for('index'))

app.debug = True

if __name__ == "__main__":
   app.run(host="0.0.0.0", port=8080)
```

# 2. Rendering

 렌더링 방법 중에서 취약한 함수를 취약한 방법으로 사용하게 될 시 발생하는 취약점이 있습니다. 취약한 렌더링 함수는 render_template_string 함수가 있습니다.
{% raw %}
```python
#/app/main/__init__.py
from flask import current_app as app
from flask import Blueprint, render_template_string, session

main = Blueprint("main", __name__)

@main.route('/search', methods=['GET', 'POST'])
def search():
    if request.method == 'POST':
        keyword = request.form.get("keyword", type=str)

        query = "SELECT * FROM user WHERE keyword \
                             LIKE '%%(keyword)s%'"
        args = { "keyword" : keyword }
        data = db.execute(query, args)

       # jinja2 template with keyword
        temp = """{% extends /main/search_tables.html %}
                  {% block search %}
                  search : %(keyword)s
                  {% endblock %}""" % { "keyword" : keyword }

       # Render Template String
       return render_template_string(temp, data=data)

# if method is GET
return render_template("/main/search.html")
```
{% endraw %}

 위와 같이 render_template_string 함수에 이용자가 입력한 값이 들어가게 되면, 각별히 주의해야 합니다. keyword 라는 값에 적절한 필터링이 이루어지지 않으면 SSTI(Server Side Template Injection)취약점이 발생하게 됩니다. 

 SSTI 취약점은 RCE(Remote Command Execution) 취약점을 유발할 수 있는 매우 위험한 취약점으로 해당 취약점이 발생하지 않도록 render_template_string을 사용하지 않거나 해당 함수에 이용자의 입력 값이 들어가지 않도록 합니다. 만약 부득이하게 render_template_string 함수를 사용해야 한다면, 문자열에 대한 필터링이 반드시 이루어져야 합니다.