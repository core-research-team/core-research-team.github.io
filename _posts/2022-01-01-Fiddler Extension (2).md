---
layout: post
title: "Fiddler Extension (2)"
author: "choiys"
comments: true
tags: [web, dev, c#]
---

라온화이트햇 핵심연구팀 최용선

# 0. 개요

---

Fiddler Extension은 Fiddler Plugin이라고도 하며, Fiddler에 탭을 추가하여 기능을 제공하거나, 메뉴를 추가하는 등의 행위를 가능하게 해줍니다.

Fiddler가 C#으로 개발되었기 때문에 Fiddler Extension 또한 .NET 기반의 C#을 이용하여 개발할 수 있습니다. 

본 문서는 작년 초 제작했던 Fiddler Extension에 이어, 당시 구현하지 못했던 POST method에서 json이나 octet-stream 기반의 file 첨부 기능을 추가하였습니다. 또한, Fiddler 내부에서 Python을 사용할 수 있도록 기능을 추가하였습니다.

# 1. 사용 방법

---

본 문서에서 만들어진 Fiddler Extension을 사용하는 방법 및 선행 작업에 대해 소개하도록 하겠습니다.

## 1.1. 선행 작업

---

해당 Extension을 사용하기 전에 사전에 해야하는 작업이 몇 가지 있습니다.

1. Fiddler Scripts 폴더에 Extension 파일 추가

[FiddlerPython.zip](/assets/2022-01-01/FiddlerPython.zip)

`%userprofile%\AppData\Local\Programs\Fiddler\Scripts` 경로로 이동하여 위의 zip파일을 압축 해제합니다.

현재 해당 폴더에는 기본 built-in 라이브러리, requests 라이브러리 및 의존성 라이브러리를 제외하고는 설치되어 있지 않습니다. `pip` 를 사용하여 추가 라이브러리를 설치할 수 있으며, 이를 위해서는 `python37` 폴더에 있는 `Scripts` 폴더를 삭제하고, `get-pip.py` 를 실행하여 현재 환경에 맞는 `pip` 를 재설치 후 `Scripts\pip.exe` 등을 사용하여 라이브러리를 추가할 수 있습니다.

1. FiddlerScript 수정

Fiddler에서 `Ctrl + r` 을 누르면 Fiddler ScriptEditor가 나타납니다.

```jsx
static function Main() {
    var today: Date = new Date();
    FiddlerObject.StatusText = " CustomRules.js was loaded at: " + today;

    // Uncomment to add a "Server" column containing the response "Server" header, if present
    // UI.lvSessions.AddBoundColumn("Server", 50, "@response.server");

    // Uncomment to add a global hotkey (Win+G) that invokes the ExecAction method below...
    UI.RegisterCustomHotkey(HotkeyModifiers.Alt, Keys.D1, "Inspectors"); 
    UI.RegisterCustomHotkey(HotkeyModifiers.Alt, Keys.D2, "Python"); 
}
```

하단부를 살펴보면 Main 함수가 존재하는데, 위와 같이 `UI.RegisterCustomHotkey` 함수를 사용하여 단축키를 활용하여 탭 간 이동을 할 수 있도록 핫키를 등록해줍니다.

```jsx
static function OnExecAction(sParams: String[]): Boolean {

    FiddlerObject.StatusText = "ExecAction: " + sParams[0];

    var sAction = sParams[0].toLowerCase();
    switch (sAction) {
    case "bold":
        if (sParams.Length<2) {uiBoldURI=null; FiddlerObject.StatusText="Bolding cleared"; return false;}
        uiBoldURI = sParams[1]; FiddlerObject.StatusText="Bolding requests for " + uiBoldURI;
        return true;
    case "bp":
        FiddlerObject.alert("bpu = breakpoint request for uri\nbpm = breakpoint request method\nbps=breakpoint response status\nbpafter = breakpoint response for URI");
        return true;

    // ...
		case "dump":
        UI.actSelectAll();
        UI.actSaveSessionsToZip(CONFIG.GetPath("Captures") + "dump.saz");
        UI.actRemoveAllSessions();
        FiddlerObject.StatusText = "Dumped all sessions to " + CONFIG.GetPath("Captures") + "dump.saz";
        return true;
		case "inspectors":
				UI.ActivateView("Inspectors");
				return true;
		case "python":
				UI.ActivateView("Python");
				return true;
	
    default:
        if (sAction.StartsWith("http") || sAction.StartsWith("www.")) {
            System.Diagnostics.Process.Start(sParams[0]);
            return true;
        }
        else
        {
            FiddlerObject.StatusText = "Requested ExecAction: '" + sAction + "' not found. Type HELP to learn more.";
            return false;
        }
    }
}
```

OnExecAction 함수는 Fiddler에서 `Alt + q` 를 입력했을 때 세션 하단부의 `QuicExec` 로 이동하게 되는데, 여기서 실행하는 명령어에 대한 행위를 정의합니다.

위에서 등록했던 Inspectors와 Python에 대한 정의를 해당 함수에 등록해주게 되는데, `UI.ActivateView` 함수를 사용하여 해당 탭으로 이동할 수 있는 기능을 정의해줍니다.

## 1.2. 사용 방법

---

![auto1.png](/assets/2022-01-01/auto1.png)

Fiddler 실행 후 사용 방법은 위와 같습니다.

한 세션을 선택한 뒤, 단축키 `a` 를 누르면 자동으로 `python` 탭으로 이동합니다. 그 후 HTTP 패킷에 해당하는 python requests 코드로 자동 변환시켜줍니다. 현재 지원하는 메소드는 GET/POST 이며, `querystring` , `form 데이터` `json` , `octet-stream 기반 file` 에 대한 변환을 지원합니다. 아직까지도 `PUT` , `UPDATE` , `DELETE` 등의 RESTFUL 메소드는 잘 사용되는 것을 보지 못했기 때문에 가장 많이 사용되는 GET과 POST 메소드에 대응하여 개발하였습니다.

부가 기능 몇 가지를 소개하자면

1.  `ctrl+q` 를 누르면 Fiddler 내부에서 Python이 수행되며, 위에서 작성된 코드를 실행하고, 결과를 확인할 수 있습니다. 자동 변환된 코드를 수정하여 실행하는 것 또한 가능합니다.
2. `Alt+1` 을 누르면 `Inspectors` 탭으로, `Alt+2` 를 누르면 `Python` 탭으로 이동하는 단축키를 추가하였습니다.

# 2. 개발 환경 구성

---

Fiddler Extension은 개요에서 언급한 것과 같이 C#을 이용하여 개발이 가능합니다.

문제는, Visual Studio 2019부터 프로젝트의 `참조` 항목에서 `어셈블리` 항목이 사라져, DLL을 빌드한 후 Fiddler에 연동할 때 오류가 발생하는 문제가 존재하였습니다. 해당 문제를 해결하기 위해 노력하였으나, 해결하지 못하여 결국 Visual Studio 2022 → 2019 → 2017 순으로 다운그레이드 하여 기존에 개발했던 방법대로 개발하였습니다.

![Untitled](/assets/2022-01-01/Fiddler_Ex.png)

위와 같이 `참조 > 어셈블리` 에서 해당 프로젝트에 사용할 참조 라이브러리를 추가해주는데, UI를 구성해주기 위한 필수 요소 중 하나인 `System.Windows.Forms` 를 추가해줍니다. 개발 중 필요한 라이브러리를 동일한 방법으로 추가해줄 수 있습니다.

![Untitled](/assets/2022-01-01/Fiddler_Ex1.png)

`참조 > 찾아보기` 에서 Fiddler.exe가 존재하는 경로로 찾아가 추가해줍니다. 이는 C#에서 Fiddler Extension 개발을 하기 위한 필수 요소입니다. 이를 통해 Fiddler 프로그램에 직접 개입할 수 있게 됩니다.

![Untitled](/assets/2022-01-01/Fiddler_Ex2.png)

그리고, Fiddler에서 python을 사용하기 위해 `pythonnet` 라이브러리를 활용하였습니다. 이는 .NET에서 제공하는 패키지 관리자인 `NuGet` 을 이용하여 손쉽게 설치가 가능합니다. 다양한 라이브러리 중 `pythonnet_netstandard_py37_win` 을 설치하여 사용하였습니다.

# 3. 구현

---

개발한 Fiddler Extension의 전체 소스코드를 하나씩 살펴보도록 하겠습니다.

## 3.1. main.cs

---

```csharp
using Fiddler;
using System.Windows.Forms;
using System.Windows.Forms.Integration;
using System.Drawing;

[assembly: RequiredVersion("5.0.0.0")]

namespace FiddlerPython
{
    public class Main : IFiddlerExtension
    {
        private static MakeScript makeScript;
        private TabPage page;
        private UserControl1 uc;
        private ElementHost element;
        public void OnLoad()
        {
            page = new TabPage("Python");
            page.ImageIndex = (int)SessionIcons.HTML;
            uc = new UserControl1();
            element = new ElementHost();
            element.Child = uc;
            element.Dock = DockStyle.Fill;
            page.Controls.Add(element);
            makeScript = new MakeScript(element);
            FiddlerApplication.UI.tabsViews.TabPages.Add(page);
            FiddlerApplication.UI.lvSessions.KeyPress += new KeyPressEventHandler(KeyPress);
        }
        
        public void KeyPress(object s, KeyPressEventArgs e)
        {
            if(e.KeyChar == 97)
            {
                Session[] SelectedSessions = FiddlerApplication.UI.GetSelectedSessions();
                if (SelectedSessions.Length > 0)
                {
                    if (makeScript.SessionToCode(SelectedSessions[0]))
                    {
                        FiddlerApplication.UI.tabsViews.SelectTab(FiddlerApplication.UI.tabsViews.TabPages.IndexOf(page));
                    }
                }
            }
        }
        public void OnBeforeUnload()
        {

        }
    }
}
```

main.cs의 전체 소스코드입니다. 각 기능별로 자세히 알아보도록 하겠습니다.

```csharp
[assembly: RequiredVersion("5.0.0.0")]
```

Fiddler Extension은 2.x 버전 이상부터 지원을 하며, 이 버전 이상으로 명시를 해주면 됩니다. 가장 최근 버전인 5.x 버전 근처로 임의로 지정해줘도 무방합니다.

```csharp
public class Main : IFiddlerExtension
{
	// ...
	public void onLoad() { // ... }
	// ...
	public void onBeforeUnload() {}
}
```

Fiddler 라이브러리의 IFiddlerExtension 클래스를 상속 받고 있으며, onLoad와 onBeforeUnload 메소드를 인터페이스로 정의해놓았기 때문에 두 메소드를 오버라이딩 하여 정의해주어야 정상적으로 작동합니다. onLoad는 Fiddler 프로그램이 정상적으로 로딩되었을 때 실행되며, onBeforeUnload는 Fiddler 프로그램이 종료되는 등의 이벤트가 발생했을 때 unload하는 시점에서 실행됩니다.

Extension을 추가하려면 onLoad 메소드에 해당 내용을 구현해주면 됩니다.

```csharp
private static MakeScript makeScript;
private TabPage page;
private UserControl1 uc;
private ElementHost element;
public void OnLoad()
{
    page = new TabPage("Python");
    page.ImageIndex = (int)SessionIcons.HTML;
    uc = new UserControl1();
    element = new ElementHost();
    element.Child = uc;
    element.Dock = DockStyle.Fill;
    page.Controls.Add(element);
    makeScript = new MakeScript(element);
    FiddlerApplication.UI.tabsViews.TabPages.Add(page);
    FiddlerApplication.UI.lvSessions.KeyPress += new KeyPressEventHandler(KeyPress);
}
```

MakeScript는 별도의 파일로 만들어준 클래스 파일이며, 추후에 python requests 코드로 변환해주는 기능을 사용할 때 다시 언급하겠습니다.

`new TabPage("Python")` 를 사용하여 Python이라는 이름의 새로운 탭을 추가할 수 있습니다.

여기에 `UserControl1()` 이라는 인스턴스를 생성하는데, 이는 WPF를 이용하여 UI를 구성하였기 때문에 외부의 WPF 객체를 인스턴스화 하는 과정입니다.

생성한 WPF 인스턴스를 `page.Controls.Add(element)` 와 같이 생성한 page에 추가해줍니다.

그리고 `FiddlerApplication.UI.tabsViews.TabPages.Add(page)` 를 통해 실제로 Fiddler UI에 탭으로 등록이 됩니다.

`FiddlerApplication.UI.lvSessions.KeyPress += new KeyPressEventHandler(KeyPress)` 는 Fiddler의 세션 탭에서 keypress 이벤트가 발생했을 때 어떤 행위를 할 지에 대한 내용을 추가해주는 작업입니다. 이는 `a` 단축키를 눌렀을 때 python requests code로 변환해주는 작업을 추가해주기 위한 코드입니다.

```csharp
public void KeyPress(object s, KeyPressEventArgs e)
{
    if(e.KeyChar == 97)
    {
        Session[] SelectedSessions = FiddlerApplication.UI.GetSelectedSessions();
        if (SelectedSessions.Length > 0)
        {
            if (makeScript.SessionToCode(SelectedSessions[0]))
            {
                FiddlerApplication.UI.tabsViews.SelectTab(FiddlerApplication.UI.tabsViews.TabPages.IndexOf(page));
            }
        }
    }
}
```

앞서 언급한 KeyPress 이벤트 핸들러입니다. 입력된 키가 97(a)에 해당할 경우 `FiddlerApplication.UI.GetSelectedSessions()` 를 통해 해당 세션의 정보를 가져옵니다.

`makeScript.SessionToCode(SelectedSessions[0])` 를 사용하여 선택된 세션의 첫번째 내용을 `makeScript.SessionToCode` 메소드의 인자로 전달하여 호출합니다. `makeScript` 는 이전에 언급했던 python requests 코드로 변환해주는 소스코드가 포함되어 있습니다.

이후 Python 코드로 정상적으로 변환이 완료됐을 경우 `FiddlerApplication.UI.tabsViews.SelectTab(FiddlerApplication.UI.tabsViews.TabPages.IndexOf(page))` 를 사용하여 Python 탭으로 이동시켜줍니다.

## 3.2. UserControl1.xaml

---

![Untitled](/assets/2022-01-01/Fiddler_Ex3.png)

WPF를 생성한 뒤, UI를 구성하고 완료한 화면입니다. 위쪽에서 UI를 확인할 수 있으며, 아래 xaml에 상세 내용을 작성할 수 있습니다.

```csharp
<UserControl x:Class="FiddlerPython.UserControl1"
             xmlns="http://schemas.microsoft.com/winfx/2006/xaml/presentation"
             xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml"
             xmlns:mc="http://schemas.openxmlformats.org/markup-compatibility/2006" 
             xmlns:d="http://schemas.microsoft.com/expression/blend/2008" 
             xmlns:local="clr-namespace:FiddlerPython"
             mc:Ignorable="d" 
             d:DesignHeight="600" d:DesignWidth="800">
    <Grid>
        <Label HorizontalAlignment="Left" VerticalAlignment="Top" Margin="6,-2,0,0">PATH</Label>
        <TextBox x:Name="PathBox" IsReadOnly="False" Text="Python Path" Height="20" Margin="50 0 50 0" VerticalAlignment="Top"/>
        <Button Width="50" Height="20" Margin="750,0,0,580" Panel.ZIndex="1" HorizontalAlignment="Right" VerticalAlignment="Top" Click="OnButtonClick">OK</Button>
        <TabControl Margin="0 20 0 0">
            <TabItem Header="Code">
                <Grid>
                    <Grid.RowDefinitions>
                        <RowDefinition Height="2*"/>
                        <RowDefinition Height="30"/>
                        <RowDefinition Height="1*"/>
                    </Grid.RowDefinitions>
                    <TextBox x:Name="CodeBox" Grid.Row="0" FontSize="15" HorizontalAlignment="Stretch" VerticalAlignment="Stretch" IsReadOnly="False" TextWrapping="Wrap" Text="" VerticalScrollBarVisibility="Visible" ScrollViewer.CanContentScroll="True" PreviewKeyDown="OnKeyDownHandler" AcceptsReturn="True"/>
                    <Label FontSize="15" Grid.Row="1">Result</Label>
                    <TextBox x:Name="CodeResBox" Grid.Row="2" FontSize="15" IsReadOnly="True" TextWrapping="Wrap" Text="" VerticalScrollBarVisibility="Visible" ScrollViewer.CanContentScroll="True" AcceptsReturn="True"/>
                </Grid>
            </TabItem>
        </TabControl>
    </Grid>
</UserControl>
```

아래 작성한 xaml입니다.

```csharp
<Label HorizontalAlignment="Left" VerticalAlignment="Top" Margin="6,-2,0,0">PATH</Label>
<TextBox x:Name="PathBox" IsReadOnly="False" Text="Python Path" Height="20" Margin="50 0 50 0" VerticalAlignment="Top"/>
<Button Width="50" Height="20" Margin="750,0,0,580" Panel.ZIndex="1" HorizontalAlignment="Right" VerticalAlignment="Top" Click="OnButtonClick">OK</Button>
```

가장 먼저 Grid를 생성하고, UI의 가장 상단에 `PATH`라는 내용의 `Label`과 `PathBox`라는 이름의 `TextBox`를 생성해주었습니다. 이는 python을 실행하기 위한 python home 디렉토리를 지정하는 용도로 사용됩니다. 그리고 바로 아래 생성한 `Button` 을 클릭하면 `PathBox` 에 작성된 디렉토리로 python home 디렉토리를 추가합니다.

```csharp
<TabControl Margin="0 20 0 0">
    <TabItem Header="Code">
        <Grid>
            <Grid.RowDefinitions>
                <RowDefinition Height="2*"/>
                <RowDefinition Height="30"/>
                <RowDefinition Height="1*"/>
            </Grid.RowDefinitions>
            <TextBox x:Name="CodeBox" Grid.Row="0" FontSize="15" HorizontalAlignment="Stretch" VerticalAlignment="Stretch" IsReadOnly="False" TextWrapping="Wrap" Text="" VerticalScrollBarVisibility="Visible" ScrollViewer.CanContentScroll="True" PreviewKeyDown="OnKeyDownHandler" AcceptsReturn="True"/>
            <Label FontSize="15" Grid.Row="1">Result</Label>
            <TextBox x:Name="CodeResBox" Grid.Row="2" FontSize="15" IsReadOnly="True" TextWrapping="Wrap" Text="" VerticalScrollBarVisibility="Visible" ScrollViewer.CanContentScroll="True" AcceptsReturn="True"/>
        </Grid>
    </TabItem>
</TabControl>
```

그 다음 `TabControl`을 추가해주었는데, 이는 UI에 보이는 `Code` 라는 탭을 만들어주는 역할을 합니다.

`<TabItem Header="Code">` 를 이용하여 만들어준 탭의 이름을 Code로 지정해줍니다.

그리고 화면의 크기에 따라 반응형 형태로 화면을 구성하기 위해 Grid를 사용하였으며, `Grid.RowDefinitions` 를 사용하여 Row의 비율을 설정해주었습니다. `RowDefinition` 의 가운데 30px를 제외하고는 `2*`, `1*` 가 설정되어 있는데, 가운데의 30px를 제외한 나머지 구역을 세로 기준 2:1로 나누어 설정해주는 역할을 합니다.

다음으로, `CodeBox` 라는 이름의 `TextBox` 를 생성해주는데, `Grid.Row` 를 `0` 으로 지정해주어 `RowDefinition` 으로 지정했던 첫번째 구역인 `2*` 만큼의 영역을 차지하도록 해주었습니다. `HorizontalAlignment` 요소는 `Stretch` 로 설정하여 가로 최대 길이만큼 늘어나도록 설정해주었으며, 추후에 설명할 tab 키의 작동 방식을 변경하기 위해 `PreviewKeyDown="OnKeyDownHandler"` 를 통해 KeyDown 이벤트가 발생했을 때의 핸들러를 추가해주었습니다.

그리고 중간에 `Grid.Row` 의 1번 요소인 30px만큼의 영역을 차지하는 Result `Label` 을 추가하였습니다.

마지막으로, `CodeResBox` 라는 이름의 `TextBox` 를 생성해주었습니다. 이는 `CodeBox` 에 작성된 python code를 실행한 후 결과를 출력해주는 영역입니다. `IsReadOnly` 를 True로 설정한 것 외에는 `CodeBox` 의 요소와 비슷하므로, 설명은 생략하겠습니다.

## 3.3. UserControl1.xaml.cs

---

```csharp
using System;
using System.Linq;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Diagnostics;
using System.IO;
using Python.Runtime;

namespace FiddlerPython
{
    /// <summary>
    /// UserControl1.xaml에 대한 상호 작용 논리
    /// </summary>
    public partial class UserControl1 : UserControl
    {
        public UserControl1()
        {
            InitializeComponent();

            string pythonPath = getPythonPath();
            addPythonPath(pythonPath);
        }
        private void OnButtonClick(object sender, RoutedEventArgs e)
        {
            if (!addPythonPath(PathBox.Text)){ 
                MessageBox.Show("Invalid Path");
            }
        }
        private void OnKeyDownHandler(object sender, KeyEventArgs e)
        {
            if((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
            {
                if(e.Key == Key.Q)
                {
                    execPython();
                }
            }
            else if(e.Key == Key.Tab)
            {
                e.Handled = true;
                int current = CodeBox.SelectionStart;
                int selected = CodeBox.SelectionLength;
                CodeBox.Text = CodeBox.Text.Substring(0, current) + "    " + CodeBox.Text.Substring(current + selected);
                CodeBox.SelectionStart = current + 4;
                CodeBox.SelectionLength = 0;
            }
        }
        private void execPython()
        {
            PythonEngine.Initialize();
            try
            {
                using (Py.GIL())
                {
                    dynamic sys = PythonEngine.ImportModule("sys");

                    string codeToRedirectOutput = "import sys\n" +
                        "from io import StringIO\n" +
                        "sys.stdout = mystdout = StringIO()\n" +
                        "sys.stdout.flush()\n" +
                        "sys.stderr = mystderr = StringIO()\n" +
                        "sys.stderr.flush()\n";
                    PythonEngine.RunSimpleString(codeToRedirectOutput);

                    string code = CodeBox.Text;
                    dynamic result = PythonEngine.RunSimpleString(@code);
                    if (result != -1)
                    {
                        string stdout = sys.stdout.getvalue();
                        CodeResBox.Text = stdout;
                    }
                    else
                    {
                        string stderr = sys.stderr.getvalue();
                        CodeResBox.Text = stderr;
                    }
                }
            }
            catch (Exception e)
            {
                CodeResBox.Text = e.Message;
            }

            PythonEngine.Shutdown();
        }
				private string getPythonPath()
        {
            string temp = @"%userprofile%\AppData\Local\Programs\Fiddler\Scripts";
            string pythonPath = Path.Combine(temp, @"Scripts\python37");
            PathBox.Text = pythonPath;

            return pythonPath;
        }
        private Boolean addPythonPath(string pythonPath)
        {
            DirectoryInfo di = new DirectoryInfo(pythonPath);
            if (!di.Exists)
            {
                return false;
            }
            string PYTHON_HOME = Environment.ExpandEnvironmentVariables(@pythonPath);
            AddEnvPath(PYTHON_HOME);
            PythonEngine.PythonHome = PYTHON_HOME;
            PythonEngine.PythonPath = string.Join(
                Path.PathSeparator.ToString(),
                new string[]
                {
                    PythonEngine.PythonPath,
                    Path.Combine(PYTHON_HOME, @"Lib"),
                    Path.Combine(PYTHON_HOME, @"Lib\site-packages"),
                    Path.Combine(PYTHON_HOME, @"DLLs"),
                    Path.Combine(PYTHON_HOME, @"Script"),
                }
            );
            return true;
        }
        private void AddEnvPath(params string[] paths)
        {
            var envPaths = Environment.GetEnvironmentVariable("PATH").Split(Path.PathSeparator).ToList();
            envPaths.InsertRange(0, paths.Where(x => x.Length > 0 && !envPaths.Contains(x)).ToArray());
            Environment.SetEnvironmentVariable("PATH", string.Join(Path.PathSeparator.ToString(), envPaths), EnvironmentVariableTarget.Process);
        }
    }
}
```

UserControl1.xaml 에서 작성한 WPF의 소스코드형 구현부입니다. 버튼을 클릭하거나, keypress 등의 이벤트가 발생했을 때 핸들링하는 핸들러를 추가하는 등의 역할을 수행합니다.

하나씩 살펴보도록 하겠습니다.

```csharp
public UserControl1()
{
    InitializeComponent();

    string pythonPath = getPythonPath();
    addPythonPath(pythonPath);
}
```

`InitializeComponent` 는 기본적으로 제공되는 메소드이며, WPF 컴포넌트를 초기화시켜주는 역할을 수행합니다.

초기화가 수행된 이후, `getPythonPath` 를 호출하여 python home path를 불러온 후 `addPythonPath` 함수를 호출하여 이를 등록합니다.

```csharp
private string getPythonPath()
{
    string temp = @"%userprofile%\AppData\Local\Programs\Fiddler\Scripts";
    string pythonPath = Path.Combine(temp, @"Scripts\python37");
    PathBox.Text = pythonPath;

    return pythonPath;
}
```

getPythonPath 메소드에서는 `Fiddler\Scripts` 폴더 내에 있는 python home path를 가져오는 역할을 수행합니다.

```csharp
private Boolean addPythonPath(string pythonPath)
{
    DirectoryInfo di = new DirectoryInfo(pythonPath);
    if (!di.Exists)
    {
        return false;
    }
    string PYTHON_HOME = Environment.ExpandEnvironmentVariables(@pythonPath);
    AddEnvPath(PYTHON_HOME);
    PythonEngine.PythonHome = PYTHON_HOME;
    PythonEngine.PythonPath = string.Join(
        Path.PathSeparator.ToString(),
        new string[]
        {
            PythonEngine.PythonPath,
            Path.Combine(PYTHON_HOME, @"Lib"),
            Path.Combine(PYTHON_HOME, @"Lib\site-packages"),
            Path.Combine(PYTHON_HOME, @"DLLs"),
            Path.Combine(PYTHON_HOME, @"Script"),
        }
    );
    return true;
}
```

addPythonPath 메소드는 전달받은 pythonPath가 존재하는지 확인 후 환경변수에 해당 디렉토리를 추가하는 역할을 수행합니다. 또한, `PythonEngine`의 `PythonHome` 및 `PythonPath` 를 추가합니다. `PythonPath`에는 실행하는 python에서 참조할 라이브러리 등의 경로가 포함됩니다.

```csharp
private void AddEnvPath(params string[] paths)
{
    var envPaths = Environment.GetEnvironmentVariable("PATH").Split(Path.PathSeparator).ToList();
    envPaths.InsertRange(0, paths.Where(x => x.Length > 0 && !envPaths.Contains(x)).ToArray());
    Environment.SetEnvironmentVariable("PATH", string.Join(Path.PathSeparator.ToString(), envPaths), EnvironmentVariableTarget.Process);
}
```

AddEnvPath 메소드는 `PATH` 환경변수에 전달 받은 path가 존재하지 않을 경우 추가해주는 역할을 수행합니다. 이 때 지정된 환경변수는 영구적으로 저장되는 것이 아니라, 프로그램이 실행되는 동안에만 유지됩니다.

```csharp
private void OnKeyDownHandler(object sender, KeyEventArgs e)
{
    if((Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control)
    {
        if(e.Key == Key.Q)
        {
            execPython();
        }
    }
    else if(e.Key == Key.Tab)
    {
        e.Handled = true;
        int current = CodeBox.SelectionStart;
        int selected = CodeBox.SelectionLength;
        CodeBox.Text = CodeBox.Text.Substring(0, current) + "    " + CodeBox.Text.Substring(current + selected);
        CodeBox.SelectionStart = current + 4;
        CodeBox.SelectionLength = 0;
    }
}
```

해당 메소드는 `CodeBox` 에서 입력된 keydown 이벤트를 핸들링하는 핸들러입니다. `Ctrl + q` 가 입력되었을 때 `execPython` 메소드는 호출하며, 이는 `CodeBox` 내의 python 코드를 실행해주는 역할을 수행합니다.

다음으로 `Tab` 키를 눌렀을 때의 동작입니다. `TextBox` 옵션 중 `AcceptsTab` 옵션이 `True` 로 설정되었을 경우에는 `\t` 가 입력이 되어 `indent` 가 지나치게 넓어지는 현상을 개선하기 위해 핸들러로 처리하였습니다. `AcceptsTab` 옵션을 `False` 로 지정하는 경우에는 다음 컴포넌트(예를 들면 `CodeResBox`)로 이동하게 되는데, `e.Handled = true` 를 사용하게 되면 포커스가 변하지 않게 됩니다.

그리고 현재 위치에서 공백 4칸을 추가하고 포커스를 4만큼 늘려주는 형태로 `Tab`의 기능을 구현하였습니다.

```csharp
private void execPython()
{
    PythonEngine.Initialize();
    try
    {
        using (Py.GIL())
        {
            dynamic sys = PythonEngine.ImportModule("sys");

            string codeToRedirectOutput = "import sys\n" +
                "from io import StringIO\n" +
                "sys.stdout = mystdout = StringIO()\n" +
                "sys.stdout.flush()\n" +
                "sys.stderr = mystderr = StringIO()\n" +
                "sys.stderr.flush()\n";
            PythonEngine.RunSimpleString(codeToRedirectOutput);

            string code = CodeBox.Text;
            dynamic result = PythonEngine.RunSimpleString(@code);
            if (result != -1)
            {
                string stdout = sys.stdout.getvalue();
                CodeResBox.Text = stdout;
            }
            else
            {
                string stderr = sys.stderr.getvalue();
                CodeResBox.Text = stderr;
            }
        }
    }
    catch (Exception e)
    {
        CodeResBox.Text = e.Message;
    }

    PythonEngine.Shutdown();
}
```

마지막으로, execPython 메소드입니다.

먼저, `PythonEngine.Initialize` 를 사용하여 PythonEngine을 초기화 시켜줍니다.

이후 python 코드가 실행되는 처리는 모두 `using (Py.GIL()){}` 내부에서 처리하게 됩니다.

`pythonnet` 을 이용하여 python 코드를 실행하게 되면 기본적으로 `stdout` 으로 결과가 출력되는데, 이를 C# 컴포넌트에 출력하기 위해 몇 가지 작업을 처리하였습니다.

```csharp
dynamic sys = PythonEngine.ImportModule("sys");

string codeToRedirectOutput = "import sys\n" +
    "from io import StringIO\n" +
    "sys.stdout = mystdout = StringIO()\n" +
    "sys.stdout.flush()\n" +
    "sys.stderr = mystderr = StringIO()\n" +
    "sys.stderr.flush()\n";
PythonEngine.RunSimpleString(codeToRedirectOutput);
```

`PythonEngine.ImportModule` 을 사용하면 python에서 사용하는 라이브러리를 import 하여 C# 레벨에서 사용할 수 있습니다.

그리고 `stringIO` 를 사용하여 `sys.stdout` 과 `sys.stderr` 에 해당하는 내용을 파일과 같은 형태로 임시 저장하도록 구현하였습니다.

그리고 `PythonEngine.RunSimpleString` 메소드를 사용하여 입력한 python code를 실행할 수 있습니다.

```csharp
string code = CodeBox.Text;
dynamic result = PythonEngine.RunSimpleString(@code);
if (result != -1)
{
    string stdout = sys.stdout.getvalue();
    CodeResBox.Text = stdout;
}
else
{
    string stderr = sys.stderr.getvalue();
    CodeResBox.Text = stderr;
}
```

이후 `CodeBox` 에 있는 소스코드를 받아와 `RunSimpleString` 메소드를 이용하여 python에서 실행합니다. 실행이 성공하였을 경우 `stdout` 에 담긴 내용을 `CodeResBox` 에 출력하고, 실패하였을 경우 에러 메시지가 담긴 `stderr` 를 `CodeResBox` 에 출력해줍니다.

## 3.4. MakeScript.cs

---

```csharp
using System;
using System.Collections.Specialized;
using System.Web;
using System.Windows.Forms.Integration;
using Fiddler;

namespace FiddlerPython
{
    class MakeScript
    {
        private ElementHost element;
        public MakeScript()
        {

        }
        public MakeScript(ElementHost element)
        {
            this.element = element;
        }
        public Boolean isJSON(string bodies)
        {
            if ((bodies[0] == '{' && bodies[bodies.Length - 1] == '}') || (bodies[0] == '[' && bodies[bodies.Length - 1] == ']'))
            {
                return true;
            }
            return false;
        }

        public string isFile(HTTPRequestHeaders headers)
        {
            try
            {
                foreach (HTTPHeaderItem h in headers)
                {
                    if (h.Name == "Content-Type")
                    {
                        string token = "boundary=";
                        string[] boundaries = h.Value.Split(new string[] { token }, StringSplitOptions.None);
                        return boundaries[1];
                    }
                }
            }
            catch { }
            return null;
        }

        public string makeHeader(HTTPRequestHeaders headers)
        {
            string header = "{";
            try
            {
                foreach (HTTPHeaderItem h in headers)
                {
                    header += $"\n    '{h.Name}': '{h.Value}',";
                }
            }
            catch { }
            header += "\n}";

            return header;
        }

        public string makeParam(NameValueCollection queries)
        {
            string param = "{";
            try
            {
                foreach (string name in queries)
                {
                    string value = queries.Get(name);
                    param += $"\n    '{name}': '{value}',";
                }
            }
            catch { }
            param += "\n}";

            return param;
        }

        public string makeJson(string bodies)
        {
            string json = "{";
            if (bodies != null && bodies != "" && isJSON(bodies))
            {
                json = bodies;
            }
            else
            {
                json += "\n}";
            }

            return json;
        }

        public string makeData(string bodies)
        {
            string data = "{";
            try
            {
                string[] splitBodies = bodies.Split('&');
                foreach (string body in splitBodies)
                {
                    string[] token = body.Split('=');
                    string name = token[0];
                    string value = token[1];
                    value = value.Replace("'", "%27");
                    data += $"\n    '{name}': '{value}',";
                }
            }
            catch { }
            data += "\n}";

            return data;
        }

        public string parseContents(string file)
        {
            string contents = "";
            string[] tempContents = file.Split(new string[] { "\r\n\r\n" }, StringSplitOptions.None);
            Boolean flag = true;
            foreach (string content in tempContents)
            {
                if (flag)
                {
                    flag = false;
                    continue;
                }
                contents += content;
            }
            return contents;
        }

        public NameValueCollection makeFile(HTTPRequestHeaders headers, string bodies)
        {
            string boundary = isFile(headers);
            // 0 : 공백
            // 1 : 1번째
            // 2 : 2번째
            // 3 : --
            string filename = "";
            string realname = "";
            string fileContents = "";
            if (boundary != null)
            {
                string data = "{";
                boundary = "--" + boundary;
                try
                {
                    string[] files = bodies.Split(new string[] { boundary }, StringSplitOptions.None);
                    foreach (string file in files)
                    {
                        if (file == "")
                        {
                            continue;
                        }
                        if (file.Contains("filename="))
                        {
                            filename = file.Split(new string[] { "name=\"" }, StringSplitOptions.None)[1];
                            filename = filename.Split('\"')[0];
                            realname = file.Split(new string[] { "filename=\"" }, StringSplitOptions.None)[1];
                            realname = realname.Split('\"')[0];
                            fileContents = parseContents(file);
                            System.IO.File.WriteAllText("temp", fileContents);
                        }
                        else
                        {
                            string name = file.Split(new string[] { "name=\"" }, StringSplitOptions.None)[1];
                            name = name.Split('\"')[0];
                            string contents = parseContents(file).Replace("'", "%27").Replace("\r\n", "");
                            data += $"\n    '{name}': '{contents}',";
                        }
                    }
                }
                catch { }
                data += "\n}";
                NameValueCollection res = new NameValueCollection();
                res.Set("filename", filename);
                res.Set("realname", realname);
                res.Set("data", data);
                return res;
            }

            //MessageBox.Show(filename);
            //MessageBox.Show(fileContents);
            //MessageBox.Show(data);
            return null;
        }

        public string CodeTemplate(string method, string url, HTTPRequestHeaders headers, NameValueCollection queries, string bodies)
        {
            string header = makeHeader(headers);
            string param = makeParam(queries);
            string json = makeJson(bodies);
            string filename = "";
            string realname = "";
            string data = "";
            NameValueCollection fileCollection = makeFile(headers, bodies);
            if (fileCollection != null)
            {
                filename = fileCollection.Get("filename");
                realname = fileCollection.Get("realname");
                data = fileCollection.Get("data");
            }
            if (data == "")
            {
                data = makeData(bodies);
            }

            string code = $@"#coding: utf-8

import requests

requests.packages.urllib3.disable_warnings()

s = requests.Session()

def send(url, headers, params, data, files):
    proxies = {% raw %}{{
        'http': 'http://localhost:8888',
        'https': 'http://localhost:8888'
    }}{% endraw %}
    r = s.{method.ToLower()}(url, headers=headers, proxies=proxies, verify=False, params=params, data=data, json=json, files=files)
    return r

url = '{url}'
headers = {header}

params = {param}

data = {data}

json = {json}
";
            if (filename != "")
            {
                code += $@"
file = open('temp', 'rb')
files = {% raw %}{{'{filename}': ('{realname}', file)}}{% endraw %}

r = send(url, headers, params, data, files)

print(r.text)";
            }
            else
            {
                code += $@"
r = send(url, headers, params, data, None)

print(r.text)";
            }

            return code;
        }

        public void getMethod(string url, HTTPRequestHeaders headers, NameValueCollection queries)
        {
            string code = CodeTemplate("GET", url, headers, queries, null);
            (element.Child as UserControl1).CodeBox.Text = code;
        }

        public void postMethod(string url, HTTPRequestHeaders headers, NameValueCollection queries, string requestBody)
        {
            string code = CodeTemplate("POST", url, headers, queries, requestBody);
            (element.Child as UserControl1).CodeBox.Text = code;
        }

        public NameValueCollection parseQuery(string fullUrl, int queryIdx)
        {
            NameValueCollection queries = null;
            if (queryIdx != -1)
            {
                string queryString = fullUrl.Substring(queryIdx + 1);
                queries = HttpUtility.ParseQueryString(queryString);
            }

            return queries;
        }

        public Boolean SessionToCode(Session session)
        {
            string requestMethod = session.RequestMethod;

            string fullUrl = session.fullUrl;
            string url = fullUrl;
            HTTPRequestHeaders headers = session.RequestHeaders;

            int queryIdx = fullUrl.IndexOf('?');
            NameValueCollection queries = parseQuery(fullUrl, queryIdx);

            if (queries != null)
            {
                url = fullUrl.Substring(0, queryIdx);
            }

            if (requestMethod == "GET")
            {
                getMethod(url, headers, queries);
            }
            else if (requestMethod == "POST")
            {
                string requestBody = session.GetRequestBodyAsString();
                (element.Child as UserControl1).CodeBox.Text = requestBody;
                postMethod(url, headers, queries, requestBody);
            }
            else
            {
                return false;
            }
            return true;
        }
    }
}
```

MakeScript.cs 에는 Fiddler 세션에 담긴 내용을 가져와 파싱하여 python requests 코드로 변환해주는 역할을 수행합니다.

```csharp
public MakeScript(ElementHost element)
{
    this.element = element;
}
```

가장 먼저 생성자를 오버로딩하여 `ElementHost` 타입이 들어올 경우 `element` 에 할당하게 되는데, 이는 외부 클래스에서 WPF 요소를 참조하기 위해 필요한 element를 받아오는 작업입니다.

`(element.Child as UserControl1).CodeBox.Text` 와 같은 형태로 WPF의 컴포넌트를 참조하여 활용할 수 있습니다.

```csharp
public Boolean SessionToCode(Session session)
{
    string requestMethod = session.RequestMethod;

    string fullUrl = session.fullUrl;
    string url = fullUrl;
    HTTPRequestHeaders headers = session.RequestHeaders;

    int queryIdx = fullUrl.IndexOf('?');
    NameValueCollection queries = parseQuery(fullUrl, queryIdx);

    if (queries != null)
    {
        url = fullUrl.Substring(0, queryIdx);
    }

    if (requestMethod == "GET")
    {
        getMethod(url, headers, queries);
    }
    else if (requestMethod == "POST")
    {
        string requestBody = session.GetRequestBodyAsString();
        (element.Child as UserControl1).CodeBox.Text = requestBody;
        postMethod(url, headers, queries, requestBody);
    }
    else
    {
        return false;
    }
    return true;
}
```

main.cs에서 봤던 `SessionToCode` 메소드입니다. 전달 받은 Fiddler session에서 fullURL을 받아와 parseQuery 메소드에서 querystring을 파싱하고, GET/POST 메소드에 따라 getMethod와 postMethod로 헤더를 포함한 인자를 전달합니다.

```csharp
public NameValueCollection parseQuery(string fullUrl, int queryIdx)
{
    NameValueCollection queries = null;
    if (queryIdx != -1)
    {
        string queryString = fullUrl.Substring(queryIdx + 1);
        queries = HttpUtility.ParseQueryString(queryString);
    }

    return queries;
}
```

parseQuery 메소드는 fullURL을 전달 받아 querystring이 존재할 경우 `HttpUtility.ParseQueryString` 메소드를 호출하여 NameValueCollection의 형태로 querystring을 파싱하여 리턴합니다.

```csharp
public void getMethod(string url, HTTPRequestHeaders headers, NameValueCollection queries)
{
    string code = CodeTemplate("GET", url, headers, queries, null);
    (element.Child as UserControl1).CodeBox.Text = code;
}

public void postMethod(string url, HTTPRequestHeaders headers, NameValueCollection queries, string requestBody)
{
    string code = CodeTemplate("POST", url, headers, queries, requestBody);
    (element.Child as UserControl1).CodeBox.Text = code;
}
```

getMethod와 postMethod는 url, header, query, requestbody를 입력 받아 CodeTemplate 메소드로 각각 전달합니다.

```csharp
public string CodeTemplate(string method, string url, HTTPRequestHeaders headers, NameValueCollection queries, string bodies)
{
    string header = makeHeader(headers);
    string param = makeParam(queries);
    string json = makeJson(bodies);
    string filename = "";
    string realname = "";
    string data = "";
    NameValueCollection fileCollection = makeFile(headers, bodies);
    if (fileCollection != null)
    {
        filename = fileCollection.Get("filename");
        realname = fileCollection.Get("realname");
        data = fileCollection.Get("data");
    }
    if (data == "")
    {
        data = makeData(bodies);
    }

    string code = $@"#coding: utf-8

import requests

requests.packages.urllib3.disable_warnings()

s = requests.Session()

def send(url, headers, params, data, files):
proxies = {% raw %}{{
'http': 'http://localhost:8888',
'https': 'http://localhost:8888'
}}{% endraw %}
r = s.{method.ToLower()}(url, headers=headers, proxies=proxies, verify=False, params=params, data=data, json=json, files=files)
return r

url = '{url}'
headers = {header}

params = {param}

data = {data}

json = {json}
";
    if (filename != "")
    {
        code += $@"
file = open('temp', 'rb')
files = {% raw %}{{'{filename}': ('{realname}', file)}}{% endraw %}

r = send(url, headers, params, data, files)

print(r.text)";
    }
    else
    {
        code += $@"
r = send(url, headers, params, data, None)

print(r.text)";
    }

    return code;
}
```

CodeTemplate 메소드는 python requests 코드로 변경해주는 템플릿을 작성해주는 역할을 수행합니다. makeHeader, makeParam, makeJson, makeFile, makeData 메소드 등을 불러와 각 변수에 저장하고, python 코드 템플릿에 대입하여 코드를 완성합니다.

```csharp
public string makeHeader(HTTPRequestHeaders headers)
{
    string header = "{";
    try
    {
        foreach (HTTPHeaderItem h in headers)
        {
            header += $"\n    '{h.Name}': '{h.Value}',";
        }
    }
    catch { }
    header += "\n}";

    return header;
}

public string makeParam(NameValueCollection queries)
{
    string param = "{";
    try
    {
        foreach (string name in queries)
        {
            string value = queries.Get(name);
            param += $"\n    '{name}': '{value}',";
        }
    }
    catch { }
    param += "\n}";

    return param;
}
```

makeHeader와 makeParam의 경우, dictionary 구조와 비슷한 형태로 전달 받기 때문에 foreach를 활용하여 각 요소의 key와 value를 불러와 각각의 변수에 할당해주는 역할을 합니다.

```csharp
public string makeJson(string bodies)
{
    string json = "{";
    if (bodies != null && bodies != "" && isJSON(bodies))
    {
        json = bodies;
    }
    else
    {
        json += "\n}";
    }

    return json;
}
```

makeJson은 requestBody에 포함된 내용이 json일 경우 python 코드에서도 그대로 쓰일 수 있기 때문에 그대로 할당해줍니다.

```csharp
public string makeData(string bodies)
{
    string data = "{";
    try
    {
        string[] splitBodies = bodies.Split('&');
        foreach (string body in splitBodies)
        {
            string[] token = body.Split('=');
            string name = token[0];
            string value = token[1];
            value = value.Replace("'", "%27");
            data += $"\n    '{name}': '{value}',";
        }
    }
    catch { }
    data += "\n}";

    return data;
}
```

makeData 메소드는 requestBody가 form 형태로 전달되었을 경우 수행하며, name=value 형태를 파싱하여 dictionary 형태로 변환하여 data 변수에 할당합니다.

```csharp
public NameValueCollection makeFile(HTTPRequestHeaders headers, string bodies)
{
    string boundary = isFile(headers);
    // 0 : 공백
    // 1 : 1번째
    // 2 : 2번째
    // 3 : --
    string filename = "";
    string realname = "";
    string fileContents = "";
    if (boundary != null)
    {
        string data = "{";
        boundary = "--" + boundary;
        try
        {
            string[] files = bodies.Split(new string[] { boundary }, StringSplitOptions.None);
            foreach (string file in files)
            {
                if (file == "")
                {
                    continue;
                }
                if (file.Contains("filename="))
                {
                    filename = file.Split(new string[] { "name=\"" }, StringSplitOptions.None)[1];
                    filename = filename.Split('\"')[0];
                    realname = file.Split(new string[] { "filename=\"" }, StringSplitOptions.None)[1];
                    realname = realname.Split('\"')[0];
                    fileContents = parseContents(file);
                    System.IO.File.WriteAllText("temp", fileContents);
                }
                else
                {
                    string name = file.Split(new string[] { "name=\"" }, StringSplitOptions.None)[1];
                    name = name.Split('\"')[0];
                    string contents = parseContents(file).Replace("'", "%27").Replace("\r\n", "");
                    data += $"\n    '{name}': '{contents}',";
                }
            }
        }
        catch { }
        data += "\n}";
        NameValueCollection res = new NameValueCollection();
        res.Set("filename", filename);
        res.Set("realname", realname);
        res.Set("data", data);
        return res;
    }

    //MessageBox.Show(filename);
    //MessageBox.Show(fileContents);
    //MessageBox.Show(data);
    return null;
}
```

마지막으로 makeFile입니다. isFile 메소드에서 Content-Type을 확인하여 boundary 포함 유무를 판별합니다. 이후 filename과 contents를 파싱하고, contents는 parseContents 메소드를 사용하여 별도의 임시파일로 저장합니다. 그리고 file을 전송할 때 boundary 영역 내에 form 형태의 파라미터 또한 전달되기 때문에, 이를 파싱하여 res 객체에 저장합니다.

이렇게 파싱한 모든 데이터를 CodeTemplate 메소드에서 조합하여 python requests 코드를 완성하게 되는 구조입니다.

# 4. 결론

---

![auto1.png](/assets/2022-01-01/auto1.png)

위의 모든 과정을 거쳐 이러한 형태로 python requests code를 자동 생성 및 실행할 수 있는 extension을 완성하였습니다.

Fiddler Extension을 개발하면서 발생할 수 있는 몇 가지 문제점에 대해서 이야기 하고 마무리하도록 하겠습니다.

1. Fiddler에서 제공하는 라이브러리에서 개발에 도움이 되는 여러가지 함수들이 존재하는데, session에서 fullURL과 requestBody 등을 따로 받아오는 기능은 존재하지만 모두 string 형태로 리턴해주기 때문에 파일 추출, querystring과 requestBody 등에 대한 dictionary 형태의 파싱이 없는 점이 아쉬웠습니다. 이 때문에 raw string에서 직접 파라미터를 파싱하여 변환하는 작업에서 미흡한 점이 있어 오동작이 발생할 수 있으며, 많은 테스트를 진행하지는 못하여 예외 처리 또한 완벽하게 수행하지는 못했습니다.
2. python 코드를 실행할 때 requests와 같은 웹 요청을 시도했을 때 응답을 받아오기까지의 interval이 존재하는데, 이 기간 동안 python 탭 뿐만 아니라 fiddler 전체가 일시적으로 멈추는 현상이 있습니다. 추후에 개발을 더 진행하게 된다면 이러한 부분을 고려하여 해결할 수 있도록 고민하는 시간을 가져야 할 것 같습니다.
3. 개발 도중에는 중간중간 겪었지만, 완료된 시점에서는 발생하지 않고 있는 문제입니다. 개발하는 과정에서 python을 실행할 때 crash가 발생하거나, 코딩 문제로 인해 크리티컬한 에러가 발생했을 때 fiddler가 종료되는 현상이 간혹 발생하였습니다. 해당 extension을 사용할 생각이 있으신 분은 fiddler의 autosave 기능을 활용하여 세션을 주기적으로 저장해주는 것이 좋을 것 같습니다. 추후에 개발을 추가적으로 진행하게 되면 autosave 기능 또한 넣어볼 생각입니다.

본 문서에 잘못된 부분이나 수정, 추가하면 좋을 것 같은 사항이 있는 경우 연락 바랍니다.

감사합니다.