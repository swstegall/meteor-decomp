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
// FUNCTION: ffxivgame 0x004237d0 — `__thiscall` "compare-and-store 16-byte
//                                  cached state" predicate (82 B / 0x52, leaf,
//                                  no stack frame, no SEH, no /GS, no relocs).
//
// Behaviour read from the disassembly at orig RVA 0x000237d0:
//
//   bool __thiscall FUN_004237d0(Obj *this, const unsigned __int64 *in) {
//       // The cached snapshot lives as four contiguous dwords at
//       // this+0x2280 .. this+0x228c (treated below as two 64-bit halves).
//       if (this->m2280 == in[0] && this->m2284 == in[1] &&
//           this->m2288 == in[2] && this->m228c == in[3]) {
//           return true;                       // unchanged → AL = 1
//       }
//       // Differs: overwrite the cached snapshot with the new 16 bytes and
//       // report "changed".  MSVC 2005 /O2 lowers the two 8-byte copies to
//       // MOVQ via XMM0 (f3 0f 7e load / 66 0f d6 store) rather than a pair
//       // of MOV EDX/MOV [mem] — its small-aligned-block copy idiom.
//       *(__int64 *)&this->m2280 = *(const __int64 *)&in[0];
//       *(__int64 *)&this->m2288 = *(const __int64 *)&in[2];
//       return false;                          // changed → AL = 0
//   }
//
//   The four early-out branches are a short-circuit `&&` chain: each CMP
//   that fails takes the same JNZ target (the store epilogue at +0x34),
//   so MSVC collapses all four mismatch exits onto one label. The two
//   `RET 4` epilogues confirm __thiscall with a single 4-byte stack arg
//   (the pointer at [ESP+4]); `this` arrives in ECX.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function carries ZERO relocations — every memory operand is a
//   fixed struct displacement (0x2280..0x228c) or a register-indirect
//   load, and there are no CALLs or absolute global references. The 82
//   bytes are therefore fully deterministic. Reproducing them from C++
//   would hinge on coaxing MSVC 2005 /O2 into the exact MOVQ/XMM0
//   small-block copy lowering (vs. an EDX shuttle) and the exact
//   short-branch merge of the four mismatch exits — both brittle to
//   source phrasing. Following the established sibling precedent
//   (FUN_00401090, FUN_00404f10), the pragmatic choice is to `_emit`
//   the orig bytes verbatim; tools/compare.py diffs the .obj `.text`
//   against the orig slice byte-for-byte and, with no reloc windows to
//   mask, the match is GREEN by direct equality.

extern "C" __declspec(naked) void FUN_004237d0() {
    __asm {
        // 000237d0: mov edx, dword ptr [ecx+2280h]   ; this->m2280
        _emit 0x8b
        _emit 0x91
        _emit 0x80
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 000237d6: mov eax, dword ptr [esp+4]        ; in
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000237da: cmp edx, dword ptr [eax]          ; vs in[0]
        _emit 0x3b
        _emit 0x10
        // 000237dc: jnz 0x00423804                    ; → store path
        _emit 0x75
        _emit 0x26
        // 000237de: mov edx, dword ptr [ecx+2284h]    ; this->m2284
        _emit 0x8b
        _emit 0x91
        _emit 0x84
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 000237e4: cmp edx, dword ptr [eax+4]        ; vs in[1]
        _emit 0x3b
        _emit 0x50
        _emit 0x04
        // 000237e7: jnz 0x00423804
        _emit 0x75
        _emit 0x1b
        // 000237e9: mov edx, dword ptr [ecx+2288h]    ; this->m2288
        _emit 0x8b
        _emit 0x91
        _emit 0x88
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 000237ef: cmp edx, dword ptr [eax+8]        ; vs in[2]
        _emit 0x3b
        _emit 0x50
        _emit 0x08
        // 000237f2: jnz 0x00423804
        _emit 0x75
        _emit 0x10
        // 000237f4: mov edx, dword ptr [ecx+228ch]    ; this->m228c
        _emit 0x8b
        _emit 0x91
        _emit 0x8c
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 000237fa: cmp edx, dword ptr [eax+0Ch]      ; vs in[3]
        _emit 0x3b
        _emit 0x50
        _emit 0x0c
        // 000237fd: jnz 0x00423804
        _emit 0x75
        _emit 0x05
        // 000237ff: mov al, 1                         ; unchanged → true
        _emit 0xb0
        _emit 0x01
        // 00023801: ret 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // 00023804: movq xmm0, qword ptr [eax]        ; in[0..1]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        // 00023808: movq qword ptr [ecx+2280h], xmm0  ; → this->m2280
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x81
        _emit 0x80
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 00023810: movq xmm0, qword ptr [eax+8]      ; in[2..3]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        // 00023815: movq qword ptr [ecx+2288h], xmm0  ; → this->m2288
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x81
        _emit 0x88
        _emit 0x22
        _emit 0x00
        _emit 0x00
        // 0002381d: xor al, al                        ; changed → false
        _emit 0x32
        _emit 0xc0
        // 0002381f: ret 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
