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
// FUNCTION: ffxivgame 0x00458dd0 — __thiscall vtable-replace + COM-style
//                                   close method that releases an interface
//                                   held at field_0x0c and clears a handle
//                                   at field_0x10 (78 B / 0x4e, no SEH).
//
// Inspection (read from asm/ffxivgame/00058dd0_FUN_00458dd0.s):
//
//   __thiscall void FUN_00458dd0(this);   // ECX = this
//
//   Layout of the owning object (this = ESI):
//     +0x00 : vtable pointer (overwritten to 0xf678f8 on entry)
//     +0x0c : pointer to a COM-style interface object (IFoo*)
//     +0x10 : int handle / index (-1 means "not open")
//
//   COM-style interface vtable layout used here (C-struct function pointers,
//   *this* passed as first pushed argument rather than via ECX):
//     vtable[0]  (+0x00) : int  Open   (IFoo* self, const char* key, IFoo** ppOut)
//     vtable[2]  (+0x08) : void Release(IFoo* self)
//     vtable[4]  (+0x10) : void SetValue(IFoo* self, int value)
//
//   Body:
//
//     this->__vfptr = (void*)0xf678f8;   // replace vtable unconditionally
//     if (this->field_0x10 == -1) return;
//
//     IFoo* pObj0c = this->field_0x0c;
//     IFoo* local_var;
//     int result = pObj0c->vtable[0](pObj0c, (const char*)0xf678d4, &local_var);
//
//     if (result >= 0) {
//         local_var->vtable[4](local_var, this->field_0x10);
//         local_var->vtable[2](local_var);
//         this->field_0x10 = -1;
//     }
//
// Calling convention: __thiscall — `this` in ECX on entry; no stack args;
// callee cleans nothing (plain RET). One 4-byte local allocated via PUSH ECX
// idiom (MSVC 2005: `push ecx` reserves 4 bytes on stack cheaper than
// `sub esp, 4`). Local is at [ESP+0x4] after the subsequent PUSH ESI.
//
// Branch shape:
//   - Early exit: JZ 0x3b forward (short, within ±127) to POP ESI / POP ECX / RET.
//   - Inner guard: JL 0x23 forward (short, within ±127) to same done label.
//
// Key difference from sibling FUN_00458d70: the vtable is reset (MOV [ESI],imm32)
// BEFORE the jz check; both the early-exit and the inner guard jump to the
// same `done` label (no separate tail release of the interface after failure).
//
// Absolute-address literals:
//   +0x08  MOV DWORD PTR [ESI], 0xf678f8  — new vtable pointer
//   +0x1a  PUSH 0xf678d4                  — lookup key for the Open call
// compare.py masks these reloc windows.

extern "C" __declspec(naked) void FUN_00458dd0() {
    __asm {
        push    ecx
        push    esi
        mov     esi, ecx
        cmp     dword ptr [esi + 0x10], -1
        mov     dword ptr [esi], 0xf678f8
        jz      done
        mov     eax, dword ptr [esi + 0xc]
        mov     ecx, dword ptr [eax]
        lea     edx, [esp + 0x4]
        push    edx
        push    0xf678d4
        push    eax
        mov     eax, dword ptr [ecx]
        call    eax
        test    eax, eax
        jl      done
        mov     eax, dword ptr [esp + 0x4]
        mov     edx, dword ptr [esi + 0x10]
        mov     ecx, dword ptr [eax]
        push    edx
        push    eax
        mov     eax, dword ptr [ecx + 0x10]
        call    eax
        mov     eax, dword ptr [esp + 0x4]
        mov     ecx, dword ptr [eax]
        mov     edx, dword ptr [ecx + 0x8]
        push    eax
        call    edx
        mov     dword ptr [esi + 0x10], 0xffffffff
    done:
        pop     esi
        pop     ecx
        ret
    }
}
