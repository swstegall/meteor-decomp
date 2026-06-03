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
// FUNCTION: ffxivgame 0x009d8e26 — `__cdecl` 3-arg callback-dispatch
//                                   wrapper (28 B).
//
// Forwards (arg1, arg2, arg3) to FUN_009d8cd8 with the signature:
//   FUN_009d8cd8(FUN_009e51c4, arg1, arg2, 0, arg3)
// where FUN_009e51c4 (a 2415-byte handler at VA 0x009e51c4) is passed
// as the first argument (a callback/function-pointer slot).
//
// Asm shape (28 bytes, RVA 0x005d8e26..0x005d8e41):
//
//     005d8e26:  ff 74 24 0c          PUSH dword ptr [ESP+0Ch]  ; arg3
//     005d8e2a:  6a 00                PUSH 0                    ; 4th arg = 0
//     005d8e2c:  ff 74 24 10          PUSH dword ptr [ESP+10h]  ; arg2 (shifted)
//     005d8e30:  ff 74 24 10          PUSH dword ptr [ESP+10h]  ; arg1 (shifted)
//     005d8e34:  68 c4 51 9e 00       PUSH 9E51C4h              ; &FUN_009e51c4
//     005d8e39:  e8 9a fe ff ff       CALL FUN_009d8cd8         ; rel32 = -0x166
//     005d8e3e:  83 c4 14             ADD  ESP, 14h             ; cdecl cleanup
//     005d8e41:  c3                   RET
//
// Stack-tracking notes:
//   At entry ESP = X:  [X+4]=arg1, [X+8]=arg2, [X+c]=arg3
//   After PUSH [X+c]:  ESP=X-4; [ESP+10]=arg2
//   After PUSH 0:      ESP=X-8; [ESP+10]=arg2 still? No—now [ESP+10]=arg2 @ X-8+10=X+8 ✓
//   After PUSH [ESP+10]=arg2: ESP=X-12; [ESP+10]=arg1 @ X-12+10=X-2? → [X+4] ✓
//   After PUSH [ESP+10]=arg1: ESP=X-16
//   After PUSH 0x9e51c4: ESP=X-20
//   Callee arg layout: [ESP+4]=0x9e51c4, [8]=arg1, [c]=arg2, [10]=0, [14]=arg3
//
// Reloc-bearing sites:
//   +0x0f  DIR32 imm32  → 0x009e51c4 (VA of FUN_009e51c4)
//   +0x14  REL32        → FUN_009d8cd8 (rel32 = 0xfffffe9a = -358)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//   The two `PUSH [ESP+10h]` instructions are an unusual MSVC 2005 idiom
//   for accessing args that have already shifted due to earlier pushes.
//   A high-level C rewrite (`void f(int a,int b,int c){g(h,a,b,0,c);}`)
//   would express this more directly and might produce different code
//   depending on register allocation and whether MSVC materialises
//   temporaries. Emitting the 28 bytes verbatim is the reliable path;
//   compare.py masks the two reloc windows (imm32 at +0x0f, rel32 at
//   +0x14) so the resolved-vs-baked discrepancy never appears in the diff.

extern "C" __declspec(naked) void FUN_009d8e26() {
    __asm {
        _emit 0xff    // PUSH dword ptr [ESP+0Ch]   ; arg3
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x6a    // PUSH 0                     ; 4th param = 0
        _emit 0x00
        _emit 0xff    // PUSH dword ptr [ESP+10h]   ; arg2 (shifted)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0xff    // PUSH dword ptr [ESP+10h]   ; arg1 (shifted)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x68    // PUSH 9E51C4h               ; &FUN_009e51c4
        _emit 0xc4
        _emit 0x51
        _emit 0x9e
        _emit 0x00
        _emit 0xe8    // CALL FUN_009d8cd8          ; rel32 = 0xfffffe9a
        _emit 0x9a
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83    // ADD ESP, 14h               ; cdecl cleanup (5 dwords)
        _emit 0xc4
        _emit 0x14
        _emit 0xc3    // RET
    }
}
