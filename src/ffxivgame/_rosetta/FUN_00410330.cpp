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
// FUNCTION: ffxivgame 0x00410330 — __cdecl 6-arg factory with MSVC SEH frame
//                                  (142 B / 0x8e)
//
// Calling convention: __cdecl (6 stack args, caller cleans).  Only ESI is
// saved/restored; no EBP frame.
//
// Stack layout at entry (ESP = E):
//   [E+0x00]: return address
//   [E+0x04]: param_1
//   [E+0x08]: param_2
//   [E+0x0c]: param_3  — __thiscall receiver for FUN_0040e110
//   [E+0x10]: param_4
//   [E+0x14]: param_5
//   [E+0x18]: param_6
//
// After the SEH prologue (PUSH -1 / PUSH handler / PUSH prev_FS0 /
// SUB ESP,0x10 / PUSH ESI), ESP = E-32 = E-0x20.  The SEH dwords sit at:
//   [E-0x04]: try-state   (−1 initial → 0 once allocation succeeds)
//   [E-0x08]: SEH handler VA (0x00e54f87)
//   [E-0x0c]: previous FS:[0]
//
// Behaviour:
//   1. Calls FUN_0040e2d0(0x10, 0xf56ca8) __thiscall on a local scratch
//      buffer (ECX = ESP+0x14 relative to post-PUSH state) to initialize
//      some helper object.
//   2. Calls FUN_0040e110(0x6c, result1) __thiscall on param_3.
//      Stores the return value in two locals (MSVC temp-tracking pattern).
//      If null → returns 0.
//   3. Sets SEH try-state to 0 (entering the guarded region).
//   4. Calls FUN_0040f990(param_1..6) __thiscall on the allocation result.
//   5. Returns whatever FUN_0040f990 returns (EAX after the call).
//
// CALL reloc sites masked by compare.py (4-byte rel32 windows):
//   func+0x24: CALL FUN_0040e2d0
//   func+0x35: CALL FUN_0040e110
//   func+0x67: CALL FUN_0040f990

extern "C" {
    void FUN_0040e2d0();    // __thiscall helper init
    void FUN_0040e110();    // __thiscall allocator / lookup
    void FUN_0040f990();    // __thiscall primary method
}

extern "C" __declspec(naked) void FUN_00410330() {
    __asm {
        // SEH prologue
        push    -1
        push    0xe54f87
        mov     eax, dword ptr fs:[0]
        push    eax
        mov     dword ptr fs:[0], esp
        sub     esp, 0x10
        push    esi

        // call FUN_0040e2d0(0x10, 0xf56ca8) on local buffer
        push    0xf56ca8
        push    0x10
        lea     ecx, [esp + 0x14]
        call    FUN_0040e2d0

        // call FUN_0040e110(0x6c, eax) on param_3
        mov     esi, dword ptr [esp + 0x2c]     // esi = param_3
        push    eax
        push    0x6c
        mov     ecx, esi
        call    FUN_0040e110

        mov     dword ptr [esp + 0x4], eax
        mov     dword ptr [esp + 0x8], eax
        test    eax, eax
        mov     dword ptr [esp + 0x1c], 0       // SEH try-state = 0
        jz      short null_result

        // call FUN_0040f990(param_1..6) on eax
        mov     ecx, dword ptr [esp + 0x38]     // param_6
        mov     edx, dword ptr [esp + 0x34]     // param_5
        push    ecx
        mov     ecx, dword ptr [esp + 0x34]     // param_4
        push    edx
        mov     edx, dword ptr [esp + 0x30]     // param_2
        push    ecx
        mov     ecx, dword ptr [esp + 0x30]     // param_1
        push    esi                             // param_3
        push    edx                             // param_2
        push    ecx                             // param_1
        mov     ecx, eax
        call    FUN_0040f990

        pop     esi
        mov     ecx, dword ptr [esp + 0x10]
        mov     dword ptr fs:[0], ecx
        add     esp, 0x1c
        ret

    null_result:
        mov     ecx, dword ptr [esp + 0x14]
        xor     eax, eax
        pop     esi
        mov     dword ptr fs:[0], ecx
        add     esp, 0x1c
        ret
    }
}
