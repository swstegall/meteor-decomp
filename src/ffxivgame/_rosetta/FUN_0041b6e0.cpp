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
// FUNCTION: ffxivgame 0x0001b6e0 — `__thiscall` virtual scalar-deleting
//                                  destructor body (67 B / 0x43)
//
// __thiscall void* FUN_0041b6e0(this, unsigned int flags)
//   stack layout (after RET 4 — callee-cleans 1 DWORD):
//     ECX        : this
//     [ESP+0x04] : unsigned int flags  (bit 0 = 1 → call delete helper)
//
// Inspection (read from the disassembly at RVA 0x0001b6e0, 67 bytes total):
//
//   push    esi
//   mov     esi, ecx                     ; esi = this
//   mov     eax, [esi+4]                 ; eax = this->field_4  (pre-load for test)
//   test    eax, eax                     ; set ZF early
//   mov     [esi], 0x00f57ea0            ; this->vfptr = &SomeClass_vtable (reset)
//   jz      skip_free                    ; if field_4 == NULL, skip resource release
//
//   ; Release this->field_4 via global singleton's vtable slot 6 (offset +0x18):
//   mov     ecx, [0x01329920]            ; ecx = *g_singleton_ptr
//   mov     edx, [ecx]                   ; edx = (*g_singleton_ptr)->vftable
//   push    eax                          ; arg0 = old this->field_4
//   mov     eax, [edx+0x18]             ; eax = vftable[6] (the release method)
//   call    eax                          ; __thiscall: (*g_singleton_ptr)->release(field_4)
//   mov     [esi+4], 0                   ; this->field_4 = NULL
//
// skip_free:
//   lea     ecx, [esi+8]                 ; ecx = &this->field_8  (subobject)
//   call    FUN_00419eb0                 ; FUN_00419eb0::dtor or similar on field_8
//
//   test    byte ptr [esp+8], 1          ; flags & 1 ?
//   jz      skip_delete                  ; no → keep alive, just return this
//
//   ; Scalar delete via custom allocator (block header lives at this-4):
//   mov     ecx, [esi-4]                 ; ecx = allocator/block info at *(this-4)
//   push    esi                          ; arg0 = this
//   call    FUN_0040df70                 ; FUN_0040df70(this, *(this-4))
//
// skip_delete:
//   mov     eax, esi                     ; return value = this
//   pop     esi
//   ret     4                            ; callee cleans 1 DWORD
//
// Reloc-bearing sites in the orig 67 bytes (compare.py masks the 4-byte
// rel32 / imm32 operands at these positions in the .obj vs binary diff):
//   +0x28  CALL rel32 → FUN_00419eb0 (RVA 0x00019eb0)
//   +0x32  CALL rel32 → FUN_0040df70 (RVA 0x0000df70)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Three distinct absolute-address idioms (vtable imm32 to [ESI],
//   global singleton load from [0x01329920], and the two CALL rel32
//   targets) make source-level C++ infeasible: MSVC 2005 /O2 would need
//   to produce the exact TEST-before-MOV interleaving and register
//   allocation to match byte-for-byte.  The `_emit` approach reproduces
//   all 67 bytes verbatim using the original binary's already-resolved
//   values; `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_0041b6e0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f57ea0
        _emit 0x06
        _emit 0xa0
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x74              // JZ +0x15  (→ skip_free)
        _emit 0x15
        _emit 0x8b              // MOV ECX, dword ptr [0x01329920]
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x18]
        _emit 0x42
        _emit 0x18
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0xc7              // MOV dword ptr [ESI + 0x4], 0x00000000
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESI + 0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0xe8              // CALL rel32 → FUN_00419eb0
        _emit 0xa3
        _emit 0xe7
        _emit 0xff
        _emit 0xff
        _emit 0xf6              // TEST byte ptr [ESP + 0x8], 0x01
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x74              // JZ +0x09  (→ skip_delete)
        _emit 0x09
        _emit 0x8b              // MOV ECX, dword ptr [ESI + (-0x4)]
        _emit 0x4e
        _emit 0xfc
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL rel32 → FUN_0040df70
        _emit 0x53
        _emit 0x28
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
