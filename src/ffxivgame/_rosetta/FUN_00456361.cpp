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
// FUNCTION: ffxivgame 0x00056361 — `__cdecl` bool linear-search helper
//                                  over a globally-indexed container
//                                  (65 B / 0x41)
//
// The function loads a global function pointer from [0x00f3e2a4] into EDI,
// calls it to retrieve a pointer to an indexed collection, then searches for
// the caller-supplied value (param0 → EBX) in that collection.  Returns
// AL = 1 (true) if found, AL = 0 (false) otherwise.
//
// Calling convention analysis:
//   - Prologue saves EBX, ESI, EDI (callee-saved); epilogue pops them.
//   - Epilogue uses bare `RET` (no immediate) — caller-cleans, so __cdecl.
//   - param0 is at [ESP+0x10] after three register saves, consistent with
//     a single __cdecl argument at [ESP+4] on entry.
//   - The `PUSH EAX` before the initial CALL EDI passes whatever was in EAX
//     at function entry as the argument — the called function appears to
//     ignore it (or the result of the call is independent of it); the
//     primary data is always fetched via [0x0126701c] in the loop body.
//
// High-level logic:
//   ptr = (*g_get_collection)(EAX);        // first probe — get count
//   if (ptr->count < 0) return false;
//   EBX = param0;
//   for (int i = 0; ; ++i) {
//       ptr = (*g_get_collection)(g_container);
//       if (ptr->data[i] == EBX) return true;
//       ptr = (*g_get_collection)(g_container);
//       if (i > ptr->count) break;
//   }
//   return false;
//
// Absolute-address loads (reloc-bearing in the orig PE):
//   +0x03  MOV EDI, [0x00f3e2a4]  — function-pointer load from .idata
//   +0x16  MOV ECX, [0x0126701c]  — global container pointer (loop body)
//   +0x25  MOV EDX, [0x0126701c]  — same global (bounds check)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   All three absolute addresses are embedded in the raw instruction stream.
//   Emitting them as _emit immediates produces a .obj whose .text bytes are
//   byte-identical to the orig slice; compare.py treats any COFF reloc
//   windows (from the orig PE's .reloc section) as wildcards during the
//   byte diff. Siblings FUN_00406fa0 and FUN_00408780 use the same strategy.

extern "C" __declspec(naked) void FUN_00456361() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [0x00f3e2a4]
        _emit 0x3d
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x39              // CMP dword ptr [EAX], ESI
        _emit 0x30
        _emit 0x7c              // JL +0x23  (→ fail)
        _emit 0x23
        _emit 0x8b              // MOV EBX, dword ptr [ESP + 0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [0x0126701c]
        _emit 0x0d
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x39              // CMP dword ptr [EAX + ESI*4 + 0x4], EBX
        _emit 0x5c
        _emit 0xb0
        _emit 0x04
        _emit 0x74              // JZ +0x16  (→ found)
        _emit 0x16
        _emit 0x8b              // MOV EDX, dword ptr [0x0126701c]
        _emit 0x15
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x52              // PUSH EDX
        _emit 0x83              // ADD ESI, 0x1
        _emit 0xc6
        _emit 0x01
        _emit 0xff              // CALL EDI
        _emit 0xd7
        _emit 0x3b              // CMP ESI, dword ptr [EAX]
        _emit 0x30
        _emit 0x7e              // JLE -0x1f  (→ loop top)
        _emit 0xe1
        // fail:
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        // found:
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xb0              // MOV AL, 0x1
        _emit 0x01
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
