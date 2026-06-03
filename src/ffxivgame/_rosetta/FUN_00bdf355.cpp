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
// FUNCTION: ffxivgame 0x00bdf355 — iterator over an object array, dispatching
//                                  vtable slot 23 on each element; returns true
//                                  on the first truthy result, false if exhausted.
//
// This function is reached exclusively via a tail-call JMP from FUN_00bdf330
// (at RVA 0x007df330, 37 bytes). FUN_00bdf330 is a __thiscall guard that:
//   1. PUSH EDI      — saves caller's EDI (callee-save obligation)
//   2. MOV EDI, ECX — loads 'this' into EDI
//   3. Tests a flag at [EDI+4]; if clear, logs and returns false (RET 0x10)
//   4. JNZ here (offset +37 from FUN_00bdf330's start = our entry point)
//
// At entry to FUN_00bdf355:
//   EDI              = this (set by FUN_00bdf330 step 2)
//   [ESP+0]          = FUN_00bdf330's saved EDI (pushed in step 1)
//   [ESP+4]          = return address (back to FUN_00bdf330's caller)
//   [ESP+8]          = arg1  (passed to FUN_00bdf330, unused here)
//   [ESP+0x0c]       = arg2  (→ third  explicit arg of the virtual call)
//   [ESP+0x10]       = arg3  (→ second explicit arg of the virtual call)
//   [ESP+0x14]       = arg4  (→ first  explicit arg of the virtual call)
//
// After PUSH EBX / PUSH EBP / PUSH ESI, stack offsets shift by 12:
//   [ESP+0x18]  = arg2   (pushed first to virtual call as EAX)
//   [ESP+0x1c]  = arg3   (held in EBP; pushed second)
//   [ESP+0x20]  = arg4   (held in EBX; pushed third)
//
// this->count = [EDI+0x0c]   (number of elements)
// this->array = [EDI+0x10]   (pointer to pointer array)
// element     = array[i]     = [array + i*4]
// vtable call = element->vtable[23](arg2, arg3, arg4)   (__thiscall on element)
//              (vtable entry at byte offset 0x5c = 23 * 4)
//
// Both exit paths unwind FUN_00bdf330's saved-EDI slot via POP EDI before
// RET 0x10, which returns to FUN_00bdf330's caller and cleans 4 dword args.
//
// No linker-resolved relocations — the virtual dispatch uses CALL EDX (register
// indirect), so the .obj is byte-identical to the orig without any reloc masking.
//
// Reconstruction strategy: __declspec(naked) inline asm.
//   The asymmetric push/pop of EDI cannot be expressed in a standard C++ source
//   function (no calling convention passes 'this' via EDI without a corresponding
//   push in the callee's own prolog). Naked asm reproduces the 68 bytes verbatim.

extern "C" __declspec(naked) void FUN_00bdf355()
{
    __asm {
        push    ebx                             // 53
        push    ebp                             // 55
        push    esi                             // 56
        xor     esi, esi                        // 33 f6  — i = 0
        cmp     dword ptr [edi + 0x0c], esi     // 39 77 0c  — count vs 0 (r/m32, r32 form)
        jle     end_false                       // 7e 28
        mov     ebx, dword ptr [esp + 0x20]     // 8b 5c 24 20  — arg4 → EBX (pushed last to call)
        mov     ebp, dword ptr [esp + 0x1c]     // 8b 6c 24 1c  — arg3 → EBP (pushed second)
    loop_start:
        mov     eax, dword ptr [edi + 0x10]     // 8b 47 10  — EAX = this->array
        mov     ecx, dword ptr [eax + esi*4]    // 8b 0c b0  — ECX = array[i]
        mov     edx, dword ptr [ecx]            // 8b 11     — EDX = element->vtable
        mov     eax, dword ptr [esp + 0x18]     // 8b 44 24 18  — EAX = arg2 (pushed first)
        mov     edx, dword ptr [edx + 0x5c]     // 8b 52 5c  — EDX = vtable[23]
        push    ebx                             // 53  — arg4
        push    ebp                             // 55  — arg3
        push    eax                             // 50  — arg2
        call    edx                             // ff d2  — element->vtable[23](arg2, arg3, arg4)
        test    al, al                          // 84 c0
        jnz     return_true                     // 75 11
        add     esi, 1                          // 83 c6 01  — ++i
        cmp     esi, dword ptr [edi + 0x0c]     // 3b 77 0c  — i vs count (r32, r/m32 form)
        jl      loop_start                      // 7c e0
    end_false:
        pop     esi                             // 5e
        pop     ebp                             // 5d
        pop     ebx                             // 5b
        xor     al, al                          // 32 c0  — return false
        pop     edi                             // 5f  — unwind FUN_00bdf330's saved EDI
        ret     16                              // c2 10 00
    return_true:
        pop     esi                             // 5e
        pop     ebp                             // 5d
        pop     ebx                             // 5b
        mov     al, 1                           // b0 01  — return true
        pop     edi                             // 5f  — unwind FUN_00bdf330's saved EDI
        ret     16                              // c2 10 00
    }
}
