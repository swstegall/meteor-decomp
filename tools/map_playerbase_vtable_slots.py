#!/usr/bin/env python3
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
"""
Map the 39 named PlayerBase Lua bindings to their MyPlayer vtable
member-function bodies, naming 39 functions we carry as FUN_xxx.

ffxivDecomp named the PlayerBase *registrars* (FUN_0073xxxx) and gave
each binding's Functor *thunk* address (LAB_006dexxx). The actual
member-function *bodies* are MyPlayer's vtable slots. Two of those slots
were Ghidra-validated previously (slot 52 = _cancelNotice = FUN_006e8f50,
slot 66 = _fadeInNowLoadingForNoticeEventJustInArea = FUN_006e32f0).

The bindings register into contiguous vtable slots in *thunk-address
order* (the 3 out-of-cluster thunks — fadeInNowLoading / setMusic /
setWeather, in the 0x71exxx range — slot in at their registration
position). That order, anchored at the two validated slots, fixes all 39.

SELF-VALIDATING: this tool asserts both anchors land on their known
addresses. If the vtable ever shifts, it aborts rather than mislabel.

Emits:
  config/ffxivgame.myplayer_bindings.json   [{slot, rva, rva_hex, name,
      lua_binding, confidence, current_symbol}]

`confidence`: "validated" (the 2 anchors), "high" (between the anchors,
slots 52-66, bracketed both sides), "inferred" (outside the anchor span;
same thunk-order mechanism, anchored one side — verify before relying).

Run `tools/build_name_overrides.py` afterward to fold these into the
name-override layer. Usage: python3 tools/map_playerbase_vtable_slots.py
"""

from __future__ import annotations

import json
import re
import subprocess
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
IMAGE_BASE = 0x00400000
VTABLE_DUMP = (REPO_ROOT.parent / "FFXIVLegacyClientStructs"
               / "FFXIVClientStructs.Tools.CLI/bin/Release/net8.0"
               / "vtable_Client_Control_MyPlayer.txt")

# Registration order = thunk-address order (from ffxivDecomp's PlayerBase
# binding doc), with the 3 out-of-cluster thunks placed at their
# registration position. cancelNotice (index 10) anchors slot 52;
# fadeInNowLoading (index 24) anchors slot 66.
ORDER = [
    "executeCommand", "executeTalk", "executeEmote",
    "callServerOnCommand", "doServerOnCommand",
    "canExecuteCommand", "canExecuteTalk", "canExecuteEmote",
    "cancelCommand", "cancelTalk", "cancelNotice",        # <- slot 52
    "cancelEmote", "cancelPush",
    "breakCommand", "isEventPlaying", "isCommandPlaying", "countCommandPlaying",
    "fadeIn", "fadeOut", "waitForFading", "isFading", "cancelFading",
    "fadeInAfterWarp", "resetFade",
    "fadeInNowLoadingForNoticeEventJustInArea",            # <- slot 66
    "lockPlayerControl", "unlockPlayerControl", "isPlayerControlEnabled",
    "lockLockonControl", "unlockLockonControl", "isLockonControlEnabled",
    "lockCameraControl", "unlockCameraControl", "isCameraControlEnabled",
    "setLockonTarget", "getLockonTarget",
    "waitForMapLoaded", "setMusic", "setWeather",
]
BASE_SLOT = 42  # slot = BASE_SLOT + index  (cancelNotice@idx10 -> 52)
ANCHORS = {52: 0x006e8f50, 66: 0x006e32f0}  # known from prior Ghidra validation


def load_vtable() -> dict[int, int]:
    if not VTABLE_DUMP.exists():
        subprocess.run([str(REPO_ROOT / "tools/analyze_legacy_struct.sh"),
                        "--vtable", "Client::Control::MyPlayer"],
                       check=True, stdout=subprocess.DEVNULL)
    slots = {}
    for ln in VTABLE_DUMP.read_text().splitlines():
        m = re.match(r"\s*vt\[\s*(\d+)\]\s*=\s*(0x[0-9A-Fa-f]+)", ln)
        if m:
            slots[int(m.group(1))] = int(m.group(2), 16)
    return slots


def main() -> int:
    vt = load_vtable()
    for slot, want in ANCHORS.items():
        got = vt.get(slot)
        if got != want:
            print(f"ABORT: anchor slot {slot} = 0x{got:08x}, expected 0x{want:08x}. "
                  f"vtable shifted — refusing to emit a possibly-mislabeled map.")
            return 1
    syms = {s["rva"]: s["name"] for s in json.loads(
        (REPO_ROOT / "config/ffxivgame.symbols.json").read_text())}

    rows = []
    for i, b in enumerate(ORDER):
        slot = BASE_SLOT + i
        va = vt[slot]
        rva = va - IMAGE_BASE
        if slot in ANCHORS:
            conf = "validated"
        elif 52 < slot < 66:
            conf = "high"          # bracketed by both validated anchors
        else:
            conf = "inferred"      # same mechanism, anchored one side
        rows.append({
            "slot": slot, "rva": rva, "rva_hex": f"0x{rva:08x}",
            "name": f"MyPlayer::_{b}", "lua_binding": f"_{b}",
            "confidence": conf, "current_symbol": syms.get(rva, "(none)"),
        })

    out = REPO_ROOT / "config/ffxivgame.myplayer_bindings.json"
    out.write_text(json.dumps(rows, indent=1), encoding="utf-8")
    by = {}
    for r in rows:
        by[r["confidence"]] = by.get(r["confidence"], 0) + 1
    named = sum(1 for r in rows if r["current_symbol"].startswith(("FUN_", "thunk_", "LAB_", "SUB_")))
    print(f"wrote config/{out.name}: {len(rows)} MyPlayer binding slots")
    print(f"  anchors validated (slot 52 + 66 match known addresses)")
    print(f"  confidence: {by}")
    print(f"  still FUN_xxx (namable): {named}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
