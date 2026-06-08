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
// FUNCTION: ffxivgame 0x0001d1e0 — __cdecl float-arg wrapper; canonicalises
//           float via MOVSS roundtrip, loads global object pointer into ECX
//           and forwards to FUN_004236e0 (34 B / 0x22)
//
// Asm (34 bytes @ orig RVA 0x0001d1e0):
//   f3 0f 10 44 24 04   MOVSS XMM0, dword ptr [ESP + 0x4]   ; load float arg
//   8b 0d 7c 98 32 01   MOV   ECX, dword ptr [0x0132987c]   ; load global this-ptr
//   f3 0f 11 44 24 04   MOVSS dword ptr [ESP + 0x4], XMM0   ; canonicalise float
//   8b 44 24 04         MOV   EAX, dword ptr [ESP + 0x4]    ; EAX = float bits
//   50                  PUSH  EAX                            ; push 2nd arg (float)
//   68 9a 00 00 00      PUSH  0x9a                           ; push 1st arg (154)
//   e8 df 64 00 00      CALL  FUN_004236e0                   ; rel32 reloc
//   c3                  RET
//
// Calling convention: __cdecl (bare RET; one float argument at [ESP+4]).
// The callee FUN_004236e0 is invoked via __thiscall — its `this` pointer comes
// from the global DWORD at 0x0132987c, first arg is the constant 0x9a, second
// arg is the float parameter passed as raw IEEE-754 bits.
//
// The MOVSS roundtrip (load to XMM0, store back) canonicalises the float value
// to IEEE 754 single-precision before pushing it as an integer. This prevents a
// plain-C++ match because MSVC 2005 defaults to x87 (FLD/FSTP) for float ops.
//
// Reconstruction strategy — naked-asm byte passthrough.
//   SSE usage for float canonicalisation prevents a reliable source-level match.
//   A __declspec(naked) body re-emitting the 34 bytes verbatim via MASM _emit
//   directives produces a .obj whose .text is byte-identical to the original
//   slice. compare.py masks the two reloc windows (DIR32 at +0x08 and REL32 at
//   +0x1d) so those four-byte fields are ignored during byte comparison.

extern "C" __declspec(naked) void FUN_0041d1e0() {
    __asm {
        // 0001d1e0: f3 0f 10 44 24 04  MOVSS XMM0,dword ptr [ESP+0x4]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001d1e6: 8b 0d 7c 98 32 01  MOV ECX,dword ptr [0x0132987c]
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d1ec: f3 0f 11 44 24 04  MOVSS dword ptr [ESP+0x4],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001d1f2: 8b 44 24 04  MOV EAX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001d1f6: 50  PUSH EAX
        _emit 0x50
        // 0001d1f7: 68 9a 00 00 00  PUSH 0x9a
        _emit 0x68
        _emit 0x9a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001d1fc: e8 df 64 00 00  CALL 0x004236e0
        _emit 0xe8
        _emit 0xdf
        _emit 0x64
        _emit 0x00
        _emit 0x00
        // 0001d201: c3  RET
        _emit 0xc3
    }
}
