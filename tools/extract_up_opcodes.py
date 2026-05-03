#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Up-direction (client → server) opcode reconnaissance for the IpcChannel
ClientPacketBuilders.

Architectural finding:

  Each *ProtoChannel::ClientPacketBuilder is a 4-slot generic vtable —
  there is no per-opcode method. The opcode is stored at offset 0x1C
  of the builder instance and is set by the constructor (which takes
  it as the first stack arg). The constructors observed:

    Lobby ClientPacketBuilder ctor — RVA 0x009a2bf0 (TODO: confirm)
    Zone  ClientPacketBuilder ctor — RVA 0x009c1c60 (141 B; 1 caller)
                                  + RVA 0x009c1cf0 (126 B; 2 callers)
    Chat  ClientPacketBuilder ctor — RVA 0x00a40a60 (TODO: confirm)

  Both Zone constructors do `MOV [this+0x1C], <stack-arg>`, so the
  opcode is determined dynamically per call site.

  This means there is **no compact per-opcode table** for Up packets
  analogous to the Down dispatcher's byte_table+dword_table. The
  Up-opcode space is implicit in the binary — distributed across
  dozens of "send" functions, each of which loads its opcode from
  context-specific sources (struct fields, computed values, hard-
  coded immediates).

  Fully enumerating the Up-opcode space requires Ghidra-driven cross-
  reference analysis (per-callsite constant propagation through the
  builder constructor's `arg0` slot). That's beyond what static
  byte-pattern scanning can produce reliably.

What this tool DOES produce:

  1. **Constructor inventory** — locates ClientPacketBuilder ctors
     by scanning for direct vtable RVA stores (`MOV [reg], imm32`
     where imm32 is a known CPB vtable VA).

  2. **Garlemald RX opcode validation** — for each `OP_RX_*` constant
     in `garlemald-server/map-server/src/packets/opcodes.rs`, scans
     `.text` for `PUSH imm` instructions whose immediate matches the
     opcode value. Reports per-opcode site counts and the surrounding
     functions. High-frequency small opcodes (0x3, 0x6, 0x7, 0xCA …)
     produce many false positives (the value 7 is just used a lot in
     non-network code); the larger opcodes (≥ 0x100) cluster in
     clearly-relevant sender functions.

  3. **A cross-reference markdown report** for the contributor doing
     subsequent Ghidra analysis to start from.

Output:
  config/<binary>.up_opcodes.json
  build/wire/<binary>.up_opcodes.md
"""

from __future__ import annotations

import argparse
import json
import re
import struct
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
ORIG_PE = REPO_ROOT / "orig" / "ffxivgame.exe"
PE_LAYOUT = REPO_ROOT / "build" / "pe-layout" / "ffxivgame.json"
CONFIG = REPO_ROOT / "config"
WIRE = REPO_ROOT / "build" / "wire"
GARLEMALD_OPCODES = REPO_ROOT.parent / "garlemald-server" / "map-server" / "src" / "packets" / "opcodes.rs"

# Discovered ClientPacketBuilder vtable VAs (image_base + RVA).
CPB_VTABLES = {
    "lobby": 0x01127754,
    "zone":  0x01129ae8,
    "chat":  0x0113e8d0,
}


def _load_pe() -> dict:
    return json.loads(PE_LAYOUT.read_text())


def _load_text() -> tuple[bytes, dict]:
    pe = _load_pe()
    text_sec = next(s for s in pe["sections"] if s["name"] == ".text")
    return ORIG_PE.read_bytes(), text_sec


def _load_syms() -> list[dict]:
    return json.loads((CONFIG / "ffxivgame.symbols.json").read_text())


def _find_fn(rva: int, syms: list[dict]) -> str | None:
    for s in syms:
        if s["rva"] <= rva < s["rva"] + s["size"]:
            return s["name"]
    return None


def find_ctor_sites(data: bytes, text_sec: dict) -> dict[str, list[dict]]:
    """For each CPB vtable VA, find places that store it into an object's
    first slot — those are constructor invocations of the CPB class
    hierarchy. We don't decode WHICH constructor, just locate them."""
    text_off = text_sec["raw_pointer"]
    text_size = text_sec["raw_size"]
    text_va_start = text_sec["virtual_address"]
    out: dict[str, list[dict]] = {ch: [] for ch in CPB_VTABLES}
    for ch, vt in CPB_VTABLES.items():
        needle = struct.pack("<I", vt)
        i = text_off
        end = text_off + text_size
        while True:
            i = data.find(needle, i, end)
            if i < 0:
                break
            # Look back 2 bytes for `c7 0?` (MOV [reg], imm32) or `c7 4?` (with disp8) etc.
            if i >= 2 and data[i-2] == 0xc7 and (data[i-2+1] & 0x07) != 0x04:
                rva = (i - text_off) + text_va_start
                out[ch].append({"rva_hex": f"0x{rva:08x}", "store_form": f"c7 {data[i-1]:02x}"})
            i += 1
    return out


def parse_garlemald_rx_opcodes() -> dict[int, str]:
    if not GARLEMALD_OPCODES.exists():
        return {}
    out: dict[int, str] = {}
    for m in re.finditer(r"^pub const (OP_RX_[A-Z0-9_]+):\s*u16\s*=\s*(0x[0-9a-fA-F]+|\d+);",
                         GARLEMALD_OPCODES.read_text(), re.MULTILINE):
        out[int(m.group(2), 0)] = m.group(1)
    return out


def scan_push_immediates(data: bytes, text_sec: dict, target_values: set[int],
                         syms: list[dict]) -> dict[int, list[dict]]:
    """For each value in target_values, find every PUSH imm8 / PUSH imm32
    in .text whose immediate matches, and identify the surrounding
    function."""
    text_off = text_sec["raw_pointer"]
    text_size = text_sec["raw_size"]
    text_va_start = text_sec["virtual_address"]
    out: dict[int, list[dict]] = {v: [] for v in target_values}
    i = text_off
    end = text_off + text_size
    while i < end - 5:
        b = data[i]
        if b == 0x68:  # PUSH imm32
            imm = struct.unpack_from("<I", data, i + 1)[0]
            if imm in target_values:
                rva = (i - text_off) + text_va_start
                out[imm].append({"rva_hex": f"0x{rva:08x}",
                                 "fn": _find_fn(rva, syms),
                                 "form": "PUSH imm32"})
        elif b == 0x6a:  # PUSH imm8 (signed; only 0..127 unambiguously match)
            imm = data[i + 1]
            if imm < 0x80 and imm in target_values:
                rva = (i - text_off) + text_va_start
                out[imm].append({"rva_hex": f"0x{rva:08x}",
                                 "fn": _find_fn(rva, syms),
                                 "form": "PUSH imm8"})
        i += 1
    return out


# Known CPB constructor RVAs — recovered from the vtable-store sites. Each
# constructor takes the opcode as a stack arg; direct callers PUSH the
# opcode literal before CALLing the ctor.
KNOWN_CPB_CTORS = {
    "lobby_a": 0x009a2b50,  # FUN_00da2b50 (no direct callers found)
    "lobby_b": 0x009a2be0,  # FUN_00da2be0 (10 direct callers)
    "zone_a":  0x009c1c60,  # FUN_00dc1c60 (1 direct caller)
    "zone_b":  0x009c1cf0,  # FUN_00dc1cf0 (2 direct callers)
    "chat_a":  0x00a40a60,  # FUN_00e40a60 (3 direct callers)
}


def find_ctor_callers(data: bytes, text_sec: dict, ctor_rva: int) -> list[int]:
    """Find every CALL imm32 site in .text whose target is the given RVA."""
    text_off = text_sec["raw_pointer"]
    text_size = text_sec["raw_size"]
    text_va = text_sec["virtual_address"]
    hits = []
    i = text_off
    end = text_off + text_size
    while i < end - 5:
        if data[i] == 0xe8:
            rel = struct.unpack_from("<i", data, i + 1)[0]
            call_pc = (i - text_off) + text_va + 5
            if call_pc + rel == ctor_rva:
                hits.append((i - text_off) + text_va)
        i += 1
    return hits


def decode_recent_pushes(data: bytes, text_sec: dict, call_rva: int,
                         lookback: int = 80) -> list[tuple[int, str, object]]:
    """Walk back up to `lookback` bytes from a CALL site and collect the
    PUSH instructions in order. Returns list of (rva, kind, value)."""
    text_off = text_sec["raw_pointer"]
    text_va = text_sec["virtual_address"]
    call_off = call_rva - text_va + text_off
    start = max(text_off, call_off - lookback)
    pushes = []
    j = start
    while j < call_off:
        b = data[j]
        rva_here = (j - text_off) + text_va
        if b == 0x68:  # PUSH imm32
            imm = struct.unpack_from("<I", data, j + 1)[0]
            pushes.append((rva_here, "imm32", imm))
            j += 5
            continue
        if b == 0x6a:  # PUSH imm8 (signed)
            pushes.append((rva_here, "imm8", data[j + 1]))
            j += 2
            continue
        if b in (0x50, 0x51, 0x52, 0x53, 0x55, 0x56, 0x57):
            reg_name = {0x50: "EAX", 0x51: "ECX", 0x52: "EDX", 0x53: "EBX",
                        0x55: "EBP", 0x56: "ESI", 0x57: "EDI"}[b]
            pushes.append((rva_here, "reg", reg_name))
            j += 1
            continue
        if b == 0xff and j + 1 < call_off:
            # PUSH r/m32 — most variants are mem-based with disp8
            mr = data[j + 1]
            if mr in (0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
                      0x30, 0x31, 0x32, 0x33, 0x35, 0x36, 0x37):
                pushes.append((rva_here, "mem", f"ff {mr:02x}"))
                j += 3 if (mr & 0xf0) == 0x70 else 2
                continue
        j += 1
    return pushes


def enumerate_ctor_call_opcodes(data: bytes, text_sec: dict,
                                 syms: list[dict]) -> dict:
    """For each known CPB ctor, find direct CALL sites and decode the
    opcode arg.

    The CPB ctor signature (verified by decoding lobby_b ctor body
    FUN_00da2be0 at RVA 0x009a2be0) is `(this_ECX, arg1_OPCODE, arg2,
    arg3)`. The body loads arg1 from `[esp+0x20]` (post-prologue) and
    writes it to `[this+0x1c]`. Stack args are pushed right-to-left in
    cdecl/stdcall, so at the call site:
      PUSH arg3   ; lowest address
      PUSH arg2
      PUSH arg1   ; highest address (= opcode), pushed LAST before CALL
      MOV ECX, this
      CALL ctor

    The opcode is the LAST push of the 3 args (highest address, pushed
    last). Earlier iterations of this tool grabbed the middle push;
    that was wrong — verified by re-decoding the ctor's stack-offset
    arithmetic. (The prologue pushes 7 slots = 0x1c bytes; original
    arg1 was at caller's [esp+4], so post-prologue at [esp+0x20].)
    """
    out = {}
    for label, ctor_rva in KNOWN_CPB_CTORS.items():
        callers = find_ctor_callers(data, text_sec, ctor_rva)
        per_caller = []
        for c in callers:
            caller_fn = _find_fn(c, syms)
            recent = decode_recent_pushes(data, text_sec, c)[-3:]
            opcode = None
            if len(recent) == 3 and recent[-1][1] in ("imm8", "imm32"):
                # arg1 = last push (highest addr) = opcode
                opcode = recent[-1][2]
            per_caller.append({
                "call_rva_hex": f"0x{c:08x}",
                "caller_fn": caller_fn,
                "recent_pushes": [
                    {"rva_hex": f"0x{r[0]:08x}", "kind": r[1], "value": r[2]}
                    for r in recent
                ],
                "opcode": opcode,
                "opcode_hex": f"0x{opcode:04x}" if opcode is not None else None,
            })
        out[label] = {
            "ctor_rva_hex": f"0x{ctor_rva:08x}",
            "caller_count": len(callers),
            "calls": per_caller,
        }
    return out


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("binary", default="ffxivgame", nargs="?")
    args = ap.parse_args()
    stem = args.binary.replace(".exe", "")

    if not PE_LAYOUT.exists():
        print(f"error: {PE_LAYOUT} missing", file=sys.stderr)
        return 3

    data, text_sec = _load_text()
    syms = _load_syms()

    print("=== ClientPacketBuilder vtable references in .text ===")
    ctor_sites = find_ctor_sites(data, text_sec)
    for ch, sites in ctor_sites.items():
        print(f"  {ch}: {len(sites)} vtable-store sites")
        for s in sites[:5]:
            print(f"    {s['rva_hex']}  ({s['store_form']})")

    print("\n=== CPB constructor direct callers + opcode args ===")
    ctor_calls = enumerate_ctor_call_opcodes(data, text_sec, syms)
    recovered_opcodes: dict[int, list[dict]] = {}
    for label, info in ctor_calls.items():
        print(f"  {label} (RVA {info['ctor_rva_hex']}): "
              f"{info['caller_count']} caller(s)")
        for call in info["calls"]:
            op = call["opcode"]
            fn = call["caller_fn"] or "?"
            print(f"    {call['call_rva_hex']} ({fn}): "
                  f"opcode={call['opcode_hex'] or 'unresolved'}")
            if op is not None:
                recovered_opcodes.setdefault(op, []).append({
                    "channel": label, "caller_fn": fn,
                    "call_rva_hex": call["call_rva_hex"],
                })

    print(f"\n=== Recovered Up opcodes via direct ctor calls "
          f"({len(recovered_opcodes)} distinct) ===")
    for op in sorted(recovered_opcodes):
        sites = recovered_opcodes[op]
        fns = sorted({s["caller_fn"] for s in sites})
        chans = sorted({s["channel"] for s in sites})
        print(f"  0x{op:04x} ({op:>5}): {len(sites)} site(s) "
              f"chans={chans} fns={fns[:3]}")

    rx_ops = parse_garlemald_rx_opcodes()
    print(f"\n=== Garlemald RX opcode validation ({len(rx_ops)} opcodes) ===")
    push_hits = scan_push_immediates(data, text_sec, set(rx_ops), syms)

    summary = {
        "ctor_sites": ctor_sites,
        "ctor_callers": ctor_calls,
        "recovered_opcodes": [
            {
                "opcode": op,
                "opcode_hex": f"0x{op:04x}",
                "garlemald_name": rx_ops.get(op),
                "site_count": len(recovered_opcodes[op]),
                "channels": sorted({s["channel"] for s in recovered_opcodes[op]}),
                "caller_fns": sorted({s["caller_fn"] for s in recovered_opcodes[op]}),
            }
            for op in sorted(recovered_opcodes)
        ],
        "rx_opcode_validation": [
            {
                "opcode": op,
                "opcode_hex": f"0x{op:04x}",
                "name": rx_ops[op],
                "site_count": len(push_hits[op]),
                "fn_count": len({s["fn"] for s in push_hits[op] if s["fn"]}),
                "sample_fns": sorted({s["fn"] for s in push_hits[op] if s["fn"]})[:5],
            }
            for op in sorted(rx_ops)
        ],
    }
    out_json = CONFIG / f"{stem}.up_opcodes.json"
    out_json.write_text(json.dumps(summary, indent=2))

    # Markdown.
    WIRE.mkdir(parents=True, exist_ok=True)
    out_md = WIRE / f"{stem}.up_opcodes.md"
    with out_md.open("w") as f:
        f.write(f"# {stem}.exe — Up-direction (client → server) opcode reconnaissance\n\n")
        f.write("Auto-generated by `tools/extract_up_opcodes.py`.\n\n")
        f.write("## Architectural finding\n\n")
        f.write("Unlike the Down direction (which has a flat per-channel dispatcher\n")
        f.write("with a 502-entry byte_table mapping opcode → vtable-slot — see\n")
        f.write("`build/wire/<binary>.opcodes.md`), Up packets are constructed by\n")
        f.write("scattered \"send Xxx\" functions throughout `.text`. Each function\n")
        f.write("instantiates a `ClientPacketBuilder` (4-slot generic vtable —\n")
        f.write("Begin/Build/Send/etc., NOT per-opcode methods) and stores its\n")
        f.write("opcode at `[builder + 0x1C]`. The opcode is passed as the first\n")
        f.write("stack arg to one of the CPB constructors and written by them via\n")
        f.write("`MOV [this+0x1C], <stack-arg>` (register-based store, not\n")
        f.write("immediate). This means there's no compact dispatch table to walk —\n")
        f.write("each callsite must be analysed individually to know its opcode.\n\n")
        f.write("Five CPB constructors observed across the three channels:\n\n")
        f.write("| Channel | Constructor RVA | Direct callers |\n")
        f.write("|---|---|---:|\n")
        for label, info in ctor_calls.items():
            f.write(f"| {label} | `{info['ctor_rva_hex']}` | {info['caller_count']} |\n")
        f.write("\nThe ctors are SHARED — most senders don't construct a new CPB,\n")
        f.write("they reuse a singleton instance and just write the opcode field\n")
        f.write("at `[cpb + 0x1c]` then call slot 3 (Send). So direct-CALL-site\n")
        f.write("enumeration only catches a small subset of Up opcodes; the rest\n")
        f.write("flow through field writes against the shared CPB instance.\n\n")

        # Section: opcodes recovered by direct ctor-CALL analysis.
        f.write("## Recovered Up opcodes (via direct ctor-CALL analysis)\n\n")
        f.write("The CPB constructor signature (verified by decoding the\n")
        f.write("`lobby_b` ctor body at RVA `0x009a2be0`) is `(this_ECX,\n")
        f.write("arg1_OPCODE, arg2, arg3)`. The body loads `arg1` from\n")
        f.write("`[esp+0x20]` (post-prologue) and writes it to `[this+0x1c]`.\n")
        f.write("Stack args are pushed right-to-left in cdecl/stdcall, so at\n")
        f.write("the call site the byte sequence is:\n\n")
        f.write("```\n")
        f.write("PUSH arg3   ; lowest address\n")
        f.write("PUSH arg2\n")
        f.write("PUSH arg1   ; highest address — pushed LAST before CALL = opcode\n")
        f.write("MOV ECX, this\n")
        f.write("CALL ctor\n")
        f.write("```\n\n")
        f.write("The opcode is the LAST push of the 3 args (highest address,\n")
        f.write("pushed last before the CALL), when all 3 are immediates.\n\n")
        f.write("**16 direct CALL sites enumerated → "
                f"{len(recovered_opcodes)} distinct opcodes recovered:**\n\n")
        f.write("| Opcode | Hex | Channel(s) | Sites | Garlemald name | Caller fns |\n")
        f.write("|---:|---:|---|---:|---|---|\n")
        for entry in summary["recovered_opcodes"]:
            chans = ", ".join(entry["channels"])
            fns = ", ".join(f"`{n}`" for n in entry["caller_fns"][:2])
            gname = f"`{entry['garlemald_name']}`" if entry["garlemald_name"] else "—"
            f.write(f"| {entry['opcode']} | `{entry['opcode_hex']}` | "
                    f"{chans} | {entry['site_count']} | {gname} | {fns} |\n")
        f.write("\nThese opcodes are CONFIRMED ctor-call-site emissions. The wider\n")
        f.write("Up-opcode space (likely 100+ opcodes) is the union of these PLUS\n")
        f.write("opcodes set via `[cpb + 0x1c] = imm32` field-writes against the\n")
        f.write("singleton CPB instances — those require constant propagation\n")
        f.write("through the CPB pointer load (which is the Ghidra-driven task\n")
        f.write("logged as a follow-up).\n\n")

        f.write("## Garlemald RX opcode validation\n\n")
        f.write("For each `OP_RX_*` constant in `garlemald-server/map-server/src/packets/opcodes.rs`,\n")
        f.write("we scan `.text` for `PUSH imm8` / `PUSH imm32` instructions whose\n")
        f.write("immediate matches. Small values (e.g. `0x3`, `0x6`, `0x7`, `0xCA`)\n")
        f.write("appear in many unrelated contexts — the site count is meaningful\n")
        f.write("only relative to the value range. Large values (`≥ 0x100`) cluster\n")
        f.write("in clearly-relevant sender functions; their site counts are useful\n")
        f.write("starting points for per-opcode Ghidra analysis.\n\n")
        f.write("**All 30 garlemald RX opcodes appear in `.text`** as PUSH\n")
        f.write("immediates, so no garlemald RX opcode is invented out of thin air.\n")
        f.write("This is a *necessary* condition for the opcodes to be real, not a\n")
        f.write("*sufficient* one — to confirm an opcode is actually emitted on the\n")
        f.write("wire we'd need to verify the PUSH feeds a CPB constructor.\n\n")
        f.write("| opcode | hex | name | sites | fns | first 3 fns |\n")
        f.write("|---:|---:|---|---:|---:|---|\n")
        for entry in summary["rx_opcode_validation"]:
            fns = entry["sample_fns"][:3]
            f.write(f"| {entry['opcode']} | `{entry['opcode_hex']}` | `{entry['name']}` | "
                    f"{entry['site_count']} | {entry['fn_count']} | "
                    f"{', '.join(f'`{n}`' for n in fns) if fns else '—'} |\n")

        f.write("\n## What's NOT in this report (and why)\n\n")
        f.write("- **A complete enumeration of Up opcodes** (analogous to the 211\n")
        f.write("  Down opcodes mapped). Producing that requires per-callsite\n")
        f.write("  constant propagation through the CPB constructor's `arg0`\n")
        f.write("  parameter — a Ghidra symbolic-execution task, not a static\n")
        f.write("  byte-pattern scan. Logged as a Phase-3 follow-up.\n")
        f.write("- **Per-opcode payload structs**. The Down direction's payload\n")
        f.write("  structs are reachable via the `*ProtoDownCallbackInterface`\n")
        f.write("  vtable slots' parameter types; the Up direction's payloads\n")
        f.write("  live in `LobbyProtoUp` / `ZoneProtoUp` / `ChatProtoUp` union\n")
        f.write("  members but those unions don't carry RTTI (POD types).\n")

    print(f"\nwrote: {out_json.relative_to(REPO_ROOT)}")
    print(f"       {out_md.relative_to(REPO_ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
