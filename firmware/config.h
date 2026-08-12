// ============================================================================
// QMK config.h — Pico Hunting Macro for MapleStory
// Target: RP2040 (Raspberry Pi Pico 호환 보드)
// ============================================================================

#pragma once

// ----- VID / PID — Logitech 키보드로 스푸핑 --------------------------------
#define VENDOR_ID       0x046D
#define PRODUCT_ID      0xC33C
#define MANUFACTURER    "Logitech"
#define PRODUCT         "USB Keyboard"

// ----- MCU 설정 -------------------------------------------------------------
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_TIMEOUT 200U
#define RP2040_BOOTLOADER_DOUBLE_TAP_RESET_LED GP25

// ----- USB 설정 -------------------------------------------------------------
#define USB_POLLING_INTERVAL_MS 1    // 1ms = 1000Hz 폴링
#define USB_MAX_POWER_CONSUMPTION 100

// ----- 키보드 매트릭스 ------------------------------------------------------
// 택트 스위치 1개 → GP20
#define MATRIX_ROWS 1
#define MATRIX_COLS 1

#define MATRIX_ROW_PINS { GP20 }
// 컬럼 핀은 사용하지 않지만 QMK가 요구하므로 더미 핀 지정
#define MATRIX_COL_PINS { GP21 }

#define DIODE_DIRECTION COL2ROW

// ----- 디버깅 ---------------------------------------------------------------
// 초기 개발 중에는 활성화, 실사용 시 비활성화
// #define NO_DEBUG
// #define NO_PRINT

// ----- 기능 활성화 ----------------------------------------------------------
// 보조 MCU 기능 (USB Host Shield SPI 통신용) — Core 0에서 실행되도록 설정
#define SPLIT_KEYBOARD
