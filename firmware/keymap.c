/*
 * keymap.c — 메이플스토리 사냥 매크로
 * RP2040 Pico + PIO-USB Host + QMK
 *
 * 아키텍처:
 *   1. PIO USB Host가 GPIO D+/D-로 실제 키보드의 HID 리포트를 읽음
 *   2. 읽은 키 입력을 QMK 이벤트로 변환하여 내부 큐에 저장
 *   3. 매크로 엔진이 사냥 패턴을 생성
 *   4. 실제 키보드 입력 + 매크로 입력을 단일 HID 리포트로 PC에 전송
 */

#include QMK_KEYBOARD_H
#include "pio_usb.h"

/*────────────────────────────────────────────────────────────
 * 커스텀 키코드
 *────────────────────────────────────────────────────────────*/
enum custom_keycodes {
    HUNT_TOGGLE = SAFE_RANGE,   // 사냥 시작/정지 토글
};

/*────────────────────────────────────────────────────────────
 * 매크로 엔진 상태
 *────────────────────────────────────────────────────────────*/
typedef enum {
    PHASE_IDLE,
    PHASE_MOVE_LEFT,
    PHASE_ATTACK,
    PHASE_MOVE_RIGHT,
    PHASE_LOOT,
} hunt_phase_t;

static bool        hunting        = false;     // 매크로 ON/OFF
static hunt_phase_t hunt_phase    = PHASE_IDLE;
static uint8_t     hunt_variant   = 0;         // 0~2 순환
static uint32_t    phase_timer    = 0;
static uint32_t    skill_timer    = 0;
static uint32_t    idle_timer     = 0;

/* 타이밍 파라미터 (ms) */
#define MOVE_HOLD_MIN      30
#define MOVE_HOLD_MAX      80
#define MOVE_WAIT_MIN      400
#define MOVE_WAIT_MAX      900
#define ATTACK_HOLD_MIN    50
#define ATTACK_HOLD_MAX    180
#define LOOT_DELAY_MIN     200
#define LOOT_DELAY_MAX     1500
#define SKILL_COOLDOWN     5000
#define IDLE_NOISE_INTERVAL 600000  // 10분
#define IDLE_NOISE_CHANCE   30      // 30%

/*────────────────────────────────────────────────────────────
 * 유틸리티: 범위 기반 랜덤
 *────────────────────────────────────────────────────────────*/
static uint32_t rand_range(uint32_t min, uint32_t max) {
    return min + (rand() % (max - min + 1));
}

/*────────────────────────────────────────────────────────────
 * 키보드 초기화
 *────────────────────────────────────────────────────────────*/
void keyboard_post_init_user(void) {
    // PIO USB Host 초기화
    pio_usb_host_init();

    // 시드 초기화 (타이머 기반)
    srand(timer_read32());
}

/*────────────────────────────────────────────────────────────
 * 메인 루프 — 매트릭스 스캔마다 호출
 *────────────────────────────────────────────────────────────*/
void matrix_scan_user(void) {
    uint32_t now = timer_read32();

    /* ── PIO USB Host: 연결된 키보드의 HID 리포트 읽기 ── */
    pio_usb_host_task();

    /* ── 매크로 엔진 ── */
    if (!hunting) {
        hunt_phase = PHASE_IDLE;
        return;
    }

    switch (hunt_phase) {
    /*───────────────────────────────────────────
     * Phase 0: 좌측 이동 + 점프
     *───────────────────────────────────────────*/
    case PHASE_MOVE_LEFT:
        if (timer_expired32(now, phase_timer)) {
            uint32_t hold = rand_range(MOVE_HOLD_MIN, MOVE_HOLD_MAX);
            register_code(KC_LEFT);
            register_code(KC_LALT);
            phase_timer = timer_read32() + hold;
            hunt_phase = PHASE_ATTACK;
        }
        break;

    /*───────────────────────────────────────────
     * Phase 1: 공격 (3종 Variant)
     *───────────────────────────────────────────*/
    case PHASE_ATTACK:
        if (timer_expired32(now, phase_timer)) {
            // 이전 페이즈 키 릴리즈
            unregister_code(KC_LEFT);
            unregister_code(KC_LALT);
            unregister_code(KC_RIGHT);

            uint32_t hold = rand_range(ATTACK_HOLD_MIN, ATTACK_HOLD_MAX);

            switch (hunt_variant) {
            case 0: // 단일 공격 → Phase 2
                register_code(KC_X);
                phase_timer = timer_read32() + hold;
                hunt_phase = PHASE_MOVE_RIGHT;
                hunt_variant = 1;
                break;
            case 1: // 이중 공격 → Phase 2
                tap_code(KC_X);
                wait_ms(rand_range(30, 60));
                register_code(KC_X);
                phase_timer = timer_read32() + hold;
                hunt_phase = PHASE_MOVE_RIGHT;
                hunt_variant = 2;
                break;
            case 2: // 공격 + 줍기 → Phase 3
                register_code(KC_X);
                phase_timer = timer_read32() + hold;
                hunt_phase = PHASE_LOOT;
                hunt_variant = 0;
                break;
            }
        }
        break;

    /*───────────────────────────────────────────
     * Phase 2: 우측 이동 + 점프
     *───────────────────────────────────────────*/
    case PHASE_MOVE_RIGHT:
        if (timer_expired32(now, phase_timer)) {
            unregister_code(KC_X);

            uint32_t hold = rand_range(MOVE_HOLD_MIN, MOVE_HOLD_MAX);
            register_code(KC_RIGHT);
            register_code(KC_LALT);
            phase_timer = timer_read32() + hold;
            hunt_phase = PHASE_ATTACK;
        }
        break;

    /*───────────────────────────────────────────
     * Phase 3: 줍기
     *───────────────────────────────────────────*/
    case PHASE_LOOT:
        if (timer_expired32(now, phase_timer)) {
            unregister_code(KC_X);

            tap_code(KC_Z);
            phase_timer = timer_read32() + rand_range(LOOT_DELAY_MIN, LOOT_DELAY_MAX);
            hunt_phase = PHASE_MOVE_RIGHT;
        }
        break;

    default:
        break;
    }

    /* ── 비동기: 보조 스킬 (5초 쿨다운) ── */
    if (timer_expired32(now, skill_timer)) {
        tap_code(KC_C);
        skill_timer = timer_read32() + SKILL_COOLDOWN;
    }

    /* ── 비동기: 채팅 노이즈 (10분마다 30%) ── */
    if (timer_expired32(now, idle_timer)) {
        if ((rand() % 100) < IDLE_NOISE_CHANCE) {
            tap_code(KC_ENTER);
            wait_ms(rand_range(100, 300));
            tap_code(KC_ENTER);
        }
        idle_timer = timer_read32() + IDLE_NOISE_INTERVAL;
    }
}

/*────────────────────────────────────────────────────────────
 * 키 입력 처리 — 택트 스위치 토글
 *────────────────────────────────────────────────────────────*/
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case HUNT_TOGGLE:
        if (record->event.pressed) {
            hunting = !hunting;
            if (hunting) {
                // 매크로 시작: Phase 0로 진입
                hunt_phase   = PHASE_MOVE_LEFT;
                hunt_variant = 0;
                phase_timer  = timer_read32() + rand_range(100, 500);
                skill_timer  = timer_read32() + SKILL_COOLDOWN;
                idle_timer   = timer_read32() + IDLE_NOISE_INTERVAL;
            } else {
                // 매크로 정지: 모든 키 릴리즈
                unregister_code(KC_LEFT);
                unregister_code(KC_RIGHT);
                unregister_code(KC_LALT);
                unregister_code(KC_X);
                unregister_code(KC_Z);
                unregister_code(KC_C);
                hunt_phase = PHASE_IDLE;
            }
        }
        return false; // PC로 전송 안 함 (내부 처리)
    }

    return true; // 다른 키는 PC로 전송
}

/*────────────────────────────────────────────────────────────
 * 키 레이어 정의
 *────────────────────────────────────────────────────────────*/
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        HUNT_TOGGLE   // GP20 — 택트 스위치
    ),
};
