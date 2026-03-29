#!/usr/bin/env python3
"""
Keymap layout linter and formatter for QMK keymap.c files.

Checks and fixes the formatting of ONEDEADKEY_LAYOUT() layer blocks.

Each layer has a fixed shape:
- 4 rows of 12 keys (2 pads of 6 keys separated by a gap)
- 1 thumb row of 6 keys (2 pads of 3 on the same line)

Usage:
    keymap_lint.py check <file> [--compact|--wide]    Report formatting issues
    keymap_lint.py fix <file>   [--compact|--wide]    Fix formatting in place
    keymap_lint.py diff <file>  [--compact|--wide]    Show what would change

Alignment modes:
    --compact    Each layer aligns columns independently (default)
    --wide       All layers share the same column widths
"""

import argparse
import re
import sys
from dataclasses import dataclass
from itertools import zip_longest


# ─────────────────────────────────────────────────────────────
# Constants (customize these to adjust formatting)
# ─────────────────────────────────────────────────────────────

LAYOUT_IDENTIFIER = "ONEDEADKEY_LAYOUT"

# Layer shape
COLS_PER_PAD = 6
NUM_PADS = 2
COLS_TOTAL = COLS_PER_PAD * NUM_PADS  # 12
MAIN_ROWS = 4
THUMB_COLS = 6
THUMB_KEYS_PER_PAD = THUMB_COLS // NUM_PADS  # 3
TOTAL_KEYS = MAIN_ROWS * COLS_TOTAL + THUMB_COLS  # 54

# Spacing
PAD_GAP = 8
MIN_COL_SPACING = 2
THUMB_SPACING = 2
ROW_INDENT = "        "  # 8 spaces

# Colors
RED = "\033[31m"
GREEN = "\033[32m"
DIM = "\033[2m"
BOLD = "\033[1m"
RESET = "\033[0m"


# ─────────────────────────────────────────────────────────────
# Token parsing
# ─────────────────────────────────────────────────────────────

def parse_tokens(text: str) -> list[str]:
    """Parse comma-separated key tokens, respecting parentheses depth."""
    tokens = []
    depth = 0
    current = ""
    for ch in text:
        if ch == '(':
            depth += 1
            current += ch
        elif ch == ')':
            depth -= 1
            current += ch
        elif ch == ',' and depth == 0:
            token = current.strip()
            if token:
                tokens.append(token)
            current = ""
        else:
            current += ch
    token = current.strip()
    if token:
        tokens.append(token)
    return tokens


# ─────────────────────────────────────────────────────────────
# Layer block parsing
# ─────────────────────────────────────────────────────────────

@dataclass
class LayerBlock:
    """A raw layer definition extracted from the file."""
    name: str
    header: str
    all_tokens: list[str]
    start_line: int
    end_line: int


@dataclass
class ParsedLayer:
    """A layer's tokens split into pads, ready for formatting."""
    block: LayerBlock
    left_pads: list[list[str]]
    right_pads: list[list[str]]
    thumb_tokens: list[str]


def find_layer_blocks(lines: list[str]) -> list[LayerBlock]:
    """Find all layout blocks in the file."""
    blocks = []
    i = 0
    while i < len(lines):
        line = lines[i]
        if LAYOUT_IDENTIFIER + "(" in line and "=" in line:
            header = line
            start_line = i

            # Track paren depth to find the matching closing )
            # The opening ( is on the header line
            depth = 1
            content = ""
            i += 1
            while i < len(lines) and depth > 0:
                line_text = lines[i]
                for ch_idx, ch in enumerate(line_text):
                    if ch == '(':
                        depth += 1
                    elif ch == ')':
                        depth -= 1
                        if depth == 0:
                            # Found the closing ) — content is everything before it
                            content += line_text[:ch_idx]
                            end_line = i
                            break
                else:
                    content += line_text
                    i += 1
                    continue
                break
            else:
                i += 1
                continue

            all_tokens = parse_tokens(content)
            match = re.search(r'\[(\w+)\]', header)
            name = match.group(1) if match else "unknown"

            blocks.append(LayerBlock(
                name=name, header=header, all_tokens=all_tokens,
                start_line=start_line, end_line=end_line,
            ))
        i += 1
    return blocks


def parse_layer_tokens(block: LayerBlock) -> ParsedLayer | None:
    """Split a layer block's tokens into pads. Returns None on error."""
    if len(block.all_tokens) != TOTAL_KEYS:
        return None

    main_tokens = block.all_tokens[:MAIN_ROWS * COLS_TOTAL]
    thumb_tokens = block.all_tokens[MAIN_ROWS * COLS_TOTAL:]

    main_rows = [
        main_tokens[r * COLS_TOTAL:(r + 1) * COLS_TOTAL]
        for r in range(MAIN_ROWS)
    ]

    return ParsedLayer(
        block=block,
        left_pads=[row[:COLS_PER_PAD] for row in main_rows],
        right_pads=[row[COLS_PER_PAD:] for row in main_rows],
        thumb_tokens=thumb_tokens,
    )


# ─────────────────────────────────────────────────────────────
# Formatting
# ─────────────────────────────────────────────────────────────

@dataclass
class ColumnWidths:
    """Pre-computed column widths for formatting."""
    left: list[int]
    right: list[int]
    max_left_str_width: int  # widest formatted left pad string


def compute_column_widths(rows: list[list[str]]) -> list[int]:
    """Compute max token width for each column across rows."""
    if not rows:
        return []
    num_cols = max(len(row) for row in rows)
    widths = [0] * num_cols
    for row in rows:
        for col_idx, token in enumerate(row):
            widths[col_idx] = max(widths[col_idx], len(token))
    return widths


def compute_widths(layers: list[ParsedLayer]) -> ColumnWidths:
    """Compute column widths from a list of layers.
    Works for both compact (1 layer) and wide (all layers) modes."""
    all_left = [pad for layer in layers for pad in layer.left_pads]
    all_right = [pad for layer in layers for pad in layer.right_pads]

    left = compute_column_widths(all_left)
    right = compute_column_widths(all_right)

    max_left_str_width = max(
        len(format_pad(pad, left, MIN_COL_SPACING))
        for pad in all_left
    )

    return ColumnWidths(left=left, right=right, max_left_str_width=max_left_str_width)


def format_pad(tokens: list[str], widths: list[int], spacing: int,
               trailing_comma: bool = True) -> str:
    """Format a pad with column alignment."""
    parts = []
    for col_idx, token in enumerate(tokens):
        if col_idx < len(tokens) - 1:
            padding = max(spacing, widths[col_idx] - len(token) + spacing)
            parts.append(token + "," + " " * padding)
        else:
            parts.append(token + ("," if trailing_comma else ""))
    return "".join(parts)


def format_main_rows(parsed: ParsedLayer, widths: ColumnWidths) -> list[str]:
    """Format the 4 main rows."""
    lines = []
    for r in range(MAIN_ROWS):
        left_str = format_pad(parsed.left_pads[r], widths.left, MIN_COL_SPACING)
        right_str = format_pad(parsed.right_pads[r], widths.right, MIN_COL_SPACING)
        gap = max(PAD_GAP, widths.max_left_str_width - len(left_str) + PAD_GAP)
        lines.append(f"{ROW_INDENT}{left_str}{' ' * gap}{right_str}")
    return lines


def format_thumb_row(thumb_tokens: list[str], widths: ColumnWidths) -> str:
    """Format the thumb row: left pad right-aligned, right pad aligned
    with the main rows' right pad."""
    left = thumb_tokens[:THUMB_KEYS_PER_PAD]
    right = thumb_tokens[THUMB_KEYS_PER_PAD:]

    left_widths = [len(t) for t in left]
    right_widths = [len(t) for t in right]

    left_str = format_pad(left, left_widths, THUMB_SPACING)
    right_str = format_pad(right, right_widths, THUMB_SPACING, trailing_comma=False)

    right_pad_col = len(ROW_INDENT) + widths.max_left_str_width + PAD_GAP
    left_padding = max(0, right_pad_col - PAD_GAP - len(left_str))

    return f"{' ' * left_padding}{left_str}{' ' * PAD_GAP}{right_str}"


def format_layer(parsed: ParsedLayer, widths: ColumnWidths) -> list[str]:
    """Format a single layer."""
    return [
        parsed.block.header,
        *format_main_rows(parsed, widths),
        "",  # empty line before thumbs
        format_thumb_row(parsed.thumb_tokens, widths),
        "    ),",
    ]


# ─────────────────────────────────────────────────────────────
# Comparison and display
# ─────────────────────────────────────────────────────────────

@dataclass
class LayerResult:
    """Result of checking one layer."""
    name: str
    original: list[str]
    formatted: list[str]
    has_issues: bool
    error: str | None


def compare_layer(block: LayerBlock, formatted: list[str] | None,
                  lines: list[str]) -> LayerResult:
    """Compare a layer block with its formatted version."""
    original = lines[block.start_line:block.end_line + 1]

    if formatted is None:
        return LayerResult(
            name=block.name, original=original, formatted=[],
            has_issues=True,
            error=f"has {len(block.all_tokens)} tokens, expected {TOTAL_KEYS}",
        )

    has_issues = (
        len(original) != len(formatted) or
        any(a.rstrip() != b.rstrip() for a, b in zip(original, formatted))
    )

    return LayerResult(
        name=block.name, original=original, formatted=formatted,
        has_issues=has_issues, error=None,
    )


def highlight_diff_spans(line_a: str, line_b: str, color: str) -> str:
    """Return line_a with differing spans highlighted in color."""
    a = line_a.rstrip()
    b = line_b.rstrip()

    prefix_len = 0
    while prefix_len < len(a) and prefix_len < len(b) and a[prefix_len] == b[prefix_len]:
        prefix_len += 1

    suffix_len = 0
    while (suffix_len < len(a) - prefix_len and
           suffix_len < len(b) - prefix_len and
           a[-(suffix_len + 1)] == b[-(suffix_len + 1)]):
        suffix_len += 1

    if prefix_len + suffix_len >= len(a):
        return a

    mid_end = len(a) - suffix_len
    return a[:prefix_len] + color + BOLD + a[prefix_len:mid_end] + RESET + a[mid_end:]


def print_layer_header(name: str, ok: bool):
    """Print a layer separator line."""
    if ok:
        print(f"\n{GREEN}── {name} {'─' * max(1, 60 - len(name) - 4)}{RESET}")
    else:
        print(f"\n{DIM}{'─' * 64}{RESET}")


def print_colored_block(lines_a: list[str], lines_b: list[str], color: str):
    """Print lines_a with differing spans highlighted against lines_b.
    Extra lines (no counterpart) are fully colored."""
    for a, b in zip_longest(lines_a, lines_b, fillvalue=None):
        text = a.rstrip() if a else ""
        if b is None or a is None:
            print(f"    {color}{text}{RESET}")
        elif text != b.rstrip():
            print(f"    {highlight_diff_spans(text, b.rstrip(), color)}")
        else:
            print(f"    {DIM}{text}{RESET}")


def print_layer_diff(result: LayerResult):
    """Print original and corrected layers with highlighted differences."""
    if result.error:
        print(f"  {RED}{result.error}{RESET}")
        return

    if not result.formatted:
        return

    print_colored_block(result.original, result.formatted, RED)
    print()
    print_colored_block(result.formatted, result.original, GREEN)


def print_summary(results: list[LayerResult]):
    """Print a summary line."""
    ok_count = sum(1 for r in results if not r.has_issues)
    fail_count = sum(1 for r in results if r.has_issues)
    issue_lines = sum(
        sum(1 for a, b in zip(r.original, r.formatted) if a.rstrip() != b.rstrip())
        for r in results
        if r.has_issues and r.formatted and len(r.original) == len(r.formatted)
    )

    parts = []
    if ok_count:
        parts.append(f"{GREEN}{ok_count} OK{RESET}")
    if fail_count:
        detail = f" ({issue_lines} lines)" if issue_lines else ""
        parts.append(f"{RED}{fail_count} need fixing{detail}{RESET}")

    print(f"\n{BOLD}{'  '.join(parts)}{RESET}")


def display_results(results: list[LayerResult], show_ok: bool = True):
    """Display layer results with diffs."""
    for result in results:
        if result.has_issues:
            print_layer_header(result.name, ok=False)
            print_layer_diff(result)
        elif show_ok:
            print_layer_header(result.name, ok=True)
    print_summary(results)


# ─────────────────────────────────────────────────────────────
# File processing
# ─────────────────────────────────────────────────────────────

def analyze_file(raw_lines: list[str], wide: bool = False) -> tuple[list[LayerResult], dict]:
    """Parse and compare all layers. Returns (results, replacements)."""
    blocks = find_layer_blocks(raw_lines)

    # Parse all layers
    parsed_map = [(block, parse_layer_tokens(block)) for block in blocks]
    valid_layers = [p for _, p in parsed_map if p]

    # Compute widths (global for wide, per-layer for compact)
    global_widths = compute_widths(valid_layers) if wide and valid_layers else None

    # Format and compare
    results = []
    replacements = {}

    for block, parsed in parsed_map:
        if parsed:
            widths = global_widths or compute_widths([parsed])
            formatted = format_layer(parsed, widths)
        else:
            formatted = None

        result = compare_layer(block, formatted, raw_lines)
        results.append(result)
        if result.has_issues and formatted:
            replacements[block.start_line] = (block.end_line, formatted)

    return results, replacements


def apply_replacements(raw_lines: list[str], replacements: dict) -> list[str]:
    """Apply formatting replacements to the file lines."""
    new_lines = []
    i = 0
    while i < len(raw_lines):
        if i in replacements:
            end_line, formatted = replacements[i]
            new_lines.extend(formatted)
            i = end_line + 1
        else:
            new_lines.append(raw_lines[i])
            i += 1
    return new_lines


def process_file(filepath: str, mode: str, wide: bool = False) -> int:
    """Process a keymap file. Returns exit code."""
    with open(filepath, "r") as f:
        content = f.read()
    raw_lines = content.rstrip("\n").split("\n")

    results, replacements = analyze_file(raw_lines, wide=wide)

    if not results:
        print(f"No {LAYOUT_IDENTIFIER} blocks found in {filepath}")
        return 0

    if mode == "check":
        display_results(results, show_ok=True)
        return 1 if any(r.has_issues for r in results) else 0

    if mode == "diff":
        has_issues = any(r.has_issues for r in results)
        if not has_issues:
            print(f"{GREEN}No changes needed{RESET}")
        display_results(results, show_ok=False)
        return 1 if has_issues else 0

    if mode == "fix":
        if not replacements:
            print(f"{GREEN}No changes needed in {filepath}{RESET}")
            return 0

        display_results(results, show_ok=True)

        passes = 0
        while replacements:
            passes += 1
            raw_lines = apply_replacements(raw_lines, replacements)
            _, replacements = analyze_file(raw_lines, wide=wide)
            if passes > 5:
                print(f"{RED}Fix did not converge after {passes} passes{RESET}")
                break

        with open(filepath, "w") as f:
            f.write("\n".join(raw_lines) + "\n")

        print(f"\n{BOLD}Fixed in {filepath} ({passes} pass{'es' if passes > 1 else ''}){RESET}")
        return 0

    return 1


def main():
    parser = argparse.ArgumentParser(
        description="Keymap layout linter and formatter for QMK keymap.c files.",
    )
    parser.add_argument(
        "mode", choices=["check", "fix", "diff"],
        help="check: report issues, fix: fix in place, diff: show changes",
    )
    parser.add_argument("file", help="path to keymap.c file")

    alignment = parser.add_mutually_exclusive_group()
    alignment.add_argument(
        "--compact", action="store_true", default=True,
        help="each layer aligns columns independently (default)",
    )
    alignment.add_argument(
        "--wide", action="store_true",
        help="all layers share the same column widths",
    )

    args = parser.parse_args()
    sys.exit(process_file(args.file, args.mode, wide=args.wide))


if __name__ == "__main__":
    main()
