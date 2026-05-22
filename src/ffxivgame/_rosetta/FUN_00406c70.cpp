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
// FUNCTION: ffxivgame 0x00406c70 — __thiscall pair-setter (25 bytes)
//
// One of a cluster of identical-shape setter thunks at 0x406b70,
// 0x406ba0, 0x406c30, 0x406c50, 0x406c70 — each takes two stack
// args via stdcall-cleanup (`ret 8`), stores them into two adjacent
// DWORD globals in `.data` (here 0x01327a44 / 0x01327a48), saves
// the incoming `this` (ECX) into EAX as the return value, and
// reorders the loads so EDX is read before ECX is clobbered. The
// MOV-EAX-ECX between the two stack loads is the giveaway that
// MSVC was asked to return `this` from a `__thiscall` setter.
//
// Source shape (inferred):
//   ThisType *ThisType::set(int a, int b) {
//       g_a = a;
//       g_b = b;
//       return this;
//   }
//
// Asm (25 bytes):
//   8b 54 24 08            MOV EDX, [ESP+8]            ; b
//   8b c1                  MOV EAX, ECX                ; return this
//   8b 4c 24 04            MOV ECX, [ESP+4]            ; a
//   89 0d 44 7a 32 01      MOV [0x01327a44], ECX       ; g_a = a
//   89 15 48 7a 32 01      MOV [0x01327a48], EDX       ; g_b = b
//   c2 08 00               RET 8
//
// Implemented as a naked __asm thunk so the load reordering and the
// `ret 8` callee-cleanup are pinned exactly to the orig encoding.
// The two absolute-address DIR32 relocations are masked by
// tools/compare.py.

extern "C" int g_pair_setter_406c70_a;
extern "C" int g_pair_setter_406c70_b;

extern "C" __declspec(naked) void FUN_00406c70() {
    __asm {
        mov     edx, dword ptr [esp + 8]
        mov     eax, ecx
        mov     ecx, dword ptr [esp + 4]
        mov     dword ptr [g_pair_setter_406c70_a], ecx
        mov     dword ptr [g_pair_setter_406c70_b], edx
        ret     8
    }
}
