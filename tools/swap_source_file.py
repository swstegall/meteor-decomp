#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Swap a hand-written multi-function source file (e.g.
`src/<binary>/sqex/Utf8String.cpp`) into the relink. Each function in
the source compiles to its own COMDAT `.text` section under cl.exe's
`/Gy` flag; we patch each section's name to `.text$X<rva>` so link.exe
places it at the orig RVA.

The (section → rva) mapping is *auto-discovered* via byte-pattern
search: take each section's bytes, mask the 4 bytes at every
relocation offset, and grep orig `.text` for the masked pattern. A
section that finds exactly one match in orig is the one we want;
a section that finds 0 or >1 matches is skipped (the source isn't
byte-perfect or the pattern is ambiguous).

Pipeline:
  1. Compile `src/<bin>/<subdir>/<name>.cpp` with `/Gy` →
     `build/obj/_swapsrc/<bin>/<name>.obj`. Each function lands in
     its own `.text` COMDAT section.
  2. Walk the .obj's `.text` COMDAT sections. For each, build a
     reloc-aware byte pattern and search orig `.text` for matches.
  3. For each section with exactly one match: rename `.text` →
     `.text$X<rva>` (in-place patch of the COFF section header), and
     record (rva, source_path) in the swap manifest.
  4. The next `make relink` picks up the obj alongside the per-fn
     `_swap_FUN_<va>.obj` files.

Usage:
  tools/swap_source_file.py <binary> <source_path>
  tools/swap_source_file.py <binary> --all          # walk all hand-written src dirs
  tools/swap_source_file.py <binary> --list         # show recorded source-file swaps
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import subprocess
import sys
import threading
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC = REPO_ROOT / "src"
ORIG = REPO_ROOT / "orig"
PE_LAYOUT = REPO_ROOT / "build" / "pe-layout"
SWAPSRC_OBJ = REPO_ROOT / "build" / "obj" / "_swapsrc"
PASSTHROUGH_OBJ = REPO_ROOT / "build" / "obj" / "_passthrough"

# Hand-written multi-function source dirs (per binary). These live
# OUTSIDE `_rosetta/` and contain semantic source files with `// FUNCTION:`
# comment headers per matched function.
SOURCE_SUBDIRS = ("crt", "sqex", "sqpack", "install")

LICENSE_HEADER = """\
// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
"""


# ----------------------------------------------------------------------
# COFF parsing helpers
# ----------------------------------------------------------------------


def _coff_layout(data: bytes) -> tuple[int, int, int, int, int]:
    """Return (n_sections, sec_off, sym_off, n_syms, str_table_off)."""
    sig1, sig2 = struct.unpack_from("<HH", data, 0)
    if sig1 == 0x0000 and sig2 == 0xFFFF:
        n_sections = struct.unpack_from("<I", data, 0x2c)[0]
        sym_off = struct.unpack_from("<I", data, 0x30)[0]
        n_syms = struct.unpack_from("<I", data, 0x34)[0]
        sec_off = 0x38
        sym_size = 20
    else:
        n_sections = struct.unpack_from("<H", data, 2)[0]
        sym_off = struct.unpack_from("<I", data, 8)[0]
        n_syms = struct.unpack_from("<I", data, 12)[0]
        opt_size = struct.unpack_from("<H", data, 16)[0]
        sec_off = 20 + opt_size
        sym_size = 18
    str_table_off = sym_off + n_syms * sym_size
    return n_sections, sec_off, sym_off, n_syms, str_table_off


def _coff_get_sym_name(data: bytes, idx: int, sym_off: int,
                       str_table_off: int) -> str:
    base = sym_off + idx * 18
    name_bytes = data[base:base + 8]
    if name_bytes[:4] == b"\0\0\0\0":
        offset = struct.unpack_from("<I", data, base + 4)[0]
        end = data.find(b"\0", str_table_off + offset)
        return data[str_table_off + offset:end].decode("ascii", errors="replace")
    return name_bytes.rstrip(b"\0").decode("ascii", errors="replace")


def _coff_section_to_external_symbol(data: bytes, n_sections: int,
                                     sec_off: int, sym_off: int,
                                     n_syms: int, str_table_off: int) -> dict[int, str]:
    """Return {section_index_1based: first_external_symbol_name}."""
    out: dict[int, str] = {}
    i = 0
    while i < n_syms:
        base = sym_off + i * 18
        sec_num = struct.unpack_from("<h", data, base + 12)[0]
        sclass = data[base + 16]
        n_aux = data[base + 17]
        if sclass == 2 and sec_num > 0 and sec_num not in out:
            out[sec_num] = _coff_get_sym_name(data, i, sym_off, str_table_off)
        i += 1 + n_aux
    return out


def _coff_walk_text_sections(data: bytes):
    """Yield (section_idx_1based, name, rsize, raw_ptr, ptr_relocs, n_relocs, chars_off)
    for every `.text` (COMDAT or not) section."""
    n_sections, sec_off, sym_off, n_syms, str_table_off = _coff_layout(data)
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
                continue
        if not (name == ".text" or name.startswith(".text")):
            continue
        rsize = struct.unpack_from("<I", data, base + 16)[0]
        raw_ptr = struct.unpack_from("<I", data, base + 20)[0]
        ptr_relocs = struct.unpack_from("<I", data, base + 24)[0]
        n_relocs = struct.unpack_from("<H", data, base + 32)[0]
        yield (i + 1, name, rsize, raw_ptr, ptr_relocs, n_relocs, base + 36)


# ----------------------------------------------------------------------
# Compile
# ----------------------------------------------------------------------


def compile_source_file(binary: str, source_path: Path) -> Path:
    """Compile `source_path` with /Gy + flags. Output goes to
    `build/obj/_swapsrc/<binary>/<basename>.obj`."""
    obj_dir = SWAPSRC_OBJ / binary
    obj_dir.mkdir(parents=True, exist_ok=True)
    obj_path = obj_dir / f"{source_path.stem}.obj"
    cl_wine = REPO_ROOT / "tools" / "cl-wine.sh"
    rel_src = source_path.relative_to(REPO_ROOT)
    rel_obj = obj_path.relative_to(REPO_ROOT)
    cmd = [
        str(cl_wine),
        "/c", "/O2", "/Oy", "/GR-", "/EHs-", "/Gy", "/MT",
        "/Zc:wchar_t", "/Zc:forScope", "/TP",
        f"/Fo{rel_obj}",
        str(rel_src),
    ]
    res = subprocess.run(cmd, capture_output=True, text=True, cwd=REPO_ROOT)
    if res.returncode != 0:
        raise RuntimeError(
            f"cl.exe failed for {source_path.name}:\n  stdout: {res.stdout[:500]}\n  stderr: {res.stderr[:500]}"
        )
    return obj_path


# ----------------------------------------------------------------------
# Match COMDAT .text sections to orig RVAs by reloc-aware byte search.
# ----------------------------------------------------------------------


def _build_reloc_pattern(body: bytes, reloc_offs: list[int]) -> bytes:
    """Build a regex byte pattern: literal bytes except 4-byte wildcard
    at each reloc offset. The mask is `re.escape`'d so we can use any
    byte values literally."""
    parts = []
    last = 0
    for off in sorted(reloc_offs):
        if off > last:
            parts.append(re.escape(bytes(body[last:off])))
        parts.append(b".{4}")
        last = off + 4
    if last < len(body):
        parts.append(re.escape(bytes(body[last:])))
    return b"".join(parts)


def find_section_in_orig(body: bytes, reloc_offs: list[int],
                         orig_text: bytes, text_va: int) -> list[int]:
    """Return list of orig RVAs where this section's bytes match."""
    pattern = _build_reloc_pattern(body, reloc_offs)
    return [text_va + m.start() for m in re.finditer(pattern, orig_text, flags=re.DOTALL)]


# Source `// FUNCTION: ffxivgame 0x<rva> — <signature>` comment regex.
# Captures the RVA + signature.
_FUNCTION_COMMENT_RE = re.compile(
    r"^//\s*FUNCTION:\s*\w+\s+0x([0-9a-fA-F]+)\s*[—\-]+\s*(.+?)\s*$",
    re.MULTILINE,
)


def parse_function_comments(source_path: Path) -> list[tuple[int, str]]:
    """Walk a hand-written multi-fn source's `// FUNCTION:` comments
    and return [(rva, signature)] in source order."""
    text = source_path.read_text()
    out: list[tuple[int, str]] = []
    for m in _FUNCTION_COMMENT_RE.finditer(text):
        rva = int(m.group(1), 16)
        sig = m.group(2)
        out.append((rva, sig))
    return out


def find_section_via_comment_hint(body: bytes, reloc_offs: list[int],
                                  orig_text: bytes, text_va: int,
                                  candidate_rvas: list[int],
                                  min_match_pct: float = 50.0) -> tuple[int, float] | None:
    """For a section that didn't match orig exactly, score each
    candidate RVA from the source's `// FUNCTION:` comments and pick
    the best.

    For each candidate RVA, count bytes that match between `body`
    and orig at that RVA, MASKING the relocation positions
    (4 bytes each) since they'll be different (orig has the linker-
    fixed-up address; obj has zero placeholders).

    Returns (rva, match_pct) for the best candidate IF it's above
    `min_match_pct` AND uniquely the highest match. Otherwise None.
    """
    # Mask of byte positions to ignore (relocations).
    n = len(body)
    mask = bytearray(n)
    for off in reloc_offs:
        for i in range(off, min(off + 4, n)):
            mask[i] = 1
    n_compared = sum(1 for x in mask if x == 0)
    if n_compared == 0:
        return None

    scores: list[tuple[int, float]] = []
    for rva in candidate_rvas:
        file_off = rva - text_va
        if file_off < 0 or file_off + n > len(orig_text):
            continue
        orig_slice = orig_text[file_off:file_off + n]
        n_match = sum(1 for i in range(n)
                      if mask[i] == 0 and body[i] == orig_slice[i])
        pct = 100.0 * n_match / n_compared
        scores.append((rva, pct))
    if not scores:
        return None
    scores.sort(key=lambda t: -t[1])
    best_rva, best_pct = scores[0]
    if best_pct < min_match_pct:
        return None
    # Require gap to second-best so we don't accept ambiguous matches.
    if len(scores) >= 2 and scores[1][1] >= best_pct - 5.0:
        return None
    return (best_rva, best_pct)


# ----------------------------------------------------------------------
# Patch COFF section to rename `.text` → `.text$X<rva>`.
# ----------------------------------------------------------------------


def _coff_set_section_name(raw: bytearray, sec_off: int, idx_1based: int,
                            new_name: str, str_table_off: int):
    """Rewrite the section's Name field. Long names (>8 chars) go into
    the string table via `/N` reference."""
    base = sec_off + (idx_1based - 1) * 40
    if len(new_name) <= 8:
        # Inline name (NUL-padded).
        name_bytes = new_name.encode("ascii").ljust(8, b"\0")
        raw[base:base + 8] = name_bytes
    else:
        # Append to string table. The string table starts with a
        # 4-byte size header; we append the new name + NUL.
        cur_size = struct.unpack_from("<I", raw, str_table_off)[0]
        new_offset = cur_size  # offset within string table (not file)
        # Append.
        appended = new_name.encode("ascii") + b"\0"
        raw.extend(appended)
        # Update size header.
        struct.pack_into("<I", raw, str_table_off, cur_size + len(appended))
        # Set section name to "/N\0...".
        ref = f"/{new_offset}".encode("ascii").ljust(8, b"\0")
        raw[base:base + 8] = ref


def _coff_patch_section_align(raw: bytearray, chars_off: int) -> None:
    """Force section alignment to 1 byte (clears bits 20-23, sets 0x100000)."""
    chars = struct.unpack_from("<I", raw, chars_off)[0]
    new_chars = (chars & ~0x00f00000) | 0x00100000
    struct.pack_into("<I", raw, chars_off, new_chars)


def _coff_freeze_section(raw: bytearray, sec_idx_1based: int, rva: int,
                          size: int, orig: bytes, text_va: int,
                          text_sec: dict) -> None:
    """Replace the section's bytes with orig bytes at `rva`, and zero
    out its relocation table. After this the section is a self-
    contained byte blob that link.exe doesn't need to resolve any
    externals for. We've already verified (by `find_section_in_orig`)
    that the original .obj bytes match orig's bytes at this RVA modulo
    relocations — so the post-freeze content is what orig has at this
    RVA, byte-for-byte."""
    n_sections, sec_off, sym_off, n_syms, str_table_off = _coff_layout(bytes(raw))
    base = sec_off + (sec_idx_1based - 1) * 40
    raw_size = struct.unpack_from("<I", raw, base + 16)[0]
    raw_ptr = struct.unpack_from("<I", raw, base + 20)[0]
    file_off = text_sec["raw_pointer"] + (rva - text_va)
    orig_bytes = orig[file_off:file_off + raw_size]
    if len(orig_bytes) != raw_size:
        return
    raw[raw_ptr:raw_ptr + raw_size] = orig_bytes
    # Zero relocation count (PointerToRelocations + NumberOfRelocations).
    struct.pack_into("<I", raw, base + 24, 0)  # PointerToRelocations
    struct.pack_into("<H", raw, base + 32, 0)  # NumberOfRelocations


def _coff_count_undefined_externs(raw: bytes) -> int:
    """Count EXTERNAL UNDEFINED symbols (sec_num=0, sclass=2). These
    are external references the linker MUST resolve. If any survive
    after we've frozen accepted sections + LNK_REMOVE'd skipped ones,
    the source file isn't link-clean and we should reject it."""
    sym_off = struct.unpack_from("<I", raw, 8)[0]
    n_syms = struct.unpack_from("<I", raw, 12)[0]
    n_undef = 0
    i = 0
    while i < n_syms:
        base = sym_off + i * 18
        sec_num = struct.unpack_from("<h", raw, base + 12)[0]
        sclass = raw[base + 16]
        n_aux = raw[base + 17]
        if sclass == 2 and sec_num == 0:
            n_undef += 1
        i += 1 + n_aux
    return n_undef


def _coff_neutralize_undefined_externs(raw: bytearray) -> int:
    """Convert EXTERNAL UNDEFINED symbols (sec_num=0, sclass=2) to
    a class link.exe ignores. After freezing accepted sections +
    LNK_REMOVE'ing skipped ones, no relocation actually references
    these symbols anymore — their entries in the symbol table are
    leftover noise from the original (skipped) compile.

    Setting `Value = 0`, `SectionNumber = -2` (IMAGE_SYM_DEBUG), and
    `StorageClass = 6` (IMAGE_SYM_CLASS_LABEL) makes link.exe treat
    them as harmless debug labels with no resolution requirement.

    Returns count of symbols neutralised."""
    sym_off = struct.unpack_from("<I", raw, 8)[0]
    n_syms = struct.unpack_from("<I", raw, 12)[0]
    n = 0
    i = 0
    while i < n_syms:
        base = sym_off + i * 18
        sec_num = struct.unpack_from("<h", raw, base + 12)[0]
        sclass = raw[base + 16]
        n_aux = raw[base + 17]
        if sclass == 2 and sec_num == 0:
            # Move to the IMAGE_SYM_DEBUG section (sec_num = -2);
            # this is a magic value that means "not associated with
            # any output section." Combined with class LABEL (6) link
            # treats it as ignorable debugging information.
            struct.pack_into("<I", raw, base + 8, 0)         # Value = 0
            struct.pack_into("<h", raw, base + 12, -2)       # SectionNumber = IMAGE_SYM_DEBUG
            raw[base + 16] = 6                               # StorageClass = LABEL
            n += 1
        i += 1 + n_aux
    return n


# ----------------------------------------------------------------------
# Per-source manifest persistence.
# ----------------------------------------------------------------------


_MANIFEST_LOCK = threading.Lock()


def update_source_swap_manifest(binary: str, source_path: Path,
                                accepted: list[dict]) -> None:
    """Update the swap manifest with one entry per accepted function in
    a multi-function source file. Adds them as `swaps` entries with
    a `source_obj` field pointing at the swapsrc .obj."""
    manifest_path = SRC / binary / "_passthrough" / "_swap_manifest.json"
    rel_obj = (SWAPSRC_OBJ / binary / f"{source_path.stem}.obj").relative_to(REPO_ROOT)
    with _MANIFEST_LOCK:
        if manifest_path.exists():
            manifest = json.loads(manifest_path.read_text())
        else:
            manifest = {"swaps": [], "source_swaps": []}
        manifest.setdefault("source_swaps", [])
        # Replace any existing entries from this source (so re-running
        # is idempotent).
        manifest["source_swaps"] = [
            s for s in manifest["source_swaps"]
            if s.get("source_obj") != str(rel_obj)
        ]
        for fn in accepted:
            manifest["source_swaps"].append({
                "source_obj": str(rel_obj),
                "rva": fn["rva"],
                "size": fn["size"],
                "symbol": fn["symbol"],
            })
        # Also record (rva, size) entries in `swaps` so emit_text_blob
        # leaves holes.
        existing_rvas = {s["rva"] for s in manifest["swaps"]}
        for fn in accepted:
            if fn["rva"] in existing_rvas:
                continue
            manifest["swaps"].append({
                "fun_name": f"FUN_{fn['rva'] + 0x400000:08x}",
                "rva": fn["rva"],
                "size": fn["size"],
                "from_source": str(source_path.relative_to(REPO_ROOT)),
            })
        manifest["swaps"].sort(key=lambda s: s["rva"])
        manifest["source_swaps"].sort(key=lambda s: s["rva"])
        manifest_path.parent.mkdir(parents=True, exist_ok=True)
        manifest_path.write_text(json.dumps(manifest, indent=2))


def remove_source_swap_manifest(binary: str, source_path: Path) -> None:
    """Remove all entries that originated from this source file."""
    manifest_path = SRC / binary / "_passthrough" / "_swap_manifest.json"
    if not manifest_path.exists():
        return
    rel_src = str(source_path.relative_to(REPO_ROOT))
    rel_obj = str((SWAPSRC_OBJ / binary / f"{source_path.stem}.obj").relative_to(REPO_ROOT))
    with _MANIFEST_LOCK:
        manifest = json.loads(manifest_path.read_text())
        manifest.setdefault("source_swaps", [])
        manifest["source_swaps"] = [
            s for s in manifest["source_swaps"]
            if s.get("source_obj") != rel_obj
        ]
        manifest["swaps"] = [
            s for s in manifest["swaps"]
            if s.get("from_source") != rel_src
        ]
        manifest_path.write_text(json.dumps(manifest, indent=2))


# ----------------------------------------------------------------------
# Driver: process one source file.
# ----------------------------------------------------------------------


def process_source_file(binary: str, source_path: Path,
                        verbose: bool = True,
                        strict: bool = True) -> dict:
    """Compile + match + patch + record. Returns a result dict.

    `strict=True` (default): if ANY .text COMDAT section in the .obj
    fails to byte-match orig (no_match or ambiguous), reject the
    whole file. The skipped sections would still pull unresolved
    external symbol references into the link (memcpy, _imp_*, etc.)
    that we don't satisfy.

    `strict=False`: take the accepted sections, freeze them, mark
    skipped sections with LNK_REMOVE — link.exe still complains
    about unresolved externs from skipped sections in practice.
    Use this only with a follow-up re-build of the .obj to strip
    skipped sections entirely.
    """
    # 1. Compile.
    try:
        obj_path = compile_source_file(binary, source_path)
    except RuntimeError as e:
        return {"source": str(source_path.relative_to(REPO_ROOT)),
                "status": "compile_fail", "error": str(e)[:300]}

    raw = bytearray(obj_path.read_bytes())
    n_sections, sec_off, sym_off, n_syms, str_table_off = _coff_layout(bytes(raw))
    sec_to_sym = _coff_section_to_external_symbol(
        bytes(raw), n_sections, sec_off, sym_off, n_syms, str_table_off
    )

    # 2. Load orig text.
    pe = json.loads((PE_LAYOUT / f"{binary}.json").read_text())
    text_sec = next(s for s in pe["sections"] if s["name"] == ".text")
    orig = (ORIG / f"{binary}.exe").read_bytes()
    text_va = text_sec["virtual_address"]
    orig_text = orig[text_sec["raw_pointer"]:text_sec["raw_pointer"] + text_sec["raw_size"]]

    accepted: list[dict] = []
    skipped: list[dict] = []

    # Source-comment fallback hints: list of (rva, signature) parsed
    # from `// FUNCTION:` lines. Used when exact byte-pattern search
    # finds 0 or >1 matches — we then pick the best-byte-similarity
    # candidate from this list.
    comment_hints = parse_function_comments(source_path)
    candidate_rvas = [rva for rva, _sig in comment_hints]
    # Only include hints that are inside `.text` and have at least
    # `min_size` bytes of orig content — avoids matching against
    # invalid RVAs.
    text_end = text_va + len(orig_text)
    candidate_rvas = [r for r in candidate_rvas if text_va <= r < text_end]

    # 3. For each .text COMDAT section, search orig.
    for sec_idx, name, rsize, raw_ptr, ptr_relocs, n_relocs, chars_off in _coff_walk_text_sections(bytes(raw)):
        if name != ".text":
            # Already a `.text$X<rva>` (from a previous patch) — skip.
            continue
        body = bytes(raw[raw_ptr:raw_ptr + rsize])
        reloc_offs = []
        for j in range(n_relocs):
            e = ptr_relocs + j * 10
            reloc_offs.append(struct.unpack_from("<I", raw, e)[0])
        sym = sec_to_sym.get(sec_idx, "?")
        rvas = find_section_in_orig(body, reloc_offs, orig_text, text_va)

        if len(rvas) == 1:
            rva = rvas[0]
            accepted.append({"sym": sym, "size": rsize, "rva": rva,
                              "symbol": sym, "section_idx": sec_idx,
                              "chars_off": chars_off, "match": "exact"})
            continue

        # 0 or >1 exact matches — fall back to `// FUNCTION:` hints
        # + byte-similarity scoring. If a candidate RVA's orig bytes
        # are clearly the closest match (≥50% byte overlap, ≥5 pp
        # gap to runner-up), accept and freeze. The freeze step
        # replaces our compiled bytes with orig anyway, so this is
        # safe as long as we identify the right RVA.
        if candidate_rvas:
            hit = find_section_via_comment_hint(
                body, reloc_offs, orig_text, text_va, candidate_rvas
            )
            if hit is not None:
                rva, pct = hit
                accepted.append({"sym": sym, "size": rsize, "rva": rva,
                                  "symbol": sym, "section_idx": sec_idx,
                                  "chars_off": chars_off,
                                  "match": f"comment_hint ({pct:.1f}%)"})
                continue

        if len(rvas) == 0:
            skipped.append({"sym": sym, "size": rsize, "reason": "no_match"})
        else:
            skipped.append({"sym": sym, "size": rsize, "reason": "ambiguous", "matches": len(rvas)})

    # 3.5 Strict mode: reject entire file if any function was skipped.
    # The skipped sections pull unresolved external references
    # (memcpy, operator delete, _imp_*) into the link that we can't
    # satisfy without a fake import library. Better to leave such
    # files for byte-passthrough.
    if strict and skipped:
        if verbose:
            print(f"=== {source_path.relative_to(REPO_ROOT)} ===")
            print(f"  REJECTED (strict): {len(accepted)} accepted but {len(skipped)} skipped")
            for fn in skipped[:3]:
                print(f"    skipped: {fn}")
        # Make sure manifest doesn't have stale entries from this file.
        remove_source_swap_manifest(binary, source_path)
        # Also delete the .obj so link doesn't pick it up.
        if obj_path.exists():
            obj_path.unlink()
        return {
            "source": str(source_path.relative_to(REPO_ROOT)),
            "status": "rejected_partial",
            "accepted": [],
            "skipped": skipped,
        }

    # 4. Patch obj: rename each accepted section + force align=1.
    # Also "freeze" the section bytes to orig: fill in the bytes that
    # would otherwise be filled by relocations (since we already know
    # the section matches orig at some RVA, orig's bytes are
    # ground-truth) AND zero out the section's reloc count so link.exe
    # doesn't try to resolve external symbols. Without this, link.exe
    # would fail on any extern reference (memcpy, _imp_*, etc.) that
    # we don't declare via an import library.
    accepted_idx = {fn["section_idx"] for fn in accepted}
    for fn in accepted:
        new_name = f".text$X{fn['rva']:08x}"
        _coff_set_section_name(raw, sec_off, fn["section_idx"], new_name, str_table_off)
        _coff_patch_section_align(raw, fn["chars_off"])
        _coff_freeze_section(raw, fn["section_idx"], fn["rva"], fn["size"],
                             orig, text_va, text_sec)

    # ALSO neutralise every OTHER `.text` COMDAT section (the skipped
    # ones — whose source compiles to bytes that don't byte-match
    # orig). Their unresolved external references would fail the link.
    # Mark them with `IMAGE_SCN_LNK_REMOVE` (0x00000800) which tells
    # link.exe to drop the section entirely; their relocs are not
    # consulted.
    n_sections, sec_off2, _, _, _ = _coff_layout(bytes(raw))
    for sec_idx in range(1, n_sections + 1):
        if sec_idx in accepted_idx:
            continue
        base = sec_off2 + (sec_idx - 1) * 40
        raw_name = bytes(raw[base:base + 8])
        name = raw_name.rstrip(b"\0").decode("ascii", errors="replace")
        # Only neutralise plain `.text` (skipped COMDAT functions); leave
        # `.drectve`, `.debug$*`, `.text$X<rva>` (already-accepted) alone.
        if name != ".text":
            continue
        chars_off = base + 36
        chars = struct.unpack_from("<I", raw, chars_off)[0]
        new_chars = chars | 0x00000800  # IMAGE_SCN_LNK_REMOVE
        struct.pack_into("<I", raw, chars_off, new_chars)
        # Also clear relocs on the dropped section so the link doesn't
        # complain about unresolvables before it gets to drop them.
        struct.pack_into("<I", raw, base + 24, 0)
        struct.pack_into("<H", raw, base + 32, 0)

    if accepted:
        # Neutralise leftover undefined-external symbols. Frozen
        # accepted sections have zeroed relocations + LNK_REMOVE'd
        # skipped sections, so no actual relocation references these
        # symbols anymore — they're symbol-table leftovers from the
        # original (now-skipped) compile path. Flip them to
        # IMAGE_SYM_DEBUG / class LABEL so link.exe ignores.
        n_neutralised = _coff_neutralize_undefined_externs(raw)
        obj_path.write_bytes(bytes(raw))
        if verbose and n_neutralised:
            print(f"  neutralised {n_neutralised} leftover undefined-extern symbol(s)")

    # 5. Update manifest.
    if accepted:
        update_source_swap_manifest(binary, source_path, accepted)

    if verbose:
        print(f"=== {source_path.relative_to(REPO_ROOT)} ===")
        print(f"  accepted: {len(accepted)} fn(s)")
        for fn in accepted:
            tag = fn.get("match", "exact")
            print(f"    rva 0x{fn['rva']:08x} size {fn['size']:>4} [{tag}]  {fn['sym']}")
        if skipped:
            print(f"  skipped: {len(skipped)} fn(s)")
            for fn in skipped[:5]:
                print(f"    {fn}")
    return {
        "source": str(source_path.relative_to(REPO_ROOT)),
        "obj": str(obj_path.relative_to(REPO_ROOT)),
        "accepted": accepted,
        "skipped": skipped,
    }


def cmd_one(binary: str, source: str) -> None:
    src = Path(source).resolve().relative_to(REPO_ROOT) if Path(source).is_absolute() else Path(source)
    src = REPO_ROOT / src
    if not src.exists():
        raise SystemExit(f"source not found: {src}")
    process_source_file(binary, src)


def cmd_all(binary: str) -> None:
    print(f"=== swap_source_file --all ({binary}) ===")
    total_accepted = total_skipped = 0
    for sub in SOURCE_SUBDIRS:
        d = SRC / binary / sub
        if not d.is_dir():
            continue
        for cpp in sorted(d.glob("*.cpp")):
            res = process_source_file(binary, cpp, verbose=True)
            if "accepted" in res:
                total_accepted += len(res["accepted"])
                total_skipped += len(res["skipped"])
            elif res.get("status") == "compile_fail":
                print(f"  ✗ compile fail: {res['error'][:120]}")
    print()
    print(f"TOTAL: {total_accepted} accepted, {total_skipped} skipped")
    if total_accepted:
        print(f"\nNext: `make relink BINARY={binary}.exe` to bake swaps.")


def cmd_list(binary: str) -> None:
    manifest_path = SRC / binary / "_passthrough" / "_swap_manifest.json"
    if not manifest_path.exists():
        print(f"  no manifest for {binary}")
        return
    manifest = json.loads(manifest_path.read_text())
    src_swaps = manifest.get("source_swaps", [])
    by_obj: dict[str, list[dict]] = {}
    for s in src_swaps:
        by_obj.setdefault(s["source_obj"], []).append(s)
    print(f"  {len(src_swaps)} source-file swaps in {len(by_obj)} obj(s):")
    for obj, fns in sorted(by_obj.items()):
        total = sum(f["size"] for f in fns)
        print(f"    {obj}: {len(fns)} fn(s), {total} bytes")
        for f in fns:
            print(f"      rva 0x{f['rva']:08x} size {f['size']:>4}  {f['symbol']}")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", help="binary stem")
    ap.add_argument("source", nargs="?", help="path to .cpp source (or --all)")
    ap.add_argument("--all", action="store_true", help="walk all hand-written src dirs")
    ap.add_argument("--list", action="store_true", help="list recorded source-file swaps")
    args = ap.parse_args()

    if args.list:
        cmd_list(args.binary)
        return 0
    if args.all:
        cmd_all(args.binary)
        return 0
    if not args.source:
        ap.error("either pass <source.cpp>, --all, or --list")
    cmd_one(args.binary, args.source)
    return 0


if __name__ == "__main__":
    sys.exit(main())
