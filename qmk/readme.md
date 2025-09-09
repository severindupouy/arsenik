# Arsenik QMK

This repository provides:

- `/qmk/keymap`: Generic Arsenik layout files for QMK keyboards
- `/qmk/generator.sh`: Script to generate and install an Arsenik keymap for your keyboard

## Overview

Arsenik-QMK generates a complete QMK keymap using a dummy `ARSENIK_LAYOUT` definition. At compile time, this is replaced with your keyboard's actual layout, automatically adapting to different keyboard sizes and shapes. If your keyboard is not yet supported, you can add its layout to `arsenik.h` or open an issue/PR for help.

**Supported layouts:**

- `LAYOUT_split_3x5_2`
- `LAYOUT_split_3x5_3`
- `LAYOUT_split_3x6_3`
- `LAYOUT_ortho_4x10`
- `LAYOUT_ortho_4x12`
- `LAYOUT_ortho_5x10`
- `LAYOUT_ortho_5x12`
- `LAYOUT_planck_grid`
- `LAYOUT_keebio_iris_default`

## Quick Start

> **Note:** Beginners should follow this guide and use the default Arsenik configuration. Advanced users can customize their setup using Arsenik's features. See step 8 for details.

### 1. Install & Set Up QMK

Follow the [QMK Getting Started Guide](https://docs.qmk.fm/newbs_getting_started) to:

- Install the `qmk` CLI
- Run `qmk setup` (clones the QMK repo)
- Verify you can compile a default firmware

### 2. Identify Your Keyboard and Keymap

Find your keyboard's QMK name (usually `brand/model/revision`).

```sh
qmk list-keyboards                # List all supported keyboards
qmk list-keyboards | grep corne   # Filter for your model
qmk info -kb <keyboard_model>     # Show keyboard info
qmk list-keymaps -kb <keyboard_model>  # List available keymaps
```

### 3. Configure QMK User Defaults

Set your QMK home, keyboard, and keymap:

```sh
qmk config user.qmk_home=<path/to/qmk_firmware>
qmk config user.keyboard=<keyboard_model>
qmk config user.keymap=<keymap>
qmk config                        # Check your config
```

### 4. Clone the Arsenik Repository

```sh
git clone https://github.com/OneDeadKey/arsenik.git
cd arsenik/qmk
```

Or download: https://github.com/OneDeadKey/arsenik/archive/refs/heads/main.zip

### 5. Generate and Install the Arsenik Keymap

```sh
./generator.sh --generate --copy
```

- `--generate`: Creates the Arsenik keymap in `output/<keyboard_model>/keymaps/arsenik`
- `--copy`: Installs it to your QMK home directory

You can run these steps separately. The script will prompt before overwriting existing files.

### 6. Build the Firmware

```sh
qmk list-keymaps -kb <keyboard_model>   # Confirm 'arsenik' keymap is present
qmk compile -kb <keyboard_model> -km arsenik
```

### 7. Flash Your Keyboard

Connect your keyboard in bootloader/recovery mode:

```sh
qmk flash -kb <keyboard_model> -km arsenik
```

### 8. Advanced Configuration (Optional)

Want to go further? You can fully customize your Arsenik keymap and layout:

- Explore QMK firmware options and documentation for advanced features.
- Review the Arsenik philosophy and available configuration options in the `/qmk/keymap` files and the Arsenik documentation.
- Edit `config.h` or other keymap files in `/qmk/keymap` to adjust layers, key assignments, or Arsenik-specific settings.
- After making changes, re-run:

  ```sh
  ./generator.sh --generate --copy
  ```

  and re-flash your keyboard as in steps 6 and 7.

This workflow allows you to iterate on your layout and configuration as much as you like.

---

**Need help or want to contribute?**

- Open an issue or PR if your layout is missing or you encounter problems.
- See the Arsenik documentation for advanced configuration.
