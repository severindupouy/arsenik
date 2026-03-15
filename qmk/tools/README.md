# Keymap Linter

`keymap_lint.py` checks and fixes the formatting of `ONEDEADKEY_LAYOUT()` layer blocks in QMK `keymap.c` files.

## Usage

```sh
# Check formatting (reports issues, exits non-zero if any)
python3 keymap_lint.py check path/to/keymap.c

# Fix formatting in place
python3 keymap_lint.py fix path/to/keymap.c

# Show what would change without modifying the file
python3 keymap_lint.py diff path/to/keymap.c
```

## Layer shape

Each layer block must contain exactly **54 keys**, organized as:

- **4 main rows** of 12 keys each (48 keys)
- **1 thumb row** of 6 keys (2 pads of 3)

If the key count doesn't match, the linter reports an error. If the keys are spread across the wrong number of lines, the linter redistributes them into the correct shape.

## Formatting rules

### Main rows

The 12 keys per row are split into **2 pads of 6 keys**, separated by a gap.

- Each column is aligned to the widest key in that column across all 4 rows
- At least 2 spaces after each comma within a pad
- At least 8 spaces between the left and right pads
- The right pad starts at the same column on every row
- 8-space indentation

### Thumb row

The 6 thumb keys are on a single line, split into 2 pads of 3 keys.

- An empty line separates the thumb row from the main rows above
- The left pad is **right-aligned** to the left pad column
- The right pad is **left-aligned** to the right pad column (same starting position as the main rows' right pad)
- At least 2 spaces after each comma

### Example

Before:
```c
    [_base] = ONEDEADKEY_LAYOUT(
        __, __, __, __, __, __, __, __, __, __, __, __,
        KC_TAB, KC_Q, KC_W, KC_E, KC_R, KC_T, KC_Y, KC_U, KC_I, KC_O, KC_P, KC_BSPC,
        KC_ESC, KC_A, KC_S, KC_D, KC_F, KC_G, KC_H, KC_J, KC_K, KC_L, KC_SCLN, KC_ENT,
        KC_LSFT, KC_Z, KC_X, KC_C, KC_V, KC_B, KC_N, KC_M, KC_COMM, KC_DOT, KC_SLSH, KC_RSFT,
              KC_LALT, KC_SPC, KC_LGUI,
              KC_ENT, KC_BSPC, KC_RALT
    ),
```

After `keymap_lint.py fix`:
```c
    [_base] = ONEDEADKEY_LAYOUT(
        __,       __,    __,    __,    __,    __,          __,    __,    __,      __,      __,      __,
        KC_TAB,   KC_Q,  KC_W,  KC_E,  KC_R,  KC_T,        KC_Y,  KC_U,  KC_I,    KC_O,    KC_P,    KC_BSPC,
        KC_ESC,   KC_A,  KC_S,  KC_D,  KC_F,  KC_G,        KC_H,  KC_J,  KC_K,    KC_L,    KC_SCLN, KC_ENT,
        KC_LSFT,  KC_Z,  KC_X,  KC_C,  KC_V,  KC_B,        KC_N,  KC_M,  KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,

                         KC_LALT,  KC_SPC,  KC_LGUI,        KC_ENT,  KC_BSPC,  KC_RALT
    ),
```

## Constants

All formatting values are defined as constants at the top of `keymap_lint.py`:

| Constant | Default | Description |
|----------|---------|-------------|
| `LAYOUT_IDENTIFIER` | `"ONEDEADKEY_LAYOUT"` | Pattern used to detect layer blocks |
| `COLS_PER_PAD` | `6` | Keys per pad |
| `NUM_PADS` | `2` | Pads per row |
| `MAIN_ROWS` | `4` | Main key rows |
| `THUMB_ROWS` | `1` | Thumb cluster rows |
| `THUMB_COLS` | `6` | Thumb keys (3 left + 3 right) |
| `PAD_GAP` | `8` | Minimum spaces between pads |
| `MIN_COL_SPACING` | `2` | Minimum spaces after comma within a pad |
| `THUMB_SPACING` | `2` | Minimum spaces after comma in thumb row |
| `ROW_INDENT` | 8 spaces | Indentation for main rows |
