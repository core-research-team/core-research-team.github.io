---
layout: post
title: "SmartContract hacking 101"
author: "wwwlk"
comments: true
tags: [ctf, write-up]
---

라온화이트햇 핵심연구팀 조진호

# SmartContract hacking 101

# Summary

Solidity코드도 읽을 줄 모르는 상태에서 BlockChain을 이해하고 Solidity로 작성된 컨트랙 해킹까지 하는 문서이다. 일반 프로그램과 많이 다르기 때문에 컴퓨터에 익숙한 사람도 새로운 개념으로 익혀야 하는 개념이 많다. ctf문제로 genesis block, transaction, gas 등 블록체인에서 사용하는 개념을 익히고 실제 리얼월드에서 발생한 취약점을 분석해 어떠한 방식의 취약점이 악용되고 있는지 찾는 게 목표이다.

# CTF Problem

문제는 real world ctf 2018 **Monica.** 주어진 파일은 genesis.json, player-privkey.txt, AcoraidaMonicaGame.sol인데 player는 대회용이다. 목표는 취약점을 사용해 특정 주소의 잔액을 없애는 것이다.

genesis.json은 Genesis Block으로 블록체인의 시작을 정의하는데 사용된다.

![Untitled](/assets/2021-11-01/SC.png)

```jsx
{
   "config": {
      "chainId": 31231, // Ethereum chainid
      "homesteadBlock": 0,
      "eip155Block": 0,
      "eip158Block": 0,
      "byzantiumBlock": 0
   },
   "coinbase": "0x492705c00090cb7c1cbb5ec3ab0b09f310dec399",
   "difficulty": "0", // mining difficult
   "gasLimit": "10000000000000",
   "alloc": {
      "0xcf2f3781229416d78c9861c9a5f0617ba5ca96af": {
          "balance": "100000000000000000000000000000"
      },
      "0x47a1b97b7A1f1Ad90741Ea94230b2361667fa2DB": {
          "balance": "100000000000000000000000000000"
      },
      "0x19baa751d1092c906ac84ea4681fa91e269e6cb9": { // player address
          "balance": "200000000000000000000"
      },
      "0x492705c00090cb7c1cbb5ec3ab0b09f310dec399": { // admin address
          "balance": "100000000000000000000000000000"
      },
      "0xf29e621ee00eb8aca28f7fab785c054e465805e6": {
          "balance": "200000000000000000000"
      }
   }
}
```

solidity는 컴파일러로 컴파일되서 evm에서 돌아간다. 먼저 본문 sol코드

```jsx
pragma solidity =0.4.25;

contract AcoraidaMonicaGame{
    uint256 public version = 4;
    string public description = "Acoraida Monica admires smart guys, she'd like to pay 10000ETH to the one who could answer her question. Would it be you?";
    string public constant sampleQuestion = "Who is Acoraida Monica?";
    string public constant sampleAnswer = "$*!&#^[` a@.3;Ta&*T` R`<`~5Z`^5V You beat me! :D";
    Logger public constant logger=Logger(0x5e351bd4247f0526359fb22078ba725a192872f3);
    address questioner;
    string public question;
    bytes32 private answerHash;

    constructor(bytes a) {
        assembly{
        pc
        0xe1
        add
        jump
        }
    }
    ...
}
...
```

`AcoraidaMonicaGame.sol`

Logger는 다른 로깅하는 sol코드 주소다. 맨 위에 solidity컴파일러 버전을 명시한다. 생성자쪽을 보면 무슨 어셈을 쓰고 있는데 저 어셈이 evm에서 실행된다는 것 같다. 지금은 어떻게 작동하는지 모르니 일단 제껴놓는다.

```jsx
modifier onlyHuman{
        uint size;
        address addr = msg.sender;
        assembly { size := extcodesize(addr) }
        require(size==0);
        _;
    }
```

바로 밑에 보면 modifier라는 키워드가 보인다. 코드의 내용은 inline assembly로 주소의 길이를 체크하는 함수이다. 이 어셈인 extcodesize를 찾아보면 이런 글이 있다.

![Untitled](/assets/2021-11-01/SC%201.png)

대충 읽어보면 주소를 확인하는 좋은 방법인데 constructor에서 호출하면 0이 되니까 주의해야한다~ 라는 글이다. 마침 이 컨트랙트의 constructor도 먼가 이상하게 생겼다.

이 modifier 함수의 사용은 파이썬의 데코레이터처럼 사용한다.

```jsx
function TheAnswerIs(string _answer) onlyHuman public payable{
        //require(msg.sender != questioner);
        if(answerHash == keccak256(_answer) && msg.value >= 1 ether){
            questioner = msg.sender;
            msg.sender.transfer(address(this).balance);
            logger.AcoraidaMonicaWantsToKeepALogOfTheWinner(msg.sender);
        }
    }
```

이런식으로 함수선언 라인에 onlyHuman이라는 함수를 넣을 수 있게되고 저 함수를 거치고 TheAnswerIs함수로 들어가게 된다.

```jsx
function Start(string _question, string _answer) public payable{
    if(answerHash==0){
        answerHash = keccak256(_answer);
        question = _question;
        questioner = msg.sender;
    }
}
```

이 컨트랙트의 전체적인 기능 중 가장 먼저 나오는 `Start`이다. answerHash가 설정되어있지 않다면 _answer의 해시(keccak256=SHA3)값을 저장한다.

```jsx
function NewRound(string _question, bytes32 _answerHash) public payable{
    if(msg.sender == questioner && msg.value >= 0.5 ether){
        require(_answerHash != keccak256(sampleAnswer));
        question = _question;
        answerHash = _answerHash;
        logger.AcoraidaMonicaWantsToKnowTheNewQuestion(_question);
        logger.AcoraidaMonicaWantsToKnowTheNewAnswerHash(_answerHash);
    }
}
```

`NewRound`함수는 위에서 세팅한 질문이 본인인지 확인하고 value가 0.5eth이상이면 작동한다. 보낸 사람이 설정한 `_question`과 해시값으로 새 question을 세팅한다.

```jsx
function TheAnswerIs(string _answer) onlyHuman public payable{
    //require(msg.sender != questioner);
    if(answerHash == keccak256(_answer) && msg.value >= 1 ether){
        questioner = msg.sender;
        msg.sender.transfer(address(this).balance);
        logger.AcoraidaMonicaWantsToKeepALogOfTheWinner(msg.sender);
    }
}
```

그리고 마지막으로 정답을 보내면 그 해시값이 등록된 해시값과 동일한지 판단하고 동일하다면 questioner를 마지막으로 이 트랜잭션을 실행한 지갑으로 설정한 뒤 컨트랙트에 있는 모든 잔액을 트랜잭션을 보낸 주소로 보낸다. 이 함수는 `onlyHuman`함수를 거치고 실행된다. 그러므로 주소의 유효성을 검사한다.

여기서 문제점은 Start함수이다.

```jsx
function Start(string _question, string _answer) public payable{
    if(answerHash==0){
        answerHash = keccak256(_answer);
				...
```

Start함수의 파라미터로 정답이 가는데 이 정답이 해시되지 않은 상태로 컨트랙에서 해시가 되기 때문에 트랜잭션에 평문으로 어떤 문자열 파라미터가 갔는지 기록된다. 테스트를 위해 remix를 사용해 테스트했다. ([https://remix.ethereum.org](https://remix.ethereum.org/))

![Untitled](/assets/2021-11-01/SC%202.png)

여기서 Start트랜잭션에 원하는 question과 answer를 넣게되면

![Untitled](/assets/2021-11-01/SC%203.png)

반영이 되어 `question`에 데이터가 들어가고 private인 `answer`에 `"answer"`의 해시값이 들어가게 되었을 것이다.

![Untitled](/assets/2021-11-01/SC%204.png)

이런식으로 remix에서 evm code들을 디버깅 할 수 있다. evm 어셈들인 이곳([https://github.com/crytic/evm-opcodes](https://github.com/crytic/evm-opcodes))에서 확인이 가능하다. x86어셈과 비슷해서 잘 읽힌다.

![Untitled](/assets/2021-11-01/SC%205.png)

evm code는 건드릴 수 없고 스택과 메모리, storage를 사용할 수 있으면 명령어당 gas비가 책정된다.

해당 문제는 로지컬 문제가 아닌 컨트랙트에 백도어가 있던 문제였다.

```jsx
function TheAnswerIs(string _answer) onlyHuman public payable{
    //require(msg.sender != questioner);
    if(answerHash == keccak256(_answer) && msg.value >= 1 ether){
        questioner = msg.sender;
        msg.sender.transfer(address(this).balance);
        logger.AcoraidaMonicaWantsToKeepALogOfTheWinner(msg.sender);
    }
}
```

이 TheAnswerIs의 코드를 기록된 컨트랙에서 살펴보면 이런 코드가 나온다.

```jsx
[1158] SSTORE
[1161] PUSH2 0x04b1
[1162] CALLDATASIZE
[1164] PUSH1 0xa0
[1165] EQ
[1166] ISZERO
[1167] DUP2
[1168] JUMPI
```

CALLDATASIZE로 인풋의 길이를 구하고 인풋값이 0xa0이 아니라면 점프한다.

```jsx
[1171] PUSH2 0x402e
[1174] PUSH2 0x049d
[1175] CALLER
[1178] PUSH2 0xffff
[1179] AND
[1180] JUMP
```

만약 인풋값의 길이가 0xa0이라면 점프를 하지않고 계속 실행하는데 CALLER로 호출한 wallet의 주소를 가져오고 0xffff를 push하고 주소와 AND연산을 하고 그 값으로 점프한다. 이 부분으로 인해 원하는 pc로 점프뛸 수 있게된다.

익스는 내 컨트랙을 만들고 그 컨트랙을 호출하는 delegatecall을 호출하여 익스하는건데 복잡하고 리얼월드에서 발생하기 힘든 딱 맞는 오프셋을 사용한다거나 그런경우여서 패스하였다. constructor에 있던 이상한 어셈들도 jop가젯들이다 .

컨트랙 코드가 어떻게 evm코드로 구현이 되는지와 실제 컨트랙트에 쓰여진 bytecode와 solidity코드가 다른 경우를 생각하고 넘어가면 될 것같다.

# PancakeSwap Lottery Vulnerability

다음 취약점은 PancakeSwap의 로터리에서 발생한 취약점이다. 로터리는 복권같은건데 보통 유추할 수 없는 완전한 난수를 사용해 아주 낮은 확률로 당첨되게 해 놓는다. 취약점을 이용해 100%당첨이 되게하거나 특정 시점의 난수를 예측 가능할 경우 취약해진다.

PancakeSwap의 로터리는 12시간마다 복권의 결과를 보여주는데 이 결과가 나오는 동안은 복권의 구매가 불가능하다. buy함수에서는 이게 구매 못하게 되어있는데 multibuy를 이용해 복권을 drawing페이즈에 있을 때 구매할 수 있어서 추첨중일 때 결과를 보고 살 수 있었던 취약점이라고 한다.

패치를 보면 단 한줄이 추가되었다.

```jsx
function multiBuy(uint256 _price, uint8[4][] memory _numbers) external {
require (!drawed(), ‘drawed, can not buy now’);
require (_price >= minPrice, ‘price must above minPrice’);
uint256 totalPrice = 0;
```

```jsx
function multiBuy(uint256 _price, uint8[4][] memory _numbers) external {
require (!drawed(), ‘drawed, can not buy now’);
require(!drawingPhase, ‘drawing, can not buy now’); // Patched
require (_price >= minPrice, ‘price must above minPrice’);
uint256 totalPrice = 0;`
```

drawingPhase를 확인한다.

```jsx
function buy(uint256 _price, uint8[4] memory _numbers) external {
        require(!drawed(), 'drawed, can not buy now');
        require(!drawingPhase, 'drawing, can not buy now');
        require (_price >= minPrice, 'price must above minPrice');
        for (uint i = 0; i < 4; i++) {
            require (_numbers[i] <= maxNumber, 'exceed number scope');
        }
        uint256 tokenId = lotteryNFT.newLotteryItem(msg.sender, _numbers, _price, issueIndex);
        lotteryInfo[issueIndex].push(tokenId);
        if (userInfo[msg.sender].length == 0) {
            totalAddresses = totalAddresses + 1;
        }
        userInfo[msg.sender].push(tokenId);
        totalAmount = totalAmount.add(_price);
        lastTimestamp = block.timestamp;
        uint64[keyLengthForEachBuy] memory userNumberIndex = generateNumberIndexKey(_numbers);
        for (uint i = 0; i < keyLengthForEachBuy; i++) {
            userBuyAmountSum[issueIndex][userNumberIndex[i]]=userBuyAmountSum[issueIndex][userNumberIndex[i]].add(_price);
        }
        cake.safeTransferFrom(address(msg.sender), address(this), _price);
        emit Buy(msg.sender, tokenId);
    }
```

패치전의 buy함수인데 여기는 drawingPhase를 확인하고있다. 어디서 세팅하는지, drawed()와 무엇이 다른지 찾아봤다.

```jsx
function drawed() public view returns(bool) {
    return winningNumbers[0] != 0;
}
function reset() external onlyAdmin {
    require(drawed(), "drawed?");
    lastTimestamp = block.timestamp;
    totalAddresses = 0;
    totalAmount = 0;
    winningNumbers[0]=0;
    winningNumbers[1]=0;
    winningNumbers[2]=0;
    winningNumbers[3]=0;
    drawingPhase = false;
    issueIndex = issueIndex +1;
    if(getMatchingRewardAmount(issueIndex-1, 4) == 0) {
        uint256 amount = getTotalRewards(issueIndex-1).mul(allocation[0]).div(100);
        internalBuy(amount, nullTicket);
    }
    emit Reset(issueIndex);
}

function enterDrawingPhase() external onlyAdmin {
    require(!drawed(), 'drawed');
    drawingPhase = true;
}
```

drawed() → 추첨 결과값이 세팅 되었는지 확인 세팅되어있다면 true

reset() → admin만 가능하고 추첨결과를 초기화

enterDrawingPhase() → drawingPhase를 true로 세팅

```jsx
// add externalRandomNumber to prevent node validators exploiting
    function drawing(uint256 _externalRandomNumber) external onlyAdmin {
        require(!drawed(), "reset?");
        require(drawingPhase, "enter drawing phase first");
        bytes32 _structHash;
        uint256 _randomNumber;
        uint8 _maxNumber = maxNumber;
        bytes32 _blockhash = blockhash(block.number-1);

        // waste some gas fee here
        for (uint i = 0; i < 10; i++) {
            getTotalRewards(issueIndex);
        }
        uint256 gasleft = gasleft();

        // 1
        _structHash = keccak256(
            abi.encode(
                _blockhash,
                totalAddresses,
                gasleft,
                _externalRandomNumber
            )
        );
        _randomNumber  = uint256(_structHash);
        assembly {_randomNumber := add(mod(_randomNumber, _maxNumber),1)}
        winningNumbers[0]=uint8(_randomNumber);
				...
```

drawingPhase → drawed되지 않고 drawingPhase일때 실행가능하다. 예측할 수 없는 난수들로 추첨결과값을 세팅한다.

따라서 drawingPhase에 복권을 살 수 있다는건 추첨결과가 세팅되기 전에 살 수 있다는거고 admin이 enterDrawingPhase트랜잭션을 보내고 drawing트랜잭션을 보고 가스비를 더 비싸게 주고 drawing의 _externalRandomNumber를 읽어 추첨 결과값을 예측해서 넣을 수 있다는 의미인 것 같다. 실세 익스 코드나 상세 시나리오가 없어서 추측만 가능하다.

이 외에도 ctf와 상당히 다른 성격들의 컨트랙 취약점이 나온다. 위 ctf는 문제를 위한 취약점이라는 느낌이 강하다.

# Flash Loan Attack

플래시론은 defi생태계의 대출을 이용한 공격방법이다. defi중 예치를 해 놓은 코인을 담보로 대출을 해주는 서비스가 있다. 이때 빌린 코인의 수량은 동일하기 때문에 코인을 단기간 내에 펌핑시키면 대출단가가 위로 올라가 버리기 때문에 이를 이용해 차익을 먹는 공격이다.

![Untitled](/assets/2021-11-01/SC%206.png)

한 트랜잭션으로 공격이 가능하다.

- dydx에서 10,000eth 대출
- 5500eth를 컴파운드에 보내고 112btc대출
- 1300eth로 ETH/BTC x5레버리지로 예치한다. (5637eth → 51btc)
- 바로 위에서 큰 eth가 들어와서 eth의 가격이 일시적으로 떨어진다
- 이때 컴파운드에서 빌린 112btc로 낮은 아격의 eth와 교환 (차익발생)
- 이거로 빌린 10,000eth를 갚고 나머지가 다 이익이 된다.

이게 가능한 이유가 큰 돈이 오갈때 컨트랙트에서 순간적으로 엄청나게 크게 움직이기 때문이다. 시장이 돈을 감당을 못해서 이런 현상이 발생한다.

패치는 위처럼 큰 변동이 들어올때 체크하여 안되게 하고, ETHBTC마진을 없앤다는데 이건 의미가 없지않나 싶다.

![Untitled](/assets/2021-11-01/SC%207.png)

실제 취약점이 발생했던 코드인데 require의 조건문을 자세히 보면 OracleInterface의 shouldLiquidate함수를 들어가야 유동성 체크를 정상적으로 하는데 or조건문으로 되어있어서 kyber network이고, 대출 물량이 있기만 하면 포지션 체크를 하지 않고 넘어간 것을 알 수 있다.

가장 간단한 공격 방법이지만 이런 로지컬 버그들이 훨씬 큰 버그라고 생각한다.

# Reference

[https://www.youtube.com/watch?v=ozqOlUVKL1s](https://www.youtube.com/watch?v=ozqOlUVKL1s)

[https://www.youtube.com/watch?v=P8LXLoTUJ5g](https://www.youtube.com/watch?v=P8LXLoTUJ5g)

[https://takenobu-hs.github.io/downloads/ethereum_evm_illustrated.pdf](https://takenobu-hs.github.io/downloads/ethereum_evm_illustrated.pdf)

[https://medium.com/immunefi/pancakeswap-lottery-vulnerability-postmortem-and-bug-4febdb1d2400](https://medium.com/immunefi/pancakeswap-lottery-vulnerability-postmortem-and-bug-4febdb1d2400)

[https://medium.com/iotrustlab/플래시-론-공격-flash-loan-attack-원리-및-방법-분석-94e01fe1a9c8](https://medium.com/iotrustlab/%ED%94%8C%EB%9E%98%EC%8B%9C-%EB%A1%A0-%EA%B3%B5%EA%B2%A9-flash-loan-attack-%EC%9B%90%EB%A6%AC-%EB%B0%8F-%EB%B0%A9%EB%B2%95-%EB%B6%84%EC%84%9D-94e01fe1a9c8)

[https://peckshield.medium.com/bzx-hack-full-disclosure-with-detailed-profit-analysis-e6b1fa9b18fc](https://peckshield.medium.com/bzx-hack-full-disclosure-with-detailed-profit-analysis-e6b1fa9b18fc)