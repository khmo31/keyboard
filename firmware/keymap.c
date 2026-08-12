/**
 * QMK keymap.c — MapleStory Hunting Macro
 *
 * 패턴: Pico + MAX3421E USB Host Shield → 단일 USB HID 엔드포인트
 *
 * 동작:
 *   - 택트 스위치 (GP20): 사냥 매크로 ON/OFF 토글
 *   - 실제 키보드 입력: USB Host Shield 거쳐서 Pass-through
 *   - 매크로: 좌우 이동 + 점프 + 공격 + 줍기 루틴
 */

#include QMK_KEYBOARD_H
#include <stdlib.h>

// ============================================================================
// CONFIGURATION — 메이플스토리 인게임 키 설정에 맞춰 변경
// ============================================================================
#define KEY_JUMP    KC_LALT     // 점프
#define KEY_ATTACK  KC_X        // 주력 공격 스킬
#define KEY_LOOT    KC_Z        // 줍기 / 상호작용
#define KEY_SKILL2  KC_C        // 보조 스킬 (광역기 / 버프)

// ---- 타이밍 (단위: ms) ----------------------------------------------------
#define MOVE_INTERVAL_MIN   400     // 이동 후 대기 (최소)
#define MOVE_INTERVAL_MAX   900     // 이동 후 대기 (최대)
#define ATTACK_DELAY_MIN    50      // 공격 키 누름 시간 (최소)
#define ATTACK_DELAY_MAX    180     // 공격 키 누름 시간 (최대)
#define SKILL2_COOLDOWN     5000    // 보조 스킬 쿨다운 (ms)
#define LOOT_DELAY_MIN      200     // 줍기 대기 (최소)
#define LOOT_DELAY_MAX      1500    // 줍기 대기 (최대)
#define IDLE_CHECK_MS       600000  // 10분 — 유휴 행동 간격
#define IDLE_ACTION_CHANCE  30      // 유휴 행동 확률 (%)

// ============================================================================
// STATE
// ============================================================================
static bool     hunting         = false;
static uint32_t next_action     = 0;
static uint32_t last_skill2     = 0;
static uint32_t last_idle_check = 0;
static uint8_t  hunt_phase      = 0;   // 0:좌측이동, 1:공격, 2:우측이동, 3:줍기
static uint8_t  hunt_variant    = 0;   // 0~2

// ============================================================================
// UTILITY
// ============================================================================
static uint32_t random_range(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    return min + (rand() % (max - min));
}

// ============================================================================
// KEYMAP
// ============================================================================
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [0] = LAYOUT(
        KC_F3  // 택트 스위치 = 토글 키. 실제 F3 전송은 process_record_user에서 가로챔
    )
};

// ============================================================================
// CUSTOM KEYCODES
// ============================================================================
enum custom_keycodes {
    HUNT_TOGGLE = SAFE_RANGE,
    HUNT_VARIANT,
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case HUNT_TOGGLE:
            if (record->event.pressed) {
                hunting = !hunting;
                if (hunting) {
                    hunt_phase = 0;
                    next_action = timer_read32();
                }
            }
            return false; // F3 이벤트를 PC로 전송하지 않음

        case HUNT_VARIANT:
            if (record->event.pressed) {
                hunt_variant = (hunt_variant + 1) % 3;
            }
            return false;
    }
    return true;
}

// ============================================================================
// MACRO ENGINE — QMK matrix scan cycle마다 호출
// ============================================================================
void matrix_scan_user(void) {
    if (!hunting) return;

    uint32_t now = timer_read32();
    if (now < next_action) return;

    // ---- IDLE CHECK: 10분마다 30% 확률로 채팅 노이즈 ----
    if (now - last_idle_check > IDLE_CHECK_MS) {
        last_idle_check = now;
        if ((rand() % 100) < IDLE_ACTION_CHANCE) {
            tap_code(KC_ENTER);
            wait_ms(random_range(50, 150));
            tap_code16(random_range(KC_A, KC_Z));  // 랜덤 영문자
            wait_ms(random_range(50, 150));
            tap_code(KC_ENTER);
            next_action = now + random_range(2000, 5000);
            return;
        }
    }

    // ---- SECONDARY SKILL (쿨다운 기반) ----
    if (now - last_skill2 > SKILL2_COOLDOWN) {
        last_skill2 = now;
        tap_code(KEY_SKILL2);
        wait_ms(random_range(ATTACK_DELAY_MIN, ATTACK_DELAY_MAX));
    }

    // ---- MAIN HUNTING LOOP ----
    switch (hunt_phase) {
        case 0: // LEFT 이동 + 점프
            register_code(KC_LEFT);
            register_code(KEY_JUMP);
            wait_ms(random_range(30, 80));
            unregister_code(KC_LEFT);
            unregister_code(KEY_JUMP);
            next_action = now + random_range(MOVE_INTERVAL_MIN, MOVE_INTERVAL_MAX);
            hunt_phase = 1;
            break;

        case 1: // ATTACK
            tap_code(KEY_ATTACK);
            wait_ms(random_range(ATTACK_DELAY_MIN, ATTACK_DELAY_MAX));

            if (hunt_variant == 0) {
                // 단일 공격 후 우측 이동
                next_action = now + random_range(MOVE_INTERVAL_MIN / 2, MOVE_INTERVAL_MAX / 2);
                hunt_phase = 2;
            } else if (hunt_variant == 1) {
                // 이중 공격
                tap_code(KEY_ATTACK);
                wait_ms(random_range(ATTACK_DELAY_MIN, ATTACK_DELAY_MAX));
                next_action = now + random_range(MOVE_INTERVAL_MIN / 2, MOVE_INTERVAL_MAX / 2);
                hunt_phase = 2;
            } else {
                // 공격 → 줍기
                next_action = now + random_range(LOOT_DELAY_MIN, LOOT_DELAY_MAX);
                hunt_phase = 3;
            }
            break;

        case 2: // RIGHT 이동 + 점프
            register_code(KC_RIGHT);
            register_code(KEY_JUMP);
            wait_ms(random_range(30, 80));
            unregister_code(KC_RIGHT);
            unregister_code(KEY_JUMP);
            next_action = now + random_range(MOVE_INTERVAL_MIN, MOVE_INTERVAL_MAX);
            hunt_phase = 1;
            break;

        case 3: // LOOT
            tap_code(KEY_LOOT);
            next_action = now + random_range(MOVE_INTERVAL_MIN / 2, MOVE_INTERVAL_MAX / 2);
            hunt_phase = 2;
            break;
    }
}

// ============================================================================
// USB HOST SHIELD INTEGRATION (Phase 1에서는 미구현)
// ============================================================================
//
// MAX3421E에서 읽은 실제 키보드 HID 리포트를 QMK 파이프라인에 주입:
//
// void forward_host_key(uint8_t hid_code, bool pressed) {
//     keyevent_t event = {
//         .key     = {.row = 0, .col = hid_code},
//         .pressed = pressed,
//         .time    = timer_read(),
//         .type    = KEY_EVENT,
//     };
//     action_exec(event);
// }
//
// 이 구조를 사용하면:
//   - 실제 키보드 입력 → USB Host Shield → SPI → QMK → PC
//   - 매크로 입력 → QMK → PC
//   둘 다 동일한 USB HID 엔드포인트로 전송 → 안티치트가 분리 불가
