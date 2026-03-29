// Selenium keymap for QMK — converted from the ZMK Selenium keymap
// https://github.com/OneDeadKey/selenium

/******************************************************************************
 * Layout-specific actions and symbols.
 ******************************************************************************/

// Uncomment one of the following lines if using a non-QWERTY layout:

// #define KB_LAYOUT_QWERTY_LAFAYETTE
// #define KB_LAYOUT_AZERTY
#define KB_LAYOUT_ERGOL

// Uncomment the following line if using a Mac:

// #define MACOS

/******************************************************************************
 * Hold-Taps
 ******************************************************************************/

// Uncomment one of the following lines to pick your preferred hold-tap config:

// #define HT_NONE
// #define HT_THUMB_TAPS
// #define HT_HOME_ROW_MODS  // (default behavior)
// #define HT_TWO_THUMB_KEYS

// Timing is key! Keep the default value if unsure.
// This defines how long (in ms) a hold-tap key with the "tap-preferred" flavor
// must be held to be considered as a modifier or layer shift. These hold-taps
// are the ones that may produce text, including the space-bar and home-row-mods.
// Keep this value high if you struggle with home-row-mods.

// #define HRM_TAPPING_TERM 300

// This defines how quickly (in ms) you need to press and release a hold-tap
// with the "hold-preferred" flavor for it to be considered a tap. These
// hold-taps include the ones that don't produce text when tapped.
// Keep this value low if you press Enter or Escape by mistake too often.

// #define SHORT_TAPPING_TERM 150

// When tapping then holding the same tap-hold, if the delay between the two
// key presses is lower than `QUICK_TAP`, the tap-hold will hold the tap action
// instead of the hold action. Again, keep the default value if unsure.

// #define QUICK_TAP 200

// For more information on how hold-taps operate, feel free to read ZMK's docs:
// https://zmk.dev/docs/keymaps/behaviors/hold-tap

/******************************************************************************
 * Other Options
 ******************************************************************************/

// Uncomment the following line to split the num-nav layer in two:
//  - one layer for vim-style navigation (right) and GUI shortcuts (left)
//  - one layer for a full number row + easy access to Shift+Number
//  + Escape under the left thumb (direct access)
// Highly recommended for Vim users, obviously. :-)

// #define VIM_NAVIGATION

// Uncomment the following line to enable shift as a pinky HRM.
// Useful for combined shortcuts, but NOT MEANT to type text!

// #define HRM_SHIFT

// Uncomment the following line to swap Space and Backspace.
// Beware: this increases the typing load of the left thumb.

// #define LEFT_HAND_SPACE

/******************************************************************************
 * QMK-specific settings
 ******************************************************************************/

#define ONEDEADKEY_PLACEHOLDER_LAYOUT
/* NOTE: This line gets automatically filled in by the install script, but the
 * underlying layout may not yet exist, and keyboard specific layout may have
 * other variants you might want to check out.
 */

//  ── Mapping to QMK/Arsenik internals (usually no need to change below) ──

// Host layout
#if defined KB_LAYOUT_ERGOL
#    define ARSENIK_HOST_LAYOUT_ERGOL
#elif defined KB_LAYOUT_AZERTY
#    define ARSENIK_HOST_LAYOUT_AZERTY
#elif defined KB_LAYOUT_QWERTY_LAFAYETTE
#    define ARSENIK_HOST_LAYOUT_QWERTY
#else
#    define ARSENIK_HOST_LAYOUT_QWERTY
#endif

// Mac modifier swap
#ifdef MACOS
#    define ARSENIK_MAC_MODIFIERS
#endif

// Timing defaults and QMK mapping
#ifndef HRM_TAPPING_TERM
#    define HRM_TAPPING_TERM 300
#endif

#ifndef SHORT_TAPPING_TERM
#    define SHORT_TAPPING_TERM 150
#endif
#define TAPPING_TERM SHORT_TAPPING_TERM

#ifndef QUICK_TAP
#    define QUICK_TAP 200
#endif
// QMK's action_tapping.h unconditionally sets QUICK_TAP_TERM = TAPPING_TERM
// unless QUICK_TAP_TERM_PER_KEY is defined. We use that to set our own value.
#define QUICK_TAP_TERM_PER_KEY

// QMK per-key hold-tap settings
#define PERMISSIVE_HOLD
#define HOLD_ON_OTHER_KEY_PRESS_PER_KEY
#define TAPPING_TERM_PER_KEY
