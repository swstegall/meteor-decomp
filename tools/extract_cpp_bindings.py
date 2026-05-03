#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Enumerate the engine's C++-bound (Lua-callable) API surface across the
shipped script corpus.

Background (`docs/world_master_decomp.md`): each Lua-bindable class
that has C++-implemented methods declares them in a `_u`-suffix file
(encoded as `_p` in the ciphered tree) using the
`_<method>_cpp` / `_<method>_inl` pair convention. The `_inl` body is
a 2-line stub returning the literal strings `"self"` and
`"_<method>_cpp"`; the engine reads the return values and dispatches
to the matching C++ implementation.

To enumerate the engine-bound API, this tool:

  1. Walks every `*_p.luac` file under `build/lpb/`
  2. Parses the `_<method>_inl` method names embedded in each file's
     string table (via `strings`-style scanning of the COFF / Lua
     string constants)
  3. Decodes the filename cipher to recover the source-side class
     name from each file's path
  4. Emits the canonical inventory as JSON + Markdown

Output:
  build/wire/cpp_bindings.json   — {class_name: [method_name, ...]}
  build/wire/cpp_bindings.md     — Per-class table for human reading

Required: `build/lpb/` populated (run `make decode-lpb` first).

Usage:
  tools/extract_cpp_bindings.py            # default paths
  tools/extract_cpp_bindings.py --lpb-dir build/lpb --out-dir build/wire
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
from collections import defaultdict
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent


def decode_filename(s: str) -> str:
    """FFXIV 1.x filename involution cipher (see `tools/decode_lpb.py`)."""
    out = []
    for c in s.lower():
        if c.isalpha():
            pos = ord(c) - ord("a") + 1
            if 1 <= pos <= 10:
                out.append(str(10 - pos))
            else:
                out.append(chr(ord("a") + (37 - pos) - 1))
        elif c.isdigit():
            d = int(c)
            out.append(chr(ord("a") + (10 - d) - 1))
        else:
            out.append(c)
    return "".join(out)


def class_name_from_path(path: Path, lpb_root: Path) -> str:
    """Recover the source-side class name from a ciphered .luac filename.

    Strategy: strip `_p.luac` suffix → decipher the basename → strip
    common base-class suffixes if needed. The basename usually decodes
    to "<ClassName>BaseClass" or "<ClassName>" directly.
    """
    rel = path.relative_to(lpb_root)
    stem = rel.stem  # e.g. "uy9l5s89r57y9rr_p"
    # Strip the trailing `_p` (the ciphered `_u` aspect-suffix marker)
    if stem.endswith("_p"):
        stem = stem[:-2]
    return decode_filename(stem)


# Pattern for the _inl method names: "_<name>_inl" where <name> is
# typically alphanumeric + underscores. The strings are stored in the
# Lua bytecode's string table and are extractable via raw-byte scan
# for the printable ASCII pattern.
INL_RE = re.compile(rb"(_[A-Za-z][A-Za-z0-9_]+)_inl\x00")


def extract_inl_methods(luac_path: Path) -> list[str]:
    """Scan a .luac file for `_<name>_inl\\x00` patterns in its string
    table. Returns the list of `_<name>` (without the `_inl` suffix —
    these are the canonical method-name stems).
    """
    data = luac_path.read_bytes()
    seen: set[str] = set()
    for m in INL_RE.finditer(data):
        seen.add(m.group(1).decode("ascii", errors="replace"))
    return sorted(seen)


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--lpb-dir", type=Path, default=REPO_ROOT / "build" / "lpb",
                    help="directory of decoded .luac files (default: build/lpb)")
    ap.add_argument("--out-dir", type=Path, default=REPO_ROOT / "build" / "wire",
                    help="output dir for cpp_bindings.{json,md} (default: build/wire)")
    args = ap.parse_args()

    if not args.lpb_dir.exists():
        print(f"error: {args.lpb_dir} not found — run 'make decode-lpb' first",
              file=sys.stderr)
        return 1
    args.out_dir.mkdir(parents=True, exist_ok=True)

    # Walk all *_p.luac files
    p_files: list[Path] = []
    for root, _, files in os.walk(args.lpb_dir):
        for f in files:
            if f.endswith("_p.luac"):
                p_files.append(Path(root) / f)
    print(f"Scanning {len(p_files)} *_p.luac files …", file=sys.stderr)

    # Per-class method inventory
    bindings: dict[str, list[str]] = {}
    file_stats: list[dict] = []

    for path in sorted(p_files):
        cls = class_name_from_path(path, args.lpb_dir)
        methods = extract_inl_methods(path)
        if methods:
            bindings[cls] = methods
        file_stats.append({
            "class": cls,
            "ciphered_path": str(path.relative_to(args.lpb_dir)),
            "method_count": len(methods),
        })

    # Sort for deterministic output
    bindings = {k: bindings[k] for k in sorted(bindings.keys())}
    total_methods = sum(len(v) for v in bindings.values())
    distinct_methods = len({m for v in bindings.values() for m in v})

    # Emit JSON
    json_path = args.out_dir / "cpp_bindings.json"
    json_path.write_text(json.dumps({
        "summary": {
            "files_scanned": len(p_files),
            "classes_with_bindings": len(bindings),
            "total_method_declarations": total_methods,
            "distinct_method_names": distinct_methods,
        },
        "bindings": bindings,
    }, indent=2))

    # Emit Markdown
    md_path = args.out_dir / "cpp_bindings.md"
    with md_path.open("w") as f:
        f.write("# Engine-bound (C++) Lua API — corpus inventory\n\n")
        f.write("Auto-generated by `tools/extract_cpp_bindings.py`. Do not edit\n")
        f.write("by hand — re-run after `make decode-lpb` if the install changes.\n\n")
        f.write("## How this is built\n\n")
        f.write("Per `docs/world_master_decomp.md`, every Lua-bindable class with\n")
        f.write("C++-implemented methods declares them in a `_u` (ciphered `_p`)\n")
        f.write("file using the `_<method>_cpp` / `_<method>_inl` pair convention.\n")
        f.write("The `_inl` stub returns `(\"self\", \"_<method>_cpp\")`; the\n")
        f.write("engine reads those literals and dispatches to the matching C++\n")
        f.write("implementation. This tool scans every `*_p.luac` file's string\n")
        f.write("table for `_<name>_inl\\x00` patterns.\n\n")
        f.write("## Summary\n\n")
        f.write(f"- **Files scanned**: {len(p_files)}\n")
        f.write(f"- **Classes with C++ bindings**: {len(bindings)}\n")
        f.write(f"- **Total method declarations**: {total_methods}\n")
        f.write(f"- **Distinct method names**: {distinct_methods}\n\n")

        # Top-N classes by method count
        top = sorted(bindings.items(), key=lambda x: -len(x[1]))
        f.write("## Top 30 classes by C++-bound method count\n\n")
        f.write("| Class | Methods |\n|---|---:|\n")
        for cls, methods in top[:30]:
            f.write(f"| `{cls}` | {len(methods)} |\n")
        f.write("\n")

        # Most-frequent method names (occur across many classes)
        method_class_count: dict[str, set[str]] = defaultdict(set)
        for cls, methods in bindings.items():
            for m in methods:
                method_class_count[m].add(cls)
        f.write("## Top 50 most-shared C++ methods (declared in many classes)\n\n")
        f.write("Methods that appear in many classes likely reflect inherited\n")
        f.write("base-class API (declared in each subclass's _u file).\n\n")
        f.write("| Method | Declared in N classes |\n|---|---:|\n")
        for method in sorted(method_class_count, key=lambda m: -len(method_class_count[m]))[:50]:
            f.write(f"| `{method}` | {len(method_class_count[method])} |\n")
        f.write("\n")

        # Per-class detailed listing
        f.write("## Per-class C++ binding inventory\n\n")
        for cls, methods in bindings.items():
            f.write(f"### `{cls}` ({len(methods)} methods)\n\n")
            f.write("```\n")
            for m in methods:
                f.write(f"  {m}_cpp / {m}_inl\n")
            f.write("```\n\n")

    print(f"Wrote {json_path.relative_to(REPO_ROOT)}", file=sys.stderr)
    print(f"Wrote {md_path.relative_to(REPO_ROOT)}", file=sys.stderr)
    print(f"Summary: {len(bindings)} classes, {total_methods} declarations, "
          f"{distinct_methods} distinct methods")
    return 0


if __name__ == "__main__":
    sys.exit(main())
