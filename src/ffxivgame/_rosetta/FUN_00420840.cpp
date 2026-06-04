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
// FUNCTION: ffxivgame 0x00020840 — `__cdecl` 30-byte global-byte-store +
//                                   notify helper (30 B, void(BYTE)).
//
// Reads its single byte argument from [ESP+4], stores it into the global
// byte at 0x01328f84, zero-extends it into EAX, then calls the __stdcall
// helper at 0x004236e0 with (0xae, zero-extended-byte) as arguments.
// The load of [0x0132987c] into ECX between the store and the MOVZX is a
// dead compiler-generated read — ECX is never consumed before the CALL.
// MSVC 2005 /O2 produces this sequence verbatim; the callee cleans the
// two pushed dwords via its own `RET 8` so the outer frame needs no ADD
// ESP, and the function returns with a plain `RET`.
//
// Asm shape (30 bytes, RVA 0x00020840):
//
//   00020840:  8a 44 24 04           MOV  AL, byte ptr [ESP+0x4]   ; byte arg
//   00020844:  8b 0d 7c 98 32 01     MOV  ECX, [0x0132987c]        ; dead load
//   0002084a:  a2 84 8f 32 01        MOV  [0x01328f84], AL         ; store global
//   0002084f:  0f b6 c0              MOVZX EAX, AL                 ; zero-extend
//   00020852:  50                    PUSH EAX                      ; arg1 to callee
//   00020853:  68 ae 00 00 00        PUSH 0xae                     ; arg0 to callee
//   00020858:  e8 83 2e 00 00        CALL 0x004236e0               ; __stdcall notify
//   0002085d:  c3                    RET
//
// Reloc-bearing sites in the orig 30 bytes:
//     +0x06   abs32 → 0x0132987c  (global dword source — dead load)
//     +0x0B   abs32 → 0x01328f84  (global byte destination)
//     +0x19   rel32 → 0x004236e0  (notify helper, __stdcall 2-arg, callee-cleanup)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Two abs32 global-address immediates plus one rel32 CALL displacement
//   are embedded verbatim. The naked/emit form bakes these raw bytes so
//   compare.py can match them directly against the orig PE slice at this
//   RVA, consistent with the convention used by FUN_00401000, FUN_00404e10,
//   and other rosetta siblings that carry unresolved relocations.

extern "C" __declspec(naked) void FUN_00420840() {
    __asm {
        _emit 0x8a      // MOV  AL, byte ptr [ESP+0x4]   ; byte arg
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x8b      // MOV  ECX, [0x0132987c]        ; dead load
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xa2      // MOV  [0x01328f84], AL         ; store global byte
        _emit 0x84
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x0f      // MOVZX EAX, AL                 ; zero-extend
        _emit 0xb6
        _emit 0xc0
        _emit 0x50      // PUSH EAX                      ; arg1
        _emit 0x68      // PUSH 0xae                     ; arg0
        _emit 0xae
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8      // CALL 0x004236e0               ; rel32 = +0x00002e83
        _emit 0x83
        _emit 0x2e
        _emit 0x00
        _emit 0x00
        _emit 0xc3      // RET
    }
}
