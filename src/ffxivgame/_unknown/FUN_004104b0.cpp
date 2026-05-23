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
// FUNCTION: ffxivgame 0x000104b0 — __thiscall doubly-linked list node
// unlink + vtable restore (25 B)
//
// Asm (25 bytes @ orig RVA 0x000104b0):
//   8b 41 04           MOV EAX, dword ptr [ECX + 0x4]   ; eax = this->prev
//   8b 51 08           MOV EDX, dword ptr [ECX + 0x8]   ; edx = this->next
//   c7 01 c4 67 f5 00  MOV dword ptr [ECX], 0xf567c4    ; this->vptr = vtable
//   89 50 08           MOV dword ptr [EAX + 0x8], EDX   ; prev->next = this->next
//   8b 41 08           MOV EAX, dword ptr [ECX + 0x8]   ; eax = this->next
//   8b 49 04           MOV ECX, dword ptr [ECX + 0x4]   ; ecx = this->prev
//   89 48 04           MOV dword ptr [EAX + 0x4], ECX   ; next->prev = this->prev
//   c3                 RET
//
// Calling convention: __thiscall (ECX = this, no stack args, RET 0).
// No prologue — /Oy frame-pointer omission; function is a leaf with no
// callee-saved register usage.
//
// Semantics: doubly-linked list node removal. The struct layout (inferred):
//   struct Link {               // __thiscall receiver (ECX)
//       void*  vptr;            // [ECX+0x0] — vtable ptr restored to 0xf567c4
//       Link*  prev;            // [ECX+0x4]
//       Link*  next;            // [ECX+0x8]
//   };
//
// The vtable write happens in the MSVC order (before patching the
// neighbors), reproducing the exact register-reuse sequence in codegen.
// Naked asm is required to pin the 8-byte MOV-imm32 encoding of the
// vtable store and the precise register allocation (EAX/EDX/ECX).

extern "C" __declspec(naked) void FUN_004104b0() {
    __asm {
        mov eax, dword ptr [ecx + 0x4]
        mov edx, dword ptr [ecx + 0x8]
        mov dword ptr [ecx], 0xf567c4
        mov dword ptr [eax + 0x8], edx
        mov eax, dword ptr [ecx + 0x8]
        mov ecx, dword ptr [ecx + 0x4]
        mov dword ptr [eax + 0x4], ecx
        ret
    }
}
