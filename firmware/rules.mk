# rules.mk — RP2040 Pico + PIO-USB Host 빌드 규칙

# MCU
MCU_FAMILY = PICO
MCU_SERIES = RP2040
MCU = cortex-m0plus

# Pico-PIO-USB 라이브러리
SRC += pio_usb.c usb_crc.c

# CFLAGS: PIO-USB 헤더 경로
CFLAGS += -Ilib/pico-pio-usb

# 기능 활성화
NKRO_ENABLE = yes           # N-Key Rollover
EXTRAKEY_ENABLE = no        # 미디어 키 불필요
CONSOLE_ENABLE = yes        # 디버깅 콘솔
COMMAND_ENABLE = no         # 매직 커맨드 불필요
MOUSEKEY_ENABLE = no        # 마우스 키 불필요
BOOTMAGIC_ENABLE = yes      # 부트매직 Lite

# PIO-USB Host 사용 시 필요한 TinyUSB 설정
TINYUSB_HOST_ENABLED = yes
