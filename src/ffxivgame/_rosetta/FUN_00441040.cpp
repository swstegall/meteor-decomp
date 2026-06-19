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
// FUNCTION: ffxivgame 0x00441040 — __thiscall constructor/init (132 B).
//
// Copies one byte from *arg1 into this[0], calls FUN_004405e0 to
// initialise a sub-object at this[4] (capturing the return value and
// zeroing an adjacent field), then calls FUN_00440e10 to initialise
// a second sub-object at this[16] passing count=9 and a pointer to
// a local stack slot. Finishes by writing 1 into this[32] and this[36],
// and returns `this` in EAX.
//
// Frame layout (ESP-relative, no EBP, after the full prologue):
//   [ESP+0x00] = GS cookie (XOR'd __security_cookie ^ ESP)
//   [ESP+0x04] = saved EDI
//   [ESP+0x08] = saved ESI
//   [ESP+0x0c] = local[0] — spill of `this`
//   [ESP+0x10] = local[1] — later written with &this[4] (EDI)
//   [ESP+0x14] = local[2] — later written with this[8]
//   [ESP+0x18] = prev FS:[0]  (base of our SEH registration record)
//   [ESP+0x1c] = SEH handler stub (DIR32 → FUN_00e56ffb)
//   [ESP+0x20] = unwind state (initial -1, then 0 on try-entry)
//   [ESP+0x24] = return address
//   [ESP+0x28] = arg1 (pointer — first byte read as `this[0]`)
//   [ESP+0x2c] = arg2 (unused in function body)
//
// Calling conventions:
//   this function : __thiscall, cleans 8 bytes of stack args (RET 0x8)
//   FUN_004405e0  : __thiscall (ECX = &this[4]), no stack args
//   FUN_00440e10  : __thiscall (ECX = &this[16]), 2 stack args → RET 8

extern "C" {

// SEH handler stub registered in FS:[0] for this frame.
int FUN_00e56ffb();

// Sub-object initialiser for the field at this+4.
int FUN_004405e0();

// Sub-object initialiser for the field at this+16;
// called __thiscall with two extra stack args (count, ptr).
int FUN_00440e10();

extern unsigned __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_00441040() {
    __asm {
        // --- SEH / GS prologue (ESP-relative frame, no EBP) ---
        push    -1                              // initial unwind state
        push    offset FUN_00e56ffb             // SEH handler stub (DIR32 reloc)
        mov     eax, dword ptr fs:[0]           // prev SEH head
        push    eax
        sub     esp, 0x0c                       // 3-dword local area
        push    esi
        push    edi
        mov     eax, __security_cookie          // GS cookie (DIR32 reloc, a1 form)
        xor     eax, esp
        push    eax
        lea     eax, [esp + 0x18]               // &prev_fs0 slot in our frame
        mov     dword ptr fs:[0], eax           // install SEH registration

        // --- Body ---
        mov     esi, ecx                        // esi = this
        mov     dword ptr [esp + 0x0c], esi     // spill this into local[0]

        mov     eax, dword ptr [esp + 0x28]     // eax = arg1 (byte pointer)
        mov     cl, byte ptr [eax]              // cl = *arg1
        mov     byte ptr [esi], cl              // this[0] = *arg1

        lea     edi, [esi + 0x4]               // edi = &this[4]
        mov     ecx, edi                        // ecx = &this[4] (thiscall)
        call    FUN_004405e0                    // init sub-object at this[4]
        mov     dword ptr [edi + 0x4], eax     // this[8] = return value
        xor     eax, eax
        mov     dword ptr [edi + 0x8], eax     // this[12] = 0

        lea     edx, [esp + 0x10]              // edx = &local[1]
        mov     dword ptr [esp + 0x20], eax    // state = 0 (enter guarded block)
        mov     eax, dword ptr [edi + 0x4]     // eax = this[8]
        push    edx                             // stack arg2 = &local[1]
        push    0x9                             // stack arg1 = 9
        lea     ecx, [esi + 0x10]              // ecx = &this[16] (thiscall)
        mov     dword ptr [esp + 0x1c], eax    // local[2] = this[8]
        mov     dword ptr [esp + 0x18], edi    // local[1] = &this[4]
        call    FUN_00440e10                   // init sub-object at this[16]

        mov     eax, 0x1
        mov     dword ptr [esi + 0x20], eax    // this[32] = 1
        mov     dword ptr [esi + 0x24], eax    // this[36] = 1
        mov     eax, esi                        // return value = this

        // --- SEH teardown / epilogue ---
        mov     ecx, dword ptr [esp + 0x18]    // ecx = saved prev FS:[0]
        mov     dword ptr fs:[0], ecx           // restore SEH chain
        pop     ecx                             // discard GS cookie
        pop     edi
        pop     esi
        add     esp, 0x18
        ret     0x8
    }
}

// vim: ts=4 sts=4 sw=4 et
