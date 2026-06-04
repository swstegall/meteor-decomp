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
// FUNCTION: ffxivgame 0x0041b9d0 — __cdecl two-pointer compare-and-notify
//                                   (61 bytes / 0x3d)
//
// Signature (inferred):
//   void __cdecl FUN_0041b9d0(SomeNode *src, SomeNode *dst);
//
// Shape:
//   1. Load src → ESI, dst → EDI.
//   2. If src == dst, return immediately.
//   3. Call a two-arg log/assert helper (FUN_009fc74a, __stdcall)
//      with a string literal at 0x00f5947c and 0 as args.
//   4. Read src->field_0x28 into ECX and dst->field_0x28 into EAX.
//   5. Load the global object pointer at 0x0132987c into ECX (__thiscall
//      "this") and call FUN_00423310 with five stack args:
//        (src->field_0x28, 0, dst->field_0x28, 0, 2)
//   6. Restore EDI, ESI, then tail-call FUN_009fc750.
//
// Reloc-bearing sites (masked by tools/compare.py):
//   +0x0e  PUSH imm32   → string literal VA 0x00f5947c  (ABS32 reloc)
//   +0x15  CALL rel32   → FUN_009fc74a  (VA 0x009fc74a)
//   +0x28  MOV ECX,[m32]→ global pointer VA 0x0132987c  (ABS32 reloc)
//   +0x2e  CALL rel32   → FUN_00423310  (VA 0x00423310)
//   +0x35  JMP  rel32   → FUN_009fc750  (VA 0x009fc750)
//
// Calling conventions (inferred from stack discipline):
//   FUN_009fc74a  — __stdcall 2 args (callee-cleans 8; no ADD ESP after call)
//   FUN_00423310  — __thiscall 5 stack args (callee-cleans 20)
//   FUN_009fc750  — tail-call target with the same frame as FUN_0041b9d0

extern "C" char g_str_f5947c[];             // string literal at VA 0x00f5947c
extern "C" void FUN_009fc74a();             // __stdcall log/assert helper (2 args)
extern "C" void FUN_009fc750();             // tail-call target
extern "C" void FUN_00423310();             // __thiscall method (5 stack args)
extern "C" void *g_obj_132987c;             // global object pointer at VA 0x0132987c

extern "C" __declspec(naked) void FUN_0041b9d0() {
    __asm {
        push    esi
        mov     esi, dword ptr [esp + 0x8]
        push    edi
        mov     edi, dword ptr [esp + 0x10]
        cmp     esi, edi
        jz      same
        push    offset g_str_f5947c
        push    0
        call    FUN_009fc74a
        mov     eax, dword ptr [edi + 0x28]
        mov     ecx, dword ptr [esi + 0x28]
        push    2
        push    0
        push    eax
        push    0
        push    ecx
        mov     ecx, dword ptr [g_obj_132987c]
        call    FUN_00423310
        pop     edi
        pop     esi
        jmp     FUN_009fc750
    same:
        pop     edi
        pop     esi
        ret
    }
}
