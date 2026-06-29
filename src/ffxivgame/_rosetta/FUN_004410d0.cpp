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
// FUNCTION: ffxivgame 0x000410d0 — __cdecl container-insert shim (60 B / 0x3c)
//
// Inspection (read from the disassembly at orig RVA 0x000410d0):
//
//   __cdecl void* FUN_004410d0(arg0, arg1, arg2, arg3, arg4, arg5, arg6)
//   Returns arg0.
//
//   Frame layout (all ESP-relative, no EBP):
//     Prologue:  PUSH ECX (4-byte local slot), PUSH ESI (callee-save)
//     [esp+0x4]  local byte variable, zeroed to 0
//     [esp+0xC]  arg0 (first explicit stack parameter)
//     [esp+0x10] arg1
//     [esp+0x14] arg2
//     [esp+0x18] arg3
//     [esp+0x1C] arg4
//     [esp+0x20] arg5
//     [esp+0x24] arg6
//
//   The function pre-loads arg0 into ECX, EDX, and ESI, initialises a
//   1-byte local to false (0), then calls FUN_008cfd10 (a buffer-insert
//   helper) with 8 arguments built by interleaving register pushes with
//   in-flight stack reads:
//     callee arg0  = arg0   (ESI, pushed last)
//     callee arg1  = arg2   (EAX, read at K=3 from [esp+0x24])
//     callee arg2  = arg4   (EDX, read at K=5 from [esp+0x28])
//     callee arg3  = arg5   (ECX, read at K=4 from [esp+0x28])
//     callee arg4  = arg6   (EAX, read at K=3 from [esp+0x28])
//     callee arg5  = arg0   (EDX original, pre-loaded before argument build)
//     callee arg6  = arg0   (ECX original, pre-loaded before argument build)
//     callee arg7  = 0 (bool local)
//   Returns arg0 in EAX.
//
//   MSVC 2005 register-allocation artefact: arg0 is loaded into both ECX
//   and EDX from [esp+0x08] (after the first PUSH), and again into ESI
//   from [esp+0x0c] (after the second PUSH). All three reads hit the same
//   logical stack slot; the compiler emits three independent loads because
//   it needs three separate registers pre-populated with arg0 as push fodder
//   for callee arg positions 5, 6, and 0 respectively.
//
//   Callee FUN_008cfd10 (at RVA 0x004cfd10 / VA 0x008cfd10) appears to be
//   a buffer-range-insert helper (it checks begin==end and performs a
//   single-byte append into a capacity-tracked buffer structure).
//
// Reloc-bearing sites in the orig 60 bytes:
//     +0x2f   CALL rel32 → FUN_008cfd10 (RVA 0x004cfd10); masked by compare.py
//
// No IAT imports; the sole relocation is the REL32 call, which compare.py
// masks. Naked asm is used to pin the exact register-allocation sequence
// (triple load of arg0, interleaved push/load pattern for the 8 callee
// arguments) that MSVC 2005 emitted for this shim.

extern "C" void FUN_008cfd10();

extern "C" __declspec(naked) void FUN_004410d0() {
    __asm {
        push    ecx                              // allocate 4-byte local slot
        mov     ecx, dword ptr [esp + 0x8]       // ecx = arg0
        mov     edx, dword ptr [esp + 0x8]       // edx = arg0 (independent load)
        push    esi                              // save ESI
        mov     esi, dword ptr [esp + 0xc]       // esi = arg0 (after 2nd push)
        mov     byte ptr [esp + 0x4], 0          // local = false
        mov     eax, dword ptr [esp + 0x4]       // eax = local (dword read)
        push    eax                              // push local (callee arg7)
        mov     eax, dword ptr [esp + 0x28]      // eax = arg6
        push    ecx                              // push ecx=arg0 (callee arg6)
        mov     ecx, dword ptr [esp + 0x28]      // ecx = arg5
        push    edx                              // push edx=arg0 (callee arg5)
        mov     edx, dword ptr [esp + 0x28]      // edx = arg4
        push    eax                              // push eax=arg6 (callee arg4)
        mov     eax, dword ptr [esp + 0x24]      // eax = arg2
        push    ecx                              // push ecx=arg5 (callee arg3)
        push    edx                              // push edx=arg4 (callee arg2)
        push    eax                              // push eax=arg2 (callee arg1)
        push    esi                              // push esi=arg0 (callee arg0)
        call    FUN_008cfd10
        add     esp, 0x20                        // clean 8 callee args
        mov     eax, esi                         // return = arg0
        pop     esi                              // restore ESI
        pop     ecx                              // deallocate local slot
        ret
    }
}
