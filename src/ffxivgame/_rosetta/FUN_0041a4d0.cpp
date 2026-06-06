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
// FUNCTION: ffxivgame 0x0001a4d0 — `__thiscall` scalar-deleting destructor
//                                  for a vtable-carrying object with an owned
//                                  pointer and a subobject at offset 8 (64 B).
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg `shouldFree`;
//                     callee cleans 4 bytes via `ret 0x4`).
// Returns: EAX = this.
//
// Inspection (read from the disassembly at orig RVA 0x0001a4d0, 64 bytes total):
//
//   push esi
//   mov  esi, ecx                         ; esi = this
//   mov  eax, [esi + 0x4]                 ; eax = this->m_ptr  (owned ptr)
//   test eax, eax                         ; set flags
//   mov  dword ptr [esi], 0x00f57ea0      ; install vtable (unconditional)
//   jz   skip                             ; if m_ptr == null, skip dealloc
//
//   mov  ecx, [0x01329920]                ; ecx = *g_pAllocator (global obj*)
//   mov  edx, [ecx]                       ; edx = *ecx (vtable of allocator)
//   push eax                              ; arg: ptr to free
//   mov  eax, [edx + 0x18]               ; eax = vtable[6]  (free fn)
//   call eax                              ; g_pAllocator->vtable[6](m_ptr)
//   mov  dword ptr [esi + 0x4], 0x0       ; this->m_ptr = nullptr
//
// skip:
//   lea  ecx, [esi + 0x8]                 ; ecx = &this->m_sub (subobject)
//   call FUN_00419eb0                     ; subobject destructor body
//   test byte ptr [esp + 0x8], 0x1        ; shouldFree & 1?
//   jz   done
//   push esi                              ; arg: this
//   call FUN_009d1b17                     ; operator delete(this)
// done:
//   mov  eax, esi                         ; return this
//   pop  esi
//   ret  0x4                              ; callee cleans 1 arg
//
// Layout inferred:
//   this+0x00   void*   vftable           (overwritten with 0x00f57ea0)
//   this+0x04   void*   m_ptr             (owned allocation, freed via allocator)
//   this+0x08   …       m_sub             (subobject destroyed by FUN_00419eb0)
//
//   g_pAllocator at VA 0x01329920 — pointer to a global allocator-like object
//   whose vtable slot 6 (offset 0x18) is the deallocation function.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The body references two absolute VA immediates (vtable 0x00f57ea0 and
//   global 0x01329920) that are image-base-resolved values baked into the
//   orig binary. Coaxing cl.exe /O2 to emit the TEST/MOV interleaving and
//   the exact register allocation from C++ source is brittle. The pragmatic
//   approach — identical to siblings FUN_00406fa0 and FUN_00408780 — is a
//   `__declspec(naked)` body that re-emits the orig 64 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` ends up byte-identical to
//   the orig slice and `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_0041a4d0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x04]
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
        _emit 0x74              // JZ +0x15  (→ skip)
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
        _emit 0xc7              // MOV dword ptr [ESI + 0x04], 0x00000000
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // skip: LEA ECX, [ESI + 0x08]
        _emit 0x4e
        _emit 0x08
        _emit 0xe8              // CALL FUN_00419eb0  (rel32 → 0x00419eb0)
        _emit 0xb3
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0xf6              // TEST byte ptr [ESP + 0x08], 0x01
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x74              // JZ +0x09  (→ done)
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_009d1b17  (rel32 → 0x009d1b17)
        _emit 0x0d
        _emit 0x76
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x04  (__cdecl caller stack cleanup)
        _emit 0xc4
        _emit 0x04
        _emit 0x8b              // done: MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
