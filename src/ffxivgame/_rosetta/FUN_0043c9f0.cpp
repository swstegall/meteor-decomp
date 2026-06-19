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
// FUNCTION: ffxivgame 0x0003c9f0 — opcode-to-enum mapper
//           (non-standard register CC: EAX=code, ESI=out ptr; 181 B / 0xb5)
//
// Maps a network opcode/message-type code (passed in EAX) to an internal
// enum value, writing the result to *ESI and returning ESI in EAX.
//
// Mapping table:
//   0x8045 → 0x1b (27)
//   0x1909 → 0x05 (5)
//   0x805b → 0x16 (22)
//   0x80e0 → 0x03 (3)
//   0x80e1 → 0x04 (4)
//   0x83f1 → 0x18 (24)
//   0x83f2 → 0x19 (25)
//   0x83f3 → 0x1a (26)
//   other  → calls FUN_00406550 (error/assert), writes -1
//
// The branch tree is built as a series of CMP+JA/JZ probes:
//   CMP EAX,0x80e1: JA→high-range, JZ→0x80e1-handler
//   CMP EAX,0x805b: JA→mid-range,  JZ→0x805b-handler
//   CMP EAX,0x1909: JZ→0x1909-handler
//   CMP EAX,0x8045: JNZ→error
//   (fall-through) → 0x8045-handler
//   mid-range:  CMP EAX,0x80e0: JNZ→error, fall→0x80e0-handler
//   high-range: SUB EAX,0x83f1: JZ→0x83f1-handler
//               SUB EAX,0x1:    JZ→0x83f2-handler
//               SUB EAX,0x1:    JZ→0x83f3-handler
//               (fall-through) → error
//
// Calling convention: non-standard register-based; EAX and ESI are live
// register inputs at call time (not stack args). ECX is saved/restored
// with PUSH/POP so callers using __thiscall conventions are unaffected.
// Using __declspec(naked) to reproduce the exact byte sequence.
//
// Relocation sites in the 181-byte body (wildcarded by compare.py):
//   +0x6c   PUSH offset rdata_f666e8
//   +0x76   PUSH offset rdata_f66728
//   +0x7b   PUSH offset rdata_f6666e
//   +0x80   PUSH offset rdata_f66774
//   +0x89   CALL rel32 → FUN_00406550

extern "C" {
    // .text — error/assert helper, __thiscall
    int FUN_00406550();

    // .rdata — string literals referenced in the error path.
    // Addresses are wildcarded by compare.py at the relocation sites.
    extern int rdata_f666e8;   // first arg  (pushed first = last on callee stack)
    extern int rdata_f66728;   // third arg
    extern int rdata_f6666e;   // fourth arg
    extern int rdata_f66774;   // fifth arg  (pushed last = first on callee stack)
}

extern "C" __declspec(naked) void FUN_0043c9f0() {
    __asm {
        push    ecx
        cmp     eax, 0x80e1
        ja      short block_gt_80e1
        jz      short block_eq_80e1
        cmp     eax, 0x805b
        ja      short block_gt_805b
        jz      short block_eq_805b
        cmp     eax, 0x1909
        jz      short block_eq_1909
        cmp     eax, 0x8045
        jnz     short block_error
        mov     dword ptr [esi], 0x1b
        mov     eax, esi
        pop     ecx
        ret

    block_eq_1909:
        mov     dword ptr [esi], 0x5
        mov     eax, esi
        pop     ecx
        ret

    block_eq_805b:
        mov     dword ptr [esi], 0x16
        mov     eax, esi
        pop     ecx
        ret

    block_gt_805b:
        cmp     eax, 0x80e0
        jnz     short block_error
        mov     dword ptr [esi], 0x3
        mov     eax, esi
        pop     ecx
        ret

    block_eq_80e1:
        mov     dword ptr [esi], 0x4
        mov     eax, esi
        pop     ecx
        ret

    block_gt_80e1:
        sub     eax, 0x83f1
        jz      short block_eq_83f1
        sub     eax, 0x1
        jz      short block_eq_83f2
        sub     eax, 0x1
        jz      short block_eq_83f3

    block_error:
        push    offset rdata_f666e8
        push    0x82
        push    offset rdata_f66728
        push    offset rdata_f6666e
        push    offset rdata_f66774
        lea     ecx, [esp + 0x17]
        call    FUN_00406550
        mov     dword ptr [esi], 0xffffffff
        mov     eax, esi
        pop     ecx
        ret

    block_eq_83f3:
        mov     dword ptr [esi], 0x1a
        mov     eax, esi
        pop     ecx
        ret

    block_eq_83f2:
        mov     dword ptr [esi], 0x19
        mov     eax, esi
        pop     ecx
        ret

    block_eq_83f1:
        mov     dword ptr [esi], 0x18
        mov     eax, esi
        pop     ecx
        ret
    }
}
