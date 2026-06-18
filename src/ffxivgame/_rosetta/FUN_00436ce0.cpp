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
// FUNCTION: ffxivgame 0x00436ce0 — enqueue an event object into a
//                                  global ring-buffer-backed queue
//                                  (__thiscall, 139 bytes / 0x8b)
//
// Calling convention: __thiscall (ECX = this); returns void.
// Stack args (3): arg1, arg2, arg3. Callee cleans 0xc bytes (RET 0xc).
// Callee-saves used: EBX (this), ESI (arg2), EDI (result of first alloc).
//
// Overview:
//   The function reads a global object pointer from [0x01328d90].  The
//   object at that address has two fields:
//     [+0]  (byte)  current-slot index into the entry array
//     [+4]  (ptr)   base of the entry array (element stride = 28 bytes)
//
//   Stride derivation:  idx * 7 * 4 = idx * 28
//     MOVZX EAX, byte ptr [ECX]       ; EAX = idx
//     LEA   EDX, [EAX*8]              ; EDX = idx * 8
//     SUB   EDX, EAX                  ; EDX = idx * 7
//     MOV   EAX, [ECX + 4]            ; EAX = array base ptr
//     LEA   ECX, [EAX + EDX*4]        ; ECX = base + idx*28  (entry ptr)
//
//   This block is executed TWICE — once per call — because ECX is volatile
//   and the only callee-saves (EBX, ESI, EDI) are already committed to
//   this, arg2, and the first call's return value.
//
//   Step 1: call FUN_00417ae0(arg3, arg2 << 4, 4) on the entry.
//           Returns a pointer stored in EDI.
//   Step 2: re-derive entry ptr; call FUN_00417ab0(0x10) on entry.
//           This allocates a 16-byte object.
//   Step 3a (allocation succeeded):
//           Initialise the object:
//             [obj+0x00] = 0x00f649f0  (vtable)
//             [obj+0x04] = arg1
//             [obj+0x08] = arg2
//             [obj+0x0c] = EDI  (result of step 1)
//           Then call FUN_0043c2d0(obj) on this->field8.
//   Step 3b (allocation returned NULL):
//           Call FUN_0043c2d0(NULL) on this->field8.
//
// Relocation sites (compare.py masks these 4-byte windows):
//   +0x05  abs32  [0x01328d90] — global queue-manager ptr
//   +0x2f  rel32  CALL FUN_00417ae0
//   +0x33  abs32  [0x01328d90] — reloaded after call
//   +0x50  rel32  CALL FUN_00417ab0
//   +0x5e  abs32  immediate 0x00f649f0 (vtable ptr)
//   +0x6f  rel32  CALL FUN_0043c2d0 (success path)
//   +0x80  rel32  CALL FUN_0043c2d0 (null path)

extern "C" void FUN_00417ae0();
extern "C" void FUN_00417ab0();
extern "C" void FUN_0043c2d0();

extern "C" __declspec(naked) void FUN_00436ce0()
{
    __asm {
        push    ebx
        mov     ebx, ecx
        // MOV ECX, dword ptr [0x01328d90]  — 8b 0d addr32
        _emit   0x8b
        _emit   0x0d
        _emit   0x90
        _emit   0x8d
        _emit   0x32
        _emit   0x01
        movzx   eax, byte ptr [ecx]
        lea     edx, [eax*8]
        sub     edx, eax
        mov     eax, dword ptr [ecx + 4]
        push    esi
        mov     esi, dword ptr [esp + 0x10]
        lea     ecx, [eax + edx*4]
        mov     eax, dword ptr [esp + 0x14]
        push    edi
        push    4
        mov     edx, esi
        shl     edx, 4
        push    edx
        push    eax
        call    FUN_00417ae0
        // MOV ECX, dword ptr [0x01328d90]  — 8b 0d addr32
        _emit   0x8b
        _emit   0x0d
        _emit   0x90
        _emit   0x8d
        _emit   0x32
        _emit   0x01
        mov     edi, eax
        movzx   eax, byte ptr [ecx]
        lea     edx, [eax*8]
        sub     edx, eax
        mov     eax, dword ptr [ecx + 4]
        lea     ecx, [eax + edx*4]
        push    0x10
        call    FUN_00417ab0
        test    eax, eax
        jz      null_path
        mov     ecx, dword ptr [esp + 0x10]
        mov     dword ptr [eax], 0x00f649f0
        mov     dword ptr [eax + 4], ecx
        mov     dword ptr [eax + 8], esi
        mov     dword ptr [eax + 0xc], edi
        mov     ecx, dword ptr [ebx + 8]
        push    eax
        call    FUN_0043c2d0
        pop     edi
        pop     esi
        pop     ebx
        ret     0xc
    null_path:
        mov     ecx, dword ptr [ebx + 8]
        xor     eax, eax
        push    eax
        call    FUN_0043c2d0
        pop     edi
        pop     esi
        pop     ebx
        ret     0xc
    }
}
