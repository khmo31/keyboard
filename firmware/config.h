/*
 * config.h — 메이플스토리 사냥 매크로 키보드 설정
 * RP2040 Pico + PIO-USB Host
 */

#pragma once

/*────────────────────────────────────────────────────────────
 * USB Device Descriptor (PC에 인식될 정보)
 *────────────────────────────────────────────────────────────*/
#define VENDOR_ID       0x046D   // Logitech
#define PRODUCT_ID      0xC33C   // G 시리즈
#define DEVICE_VER      0x0001
#define MANUFACTURER    "Logitech"
#define PRODUCT         "USB Keyboard"

/*────────────────────────────────────────────────────────────
 * PIO-USB Host 설정
 *────────────────────────────────────────────────────────────*/
#define PIO_USB_HOST_ENABLE
#define PIO_USB_DP_GPIO   0       // GP0 = USB D+
#define PIO_USB_DM_GPIO   1       // GP1 = USB D-

/* 시스템 클럭 (PIO-USB 안정 동작을 위해 120MHz 필수) */
#define PICO_SYSTEM_CLOCK_KHZ 120000

/*────────────────────────────────────────────────────────────
 * 키 매트릭스 (택트 스위치 1개)
 *────────────────────────────────────────────────────────────*/
#define MATRIX_ROWS 1
#define MATRIX_COLS 1

#define MATRIX_ROW_PINS { GP20 }
#define MATRIX_COL_PINS { GP21 }

#define DIODE_DIRECTION COL2ROW

/*────────────────────────────────────────────────────────────
 * USB 설정
 *────────────────────────────────────────────────────────────*/
#define USB_POLLING_INTERVAL_MS 1   // 1000Hz 폴링
#define FORCE_NKRO                    // N-Key Rollover 강제

/*────────────────────────────────────────────────────────────
 * 매크로 타이밍
 *────────────────────────────────────────────────────────────*/
#define DEBOUNCE 5

/*────────────────────────────────────────────────────────────
 * 기능 활성화
 *────────────────────────────────────────────────────────────*/
#define LOCKING_SUPPORT_ENABLE
#define LOCKING_RESYNC_ENABLE

/*────────────────────────────────────────────────────────────
 * 사이즈 최적화 (불필요 기능 비활성화)
 *────────────────────────────────────────────────────────────*/
#define NO_ACTION_MACRO
#define NO_ACTION_FUNCTION
