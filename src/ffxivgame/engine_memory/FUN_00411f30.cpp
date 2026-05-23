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
// FUNCTION: ffxivgame 0x00011f30 — thread-safe push_front into a spinlocked
//           doubly-linked free-list  (__thiscall, 1 stack arg, void, RET 4)
//
// Asm (103 bytes @ orig RVA 0x00011f30):
//
//   Prologue:  PUSH EBX / PUSH ESI / MOV EBX,ECX / PUSH EDI
//   EDI = this->field_0x10          (vtable-bearing inner object, callee-saved)
//   Call  EDI->vtable[0x2c/4]()     (lock / begin-access, vtable slot 11)
//   ESI  = param_1  ([ESP+0x10])
//   Call  param_1->vtable[0](0)     (virtual destructor, no-delete flag)
//   Call  this->field_0x10->vtable[0x4/4]()  → EAX  (vtable slot 1)
//   EAX  = *(EAX + 0x10)            (inner lock/list object)
//   EDX  = &EAX->field_4            (spinlock address)
//   NOP  (alignment pad at +0x2f)
//   Spin loop:
//     ECX = 1  ;  EBX = EDX  ;  XCHG [EBX], ECX  ;  TEST ECX,ECX  ;  JNZ loop
//   (loop exits when old spinlock value was 0 — lock acquired)
//   ECX = *(EAX + 0xc)              (list sentinel / head node)
//   EBX = *(ECX + 4)                (old sentinel→next)
//   Insert ESI (param_1) between sentinel and old-next (push_front):
//     [EBX]    = ESI
//     ESI[4]   = EBX
//     [ESI]    = ECX
//     ECX[4]   = ESI
//   [EAX+0x18] -= 1                 (free-slot count decrement)
//   EAX = 0  ;  XCHG [EDX], EAX    (release spinlock)
//   Call  EDI->vtable[0x30/4]()     (unlock / end-access, vtable slot 12)
//   Epilogue: POP EDI / POP ESI / POP EBX / RET 4
//
// Register reuse note: EBX holds `this` through the first three vtable
// calls, then is freed (last use at 0x11f4f MOV ECX,[EBX+0x10]).  MSVC
// reuses EBX as a copy of EDX inside the spinlock loop — this produces
// the MOV EBX,EDX inside the spin body rather than using EDX directly.
// The NOP at +0x2f is a MSVC alignment pad before the hot loop.

#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_00411f30() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00411f30()
{
    __asm {
        // --- prologue ---
        push    ebx
        push    esi
        mov     ebx, ecx                    // EBX = this
        push    edi
        // --- EDI = this->field_0x10 ---
        mov     edi, dword ptr [ebx + 0x10]
        // --- call this->field_0x10->vtable[0x2c/4]() ---
        mov     eax, dword ptr [edi]
        mov     edx, dword ptr [eax + 0x2c]
        mov     ecx, edi
        call    edx
        // --- ESI = param_1 (1st stack arg) ---
        mov     esi, dword ptr [esp + 0x10]
        // --- call param_1->vtable[0](0) ---
        mov     eax, dword ptr [esi]
        mov     edx, dword ptr [eax]
        push    0
        mov     ecx, esi
        call    edx
        // --- call this->field_0x10->vtable[0x4/4]() ---
        mov     ecx, dword ptr [ebx + 0x10]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x4]
        call    edx
        // --- EAX = *(retval + 0x10); EDX = &EAX->field_4 (spinlock) ---
        mov     eax, dword ptr [eax + 0x10]
        lea     edx, [eax + 0x4]
        nop                                 // alignment pad
        // --- spinlock acquire loop ---
    _loop:
        mov     ecx, 1
        mov     ebx, edx                    // MSVC reuses EBX here
        xchg    dword ptr [ebx], ecx
        test    ecx, ecx
        jnz     _loop
        // --- insert ESI into doubly-linked list at ECX->next ---
        mov     ecx, dword ptr [eax + 0xc]
        mov     ebx, dword ptr [ecx + 0x4]
        mov     dword ptr [ebx], esi
        mov     ebx, dword ptr [ecx + 0x4]
        mov     dword ptr [esi + 0x4], ebx
        mov     dword ptr [esi], ecx
        mov     dword ptr [ecx + 0x4], esi
        // --- decrement free-slot count ---
        add     dword ptr [eax + 0x18], -1
        // --- release spinlock ---
        xor     eax, eax
        xchg    dword ptr [edx], eax
        // --- call this->field_0x10->vtable[0x30/4]() ---
        mov     edx, dword ptr [edi]
        mov     eax, dword ptr [edx + 0x30]
        mov     ecx, edi
        call    eax
        // --- epilogue ---
        pop     edi
        pop     esi
        pop     ebx
        ret     4
    }
}
#endif
