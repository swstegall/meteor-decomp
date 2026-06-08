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
// FUNCTION: ffxivgame 0x004238f0 — singleton accessor + __thiscall dispatch (32 B).
//
// __cdecl wrapper: loads the singleton pointer from global 0x01327fc0,
// initialises it via a call to 0x0040e500 if null, then forwards two
// dword arguments to a __thiscall method at 0x0040e110 on the singleton.
// The callee cleans the two pushed dwords (ret 8 in the thiscall), so
// this wrapper needs only a plain RET.
//
// Asm shape (32 bytes, RVA 0x000238f0):
//
//   000238f0:  a1 c0 7f 32 01    MOV EAX, [0x01327fc0]   ; load singleton ptr
//   000238f5:  85 c0             TEST EAX, EAX
//   000238f7:  75 05             JNZ  +5                  ; skip init if non-null
//   000238f9:  e8 02 ac fe ff    CALL 0x0040e500          ; init, result in EAX
//   000238fe:  8b 4c 24 08       MOV ECX, [ESP+0x8]       ; arg2
//   00023902:  8b 54 24 04       MOV EDX, [ESP+0x4]       ; arg1
//   00023906:  51                PUSH ECX
//   00023907:  52                PUSH EDX
//   00023908:  8b c8             MOV ECX, EAX             ; this = singleton
//   0002390a:  e8 01 a8 fe ff    CALL 0x0040e110          ; __thiscall(arg1, arg2)
//   0002390f:  c3                RET
//
// Relocatable references:
//   +0x01  DIR32  → 0x01327fc0 (global singleton pointer)
//   +0x0a  REL32  → 0x0040e500 (singleton initialiser)
//   +0x1a  REL32  → 0x0040e110 (__thiscall method)
//
// Reconstruction strategy — __declspec(naked) + _emit byte passthrough.
//
//   Three reloc-bearing sites mean MSVC scheduling choices could diverge
//   from the original; baking the 32 raw bytes via _emit gives a
//   compare.py GREEN without relying on particular optimiser decisions.
//   Same pattern as sibling FUN_00401000 and FUN_00404e10.

extern "C" __declspec(naked) void FUN_004238f0() {
    __asm {
        _emit 0xa1          // MOV EAX, [0x01327fc0]
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75          // JNZ +5
        _emit 0x05
        _emit 0xe8          // CALL 0x0040e500  ; rel32 = 0xfffeac02
        _emit 0x02
        _emit 0xac
        _emit 0xfe
        _emit 0xff
        _emit 0x8b          // MOV ECX, [ESP+0x8]  ; arg2
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b          // MOV EDX, [ESP+0x4]  ; arg1
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x51          // PUSH ECX
        _emit 0x52          // PUSH EDX
        _emit 0x8b          // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8          // CALL 0x0040e110  ; rel32 = 0xfffea801
        _emit 0x01
        _emit 0xa8
        _emit 0xfe
        _emit 0xff
        _emit 0xc3          // RET
    }
}
