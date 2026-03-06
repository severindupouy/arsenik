#include QMK_KEYBOARD_H
#include "arsenik.h"

// Shortcuts adapted for Ergol host layout
// (In Ergol, some letters are at different physical positions than QWERTY)
#define SC_UNDO  C(KC_Z)
#define SC_CUT   C(KC_X)
#define SC_COPY  C(KC_W)     // C is at W position in Ergol
#define SC_PASTE C(KC_V)
#define SC_REDO  C(KC_P)     // Y is at P position in Ergol
#define SC_CLOSE C(KC_T)     // W is at T position in Ergol
#define SC_SAVE  C(KC_S)
#define SC_ALL   C(KC_A)

enum arsenik_layers {
    _base,
    _num_lock,
    _symbols,
    _vim_nav,
    _num_nav,
    _num_row,
    _function,
};

enum custom_keycodes {
    ODK_1 = SAFE_RANGE,  // „
    ODK_2,  // "
    ODK_3,  // "
    ODK_4,  // ¢
    ODK_5,  // ‰
};

// Converted from the Selenium ZMK keymap (zmk-keyboard-quacken).
//
// Default thumb config (no VIM_NAVIGATION):
//   Left:  OSM(Shift) | LT(NumNav, Bksp) | LT(Function, Tab)
//   Right: LT(Function, Enter) | LT(NumNav, Space) | OSL(Symbols)
//
// Layers accessible by default: Base, NumLock, Symbols, NumNav, Function.
// VimNav and NumRow exist but are only reachable if you change the thumb keys.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    // 0. Base layer
    [_base] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        KC_TAB,   KC_Q, KC_W,  KC_E,  KC_R,  KC_T,      KC_Y, KC_U,  KC_I,    KC_O,   KC_P,    KC_BSPC,
        KC_ESC,   KC_A, KC_SS, KC_DD, KC_FF, KC_G,      KC_H, KC_JJ, KC_KK,   KC_LL,  KC_SCLN, KC_ENT,
        KC_LSFT,  KC_Z, KC_X,  KC_C,  KC_V,  KC_B,      KC_N, KC_M,  KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
              OSM(MOD_LSFT),  LT(_num_nav, KC_BSPC),  LT(_function, KC_TAB),
              LT(_function, KC_ENT),  LT(_num_nav, KC_SPC),  OSL(_symbols)
    ),

    // 1. NumLock layer -- sticky NumNav that stays on until deactivated
    [_num_lock] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,  KC_ESC,   KC_HOME,  KC_UP,    KC_END,   KC_PGUP,      TO(_base), AS(7),    AS(8),    AS(9),    AS(SLSH), __,
        __,  AS(EQL),  KC_LEFT,  KC_DOWN,  KC_RGHT,  KC_PGDN,      AS(MINS),  AS(4),    AS(5),    AS(6),    AS(0),    __,
        __,  AS(DLR),  AS(COLN), AS(ASTR), AS(PLUS), AS(PERC),     AS(COMM),  AS(1),    AS(2),    AS(3),    AS(DOT),  __,
              __,  LT(_num_nav, KC_BSPC),  KC_TAB,
              __,  LT(_num_nav, KC_SPC),  OSL(_symbols)
    ),

    // 2. Symbols layer -- programming symbols (AltGr layer for Ergol)
    [_symbols] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,  AS(CIRC), AS(LABK), AS(RABK), AS(DLR),  AS(PERC),     AS(AT),   AS(AMPR), AS(ASTR), AS(QUOT), AS(GRV),  __,
        __,  AS(LCBR), AS(LPRN), AS(RPRN), AS(RCBR), AS(EQL),      AS(BSLS), AS(PLUS), AS(MINS), AS(SLSH), AS(DQUO), __,
        __,  AS(TILD), AS(LBRC), AS(RBRC), AS(UNDS), AS(HASH),     AS(PIPE), AS(EXLM), AS(SCLN), AS(COLN), AS(QUES), __,
              MO(_num_nav),  KC_SPC,  KC_ENT,
              __,  KC_RALT,  __
    ),

    // 3. VimNav layer -- HJKL arrow cluster + GUI shortcuts (not accessible by default)
    [_vim_nav] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,  KC_CAPS,  SC_CLOSE,     A(KC_LEFT),  A(KC_RGHT),  XX,         KC_HOME, KC_PGDN, KC_PGUP, KC_END,  KC_DEL, __,
        __,  SC_ALL,    SC_SAVE,      S(KC_TAB),   KC_TAB,      XX,         KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, XX,     __,
        __,  SC_UNDO,   SC_CUT,       SC_COPY,      SC_PASTE,     SC_REDO,     XX,      XX,      XX,      XX,      XX,     __,
              KC_CAPS,  LT(_function, KC_DEL),  MO(_num_row),
              __,  MO(_function),  KC_RALT
    ),

    // 4. NumNav layer -- inverted T navigation + numpad
    [_num_nav] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,  KC_ESC,  KC_HOME, KC_UP,   KC_END,   KC_PGUP,      TO(_num_lock), AS(7),   AS(8),   AS(9),   AS(SLSH), __,
        __,  SC_ALL,   KC_LEFT, KC_DOWN, KC_RGHT,  KC_PGDN,      AS(MINS),      AS(4),   AS(5),   AS(6),   AS(0),    __,
        __,  SC_UNDO,  SC_CUT,   SC_COPY,  SC_PASTE,  SC_REDO,       AS(COMM),      AS(1),   AS(2),   AS(3),   AS(DOT),  __,
              KC_CAPS,  LT(_function, KC_DEL),  S(KC_TAB),
              KC_ESC,  LT(_function, KC_SPC),  KC_RALT
    ),

    // 5. NumRow layer -- numbers on homerow (not accessible by default)
    [_num_row] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,  AS_S1,  AS_S2,  AS_S3,  AS_S4,  AS_S5,       AS_S6,    AS_S7,    AS_S8,    AS_S9,    AS_S0,    __,
        __,  AS(1),  AS(2),  AS(3),  AS(4),  AS(5),       AS(6),    AS(7),    AS(8),    AS(9),    AS(0),    __,
        __,  XX,     XX,     XX,     XX,     XX,          AS(MINS), AS(COMM), AS(DOT),  AS(COLN), AS(SLSH), __,
              __,  S(KC_SPC),  __,
              __,  S(KC_SPC),  KC_RALT
    ),

    // 6. Function layer -- F1..12 + media controls + modifiers on right homerow
    [_function] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,  KC_F1,  KC_F2,  KC_F3,  KC_F4,  XX,       XX,  KC_MPRV,         KC_VOLU,         KC_BRIU,  KC_SCRL, __,
        __,  KC_F5,  KC_F6,  KC_F7,  KC_F8,  XX,       XX,  LALT_T(KC_MPLY), RCTL_T(KC_MUTE), KC_RGUI,  KC_PSCR, __,
        __,  KC_F9,  KC_F10, KC_F11, KC_F12, XX,       XX,  KC_MNXT,         KC_VOLD,         KC_BRID,  KC_INS,  __,
              __,  __,  QK_BOOT,
              QK_RBT,  __,  __
    ),
};

bool process_record_user(uint16_t keycode, keyrecord_t* record) {
    // Let QMK do its thing on key releases.
    if (!record->event.pressed) return true;

    switch (keycode) {
        case ODK_1: ODK1_SEQUENCE; return false;
        case ODK_2: ODK2_SEQUENCE; return false;
        case ODK_3: ODK3_SEQUENCE; return false;
        case ODK_4: ODK4_SEQUENCE; return false;
        case ODK_5: ODK5_SEQUENCE; return false;
    }

    return true;
}

static inline bool tap_keycode_used_in_text(uint16_t keycode) {
    // We can't make assumptions on custom keycodes
    if (keycode >= SAFE_RANGE) return false;

    // Remove "quantum" part of the keycode to get the action on tap.
    const uint16_t tap_keycode = keycode & 0xff;
    // `tap_keycode <= KC_0` includes all letters and numbers, but also
    // `KC_NO` which is safer to include, since it is commonly used in the
    // keymap as a placeholder for complex actions on tap.
    return (tap_keycode <= KC_0) || (tap_keycode == KC_SPACE);
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    return tap_keycode_used_in_text(keycode) ? ARSENIK_HRM_TAPPING_TERM : TAPPING_TERM;
}

bool get_hold_on_other_key_press(uint16_t keycode, keyrecord_t *record) {
    return !tap_keycode_used_in_text(keycode);
}

bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        // Keycodes that continue Caps Word, with shift applied.
        case KC_A:
        case KC_B:
        case KC_D ... KC_M:
        case KC_P ... KC_Z:
        case KC_SCLN:
        case KC_COMM:
        case KC_SLSH:
            add_weak_mods(MOD_BIT(KC_LSFT));  // Apply shift to next key.
            return true;

        // Keycodes that continue Caps Word, without shifting.
        case KC_1 ... KC_0:
        case KC_C:
        case KC_O:
        case KC_BSPC:
        case KC_DEL:
        case KC_RIGHT:
        case KC_LEFT:
        case KC_UNDS:
            return true;

        default:
            return false;  // Deactivate Caps Word.
    }
}
