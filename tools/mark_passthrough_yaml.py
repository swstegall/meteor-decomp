#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Mark `config/<binary>.yaml` rows as `status: passthrough` for every
function whose `_passthrough/<sym>.cpp` compiles to an .obj whose
`.text` matches orig byte-for-byte.

`passthrough` is a NEW YAML status, distinct from `matched`:
  - `matched`     — source-level decomp landed under `_rosetta/` and
                    cl.exe produces a byte-identical .text (or the
                    template stamper does). Readable C++.
  - `passthrough` — bytewise-equivalent .obj exists, but the source is
                    just `_emit` of the orig bytes (no readable code).
                    A fallback that lets the binary be relinked even
                    when the source-level decomp isn't done yet.

Pipeline:
  emit_passthrough_cpp.py      → src/<bin>/_passthrough/*.cpp
  cl-wine.sh / make compile-passthrough → build/obj/_passthrough/<bin>/*.obj
  mark_passthrough_yaml.py     → flips matching YAML rows to `passthrough`
                                  AND sanity-checks every .obj's .text
                                  against orig.

Idempotent. Only touches `unmatched` rows. Never overwrites `matched`
(source-level decomp is the canonical path).

Reads:
  config/<binary>.yaml
  build/pe-layout/<binary>.json
  orig/<binary>.exe
  build/obj/_passthrough/<binary>/FUN_<va>.obj  (one per function)

Writes:
  config/<binary>.yaml — in-place rewrite

Usage:
  tools/mark_passthrough_yaml.py ffxivlogin
  tools/mark_passthrough_yaml.py ffxivlogin --dry-run
  tools/mark_passthrough_yaml.py            # all five binaries
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
CONFIG = REPO_ROOT / "config"
SRC = REPO_ROOT / "src"
OBJ = REPO_ROOT / "build" / "obj" / "_passthrough"
ORIG = REPO_ROOT / "orig"
PE_LAYOUT = REPO_ROOT / "build" / "pe-layout"

ALL_BINARIES = ("ffxivboot", "ffxivconfig", "ffxivgame", "ffxivlogin", "ffxivupdater")

RE_RVA = re.compile(r"^\s*-\s*rva:\s*0x([0-9a-fA-F]+)\s*$")
RE_STATUS = re.compile(r"^(\s*)status:\s*(\S+)\s*$")
RE_TYPE = re.compile(r"^\s*type:\s*(\S+)\s*$")
RE_SIZE = re.compile(r"^\s*size:\s*0x([0-9a-fA-F]+)\s*$")


# ----------------------------------------------------------------------
# COFF .text extraction (handles `.text$<key>` long-name subsections)
# ----------------------------------------------------------------------


def _coff_text_bytes(obj_path: Path) -> bytes:
    data = obj_path.read_bytes()
    n_sections = struct.unpack_from("<H", data, 2)[0]
    sym_off = struct.unpack_from("<I", data, 8)[0]
    n_syms = struct.unpack_from("<I", data, 12)[0]
    opt_size = struct.unpack_from("<H", data, 16)[0]
    sec_off = 20 + opt_size
    str_table_off = sym_off + n_syms * 18
    chunks: list[bytes] = []
    for i in range(n_sections):
        base = sec_off + i * 40
        raw_name = data[base:base + 8]
        name = raw_name.rstrip(b"\0").decode("ascii", errors="replace")
        if name.startswith("/"):
            try:
                offset = int(name[1:])
                end = data.find(b"\0", str_table_off + offset)
                if end >= 0:
                    name = data[str_table_off + offset:end].decode("ascii", errors="replace")
            except ValueError:
                pass
        if not (name == ".text" or name.startswith(".text$")):
            continue
        raw_size = struct.unpack_from("<I", data, base + 16)[0]
        raw_ptr = struct.unpack_from("<I", data, base + 20)[0]
        chunks.append(data[raw_ptr:raw_ptr + raw_size])
    return b"".join(chunks)


# ----------------------------------------------------------------------
# PE layout — image base + sections, for RVA → file offset mapping.
# ----------------------------------------------------------------------


def _read_pe_layout(binary: str) -> tuple[int, list[dict]]:
    p = PE_LAYOUT / f"{binary}.json"
    data = json.loads(p.read_text())
    return int(data["image_base"], 16), data["sections"]


def _rva_to_file_off(rva: int, sections: list[dict]) -> int | None:
    for s in sections:
        va = s["virtual_address"]
        rsize = s["raw_size"]
        if va <= rva < va + rsize:
            return s["raw_pointer"] + (rva - va)
    return None


# ----------------------------------------------------------------------
# Walk YAML and rewrite passthrough-eligible rows.
# ----------------------------------------------------------------------


def _stream_yaml_blocks(yaml_path: Path):
    """Yield (rva, size, status, type) per row. Streams the YAML."""
    rva = size = None
    status = type_ = None
    for line in yaml_path.read_text().splitlines():
        m_rva = RE_RVA.match(line)
        if m_rva:
            if rva is not None:
                yield (rva, size, status, type_)
            rva = int(m_rva.group(1), 16)
            size = None
            status = type_ = None
            continue
        m_size = RE_SIZE.match(line)
        if m_size:
            size = int(m_size.group(1), 16)
            continue
        m_status = RE_STATUS.match(line)
        if m_status:
            status = m_status.group(2)
            continue
        m_type = RE_TYPE.match(line)
        if m_type:
            type_ = m_type.group(1)
            continue
    if rva is not None:
        yield (rva, size, status, type_)


def collect_passthrough_eligible(binary: str) -> tuple[set[int], dict[str, int]]:
    """Walk every `_passthrough/*.obj`, byte-compare against orig at the
    embedded RVA, and return (rva_set, stats). Only RVAs whose .obj is
    GREEN make it into the set."""
    image_base, sections = _read_pe_layout(binary)
    orig_bytes = (ORIG / f"{binary}.exe").read_bytes()
    yaml_path = CONFIG / f"{binary}.yaml"

    rva_to_size: dict[int, int] = {}
    rva_to_status: dict[int, str | None] = {}
    rva_to_type: dict[int, str | None] = {}
    for rva, size, status, type_ in _stream_yaml_blocks(yaml_path):
        if size is None:
            continue
        rva_to_size[rva] = size
        rva_to_status[rva] = status
        rva_to_type[rva] = type_

    obj_dir = OBJ / binary
    if not obj_dir.is_dir():
        return set(), {
            "obj_dir_missing": 1,
            "checked": 0, "green": 0, "mismatch": 0,
            "no_yaml_row": 0, "wrong_size": 0, "skipped_not_unmatched": 0,
        }

    eligible: set[int] = set()
    stats = {
        "obj_dir_missing": 0,
        "checked": 0,
        "green": 0,
        "mismatch": 0,
        "no_yaml_row": 0,
        "wrong_size": 0,
        "skipped_not_unmatched": 0,
    }
    fun_re = re.compile(r"^FUN_([0-9a-fA-F]+)\.obj$")
    for obj_path in obj_dir.iterdir():
        m = fun_re.match(obj_path.name)
        if not m:
            continue
        va = int(m.group(1), 16)
        rva = va - image_base
        size = rva_to_size.get(rva)
        if size is None:
            stats["no_yaml_row"] += 1
            continue
        # Only flip rows that are still `unmatched` — never overwrite
        # `matched`/`functional`/etc. (those are source-level wins).
        existing_status = rva_to_status.get(rva)
        if existing_status not in (None, "unmatched"):
            stats["skipped_not_unmatched"] += 1
            continue
        file_off = _rva_to_file_off(rva, sections)
        if file_off is None:
            stats["no_yaml_row"] += 1
            continue
        orig_slice = orig_bytes[file_off:file_off + size]
        try:
            ours = _coff_text_bytes(obj_path)
        except Exception:
            stats["mismatch"] += 1
            continue
        stats["checked"] += 1
        if len(ours) != len(orig_slice):
            stats["wrong_size"] += 1
            continue
        if ours == orig_slice:
            eligible.add(rva)
            stats["green"] += 1
        else:
            stats["mismatch"] += 1
    return eligible, stats


def rewrite_yaml(binary: str, eligible: set[int], dry_run: bool) -> tuple[int, int]:
    """Rewrite YAML, flipping eligible rva rows from `unmatched` to
    `passthrough`. Returns (flipped, already_passthrough)."""
    yaml_path = CONFIG / f"{binary}.yaml"
    lines = yaml_path.read_text().splitlines(keepends=True)
    out: list[str] = []
    flipped = already = 0
    current_rva: int | None = None
    current_type: str | None = None
    for line in lines:
        m_rva = RE_RVA.match(line)
        if m_rva:
            current_rva = int(m_rva.group(1), 16)
            current_type = None
            out.append(line)
            continue
        m_type = RE_TYPE.match(line)
        if m_type and current_rva is not None:
            current_type = m_type.group(1)
            out.append(line)
            continue
        m_status = RE_STATUS.match(line)
        if (m_status and current_rva is not None and current_type == "matching"
                and current_rva in eligible):
            existing = m_status.group(2)
            if existing == "passthrough":
                already += 1
                out.append(line)
            elif existing == "unmatched":
                indent = m_status.group(1)
                out.append(f"{indent}status: passthrough\n")
                flipped += 1
            else:
                # `matched` / `functional` / etc — never overwrite
                out.append(line)
            continue
        out.append(line)
    if not dry_run:
        yaml_path.write_text("".join(out))
    return flipped, already


def process_binary(binary: str, dry_run: bool) -> dict:
    eligible, stats = collect_passthrough_eligible(binary)
    flipped, already = rewrite_yaml(binary, eligible, dry_run)
    return {
        "binary": binary,
        **stats,
        "yaml_flipped": flipped,
        "yaml_already_passthrough": already,
    }


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", nargs="?", help="binary stem (default: all five)")
    ap.add_argument("--dry-run", action="store_true")
    args = ap.parse_args()

    binaries = (args.binary,) if args.binary else ALL_BINARIES
    print(f"=== mark_passthrough_yaml (dry-run={args.dry_run}) ===")
    total = {
        "checked": 0, "green": 0, "mismatch": 0,
        "no_yaml_row": 0, "wrong_size": 0,
        "skipped_not_unmatched": 0,
        "yaml_flipped": 0, "yaml_already_passthrough": 0,
    }
    for stem in binaries:
        s = process_binary(stem, args.dry_run)
        print(f"  {stem}: checked={s['checked']:>5} green={s['green']:>5} "
              f"mismatch={s['mismatch']:>3} skipped_status={s['skipped_not_unmatched']:>4} "
              f"flipped={s['yaml_flipped']:>5} already={s['yaml_already_passthrough']:>3}")
        for k in total:
            total[k] += s.get(k, 0)
    print(f"\ntotal: green={total['green']} flipped={total['yaml_flipped']} mismatch={total['mismatch']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
