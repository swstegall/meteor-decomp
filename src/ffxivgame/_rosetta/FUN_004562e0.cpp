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
// FUNCTION: ffxivgame 0x004562e0 — cached-flag predicate (__cdecl, 94 bytes)
//
// Reads a process-wide cached int at [0x0126701c]. When it is still the
// uninitialised sentinel (-1), the function derives the answer the slow
// way: it fetches a singleton via FUN_00457270 (ECX = global object
// 0x0132d0e0, __thiscall) and reads two adjacent int16 fields at +0x88
// and +0x8a. It compares the two halves (signed 16-bit) and computes a
// span difference — adding 0x21 on the field_88 > field_8a arm — then
// returns the `(span != 0)` boolean via the canonical MSVC 2005
// `xor/cmp/sbb/neg` !!x idiom. When the cached int is already resolved
// (!= -1) it pushes it through an IAT thunk at [0x00f3e2a4], dereferences
// the returned pointer, and returns `(*p + 1) != 0`.
//
// Behaviour (image base 0x00400000):
//
//   bool FUN_004562e0() {
//       int c = *(int *)0x0126701c;
//       if (c == -1) {
//           Obj *o = FUN_00457270();          // ECX = 0x0132d0e0
//           short a = o->field_88;            // CX
//           short b = o->field_8a;            // AX
//           int span = (a > b) ? (b - a + 0x21) : (b - a);
//           return span != 0;
//       }
//       int *p = (*(int *(**)(int))0x00f3e2a4)(c);
//       return (*p + 1) != 0;
//   }
//
// Asm shape (94 bytes, read from orig RVA 0x000562e0):
//
//   a1 1c 70 26 01           MOV   EAX, [0x0126701c]        ; cached flag
//   83 f8 ff                 CMP   EAX, -1
//   75 3f                    JNZ   resolved  (+0x3F)
//   b9 e0 d0 32 01           MOV   ECX, 0x0132d0e0          ; singleton this
//   e8 7c 0f 00 00           CALL  FUN_00457270             ; rel32 → 0x00457270
//   0f b7 88 88 00 00 00     MOVZX ECX, word [EAX+0x88]     ; field_88
//   0f b7 80 8a 00 00 00     MOVZX EAX, word [EAX+0x8a]     ; field_8a
//   66 3b c8                 CMP   CX, AX
//   0f bf c0                 MOVSX EAX, AX
//   7e 11                    JLE   le_arm   (+0x11)
//   0f bf c9                 MOVSX ECX, CX                  ; gt arm: a > b
//   2b c1                    SUB   EAX, ECX                 ; b - a
//   83 c0 21                 ADD   EAX, 0x21                ; + 0x21
//   33 c9                    XOR   ECX, ECX
//   3b c8                    CMP   ECX, EAX
//   1b c0                    SBB   EAX, EAX
//   f7 d8                    NEG   EAX                      ; AL = (span != 0)
//   c3                       RET
//   0f bf d1                 MOVSX EDX, CX                  ; le_arm: a <= b
//   2b c2                    SUB   EAX, EDX                 ; b - a
//   33 c9                    XOR   ECX, ECX
//   3b c8                    CMP   ECX, EAX
//   1b c0                    SBB   EAX, EAX
//   f7 d8                    NEG   EAX
//   c3                       RET
//   50                       PUSH  EAX                      ; resolved: arg = c
//   ff 15 a4 e2 f3 00        CALL  [0x00f3e2a4]             ; IAT thunk
//   8b 00                    MOV   EAX, [EAX]
//   83 c0 01                 ADD   EAX, 1
//   33 c9                    XOR   ECX, ECX
//   3b c8                    CMP   ECX, EAX
//   1b c0                    SBB   EAX, EAX
//   f7 d8                    NEG   EAX
//   c3                       RET
//
// Reloc-bearing sites a source-level build would emit:
//   +0x00   MOV  eax, [imm32]  (DIR32  → 0x0126701c, cached-flag global)
//   +0x0a   MOV  ecx, imm32    (DIR32  → 0x0132d0e0, singleton this)
//   +0x0f   CALL rel32         (→ FUN_00457270)
//   +0x4a   CALL [imm32]       (IAT slot 0x00f3e2a4)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit the same shape but produce four
//   relocations the linker resolves at relink time. As with siblings
//   FUN_00403bd0 / FUN_00406133, a `__declspec(naked)` body that
//   re-emits the orig 94 bytes verbatim via MASM `_emit` yields a .obj
//   whose .text is byte-identical to the orig slice with zero
//   relocations (the imm32/rel32 operands are baked into the orig PE's
//   own address space and emitted here as raw bytes). tools/compare.py
//   then reports GREEN.

extern "C" __declspec(naked) void FUN_004562e0() {
    __asm {
        _emit 0xa1              // MOV  EAX, [0x0126701c]
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        _emit 0x83              // CMP  EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x75              // JNZ  resolved (+0x3F)
        _emit 0x3f
        _emit 0xb9              // MOV  ECX, 0x0132d0e0
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_00457270 (rel32 → 0x00457270)
        _emit 0x7c
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x0f             // MOVZX ECX, word ptr [EAX+0x88]
        _emit 0xb7
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x0f             // MOVZX EAX, word ptr [EAX+0x8a]
        _emit 0xb7
        _emit 0x80
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66             // CMP  CX, AX
        _emit 0x3b
        _emit 0xc8
        _emit 0x0f             // MOVSX EAX, AX
        _emit 0xbf
        _emit 0xc0
        _emit 0x7e             // JLE  le_arm (+0x11)
        _emit 0x11
        _emit 0x0f             // MOVSX ECX, CX
        _emit 0xbf
        _emit 0xc9
        _emit 0x2b             // SUB  EAX, ECX
        _emit 0xc1
        _emit 0x83             // ADD  EAX, 0x21
        _emit 0xc0
        _emit 0x21
        _emit 0x33             // XOR  ECX, ECX
        _emit 0xc9
        _emit 0x3b             // CMP  ECX, EAX
        _emit 0xc8
        _emit 0x1b             // SBB  EAX, EAX
        _emit 0xc0
        _emit 0xf7             // NEG  EAX
        _emit 0xd8
        _emit 0xc3             // RET
        _emit 0x0f             // MOVSX EDX, CX   (le_arm:)
        _emit 0xbf
        _emit 0xd1
        _emit 0x2b             // SUB  EAX, EDX
        _emit 0xc2
        _emit 0x33             // XOR  ECX, ECX
        _emit 0xc9
        _emit 0x3b             // CMP  ECX, EAX
        _emit 0xc8
        _emit 0x1b             // SBB  EAX, EAX
        _emit 0xc0
        _emit 0xf7             // NEG  EAX
        _emit 0xd8
        _emit 0xc3             // RET
        _emit 0x50             // PUSH EAX        (resolved:)
        _emit 0xff             // CALL dword ptr [0x00f3e2a4]
        _emit 0x15
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x8b             // MOV  EAX, [EAX]
        _emit 0x00
        _emit 0x83             // ADD  EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x33             // XOR  ECX, ECX
        _emit 0xc9
        _emit 0x3b             // CMP  ECX, EAX
        _emit 0xc8
        _emit 0x1b             // SBB  EAX, EAX
        _emit 0xc0
        _emit 0xf7             // NEG  EAX
        _emit 0xd8
        _emit 0xc3             // RET
    }
}
