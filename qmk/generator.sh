#!/usr/bin/env bash

# Arsenik QMK keymap generator
# Generates files required by QMK to build/flash a keymap for a given keyboard
# Outputs files in ./output/<keyboard_model>/keymaps/<target>

set -euo pipefail

# Colors for logs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

log_info() { echo -e "${BLUE}[INFO]${NC} $1"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
log_error() { echo -e "${RED}[ERROR]${NC} $1" >&2; }

# Resolve the directory where this script lives
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Available keymap targets
available_targets() {
    local targets=()
    for dir in "$SCRIPT_DIR"/*/; do
        local name="$(basename "$dir")"
        if [ "$name" != "shared" ] && [ "$name" != "output" ]; then
            targets+=("$name")
        fi
    done
    echo "${targets[*]}"
}

# Print usage and exit
usage() {
    echo "Usage: $0 [-kb <keyboard_model>] [-km <ref_keymap>] [-target <target>] [--generate|-g] [--copy|-cp]"
    echo "  -kb <keyboard_model>   Specify the keyboard model (optional, will use qmk config if not provided)"
    echo "  -km <ref_keymap>       Reference keymap for layout detection (optional, will use qmk config if not provided)"
    echo "  -target <target>       Keymap target (default: selenium). Available: $(available_targets)"
    echo "  --generate, -g         Generate the keymap (default: off)"
    echo "  --copy, -cp            Copy ./output/<keyboard_model>/keymaps/<target> to \$QMK_HOME/keyboards/<keyboard_model>/keymaps/<target> (default: off)"
    exit 1
}

# Parse arguments
copy_keymap=false
generate_keymap=false
keyboard_model=""
keymap=""
keymap_target="selenium"

while [ "$#" -gt 0 ]; do
    case "$1" in
    -kb)
        shift
        keyboard_model="$1"
        ;;
    -km)
        shift
        keymap="$1"
        ;;
    -target)
        shift
        keymap_target="$1"
        ;;
    --copy | -cp)
        copy_keymap=true
        ;;
    --generate | -g)
        generate_keymap=true
        ;;
    *)
        usage
        ;;
    esac
    shift
done

# Validate keymap source
if [ ! -d "$SCRIPT_DIR/$keymap_target" ]; then
    log_error "Keymap target '${CYAN}$keymap_target${NC}' not found. Available: $(available_targets)"
    exit 1
fi

if [ -z "$keyboard_model" ] || [ -z "$keymap" ]; then
    log_info "Requirements: It is recommended to have set QMK user.keyboard, user.keymap, and user.qmk_home using 'qmk config'."
    log_info "See https://docs.qmk.fm/cli_configuration#setting-user-defaults for more information."
    log_info "Your current configuration:"
    qmk config || true
fi

# If keyboard_model or keymap is empty, get from qmk config
if [ -z "$keyboard_model" ]; then
    keyboard_model=$(qmk config user.keyboard | awk -F= '{print $2}' | xargs)
    log_info "No keyboard_model provided with '-kb' argument, using value from qmk config: ${CYAN}$keyboard_model${NC}"
fi
if [ -z "$keymap" ]; then
    keymap=$(qmk config user.keymap | awk -F= '{print $2}' | xargs)
    log_info "No keymap provided with '-km' argument, using value from qmk config: ${CYAN}$keymap${NC}"
fi

# Set QMK_HOME if not already set
if [ -z "${QMK_HOME+x}" ]; then
    QMK_HOME=$(qmk config user.qmk_home | awk -F= '{print $2}' | xargs | envsubst)
fi

# Find the keymaps folder for a given keyboard name
get_keymaps_folder() {
    local keyboard_model="$1"
    local keymap="$2"
    while [ ! -d "$QMK_HOME/keyboards/$keyboard_model/keymaps/$keymap" ]; do
        if [ "$keyboard_model" = "." ]; then
            echo "Couldn't find keymap folder for your keyboard/keymap" >&2
            exit 1
        fi
        keyboard_model=$(dirname "$keyboard_model")
    done
    echo "$QMK_HOME/keyboards/$keyboard_model/keymaps/$keymap"
}

# Detect layout name
detect_layout_name() {
    local keyboard_model="$1"
    local keymap="$2"

    local keymap_folder
    keymap_folder=$(get_keymaps_folder "$keyboard_model" "$keymap")

    local layout=""
    if [ -f "$keymap_folder/keymap.c" ]; then
        layout=$(grep 'LAYOUT' "$keymap_folder/keymap.c" | sed 's/.*= \(.*\)(/\1/' | head -n 1)
    elif [ -f "$keymap_folder/keymap.json" ]; then
        layout=$(grep 'LAYOUT' "$keymap_folder/keymap.json" | sed 's/ *\"layout\": \"\(.*\)\",/\1/')
    fi

    if [ "$layout" = "LAYOUT" ]; then
        layout+="_$(echo "$keyboard_model" | sed 's,/,_,g' | sed 's/_rev[0-9]*$//')"
    fi

    # Prefix with ONEDEADKEY_ only if not already prefixed
    if [[ "$layout" != ONEDEADKEY_* ]]; then
        layout="ONEDEADKEY_$layout"
    fi
    echo "$layout"
}

create_output_dir() {
    local output_dir="$1"
    if [ -d "$output_dir" ]; then
        log_warn "Output directory ${CYAN}$output_dir${NC} already exists."
        read -p "Do you want to remove it and continue? [y/N]: " confirm
        if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
            log_info "Aborted by user."
            exit 0
        fi
        log_info "Removing existing output directory: ${CYAN}$output_dir${NC}"
        rm -rf "$output_dir"
    fi
    log_info "Creating output directory: ${CYAN}$output_dir${NC}"
    mkdir -p "$output_dir"
}

# Generate the keymap in ./output/<keyboard_model>/<keymap>
generate_arsenik_keymap() {
    local keyboard_model="$1"
    local keymap="$2"
    local output_dir="$3"
    local target="$4"

    local layout_name="$(detect_layout_name "$keyboard_model" "$keymap")"
    log_info "Detected layout name: ${CYAN}$layout_name${NC}"
    log_info "Using keymap target: ${CYAN}$target${NC}"

    log_info "Copying shared files to output directory..."
    rsync -a "$SCRIPT_DIR/shared/" "$output_dir"

    log_info "Copying ${CYAN}$target${NC} keymap files to output directory..."
    rsync -a "$SCRIPT_DIR/$target/" "$output_dir"

    # Flatten include paths (../shared/ -> same directory)
    log_info "Flattening include paths..."
    sed -i 's|"../shared/|"|g' "$output_dir"/*.h "$output_dir"/*.c 2>/dev/null || true

    # Replace placeholder in config.h template file
    log_info "Set ${CYAN}$layout_name${NC} layout name in config.h template file..."
    sed -i "s/ONEDEADKEY_PLACEHOLDER_LAYOUT/$layout_name/" "$output_dir/config.h"

    log_success "Generated keymap in ${CYAN}$output_dir${NC}"
}

# Main
output_dir="./output/$keyboard_model/keymaps/$keymap_target"

if [ "$generate_keymap" = true ]; then
    create_output_dir "$output_dir"
    generate_arsenik_keymap "$keyboard_model" "$keymap" "$output_dir" "$keymap_target"
    log_info "Now you may want to copy the '/output' directory into your QMK home directory."
    log_info "This can be done with the './generator.sh --copy' command."
fi

if [ "$copy_keymap" = true ]; then
    if [ ! -d "$output_dir" ]; then
        log_error "Output directory ${CYAN}$output_dir${NC} does not exist. Cannot copy."
        exit 1
    fi
    dest_dir="$QMK_HOME/keyboards/$keyboard_model/keymaps/$keymap_target"
    if [ -d "$dest_dir" ]; then
        log_warn "Destination directory ${CYAN}$dest_dir${NC} already exists."
        read -p "Do you want to overwrite it and continue? [y/N]: " confirm
        if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
            log_info "Aborted by user."
            exit 0
        fi
        log_info "Removing existing destination directory: ${CYAN}$dest_dir${NC}"
        rm -rf "$dest_dir"
    fi
    log_info "Copying ${CYAN}$output_dir${NC} to ${CYAN}$dest_dir${NC} ..."
    rsync -a "$output_dir/" "$dest_dir"
    log_success "Copied keymap to ${CYAN}$dest_dir${NC}"
    log_info "Finally, you may want to :"
    log_info "    cd $dest_dir"
    log_info "    qmk compile -kb $keyboard_model -km $keymap_target    # compile the keymap"
    log_info "    # connect the keyboard"
    log_info "    qmk flash -kb $keyboard_model -km $keymap_target      # flash the keyboard"
fi

if [ "$generate_keymap" = false ] && [ "$copy_keymap" = false ]; then
    log_info "No action requested (--generate/-g or --copy/-cp). Nothing to do."
fi
