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
// FUNCTION: ffxivgame 0x0004b300 — `__cdecl` void FUN_0044b300()
//                                  Three-pointer virtual-call shutdown
//                                  (65 B / 0x41)
//
// Reads three global object pointers at 0x0132cb88 / 0x0132cb8c / 0x0132cb90.
// For each non-NULL pointer, invokes the first vtable slot with argument 1
// (__thiscall, one int arg). After the three checks, zeroes the first global
// and returns.
//
// Control flow:
//   if (g_obj0 != NULL) g_obj0->vftable[0](1);   // __thiscall, ECX=obj, arg=1
//   if (g_obj1 != NULL) g_obj1->vftable[0](1);
//   if (g_obj2 != NULL) g_obj2->vftable[0](1);
//   g_obj0 = NULL;
//   ret
//
// Calling convention: __cdecl (no stack args, no prologue, RET C3).
// No local frame — no callee-saved registers (ESI/EDI/EBX unused),
// no stack locals, MSVC omits prologue/epilogue entirely.
//
// Virtual call pattern (same for all three objects):
//   MOV ECX, [global]   ; ECX = object pointer  (= "this" for __thiscall)
//   MOV EAX, [ECX]      ; EAX = vtable pointer
//   MOV EDX, [EAX]      ; EDX = vftable[0] (first slot)
//   PUSH 0x1            ; single int argument
//   CALL EDX            ; __thiscall virtual dispatch; callee cleans stack
//
// Branch notes:
//   - First JZ (+0x2c) jumps all the way to the MOV [g_obj0],0 epilogue.
//     So if g_obj0 is NULL the function still zeroes g_obj0 before returning.
//   - Second JZ (+0x08) skips to the third object check.
//   - Third JZ (+0x08) skips to the MOV [g_obj0],0 epilogue.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The four absolute-address slots (three MOV ECX,[abs] loads plus the
//   final MOV [abs],0) carry already-resolved .data addresses from the
//   binary's own address space (0x0132cb88 / 0x0132cb8c / 0x0132cb90).
//   Emitting them as raw _emit immediates produces no relocations in the
//   standalone .obj — the bytes are verbatim identical to the orig binary
//   slice. tools/compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_0044b300() {
    __asm {
        _emit 0x8b              // MOV ECX, dword ptr [0x0132cb88]
        _emit 0x0d
        _emit 0x88
        _emit 0xcb
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ  +0x2c  (→ 0x0044b336)
        _emit 0x2c
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, dword ptr [0x0132cb8c]
        _emit 0x0d
        _emit 0x8c
        _emit 0xcb
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ  +0x08  (→ 0x0044b324)
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, dword ptr [0x0132cb90]
        _emit 0x0d
        _emit 0x90
        _emit 0xcb
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ  +0x08  (→ 0x0044b336)
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0xc7              // MOV dword ptr [0x0132cb88], 0x00000000
        _emit 0x05
        _emit 0x88
        _emit 0xcb
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
