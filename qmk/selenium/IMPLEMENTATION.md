# Selenium QMK Implementation

This is a QMK implementation of [Selenium](https://github.com/OneDeadKey/selenium), based on the [ZMK source of truth](https://github.com/OneDeadKey/zmk-config-selenium).

QMK and ZMK don't offer the same features. This document explains how the conversion was handled.

## Timing

### ZMK model: per-behavior timing

ZMK assigns timing and flavor to each behavior definition independently:

| Behavior | Flavor | `tapping-term-ms` | `quick-tap-ms` | Used by |
| -------- | -------------- | ------------------ | --------------- | ------- |
| `&hrm`   | tap-preferred  | 300ms (`TAPPING_TERM`) | 200ms (`QUICK_TAP`) | HRM keys (S/D/F/J/K/L), function layer media HRMs |
| `&sc`    | hold-preferred | 150ms (`SHORT_TAPPING_TERM`) | 200ms (`QUICK_TAP`) | Thumb keys that don't produce text (Enter, Escape, Del, layer taps) |
| `&lt`    | tap-preferred  | 300ms (`TAPPING_TERM`) | 200ms (`QUICK_TAP`) | Space thumb, NavNum/NumLock layer+space |
| `&mt`    | hold-preferred | 150ms (`SHORT_TAPPING_TERM`) | 200ms (`QUICK_TAP`) | LCTL+Backspace thumb (HT_THUMB_TAPS) |
| `bsl` / `bsk` / `lbsk` | tap-preferred | 300ms (`TAPPING_TERM`) | — | EZ_SL, EZ_SK, EZ_LSK wrappers |

### QMK model: single global + per-key callbacks

QMK has one global `TAPPING_TERM` plus per-key callback functions:

| Setting | Value | How |
| ------- | ----- | --- |
| `TAPPING_TERM` | 150ms | Global default (set in `config.h` as `SHORT_TAPPING_TERM`) |
| `ARSENIK_HRM_TAPPING_TERM` | 300ms | Returned by `get_tapping_term()` for tap-preferred keys |
| `QUICK_TAP` | 200ms | Returned by `get_quick_tap_term()` for all keys |

The split is driven by `tap_keycode_is_tap_preferred()`, which returns `true` for keycodes that should use ZMK's `&hrm`/`&lt` behavior:

- Letters, numbers, `KC_NO`, `KC_SPACE` — text-producing keys on base layer HRMs and Space thumb
- `KC_MPLY`, `KC_MUTE`, `KC_PSCR` — media/system keys used as HRM taps on the function layer (ZMK uses `&hrm` for these)

The result:

- **Tap-preferred keys** → `get_tapping_term()` returns 300ms, `get_hold_on_other_key_press()` returns `false`
- **Hold-preferred keys** → `get_tapping_term()` returns 150ms, `get_hold_on_other_key_press()` returns `true`

### How the mapping works

- `TAPPING_TERM = 150` is set as the global default (short, for hold-preferred keys).
- `TAPPING_TERM_PER_KEY` is enabled, and `get_tapping_term()` returns `ARSENIK_HRM_TAPPING_TERM` (300ms) for text-producing keys and `TAPPING_TERM` (150ms) for everything else.
- For `QUICK_TAP`: QMK's `action_tapping.h` unconditionally redefines `QUICK_TAP_TERM = TAPPING_TERM` unless `QUICK_TAP_TERM_PER_KEY` is defined. We define `QUICK_TAP_TERM_PER_KEY` and provide `get_quick_tap_term()` returning 200ms.

| ZMK | QMK | Match |
| --- | --- | ----- |
| `&hrm` 300ms tap-preferred | `*_T()` on letter → 300ms, tap-preferred | Exact |
| `&lt` 300ms tap-preferred | `LT()` on Space → 300ms, tap-preferred | Exact |
| `&sc` 150ms hold-preferred | `LT()` on Enter/Escape/Del → 150ms, `hold_on_other_key_press=true` | Approximate (see below) |
| `&mt` 150ms hold-preferred | `*_T()` on Backspace → 150ms, `hold_on_other_key_press=true` | Approximate (see below) |

## Hold-tap behavior

ZMK uses named behaviors (`sc`, `lt`, `hrm`) with different hold/tap flavors. QMK achieves the same split with per-key callbacks:

- **ZMK `sc` / `mt` (hold-preferred)**: keys like Enter, Escape, layer taps. Mapped via `get_hold_on_other_key_press()` returning `true` for non-text keys.
- **ZMK `lt` / `hrm` (tap-preferred)**: keys like spacebar, HRM. QMK's default behavior (tap-preferred) applies to text-producing keys since `get_hold_on_other_key_press()` returns `false`.

### Remaining gap: hold-preferred vs hold_on_other_key_press

ZMK's hold-preferred flavor: if the key is still held when `tapping-term-ms` expires, it's a hold. Tap only registers if the key is released before the tapping term. Other key presses during the hold do not influence the decision — only the timer matters.

QMK's `hold_on_other_key_press=true`: if **any other key is pressed** while this key is held, immediately trigger hold — regardless of the tapping term. If no other key is pressed, the tapping term still applies normally.

The QMK version triggers hold *faster* (on any key down) while ZMK waits for the timing threshold.

#### Explored option: exact ZMK hold-preferred in QMK

QMK's **default** behavior (no `HOLD_ON_OTHER_KEY_PRESS`, no `PERMISSIVE_HOLD`) is actually an exact match for ZMK's hold-preferred — hold triggers purely on tapping term expiry. To get there, we would need to:

1. Remove `HOLD_ON_OTHER_KEY_PRESS_PER_KEY` (or make `get_hold_on_other_key_press()` return `false` for all keys)
2. Replace global `PERMISSIVE_HOLD` with `PERMISSIVE_HOLD_PER_KEY`
3. Add `get_permissive_hold()` returning `true` only for text-producing keys (HRMs), `false` for non-text keys

This would give non-text keys pure tapping-term behavior (exact ZMK hold-preferred) while keeping `PERMISSIVE_HOLD` for HRMs.

#### Decision

Not implemented. With a 150ms tapping term on non-text keys, the difference is near-impossible to trigger in practice: you almost always press another key while holding a layer-tap, and 150ms expires almost immediately. The edge case (holding a layer-tap alone for 50-149ms while another key happens to be pressed at the same moment) is extremely unlikely in real typing. The added complexity (`PERMISSIVE_HOLD_PER_KEY` + a new callback function) is not justified.

## ZMK features without exact QMK equivalent

Some ZMK behaviors have no direct QMK counterpart. These were approximated:

| ZMK behavior                                   | QMK approximation       | What's lost                                                                                                                                                                |
| ---------------------------------------------- | ----------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `EZ_SK(LSHIFT)` (sticky key hold-tap)          | `OSM(MOD_LSFT)`         | — (equivalent: QMK's `OSM()` is one-shot on tap and continuous modifier on hold) |
| `sym_shift_altgr` (shift→AltGr morph)          | `OSL(_symbols)`         | Shift morph: tapping shift doesn't switch to AltGr                                                                                                                         |
| `EZ_SL` (hold=momentary, tap=one-shot layer)   | `OSL()`                 | — (equivalent: QMK's `OSL()` is one-shot on tap and momentary on hold)                                                                                                     |
| `EZ_LSK(RALT)` (sticky key on base layer)      | `LSK_RALT` custom keycode | — (equivalent: see [EZ_LSK(RALT)](#ez_lskralt-sticky-altgr-on-base-layer) below)                                                                                          |
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

## EZ_LSK(RALT) (sticky AltGr on base layer)

In ZMK, `EZ_LSK(RALT)` is used on the VimNav and NavNum layers (RT tuck). It expands to `lbsk RALT RALT`, a hold-tap that:

- **Tap** (`losm`): goes to base layer via `&to BASE_LAYER`, then sends one-shot RALT
- **Hold** (`lkp`): goes to base layer via `&to BASE_LAYER`, then holds RALT until released

The purpose is to exit the navigation layer and apply AltGr on the base layer, where letter keys are available for typing accented characters (e.g. on Ergol).

In QMK, this is implemented via a custom keycode `LSK_RALT` with manual tap/hold detection in `process_record_user`:

- On press: `layer_move(_base)` + `register_mods(MOD_BIT(KC_RALT))`
- On release: `unregister_mods(MOD_BIT(KC_RALT))`, and if no other key was pressed during the hold, `set_oneshot_mods(MOD_BIT(KC_RALT))` for one-shot behavior

A static flag (`lsk_ralt_used`) tracks whether another key was pressed while LSK_RALT was held, distinguishing tap from hold.

## Function layer: C_AL_LOCK (screen lock)

In ZMK, position (2,9) on the function layer uses `&hrm RGUI C_AL_LOCK` — a home-row mod with RGUI on hold and AL_LOCK (HID consumer usage 0x19E, screen lock/screensaver) on tap.

This tap action is **not implemented** in the QMK version. QMK defines the HID constant `AL_LOCK = 0x19E` internally but does not map it to any keycode. Implementing it would require a custom keycode with `host_consumer_send(0x19E)`, which cannot be combined with `RGUI_T()` (only accepts basic keycodes). The purpose of a dedicated screen lock key is not meaningful enough to justify the complexity — most users lock their screen via OS shortcuts. The position uses plain `KC_RGUI`.

## SYM_NUM_LAYER (number access from symbols layer)

In ZMK, `SYM_NUM_LAYER` uses `EZ_SL(NUM_LAYER)` — a hold-tap where tap activates the number layer as one-shot (type one number, return to symbols) and hold keeps it active momentarily.

In QMK, we use `OSL(_num_nav)` which provides the same tap=one-shot / hold=momentary behavior.

**Exception**: for `HT_TWO_THUMB_KEYS`, the ZMK version uses `&sc NUM_NAV_LAYER CAPSLOCK` — a hold-preferred hold-tap with CapsLock on tap. In QMK, we use `LT(_num_nav, KC_CAPS)` which approximates this but uses QMK's tap-preferred default (since `LT()` doesn't support hold-preferred). The one-shot behavior from `OSL()` is lost in this specific variant because `LT()` sends the tap keycode (CapsLock) instead of activating a one-shot layer.

## Configurable options

All options from the ZMK implementation are available in `config.h`:

- **Hold-tap configs**: `HT_NONE`, `HT_THUMB_TAPS`, `HT_HOME_ROW_MODS` (default), `HT_TWO_THUMB_KEYS`
- **VIM_NAVIGATION**: splits num-nav into vim-style navigation + number row layers
- **HRM_SHIFT**: adds shift as a pinky home-row mod
- **LEFT_HAND_SPACE**: swaps space and backspace on thumbs
- **Timing overrides**: `HRM_TAPPING_TERM`, `SHORT_TAPPING_TERM`, `QUICK_TAP`
