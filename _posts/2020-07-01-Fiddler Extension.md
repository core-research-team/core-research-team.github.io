---
layout: post
title: "Fiddler Extension"
author: "choiys"
comments: true
tags: [programming]
---

라온화이트햇 핵심연구팀 최용선

# 0. 개요

웹 취약점 분석 및 모의해킹 시 Fiddler에서 패킷을 확인 후 Python 스크립트를 작성하여 자동화하는 과정을 수행하고 있습니다. 본 문서는 이를 위한 기초 템플릿(스니펫)을 만들어주는 Fiddler Extension을 제작하는 과정을 작성하였습니다.

# 1. Fiddler Extension

Fiddler는 Extension이라는 이름으로 플러그인 비슷한 기능을 지원하고 있습니다. 이미 만들어진 Extension을 추가하여 사용할수도 있고, 사용자가 직접 커스텀하여 제작한 Extension을 추가하여 사용하는 것 또한 가능합니다.

Fiddler Extension을 만들기 위해서는 C#을 사용하여 개발 후 .dll 파일로 빌드해야합니다.

# 2. 개발 준비

![/assets/2020-07-01/sun0700.png](/assets/2020-07-01/sun0700.png)

Fiddler Extension을 개발하기에 앞서, 설정해주어야하는 부분이 몇 가지 있습니다. 개발에 사용된 IDE는 Visual Studio 2019입니다.

먼저, 위와 같이 `새 프로젝트 만들기` 를 선택합니다.

![/assets/2020-07-01/sun0701.png](/assets/2020-07-01/sun0701.png)

다음으로, dll을 만들기 위해 .NET 프레임워크 프로젝트를 생성합니다.

![/assets/2020-07-01/sun0702.png](/assets/2020-07-01/sun0702.png)

만들어진 프로젝트의 솔루션 탐색기에서 `참조` 를 우클릭하여 `참조 추가` 를 선택합니다.

![/assets/2020-07-01/sun0703.png](/assets/2020-07-01/sun0703.png)

`찾아보기` 에서 Fiddler Client를 찾아 선택 후 참조에 추가해줍니다.

```powershell
C:\Program Files\Fiddler2\Fiddler.exe
C:\Program Files(x86)\Fiddler2\Fiddler.exe
C:\Users\[UserName]\AppData\Local\Programs\Fiddler\Fiddler.exe
```

일반적인 Fiddler Client의 경로는 위와 같습니다.

![/assets/2020-07-01/sun0704.png](/assets/2020-07-01/sun0704.png)

추가적으로, 최근 Visual Studio를 사용할 때 `System.Windows.Forms` 가 기본으로 등록되어있지 않아 사용 시 에러가 발생할 수 있는데, 위에서 Fiddler Client를 등록한 과정과 유사하게 `참조 관리자`에서  `System.Windows.Forms` 를 사용하도록 체크해줍니다.

```powershell
C:\Users\[UserName]\Documents\Fiddler2
```

마지막으로, 빌드하여 만들어진 .dll 파일은 위에서 소개한 Fiddler의 경로 또는 위 경로의 `Scripts` 폴더에 넣어준 뒤 Fiddler를 재시작하면 자동으로 Extension이 등록됩니다.

# 3. 개발 방법

Fiddler Extension 개발은 C#으로 이루어지며, 일반적인 Windows GUI 개발과 비슷하다고 볼 수 있습니다. `2. 개발 준비` 에서 등록한 Fiddler 클래스에서 클래스, 메소드 등을 사용하여 Fiddler에서 사용하는 정보를 얻어와 가공하는 것이 가능합니다.

```csharp
[assembly: Fiddler.RequiredVersion("2.2.8.6")]
```

먼저, Fiddler Extension은 Fiddler v2.2.8 이후 적용이 되었기 때문에 위와 같은 코드를 추가해줍니다.

```csharp
using System;
using Fiddler;

[assembly: Fiddler.RequiredVersion("2.2.8.6")]

namespace ClassLibrary2
{
    public class Class1 : IFiddlerExtension
    {
        public void OnLoad()
        {

        }

        public void OnBeforeUnload()
        {

        }
    }
}
```

그리고 Fiddler Extension 개발을 위해 `IFiddlerExtension` 클래스를 상속받습니다.

```csharp
// IFiddlerExtension 원형

namespace Fiddler
{
    public interface IFiddlerExtension
    {
        void OnBeforeUnload();
        void OnLoad();
    }
}
```

`IFiddlerExtension` 는 인터페이스로 선언되어있기 때문에 `OnLoad()` 와 `OnBeforeUnload()` 를 구현해주어야만 합니다.

```csharp
// ...

namespace ClassLibrary2
{
    public class Class1 : IFiddlerExtension
    {
        TabPage page;
        public void OnLoad()
        {
            page = new TabPage("TestTab");
            FiddlerApplication.UI.tabsViews.TabPages.Add(page);
        }

        public void OnBeforeUnload()
        {

        }
    }
}

// ...
```

![/assets/2020-07-01/sun0705.png](/assets/2020-07-01/sun0705.png)


그리고 `System.Windows.Forms` 클래스의 `TabPage` 컴포넌트를 사용하여 인스턴스를 생성해주고, `FiddlerApplication.UI.tabsViews.TabPages.Add` 메소드를 사용하여 등록해주면 위와 같이 Fiddler에 `TestTab` 이라는 이름의 탭이 생성됩니다.

```csharp
// ...

public class Class1 : IFiddlerExtension
    {
        TabPage page;
        TextBox textbox;
        public void OnLoad()
        {
            page = new TabPage("TestTab");
            textbox = new TextBox();
            textbox.Multiline = true;
            textbox.AcceptsTab = true;
            textbox.AutoSize = true;
            textbox.AllowDrop = true;
            textbox.ReadOnly = true;
            textbox.ScrollBars = ScrollBars.Both;
            textbox.Dock = DockStyle.Fill;

            page.Controls.Add(textbox);

            FiddlerApplication.UI.tabsViews.TabPages.Add(page);
        }

        public void OnBeforeUnload()
        {

        }
    }

// ...
```

![/assets/2020-07-01/sun0706.png](/assets/2020-07-01/sun0706.png)

탭 내에 UI를 적절히 구성한 뒤, 앞서 만들어놓은 `page` 인스턴스에 있는 `Page.Controls.Add` 메소드를 사용하여 구성한 UI 인스턴스를 등록해주면 위와 같이 화면을 구성할 수 있습니다.

# 4. 소스코드

```csharp
using System;
using System.Windows.Forms;
using Fiddler;

[assembly: Fiddler.RequiredVersion("2.2.8.6")]

namespace Test
{
	public class Test : IFiddlerExtension
	{
		private TabPage page;
		private TextBox textbox;

		// QueryString / POST Data 파싱
		private String parseData(String method, String data)
        {
			int ampIdx = data.IndexOf('&');
			String[] queryStrings = null;
			String res = "data = {\r\n";
			if(ampIdx > 0)
            {
				queryStrings = data.Split('&');
				foreach(String queryString in queryStrings)
                {
					int equalsIdx = queryString.IndexOf('=');
					if(equalsIdx > 0)
                    {
						String[] keyValue = queryString.Split('=');
						res += "\t\"" + keyValue[0] + "\": \"" + keyValue[1].Replace("\"", "%22") + "\",\r\n";
					}
                }
			}
            else
            {
				int equalsIdx = data.IndexOf('=');
				if(equalsIdx > 0)
                {
					String[] keyValue = data.Split('=');
					res += "\t\"" + keyValue[0] + "\": \"" + keyValue[1].Replace("\"", "%22") + "\",\r\n";
                }
                else
                {
					return "";
				}
            }

			res += "}\r\n\r\n";

			return res;
		}

		// 인자로 받아온 세션 데이터에서
		// 필요한 요소 추출하여 Python 코드로 변환
		private void SessionToCode(Session session)
        {
			String result = "";
			var req = session.oRequest;
			var origin_headers = req.headers;
			var headers = origin_headers.ToArray();

			String url = "";
			String fullUrl = session.fullUrl;
            String RequestMethod = session.RequestMethod;

            String RequestBody = System.Text.Encoding.Default.GetString(session.RequestBody);
            
            result = "#coding: utf-8\r\n\r\n";
            result += "import requests\r\n\r\n\r\n";
            result += "requests.packages.urllib3.disable_warnings()\r\n\r\n";

            result += "s = requests.Session()\r\n\r\n";

			int questIdx = fullUrl.IndexOf('?');
			if (questIdx > 0)
			{
				result += "url = \"" +  (fullUrl.Split('?'))[0] + "\"\r\n\r\n";
			}
            else
            {
				result += "url = \"" + fullUrl + "\"\r\n\r\n";
			}

			result += "headers = {\r\n";
			foreach (HTTPHeaderItem header in headers)
			{
				String Name = header.Name;
				String Value = header.Value;
				result += "\t\"" + Name + "\": \"" + Value.Replace("\"", "%22") + "\",\r\n";
			}
			result += "}\r\n\r\n";

			if (RequestMethod == "GET")
			{
				if (questIdx > 0)
				{
					String parsedQuery = fullUrl.Substring(questIdx + 1);
					result += parseData(RequestMethod, parsedQuery);

					result += "req = requests.get(url, headers=headers, params=data, verify=False)\r\n";
				}
				else
				{
					result += "req = requests.get(url, headers=headers, verify=False)\r\n";
				}
			}

			else if (RequestMethod == "POST")
			{
				String oRequestBody = System.Text.Encoding.Default.GetString(session.RequestBody);
				result += parseData(RequestMethod, oRequestBody);

				result += "req = requests.post(url, headers=headers, data=data, verify=False)\r\n\r\n";
			}

			result += "res = req.text\r\n\r\n";
			result += "print(res)\r\n";
			
			textbox.Text = result;
			
        }

		// 드래그 중 텍스트박스에 진입했을 때
		// 마우스 커서 모양 변경
		private void DragEnter(object s, DragEventArgs e)
        {
			e.Effect = DragDropEffects.Copy;
        }
		
		// 드래그 후 텍스트박스에 놓았을 때
		// SessionToCode 함수 호출
		private void DragDrop(object s, DragEventArgs e)
        {
			DataObject obj = (DataObject)e.Data;

			Session[] sessions = (Session[])obj.GetData("Fiddler.Session[]");
			Session session = sessions[0];

			SessionToCode(session);

			// 값 자동 복사
			textbox.SelectAll();
			textbox.Copy();
			textbox.DeselectAll();
		}

		// 세션 선택 후 'a' 키 눌렀을 때
		// SessionToCode 함수 호출
		public void KeyPress(object s, KeyPressEventArgs e)
        {
			if(e.KeyChar == 97)
            {
                Session[] SelectedSessions = FiddlerApplication.UI.GetSelectedSessions();
				if (SelectedSessions.Length > 0)
                {
					Session session = SelectedSessions[0];
                    SessionToCode(session);

					//값 자동 복사
					textbox.SelectAll();
					textbox.Copy();
					textbox.DeselectAll();

					// 단축키 눌렀을 때 탭 페이지 이동
					FiddlerApplication.UI.tabsViews.SelectTab(FiddlerApplication.UI.tabsViews.TabPages.IndexOf(page));
				}
            }
        }

		public void OnLoad()
		{
			page = new TabPage("requests");
			page.ImageIndex = (int)Fiddler.SessionIcons.HTML;

			textbox = new TextBox();
			textbox.Multiline = true;
			textbox.AcceptsTab = true;
			textbox.AutoSize = true;
			textbox.AllowDrop = true;
			textbox.ReadOnly = true;
			textbox.ScrollBars = ScrollBars.Both;
			textbox.Dock = DockStyle.Fill;

            // 드래그 중 탭화면 진입 이벤트
            textbox.DragEnter += new DragEventHandler(DragEnter);

			// 드래그 중 탭화면에 끌어놓았을 때 이벤트
			textbox.DragDrop += new DragEventHandler(DragDrop); 

			page.Controls.Add(textbox);

			// Fiddler 탭에 페이지 추가
            FiddlerApplication.UI.tabsViews.TabPages.Add(page);

			// KeyPress 이벤트
			FiddlerApplication.UI.lvSessions.KeyPress += new KeyPressEventHandler(KeyPress);
		}

		public void OnBeforeUnload()
        {

        }
	}
}
```

`3. 구현` 에서 설명한 방식을 토대로 기능을 구현한 전체 소스코드입니다. 현재는 GET, POST Method에서 일반적인 파라미터만 처리가 가능한 수준이며, 추후 JSON 및 파일 업로드 처리 등의 기능을 추가할 예정입니다.

# 5. 기능

![/assets/2020-07-01/sun0707.png](/assets/2020-07-01/sun0707.png)

1. Fiddler 좌측의 세션뷰에서 항목을 선택한 뒤 'a'를 누르면 자동으로 `requests` 탭으로 이동하며, Python Requests 모듈을 이용하여 HTTP Request를 요청할 수 있는 기반 코드를 생성해줍니다.
2. 또는 `requests` 탭을 활성화시킨 상태에서 세션 항목을 드래그하여 끌어다 놓아도 동일한 결과를 얻을 수 있습니다.

변환 시 내용은 클립보드에 자동으로 복사됩니다.

# 5. 참조

[https://docs.telerik.com/fiddler/Extend-Fiddler/ExtendWithDotNet](https://docs.telerik.com/fiddler/Extend-Fiddler/ExtendWithDotNet)

[https://gist.github.com/y3rsh/1054336/6df9d5f12079966fa845d243fbe1a7581be1358b](https://gist.github.com/y3rsh/1054336/6df9d5f12079966fa845d243fbe1a7581be1358b)

[https://github.com/qiaofei32/Fiddler-Request-To-code](https://github.com/qiaofei32/Fiddler-Request-To-code)

[https://www.cnblogs.com/milantgh/p/4805280.html](https://www.cnblogs.com/milantgh/p/4805280.html)