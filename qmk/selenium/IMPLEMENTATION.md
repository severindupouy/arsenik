# Selenium QMK Implementation

This is a QMK implementation of [Selenium](https://github.com/OneDeadKey/selenium), based on the [ZMK reference implementation](https://github.com/Nuclear-Squid/zmk-keyboard-quacken/).

QMK and ZMK don't offer the same features. This document explains how the conversion was handled.

## Timing

ZMK Selenium uses 3 distinct timing values. All 3 are preserved exactly in QMK.

| Parameter            | Value | Purpose                                         | QMK mapping                           |
| -------------------- | ----- | ----------------------------------------------- | ------------------------------------- |
| `SHORT_TAPPING_TERM` | 150ms | Hold-preferred keys (Enter, Escape, layer taps) | `TAPPING_TERM = 150` (global default) |
| `HRM_TAPPING_TERM`   | 300ms | Tap-preferred keys (HRM, spacebar)              | `get_tapping_term()` returns 300ms    |
| `QUICK_TAP`          | 200ms | Repeated tap threshold for tap-hold keys        | `get_quick_tap_term()` returns 200ms  |

### How it works

QMK only has a single global `TAPPING_TERM`. To replicate ZMK's per-behavior timing:

- `TAPPING_TERM = 150` is set as the global default (short, for hold-preferred keys).
- `TAPPING_TERM_PER_KEY` is enabled, and `get_tapping_term()` returns `ARSENIK_HRM_TAPPING_TERM` (300ms) for text-producing keys (letters, numbers, space) and `TAPPING_TERM` (150ms) for everything else.
- For `QUICK_TAP`: QMK's `action_tapping.h` unconditionally redefines `QUICK_TAP_TERM = TAPPING_TERM` unless `QUICK_TAP_TERM_PER_KEY` is defined. We define `QUICK_TAP_TERM_PER_KEY` and provide `get_quick_tap_term()` returning 200ms.

A single helper function `tap_keycode_used_in_text()` drives both `get_tapping_term()` and `get_hold_on_other_key_press()`, cleanly splitting behavior based on whether a key produces text.

## Hold-tap behavior

ZMK uses named behaviors (`sc`, `lt`) with different hold/tap preferences. QMK achieves the same with per-key callbacks:

- **ZMK `sc` (hold-preferred)**: keys like Enter, Escape, layer taps. Mapped via `get_hold_on_other_key_press()` returning `true` for non-text keys — the hold action triggers immediately when another key is pressed.
- **ZMK `lt` (tap-preferred)**: keys like spacebar, HRM. QMK's default behavior (tap-preferred) applies to text-producing keys since `get_hold_on_other_key_press()` returns `false`.

## ZMK features without exact QMK equivalent

Some ZMK behaviors have no direct QMK counterpart. These were approximated:

| ZMK behavior                                   | QMK approximation       | What's lost                                                                                                                                                                |
| ---------------------------------------------- | ----------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `EZ_SK(LSHIFT)` (sticky key hold-tap)          | `OSM(MOD_LSFT)`         | — (equivalent: QMK's `OSM()` is one-shot on tap and continuous modifier on hold) |
| `sym_shift_altgr` (shift→AltGr morph)          | `OSL(_symbols)`         | Shift morph: tapping shift doesn't switch to AltGr                                                                                                                         |
| `EZ_SL` (hold=momentary, tap=one-shot layer)   | `OSL()`                 | — (equivalent: QMK's `OSL()` is one-shot on tap and momentary on hold)                                                                                                     |
| `magic_backspace` / `magic_space` (mod-morphs) | Not implemented         | See [Mod-morph decision](#mod-morph-magic_backspacemagic_space) below                                                                                                      |
| `&lt FUNCTION LS(SPACE)`                       | Not implemented          | See [Insecable space](#insecable-space) below. |
| `&sl { ignore-modifiers; }`                    | Default QMK behavior     | — (equivalent: QMK's OSL natively preserves modifiers when chaining OSM→OSL)                                                                                               |
| `&sk { quick-release; }`                       | Similar QMK default      | Minor timing difference: QMK releases OSM on next key release, ZMK quick-release on next key press. Negligible in practice. |

## Mod-morph (magic_backspace/magic_space)

In ZMK, `magic_space` and `magic_backspace` are mod-morphs that swap Space ↔ Backspace when Shift is held. The morph is bound to a **specific thumb key** (the secondary thumb — the one opposite the main space key), not globally.

This feature is **not implemented** in the QMK version. Two approaches were explored:

### QMK Key Override (`KEY_OVERRIDE`)

QMK's `KEY_OVERRIDE` feature can swap keycodes when a modifier is held:

```c
const key_override_t shift_space = ko_make_basic(MOD_MASK_SHIFT, KC_SPC, KC_BSPC);
```

**Problem**: Key Override is global per keycode — it applies everywhere `KC_SPC` appears, not just on the secondary thumb. This would also morph `KC_BSPC` on the top row of the base layer and `KC_SPC`/`KC_BSPC` in other layers, which ZMK does not do.

### Custom logic in `process_record_user`

Intercepting the keycode and checking modifiers manually:

```c
if (tap_kc == KC_SPC && record->tap.count > 0 && (get_mods() & MOD_MASK_SHIFT)) {
    del_mods(MOD_MASK_SHIFT);
    tap_code(KC_BSPC);
    return false;
}
```

**Problem**: Still global per keycode — can't distinguish which physical key was pressed without hardcoding row/column positions, which breaks portability across keyboards.

### Decision

Both approaches affect all instances of the keycode, not just the secondary thumb. In ZMK, mod-morphs are bound to specific key positions via behavior bindings — a concept that doesn't exist in QMK. Since there is no clean way to replicate position-specific mod-morphs, this feature is intentionally left out rather than implementing a global workaround with unintended side effects.

## sym_shift_altgr (Shift→AltGr morph)

In ZMK, `sym_shift_altgr` is a nested mod-morph used as the right tuck thumb key (HT_THUMB_TAPS and HT_HOME_ROW_MODS variants):
- Default: `EZ_SL(SYMBOLS_LAYER)` — one-shot/momentary symbols layer
- When Shift is held: `EZ_SK(RALT)` — one-shot/hold AltGr (for typing accented characters via Ergol's AltGr layer)
- When Ctrl/GUI/Alt is held: passes through to the symbols layer

The purpose: on Ergol, this lets you tap Shift (left tuck) then tap the symbol key (right tuck) to get AltGr instead of the symbol layer, giving access to accented characters.

This feature is **not implemented** in the QMK version. The right tuck thumb uses `OSL(_symbols)` unconditionally.

### Why not implement it

QMK has no mod-morph behavior. The closest approach would be to intercept the `OSL(_symbols)` keypress in `process_record_user`, check if Shift is held via `get_mods()`, and send `OSM(MOD_RALT)` instead. However, `OSL()` is not easily interceptable as a custom keycode — it would require replacing it with a custom keycode and reimplementing both the one-shot layer and the shift detection manually.

### Decision

The added complexity is not justified. Ergol users can still access AltGr via the dedicated `KC_RALT` key available on several layers. The shift→AltGr shortcut is a convenience, not a necessity.

## Insecable space

In ZMK, the right thumb home key on the NumLock and NumNav layers uses `&lt LAYER LS(SPACE)` — hold activates a layer, tap sends Shift+Space. On Ergol, Shift+Space produces a non-breaking space (espace insécable).

This feature is **not implemented** in the QMK version.

QMK's `LT()` only accepts basic keycodes, so `LT(layer, S(KC_SPC))` is not possible. An initial approach intercepted taps in `process_record_user` by checking if the keycode matched `LT(_num_nav, KC_SPC)` or `LT(_function, KC_SPC)`. However, the base layer space key also uses `LT(_num_nav, KC_SPC)` — making it impossible to distinguish a base layer space tap from a NumLock/NumNav layer space tap. This caused every space on the base layer to send Shift+Space (non-breaking space), breaking normal typing.

## SYM_NUM_LAYER (number access from symbols layer)

In ZMK, `SYM_NUM_LAYER` uses `EZ_SL(NUM_LAYER)` — a hold-tap where tap activates the number layer as one-shot (type one number, return to symbols) and hold keeps it active momentarily.

In QMK, we use `OSL(_num_nav)` which provides the same tap=one-shot / hold=momentary behavior.

**Exception**: for `HT_TWO_THUMB_KEYS` without `VIM_NAVIGATION`, the ZMK version uses `&sc NUM_NAV_LAYER CAPSLOCK` — a hold-preferred hold-tap with CapsLock on tap. In QMK, we use `LT(_num_nav, KC_CAPS)` which approximates this but uses QMK's tap-preferred default (since `LT()` doesn't support hold-preferred). The one-shot behavior from `OSL()` is lost in this specific variant because `LT()` sends the tap keycode (CapsLock) instead of activating a one-shot layer.

## Configurable options

All options from the ZMK implementation are available in `config.h`:

- **Hold-tap configs**: `HT_NONE`, `HT_THUMB_TAPS`, `HT_HOME_ROW_MODS` (default), `HT_TWO_THUMB_KEYS`
- **VIM_NAVIGATION**: splits num-nav into vim-style navigation + number row layers
- **HRM_SHIFT**: adds shift as a pinky home-row mod
- **LEFT_HAND_SPACE**: swaps space and backspace on thumbs
- **Timing overrides**: `HRM_TAPPING_TERM`, `SHORT_TAPPING_TERM`, `QUICK_TAP`
