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
#include "custom.h"

static struct lock locks[] = {
    [LCK_SHIFT] = LOCK_MOD(QK_LSFT, COLOR_SH),
    [LCK_CTRL]  = LOCK_MOD(QK_LCTL, COLOR_CTL),
    [LCK_ALT]   = LOCK_MOD(QK_LALT, COLOR_ALT),
    [LCK_RPT]   = {
        .state = LOCK_OFF,
        .mods = 0,
        .set_active = &set_active_repeat,
        .process_record = &process_record_repeat,
        .color = COLOR_RPT,
        ._lyr = (1 << BIN),
    },
    [LCK_UTIL]  = LOCK_LYR((1 << UTIL) | (1 << BIN), COLOR_LYR),
    [LCK_FN]    = LOCK_LYR(1 << FN, COLOR_MLTPL),
};


static struct bin_entry bin_entry;

static struct key_rpt key_rpt;

// uint16_t pt_dir = 0;

/*
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
*/

/*
    LAYOUT_moonlander(
        _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______,          _______,   _______,          _______, _______, _______, _______, _______,
                                            _______, _______, _______,   _______, _______, _______
    ),
*/


// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [BASE] = LAYOUT_moonlander(
        // dollar ampersand    bracket    brace          brace     paren     equal      asterisk    paren     percent    plus      bracket exclamation   pound
        S(KC_4),  S(KC_7),   KC_LBRC,   S(KC_LBRC),  S(KC_RBRC),  S(KC_9),  KC_EQL,      S(KC_8),  S(KC_0),  KC_PERC,   S(KC_EQL),  KC_RBRC,  S(KC_1),  S(KC_3),
        KC_TAB,   KC_SCLN,   KC_COMMA,  KC_DOT,      KC_P,        KC_Y,     KC_1,        KC_0,     KC_F,     KC_G,      KC_C,       KC_R,     KC_L,     KC_BSLS,
        KC_ESC,   KC_A,      KC_O,      KC_E,        KC_U,        KC_I,     LSFT(KC_6),  KC_BSPC,  KC_D,     KC_H,      KC_T,       KC_N,     KC_S,     KC_MINUS,
        KC_GRV,   KC_QUOTE,  KC_Q,      KC_J,        KC_K,        KC_X,                            KC_B,     KC_M,      KC_W,       KC_V,     KC_Z,     KC_SLASH,
        KC_AT,    KC_NO,     KC_NO,     KC_NO,       MOD_UTIL,              MOD_UNLK,    MOD_LOCK,           KC_LALT,   KC_BTN1,    KC_BTN2,  KC_INS,   KC_DEL,
                                                     KC_LSFT,     KC_ENT,   KC_NO,       KC_LSFT,  KC_LCTL,  KC_SPC
    ),

    [BIN] = LAYOUT_moonlander(
        _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, NUM_8,   NUM_4,   NUM_2,   NUM_1,   _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______,          _______,   _______,          _______, _______, _______, _______, _______,
                                            _______, _______, _______,   _______, _______, _______
    ),

    [UTIL] = LAYOUT_moonlander(
        _______,   _______,  _______,  _______, _______,    _______, _______,           _______, _______, _______,  _______, _______, _______,  RGB_TOG,
        _______,   KC_SCLN,  KC_COMMA, KC_DOT,  S(KC_SCLN), _______, _______,           _______, _______, _______,  _______, _______, _______,  _______,
        NXT_FN,    _______,  _______,  _______, _______,    KC_0,    HEX_PFX,           _______, _______, KC_LEFT,  KC_DOWN, KC_UP,   KC_RIGHT, _______,
        _______,   _______,  _______,  _______, _______,    _______,                             _______, PT_LF,    PT_DN,   PT_UP,   PT_RT,    _______,
        EEP_RST,   _______,  _______,  _______, _______,             _______,           _______,          _______,  _______, _______, _______,  RESET,
                                                _______,    _______, _______,           _______, _______, _______
    ),

    [FN] = LAYOUT_moonlander(
        _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, FN_8,    FN_4,    FN_2,    FN_1,    _______, _______,   _______, _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______, _______,                     _______, _______, _______, _______, _______, _______,
        _______, _______, _______, _______, _______,          _______,   _______,          _______, _______, _______, _______, _______,
                                            _______, _______, _______,   _______, _______, _______
    ),
};

void set_active_layer(struct lock *lock, bool active)
{
    if (active)
        layer_or(lock->_lyr);
    else
        layer_and(~lock->_lyr);
}

void set_active_mod(struct lock *lock, bool active)
{
    if (active)
        lock->mods = lock->_mod;
    else
        lock->mods = 0;
}

void set_active_repeat(struct lock *l, bool active)
{
    if (active)
    {
        layer_or(1 << BIN);
        key_rpt.press_count = 1;
        key_rpt.future = timer_read32();
    }
    else
        layer_and(~(1 << BIN));

    bin_entry.entered = 0;
}

bool process_record_repeat(uint16_t keycode)
{
    if (key_rpt.lock == keycode)
    {
        if (++key_rpt.press_count > 3)
            process_lock(LCK_RPT, false);
    }
    else
    {
        key_rpt.lock = keycode;
        key_rpt.press_count = 1;
    }

    return false;
}

bool process_bin(uint8_t val, bool pressed, bool fn)
{
    if (val == 0)
        return false;

    if (pressed)
    {
        bin_entry.pressed |= val;
        bin_entry.entered |= val;
    }
    else
    {
        bin_entry.pressed &= ~val;

        if (!LOCK_IS_ACTIVE(locks[LCK_RPT].state) && bin_entry.pressed == 0)
        {
            if (fn)
                send_key(KC_CAPS + bin_entry.entered);
            else if (bin_entry.entered < 0xa)
                send_key(KC_Z + bin_entry.entered);
            else
                send_key(KC_A + bin_entry.entered - 0xa);
            bin_entry.entered = 0;
        }
    }
    return false;
}

bool process_lock(struct lock *l, bool pressed)
{
    switch (l->state)
    {
        // initial state
        case LOCK_OFF:
            if (pressed)
                set_lock_state(l, LOCK_PRESSED);
            break;
        // presed without sending keys -> sticky
        case LOCK_PRESSED:
            if (!pressed)
                set_lock_state(l, LOCK_STICKY);
            break;
        // held and send keys -> release off
        case LOCK_HELD:
            if (!pressed)
                set_lock_state(l, LOCK_OFF);
            break;
        // locked -> press to turn off
        case LOCK_STICKY:
        case LOCK_LOCKED:
            if (pressed)
                set_lock_state(l, LOCK_OFF);
            break;
    }

    return false;
}

void send_key(uint16_t kc)
{
    int i;
    struct lock *l;
    bool lock_process = false;

    // most symbols n stuff turn off shift
    switch (kc & 0xff)
    {
        // keys that don't break shift
        case KC_A ... KC_Z:
        case KC_1 ... KC_0:
        case KC_MINUS:
        case KC_GRAVE:
        case KC_SLASH:
        case KC_SPC:
        case KC_BSPC:
        case KC_QUOT:
            break;
        default:
            set_lock_state(&locks[LCK_SHIFT], LOCK_OFF);
            break;
    }

    for (i = 0; i < LCK_MAX; i++)
    {
        l = &locks[i];
        if (LOCK_IS_ACTIVE(l->state))
        {
            kc |= l->mods;
            if (l->state == LOCK_PRESSED)
                set_lock_state(l, LOCK_HELD);
            else if (l->state == LOCK_STICKY)
                set_lock_state(l, LOCK_OFF);

            if (l->process_record && l->process_record(kc))
            {
                // first layer to return true stops further processing
                lock_process = true;
                break;
            }
        }
    }

    key_rpt.lock = kc;
    key_rpt.press_count = 1;

    if (!lock_process)
        tap_code16(kc);

   // return false;
}

void lock(bool set)
{
    int i;
    struct lock *l;

    for (i = 0; i < LCK_MAX; i++)
    {
        l = &locks[i];
        if (LOCK_IS_ACTIVE(l->state))
        {
            if (set)
                set_lock_state(l, LOCK_LOCKED);
            else
                set_lock_state(l, LOCK_OFF);
        }
    }

    if (!set)
        bin_entry.entered = 0;

    // set_color();
}

void set_lock_state(struct lock *l, enum lock_state state)
{
    bool was_active;

    if (l->state != state)
    {
        was_active = LOCK_IS_ACTIVE(l->state);
        l->state = state;
        if (was_active != LOCK_IS_ACTIVE(l->state) && l->set_active)
            l->set_active(l, !was_active);
    }
}

/*
void unlock(void)
{
    mods_lock = 0;
    lock_key = 0;
    entered_num = 0;
    pressed_nums = 0;
    pt_dir = 0;
    //if (layer_mode == LYR_LOCK)
    {
     //   layer_mode = LYR_NONE;
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
*/

void set_color(void)
{
   /* 
    if (LOCK_IS_ACTIVE(locks[LCK_RPT].state))
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
    */
}

void keyboard_post_init_user(void) {
    rgb_matrix_enable_noeeprom();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    rgb_matrix_sethsv_noeeprom(COLOR_BASE);
    // set_color();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool pressed = record->event.pressed;

    switch (keycode) {
        case HEX_PFX:
            if (pressed)
                SEND_STRING ("0x");
            return false;
        case RGB_TOG:
            if (pressed)
                rgb_matrix_toggle();
            return false;
        case NUM_1:
            return process_bin(1, pressed, false);
        case NUM_2:
            return process_bin(2, pressed, false);
        case NUM_4:
            return process_bin(4, pressed, false);
        case NUM_8:
            return process_bin(8, pressed, false);
        case FN_1:
            return process_bin(1, pressed, true);
        case FN_2:
            return process_bin(2, pressed, true);
        case FN_4:
            return process_bin(4, pressed, true);
        case FN_8:
            return process_bin(8, pressed, true);
        case KC_LSFT:
            return process_lock(&locks[LCK_SHIFT], pressed);
        case KC_LCTL:
            return process_lock(&locks[LCK_CTRL], pressed);
        case KC_LALT:
            return process_lock(&locks[LCK_ALT], pressed);
        case MOD_UTIL:
            return process_lock(&locks[LCK_UTIL], pressed);
        case MOD_LOCK:
            if (pressed)
                lock(true);
            return false;
        case MOD_UNLK:
            lock(false);
            return false;
        case KC_ESC:
            if (pressed)
                lock(false);
            return true;
        /*
        case PT_LF:
        case PT_DN:
        case PT_UP:
        case PT_RT:
            last_key = keycode;
            return false;
        case NXT_FN:
            next_function = 1;
            return false;
        */
        case KC_BTN1:
        case KC_BTN2:
            return true;
        default:
            if (pressed)
                send_key(keycode);
            return false;
    }

    return true;
}

void matrix_scan_user()
{
    uint32_t now;
    //report_mouse_t mouse;

    if (LOCK_IS_ACTIVE(locks[LCK_RPT].state) && key_rpt.lock && bin_entry.pressed)
    {
        now = timer_read32();
        if (timer_expired32(now, key_rpt.future))
        {
            tap_code16(key_rpt.lock);
            key_rpt.future = now + (1000 / (bin_entry.pressed << 4));
            /*
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
            }
            */
        }
    }
}
