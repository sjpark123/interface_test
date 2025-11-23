# cynw_p2p vs cynw_p2p_vld_only 상세 비교

## 1. cynw_p2p란?

**cynw_p2p**는 **완전한 핸드셰이크(Full Handshake)** 메커니즘을 사용하는 스트리밍 프로토콜입니다.

- **신호**: `data`, `vld` (valid), `busy` (또는 `ready`)
- **백프레셔(Backpressure) 지원**: 수신자가 준비되지 않으면 송신자를 제어할 수 있음
- **신뢰성**: 데이터 손실 없이 안전한 전송 보장
- **용도**: 수신자의 준비 상태가 보장되지 않는 일반적인 데이터 스트리밍

## 2. cynw_p2p_vld_only란?

**cynw_p2p_vld_only**는 cynw_p2p의 **단순화된 버전**으로, **부분 핸드셰이크(Partial Handshake)**만 제공합니다.

- **신호**: `data`, `vld` (valid) **만 존재** (busy 신호 없음)
- **백프레셔 없음**: 수신자가 송신자를 제어할 수 없음
- **장점**: 하드웨어 단순화, 타이밍 개선
- **단점**: 잘못 사용하면 데이터 손실 가능

---

## 3. 핵심 차이점

### 프로토콜 신호 비교

| 항목 | cynw_p2p | cynw_p2p_vld_only |
|------|----------|-------------------|
| **data 신호** | ✅ 있음 | ✅ 있음 |
| **vld 신호** | ✅ 있음 | ✅ 있음 |
| **busy/ready 신호** | ✅ 있음 | ❌ **없음** |
| **백프레셔** | ✅ 지원 | ❌ 지원 안 함 |
| **핸드셰이크** | Full (완전) | Partial (부분) |

### 동작 방식 차이

#### cynw_p2p (Full Handshake)
```
Writer               Reader
  |                    |
  |---> vld + data --->|
  |                    |
  |<--- busy/ready <---|  ← 수신자가 준비 상태 전달
  |                    |
```
- 수신자가 `busy` 신호로 흐름 제어 가능
- 수신자가 준비되지 않으면 송신자가 대기
- **데이터 손실 없음**

#### cynw_p2p_vld_only (Partial Handshake)
```
Writer               Reader
  |                    |
  |---> vld + data --->|
  |                    |  (피드백 신호 없음)
  |---> vld + data --->|
  |                    |
```
- 수신자가 송신자를 제어할 수 없음
- 송신자는 계속 데이터 전송
- **수신자가 매 클럭마다 listening 해야 함**
- 수신하지 않으면 **데이터 손실**

### 주요 동작 차이점

| 특징 | cynw_p2p | cynw_p2p_vld_only |
|------|----------|-------------------|
| **수신자 책임** | 준비되면 수신 | **매 클럭마다 모니터링 필수** |
| **데이터 손실** | 없음 | listening 안 하면 손실 |
| **데이터 기억** | `nb_can_get()` 후 데이터 유지 | 같은 사이클에 `nb_get()` 안 하면 손실 |
| **nb_can_put()** | 수신자 상태 반영 | **항상 true 반환** |
| **하드웨어 복잡도** | 높음 | **낮음 (단순화)** |
| **타이밍** | 일반 | **개선됨** |
| **성능** | 백프레셔로 인한 지연 가능 | 높은 처리량 가능 |

---

## 4. 사용 시기

### cynw_p2p_vld_only를 사용해야 하는 경우:

✅ **하드웨어 단순화와 타이밍 개선이 중요한 경우**

✅ **수신자가 항상 준비 상태를 보장할 수 있는 경우**

✅ 단순한 애플리케이션 또는 예측 가능한 데이터 흐름

✅ 고성능, 고처리량이 필요한 경우

✅ `CYNW_II1_BEH` 옵션 사용 시 성능 이점

### cynw_p2p를 사용해야 하는 경우:

✅ **신뢰성 있는 데이터 전송과 백프레셔가 필요한 경우**

✅ 수신자의 준비 상태가 보장되지 않는 경우

✅ **데이터 무결성이 중요하고 손실이 허용되지 않는 경우**

✅ 복잡한 시스템에서 컴포넌트 속도가 다른 경우

✅ 동적인 처리 부하가 있는 경우

---

## 5. 코드 예제

### cynw_p2p_vld_only 비차단(Non-Blocking) 코딩 스타일

```cpp
while (1) {
    HLS_PIPELINE_LOOP(SOFT_STALL, 1, "main pipeline");

    // 두 입력 모두 유효한 값 확인
    DT v1, v2;
    bool valid1 = false, valid2 = false;

    // 입력 1 확인 및 수신
    if (din1.nb_can_get()) {
        din1.nb_get(v1);
        valid1 = true;
    }

    // 입력 2 확인 및 수신
    if (din2.nb_can_get()) {
        din2.nb_get(v2);
        valid2 = true;
    }

    // 출력 계산
    DT o1, o2;
    f(v1, v2, o1, o2);

    // 유효한 입력이 수신된 경우에만 출력 쓰기
    dout1.nb_put(o1, valid1);  // valid1이 true일 때만 유효
    dout2.nb_put(o2, valid2);  // valid2가 true일 때만 유효

    wait();  // 메인 루프에서 단일 wait
}
```

### 중요 구현 세부사항

#### 1. 수신자는 항상 listening 해야 함
- cynw_p2p_vld_only에서 수신자는 매 클럭 사이클마다 입력을 모니터링해야 함
- `get()` 또는 `nb_get()`을 호출하지 않으면 데이터 손실

#### 2. Zero-Time put()
- `CYNW_II1_BEH` 옵션 설정 시, `put()` 함수가 시간을 소비하지 않음
- 여러 출력 포트에 순차적으로 `put()` 호출 가능 (성능 영향 없음)

#### 3. API 일관성
- cynw_p2p_vld_only의 클래스 이름과 API는 cynw_p2p와 동일한 규칙
- 기존 cynw_p2p 프로젝트를 쉽게 변환 가능
- **단, 직접 연결은 불가능** (신호 레벨 프로토콜이 다름)

#### 4. 데이터 유지 불가
- `nb_can_get()`이 true를 반환했지만 같은 사이클에 `nb_get()`을 호출하지 않으면
- **해당 값은 손실됨** (cynw_p2p와 다른 점)

---

## 6. 프로토콜 상세 비교

### cynw_p2p_vld_only 프로토콜

- **전송 속도**: 클럭당 1개 값
- **vld 신호**: Active high, 유효/무효 데이터 표시
- **가정**: 수신자가 항상 준비됨
- **특성**: 수신자가 송신자를 제어할 수 없음
- **데이터 검증**: `vld`가 클럭 엣지에서 high일 때만 유효

### cynw_p2p 프로토콜

- **전송 속도**: 클럭당 1개 값 (백프레셔 없을 때)
- **vld 신호**: 송신자가 데이터 유효성 표시
- **busy/ready 신호**: 수신자가 준비 상태 표시
- **특성**: 완전한 동기화 메커니즘
- **신뢰성**: 데이터 손실 방지 보장

---

## 7. 원문 발췌 (Stratus HLS User Guide)

### cynw_p2p_vld_only 프로토콜 특성

> The cynw_p2p_vld_only protocol uses two signals:
> - a data signal (with user-defined data type), and
> - a one-bit vld signal.
>
> Unlike cynw_p2p, there is no busy signal.

### 프로토콜의 핵심 특성

> The protocol has the following characteristics:
> - Data can be transferred at the rate of 1 value per clock cycle.
> - The vld signal, active high, indicates valid or invalid data during any given clock cycle.
> - The reader cannot control the writer, since there is no busy or ready signal to indicate whether the reader is ready to receive data.
> - It is assumed that the reader of a cynw_p2p_vld_only data transfer is always ready to receive.
> - **This is the crucial difference between cynw_p2p and cynw_p2p_vld_only.**
> - The reader must monitor the input every clock cycle for valid data. If this is not done, valid data written by the writer will be lost.

### cynw_p2p와의 주요 차이점

> **Differences Between cynw_p2p_vld_only and cynw_p2p**
>
> - The cynw_p2p_vld_only protocol does not support backpressure.
> - It is always possible to write a value to the channel; the receiver is assumed to be always listening.
> - This implies that `nb_can_put()` on the output metaport always returns true, and `put()` can be called at any time.
> - Unlike cynw_p2p, the cynw_p2p_vld_only protocol does not remember valid values that may have been seen on an input port but not actually received by the calling thread.
> - Such values will be lost when using cynw_p2p_vld_only.

### 성능 이점

> When the `CYNW_II1_BEH` option is set on output ports, the `put()` function consumes no time, and multiple `put()` functions can be called on different output ports in sequence without affecting the behavioral or RTL performance.

---

## 결론

**cynw_p2p_vld_only**는 성능과 하드웨어 단순화가 중요하고, 수신자가 항상 준비된 경우에 적합합니다. 그러나 **매 클럭마다 listening 해야 하는 엄격한 요구사항**을 만족해야 합니다.

**cynw_p2p**는 더 안전하고 유연한 선택으로, 복잡한 시스템이나 데이터 무결성이 중요한 경우에 사용해야 합니다.

### 선택 가이드라인

```
데이터 무결성이 최우선인가?
└─ YES → cynw_p2p 사용
└─ NO  → 다음 질문으로

수신자가 항상 준비 상태를 보장할 수 있는가?
└─ YES → cynw_p2p_vld_only 고려 (성능 이점)
└─ NO  → cynw_p2p 사용

하드웨어 단순화와 타이밍이 매우 중요한가?
└─ YES → cynw_p2p_vld_only 고려 (조건 충족 시)
└─ NO  → cynw_p2p 사용 (안전한 선택)
```

---

**출처**: Stratus High-Level Synthesis User Guide (December 2024, Product Version 24.02)
**페이지**: 1223-1249 (Synthesizable Interface Libraries)
