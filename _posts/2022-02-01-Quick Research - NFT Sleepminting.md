---
layout: post
title: "Quick Research - NFT Sleepminting"
author: "donghyunlee00"
comments: true
tags: [blockchain, nft, research, analysis]
---

라온화이트햇 핵심연구팀 [이동현](http://donghyunlee.me)

# 개요

## Beeple의 <Everydays: The First 5000 Days>

2021년 3월 11일, 작가 비플(Beeple)의 NFT 작품 <Everydays: The First 5000 Days>이 경매 회사인 크리스티스(Christie's)의 [온라인 경매](https://onlineonly.christies.com/s/beeple-first-5000-days/beeple-b-1981-1/112924)에서 69,346,250달러(약 830억 원)에 낙찰되며 화제가 되었습니다.

하지만 한 달 후인 4월 4일, 이 작품을 위조해 민팅(minting)한 사건을 다룬 국내 언론은 전무했고, 외국 언론에서도 거의 다루지 않았습니다.

## NFT 절도

2021년 4월 4일, 비플이 <Everydays: The First 5000 Days>의 두 번째 에디션을 민팅한 것으로 보이는 사건이 일어났습니다.

NFTheft, 혹은 Monsieur Personne(Mr. Nobody의 불어, 이하 아무개)라는 가명을 쓰는 해커는 수 차례에 걸쳐 비플 작품의 슬립민팅을 시도했고 이를 NFT 마켓플레이스인 라리블(Rarible)에 올렸습니다.

![Untitled](/assets/2022-02-01/Untitled.png)

(현재는 내려가 열람이 불가한) 위 증거 자료를 보면 Creator가 실제 비플의 계정으로 표기되어 있고 현재 아르센 루팡(Arsene Lupin)에게 팔린 상태 임을 확인할 수 있습니다. (‘아르센 루팡’은 소설 속 괴도 캐릭터의 이름입니다.)

# 배경 지식

## 민팅(Minting)

![Untitled](/assets/2022-02-01/Untitled%201.png)

블록체인 생태계에서 민팅의 엄밀한 정의는 따로 있겠지만, NFT에서 쓰이는 민팅의 정의는 좀 더 직관적으로 설명될 수 있습니다.

NFT 민팅은 작가가 자신의 작품을 NFT화 하여 발행하고 판매하는 의미로 통상 쓰입니다.

슬립민팅(sleepminting)은 서두에 언급한 해커 NFTheft가 사용한 단어로, 원작자의 의도와 다르게 위변조를 통해 원칙적으로는 유효할 수 없는 민팅을 하는 것을 의미합니다.

## ERC

![Untitled](/assets/2022-02-01/Untitled%202.png)

(Source: [https://blog.naver.com/moon0819/221961919862](https://blog.naver.com/moon0819/221961919862))

ERC는 Ethereum Request for Comments의 약자로, 여기서 Request for Comments의 의미는 ‘인터넷’ 상에서 기술을 구현함에 있어 요구되는 상세한 절차와 기본 틀을 제공하는 표준 문서로 RFC라고 부릅니다.

대부분의 인터넷 표준은 RFC로 문서화 되어 있으며 ERC는 '이더리움 RFC' 로서 인터넷이 아닌 이더리움의 표준을 말합니다.

## NFT 마켓플레이스에 민팅하기

오픈씨, 라리블과 같은 NFT 마켓플레이스에 민팅을 하는 방법은 두 가지가 있습니다.

1. 코딩 없이 마켓플레이스 사이트 상에서 정보 입력과 버튼 클릭 등으로 민팅하기
2. 직접 ERC-720 혹은 ERC-1155 규약을 만족하는 토큰을 코딩하여 마켓플레이스에 민팅하기

![Untitled](/assets/2022-02-01/Untitled%203.png)

마켓플레이스는 해당 마켓플레이스가 어떤 ERC를 지원하는지 명시합니다. 그리고 위와 같이 [가이드](https://guide.rarible.com/)를 마련하곤 합니다.

코딩으로 제작한 토큰을 민팅하기 위해선 해당 토큰이 필히 마켓플레이스에서 지원하는 ERC 규약을 만족하여야 합니다.

예를 들어 ERC-721의 경우, [아래](https://docs.openzeppelin.com/contracts/4.x/api/token/erc721)와 같은 조건 등이 있습니다. (일부만 캡쳐한 것)

![Untitled](/assets/2022-02-01/Untitled%204.png)

# NFT 슬립민팅

NFTheft가 비플의 작품을 어떻게 슬립민팅 하였을까요? 블록체인 혹은 이더리움 상의 취약점인 것일까요?

2021년 4월 21일, Keir Finlow-Bates가 ‘How to sleepmint NFT tokens’라는 제목의 글을 올려 그 원리를 규명하였습니다. (본 문서에서는 핵심 원리만 간추려 설명하니 코드 및 데모를 포함한 좀 더 상세한 설명을 보고 싶으면 아래 링크에서 확인바랍니다.)

[https://kf106.medium.com/how-to-sleepmint-nft-tokens-bc347dc148f2](https://kf106.medium.com/how-to-sleepmint-nft-tokens-bc347dc148f2)

결론부터 말하면, 보안 취약점이라기보단 마켓플레이스의 인터페이스 표기 문제입니다.

## 인터페이스 표기 문제

크롬, 사파리 등의 브라우저를 통해 어떤 사이트에 접속할 때 사용자는 주소창 옆의 자물쇠 표시를 보고 해당 사이트와 연결이 안전한지 쉽게 파악할 수 있습니다.

![Untitled](/assets/2022-02-01/Untitled%205.png)

![Untitled](/assets/2022-02-01/Untitled%206.png)

각 브라우저는 신뢰할 만한 코드를 통해 해당 사이트로의 접속이 안전한지 안전하지 않은지를 판별하여 간단한 인터페이스(자물쇠 등)로 나타내는 것입니다. 이를 통해 사용자가 매번 직접 연결의 안정성을 검증할 필요 없이 자물쇠 표시만 보고도 판단이 가능하게 됩니다.

그러나 만일 브라우저의 판별 로직 혹은 코드 구현이 잘못되었을 경우 실제로는 안전한 연결을 안전하지 않다고 표시하거나, 안전하지 않은 연결을 안전하다고 표시할 수도 있습니다.

사용자가 브라우저가 안전하다고 표시한 것만을 믿고 계정 정보를 의심없이 입력해 전송하는 행위를 수행하고 그로 인해 피해를 입었을 때, 사실상 이는 인터넷 프로토콜 등의 보안 취약점으로 인한 것이 아닌 브라우저의 잘못입니다.

## 공개키와 개인키의 올바른 용도

다음은 이더리움 공식 사이트에서 열람 가능한 NFT 민팅 [튜토리얼 코드](https://ethereum.org/en/developers/tutorials/how-to-mint-an-nft/#create-txn) 중 일부입니다.

```solidity
require('dotenv').config();
const API_URL = process.env.API_URL;
const PUBLIC_KEY = process.env.PUBLIC_KEY;
const PRIVATE_KEY = process.env.PRIVATE_KEY;
...
async function mintNFT(tokenURI) {
  ...
  const tx = {
    'from': PUBLIC_KEY,
    'to': contractAddress,
    'nonce': nonce,
    'gas': 500000,
    'data': nftContract.methods.mintNFT(**PUBLIC_KEY**, tokenURI).encodeABI()
  };
}
```

코드에서 확인할 수 있듯이, 공개키와 개인키를 변수로 저장해 사용합니다.

여기서 ‘data’ 부분에서 사용하는 공개키 부분을 아래와 같이 비플의 이더리움 주소로 치환할 수 있습니다.

```solidity
require('dotenv').config();
const API_URL = process.env.API_URL;
const PUBLIC_KEY = process.env.PUBLIC_KEY;
const PRIVATE_KEY = process.env.PRIVATE_KEY;
const BEEPLE_KEY = '0xc6b0562605D35eE710138402B878ffe6F2E23807';
...
async function mintNFT(tokenURI) {
  ...
  const tx = {
    'from': PUBLIC_KEY,
    'to': contractAddress,
    'nonce': nonce,
    'gas': 500000,
    'data': nftContract.methods.mintNFT(**BEEPLE_KEY**, tokenURI).encodeABI()
  };
}
```

추가적인 다른 단계들을 거쳐야 NFTheft의 슬립민팅을 온전히 모방할 수 있지만, 방금의 단순한 조작이 핵심입니다.

라리블, 오픈씨 등의 마켓플레이스에선 개인키가 아닌 공개키를 확인해 트랜잭션(transaction)의 발송자(sender)를 표시합니다. 개인키는 본인이 아니면 알 수 없지만 공개키는 누구나 알 수 있기 때문에 이는 바람직한 판단 로직이 아닙니다.

아래는 이더스캔(Etherscan)에서 앞서 설명한 방식으로 변조한 트랜잭션 정보를 열람한 것입니다.

![(Source: [https://timdaub.github.io/2021/04/22/nft-sleepminting-beeple-provenance/](https://timdaub.github.io/2021/04/22/nft-sleepminting-beeple-provenance/))](/assets/2022-02-01/Untitled%207.png)

(Source: [https://timdaub.github.io/2021/04/22/nft-sleepminting-beeple-provenance/](https://timdaub.github.io/2021/04/22/nft-sleepminting-beeple-provenance/))

붉은 글씨로 ‘NFT sender’라고 적혀있는 부분이 마켓플레이스 판별 로직이 인식하는 원작자이고, ‘Transaction sender’라고 적혀있는 부분이 실제 원작자(트랜잭션 발송자)입니다.

NFT sender 부분은 비플의 공개키로 위조할 수 있지만 Transaction sender 부분은 정상적인 방법으로는 비플의 개인키를 알 수 없기 때문에 위조가 불가합니다.

# 결론

블록체인 혹은 이더리움 상의 보안 취약점이 아닌, 마켓플레이스의 인터페이스 표기 문제입니다.

# 참고 문헌

[https://kf106.medium.com/how-to-sleepmint-nft-tokens-bc347dc148f2](https://kf106.medium.com/how-to-sleepmint-nft-tokens-bc347dc148f2)

[https://timdaub.github.io/2021/04/22/nft-sleepminting-beeple-provenance/](https://timdaub.github.io/2021/04/22/nft-sleepminting-beeple-provenance/)

[https://www.niftynftnews.com/blog/nft-heist](https://www.niftynftnews.com/blog/nft-heist)

[https://ethereum.org/en/developers/docs/standards/tokens/erc-721/](https://ethereum.org/en/developers/docs/standards/tokens/erc-721/)

[https://docs.openzeppelin.com/contracts/4.x/api/token/erc721](https://docs.openzeppelin.com/contracts/4.x/api/token/erc721)

[https://news.artnet.com/market/beeple-everydays-controversy-nft-or-not-1952124](https://news.artnet.com/market/beeple-everydays-controversy-nft-or-not-1952124)

[https://news.artnet.com/opinion/sleepminting-nftheft-monsieur-personne-1960744](https://news.artnet.com/opinion/sleepminting-nftheft-monsieur-personne-1960744)

[https://www.coindesk.com/markets/2021/04/27/the-art-of-the-prank-how-a-hacker-tried-to-fake-the-worlds-most-expensive-nft/](https://www.coindesk.com/markets/2021/04/27/the-art-of-the-prank-how-a-hacker-tried-to-fake-the-worlds-most-expensive-nft/)