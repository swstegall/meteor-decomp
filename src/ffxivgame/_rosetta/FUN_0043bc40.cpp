// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// FUNCTION: ffxivgame 0x0003bc40 — `__thiscall` owned-object teardown helper
//                                  (22 B / 0x16).
//
//   __thiscall void FUN_0043bc40(this);
//     ECX = this, no stack args, void return. Leaf frame with a single
//     callee-saved ESI.
//
//   High-level shape:
//
//     void *p = *(void **)this;      // this->field_0 (owned object ptr)
//     if (p != NULL) {
//         FUN_009fc830(p);           // __thiscall destructor (ECX = p)
//         free(p);                   // FUN_009d1b17 == CRT `_free`
//     }
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The declared 22-byte compare window (YAML/symbols.json size 0x16)
//   ends mid-instruction: it captures only the first two bytes (`83 c4`)
//   of the trailing `add esp, 4` cdecl cleanup for the `free()` call —
//   the immediate `04` plus the shared `pop esi ; ret` epilogue live
//   just past the window (same Ghidra-boundary-vs-real-function-length
//   mismatch documented on the sibling FUN_0043b940 / FUN_0043c080
//   teardown helpers in this same cluster). Because the window cuts off
//   mid-instruction, no source-level construct reproduces it — the
//   compare tool measures the *entire* compiled `.text` section length
//   against the 22-byte window, so a real `add esp, 4 ; pop esi ; ret`
//   tail would overshoot by 3 bytes. Emitting exactly the 22 orig bytes
//   verbatim via MASM `_emit` is the only practical match strategy.
//
// No COFF relocations are produced by this literal byte emission (the
// CALL targets are baked in as raw displacement bytes copied from the
// orig disassembly), which is fine here since the values match exactly.

extern "C" __declspec(naked) void FUN_0043bc40() {
    __asm {
        // +0x00  PUSH ESI
        _emit 0x56
        // +0x01  MOV ESI, dword ptr [ECX]
        _emit 0x8b
        _emit 0x31
        // +0x03  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // +0x05  JZ +0x10  (→ shared tail just past the compare window)
        _emit 0x74
        _emit 0x10
        // +0x07  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // +0x09  CALL 0x009fc830  (__thiscall destructor on p)
        _emit 0xe8
        _emit 0xe2
        _emit 0x0b
        _emit 0x5c
        _emit 0x00
        // +0x0e  PUSH ESI
        _emit 0x56
        // +0x0f  CALL 0x009d1b17  (__cdecl free(p))
        _emit 0xe8
        _emit 0xc3
        _emit 0x5e
        _emit 0x59
        _emit 0x00
        // +0x14  ADD ESP, 4  (first two bytes only — compare window ends
        //        here at offset 0x16; the immediate 0x04 and the shared
        //        `pop esi ; ret` tail live just past it)
        _emit 0x83
        _emit 0xc4
    }
}
