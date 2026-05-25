#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Import ffxivDecomp's opcode + Lua-binding findings into meteor-decomp.

ffxivDecomp (github.com/Yokimitsuro/ffxivDecomp) is a docs-only RE of the
same 1.23b ffxivgame.exe. Several of its findings name functions by VA
that we currently carry as FUN_xxxxxxxx. This tool harvests those
(name, VA) pairs from the finding docs, joins them to our config catalog
(VA - 0x00400000 = RVA), and emits a symbol-name enrichment + an opcode
cross-validation.

Sources parsed (under <ffxivDecomp>/docs/re/):
  exe/finding_zone_outbound_opcode_roster.md      "Ghidra Annotations Made" block (17 senders/builders)
  exe/finding_playerbase_lua_bindings_39_of_94_named.md   per-family registrar tables (39 bindings)
  correlation/finding_lua_api_to_zone_opcode_systematic_xref.md   (irregular -> hand-seeded below)

Emits:
  config/ffxivgame.ffxivdecomp_symbols.json   [{rva, rva_hex, name, kind,
      opcode, lua_binding, current_name, auto_named, source}]
  docs/ffxivdecomp_opcode_binding_map.md       human reference (roster + binding map)
  build/ffxivgame.ffxivdecomp_import_report.md  validation report (gitignored)

Every VA is cross-checked against config/ffxivgame.symbols.json: a pair
whose VA does not resolve to a known function is dropped and flagged
(catches doc-parse / transcription errors). Opcodes are cross-validated
against config/ffxivgame.up_opcodes.json.

Usage:
  python3 tools/import_ffxivdecomp_symbols.py [--ffxivdecomp-root ../ffxivDecomp]
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
IMAGE_BASE = 0x00400000

# Irregular correlation-doc entries (name @ 0xVA on wrapped lines) +
# the channel dispatcher — hand-seeded from
# correlation/finding_lua_api_to_zone_opcode_systematic_xref.md and
# exe/finding_complete_3channel_opcode_inventory.md because their tables
# wrap the VA onto a continuation line (not cleanly regex-able).
HAND_SEED = [
    # Lua-binding outbound implementations (Lua action -> Zone opcode)
    ("0x006e85e0", "lua_updateWork_impl", "lua_impl", "0x12f", "_updateWork"),
    ("0x00705eb0", "Lua_queryBinding_dispatchType_sends_0x135", "lua_impl", "0x135", "_queryBinding"),
    ("0x006e6360", "Lua_sendChallenge_via_opcode_0x134", "lua_impl", "0x134", "_sendChallenge"),
    ("0x006e5ad0", "Lua_sendByteToggle_via_opcode_0x131", "lua_impl", "0x131", "_sendByteToggle"),
    ("0x006dacd0", "Lua_listObjectDelete_sends_0x130_variantA", "lua_impl", "0x130", "_sendListObjectDelete"),
    ("0x006dae90", "Lua_listObjectQueueAdd_sends_0x130_variantA", "lua_impl", "0x130", "_sendListObjectQueueAdd"),
    ("0x006e42e0", "Lua_listIndexSend_via_0x130_variantA", "lua_impl", "0x130", "_sendListIndex"),
    ("0x006e2130", "Lua_send8byteStateAt0x68_via_0x130_variantB", "lua_impl", "0x130", "_sendMovementState"),
    ("0x006e2af0", "Lua_sendByteUshortAt0x68_via_0x132", "lua_impl", "0x132", "_sendCompoundState"),
    ("0x00894090", "Lua_send6argRpc_via_opcode_0x12e", "lua_impl", "0x12e", "_send6argRpc"),
    ("0x006c72e0", "WorkSyncAlt_serializePayloadAndSend_opcode_0x133", "lua_impl", "0x133", "_updateWorkAlt"),
    ("0x006e6d90", "Lua_worldMaster__lookAtPlayerTutorial", "lua_impl", None, "_lookAtPlayerTutorial"),
    # NpcBase server-callback registrars
    ("0x00736fc0", "NpcBaseClass_registerLua_callServerOnTalk", "registrar", None, "_callServerOnTalk"),
    ("0x00737110", "NpcBaseClass_registerLua_callServerOnEmote", "registrar", None, "_callServerOnEmote"),
    ("0x00737260", "NpcBaseClass_registerLua_callServerOnPush", "registrar", None, "_callServerOnPush"),
    # Channel dispatcher (shared across all 3 channels)
    ("0x00db5300", "ProtoChannel_dispatchPacketById", "dispatcher", None, None),
    # Inbound data-packet receiver region (0x0089exxx) — SEQ-005 kick-gate
    # neighbourhood. Named in ffxivDecomp prose (finding_polymorphic_block_
    # userdataReceiver.md), not a rename block, so hand-seeded. This is the
    # same cluster as meteor-decomp's kick-gate FUN_0089f180 (parser) /
    # FUN_0089e200 (setter of receiver+0x80). See the SEQ-005 synthesis note.
    ("0x0089eed0", "Network_UserDataReceiver_ctor", "receiver", None, None),
    ("0x0089fbf0", "Network_UserDataReceiver_multiModeDispatcher", "dispatcher", None, None),
    # NOTE: the MyPlayer vtable member-fn bodies for the Lua bindings
    # (incl. the SEQ-005 clearer FUN_006e32f0 = _fadeInNowLoadingForNotice...
    # and FUN_006e8f50 = _cancelNotice) are owned by the dedicated
    # tools/map_playerbase_vtable_slots.py (all 39 bindings, anchored +
    # self-validating), not hand-seeded here.
]

RE_HEX = re.compile(r"0x[0-9a-fA-F]{6,8}")
# curated "RENAMES" annotation lines across all docs: `- 0xVA -> Name`
RE_RENAME = re.compile(r"^\s*-\s*(0x[0-9a-fA-F]{6,8})\s*->\s*([A-Za-z_]\w+)")


def classify(name: str) -> str:
    """Infer a kind from the ffxivDecomp Ghidra name prefix."""
    if "_invokeLua_" in name:
        return "inbound_invoker"
    if "_registerLua_" in name or name.endswith("registerAllLuaBindings"):
        return "registrar"
    if name.startswith("ZoneIn_handler") or name.startswith("ZoneOut_"):
        return "opcode_handler"
    if name.startswith("Router_"):
        return "router"
    if "vtable_slot" in name or name.endswith("_noop_inherited"):
        return "receiver_slot"
    if name.startswith("ChatBuilder") or "Chat" in name:
        return "chat"
    if name.startswith("Functor_"):
        return "functor"
    if "dispatch" in name.lower() or "Dispatcher" in name:
        return "dispatcher"
    return "named"


def parse_renames(re_root: Path):
    """Yield (va, name, kind, opcode, lua, src) from every `- 0xVA -> Name`
    annotation line across all finding docs under docs/re/."""
    for md in sorted(re_root.rglob("*.md")):
        for line in md.read_text(encoding="utf-8", errors="replace").splitlines():
            m = RE_RENAME.match(line)
            if m:
                va, name = m.group(1), m.group(2)
                mo = re.search(r"opcode_(0x[0-9a-fA-F]+)", name)
                yield va, name, classify(name), (mo.group(1) if mo else None), None, \
                    md.relative_to(re_root).as_posix()
# roster "Ghidra Annotations Made": `0xVA  Name`
RE_ROSTER = re.compile(r"^\s*(0x[0-9a-fA-F]{6,8})\s+([A-Za-z_]\w+)\s*$")
# PlayerBase family tables: `_luaName   0xVA   LAB_xxx|0xthunk   notes`
RE_PB = re.compile(r"^\s*(_\w+)\s+(0x[0-9a-fA-F]{6,8})\s+(?:LAB_\w+|0x[0-9a-fA-F]+)")


def hexint(s: str) -> int:
    return int(s, 16)


def parse_roster(path: Path):
    """Yield (va, name, kind, opcode, lua_binding) from the annotations block."""
    if not path.exists():
        return
    in_block = False
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        if "Ghidra Annotations Made" in line:
            in_block = True
            continue
        if in_block and line.startswith("## "):
            break
        if not in_block:
            continue
        m = RE_ROSTER.match(line)
        if m:
            va, name = m.group(1), m.group(2)
            mo = re.search(r"opcode_(0x[0-9a-fA-F]+)", name)
            opcode = mo.group(1) if mo else ("0x12d" if "large" in name else None)
            yield va, name, "opcode_sender", opcode, None


def parse_playerbase(path: Path):
    """Yield (va, name, kind, opcode, lua_binding) for PlayerBase registrars."""
    if not path.exists():
        return
    for line in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = RE_PB.match(line)
        if m:
            lua, va = m.group(1), m.group(2)
            name = f"PlayerBase_registerLua_{lua.lstrip('_')}"
            yield va, name, "registrar", None, lua


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--ffxivdecomp-root", default=str(REPO_ROOT.parent / "ffxivDecomp"))
    ap.add_argument("--binary", default="ffxivgame")
    args = ap.parse_args()

    re_root = Path(args.ffxivdecomp_root) / "docs" / "re"
    syms = {s["rva"]: s["name"] for s in json.loads(
        (REPO_ROOT / "config" / f"{args.binary}.symbols.json").read_text())}

    # gather (va, name, kind, opcode, lua) from all sources, dedup by va
    rows: dict[int, dict] = {}
    sources = []

    def add(va_s, name, kind, opcode, lua, src):
        va = hexint(va_s)
        rva = va - IMAGE_BASE
        if va in {hexint(x) for x in []}:  # noop guard
            return
        rec = {
            "rva": rva, "rva_hex": f"0x{rva:08x}", "va_hex": f"0x{va:08x}",
            "name": name, "kind": kind, "source": src,
        }
        if opcode:
            rec["opcode"] = opcode
        if lua:
            rec["lua_binding"] = lua
        rows.setdefault(va, rec)

    for va, name, kind, op, lua in parse_roster(re_root / "exe" / "finding_zone_outbound_opcode_roster.md"):
        add(va, name, kind, op, lua, "finding_zone_outbound_opcode_roster.md")
    for va, name, kind, op, lua in parse_playerbase(re_root / "exe" / "finding_playerbase_lua_bindings_39_of_94_named.md"):
        add(va, name, kind, op, lua, "finding_playerbase_lua_bindings_39_of_94_named.md")
    # master block + hand-seed
    add("0x00753f90", "PlayerBase_registerAllLuaBindings", "master_block", None, None,
        "finding_playerbase_lua_bindings_39_of_94_named.md")
    for va, name, kind, op, lua in HAND_SEED:
        add(va, name, kind, op, lua, "finding_lua_api_to_zone_opcode_systematic_xref.md")
    # bulk harvest: every curated `- 0xVA -> Name` rename across all docs
    for va, name, kind, op, lua, src in parse_renames(re_root):
        add(va, name, kind, op, lua, src)

    # cross-check vs symbols.json: keep only RVAs that resolve to a function
    kept, dropped = [], []
    for va, rec in sorted(rows.items()):
        cur = syms.get(rec["rva"])
        if cur is None:
            dropped.append(rec)  # parse/transcription error or label-not-function (thunks)
            continue
        rec["current_name"] = cur
        rec["auto_named"] = bool(re.match(r"(FUN_|thunk_FUN_|LAB_|SUB_)", cur))
        kept.append(rec)

    out = REPO_ROOT / "config" / f"{args.binary}.ffxivdecomp_symbols.json"
    out.write_text(json.dumps(kept, indent=1), encoding="utf-8")

    namable = sum(1 for r in kept if r["auto_named"])
    by_kind: dict[str, int] = {}
    for r in kept:
        by_kind[r["kind"]] = by_kind.get(r["kind"], 0) + 1

    print(f"parsed pairs ............... {len(rows)}")
    print(f"resolved to a function ..... {len(kept)}")
    print(f"  still auto-named (FUN_) .. {namable} (namable wins)")
    print(f"  already human-named ...... {len(kept) - namable}")
    print(f"  by kind .................. {by_kind}")
    print(f"dropped (no function@RVA) .. {len(dropped)}")
    for r in dropped:
        print(f"    {r['va_hex']}  {r['name']}")

    # opcode cross-validation vs up_opcodes.json
    up = json.loads((REPO_ROOT / "config" / f"{args.binary}.up_opcodes.json").read_text())
    up_ops = set()
    for v in up.get("ctor_callers", {}).values():
        for c in v.get("calls", []):
            if c.get("opcode") is not None:
                up_ops.add(c["opcode"])
    fd_ops = {hexint(r["opcode"]) for r in kept if "opcode" in r}
    agree = sorted(up_ops & fd_ops)
    fd_only = sorted(fd_ops - up_ops)

    write_report(args.binary, kept, dropped, namable, by_kind, up_ops, fd_ops)
    write_reference_doc(args.binary, kept)

    print(f"\nopcode cross-validation vs up_opcodes.json:")
    print(f"  agree ........ {[hex(o) for o in agree]}")
    print(f"  ffxivDecomp-only (net-new to our extraction) .. {[hex(o) for o in fd_only]}")
    print(f"\nwrote config/{out.name}, docs/ffxivdecomp_opcode_binding_map.md, build/ report")
    return 0


def write_report(binary, kept, dropped, namable, by_kind, up_ops, fd_ops):
    out = REPO_ROOT / "build" / f"{binary}.ffxivdecomp_import_report.md"
    out.parent.mkdir(parents=True, exist_ok=True)
    L = [f"# ffxivDecomp symbol import — report (`{binary}`)", "",
         f"- resolved functions: {len(kept)} ({namable} still FUN_xxx, namable)",
         f"- by kind: {by_kind}",
         f"- dropped (no function at RVA): {len(dropped)}", ""]
    if dropped:
        L += ["## Dropped (VA did not resolve to a known function)", "",
              "These are either thunk LABELs (not promoted to functions) or "
              "doc-parse misses — review before trusting.", ""]
        for r in dropped:
            L.append(f"- `{r['va_hex']}` {r['name']} ({r['source']})")
        L.append("")
    L += ["## Opcode cross-validation vs up_opcodes.json", "",
          f"- agree: {sorted(hex(o) for o in (up_ops & fd_ops))}",
          f"- ffxivDecomp-only: {sorted(hex(o) for o in (fd_ops - up_ops))}",
          f"- up_opcodes-only: {sorted(hex(o) for o in (up_ops - fd_ops))}", ""]
    out.write_text("\n".join(L), encoding="utf-8")


def write_reference_doc(binary, kept):
    out = REPO_ROOT / "docs" / "ffxivdecomp_opcode_binding_map.md"
    L = ["# ffxivDecomp opcode + Lua-binding map (imported)", "",
         "> **Imported from ffxivDecomp** (github.com/Yokimitsuro/ffxivDecomp), an",
         "> independent docs-only RE of the same 1.23b `ffxivgame.exe`. Names are",
         "> **cross-referenced, not byte-verified** here; confirm against the asm",
         "> when matching. Regenerate with `tools/import_ffxivdecomp_symbols.py`.",
         "", "All VAs are absolute (image base 0x00400000); RVA = VA - 0x400000.",
         "Machine form: `config/" + binary + ".ffxivdecomp_symbols.json`.", "",
         "| RVA | name | kind | opcode | Lua binding | current symbol |",
         "|-----|------|------|--------|-------------|----------------|"]
    for r in sorted(kept, key=lambda x: x["rva"]):
        L.append(f"| `{r['rva_hex']}` | `{r['name']}` | {r['kind']} | "
                 f"{r.get('opcode','')} | {r.get('lua_binding','')} | `{r['current_name']}` |")
    L += ["", "## SEQ-005 cutscene-hang relevance", "",
          "Two registrars here name functions the SEQ-005 kick-dispatcher work",
          "has been circling (see the SEQ-005 memory chain):",
          "",
          "- `_cancelNotice` registrar — the Notice stream's cancel path "
          "(1.x has 5 command streams: Command/Talk/Notice/Emote/Push).",
          "- `_fadeInNowLoadingForNoticeEventJustInArea` — previously located via "
          "MyPlayer vtable slot 66 as the kick-dispatcher clearer; ffxivDecomp",
          "  independently names its registrar, corroborating that anchor and",
          "  giving the registrar→thunk (LAB_0071e3f0) link to force-disassemble next.", ""]
    out.write_text("\n".join(L), encoding="utf-8")


if __name__ == "__main__":
    raise SystemExit(main())
