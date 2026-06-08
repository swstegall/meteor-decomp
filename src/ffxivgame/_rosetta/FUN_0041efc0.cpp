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
// FUNCTION: ffxivgame 0x0041efc0 — __cdecl dispatcher that looks up a handler
// entry in a global function-pointer table indexed by arg1, acquires a handle
// via FUN_0041c540, and then forwards the call through a global singleton's
// __thiscall method (FUN_004232e0).
//
// Shape (from asm at RVA 0x0001efc0, 88 bytes):
//
//   void FUN_0041efc0(int arg1, int arg2, int arg3, int arg4, int arg5)
//   {
//       if (!arg2) return;
//       void* entry = g_handler_table[arg1];   // [EAX*4 + 0xF595C4]
//       if (!entry) return;
//       void* handle = FUN_0041c540(arg1);     // -> ESI (persists to end)
//       FUN_0041ed70(arg3);                    // return value discarded
//       // __thiscall: ECX = *g_singleton_obj (global at 0x0132987C)
//       // four stack args pushed right-to-left: arg4, arg5, handle, entry
//       g_singleton_obj->FUN_004232e0(entry, handle, arg5, arg4);
//       FUN_0041d240(0);                       // __cdecl, 1 arg
//       FUN_004246f0(handle);                  // __cdecl, 1 arg
//       // ADD ESP, 8 cleans the two cdecl calls above
//   }
//
// Calling convention: __cdecl — plain RET, caller owns all arg cleanup.
//
// Register allocation:
//   ECX = arg2     (loaded at entry for the null guard, then repurposed)
//   EAX = arg1     (loaded second; kept in EAX for [EAX*4+table] SIB index
//                   and pushed directly as the single arg to FUN_0041c540)
//   EDI = entry    (table[arg1]; callee-saved, persists through all calls)
//   ESI = handle   (return from FUN_0041c540; callee-saved, persists to end)
//
// Stack accounting:
//   Entry ESP = X.
//   PUSH EDI, PUSH ESI: ESP = X-8.
//   PUSH EAX (arg1), CALL FUN_0041c540 (__cdecl, no cleanup): ESP = X-12.
//   PUSH EAX (arg3), CALL FUN_0041ed70 (__cdecl, no cleanup): ESP = X-16.
//   ADD ESP, 8 (cleans both cdecl calls): ESP = X-8.
//   PUSH ECX (arg4), PUSH EDX (arg5), PUSH ESI, PUSH EDI,
//     CALL FUN_004232e0 (__thiscall, callee cleans 4*4=16 bytes): ESP = X-8.
//   PUSH 0, CALL FUN_0041d240; PUSH ESI, CALL FUN_004246f0 (__cdecl): ESP = X-16.
//   ADD ESP, 8 (cleans those two): ESP = X-8.
//   POP ESI (at X-8), POP EDI (at X-4), RET: ESP = X. Balanced.
//
// Jump distances (both short, fits in int8):
//   jz done (offset 6): target ret at offset 87 → disp = 0x4f (74 4f) ✓
//   jz pop_edi_done (offset 22): target pop edi at offset 86 → disp = 0x3e (74 3e) ✓
//
// All cross-references (5× CALL rel32, 2× DIR32 for global addresses) are
// masked by tools/compare.py via the COFF relocation table, so the .obj
// is byte-identical to the orig slice modulo those linker fixup bytes.

extern "C" void FUN_0041c540();
extern "C" void FUN_0041ed70();
extern "C" void FUN_004232e0();
extern "C" void FUN_0041d240();
extern "C" void FUN_004246f0();
extern "C" void* g_handler_table[];   // global table base at 0x00F595C4
extern "C" void* g_singleton_obj;     // global singleton pointer at 0x0132987C

extern "C" __declspec(naked) void FUN_0041efc0() {
    __asm {
        mov     ecx, dword ptr [esp + 8]
        test    ecx, ecx
        jz      done
        mov     eax, dword ptr [esp + 4]
        push    edi
        mov     edi, dword ptr [eax * 4 + g_handler_table]
        test    edi, edi
        jz      pop_edi_done
        push    esi
        push    eax
        call    FUN_0041c540
        mov     esi, eax
        mov     eax, dword ptr [esp + 0x18]
        push    eax
        call    FUN_0041ed70
        mov     ecx, dword ptr [esp + 0x20]
        mov     edx, dword ptr [esp + 0x24]
        add     esp, 8
        push    ecx
        mov     ecx, dword ptr [g_singleton_obj]
        push    edx
        push    esi
        push    edi
        call    FUN_004232e0
        push    0
        call    FUN_0041d240
        push    esi
        call    FUN_004246f0
        add     esp, 8
        pop     esi
    pop_edi_done:
        pop     edi
    done:
        ret
    }
}
