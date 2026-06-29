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
// FUNCTION: ffxivgame 0x00458d70 — __thiscall cleanup/close method that
//                                   releases a COM-style interface held at
//                                   field_0x14 and clears a handle/index
//                                   at field_0x1c (90 B / 0x5a, no SEH).
//
// Inspection (read from asm/ffxivgame/00058d70_FUN_00458d70.s):
//
//   __thiscall void FUN_00458d70(this);   // ECX = this
//
//   Layout of the owning object (this = ESI):
//     +0x14 : pointer to a COM-style interface object (IFoo*)
//     +0x1c : int handle / index (-1 means "not open")
//
//   COM-style interface vtable layout used here (C-struct function pointers,
//   *this* passed as first pushed argument rather than via ECX):
//     vtable[0]  (+0x00) : int  Open   (IFoo* self, const char* key, IFoo** ppOut)
//     vtable[2]  (+0x08) : void Release(IFoo* self)
//     vtable[4]  (+0x10) : void SetValue(IFoo* self, int value)
//
//   Body:
//
//     if (this->field_0x1c == -1) return;      // already closed — bail
//
//     IFoo* pObj14 = this->field_0x14;
//     IFoo* local_var;
//     int result = pObj14->vtable[0](pObj14, (const char*)0x11088c0, &local_var);
//
//     if (result >= 0) {
//         local_var->vtable[4](local_var, this->field_0x1c);
//         local_var->vtable[2](local_var);
//         this->field_0x1c = -1;
//     }
//
//     pObj14->vtable[2](pObj14);              // always release pObj14
//     this->field_0x14 = 0;
//
// Calling convention: __thiscall — `this` in ECX on entry; no stack args;
// callee cleans nothing (plain RET). One 4-byte local allocated via PUSH ECX
// idiom (MSVC 2005: `push ecx` reserves 4 bytes on stack cheaper than
// `sub esp, 4`). Local is at [ESP+0x4] after the subsequent PUSH ESI.
//
// Branch shape:
//   - Early exit: JZ 0x4d forward (short, within ±127) to POP ESI / POP ECX / RET.
//   - Inner guard: JL 0x23 forward (short, within ±127) to the always-runs tail.
//
// All virtual calls use the C-style pushed-this pattern (not __thiscall ECX
// dispatch): push args right-to-left, push self (IFoo*), read vtable slot,
// call indirectly. No stack cleanup visible after any call → callees use
// __stdcall or clean up themselves.
//
// Absolute-address literal at +0x14: PUSH 0x11088c0 (some .rdata/const string
// used as lookup key for the Open call). Encoded as 68-byte push of a 32-bit
// immediate; compare.py masks the reloc window on the orig binary side, so the
// raw immediate in our naked body matches byte-for-byte.

extern "C" __declspec(naked) void FUN_00458d70() {
    __asm {
        push    ecx
        push    esi
        mov     esi, ecx
        cmp     dword ptr [esi + 0x1c], -1
        jz      done
        mov     eax, dword ptr [esi + 0x14]
        mov     ecx, dword ptr [eax]
        lea     edx, [esp + 0x4]
        push    edx
        push    0x11088c0
        push    eax
        mov     eax, dword ptr [ecx]
        call    eax
        test    eax, eax
        jl      label_b5
        mov     eax, dword ptr [esp + 0x4]
        mov     edx, dword ptr [esi + 0x1c]
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
        mov     dword ptr [esi + 0x1c], 0xffffffff
    label_b5:
        mov     eax, dword ptr [esi + 0x14]
        mov     ecx, dword ptr [eax]
        mov     edx, dword ptr [ecx + 0x8]
        push    eax
        call    edx
        mov     dword ptr [esi + 0x14], 0
    done:
        pop     esi
        pop     ecx
        ret
    }
}
