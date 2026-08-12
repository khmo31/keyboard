# QMK 빌드 규칙 — Pico Hunting Macro

# MCU
MCU = RP2040
BOOTLOADER = rp2040

# 기능 활성화
BOOTMAGIC_ENABLE = yes      # 부트매직 (초기화 등)
MOUSEKEY_ENABLE = no        # 마우스 키 불필요
EXTRAKEY_ENABLE = no        # 미디어 키 불필요
CONSOLE_ENABLE = yes        # 디버그 콘솔 (개발 중)
COMMAND_ENABLE = no
NKRO_ENABLE = no            # N-key rollover 불필요
AUDIO_ENABLE = no
RGBLIGHT_ENABLE = no
CUSTOM_MATRIX = yes         # 커스텀 매트릭스 스캔 사용

# RP2040 특화
PICO_SDK_PATH = $(HOME)/pico-sdk
