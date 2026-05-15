#!/usr/bin/env bash
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# Functional validation of the recompilable-client milestone.
#
# Demonstrates that `build/link/<bin>.exe` is byte-identical to
# the install's `<bin>.exe` for every binary, and substitutes the
# install copy with our re-link to prove the launcher / patcher /
# Wine pipeline runs against our output exactly the same as the
# orig.
#
# What this proves:
#   - cmp / md5 / sha256 of orig vs build/link is empty for all 5
#   - The OS loader sees IDENTICAL bytes — there's literally no
#     observable difference between the orig binaries and our
#     re-link. Functional identity follows from byte identity.
#   - garlemald-client's runtime PE patches (assert_log_patch,
#     null_this_guard_patch, etc.) apply to our re-link exactly the
#     same as orig because the patch byte ranges are identical.
#
# Usage:
#   tools/validate_relink.sh                 # check + substitute all 5
#   tools/validate_relink.sh --restore       # restore orig backups
#   tools/validate_relink.sh --verify-only   # check only, don't substitute

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
INSTALL_DIR="${FFXIV_INSTALL:-${REPO_ROOT}/../ffxiv-install-environment/target/prefix/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV}"
BACKUP_TAG="orig_backup_meteor_decomp_validation"

BINARIES=(ffxivgame ffxivlogin ffxivconfig ffxivupdater ffxivboot)

mode="${1:-}"

if [[ ! -d "$INSTALL_DIR" ]]; then
    echo "error: install dir not found: $INSTALL_DIR" >&2
    echo "       set FFXIV_INSTALL=<path> to override" >&2
    exit 1
fi

case "$mode" in
    --restore)
        echo "=== restore: $INSTALL_DIR ==="
        for bin in "${BINARIES[@]}"; do
            bak="$INSTALL_DIR/$bin.exe.$BACKUP_TAG"
            if [[ -f "$bak" ]]; then
                cp -p "$bak" "$INSTALL_DIR/$bin.exe"
                echo "  restored $bin.exe from backup"
            else
                echo "  no backup for $bin.exe — skipping"
            fi
        done
        exit 0
        ;;
    --verify-only|"")
        ;;
    *)
        echo "usage: $0 [--restore | --verify-only]" >&2
        exit 1
        ;;
esac

echo "=== validate_relink ==="
echo "  install: $INSTALL_DIR"
echo "  relink:  $REPO_ROOT/build/link/"
echo

all_match=1
for bin in "${BINARIES[@]}"; do
    install_exe="$INSTALL_DIR/$bin.exe"
    relink_exe="$REPO_ROOT/build/link/$bin.exe"
    if [[ ! -f "$install_exe" ]]; then
        echo "  $bin.exe — missing from install"; all_match=0; continue
    fi
    if [[ ! -f "$relink_exe" ]]; then
        echo "  $bin.exe — missing from build/link/ (run 'make relink BINARY=$bin.exe')"
        all_match=0; continue
    fi
    install_sha=$(shasum -a 256 "$install_exe" | awk '{print $1}')
    relink_sha=$(shasum -a 256 "$relink_exe"  | awk '{print $1}')
    install_size=$(stat -f%z "$install_exe" 2>/dev/null || stat -c%s "$install_exe")
    if [[ "$install_sha" == "$relink_sha" ]]; then
        printf "  %-15s ✓ BYTE-IDENTICAL  (%s bytes)\n" "$bin.exe" "$install_size"
    else
        printf "  %-15s ✗ DIFFERS\n" "$bin.exe"
        printf "    install: %s\n" "$install_sha"
        printf "    relink:  %s\n" "$relink_sha"
        all_match=0
    fi
done

if [[ "$all_match" -ne 1 ]]; then
    echo
    echo "✗ validation failed — at least one binary differs"
    exit 1
fi

echo
echo "✓ all 5 binaries byte-identical"

if [[ "$mode" == "--verify-only" ]]; then
    echo "  (--verify-only: not substituting)"
    exit 0
fi

echo
echo "=== substituting install with build/link/ ==="
for bin in "${BINARIES[@]}"; do
    install_exe="$INSTALL_DIR/$bin.exe"
    relink_exe="$REPO_ROOT/build/link/$bin.exe"
    bak="$install_exe.$BACKUP_TAG"
    if [[ ! -f "$bak" ]]; then
        cp -p "$install_exe" "$bak"
    fi
    cp "$relink_exe" "$install_exe"
    sub_sha=$(shasum -a 256 "$install_exe" | awk '{print $1}')
    bak_sha=$(shasum -a 256 "$bak" | awk '{print $1}')
    if [[ "$sub_sha" == "$bak_sha" ]]; then
        printf "  %-15s ✓ substituted (matches backup)\n" "$bin.exe"
    else
        printf "  %-15s ✗ post-sub mismatch\n" "$bin.exe"
        all_match=0
    fi
done

echo
echo "Install now runs against build/link/ output."
echo "Boot test: cd ffxiv-actor-cli && ./scripts/fresh-start-gridania.sh"
echo "Restore:   tools/validate_relink.sh --restore"
