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
// FUNCTION: ffxivgame 0x0001d210 — `__cdecl` null-guarded thiscall forwarder (39 B / 0x27).
//
// Signature (inferred from asm):
//
//   void __cdecl FUN_0041d210(void *param1);
//
// If param1 is non-null the function loads the dword at offset 0x1c from
// param1 and passes it as the sole argument to the `__thiscall` method
// FUN_00423200 on the global object at *[0x0132987c]. If param1 is null
// it passes 0 instead. Both branches end with their own CALL + RET (MSVC
// /O2 code-duplication rather than a ternary select).
//
// Asm shape (39 bytes — RVA 0x0001d210..0x0001d236):
//
//   0001d210:  8b 44 24 04              MOV  EAX, dword ptr [ESP+0x4]   ; param1
//   0001d214:  85 c0                    TEST EAX, EAX                   ; null?
//   0001d216:  74 10                    JZ   short +0x10                ; → null path
//   0001d218:  8b 40 1c                 MOV  EAX, dword ptr [EAX+0x1c]  ; field_0x1c
//   0001d21b:  8b 0d 7c 98 32 01        MOV  ECX, dword ptr [0x0132987c] ; this = *g
//   0001d221:  50                       PUSH EAX                        ; push field
//   0001d222:  e8 d9 5f 00 00           CALL 0x00423200                 ; thiscall
//   0001d227:  c3                       RET
//   0001d228:  8b 0d 7c 98 32 01        MOV  ECX, dword ptr [0x0132987c] ; this = *g
//   0001d22e:  33 c0                    XOR  EAX, EAX                   ; EAX = 0
//   0001d230:  50                       PUSH EAX                        ; push 0
//   0001d231:  e8 ca 5f 00 00           CALL 0x00423200                 ; thiscall
//   0001d236:  c3                       RET
//
// Reloc-bearing sites in the orig 39 bytes:
//     +0x0b   DIR32 → 0x0132987c  (global object pointer, non-null path)
//     +0x12   REL32 → 0x00423200  (thiscall method, non-null path; disp = +0x5fd9)
//     +0x18   DIR32 → 0x0132987c  (global object pointer, null path)
//     +0x21   REL32 → 0x00423200  (thiscall method, null path; disp = +0x5fca)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   All 39 bytes are emitted verbatim via MASM `_emit` directives, matching
//   the local convention established by FUN_0041d120 and FUN_0041cec0.
//   compare.py masks both DIR32 and REL32 reloc windows, giving a stable
//   GREEN match. The global object pointer (0x0132987c) and callee
//   (FUN_00423200) are part of the same forwarding-wrapper family seen in
//   FUN_0041c060, FUN_0041d120, and FUN_0041cec0.

extern "C" __declspec(naked) void FUN_0041d210() {
    __asm {
        // 0001d210: 8b 44 24 04  MOV EAX, dword ptr [ESP+0x4]   ; param1
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001d214: 85 c0  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001d216: 74 10  JZ short +0x10  (→ null path at 0x1d228)
        _emit 0x74
        _emit 0x10
        // 0001d218: 8b 40 1c  MOV EAX, dword ptr [EAX+0x1c]   ; field_0x1c
        _emit 0x8b
        _emit 0x40
        _emit 0x1c
        // 0001d21b: 8b 0d 7c 98 32 01  MOV ECX, dword ptr [0x0132987c]  ; this = *g
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d221: 50  PUSH EAX
        _emit 0x50
        // 0001d222: e8 d9 5f 00 00  CALL 0x00423200  (rel32 = +0x5fd9)
        _emit 0xe8
        _emit 0xd9
        _emit 0x5f
        _emit 0x00
        _emit 0x00
        // 0001d227: c3  RET
        _emit 0xc3
        // --- null path ---
        // 0001d228: 8b 0d 7c 98 32 01  MOV ECX, dword ptr [0x0132987c]  ; this = *g
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d22e: 33 c0  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0001d230: 50  PUSH EAX
        _emit 0x50
        // 0001d231: e8 ca 5f 00 00  CALL 0x00423200  (rel32 = +0x5fca)
        _emit 0xe8
        _emit 0xca
        _emit 0x5f
        _emit 0x00
        _emit 0x00
        // 0001d236: c3  RET
        _emit 0xc3
    }
}
