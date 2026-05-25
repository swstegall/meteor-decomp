#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Import the FFXIVLegacyClientStructs RTTI struct dump into meteor-decomp's
config catalog.

FFXIVLegacyClientStructs (github.com/Yokimitsuro/FFXIVLegacyClientStructs)
is an independent reverse-engineering of the same 1.23b ffxivgame.exe we
target. It ships ~2,572 [StructLayout]-explicit C# structs (one or more
per .cs file) carrying:
  - [Rtti("<mangled>")]                       MSVC RTTI mangled name
  - [StructLayout(LayoutKind.Explicit, Size=) struct size in bytes
  - [FieldOffset(0xNN)] public <type> <Name>; per-field offset + name
  - header comments // VTable: // Ctor: // Size: // Alloc: + inheritance
  - // +0xNNN: sub-object (ctor 0x...) embedded sub-object hints
plus ffxiv_1.0_rtti.txt: 4,358 rows of
  Demangled | VTable VA | VFunc Count | TypeDescriptor VA | Mangled

meteor-decomp's own config/ffxivgame.rtti.json already has the vtable
class list at parity (its Ghidra RTTI walk recovers ~4,392 classes), but
it carries NO per-struct field layouts and NO struct sizes. This tool
harvests exactly those gaps:

  - config/ffxivgame.legacy_structs.json   the full machine catalog
      [{class, mangled, size, field_count, vtable_va, vtable_rva,
        slot_count, ctor_va, ctor_rva, alloc_site, source,
        inherits[], fields[{offset, type, name, comment}],
        subobjects[{offset, ctor_va, note}]}]

All addresses are absolute VAs against image base 0x00400000; *_rva
fields are VA - 0x00400000 to match meteor-decomp's config convention.

Usage:
  python3 tools/import_legacy_structs.py \\
      [--legacy-root ../FFXIVLegacyClientStructs] \\
      [--binary ffxivgame] [--out config/ffxivgame.legacy_structs.json]
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
IMAGE_BASE = 0x00400000

# ---- regexes -------------------------------------------------------------

RE_NAMESPACE = re.compile(r"^\s*namespace\s+([A-Za-z0-9_.]+)\s*;", re.MULTILINE)
RE_RTTI = re.compile(r'\[Rtti\("([^"]+)"\)\]')
RE_STRUCTLAYOUT = re.compile(
    r"\[StructLayout\([^)]*?Size\s*=\s*(0x[0-9A-Fa-f]+|\d+)", re.IGNORECASE
)
RE_STRUCT_DECL = re.compile(
    r"^\s*public\s+(?:unsafe\s+)?(?:readonly\s+)?(?:partial\s+)?struct\s+([A-Za-z0-9_]+)"
)
# [FieldOffset(0x154)] public float PositionX;          // trailing comment
# [FieldOffset(0x1690)] public fixed uint BattleTempStats[10];
RE_FIELD = re.compile(
    r"\[FieldOffset\((0x[0-9A-Fa-f]+|\d+)\)\]\s*"
    r"public\s+(?:readonly\s+)?(fixed\s+)?([A-Za-z0-9_:<>,*\s.]+?)\s+"
    r"([A-Za-z0-9_]+)\s*(\[[^\]]*\])?\s*;"
    r"(?:\s*//\s*(.*))?$"
)
# header comments
RE_C_VTABLE = re.compile(r"//\s*VTable:\s*(0x[0-9A-Fa-f]+)")
RE_C_CTOR = re.compile(r"//\s*Ctor:\s*(0x[0-9A-Fa-f]+)")
RE_C_SIZE = re.compile(r"//\s*Size:\s*(0x[0-9A-Fa-f]+)")
RE_C_ALLOC = re.compile(r"//\s*Alloc:\s*(.+?)\s*$")
RE_C_CPPNAME = re.compile(r"^\s*//\s*([A-Za-z_][A-Za-z0-9_]*(?:::[A-Za-z0-9_]+)+)\s*$")
# // +0x2858 CharaActionController (vt=0x0103E468)
# // +0x0590: sub-object (ctor 0x007B72A0) — large block
RE_C_SUBOBJ = re.compile(
    r"//\s*\+(0x[0-9A-Fa-f]+):?\s*(.*?)(?:\(ctor\s+(0x[0-9A-Fa-f]+)\))?"
    r"(?:\(vt=(0x[0-9A-Fa-f]+)\))?\s*$"
)


def hexint(s: str) -> int:
    return int(s, 16) if s.lower().startswith("0x") else int(s)


def parse_rtti_txt(path: Path) -> dict[str, dict]:
    """mangled -> {demangled, vtable_va, vfunc_count, td_va}."""
    out: dict[str, dict] = {}
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if line.startswith("//") or "|" not in line:
            continue
        parts = [p.strip() for p in line.split("|")]
        if len(parts) < 5:
            continue
        demangled, vtva, vfc, tdva, mangled = parts[:5]
        rec = {"demangled": demangled}
        if vtva.lower().startswith("0x"):
            rec["vtable_va"] = hexint(vtva)
        if vfc.isdigit():
            rec["vfunc_count"] = int(vfc)
        if tdva.lower().startswith("0x"):
            rec["td_va"] = hexint(tdva)
        out[mangled] = rec
    return out


def split_struct_blocks(text: str):
    """Yield (header_comment_lines, attr_lines, struct_name, body_lines).

    A .cs file may declare several structs. We segment on each
    `public ... struct <Name>` line, attaching the comment + attribute
    lines that immediately precede it.
    """
    lines = text.splitlines()
    # find struct decl line indices
    decls = [i for i, ln in enumerate(lines) if RE_STRUCT_DECL.match(ln)]
    for k, di in enumerate(decls):
        name = RE_STRUCT_DECL.match(lines[di]).group(1)
        # preamble: walk backwards over //-comments and [Attr] lines
        j = di - 1
        pre: list[str] = []
        while j >= 0:
            s = lines[j].strip()
            if s.startswith("//") or s.startswith("[") or s == "":
                pre.append(lines[j])
                j -= 1
            else:
                break
        pre.reverse()
        # body: from the struct decl to the next decl (or EOF)
        end = decls[k + 1] - 1 if k + 1 < len(decls) else len(lines)
        # back the end off any preamble belonging to the next struct
        body = lines[di:end]
        yield pre, name, body


def parse_struct(pre: list[str], name: str, body: list[str], namespace: str):
    rec: dict = {"name": name, "namespace": namespace}
    blob_pre = "\n".join(pre)
    # attributes (may be in preamble)
    m = RE_RTTI.search(blob_pre) or RE_RTTI.search("\n".join(body[:3]))
    if m:
        rec["mangled"] = m.group(1)
    m = RE_STRUCTLAYOUT.search(blob_pre) or RE_STRUCTLAYOUT.search("\n".join(body[:3]))
    if m:
        rec["size"] = hexint(m.group(1))
    # header comments
    for rx, key in ((RE_C_VTABLE, "vtable_va_comment"), (RE_C_CTOR, "ctor_va"),
                    (RE_C_SIZE, "size_comment"), (RE_C_ALLOC, "alloc_site")):
        mm = rx.search(blob_pre)
        if mm:
            v = mm.group(1)
            rec[key] = hexint(v) if v.startswith("0x") else v.strip()
    # C++ demangled name from the leading `// Ns::Class` comment
    for ln in pre:
        cm = RE_C_CPPNAME.match(ln)
        if cm:
            rec["cpp_name_comment"] = cm.group(1)
            break
    # fields + sub-objects
    fields = []
    subobjs = []
    for ln in body:
        fm = RE_FIELD.search(ln)
        if fm:
            off, fixed, ftype, fname, arr, comment = fm.groups()
            f = {"offset": hexint(off), "type": (ftype or "").strip(), "name": fname}
            if fixed:
                f["type"] = "fixed " + f["type"]
            if arr:
                f["array"] = arr
            if comment:
                f["comment"] = comment.strip()
            fields.append(f)
            continue
        # only treat as sub-object hint if it mentions ctor= or vt=
        if "sub-object" in ln or "(ctor 0x" in ln or "(vt=0x" in ln:
            sm = RE_C_SUBOBJ.search(ln)
            if sm and (sm.group(3) or sm.group(4)):
                so = {"offset": hexint(sm.group(1))}
                note = (sm.group(2) or "").strip(" -—:")
                if note:
                    so["note"] = note
                if sm.group(3):
                    so["ctor_va"] = hexint(sm.group(3))
                if sm.group(4):
                    so["vt_va"] = hexint(sm.group(4))
                subobjs.append(so)
    rec["fields"] = fields
    rec["field_count"] = len(fields)
    if subobjs:
        rec["subobjects"] = subobjs
    return rec


PROVENANCE = (
    "> **Imported from FFXIVLegacyClientStructs** "
    "(github.com/Yokimitsuro/FFXIVLegacyClientStructs) — an independent RTTI\n"
    "> reverse-engineering of the same 1.23b `ffxivgame.exe`. This layout is\n"
    "> **cross-referenced, NOT byte-verified** against our own asm. Treat it as a\n"
    "> strong hint while matching; confirm offsets against the function you're on.\n"
)


def render_note(s: dict) -> str:
    cls = s["class"]
    out = [f"# `{cls}` — object layout (imported)", "", PROVENANCE, ""]
    meta = []
    if "size" in s:
        meta.append(f"- **Size**: 0x{s['size']:x} ({s['size']} bytes)")
    if "vtable_va" in s:
        sc = f", {s['slot_count']} vfuncs" if "slot_count" in s else ""
        meta.append(f"- **VTable**: 0x{s['vtable_va']:08x} (RVA 0x{s['vtable_rva']:08x}{sc})")
    if "ctor_rva" in s:
        meta.append(f"- **Ctor**: 0x{s['ctor_va']:08x} (RVA 0x{s['ctor_rva']:08x}) — "
                    f"names `FUN_{0x400000 + s['ctor_rva']:08x}` in `config/{{bin}}.symbols.json`")
    if "alloc_site" in s:
        meta.append(f"- **Alloc**: {s['alloc_site']}")
    if "mangled" in s:
        meta.append(f"- **RTTI**: `{s['mangled']}`")
    meta.append(f"- **Source**: `FFXIVLegacyClientStructs/{s['source']}`")
    out += meta + ["", "## Fields", ""]
    out.append("| Offset | Type | Name | Note |")
    out.append("|--------|------|------|------|")
    for f in s["fields"]:
        arr = f.get("array", "")
        nm = f["name"] + arr
        out.append(f"| `0x{f['offset']:04x}` | `{f['type']}` | `{nm}` | {f.get('comment','')} |")
    if s.get("subobjects"):
        out += ["", "## Embedded sub-objects", "",
                "| Offset | Ctor | VTable | Note |", "|--------|------|--------|------|"]
        for so in s["subobjects"]:
            c = f"0x{so['ctor_va']:08x}" if "ctor_va" in so else ""
            v = f"0x{so['vt_va']:08x}" if "vt_va" in so else ""
            out.append(f"| `0x{so['offset']:04x}` | {c} | {v} | {so.get('note','')} |")
    out.append("")
    return "\n".join(out)


def emit_notes(structs: list[dict], binary: str, min_fields: int = 4) -> None:
    types_dir = REPO_ROOT / "decomp-notes" / "types" / binary
    types_dir.mkdir(parents=True, exist_ok=True)
    ctor_keyed = [s for s in structs if "ctor_rva" in s]
    rich_no_ctor = [s for s in structs
                    if "ctor_rva" not in s and s["field_count"] >= min_fields]
    wrote, skipped = 0, 0
    marker = "Imported from FFXIVLegacyClientStructs"
    for s in ctor_keyed:
        path = types_dir / f"0x{s['ctor_rva']:08x}.md"
        if path.exists() and marker not in path.read_text(encoding="utf-8", errors="replace"):
            skipped += 1  # genuine hand-written note — never clobber
            continue
        path.write_text(render_note(s).replace("{bin}", binary), encoding="utf-8")
        wrote += 1
    # rich structs without a ctor RVA -> one consolidated index (not keyed to a fn)
    if rich_no_ctor:
        idx = [f"# Imported rich struct layouts — `{binary}` (no ctor RVA)", "",
               PROVENANCE, "",
               "These classes carry field layouts but no recovered ctor RVA, so they",
               "aren't keyed to a function you'd match directly. Anchored by vtable RVA.",
               "Full machine catalog: `config/{}.legacy_structs.json`.".format(binary), ""]
        for s in sorted(rich_no_ctor, key=lambda x: -x["field_count"]):
            idx.append("---\n")
            idx.append(render_note(s).replace("{bin}", binary))
        (types_dir / "_legacy_imported.md").write_text("\n".join(idx), encoding="utf-8")
    print(f"\n  type notes written ........ {wrote} ctor-keyed"
          f" (+{len(rich_no_ctor)} in _legacy_imported.md, {skipped} skipped: hand-written exists)")


def emit_symbols(structs: list[dict], binary: str, sym_path: Path) -> None:
    syms = {s["rva"]: s["name"] for s in json.loads(sym_path.read_text())} if sym_path.exists() else {}
    rows = []
    for s in structs:
        if "ctor_rva" not in s:
            continue
        rows.append({
            "rva": s["ctor_rva"],
            "rva_hex": f"0x{s['ctor_rva']:08x}",
            "name": f"{s['class']}::ctor",
            "kind": "ctor",
            "current_name": syms.get(s["ctor_rva"], ""),
            "auto_named": bool(re.match(r"(FUN_|thunk_FUN_|LAB_|SUB_)", syms.get(s["ctor_rva"], ""))),
        })
    out = REPO_ROOT / "config" / f"{binary}.legacy_symbols.json"
    out.write_text(json.dumps(rows, indent=1), encoding="utf-8")
    print(f"  legacy_symbols.json ....... {len(rows)} ctor names "
          f"({sum(1 for r in rows if r['auto_named'])} currently auto-named)")


def validate_vtables(structs: list[dict], binary: str) -> None:
    rtti_path = REPO_ROOT / "config" / f"{binary}.rtti.json"
    if not rtti_path.exists():
        return
    md_by_rva = {e["rva"]: e["class"] for e in json.loads(rtti_path.read_text())}
    agree = mismatch = only_legacy = 0
    samples = []
    for s in structs:
        if "vtable_rva" not in s:
            continue
        ours = md_by_rva.get(s["vtable_rva"])
        if ours is None:
            only_legacy += 1
        elif ours == s["class"]:
            agree += 1
        else:
            mismatch += 1
            if len(samples) < 25:
                samples.append((s["vtable_rva"], s["class"], ours))
    report = [f"# Legacy struct import — vtable name cross-validation (`{binary}`)", "",
              "Compares FFXIVLegacyClientStructs vtable RVA → class name against our",
              f"own Ghidra RTTI walk in `config/{binary}.rtti.json`.", "",
              f"- agree ........... {agree}",
              f"- mismatch ........ {mismatch}",
              f"- legacy-only RVA . {only_legacy} (vtable RVA not in our rtti.json)", ""]
    if samples:
        report += ["## Mismatches (sample)", "",
                   "| VTable RVA | FFXIVLegacyClientStructs | meteor-decomp |",
                   "|------------|--------------------------|----------------|"]
        for rva, leg, ours in samples:
            report.append(f"| `0x{rva:08x}` | `{leg}` | `{ours}` |")
        report.append("")
    out = REPO_ROOT / "build" / f"{binary}.legacy_struct_import_report.md"
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(report), encoding="utf-8")
    print(f"  vtable validation ......... agree={agree} mismatch={mismatch} "
          f"legacy-only={only_legacy}  -> build/{out.name}")


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    ap.add_argument("--legacy-root", default=str(REPO_ROOT.parent / "FFXIVLegacyClientStructs"))
    ap.add_argument("--binary", default="ffxivgame")
    ap.add_argument("--out", default=None)
    ap.add_argument("--symbols", default=None,
                    help="config/<binary>.symbols.json (for FUN_xxx ctor cross-check)")
    ap.add_argument("--emit", action="store_true",
                    help="also emit type notes, legacy_symbols.json, and a vtable validation report")
    ap.add_argument("--min-fields", type=int, default=4,
                    help="emit a type note for structs with >= this many fields (default 4)")
    args = ap.parse_args()

    legacy_root = Path(args.legacy_root)
    src_root = legacy_root / "FFXIVClientStructs"
    rtti_txt = legacy_root / "ffxiv_1.0_rtti.txt"
    if not src_root.is_dir():
        print(f"ERROR: {src_root} not found", file=sys.stderr)
        return 1

    out_path = Path(args.out) if args.out else (REPO_ROOT / "config" / f"{args.binary}.legacy_structs.json")
    sym_path = Path(args.symbols) if args.symbols else (REPO_ROOT / "config" / f"{args.binary}.symbols.json")

    rtti = parse_rtti_txt(rtti_txt) if rtti_txt.exists() else {}
    print(f"rtti.txt rows (by mangled): {len(rtti)}")

    structs = []
    for cs in sorted(src_root.rglob("*.cs")):
        text = cs.read_text(encoding="utf-8", errors="replace")
        nm = RE_NAMESPACE.search(text)
        namespace = nm.group(1) if nm else ""
        rel = cs.relative_to(legacy_root).as_posix()
        for pre, name, body in split_struct_blocks(text):
            rec = parse_struct(pre, name, body, namespace)
            rec["source"] = rel
            # join to rtti.txt by mangled name -> canonical demangled + vtable VA
            j = rtti.get(rec.get("mangled", ""))
            if j:
                rec["class"] = j.get("demangled")
                if "vtable_va" in j:
                    rec["vtable_va"] = j["vtable_va"]
                    rec["vtable_rva"] = j["vtable_va"] - IMAGE_BASE
                if "vfunc_count" in j:
                    rec["slot_count"] = j["vfunc_count"]
                if "td_va" in j:
                    rec["td_va"] = j["td_va"]
            else:
                # no rtti.txt join: prefer a leading `// Ns::Class` comment,
                # else synthesize a C++-style name from the C# namespace
                # (drop the FFXIVClientStructs.* wrapper, dots -> ::).
                cpp_ns = re.sub(r"^FFXIVClientStructs\.", "", namespace).replace(".", "::")
                rec["class"] = rec.get("cpp_name_comment") or (
                    f"{cpp_ns}::{name}" if cpp_ns else name)
                if "vtable_va_comment" in rec:
                    rec["vtable_va"] = rec["vtable_va_comment"]
                    rec["vtable_rva"] = rec["vtable_va_comment"] - IMAGE_BASE
            if "ctor_va" in rec and isinstance(rec["ctor_va"], int):
                rec["ctor_rva"] = rec["ctor_va"] - IMAGE_BASE
            structs.append(rec)

    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(structs, indent=1), encoding="utf-8")

    if args.emit:
        emit_notes(structs, args.binary, args.min_fields)
        emit_symbols(structs, args.binary, sym_path)
        validate_vtables(structs, args.binary)

    # ---- summary -------------------------------------------------------
    n = len(structs)
    with_size = sum(1 for s in structs if "size" in s)
    with_fields = sum(1 for s in structs if s["field_count"] > 0)
    rich = sum(1 for s in structs if s["field_count"] >= 4)
    with_vt = sum(1 for s in structs if "vtable_va" in s)
    with_ctor = sum(1 for s in structs if "ctor_rva" in s)
    joined = sum(1 for s in structs if s.get("mangled") in rtti)
    total_fields = sum(s["field_count"] for s in structs)

    print(f"\nwrote {out_path.relative_to(REPO_ROOT)}")
    print(f"  structs parsed ............ {n}")
    print(f"  joined to rtti.txt ........ {joined}")
    print(f"  with struct size .......... {with_size}")
    print(f"  with vtable VA ............ {with_vt}")
    print(f"  with ctor RVA ............. {with_ctor}")
    print(f"  with >=1 field ............ {with_fields}")
    print(f"  rich (>=4 fields) ......... {rich}")
    print(f"  total fields .............. {total_fields}")

    # ctor cross-check vs symbols.json: which ctors are still FUN_xxx?
    if sym_path.exists():
        syms = {s["rva"]: s["name"] for s in json.loads(sym_path.read_text())}
        ctor_recs = [s for s in structs if "ctor_rva" in s]
        namable = [s for s in ctor_recs
                   if re.match(r"(FUN_|thunk_FUN_|LAB_|SUB_)", syms.get(s["ctor_rva"], ""))]
        print(f"\n  ctor RVAs present ......... {len(ctor_recs)}")
        print(f"  ctor still auto-named ..... {len(namable)} (namable wins)")
        for s in namable[:20]:
            print(f"    0x{s['ctor_rva']:08x}  {syms.get(s['ctor_rva'])}  ->  {s['class']}::ctor")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
