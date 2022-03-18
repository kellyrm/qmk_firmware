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

#define COLOR_BASE  HSV_MAGENTA
#define COLOR_SH    HSV_RED
#define COLOR_CTL   HSV_CYAN
#define COLOR_CTLS  HSV_PURPLE
#define COLOR_ALT   HSV_GREEN
#define COLOR_ALTS  HSV_YELLOW
#define COLOR_LYR   HSV_SPRINGGREEN
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
};

uint8_t pressed_nums = 0;
uint8_t entered_num  = 0;
uint16_t mods = 0;
uint16_t mods_lock = 0;
uint8_t cur_layer = BASE;
bool layer_lock = false;

void num_pressed(uint8_t num);
void num_released(void);
void mod_pressed(uint16_t mod);
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
        KC_AT,    KC_NO,     KC_NO,     KC_LCTL,     OSL(UTIL),             MOD_UNLK,    MOD_LOCK,           KC_LALT,   KC_LCTL,    KC_NO,    KC_INS,   KC_DEL,
                                                     KC_LSFT,     KC_ENT,   RGB_TOG,     RGB_MOD,  KC_BSPC,  KC_SPC
    ),

    [UTIL] = LAYOUT_moonlander(
        LED_LEVEL, _______,  _______,  _______, _______,    _______, _______,           _______, _______, _______, _______, _______, _______,  _______,
        _______,   KC_SCLN,  KC_COMMA, KC_DOT,  S(KC_SCLN), _______, _______,           _______, _______, _______, _______, _______, _______,  _______,
        _______,   NUM_8,    NUM_4,    NUM_2,   NUM_1,      HEX_PFX,  _______,           _______, _______, KC_LEFT, KC_DOWN, KC_UP,   KC_RIGHT, _______,
        _______,   _______,  _______,  _______, _______,    _______,                             _______, _______, KC_MPRV, KC_MNXT, _______,  _______,
        EEP_RST,   _______,  _______,  _______, _______,             _______,           _______,          KC_VOLU, KC_VOLD, KC_MUTE, _______,  RESET,
                                                KC_0,       _______, _______,           _______, _______, _______
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
    mods = 0;
    if (cur_layer != BASE && !layer_lock)
    {
        cur_layer = BASE;
        layer_move(cur_layer);
    }
    set_color();
}

void unlock(void)
{
    mods_lock = 0;
    layer_lock = false;
    clear();
}

void num_pressed (uint8_t num)
{
    pressed_nums++;
    entered_num |= num;
}

void num_released (void)
{
    if (pressed_nums > 0)
        pressed_nums --;
    if (pressed_nums == 0 && entered_num > 0)
    {
        if (entered_num < 0xa)
            tap_code (KC_Z + entered_num);
        else
            tap_code (KC_A + entered_num - 0xa);
        entered_num = 0;
        clear();
    }
}

void mod_pressed (uint16_t mod)
{
    if (mods_lock & mod)
    {
        mods_lock &= ~mod;
        mods &= ~mod;
    }
    else
        mods |= mod;
    set_color();
}

void key_pressed (uint16_t code)
{
    tap_code16(code | mods | mods_lock);
    clear();
}

void key_released (uint16_t code)
{
    return;
    unregister_code16(code | mods | mods_lock);
    if (mods)
        mods = 0;

}

void set_color(void)
{
    uint16_t set;
    set = mods | mods_lock;

    if (cur_layer != BASE)
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
            mods_lock = mods;
            layer_lock = cur_layer;
            return false;
        case MOD_UNLK:
            unlock();
            return false;
        case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
            lyr = keycode & 0xff;
            if (lyr == cur_layer)
            {
                cur_layer = BASE;
                layer_lock = false;
            }
            else
            {
                cur_layer = lyr;
            }
            layer_move(cur_layer);
            set_color();
            return false;
        case KC_ESC:
            unlock();
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
        case NUM_2:
        case NUM_4:
        case NUM_8:
            num_released();
            return false;
        case KC_LSFT:
        case KC_LCTL:
        case KC_LALT:
        case MOD_LOCK:
        case MOD_UNLK:
        case RGB_MOD:
        case RGB_TOG:
            return false;
        case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
            return false;
        case KC_ESC:
            return true;
        default:
            key_released(keycode);
            return false;
        }
    }
    return true;
}
