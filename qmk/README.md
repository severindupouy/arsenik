# QMK

This repository provides multiple QMK keymap targets sharing a common set of layout definitions and keycodes.

## Structure

```
qmk/
  shared/          — Shared keycodes, layout macros, and host layout files
  arsenik/         — Arsenik keymap (default)
  selenium/        — Selenium keymap (default)
  generator.sh     — Script to generate and install a keymap for your keyboard
```

## How it works

Each keymap target defines its layers and customization options in its own folder.
At compile time, a `ONEDEADKEY_LAYOUT` macro adapts the keymap to your keyboard's
actual layout, automatically handling different keyboard sizes and shapes.

If your keyboard is not yet supported, you can add its layout to `shared/layouts.h`
or open an issue/PR for help.

**Supported layouts:**

- `LAYOUT_split_3x5_2`
- `LAYOUT_split_3x5_3`
- `LAYOUT_split_3x6_3`
- `LAYOUT_ortho_4x10`
- `LAYOUT_ortho_4x12`
- `LAYOUT_ortho_5x10`
- `LAYOUT_ortho_5x12`
- `LAYOUT_planck_grid`
- `LAYOUT_keebio_iris`

## Quick Start

### 1. Install & Set Up QMK

Follow the [QMK Getting Started Guide](https://docs.qmk.fm/newbs_getting_started) to:

- Install the `qmk` CLI
- Run `qmk setup` (clones the QMK repo)
- Verify you can compile a default firmware

### 2. Identify Your Keyboard

Find your keyboard's QMK name (usually `brand/model/revision`).

```sh
qmk list-keyboards | grep <your keyboard>
qmk info -kb <keyboard_model>
```

### 3. Configure QMK User Defaults

```sh
qmk config user.qmk_home=<path/to/qmk_firmware>
qmk config user.keyboard=<keyboard_model>
qmk config user.keymap=default
```

### 4. Clone and Generate

```sh
git clone https://github.com/OneDeadKey/arsenik.git
cd arsenik/qmk

# Generate the arsenik keymap (default target)
./generator.sh -target arsenik --generate --copy

# Or generate the selenium keymap
./generator.sh -target selenium --generate --copy
```

- `--generate` (`-g`): Creates the keymap in `output/<keyboard>/keymaps/<target>`
- `--copy` (`-cp`): Installs it to your QMK home directory
- `-target`: Choose which keymap to generate (`arsenik` or `selenium`)

### 5. Build and Flash

```sh
qmk compile -kb <keyboard_model> -km arsenik    # or -km selenium
qmk flash -kb <keyboard_model> -km arsenik      # connect keyboard first
```

### 6. Customize

Edit `config.h` in your target's folder to adjust options, then re-run:

```sh
./generator.sh -target <target> --generate --copy
```

---

**Need help or want to contribute?**

- Open an issue or PR if your layout is missing or you encounter problems.
