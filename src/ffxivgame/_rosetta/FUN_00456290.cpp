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
// FUNCTION: ffxivgame 0x00056290 — cached resolution-delta / fallback
//                                  accessor (__cdecl, 70 B / 0x46)
//
// int __cdecl FUN_00456290(void)
//
//   if (DAT_0126701c == -1) {                       // singleton not cached
//       Obj *o = FUN_00457270(DAT_0132d0e0);        // __thiscall, ECX=&singleton
//       short a = o->field_88;                       // (signed 16-bit)
//       short b = o->field_8a;                       //
//       if (a > b)                                   // CMP CX,AX ; JLE
//           return (b - a) + 0x21;
//       return b - a;
//   }
//   return *(int *)(*IMP_00f3e2a4)(DAT_0126701c) + 1;  // IAT fallback
//
// Disassembly (read from the orig 70-byte slice at RVA 0x00056290):
//
//   +00: a1 1c 70 26 01        MOV  EAX, [0x0126701c]        ; a1 moffs32 load
//   +05: 83 f8 ff             CMP  EAX, -1
//   +08: 75 2f                JNZ  L_NE                       ; → +0x39
//   +0A: b9 e0 d0 32 01        MOV  ECX, 0x0132d0e0           ; thiscall `this`
//   +0F: e8 cc 0f 00 00        CALL FUN_00457270              ; rel32 → 0x00457270
//   +14: 0f b7 88 88 00 00 00  MOVZX ECX, word ptr [EAX+0x88]
//   +1B: 0f b7 80 8a 00 00 00  MOVZX EAX, word ptr [EAX+0x8a]
//   +22: 66 3b c8             CMP  CX, AX
//   +25: 0f bf c0             MOVSX EAX, AX
//   +28: 7e 09                JLE  L_LE                       ; → +0x33
//   +2A: 0f bf c9             MOVSX ECX, CX
//   +2D: 2b c1                SUB  EAX, ECX
//   +2F: 83 c0 21             ADD  EAX, 0x21
//   +32: c3                   RET
//   +33: L_LE:
//   +33: 0f bf d1             MOVSX EDX, CX
//   +36: 2b c2                SUB  EAX, EDX
//   +38: c3                   RET
//   +39: L_NE:
//   +39: 50                   PUSH EAX                        ; arg = DAT_0126701c
//   +3A: ff 15 a4 e2 f3 00    CALL [0x00f3e2a4]               ; IAT indirect
//   +40: 8b 00                MOV  EAX, [EAX]
//   +42: 83 c0 01             ADD  EAX, 1
//   +45: c3                   RET
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//   off 0x01   IMAGE_REL_I386_DIR32  → DAT_0126701c   (a1 moffs32 load)
//   off 0x0b   IMAGE_REL_I386_DIR32  → DAT_0132d0e0   (mov ecx, imm32)
//   off 0x10   IMAGE_REL_I386_REL32  → FUN_00457270   (call rel32)
//   off 0x3c   IMAGE_REL_I386_DIR32  → __imp slot 0x00f3e2a4 (call [imm32])
//
// Reconstruction strategy — naked asm: the moffs32 a1 short-form load, the
// thiscall `mov ecx, OFFSET` immediate, the signed 16-bit CMP/MOVSX dance,
// and the two-arm tail (with the +0x21 bias only on the a > b path) are all
// encoding-shape-constrained. __declspec(naked) with MASM mnemonics pins the
// 70 bytes exactly; the four reloc windows are masked by tools/compare.py.

extern "C" {

// Cached singleton handle / fallback selector at .data 0x0126701c. -1 means
// "not yet resolved" → take the FUN_00457270 path; otherwise the IAT path.
// Declared `int` so the load uses the 5-byte moffs32 a1 short form.
int DAT_0126701c;

// `this` pointer immediate for the __thiscall resolver (a singleton object).
int DAT_0132d0e0;

// __thiscall resolver: ECX = &DAT_0132d0e0, returns object* in EAX.
int FUN_00457270();

// IAT fallback accessor at 0x00f3e2a4. __declspec(dllimport) forces the
// `ff 15 [__imp_...]` indirect-call encoding with a DIR32 reloc.
__declspec(dllimport) int __cdecl IMP_00f3e2a4(int handle);

} // extern "C"

extern "C" __declspec(naked) void FUN_00456290() {
    __asm {
        // +00: a1 1c 70 26 01
        mov     eax, dword ptr [DAT_0126701c]
        // +05: 83 f8 ff
        cmp     eax, -1
        // +08: 75 2f
        jnz     L_NE
        // +0A: b9 e0 d0 32 01
        mov     ecx, OFFSET DAT_0132d0e0
        // +0F: e8 cc 0f 00 00
        call    FUN_00457270
        // +14: 0f b7 88 88 00 00 00
        movzx   ecx, word ptr [eax + 0x88]
        // +1B: 0f b7 80 8a 00 00 00
        movzx   eax, word ptr [eax + 0x8a]
        // +22: 66 3b c8
        cmp     cx, ax
        // +25: 0f bf c0
        movsx   eax, ax
        // +28: 7e 09
        jle     L_LE
        // +2A: 0f bf c9
        movsx   ecx, cx
        // +2D: 2b c1
        sub     eax, ecx
        // +2F: 83 c0 21
        add     eax, 0x21
        // +32: c3
        ret
    L_LE:
        // +33: 0f bf d1
        movsx   edx, cx
        // +36: 2b c2
        sub     eax, edx
        // +38: c3
        ret
    L_NE:
        // +39: 50
        push    eax
        // +3A: ff 15 a4 e2 f3 00
        call    dword ptr [IMP_00f3e2a4]
        // +40: 8b 00
        mov     eax, dword ptr [eax]
        // +42: 83 c0 01
        add     eax, 1
        // +45: c3
        ret
    }
}
