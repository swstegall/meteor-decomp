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
// FUNCTION: ffxivgame 0x00440fb0 — __thiscall object initialiser: copies the
//   first byte of a pointer argument into this[0], initialises two
//   sub-objects via thiscall methods (FUN_004405e0 on this+4 and
//   FUN_00440e10 on this+0x10), stores the return value of the first call
//   into sub-object fields, then sets this->field0x20 and this->field0x24
//   to 1 before returning `this`.  Two explicit stack parameters are
//   declared (RET 8), though only param0 (a pointer) is used in the body;
//   param1 appears to be an unused/legacy parameter kept for ABI compat.
//
// Stack layout of the MSVC 2005 SEH / /GS frame (relative to body ESP
// after the full prologue, i.e. after the final PUSH EAX / cookie):
//
//   [ESP + 0x00] = security cookie (XOR'd with ESP at prologue)
//   [ESP + 0x04] = saved EDI
//   [ESP + 0x08] = saved ESI / `this` spill (same slot — ESI is `this`)
//   [ESP + 0x0c] = local[0]: `this` spill written by body for SEH handler
//   [ESP + 0x10] = local[1]: later overwritten with EDI (this+4) by body
//   [ESP + 0x14] = local[2]: later overwritten with (this+4)->off4 by body
//   [ESP + 0x18] = saved prev FS:[0] (SEH chain link)
//   [ESP + 0x1c] = SEH handler VA (0x00e56fcb, DIR32 reloc)
//   [ESP + 0x20] = exception state (−1 at entry; set to 0 before 2nd call)
//   [ESP + 0x24] = caller's return address
//   [ESP + 0x28] = param0  (pointer; first byte copied to this[0])
//   [ESP + 0x2c] = param1  (unused in body)
//
// Relocation-bearing positions in the 132-byte slice (all masked by
// compare.py for the diff):
//
//   +0x03  IMAGE_REL_I386_DIR32  → FUN_00e56fcb  (SEH handler stub)
//   +0x14  IMAGE_REL_I386_DIR32  → __security_cookie (VA 0x012ea8b0)
//   +0x39  IMAGE_REL_I386_REL32  → FUN_004405e0  (sub-object init #1)
//   +0x5f  IMAGE_REL_I386_REL32  → FUN_00440e10  (sub-object init #2)

extern "C" {

// SEH handler trampoline at VA 0x00e56fcb. The `push offset FUN_00e56fcb`
// emits a DIR32 reloc that compare.py masks; only the `68` opcode byte
// matters for the byte diff.
int FUN_00e56fcb();

// Sub-object init #1: thiscall, ECX = this+4, no explicit stack args.
// Returns a value stored into (this+4)->off4 and reloaded before #2.
int FUN_004405e0();

// Sub-object init #2: thiscall, ECX = this+0x10, two explicit stack
// args (9 and &local[1]).  Callee-cleans via RET 8.
int FUN_00440e10();

extern unsigned __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_00440fb0() {
    __asm {
        // --- SEH / GS prologue (ESP-based frame) ---------------------------
        push    -1                             ; exception state = -1
        push    offset FUN_00e56fcb            ; SEH handler (DIR32 reloc)
        mov     eax, dword ptr fs:[0]          ; save old FS:[0]
        push    eax
        sub     esp, 0xc                       ; 3 dwords of local space
        push    esi                            ; preserve ESI
        push    edi                            ; preserve EDI
        mov     eax, __security_cookie         ; load GS cookie (a1 form, DIR32)
        xor     eax, esp                       ; anchor to current ESP
        push    eax                            ; spill cookie
        lea     eax, [esp + 0x18]             ; → &saved_prev_fs0
        mov     dword ptr fs:[0], eax          ; install SEH frame

        // --- Body ----------------------------------------------------------
        mov     esi, ecx                       ; ESI = this
        mov     dword ptr [esp + 0xc], esi     ; spill `this` to local[0]

        mov     eax, dword ptr [esp + 0x28]    ; EAX = param0 (pointer)
        mov     cl, byte ptr [eax]             ; CL  = *(param0)
        mov     byte ptr [esi], cl             ; this[0] = *(param0)

        lea     edi, [esi + 0x4]               ; EDI = this+4
        mov     ecx, edi
        call    FUN_004405e0                   ; init sub-object at this+4
        mov     dword ptr [edi + 0x4], eax     ; (this+4)->off4 = retval

        xor     eax, eax
        mov     dword ptr [edi + 0x8], eax     ; (this+4)->off8 = 0

        lea     edx, [esp + 0x10]              ; EDX = &local[1]
        mov     dword ptr [esp + 0x20], eax    ; exception state = 0

        mov     eax, dword ptr [edi + 0x4]     ; reload (this+4)->off4
        push    edx                            ; arg: &local[1]
        push    9                              ; arg: 9
        lea     ecx, [esi + 0x10]              ; ECX = this+0x10
        mov     dword ptr [esp + 0x1c], eax    ; local[2] = (this+4)->off4
        mov     dword ptr [esp + 0x18], edi    ; local[1] = this+4
        call    FUN_00440e10                   ; init sub-object at this+0x10

        mov     eax, 1
        mov     dword ptr [esi + 0x20], eax    ; this->field0x20 = 1
        mov     dword ptr [esi + 0x24], eax    ; this->field0x24 = 1
        mov     eax, esi                       ; return value = this

        // --- SEH teardown / epilogue ---------------------------------------
        mov     ecx, dword ptr [esp + 0x18]    ; ECX = saved prev FS:[0]
        mov     dword ptr fs:[0], ecx          ; restore SEH chain
        pop     ecx                            ; discard cookie
        pop     edi                            ; restore EDI
        pop     esi                            ; restore ESI
        add     esp, 0x18                      ; drop locals + SEH frame
        ret     8                              ; callee-clean 2 params
    }
}

// vim: ts=4 sts=4 sw=4 et
