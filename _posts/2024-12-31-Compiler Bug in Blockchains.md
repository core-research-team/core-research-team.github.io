---
layout: post
title:  “Compiler Bug in Blockchains"
author: "kafuuchin0"
comments: true
tags: [Research, Web3, Vyper, Ethereum]
---
# Compiler Bug in Blockchains

# 1. 개요

- 현대 소프트웨어에서 컴파일러 오작동은 Javascript JIT 엔진에서 발생하는 취약점들에서 확인할 수 있듯이 보안에 치명적인 영향을 끼칠수 있습니다. 또한 컴파일러는 매우 복잡한 소프트웨어이기 때문에, 컴파일러에 사용자가 임의의 코드를 입력할 수 있는 경우에 특히 컴파일러 동작이 오작동 없이 동작하여아 합니다.
- 현재 스마트 컨트랙트를 지원하는 블록체인 플랫폼들도 컴파일러의 복잡도가 커짐에 따라서 컴파일러 버그에 대한 위험도가 증가하고, 또한 해당 버그를 통한 재산손실이 발생하고 있습니다.
- 본 글에서는 Vyper의 Reentrancy 버그를 분석하여 어떠한 영향을 끼치는지, 어떻게 발생하는지 조사하였습니다.

# 2. 배경지식

이 글의 주제가 되는 취약점에 대해 이해하기위해서는 Vyper라는 언어와 EVM, 그리고 Re-entrancy 취약점에 대해서 이해하여야합니다.

## 2.1 EVM

- 이더리움은 [공식 문서](https://ethereum.org/en/developers/docs/evm/)에 따르면 분산된 상태 기계([State Machine](https://en.wikipedia.org/wiki/Finite-state_machine))로 설명할 수 있습니다.
    - 계정 정보(잔액, 주소)와 컨트랙트와 관련된 변수 상태를 ‘상태’라고 볼 수 있습니다.
    - 이러한 ‘상태’들은 일정한 ‘규칙’으로 다른 상태로 전이할수 있습니다.
        - 이러한 규칙을 정의하는것이 EVM입니다.
    - 각 노드들은 EVM을 통해서 다음 상태를 계산하고, 이를 합의 알고리즘을 통하여 블록체인 네트워크에 합병합니다(블록 생성).

![image.png](https://ethereum.org/_next/image/?url=%2Fcontent%2Fdevelopers%2Fdocs%2Fevm%2Fevm.png&w=828&q=75)

- EVM은 Stack Machine 구조로 동작합니다.
    - Stack의 깊이는 1024로, 각 스택은 256bit의 길이를 갖습니다.
- 또한 각 Contract들은 데이터를 저장하기 위해 32바이트 단위의 Storage Slot을 갖습니다. 각 Storage는 고유한 정수(인덱스)로 접근합니다.
- EVM의 어셈블리 명령어 세트는 다음 링크에서 확인할 수 있습니다.
    - [https://ethereum.org/en/developers/docs/evm/opcodes/](https://ethereum.org/en/developers/docs/evm/opcodes/)
- 이러한 어셈블리 명령어로 컨트랙트를 작성하는것은 번거롭기 때문에 Solidity, Vyper와 같은 고수준 언어를 EVM 코드로 컴파일하여 스마트 컨트랙트를 작성하게 됩니다.

## 2.2 Account & Contract

이더리움에서 Account는 ether(ETH)를 보낼수 있는 주체를 뜻합니다.

Account는 총 두 종류로 분류할 수 있습니다.

- Externally-owned account - Private Key를 보유한 사람이라면 제어할 수 있는 계정
- Contract account - 스마트 컨트랙트 코드에 의해 제어되는 계정

두 종류 모두 ETH를 받거나 보낼수 있습니다. 또한 다른 컨트랙트와 상호작용(함수 호출, 송금, …)이 가능합니다.

- 즉, **스마트 컨트랙트간 서로 상호 작용도 가능합니다.**

## 2.3 Re-entrancy Bug

Re-entrancy 취약점은 한 컨트랙트 함수의 실행이 끝나기 전에 다시 그 함수를 호출할 수 있는 상황(콜백)을 이용하여 악의적인 동작을 수행하는 취약점입니다.

```solidity
contract EtherStore {
    mapping(address => uint256) public balances;

    function deposit() public payable {
        balances[msg.sender] += msg.value;
    }

    function withdraw() public {
        uint256 bal = balances[msg.sender];
        require(bal > 0);

        **(bool sent,) = msg.sender.call{value: bal}(""); // [1]**
        require(sent, "Failed to send Ether");

        balances[msg.sender] = 0;
    }

    // Helper function to check the balance of this contract
    function getBalance() public view returns (uint256) {
        return address(this).balance;
    }
}
```

- 컨트랙트는 이더리움을 수신받았을때 실행되는 fallback함수를 구현할 수 있습니다.
- [1] 부분에서 EtherStore는 balance[msg.sender]만큼의 잔액을 호출자에게 전송하고 있습니다. 하지만 만약 공격자가 fallback함수를 구현하여 withdraw를 다시 호출하게 된다면 다음과 같이 코드가 실행되게 됩니다.

```solidity
withdraw()
	Attacker's fallback()
	withdraw()
		Attacker's fallback()
		withdraw()
			...
```

- 그러면 결국 [1]코드가 반복적으로 실행되게 되어 공격자는 컨트랙트에 있는 잔액을 전부 가져올수 있게 됩니다.

또한 이러한 취약점은 단순히 하나의 함수를 재귀적으로 호출하는것이 아닌, 서로 다른 함수를 호출하여 악용하는 상황도 가능합니다.

```solidity
pragma solidity ^0.8.0;

contract ReentrancyExample {
    mapping(address => uint256) public balances;

    // Deposit funds
    function deposit() external payable {
        balances[msg.sender] += msg.value;
    }

    // Withdraw specific amount
    function withdraw(uint256 amount) external {
        require(balances[msg.sender] >= amount, "Insufficient balance");
        
        (bool success,) = msg.sender.call{value: amount}("");
        require(success, "Transfer failed");

        balances[msg.sender] -= amount;
    }

    // Withdraw full balance
    function withdrawAll() external {
        uint256 balance = balances[msg.sender];
        require(balance > 0, "No balance to withdraw");
        
        (bool success,) = msg.sender.call{value: balance}("");
        require(success, "Transfer failed");

        balances[msg.sender] = 0;
    }
}
```

- 위와 같은 두 기능이 존재한다면 공격자는 withdraw함수를 호출하고, fallback함수를 이용하여 withdrawAll을 호출하게 된다면, 공격자는 잔액의 두배에 해당하는 금액을 인출할 수 있습니다.

이러한 취약점은 [Checks-Effects-Interaction](https://docs.soliditylang.org/en/latest/security-considerations.html#use-the-checks-effects-interactions-pattern) 패턴의 구현을 통해서 해결하거나 혹은 다음과 같이 ReentrancyGuard을 구현하여 재진입을 방지하여 해결할 수 있습니다.

```solidity
// SPDX-License-Identifier: MIT
pragma solidity ^0.8.26;

contract ReEntrancyGuard {
    bool internal locked;

    modifier noReentrant() {
        require(!locked, "No re-entrancy");
        locked = true;
        _;
        locked = false;
    }
}

contract EtherStore is ReEntrancyGuard{
    mapping(address => uint256) public balances;

    function deposit() public payable {
        balances[msg.sender] += msg.value;
    }

    function withdraw() public noReentrant(){
        uint256 bal = balances[msg.sender];
        require(bal > 0);

        **(bool sent,) = msg.sender.call{value: bal}(""); // [1]**
        require(sent, "Failed to send Ether");

        balances[msg.sender] = 0;
    }

    // Helper function to check the balance of this contract
    function getBalance() public view returns (uint256) {
        return address(this).balance;
    }
}
```

- 이러한 보호방법은 lock 변수를 체크하는데 추가적인 코드 실행이 필요하기 때문에 Gas 소비가 있다는 단점이 존재합니다.
    - OpenZeppelin에서도 이러한 방식으로 구현된 [ReentrancyGuard](https://github.com/OpenZeppelin/openzeppelin-contracts/blob/master/contracts/utils/ReentrancyGuard.sol)를 제공합니다.

## 2.4 Vyper

Vyper는 EVM를 목적코드로 하는 언어입니다.

파이썬과 비슷한 문법을 가지고있으며, 함수 데코레이터를 통해 컨트랙트 접근 제어를 간단하게 구현할수 있으며, 정적 타입을 채택하여 안정성을 보장하는것을 목표로 하는 언어입니다.

다음과 같이 함수의 데코레이터를 통해 외부에서 접근 가능한 함수를 지정할 수 있습니다.

솔리디티와는 다르게 각 파일 한개가 컨트랙트 하나를 표현합니다.

```python
@external
def add_seven(a: int128) -> int128:
    return a + 7

@external
def add_seven_with_overloading(a: uint256, b: uint256 = 3):
    return a + b
```

또한 `@nonreentrant` 데코레이터를 통한 Reentrancy Attack을 방지할 수 있습니다.

```python
@external
@nonreentrant
def make_a_call(_addr: address):
    # this function is protected from re-entrancy
    ...
```

# 3. Root Cause

vyper의 0.4.0 미만의 버전에서는 nonreentrancy의 key 인자를 제공할 수 있었습니다.

```solidity
@external
@nonreentrant("make_a_call_lock")
def make_a_call(_addr: address):
    # this function is protected from re-entrancy
    ...
```

이러한 기능을 제공한 이유는 각 함수 별로 락 변수를 제공하고 체크하여 좀 더 유연하게 re-entrancy lock을 활용 할 수 있도록 구현하기 위해서입니다.

컴파일러에서는 이러한 락을 구현하기 위해서 각 nonreentrant마다 storage 공간을 만들어, 락 여부를 저장하고 있습니다.

해당 취약점은 이러한 스토리지 공간 할당 부분에서 발생하게 됩니다.

```solidity
def set_storage_slots(vyper_module: vy_ast.Module) -> StorageLayout:
    storage_slot = 0

    ret: Dict[str, Dict] = {}

    **for node in vyper_module.get_children(vy_ast.FunctionDef): // [1]
       type_ = node._metadata["type"]
        if type_.nonreentrant is not None:
            type_.set_reentrancy_key_position(StorageSlot(storage_slot))

            variable_name = f"nonreentrant.{type_.nonreentrant}"
            ret[variable_name] = {
                "type": "nonreentrant lock",
                "location": "storage",
                "slot": storage_slot, 
            }

            storage_slot += 1**

    for node in vyper_module.get_children(vy_ast.AnnAssign):
        type_ = node.target._metadata["type"]
        type_.set_position(StorageSlot(storage_slot))

        # this could have better typing but leave it untyped until
        # we understand the use case better
        ret[node.target.id] = {"type": str(type_), "location": "storage", "slot": storage_slot}

        # CMC 2021-07-23 note that HashMaps get assigned a slot here.
        # I'm not sure if it's safe to avoid allocating that slot
        # for HashMaps because downstream code might use the slot
        # ID as a salt.
        storage_slot += math.ceil(type_.size_in_bytes / 32)

    return ret

```

- 위 코드는 Vyper 코드의 AST를 순회하면서 변수가 사용할 Stroage Slot을 할당하는 코드입니다. 해당 코드에서 첫번째 반복문([1])은 함수 정의 노드를 순회하면서 각 Reentrancy 키마다 사용할 Storage Slot을 할당하고 있습니다.
    - 이때 반복문을 순회하면서 각 noreentrant key마다 다른 Storage Slot을 할당하는것처럼 보입니다.
    - 하지만 이미 storage가 할당된 noreentrant key에 대해서도 슬롯이 할당되어 실제로는 서로 다른 슬롯을 참조하여 락을 사용하는 상황이 발생됩니다.

```python
balanceOf: public(HashMap[address, uint256])
totalSupply: public(uint256)

@external
@payable
def mint(to: address, amount: uint256):
    self.balanceOf[to] += amount
    self.totalSupply += amount

@external
@nonreentrant("lock")
def withdrawToken(amount: uint256):
    assert self.balanceOf[msg.sender] >= amount, "Insufficient token balance"

    self.balanceOf[msg.sender] -= amount
    self.totalSupply -= amount

    raw_call(msg.sender, b"", value=amount)

@external
@nonreentrant("lock")
def withdrawAllTokens():
    amount: uint256 = self.balanceOf[msg.sender]

    self.balanceOf[msg.sender] = 0
    self.totalSupply -= amount

    raw_call(msg.sender, b"", value=amount)
```

- 따라서 위와 같은 코드에서 분명 같은 “lock” 키를 사용하고있지만, 서로 실제로는 락을 공유하지 않기 때문에, withdrawToken()에서 fallback함수를 이용하여 withdrawAllTokens() 함수를 호출하여 동일하게 재진입 공격이 가능합니다.
- 취약한 버전의 컴파일러로 위 예제를 컴파일하여 생성한 IR(중간언어)을 확인하여 실제로 어떻게 스토리지가 할당되었는지 알아보겠습니다.

```json
[seq,
  [return,
    0,
    [lll,
      [seq,
//...
            [assert, [iszero, callvalue]],
            # Line 13
            [seq,
              [if,
                **[eq, _calldata_method_id, 1354409506 <0x50baa622: withdrawToken(uint256)>],**
                [seq,
                  [goto, external_withdrawToken__uint256__common],
                  [seq,
                    [label, external_withdrawToken__uint256__common],
                    **[seq, [assert, [iszero, [sload, 0]]], [sstore, 0, 1]],**
                    # Line 14
                    [if,
                      [iszero, [ge, [sload, [sha3_64, 2 <self.balanceOf>, caller]], [calldataload, 4]]],
                      [seq,
                        [mstore, 320, 147028384],
                        [mstore, 352, 32],
                        /* Create String[26]: b'Insufficient token balance' */
                        [seq,
                          [mstore, 384, 26],
                          [mstore,
                            416,
                            33213987989631693067883787898815218034590801379243923128911631538818325151744],
                          384],
                        [revert, 348, 100]]],
                    # Line 16
                    [with,
                      _stloc,
                      [sha3_64, 2 <self.balanceOf>, caller],
                      [sstore,
                        _stloc,
                        [with,
                          l,
                          [sload, _stloc],
                          [with, r, [calldataload, 4], [seq, [seq, [assert, [ge, l, r]], [sub, l, r]]]]]]],
                    # Line 17
                    [with,
                      _stloc,
                      3 <self.totalSupply>,
                      [sstore,
                        _stloc,
                        [with,
                          l,
                          [sload, _stloc],
                          [with, r, [calldataload, 4], [seq, [seq, [assert, [ge, l, r]], [sub, l, r]]]]]]],
                    # Line 19
                    [seq,
                      /* copy bytestring to memory */
                      [with,
                        src,
                        /* Create Bytes[0]: b'' */ [seq, [mstore, 320, 0], 320],
                        [with,
                          sz,
                          [add, 32, [mload, src]],
                          [assert, [call, gas, 4, 0, src, sz, 352, sz]]]],
                      [assert, [call, gas, caller, [calldataload, 4], 384, [mload, 352], 0, 0]]],
                    # Line 13
                    [label, external_withdrawToken__uint256__cleanup],
                    **[sstore, 0, 0],**
                    stop]]]],
            # Line 23
            [seq,
              [if,
                [eq, _calldata_method_id, 671983354 <0x280da6fa: withdrawAllTokens()>],
                [seq,
                  [goto, external_withdrawAllTokens____common],
                  [seq,
                    [label, external_withdrawAllTokens____common],
                    **[seq, [assert, [iszero, [sload, 1]]], [sstore, 1, 1]],**
                    /* amount: uint256 = self.balanceOf[msg.sender] */
                    [mstore,
                      320,
                      [sload, [sha3_64, 2 <self.balanceOf>, caller]]],
                    # Line 26
                    /* self.balanceOf[msg.sender] = 0 */
                    [sstore,
                      [sha3_64, 2 <self.balanceOf>, caller],
                      0],
                    # Line 27
                    [with,
                      _stloc,
                      3 <self.totalSupply>,
                      [sstore,
                        _stloc,
                        [with,
                          l,
                          [sload, _stloc],
                          [with,
                            r,
                            [mload, 320 <amount>],
                            [seq, [seq, [assert, [ge, l, r]], [sub, l, r]]]]]]],
                    # Line 29
                    [seq,
                      /* copy bytestring to memory */
                      [with,
                        src,
                        /* Create Bytes[0]: b'' */ [seq, [mstore, 352, 0], 352],
                        [with,
                          sz,
                          [add, 32, [mload, src]],
                          [assert, [call, gas, 4, 0, src, sz, 384, sz]]]],
                      [assert, [call, gas, caller, [mload, 320 <amount>], 416, [mload, 384], 0, 0]]],
                    # Line 23
                    [label, external_withdrawAllTokens____cleanup],
                    **[sstore, 1, 0],**
                    stop]]]],
// .. skip
      0]]]
```

- 볼드체 처리한 부분에서 알 수 있듯이, 서로 같은 lock key를 사용함에도 서로 다른 Storage Slot을 사용하고 있습니다.

# 4. Fix

- 취약했던 반복문이 다음과 같이 수정되었습니다.

```solidity
for node in vyper_module.get_children(vy_ast.FunctionDef):
    type_ = node._metadata["type"]
    if type_.nonreentrant is None:
        continue
    variable_name = f"nonreentrant.{type_.nonreentrant}"

    # a nonreentrant key can appear many times in a module but it
    # only takes one slot. after the first time we see it, do not
    # increment the storage slot.
    **if variable_name in ret:
        _slot = ret[variable_name]["slot"]
        type_.set_reentrancy_key_position(StorageSlot(_slot))
        continue**

    type_.set_reentrancy_key_position(StorageSlot(storage_slot))
    # TODO this could have better typing but leave it untyped until
    # we nail down the format better
    ret[variable_name] = {
        "type": "nonreentrant lock",
        "location": "storage",
        "slot": storage_slot,
    }
    # TODO use one byte - or bit - per reentrancy key
    # requires either an extra SLOAD or caching the value of the
    # location in memory at entrance
    storage_slot += 1
```

- 위와 조건문에서 이미 같은 이름의 reentrancy key가 존재한다면 해당 위치의 슬롯을 사용하게끔 변경되었음을 확인할 수 있습니다.
- 패치된 버전으로 동일한 예제를 컴파일하였을 때의 IR를 확인해보면 다음과 같습니다.

```json
[seq,
  [return,
    0,
    [lll,
      0,
      [seq,
///...
            # Line 13
            [seq,
              [if,
                **[iszero, [xor, _calldata_method_id, 1354409506 <0x50baa622: withdrawToken(uint256)>]],**
                [seq,
                  [goto, external_withdrawToken__uint256__common],
                  [seq,
                    [label, external_withdrawToken__uint256__common],
                    **[seq, [assert, [iszero, [sload, 0]]], [sstore, 0, 1]],**
                    # Line 14
                    /* assert self.balanceOf[msg.sender] >= amount, "Insufficient token balance" */
                    [if,
                      [iszero,
                        /* self.balanceOf[msg.sender] >= amount */
                        [ge,
                          [sload,
                            /* self.balanceOf[msg.sender] */
                            [sha3_64,
                              1 <self.balanceOf>,
                              caller <msg.sender>]],
                          [calldataload, 4 <amount>]]],
                      [seq,
                        /* "Insufficient token balance" */
                        [seq,
                          [mstore, 224, 26],
                          [mstore,
                            256,
                            33213987989631693067883787898815218034590801379243923128911631538818325151744],
                          224],
                        /* Zero pad */
                        [with,
                          len,
                          [mload, 224],
                          [with,
                            dst,
                            [add, 256, len],
                            /* mzero */ [calldatacopy, dst, calldatasize, [sub, [ceil32, len], len]]]],
                        [mstore, 160, 147028384],
                        [mstore, 192, 32],
                        [revert, 188, [add, 68, [ceil32, [mload, 224]]]]]],
                    # Line 16
                    /* self.balanceOf[msg.sender] -= amount */
                    [with,
                      _stloc,
                      /* self.balanceOf[msg.sender] */
                      [sha3_64,
                        1 <self.balanceOf>,
                        caller <msg.sender>],
                      [sstore,
                        _stloc,
                        [with,
                          l,
                          [sload, _stloc],
                          [with, r, [calldataload, 4 <amount>], [seq, [assert, [ge, l, r]], [sub, l, r]]]]]],
                    # Line 17
                    /* self.totalSupply -= amount */
                    [with,
                      _stloc,
                      2 <self.totalSupply>,
                      [sstore,
                        _stloc,
                        [with,
                          l,
                          [sload, _stloc],
                          [with, r, [calldataload, 4 <amount>], [seq, [assert, [ge, l, r]], [sub, l, r]]]]]],
                    # Line 19
                    /* raw_call(msg.sender, b"", value=amount) */
                    [if,
                      [iszero,
                        [seq,
                          /* b"" */ [seq, [mstore, 224, 0], 224],
                          [call,
                            gas,
                            caller <msg.sender>,
                            [calldataload, 4 <amount>],
                            256,
                            [mload, 224],
                            0,
                            0]]],
                      [seq, [returndatacopy, 0, 0, returndatasize], [revert, 0, returndatasize]]],
                    # Line 13
                    [label, external_withdrawToken__uint256__cleanup],
                    **[sstore, 0, 0],**
                    stop]]]],
            # Line 23
            [seq,
              [if,
                **[iszero, [xor, _calldata_method_id, 671983354 <0x280da6fa: withdrawAllTokens()>]],**
                [seq,
                  [goto, external_withdrawAllTokens____common],
                  [seq,
                    [label, external_withdrawAllTokens____common],
                    **[seq, [assert, [iszero, [sload, 0]]], [sstore, 0, 1]],**
                    /* amount: uint256 = self.balanceOf[msg.sender] */
                    [mstore,
                      224,
                      [sload,
                        /* self.balanceOf[msg.sender] */
                        [sha3_64,
                          1 <self.balanceOf>,
                          caller <msg.sender>]]],
                    # Line 26
                    /* self.balanceOf[msg.sender] = 0 */
                    [sstore,
                      /* self.balanceOf[msg.sender] */
                      [sha3_64,
                        1 <self.balanceOf>,
                        caller <msg.sender>],
                      0 <0>],
                    # Line 27
                    /* self.totalSupply -= amount */
                    [with,
                      _stloc,
                      2 <self.totalSupply>,
                      [sstore,
                        _stloc,
                        [with,
                          l,
                          [sload, _stloc],
                          [with, r, [mload, 224 <amount>], [seq, [assert, [ge, l, r]], [sub, l, r]]]]]],
                    # Line 29
                    /* raw_call(msg.sender, b"", value=amount) */
                    [if,
                      [iszero,
                        [seq,
                          /* b"" */ [seq, [mstore, 256, 0], 256],
                          [call,
                            gas,
                            caller <msg.sender>,
                            [mload, 224 <amount>],
                            288,
                            [mload, 256],
                            0,
                            0]]],
                      [seq, [returndatacopy, 0, 0, returndatasize], [revert, 0, returndatasize]]],
                    # Line 23
                    [label, external_withdrawAllTokens____cleanup],
                    **[sstore, 0, 0],**
                    stop]]]],

        [seq, [label, fallback], /* Default function */ [revert, 0, 0]]]]]]
```

- 이제 같은 lock key를 사용하면 같은 Storage Slot을 사용함을 확인 할 수 있었습니다.
- 또한 Vyper 0.4.0부터는 이제 전역 reentrancy lock만을 제공하기 때문에 근본적으로 해당 이슈는 없어지게 되었습니다.

# 5. 결론

Vyper 개발진이 작성한 해당 버그에 대한 Post-mortem 보고서를 확인 하였을때 해당 버그도 다른 컴파일러 버그와 비슷하게 컴파일러의 최적화 도입으로 인한 버그로 확인되었습니다. 해당 버그는 Storage 슬롯의 효율성을 위하여 도입한 패치가 원인이 되었습니다.

또한 ZKSync는 LLVM을 이용하여 컴파일러를 구현했는데 [LLVM의 최적화가 원인이 된 버그](https://governance.aave.com/t/bgd-aave-v3-zksync-activation-issue-report/18819) 또한 발견된 사례가 있었습니다.

최근에는 Rust와 같은 언어들이 컴파일 타임에 메모리와 관련된 안정성을 분석하기도 하고, Vyper의 예제와 같이 타이핑, 보안 관련 기능을 언어에 도입하는 경우가 많아졌습니다. 또한 여러 컴파일러 최적화 기술을 도입함에 따라서, 이번 예시와 같이 기존 보안 기능들이 오동작하는 경우가 발생 할 수 있고 이는 곧 취약점으로 이어질수 있습니다.하지만 컴파일러 버그는 자바스크립트 JIT과 같은 환경이 아니면 무시되는 경향이 있습니다. 하지만 블록체인 플랫폼에서는 이러한 이슈로 인하여 큰 재산 손실이 발생할 수 있습니다.

- 일례로 이번에 소개 드린 Vyper 버그로 인하여 [7000만달러의 손실](https://hackmd.io/@LlamaRisk/BJzSKHNjn)이 발생하였습니다.
- Fuel-Swayland 네트워크 또한 [컴파일러 이슈로 인하여 2일동안 네트워크가 다운](https://anatomi.st/2024/11/07/Swaylend_shutdown/)되었습니다.

따라서 실제 취약점 진단 상황에서도 대상에 따라 컴파일러에 대한 이해와 이슈를 계속 확인하여야 합니다.

# Reference

- [https://ethereum.org/en/developers/docs/evm/](https://ethereum.org/en/developers/docs/evm/)
- [https://docs.vyperlang.org/en/stable/](https://docs.vyperlang.org/en/stable/)
- [https://solidity-by-example.org/](https://solidity-by-example.org/)
- [https://hackmd.io/@vyperlang/HJUgNMhs2#Corruption-Avoidance-Proper-Offset-Calculation](https://hackmd.io/@vyperlang/HJUgNMhs2#Corruption-Avoidance-Proper-Offset-Calculation)
- [https://anatomi.st/2024/11/07/Swaylend_shutdown/#more](https://anatomi.st/2024/11/07/Swaylend_shutdown/#more)
- [https://github.com/vyperlang/vyper](https://github.com/vyperlang/vyper)