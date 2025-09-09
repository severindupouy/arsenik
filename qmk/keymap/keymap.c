#include QMK_KEYBOARD_H
#include "arsenik.h"

enum arsenik_layers {
    _base,
    _lafayette,
    _num_layer,
    _nav_layer,
    _media_layer,
    _mouse_layer,
};

enum custom_keycodes {
    ODK_1 = SAFE_RANGE,  // „
    ODK_2,  // “
    ODK_3,  // ”
    ODK_4,  // ¢
    ODK_5,  // ‰
};

// The ARSENIK_LAYOUT macro allows us to declare a config for a 4x6+3 keyboard, then truncate it
// (or fill it with noops) depending on the size of your keyboard. Your keyboard may have extra
// definitions for this macro or none at all (preventing you from compiling the keymap). Check
// the `README.md` file for more information.
//
// A comprehensive list of QMK keycodes is available here: https://docs.qmk.fm/keycodes
// However, we used a many aliases to automatically adapt the keymap depending on the options you
// enabled in the `config.h` file (or just to have some syntaxic sugar). You can find all of them
// in the `arsenik.h` file. Feel free to remove those aliases and replace them with their actual
// value if you need something Arsenik doesn’t provide.
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_base] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        C(KC_K),     KC_Q, KC_W,  KC_E,  KC_R,  KC_T,      KC_Y, KC_U,  KC_I,    KC_O,   KC_P,    CW_TOGG,
        KC_ESC,      KC_A, KC_SS, KC_DD, KC_FF, KC_G,      KC_H, KC_JJ, KC_KK,   KC_LL,  KC_SCLN, KC_ESC,
        C(KC_SLSH),  KC_Z, KC_X,  KC_C,  KC_V,  KC_B,      KC_N, KC_M,  KC_COMM, KC_DOT, KC_SLSH, C(KC_SLSH),
              AS_TL_TUCK,  AS_TL_HOME,  AS_TL_REACH,        AS_TR_REACH,  AS_TR_HOME,  AS_TR_TUCK
    ),

    [_lafayette] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,             AS(CIRC), AS(LABK), AS(RABK), AS(DLR),  AS(PERC),      AS(AT),   AS(AMPR), AS(ASTR), AS(QUOT), AS(GRV),  KC_CAPS,
        __,             AS(LCBR), AS(LPRN), AS(RPRN), AS(RCBR), AS(EQL),       AS(BSLS), AS(PLUS), AS(MINS), AS(SLSH), AS(DQUO), __,
        QK_LAYER_LOCK,  AS(TILD), AS(LBRC), AS(RBRC), AS(UNDS), AS(HASH),      AS(PIPE), AS(EXLM), AS(SCLN), AS(COLN), AS(QUES), __,
                                        MO(_num_layer),   KC_SPC,   __,        __,   KC_SPC,   MO(_num_layer)
    ),

    [_num_layer] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        KC_BSPC,       AS_S1, AS_S2, AS_S3, AS_S4, AS_S5,      AS_S6,   AS_S7,    AS_S8,    AS_S9,    AS_S0,    KC_BSPC,
        __,            AS(1), AS(2), AS(3), AS(4), AS(5),      AS(6),   AS(7),    AS(8),    AS(9),    AS(0),    __,
        QK_LAYER_LOCK, ODK_1, ODK_2, ODK_3, ODK_4, AS(COMM),   AS(DOT), AS(MINS), AS(PLUS), AS(ASTR), AS(SLSH), __,
                                    __,   KC_SPC,   __,        __,   KC_SPC,   LAFAYETTE
    ),

    [_nav_layer] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,             G(S(KC_Q)),   KC_HOME, KC_UP,   KC_END,   KC_PGUP,      XX,  KC_F1,  KC_F2,   KC_F3,   KC_F4,   __,
        __,             KC_TAB,       KC_LEFT, KC_DOWN, KC_RGHT,  KC_PGDN,      XX,  KC_F5,  KC_F6,   KC_F7,   KC_F8,   __,
        QK_LAYER_LOCK,  KC_WH_L,      KC_WH_D, KC_WH_U, KC_WH_R,  S(KC_TAB),    XX,  KC_F9,  KC_F10,  KC_F11,  KC_F12,  __,
                                        KC_DEL,   __,   __,                     __,   __,   KC_ESC
    ),

    [_media_layer] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        TG(_mouse_layer),  S(KC_LEFT),  S(KC_RGHT),  KC_F2,    KC_KB_MUTE,  KC_MEDIA_PLAY_PAUSE,      XX, KC_BRIGHTNESS_DOWN,  KC_BRIGHTNESS_UP,    XX, KC_PRINT_SCREEN, __,
        __,                KC_WBAK,     KC_WFWD,     AS(COLN), KC_F5,       KC_KB_VOLUME_UP,          XX, KC_MEDIA_PREV_TRACK, KC_MEDIA_NEXT_TRACK, XX, XX,              __,
        QK_LAYER_LOCK,     KC_UNDO,     KC_CUT,      KC_COPY,  KC_PASTE,    KC_KB_VOLUME_DOWN,        XX, XX,                  XX,                  XX, XX,              __,
                                                    LAFAYETTE,  KC_SPC,   __,                          __,   KC_SPC,   LAFAYETTE
    ),

    [_mouse_layer] = ARSENIK_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        __,             MS_BTN3,  MS_BTN1,  MS_UP,    MS_BTN2,  MS_BTN4,    XX, XX, XX, XX, XX, __,
        __,             MS_BTN5,  MS_LEFT,  MS_DOWN,  MS_RGHT,  MS_BTN6,    XX, XX, XX, XX, XX, __,
        QK_LAYER_LOCK,  MS_BTN7,  XX,       XX,       XX,       MS_BTN8,    XX, XX, XX, XX, XX, __,
                                                __,  __,  __,          __,  __,  __
    )
};

// This is where you’ll write most of your custom code for your keyborad.
// This callback is called right before the keycode is sent to the OS.
//
// returning false cancels any furnther processing.
// for instance, calling `tap_code(KC_B)` if KC_A is pressed but true is
// returned, "ba" is sent, but if `false` is returned, it’s just "b"
bool process_record_user(uint16_t keycode, keyrecord_t* record) {
#   ifdef SELENIUM_RESTORE_SPACE
    static bool thumb_mod_same_hand_as_space_held = false;
    if ((keycode & 0xff) == KC_SPC && record->tap.count == 0)
        thumb_mod_same_hand_as_space_held = record->event.pressed;
#   endif

    // Let QMK do its thing on key releases.
    if (!record->event.pressed) return true;

#   ifdef SELENIUM_RESTORE_SPACE
    if ((keycode & 0xff) == KC_BSPC &&
        !thumb_mod_same_hand_as_space_held &&
        record->tap.count > 0
    ) {
        tap_code(KC_SPC);
        return false;
    }
#   endif

    switch (keycode) {
        // ----------------------------------------
        // Code for your custom keycodes goes here.
        // ----------------------------------------

        case ODK_1: ODK1_SEQUENCE; return false;
        case ODK_2: ODK2_SEQUENCE; return false;
        case ODK_3: ODK3_SEQUENCE; return false;
        case ODK_4: ODK4_SEQUENCE; return false;
        case ODK_5: ODK5_SEQUENCE; return false;
    }

    return true;
}

static inline bool tap_keycode_used_in_text(uint16_t keycode) {
    // We can’t make assumptions on curstom keycodes
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