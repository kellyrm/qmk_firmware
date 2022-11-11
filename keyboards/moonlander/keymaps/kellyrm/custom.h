#ifndef _KELLYRM_CUSTOM_H
#define _KELLYRM_CUSTOM_H

#include "quantum.h"

#define MOUSE_PERIOD 10

typedef uint8_t color_t[3];
#define COLOR_BASE  ((color_t) { HSV_MAGENTA } )
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
    MOUSE, // mouse util stuff
};

#define LYR_BASE  (1 << BASE)
#define LYR_BIN   (1 << BIN)
#define LYR_FN    (1 << FN)
#define LYR_UTIL  (1 << UTIL)
#define LYR_MOUSE (1 << MOUSE)

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
    PT_LEFT,
    PT_RIGHT,
    PT_UP,
    PT_DOWN,
    MOUSE_REV,
    MOUSE_DBL,
    MOUSE_HLV,
    MOUSE_ESC,
    MOD_FN,
    MOD_UTIL,
    PROFILE,
    STAT_PR,
    STAT_RS,
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
    layer_state_t   lyrs;
    uint8_t         color[3];
    void          (*set_active)  (bool active,      void *data);
    bool          (*key_pressed) (uint16_t keycode, void *data);
    void           *cb_data;
};

void set_active_repeat (bool active,      void *data);
bool key_pressed_repeat(uint16_t keycode, void *data);

#define SIMPLE_LOCK(MODS, LYRS, COLOR) \
    ((struct lock) {                    \
        .state = LOCK_OFF,              \
        .mods = MODS,                   \
        .lyrs =  LYRS,                  \
        .color = COLOR,                 \
        .set_active = NULL,             \
        .key_pressed = NULL,            \
        .cb_data = NULL,                \
    })

enum locks {
    LCK_RPT,
    LCK_UTIL,
    LCK_FN,
    LCK_SHIFT,
    LCK_CTRL,
    LCK_ALT,
    LCK_MOUSE,
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

struct mouse_ctl {
  uint8_t sel  : 1; // 0 = x , 1 = y
  uint8_t sign : 1; // 0 = pos, 1 = neg
  uint8_t mag  : 2; // 1-3
};

void set_lock_state(struct lock *l, enum lock_state state);
bool process_lock(struct lock *l, bool presesd);
bool process_bin(uint8_t val, bool pressed, bool fn);
void send_key(uint16_t kc);

void lock(bool set);
void set_color(color_t color);

void process_mouse(uint8_t val);

#endif
