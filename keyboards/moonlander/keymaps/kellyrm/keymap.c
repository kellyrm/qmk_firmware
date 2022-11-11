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
#include <print.h>
#include "version.h"
#include "rgb_matrix/rgb_matrix.h"
#include "timer.h"
#include "pointing_device.h"
#include "eeconfig.h"
#include "custom.h"
#include "stdio.h"

static struct lock locks[] = {
    [LCK_SHIFT] = SIMPLE_LOCK(QK_LSFT, 0,                   COLOR_SH),
    [LCK_CTRL]  = SIMPLE_LOCK(QK_LCTL, 0,                   COLOR_CTL),
    [LCK_ALT]   = SIMPLE_LOCK(QK_LALT, LYR_BIN,             COLOR_ALT),
    [LCK_FN]    = SIMPLE_LOCK(0,       LYR_FN,              COLOR_MLTPL),
    [LCK_UTIL]  = SIMPLE_LOCK(0,      (LYR_UTIL | LYR_BIN), COLOR_LYR),
    [LCK_RPT]   = {
        .state       =  LOCK_OFF,
        .mods        =  0,
        .lyrs        =  LYR_BIN,
        .color       =  COLOR_RPT,
        .set_active  = &set_active_repeat,
        .key_pressed = &key_pressed_repeat,
        .cb_data     =  NULL,
    },
    [LCK_MOUSE] = SIMPLE_LOCK(0,      (LYR_BIN | LYR_MOUSE), COLOR_MLTPL),
};

static struct bin_entry bin_entry;
static struct key_rpt   key_rpt;
static struct mouse_ctl mouse_ctl;

#define ENABLE_PROFILE 0
#if ENABLE_PROFILE
#define N_SAMPLES 1024
static uint8_t samples[N_SAMPLES];
static uint32_t profile_start;
static uint32_t last_sample;
static int32_t profile_progress;
static uint8_t profile_active = 0;
#endif

#define ENABLE_STATS 1
#if ENABLE_STATS
uint32_t keypresses[MATRIX_ROWS][MATRIX_COLS] = {0};
#endif

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
        KC_ESC,   KC_A,      KC_O,      KC_E,        KC_U,        KC_I,     S(KC_6),     KC_BSPC,  KC_D,     KC_H,      KC_T,       KC_N,     KC_S,     KC_MINUS,
        KC_GRV,   KC_QUOTE,  KC_Q,      KC_J,        KC_K,        KC_X,                            KC_B,     KC_M,      KC_W,       KC_V,     KC_Z,     KC_SLASH,
        KC_AT,    KC_NO,     KC_NO,     KC_ESC,      MOD_UTIL,              MOD_UNLK,    MOD_LOCK,           KC_LALT,   KC_BTN1,    KC_BTN2,  KC_INS,   KC_DEL,
                                                     KC_LSFT,     KC_ENT,   KC_ESC,      KC_NO,    KC_LCTL,  KC_SPC
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
        PROFILE,   STAT_PR,  STAT_RS,  _______, _______,    _______, _______,           _______, _______, _______,  _______, _______, _______,  RGB_TOG,
        _______,   KC_SCLN,  KC_COMMA, KC_DOT,  S(KC_SCLN), _______, _______,           _______, _______, _______,  _______, _______, _______,  _______,
        MOD_FN,    _______,  _______,  _______, _______,    KC_0,    HEX_PFX,           _______, _______, KC_LEFT,  KC_DOWN, KC_UP,   KC_RIGHT, _______,
        _______,   _______,  _______,  _______, _______,    _______,                             _______, PT_LEFT,  PT_DOWN, PT_UP,   PT_RIGHT, _______,
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

    [MOUSE] = LAYOUT_moonlander(
        _______,   _______, _______, _______, _______, _______,   _______,   _______, _______,   _______, _______, _______, _______,   _______,
        _______,   _______, _______, _______, _______, _______,   _______,   _______, _______,   _______, _______, _______, _______,   _______,
        MOUSE_ESC, _______, _______, _______, _______, MOUSE_HLV, _______,   _______, _______,   _______, _______, _______, _______, MOUSE_REV,
        _______,   _______, _______, _______, _______, _______,                       _______,   _______, _______, _______, _______,   _______,
        _______,   _______, _______, _______, _______,            _______,   _______,            _______, _______, _______, _______,   _______,
                                              _______, _______,   _______,   _______, _______, MOUSE_DBL
    ),
};

void set_active_repeat(bool active, void *data)
{
    if (active)
    {
        key_rpt.press_count = 1;
        key_rpt.future = timer_read32();
    }

    bin_entry.entered = 0;
}

bool key_pressed_repeat(uint16_t keycode, void *data)
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
            else if (LOCK_IS_ACTIVE(locks[LCK_MOUSE].state))
                process_mouse(bin_entry.entered);
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
    switch (kc & 0x7f)
    {
        // keys that don't break shift
        case KC_A ... KC_Z:
        case KC_1 ... KC_0:
        case KC_BSLS:
        case KC_MINUS:
        case KC_SLASH:
        case KC_INS:
        case KC_DEL:
        case KC_SPC:
        case KC_BSPC:
        case KC_QUOT:
        case KC_SCLN:
        case KC_COMMA:
        case KC_DOT:
        case KC_GRAVE:
        case KC_TAB:
            break;
        default:
            set_lock_state(&locks[LCK_SHIFT], LOCK_OFF);
            break;
    }

    if (((key_rpt.lock & 0x7f) == KC_SCLN) && (locks[LCK_SHIFT].state == LOCK_HELD) && ((kc & 0x7f) == KC_W))
        set_lock_state(&locks[LCK_SHIFT], LOCK_OFF);

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

            if (l->key_pressed && l->key_pressed(kc, l->cb_data))
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
    bool rpt = true;

    for (i = 0; i < LCK_MAX; i++)
    {
        l = &locks[i];
        if (LOCK_IS_ACTIVE(l->state))
        {
            rpt = false;
            if (set)
                set_lock_state(l, LOCK_LOCKED);
            else
                set_lock_state(l, LOCK_OFF);
        }
    }

    if (!set)
        bin_entry.entered = 0;

    if (set && rpt)
        set_lock_state(&locks[LCK_RPT], LOCK_LOCKED);
}

void set_lock_state(struct lock *l, enum lock_state state)
{
    bool was_active;
    int i;
    layer_state_t lyrs = LYR_BASE;

    if (l->state != state)
    {
        was_active = LOCK_IS_ACTIVE(l->state);
        l->state = state;
        if (was_active != LOCK_IS_ACTIVE(l->state))
        {
            if (l->set_active)
                l->set_active(!was_active, l->cb_data);

            for (i = 0; i < LCK_MAX; i++)
            {
                l = &locks[i];
                if (l->lyrs && LOCK_IS_ACTIVE(l->state))
                    lyrs |= l->lyrs;
            }

            layer_state_set(lyrs);
        }
    }

    //if (LOCK_IS_ACTIVE(state))
     //   rgb_matrix_sethsv_noeeprom();

}

void set_color(color_t color)
{
    // rgb_matrix_sethsv_noeeprom(color[0], color[1], color[2]);
    // rgb_matrix_sethsv_noeeprom(color[0], color[1], color[2]);
    rgb_matrix_set_color_all(0xff, 0xff, 0xff);
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

void keyboard_pre_init_user(void) {
}

void keyboard_post_init_user(void) {
/*    rgb_matrix_enable_noeeprom();
    eeconfig_disable();
    // rgb_matrix_enable();
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    // rgb_matrix_mode(RGB_MATRIX_SOLID_COLOR);
    //rgb_matrix_sethsv_noeeprom(COLOR_BASE);
  */  set_color(COLOR_BASE);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    bool pressed = record->event.pressed;
    uint16_t uval;

#if ENABLE_STATS
    char buf[16];
    int i, j;
    keypresses[record->event.key.row][record->event.key.col]++;
#endif

    switch (keycode) {
    case PROFILE:
#if ENABLE_PROFILE
        if (pressed && !profile_active)
        {
            profile_active = 1;
            profile_progress = -1;
        }
#endif
        return false;
    case STAT_PR:
#if ENABLE_STATS
        if (pressed)
        {
            for (i = 0; i < MATRIX_ROWS>>1; i++)
            {
                for (j = 0; j < MATRIX_COLS; j++)
                {
                    snprintf(buf, sizeof(buf), "%6lu ", keypresses[i][j]);
                    SEND_STRING(buf);
                }
                SEND_STRING(" | ");
                for (j = 0; j < MATRIX_COLS; j++)
                {
                    snprintf(buf, sizeof(buf), "%6lu ", keypresses[i + (MATRIX_ROWS>>1)][j]);
                    SEND_STRING(buf);
                }
                SEND_STRING("\n");
            }
        }
#endif
        return false;
    case STAT_RS:
#if ENABLE_STATS
        if (pressed)
            memset(keypresses, 0, sizeof(keypresses));
#endif
        return false;
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
        case MOD_FN:
            return process_lock(&locks[LCK_FN], pressed);
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
    case PT_LEFT...PT_DOWN:
      uval = keycode - PT_LEFT;
      mouse_ctl.sel = (uval & 2) >> 1;
      mouse_ctl.sign = ~(uval & 1);
      mouse_ctl.mag = 2;
      set_lock_state(&locks[LCK_MOUSE], LOCK_STICKY);
      break;
    case MOUSE_REV:
      mouse_ctl.sign ^= 1;
      break;
    case MOUSE_DBL:
      mouse_ctl.mag = 3;
      break;
    case MOUSE_HLV:
      mouse_ctl.mag = 1;
      break;
    case MOUSE_ESC:
      set_lock_state(&locks[LCK_MOUSE], LOCK_OFF);
      break;
        /* case PT_LF: */
        /* case PT_DN: */
        /* case PT_UP: */
        /* case PT_RT: */
        /*     last_key = keycode; */
        /*     return false; */
        /* case NXT_FN: */
        /*     next_function = 1; */
        /*     return false; */
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
#if ENABLE_PROFILE
    uint32_t mean;
    int32_t variance;
    uint32_t i;
    char buf[64];
#endif
    /* report_mouse_t mouse; */

#if ENABLE_PROFILE
    if (profile_active)
    {
        now = timer_read32();
        if (profile_progress < 0)
        {
            profile_progress = 0;
            last_sample = now;
            profile_start = now;
        }
        else if (profile_progress < N_SAMPLES)
        {
            samples[profile_progress++] = now - last_sample;
            last_sample = now;
        }
        else
        {
            profile_active = 0;
            mean = 0;
            for (i = 0; i < N_SAMPLES; i++)
                mean += samples[i];
            mean /= N_SAMPLES;
            variance = 0;
            for (i = 0; i < N_SAMPLES; i++)
                variance += (samples[i] - mean) * (samples[i] - mean);
            variance /= N_SAMPLES; 
            snprintf(buf, sizeof(buf), "m:%lu v:%ld", mean, variance);
            SEND_STRING(buf);
        }
    }
#endif

    if (bin_entry.pressed && key_rpt.lock && LOCK_IS_ACTIVE(locks[LCK_RPT].state))
    {
        now = timer_read32();
        if (timer_expired32(now, key_rpt.future))
        {
            tap_code16(key_rpt.lock);
            key_rpt.future = now + (1024 / (bin_entry.pressed << 2));
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


void process_mouse(uint8_t val)
{
  report_mouse_t report = {0};
  int8_t ival;
  // 7 bits
  // 4 bits
  // 2 bits
  ival = val << 3;
  if (mouse_ctl.sign)
    ival ^= ~0;

  if (mouse_ctl.sel)
    report.y = ival;
  else
    report.x = ival;

  for (ival = 0; ival < (1 << mouse_ctl.mag); ival++)
    host_mouse_send(&report);

  /* if (locks[LCK_MOUSE].state == LOCK_STICKY) */
  /*   locks[LCK_MOUSE].state = LOCK_OFF; */
}
