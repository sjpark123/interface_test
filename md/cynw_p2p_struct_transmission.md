# cynw_p2p 인터페이스를 통한 struct 데이터 전송

## 개요

PDF 문서 검색 결과, **struct 타입의 데이터는 cynw_p2p 인터페이스를 통해 전송이 가능합니다.**

## 주요 요구사항

struct 또는 class 타입을 cynw_p2p<T> 채널에서 사용하려면 다음 조건을 만족해야 합니다:

1. **operator==()** 멤버 함수 필요
2. **operator=()** 멤버 함수 필요
3. **sc_trace()** 함수 오버로드 필요
4. **operator<<()** 오버로드 필요
5. **기본 생성자(default constructor)** 필요

이러한 요구사항은 SystemC가 sc_signal<T>에 대해 요구하는 사항이며, cynw_p2p<T>의 pin-level 버전이 sc_signal<T>를 사용하기 때문입니다.

## 완전한 예제

검색 결과에서 제공된 예제를 바탕으로 완전한 구현 예제입니다:

```cpp
#include "cynw_p2p.h"
#include <systemc.h>

// struct 정의 - cynw_p2p 전송을 위한 모든 요구사항 포함
struct mydata {
    sc_uint<8> a;
    sc_uint<16> b;

    // 기본 생성자
    mydata() {}

    // 매개변수 생성자
    mydata(sc_uint<8> ain, sc_uint<16> bin) : a(ain), b(bin) {}

    // 비교 연산자
    bool operator==(const mydata& other) {
        return ((a == other.a) && (b == other.b));
    }

    // 할당 연산자
    mydata& operator=(const mydata& other) {
        a = other.a;
        b = other.b;
        return *this;
    }
};

// sc_trace 함수 오버로드
inline void sc_trace(sc_trace_file *tf, const mydata& object, const sc_string& name) {
    if (tf) {
        tf->trace(object.a, name + sc_string(".a"));
        tf->trace(object.b, name + sc_string(".b"));
    }
}

// 스트림 출력 연산자 오버로드
inline ostream& operator<<(ostream& os, const mydata& a) {
#ifndef stratus_hls
    os << " (" << a.a.to_string().c_str() << ","
       << a.b.to_string().c_str() << "))";
#endif
    return os;
}

// cynw_p2p를 사용하는 모듈 예제
SC_MODULE(dut) {
    // cynw_p2p 채널 및 메타포트 선언
    cynw_p2p<mydata>::in din;
    cynw_p2p<mydata>::out dout;

    // 클럭 및 리셋 포트
    sc_in_clk clk;
    sc_in<bool> rst;

    SC_CTOR(dut) : din("din"), dout("dout"), clk("clk"), rst("rst") {
        SC_CTHREAD(process, clk.pos());
        reset_signal_is(rst, 0);

        // 클럭과 리셋 신호를 메타포트에 연결
        din.clk_rst(clk, rst);
        dout.clk_rst(clk, rst);
    }

    void process() {
        // 인터페이스 리셋
        HLS_DEFINE_PROTOCOL("reset");
        din.reset();
        dout.reset();
        wait();

        // 메인 실행 루프
        while (1) {
            // struct 데이터 수신
            mydata in_val = din.get();

            // 데이터 처리
            mydata out_val;
            out_val.a = in_val.a + 1;
            out_val.b = in_val.b * 2;

            // struct 데이터 송신
            dout.put(out_val);
        }
    }
};
```

## RTL 생성 시 동작

Stratus HLS가 RTL 모델을 생성할 때, struct의 각 멤버에 대해 개별 포트가 생성됩니다.

위 예제의 경우:
- `din` 입력에는 8비트 포트(a)와 16비트 포트(b)가 생성됨
- `dout` 출력에도 동일하게 8비트 포트(a)와 16비트 포트(b)가 생성됨

## 추가 최적화 옵션

II=1 파이프라인 루프에서 모든 get() 호출이 발생하는 경우, 비동기 경로를 제거할 수 있습니다:

```cpp
cynw_p2p<mydata>::in inp("inp", CYNW_II1_OPTIM);
```

이 플래그는 cynw_p2p<>::in 포트의 생성자 두 번째 매개변수로 지정하며, vid 입력과 busy 출력 사이의 비동기 경로를 안전하게 제거합니다.

## cynw_p2p 프로토콜 규칙

### Writer(송신 측) 규칙:
- 새로운 데이터 값을 쓸 때마다 vid를 assert해야 함
- busy가 low인 클럭 에지를 볼 때까지 vld와 data를 계속 assert해야 함
- busy가 deassert된 후, writer는 vid를 deassert하거나 새로운 데이터 값을 assert해야 함
- busy가 assert된 상태에서 vld를 assert할 수 있지만, 이전 값이 !busy로 확인된 경우에만 가능

## struct/class 타입 사용 시 특별 고려사항

struct 또는 class 타입은 다음과 같은 경우에 특별한 지원 함수가 필요합니다:

- sc_in 또는 sc_out 포트, 또는 sc_signal에서 struct/class 값을 사용할 때
- cynw_p2p를 사용하여 struct/class 값을 쓸 때
- 명시적 메모리에 struct/class 값을 저장할 때
- Cynware Generated Interface를 통해 struct/class 값을 전송할 때
- 위의 인터페이스를 통해 데이터 배열을 전송할 때

## 결론

struct 타입 데이터는 cynw_p2p 인터페이스를 통해 전송 가능하며, 위의 5가지 요구사항(연산자 오버로드 및 기본 생성자)만 충족하면 일반 데이터 타입처럼 사용할 수 있습니다. RTL 합성 시 struct의 각 멤버가 개별 포트로 매핑됩니다.

---

**참고 문서**: Stratus High-Level Synthesis User Guide (December 2024, Product Version 24.02)
