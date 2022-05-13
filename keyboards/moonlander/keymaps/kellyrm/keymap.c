/* Copyright 2020 ZSA Technology Labs, Inc <@zsa>
 * Copyright 2020 Jack Humbert <jack.humb@gmail.com>
 * Copyright 2020 Christopher Courtney, aka Drashna Jael're  (@drashna) <drashna@live.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "version.h"
#include "rgb_matrix/rgb_matrix.h"
#include <print.h>
#include "timer.h"
#include "pointing_device.h"

#define MOUSE_PERIOD 10

#define COLOR_BASE  HSV_MAGENTA
#define COLOR_SH    HSV_RED
#define COLOR_ALTS  HSV_YELLOW
#define COLOR_ALT   HSV_GREEN
#define COLOR_LYR   HSV_SPRINGGREEN
#define COLOR_CTL   HSV_CYAN
#define COLOR_CTLS  HSV_PURPLE
#define COLOR_RPT   HSV_BLUE
#define COLOR_MOUSE HSV_CHARTREUSE
#define COLOR_MLTPL HSV_WHITE

enum layers {
    BASE,  // default layer
    UTIL,  // util keys
    SYMB,  // symbols
};

enum custom_keycodes {
    VRSN = ML_SAFE_RANGE,
    NUM_1,
    NUM_2,
    NUM_4,
    NUM_8,
    MOD_LOCK,
    MOD_UNLK,
    HEX_PFX,
    PT_LF,
    PT_RT,
    PT_UP,
    PT_DN,
    NXT_FN,
};

enum layer_mode {
    LYR_NONE,
    LYR_HELD_0,
    LYR_HELD_1,
    LYR_STICKY,
    LYR_LOCK,
};

uint8_t pressed_nums = 0;
uint8_t entered_num  = 0;

uint16_t mods_tap = 0;
uint16_t mods_lock = 0;
uint16_t mods_held = 0;

uint8_t cur_layer = BASE;
enum layer_mode layer_mode = LYR_NONE;

uint16_t last_key = 0;
uint16_t lock_key = 0;
uint8_t lock_key_rpt = 0;
uint32_t repeat_future = 0;

uint16_t pt_dir = 0;

uint8_t next_function = 0;

void num_pressed(uint8_t num);
void num_released(uint8_t num);

void mod_pressed(uint16_t mod);
void mod_released(uint16_t mod);

void key_pressed(uint16_t code);
void key_released(uint16_t code);

void layer_pressed(uint8_t layer);

void clear(void);
void unlock(void);
void set_color(void);

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE] = LAYOUT_moonlander(
        // dollar ampersand    bracket    brace          brace     paren     equal      asterisk    paren     percent    plus      bracket exclamation   pound
        S(KC_4),  S(KC_7),   KC_LBRC,   S(KC_LBRC),  S(KC_RBRC),  S(KC_9),  KC_EQL,      S(KC_8),  S(KC_0),  KC_PERC,   S(KC_EQL),  KC_RBRC,  S(KC_1),  S(KC_3),
        KC_TAB,   KC_SCLN,   KC_COMMA,  KC_DOT,      KC_P,        KC_Y,     KC_1,        KC_0,     KC_F,     KC_G,      KC_C,       KC_R,     KC_L,     KC_BSLS,
        KC_ESC,   KC_A,      KC_O,      KC_E,        KC_U,        KC_I,     LSFT(KC_6),  KC_BSPC,  KC_D,     KC_H,      KC_T,       KC_N,     KC_S,     KC_MINUS,
        KC_GRV,   KC_QUOTE,  KC_Q,      KC_J,        KC_K,        KC_X,                            KC_B,     KC_M,      KC_W,       KC_V,     KC_Z,     KC_SLASH,
        KC_AT,    KC_NO,     KC_NO,     KC_NO,       OSL(UTIL),             MOD_UNLK,    MOD_LOCK,           KC_LALT,   KC_BTN1,    KC_BTN2,  KC_INS,   KC_DEL,
                                                     KC_LSFT,     KC_ENT,   KC_NO,       KC_LSFT,  KC_LCTL,  KC_SPC
    ),

    [UTIL] = LAYOUT_moonlander(
        LED_LEVEL, _______,  _______,  _______, _______,    _______, _______,           _______, _______, KC_BTN1,  KC_BTN2, _______, _______,  RGB_TOG,
        _______,   KC_SCLN,  KC_COMMA, KC_DOT,  S(KC_SCLN), _______, _______,           _______, _______, _______,  _______, _______, _______,  _______,
        NXT_FN,    NUM_8,    NUM_4,    NUM_2,   NUM_1,      KC_0,    HEX_PFX,           _______, _______, KC_LEFT,  KC_DOWN, KC_UP,   KC_RIGHT, _______,
        _______,   _______,  _______,  _______, _______,    _______,                             _______, PT_LF,    PT_DN,   PT_UP,   PT_RT,    _______,
        EEP_RST,   _______,  _______,  _______, _______,             _______,           _______,          _______,  _______, _______, _______,  RESET,
                                                _______,    _______, _______,           _______, _______, _______
    ),

    [SYMB] = LAYOUT_moonlander(
        _______,  KC_F7,      KC_F5,    KC_F3,       KC_F1,       KC_F9,    KC_F11,      KC_F12,   KC_F10,   KC_F2,    KC_F4,      KC_F6,    KC_F8,    _______,
        _______,  KC_GRV,     _______,  _______,     _______,     _______,  _______,     _______,  _______,  KC_PERC,  S(KC_EQL),  S(KC_1),  S(KC_3),  _______,
        _______,  KC_TILD,    KC_LBRC,  S(KC_LBRC),  S(KC_9),     KC_EQL,   _______,     _______,  S(KC_8),  S(KC_0),  S(KC_RBRC), KC_RBRC,  KC_AT,    _______,
        _______,  _______,    _______,  _______,     _______,     _______,                         _______,  _______,  _______,    _______,  _______,  _______,
        _______,  _______,    _______,  _______,     _______,               _______,     _______,            _______,  _______,    _______,  _______,  _______,
                                                     _______,     _______,  _______,     _______,  _______,  _______
    ),
};

void clear(void)
{
    mods_tap = 0;
    pt_dir = 0;
    if (cur_layer != BASE)
    {
        if (layer_mode == LYR_STICKY)
        {
            cur_layer = BASE;
            layer_move(cur_layer);
            layer_mode = LYR_NONE;
        }
        else if (layer_mode == LYR_HELD_0)
        {
            layer_mode = LYR_HELD_1;
        }
    }
    set_color();
}

void unlock(void)
{
    mods_lock = 0;
    lock_key = 0;
    entered_num = 0;
    pressed_nums = 0;
    pt_dir = 0;
    if (layer_mode == LYR_LOCK)
    {
        layer_mode = LYR_NONE;
        cur_layer = BASE;
        layer_move(cur_layer);
    }
    clear();
}

void num_pressed (uint8_t num)
{
    pressed_nums++;
    entered_num |= num;
}

void num_released (uint8_t num)
{
    if (pressed_nums > 0)
        pressed_nums --;

    if (lock_key)
        entered_num &= ~num;
    else if (pressed_nums == 0 && entered_num > 0)
    {
        if (next_function)
        {
            last_key = KC_CAPS + entered_num;
            tap_code(last_key);
            next_function = 0;
        }
        else if (entered_num < 0xa)
            tap_code16 ((KC_Z + entered_num) | mods_tap | mods_lock | mods_held);
        else
            tap_code16 ((KC_A + entered_num - 0xa) | mods_tap | mods_lock | mods_held);
        entered_num = 0;
        clear();
    }
}

void mod_pressed (uint16_t mod)
{
    if (lock_key)
        lock_key = 0;

    if (mods_lock & mod)
    {
        mods_lock &= ~mod;
        mods_tap &= ~mod;
    }
    else
    {
        mods_tap ^= mod;
    }
    mods_held |= mod;
    set_color();
}

void mod_released (uint16_t mod)
{
    mods_held &= ~mod;
    set_color();
}

void key_pressed (uint16_t code)
{
    switch (code)
    {
    // keys that don't break shift
    case KC_A ... KC_Z:
    case KC_MINUS:
    case KC_GRAVE:
    case KC_SLASH:
    case KC_SPC:
    case KC_BSPC:
    case KC_QUOT:
        break;
    default:
        mods_tap &= ~QK_LSFT;
        mods_lock &= ~QK_LSFT;
        set_color();
        break;
    }

    if (lock_key == code)
    {
        if (++lock_key_rpt > 2)
            lock_key = 0;
    }
    else if (lock_key)
        lock_key = code;
    else
    {
        //                                                     :w protection
        tap_code16((code | mods_tap | mods_lock | mods_held) & ~(last_key == KC_SCLN && code == KC_W ? QK_LSFT : 0));
        last_key = code;
    }

    clear();
}


void key_released (uint16_t code)
{
    return;
}

void set_color(void)
{
    uint16_t set;
    set = mods_held | mods_lock | mods_tap;

    if (lock_key)
    {
        if (cur_layer != BASE)
            rgb_matrix_sethsv_noeeprom(COLOR_MOUSE);
        else
            rgb_matrix_sethsv_noeeprom(COLOR_RPT);
    }
    else if (cur_layer != BASE)
    {
        if (set)
            rgb_matrix_sethsv_noeeprom(COLOR_MLTPL);
        else
            rgb_matrix_sethsv_noeeprom(COLOR_LYR);

    }
    else
    {
        switch (set)
        {
        case 0:
            rgb_matrix_sethsv_noeeprom(COLOR_BASE);
            break;
        case QK_LSFT:
            rgb_matrix_sethsv_noeeprom(COLOR_SH);
            break;
        case QK_LCTL:
            rgb_matrix_sethsv_noeeprom(COLOR_CTL);
            break;
        case QK_LALT:
            rgb_matrix_sethsv_noeeprom(COLOR_ALT);
            break;
        case QK_LSFT | QK_LALT:
            rgb_matrix_sethsv_noeeprom(COLOR_ALTS);
            break;
        case QK_LSFT | QK_LCTL:
            rgb_matrix_sethsv_noeeprom(COLOR_CTLS);
            break;
        default:
            rgb_matrix_sethsv_noeeprom(COLOR_MLTPL);
            break;
        }
    }
}

void keyboard_post_init_user(void) {
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    set_color();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uint8_t lyr;

    if (record->event.pressed) {
        switch (keycode) {
        case VRSN:
            SEND_STRING (QMK_KEYBOARD "/" QMK_KEYMAP " @ " QMK_VERSION);
            return false;
        case HEX_PFX:
            SEND_STRING ("0x");
            return false;
        case RGB_TOG:
            rgb_matrix_toggle();
            return false;
        case RGB_MOD:
            return false;
        case NUM_1:
            num_pressed(1);
            return false;
        case NUM_2:
            num_pressed(2);
            return false;
        case NUM_4:
            num_pressed(4);
            return false;
        case NUM_8:
            num_pressed(8);
            return false;
        case KC_I:
            if (lock_key)
                num_pressed(1);
            else
                key_pressed(keycode);
            return false;
        case KC_U:
            if (lock_key)
                num_pressed(2);
            else
                key_pressed(keycode);
            return false;
        case KC_E:
            if (lock_key)
                num_pressed(4);
            else
                key_pressed(keycode);
            return false;
        case KC_O:
            if (lock_key)
                num_pressed(8);
            else
                key_pressed(keycode);
            return false;
        case KC_A:
            if (lock_key)
                num_pressed(0x10);
            else
                key_pressed(keycode);
            return false;
        case KC_LSFT:
            mod_pressed(QK_LSFT);
            return false;
        case KC_LCTL:
            mod_pressed(QK_LCTL);
            return false;
        case KC_LALT:
            mod_pressed(QK_LALT);
            return false;
        case MOD_LOCK:
            if (cur_layer != BASE)
                layer_mode = LYR_LOCK;
            else if (!mods_tap)
            {
                lock_key = last_key;
                lock_key_rpt = 0;
                last_key = 0;
                repeat_future = timer_read32();
                set_color();
            }
            else
                mods_lock = mods_tap;
            return false;
        case MOD_UNLK:
            unlock();
            return false;
        case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
            lyr = keycode & 0xff;
            if (lyr == cur_layer)
            {
                cur_layer = BASE;
                layer_mode = LYR_NONE;
            }
            else
            {
                cur_layer = lyr;
                layer_mode = LYR_HELD_0;
            }
            layer_move(cur_layer);
            set_color();
            return false;
        case KC_ESC:
            unlock();
            return true;
        case PT_LF:
        case PT_DN:
        case PT_UP:
        case PT_RT:
            last_key = keycode;
            return false;
        case NXT_FN:
            next_function = 1;
            return false;
        case KC_BTN1:
        case KC_BTN2:
            return true;
        default:
            key_pressed(keycode);
            return false;
        }
    }
    else
    {
        switch (keycode)
        {
        case NUM_1:
            num_released(1);
            return false;
        case NUM_2:
            num_released(2);
            return false;
        case NUM_4:
            num_released(4);
            return false;
        case NUM_8:
            num_released(8);
            return false;
        case KC_I:
            if (lock_key)
                num_released(1);
            else
                key_released(keycode);
            return false;
        case KC_U:
            if (lock_key)
                num_released(2);
            else
                key_released(keycode);
            return false;
        case KC_E:
            if (lock_key)
                num_released(4);
            else
                num_released(keycode);
            return false;
        case KC_O:
            if (lock_key)
                num_released(8);
            else
                key_released(keycode);
            return false;
        case KC_A:
            if (lock_key)
                num_released(0x10);
            else
                key_released(keycode);
            return false;
        case KC_LSFT:
            mod_released(QK_LSFT);
            return false;
        case KC_LCTL:
            mod_released(QK_LCTL);
            return false;
        case KC_LALT:
            mod_released(QK_LALT);
            return false;
        case RGB_MOD:
        case RGB_TOG:
        case MOD_LOCK:
        case MOD_UNLK:
        case NXT_FN:
            return false;
        case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
            lyr = keycode & 0xff;
            if (lyr == cur_layer)
            {
                switch (layer_mode)
                {
                case LYR_NONE:
                case LYR_HELD_1:
                    cur_layer = BASE;
                    layer_move(cur_layer);
                    layer_mode = LYR_NONE;
                    set_color();
                    break;
                case LYR_HELD_0:
                    layer_mode = LYR_STICKY;
                    break;
                case LYR_STICKY:
                case LYR_LOCK:
                    break;
                }
            }
            return false;
        case KC_ESC:
        case KC_BTN1:
        case KC_BTN2:
            return true;
        case PT_LF:
        case PT_DN:
        case PT_UP:
        case PT_RT:
            return false;
        default:
            key_released(keycode);
            return false;
        }
    }
    return true;
}

void matrix_scan_user()
{
    uint32_t now;
    report_mouse_t mouse;

    if (lock_key && entered_num)
    {
        now = timer_read32();
        if (timer_expired32(now, repeat_future))
        {
            switch (lock_key)
            {
            case PT_LF:
            case PT_DN:
            case PT_UP:
            case PT_RT:
                memset(&mouse, 0, sizeof(mouse));
                switch (lock_key)
                {
                case PT_LF:
                    mouse.x = -entered_num;
                    break;
                case PT_DN:
                    mouse.y = entered_num;
                    break;
                case PT_UP:
                    mouse.y = -entered_num;
                    break;
                case PT_RT:
                    mouse.x = entered_num;
                    break;
                }
                pointing_device_set_report(mouse);
                if (entered_num)
                    pointing_device_send();
                repeat_future = now + MOUSE_PERIOD;
                break;
            default:
                tap_code16(lock_key);
                repeat_future = now + (1000 / (entered_num << 3));
            }
        }
    }
}
