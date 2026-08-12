# 메이플스토리 하드웨어 사냥 매크로 — 설계도

> **버전:** 2.0 | **작성일:** 2026-08-12 | **상태:** PIO-USB 기반 재설계 완료
>
> ※ v1.x (MAX3421E USB Host Shield V560) → v2.0 (RP2040 PIO-USB) 전면 재설계

---

## 1. 아키텍처 개요

### 1.1 설계 원칙

- **단일 MCU**: Raspberry Pi Pico (RP2040) 하나로 모든 기능 통합. 외부 칩 불필요
- **단일 USB HID 엔드포인트**: 실제 키보드 입력 + 매크로 입력을 하나의 USB 장치로 통합 → 안티치트가 입력 소스를 구분 불가능
- **하드웨어 레벨**: PC에는 표준 USB 키보드로 인식. 소프트웨어 탐지 불가
- **반자동**: 룬 해제 / 거짓말 탐지기는 사람이 직접 처리. 사냥 루틴만 자동화
- **QMK 펌웨어**: 전 세계 커스텀 키보드 수만 대가 사용하는 검증된 오픈소스 펌웨어
- **PIO-USB**: RP2040의 PIO(Programmed I/O) 하드웨어로 USB Host 구현. 별도 USB 호스트 칩 불필요

### 1.2 전체 구성도

```
┌──────────────────────────────────────────────────────────┐
│                                                           │
│  [실제 키보드]                                             │
│       │ USB                                                │
│       ▼                                                    │
│  ┌─────────────────┐                                      │
│  │ USB A 암커넥터    │                                      │
│  │ (D+/D-/5V/GND)  │                                      │
│  └───────┬─────────┘                                      │
│          │ 점퍼 와이어 4개                                   │
│          ▼                                                 │
│  ┌──────────────────────────────────────────────┐         │
│  │          Raspberry Pi Pico (RP2040)           │         │
│  │                                               │         │
│  │  GP0  (D+)  ──┐                              │         │
│  │  GP1  (D-)  ──┤── PIO Block (USB Host)       │         │
│  │               │   ┌──────────────────┐       │         │
│  │               │   │  Keyboard HID    │       │         │
│  │               │   │  Report Reader   │       │         │
│  │               │   └────────┬─────────┘       │         │
│  │               │            │                  │         │
│  │               │   ┌────────▼─────────┐       │         │
│  │               │   │   Input Merger   │       │         │
│  │               │   │  (Real + Macro)  │       │         │
│  │               │   └────────┬─────────┘       │         │
│  │               │            │                  │         │
│  │               │   ┌────────▼─────────┐       │         │
│  │               │   │  Macro Engine    │       │         │
│  │               │   │  (QMK Firmware)  │       │         │
│  │               │   └────────┬─────────┘       │         │
│  │               │            │                  │         │
│  │  ┌────────────┼────────────┼──────────┐     │         │
│  │  │     Native USB Device            │     │         │
│  │  │     (QMK Keyboard HID)           │     │         │
│  │  └────────────┬─────────────────────┘     │         │
│  │               │                            │         │
│  │  GP20 ← [택트 스위치] (시작/정지)           │         │
│  │               │                            │         │
│  └───────────────┼────────────────────────────┘         │
│                  │ USB (Device)                           │
│                  ▼                                        │
│        ┌─────────────────┐                               │
│        │   게임 PC       │                               │
│        │   (메이플스토리)  │                               │
│        └─────────────────┘                               │
└──────────────────────────────────────────────────────────┘
```

**작동 원리:**
1. RP2040의 **PIO 블록**이 USB A 커넥터(D+/D- GPIO)로 실제 키보드의 HID 리포트를 읽음
2. 매크로 엔진이 사냥 패턴 키 입력을 생성
3. 실제 키보드 입력 + 매크로 입력을 **단일 HID 리포트**로 병합
4. Pico의 **네이티브 USB 포트**(Micro USB)로 PC에 표준 키보드로 출력
5. PC는 하나의 USB 키보드만 인식 → 안티치트가 입력 소스 분리 불가

### 1.3 왜 MAX3421E가 아닌 PIO-USB인가

MAX3421E USB Host Shield V560을 검토했으나 알리익스프레스 클론 보드의 고질적 불량으로 폐기:

- V560 클론은 레벨 시프터 IC로 **74HC125**가 장착됨 (정상: 보라색 위치에 **74AHC125** 필요)
- 잘못된 IC로 인해 3.3V 레일에 과전압, USB 장치 인식 불가
- SMD 칩 교체는 히팅건 + 2주 배송 필요하며 초보자에게 부적합
- 반면 PIO-USB는 **USB A 암커넥터(₩500) + 저항 2개(₩100)** 만 추가 필요
- 부품 수 감소, 배선 간소화, 고장점 감소

---

## 2. 하드웨어 구성

### 2.1 부품 목록

| # | 부품 | 수량 | 구매처 | 가격 | 비고 |
|---|------|------|--------|------|------|
| 1 | RP2040 Pico 호환 보드 (Type-C, 핀헤더 납땜 완료) | 1 | AliExpress | ₩3,820 | 기보유 |
| 2 | USB A 타입 암커넥터 (브레이크아웃, 4핀) | 1 | 국내 쇼핑몰 | ~₩500 | D+/D-/VBUS/GND 단자 분리된 것 |
| 3 | 22Ω 저항 (1/4W) | 2 | 국내 쇼핑몰 | ~₩100 | D+/D- 라인 직렬 저항 |
| 4 | 점퍼 와이어 (암-암, 10cm) | 4 | 기보유 | - | D+, D-, 5V, GND |
| 5 | 택트 스위치 6×6mm | 1 | 기보유 | - | 시작/정지 토글 |
| 6 | 브레드보드 (400홀) | 1 | 기보유 | - | |

**총 추가 구매 비용: ~₩600** (MAX3421E Shield ₩5,000 대비 88% 절감)

### 2.2 핀 연결표

```
USB A 암커넥터                    Raspberry Pi Pico
─────────────────────────────────────────────────
핀 1: VBUS (적색, 5V)     →      VBUS (물리핀 40)
핀 2: D-   (백색)         →      22Ω 저항 → GP1  (물리핀 2)
핀 3: D+   (녹색)         →      22Ω 저항 → GP0  (물리핀 1)
핀 4: GND  (흑색)         →      GND  (물리핀 3)

[택트 스위치]
스위치 다리 1              →      GP20 (물리핀 26)
스위치 다리 2              →      GND  (물리핀 28)
```

**⚠️ D+/D- 라인에는 반드시 22Ω 직렬 저항을 삽입할 것.** USB 신호 무결성을 위한 임피던스 매칭이다. 없어도 동작할 수 있지만, 간헐적 통신 오류의 원인이 된다.

### 2.3 Pico 물리 핀맵

```
         ┌──────────────────────────┐
    VBUS │40 ●                  ● 21│
         │39 ●                  ● 22│
         │   ●                  ●   │
         │   ●                  ●   │
         │   ●                  ●   │
   GP20  │26 ●                  ●   │
         │   ●                  ●   │
    GND  │28 ●                  ●   │
         │   ●                  ●   │
         └──────────────────────┘

         ┌────┐
    GP0  │ 1 ●│ ← D+ (22Ω 저항 통해)
    GP1  │ 2 ●│ ← D- (22Ω 저항 통해)
    GND  │ 3 ●│ ← USB GND
         │   ●│
         │   ●│
         └────┘
```

### 2.4 조립 순서

1. **USB A 커넥터 핀 식별**: USB A 암커넥터는 일반적으로 위에서 봤을 때 왼쪽부터 VBUS(5V, 적색), D-(백색), D+(녹색), GND(흑색)
2. **22Ω 저항 연결**: D+ 와이어와 D- 와이어에 각각 22Ω 저항을 직렬로 연결 (브레드보드 이용)
3. **Pico에 연결**:
   - USB VBUS → Pico VBUS (40번)
   - USB D- → 22Ω → Pico GP1 (2번)
   - USB D+ → 22Ω → Pico GP0 (1번)
   - USB GND → Pico GND (3번)
4. **택트 스위치 연결**: GP20 ↔ GND
5. **Pico를 PC에 연결**: Micro USB 케이블로 PC와 연결 (펌웨어 플래싱 및 동작)

### 2.5 전원 및 전압

| 항목 | 값 | 설명 |
|------|-----|------|
| Pico 공급 전원 | PC USB 5V | Micro USB 케이블로 공급 |
| 키보드 공급 전원 | VBUS에서 5V 분기 | USB A 커넥터 VBUS 핀으로 전달 |
| GPIO 전압 | 3.3V | Pico의 모든 GPIO는 3.3V (5V tolerant 아님) |
| D+/D- 신호 레벨 | 3.3V | USB LS/FS는 3.3V 차동 신호 → GPIO 직결 가능 |
| 전체 소비 전류 | ~150mA 이하 | Pico 30mA + 키보드 100mA (USB 500mA 제한 내) |

---

## 3. 소프트웨어 설계

### 3.1 기술 스택

| 계층 | 기술 | 설명 |
|------|------|------|
| 펌웨어 | **QMK** | 커스텀 키보드 펌웨어 |
| USB Host | **Pico-PIO-USB** (sekigon-gonnoc) | RP2040 PIO로 USB Host 구현 |
| USB Device | RP2040 Native USB (TinyUSB) | PC에 키보드로 인식 |
| 매크로 엔진 | QMK `process_record_user()` + `matrix_scan_user()` | 사냥 패턴 상태 머신 |

### 3.2 QMK 펌웨어 설정 (config.h)

```c
#pragma once

// USB Device descriptor — PC에 표시될 키보드 정보
#define VENDOR_ID       0x046D   // Logitech
#define PRODUCT_ID      0xC33C   // G 시리즈
#define MANUFACTURER    "Logitech"
#define PRODUCT         "USB Keyboard"

// PIO-USB Host 설정
#define PIO_USB_HOST_ENABLE
#define PIO_USB_DP_GPIO   0   // GP0 = D+
#define PIO_USB_DM_GPIO   1   // GP1 = D-

// 시스템 클럭 (PIO-USB 안정 동작 필요)
#define PICO_SYSTEM_CLOCK_KHZ 120000

// 매트릭스 설정 (택트 스위치 1개)
#define MATRIX_ROWS 1
#define MATRIX_COLS 1
#define MATRIX_ROW_PINS { GP20 }
#define MATRIX_COL_PINS { GP21 }
#define DIODE_DIRECTION COL2ROW

// USB 폴링
#define USB_POLLING_INTERVAL_MS 1  // 1000Hz
```

### 3.3 키 바인딩

| 물리 버튼 | 기능 | 동작 |
|-----------|------|------|
| GP20 (택트 스위치) | 사냥 시작/정지 토글 | QMK 내부에서 처리, PC로 전송 안 함 |

**메이플스토리 인게임 키 설정:**

| 동작 | 키코드 | QMK 상수 |
|------|--------|----------|
| 점프 | Alt | `KC_LALT` |
| 주력 공격 | X | `KC_X` |
| 줍기/상호작용 | Z | `KC_Z` |
| 보조 스킬 | C | `KC_C` |

### 3.4 매크로 엔진 상태 머신

```
        [IDLE]
          │ 토글 ON
          ▼
   ┌───────────────────────────────────────────┐
   │             HUNTING LOOP                  │
   │                                           │
   │  Phase 0: 좌측 이동 + 점프                 │
   │     ├─ LEFT + JUMP 동시 입력               │
   │     ├─ 30~80ms 유지 후 릴리즈              │
   │     └─ 400~900ms 대기 → Phase 1            │
   │                                           │
   │  Phase 1: 공격                            │
   │     ├─ ATTACK 키 탭 (50~180ms)             │
   │     ├─ Variant 0: 단일 공격 → Phase 2      │
   │     ├─ Variant 1: 이중 공격 → Phase 2      │
   │     └─ Variant 2: 공격+줍기 → Phase 3      │
   │                                           │
   │  Phase 2: 우측 이동 + 점프                 │
   │     ├─ RIGHT + JUMP 동시 입력              │
   │     ├─ 30~80ms 유지 후 릴리즈              │
   │     └─ 400~900ms 대기 → Phase 1            │
   │                                           │
   │  Phase 3: 줍기                            │
   │     ├─ LOOT 키 탭                         │
   │     └─ 200~1500ms 대기 → Phase 2           │
   │                                           │
   │  [비동기]                                  │
   │   ├─ 보조 스킬: 5초마다 C 키                │
   │   ├─ 유휴 행동: 10분마다 30% 확률           │
   │   └─ Variant 순환: 3종 패턴               │
   └───────────────────────────────────────────┘
```

### 3.5 타이밍 설계

| 파라미터 | 최소값 | 최대값 | 단위 | 근거 |
|----------|--------|--------|------|------|
| 이동 간격 | 400 | 900 | ms | 인간 반응 속도 + 지형 이동 |
| 공격 딜레이 | 50 | 180 | ms | 스킬 애니메이션 + 입력 버퍼 |
| 줍기 딜레이 | 200 | 1500 | ms | "바로 줍기" = 봇 시그니처 |
| 보조 스킬 쿨다운 | 5000 | - | ms | 버프 지속 시간 |
| 채팅 노이즈 간격 | 600000 | - | ms | 10분. 30% 확률 |
| Variant 패턴 | 3 | - | 종류 | 단조 반복 회피 |

---

## 4. 펌웨어 빌드 & 플래싱

### 4.1 개발 환경

```bash
# QMK CLI 설치
pip install qmk

# ARM 툴체인 설치
sudo apt install gcc-arm-none-eabi

# QMK 설정
qmk setup

# Pico-PIO-USB 서브모듈 추가
cd ~/qmk_firmware
git submodule add https://github.com/sekigon-gonnoc/Pico-PIO-USB.git lib/pico-pio-usb
```

### 4.2 펌웨어 컴파일

```bash
qmk compile -kb handwired/pico_hunt -km default
# 출력: handwired_pico_hunt_default.uf2
```

### 4.3 Pico에 플래싱

1. Pico의 BOOTSEL 버튼을 **누른 상태로** USB 케이블을 PC에 연결
2. RPI-RP2 드라이브로 마운트됨
3. `.uf2` 파일을 드래그 앤 드롭
4. Pico 자동 재부팅 → 키보드로 인식 완료

### 4.4 디버깅

```bash
# QMK 콘솔 (키 입력 로그)
qmk console

# HID 디바이스 확인
lsusb | grep Logitech

# PIO USB 상태 확인
qmk console -d "pio_usb"
```

---

## 5. 안티치트 회피 전략

### 5.1 기술적 방어

| 탐지 벡터 | 대응 | 구현 방식 |
|-----------|------|----------|
| **USB HID 소스 분리** | 단일 HID 엔드포인트 | Pico가 모든 입력을 하나의 USB Device로 통합 |
| **VID/PID 시그니처** | 스푸핑 | config.h에서 Logitech 키보드로 위장 |
| **USB 패턴 이상** | QMK 표준 스택 | 커스텀 USB 드라이버 사용 안 함 |
| **프로세스 탐지** | 해당 없음 | PC에 소프트웨어 설치 안 함 |
| **메모리 변조** | 해당 없음 | 게임 메모리 접근 안 함 |

### 5.2 행동적 방어

| 탐지 벡터 | 대응 | 구현 위치 |
|-----------|------|----------|
| 타이밍 일정 | 모든 딜레이 랜덤화 | `matrix_scan_user()` |
| 패턴 반복 | 3종 Variant 순환 | `hunt_variant` 상태 |
| 장시간 연속 | 8~10시간 제한 권장 | 사용자 수동 관리 |
| 채팅 없음 | 무작위 키 노이즈 | `IDLE_CHECK` 루틴 |
| 전리품 즉시 획득 | 0.2~1.5초 지연 | Phase 3 랜덤 딜레이 |

### 5.3 NGS 특이사항 (메이플스토리)

- 룬(↑↓←→) / 거짓말 탐지기(숫자) → **사람이 직접 해제**
- 엘리트 몬스터 → 자동 전투로 처리 가능
- NGS는 `WM_INPUT`의 `hDevice` 핸들로 디바이스 추적 → 단일 HID로 우회

---

## 6. 테스트 계획

### 6.1 1단계: 하드웨어 검증 (30분)

- [ ] USB A 커넥터 핀 배열 확인 (멀티미터 통전)
- [ ] Pico USB 연결 → PC에서 키보드로 인식 확인 (`lsusb`)
- [ ] USB A 포트에 실제 키보드 연결 → PIO USB Host 인식 확인
- [ ] 택트 스위치 눌러서 토글 신호 확인 (`qmk console`)

### 6.2 2단계: 키 입력 검증 (1시간)

- [ ] 메모장에서 실제 키보드 타이핑 → Pico 거쳐 정상 출력 확인
- [ ] 택트 스위치 토글 → 매크로 ON/OFF 확인
- [ ] 매크로 ON → LEFT/RIGHT/JUMP/ATTACK 시퀀스 확인
- [ ] 매크로 ON + 동시 키보드 입력 → 충돌 없이 전달 확인

### 6.3 3단계: 게임 내 검증 (2~3시간)

- [ ] 무고밸류 계정에서 사냥 시작
- [ ] 1시간 연속 사냥 → 룬/거탐 출현 시 수동 해제
- [ ] NGS 경고 발생 여부 모니터링
- [ ] Variant 3종 전환 테스트

### 6.4 4단계: 장기 검증 (1주)

- [ ] 하루 4~6시간 제한 운영
- [ ] NGS 경고 로그 수집 여부 확인
- [ ] 패턴 미세 조정

---

## 7. 프로젝트 파일 구조

```
~/keyboard/
├── README.md              # 이 설계도
├── firmware/
│   ├── config.h           # QMK 키보드 + PIO-USB 설정
│   ├── keymap.c           # 키 매핑 + 매크로 엔진 + PIO 호스트 처리
│   └── rules.mk           # QMK 빌드 규칙 (Pico-PIO-USB 포함)
├── hardware/
│   └── schematic.md       # 핀 연결 상세도
└── docs/
    └── testing_log.md     # 테스트 결과 기록
```

---

## 8. 향후 확장 (Phase 2 — 비전 기반 완전 자동)

```
[USB 카메라 / HDMI 캡처카드] → [Raspberry Pi 4 (OpenCV)]
                                        │ UART/Serial
                                        ▼
                                 [Pico (HID)] → [게임 PC]
```

- OpenCV로 화면 인식 → 룬/거탐 자동 해제
- Pi ↔ Pico UART 통신으로 명령 전달
- HP/MP 바, 미니맵 읽기 → 맵 이동 자동화

---

## 부록 A. 문제해결

| 문제 | 원인 가능성 | 해결 |
|------|-----------|------|
| PC가 Pico를 인식 못 함 | USB 케이블이 전원 전용 | 데이터 전송 가능 케이블로 교체 |
| PIO USB Host가 키보드를 인식 못 함 | D+/D- 배선 반전 | GP0 ↔ GP1 핀 스왑 |
| 키보드 입력이 간헐적으로 끊김 | 22Ω 저항 누락 | D+/D- 라인에 22Ω 직렬 저항 추가 |
| 키보드 전원 안 들어옴 | VBUS 미연결 | USB A 커넥터 VBUS → Pico VBUS 확인 |
| 매크로 ON인데 입력 없음 | `matrix_scan_user()` 미호출 | QMK 설정 확인 |
| NGS 경고 발생 | 타이밍 패턴 반복 탐지 | 딜레이 범위 2배로 확장 |
| PIO 초기화 실패 | 클럭 설정 불일치 | `PICO_SYSTEM_CLOCK_KHZ 120000` 확인 |
| USB 3.0 키보드 미인식 | LS/FS만 지원 | USB 2.0 허브를 중간에 연결 (허브가 HS→FS 변환) |

## 부록 B. 참고 자료

| 리소스 | URL |
|--------|-----|
| Pico-PIO-USB | https://github.com/sekigon-gonnoc/Pico-PIO-USB |
| QMK 공식 | https://docs.qmk.fm |
| RP2040 데이터시트 | https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf |
| Pico 핀맵 | https://pico.pinout.xyz |
| QMK Pico-PIO-USB 예제 | https://github.com/sekigon-gonnoc/qmk_firmware/tree/rp2040/keyboards/pico_pico_usb |
