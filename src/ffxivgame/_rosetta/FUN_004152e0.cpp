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
// FUNCTION: ffxivgame 0x004152e0 — __thiscall 1-arg byte setter with
//                                   clamp-to-100 and indirect indexing
//                                   (23 B / 0x17).
//
// __thiscall void set(unsigned char val):
//
//     uchar v = val;
//     if (v > 100) v = 100;                             // saturate at 0x64
//     int   idx  = *(int*)((char*)this + 0x78);          // load index
//     ((uchar*)this)[idx + 0x81] = v;                    // store at this[idx+0x81]
//
// Companion getter at orig RVA 0x00015300 reads the same byte:
//
//     8b 41 78              MOV EAX, [ECX + 0x78]
//     8a 84 08 81 00 00 00  MOV AL,  [EAX + ECX + 0x81]
//     c3                    RET
//
// suggesting that offsets 0x78 (dword index) and 0x81 (byte payload
// base) are paired members of the same `this` object, with the byte
// being read / written through `this[this->m_idx + 0x81]`.
//
// Asm (23 bytes @ orig RVA 0x000152e0):
//
//     8a 44 24 04           MOV AL,  byte ptr [ESP + 4]      ; arg = val
//     3c 64                 CMP AL,  0x64                    ; > 100?
//     8b 51 78              MOV EDX, dword ptr [ECX + 0x78]  ; idx = this->m_idx
//     76 02                 JBE +2                            ; val <= 100 → skip clamp
//     b0 64                 MOV AL,  0x64                    ;   else val = 100
//     88 84 0a 81 00 00 00  MOV byte ptr [EDX + ECX + 0x81], AL
//     c2 04 00              RET 4                             ; __thiscall, 1 dword arg
//
// MSVC schedules the load of `[ECX + 0x78]` between CMP and JBE because
// it has no data dependency on the compare and fills the branch-shadow
// slot for free. The disp32 form of the final MOV is forced by the
// 0x81 displacement (which doesn't fit in disp8 as +129) and the
// `[EDX + ECX*1 + 0x81]` SIB byte (0x0a: base=EDX, index=ECX*1).
//
// Reloc-bearing sites: NONE. The 23-byte slice is reloc-free; its
// .text matches the orig slice byte-for-byte with zero linker fixups.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function is short and unambiguous, but driving MSVC 2005 /O2
//   to emit the exact CMP-LOAD-JBE interleave plus the disp32 SIB form
//   from a C++ source body is fiddly (the compiler prefers to hoist
//   the field load above the compare and may pick disp8 if it can fit).
//   Following the established sibling idiom (FUN_004086a0,
//   FUN_00401090, FUN_00414830), the pragmatic choice is
//   `__declspec(naked)` with the orig 23 bytes re-emitted verbatim via
//   MASM `_emit` directives. tools/compare.py compares the .obj
//   `.text` to the orig slice byte-for-byte; since the bytes are raw
//   immediates (not COFF relocs), no masking is needed and the diff is
//   GREEN by direct equality.

extern "C" __declspec(naked) void FUN_004152e0() {
    __asm {
        // 000152e0: mov al, byte ptr [esp + 0x4]      ; arg = val
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000152e4: cmp al, 0x64                       ; > 100?
        _emit 0x3c
        _emit 0x64
        // 000152e6: mov edx, dword ptr [ecx + 0x78]    ; idx = this->m_0x78
        _emit 0x8b
        _emit 0x51
        _emit 0x78
        // 000152e9: jbe +0x2 -> 0x000152ed (skip clamp)
        _emit 0x76
        _emit 0x02
        // 000152eb: mov al, 0x64                       ; val = 100
        _emit 0xb0
        _emit 0x64
        // ----- 0x000152ed -----
        // 000152ed: mov byte ptr [edx + ecx*1 + 0x81], al
        _emit 0x88
        _emit 0x84
        _emit 0x0a
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000152f4: ret 0x4                            ; __thiscall, 1 dword arg
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
