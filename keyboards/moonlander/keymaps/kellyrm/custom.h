#ifndef _KELLYRM_CUSTOM_H
#define _KELLYRM_CUSTOM_H

#include "quantum.h"

#define MOUSE_PERIOD 10

#define COLOR_BASE  HSV_MAGENTA
#define COLOR_SH    { HSV_RED }
#define COLOR_ALTS  { HSV_YELLOW }
#define COLOR_ALT   { HSV_GREEN }
#define COLOR_LYR   { HSV_SPRINGGREEN }
#define COLOR_CTL   { HSV_CYAN }
#define COLOR_CTLS  { HSV_PURPLE }
#define COLOR_RPT   { HSV_BLUE }
#define COLOR_MOUSE { HSV_CHARTREUSE }
#define COLOR_MLTPL { HSV_WHITE }

enum layers {
    BASE,  // default layer
    BIN,   // binary input
    FN,    // binary fn input
    UTIL,  // util keys
};

enum custom_keycodes {
    VRSN = ML_SAFE_RANGE,
    NUM_1,
    NUM_2,
    NUM_4,
    NUM_8,
    FN_1,
    FN_2,
    FN_4,
    FN_8,
    MOD_LOCK,
    MOD_UNLK,
    HEX_PFX,
    PT_LF,
    PT_RT,
    PT_UP,
    PT_DN,
    NXT_FN,
    MOD_UTIL,
};

enum lock_state {
    // lock is inactive
    LOCK_OFF = 0,
    // lock is pressed but no keycode has been sent
    LOCK_PRESSED,
    // lock is pressed, keycode has been sent
    LOCK_HELD,
    // lock is released without keycode sent
    LOCK_STICKY,
    // lock is locked
    LOCK_LOCKED,
};

#define LOCK_IS_ACTIVE(state) ((bool) (state != LOCK_OFF))

struct lock {
    enum lock_state state;
    uint16_t        mods;
    void          (*set_active)     (struct lock *lock, bool active);
    bool          (*process_record) (uint16_t keycode);
    uint8_t         color[3];
    union {
        uint16_t      _mod;
        layer_state_t _lyr;
    };
};

void set_active_layer(struct lock *lock, bool active);
void set_active_mod(struct lock *lock, bool active);
void set_active_repeat(struct lock *lock, bool active);
// always pressed
bool process_record_repeat(uint16_t keycode);

#define LOCK_MOD(MOD, CLR) \
    (struct lock) { \
        .state = LOCK_OFF, \
        .mods = 0, \
        .set_active = &set_active_mod, \
        .process_record = NULL, \
        .color = CLR, \
        ._mod = MOD, \
    }

#define LOCK_LYR(LYR, CLR) \
    (struct lock) { \
        .state = LOCK_OFF, \
        .mods = 0, \
        .set_active = &set_active_layer, \
        .process_record = NULL, \
        .color = CLR, \
        ._lyr = LYR, \
    }

enum locks {
    LCK_RPT,
    LCK_UTIL,
    LCK_FN,
    LCK_SHIFT,
    LCK_CTRL,
    LCK_ALT,
    LCK_MAX,
};

struct bin_entry {
    uint8_t pressed; // currently pressed bits
    uint8_t entered; // entry
    bool    next_fn; // send function 
};

struct key_rpt {
    uint16_t lock; // locked keycode
    uint8_t  press_count; // press count of locked key
    uint32_t future; // for timer
};

void set_lock_state(struct lock *l, enum lock_state state);
bool process_lock(struct lock *l, bool presesd);
bool process_bin(uint8_t val, bool pressed, bool fn);
void send_key(uint16_t kc);

void lock(bool set);
void set_color(void);

#endif
