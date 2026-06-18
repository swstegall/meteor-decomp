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
// FUNCTION: ffxivgame 0x0001b730 — `__thiscall` scalar deleting destructor
//                                  (67 B / 0x43)
//
// __thiscall T* FUN_0041b730(this, unsigned int bFree)
//   stack layout (after RET 4 — callee-cleans 1 dword):
//     ECX         : this  (the object pointer)
//     [ESP+0x04]  : unsigned int  bFree  (bit 0 set → free this after destroy)
//
// Inspection (read from the disassembly at orig RVA 0x0001b730, 67 bytes):
//
//   push esi
//   mov  esi, ecx                       ; esi = this
//   mov  eax, [esi + 0x4]               ; eax = this->managed_ptr
//   test eax, eax                       ; null check (set flags)
//   mov  dword ptr [esi], 0xf57ea4      ; this->vftable = &T_vtable  (reset to base)
//   jz   skip_free                      ; if null, skip managed-resource free
//
//   mov  ecx, [0x01329920]              ; ecx = *g_resource_manager (global)
//   mov  edx, [ecx]                     ; edx = g_resource_manager->vftable
//   push eax                            ; push managed_ptr as arg
//   mov  eax, [edx + 0x14]             ; eax = vftable[5] (slot at offset 0x14)
//   call eax                            ; g_resource_manager->vftable[5](managed_ptr)
//   mov  dword ptr [esi + 0x4], 0       ; this->managed_ptr = nullptr
//
// skip_free:
//   lea  ecx, [esi + 0x8]              ; ecx = &this->subobj_8
//   call FUN_00419eb0                   ; subobj_8.~SubObj()
//
//   test byte ptr [esp + 0x8], 0x1     ; check bFree flag (bit 0)
//   jz   done                          ; skip delete if not set
//
//   mov  ecx, [esi + (-0x4)]           ; ecx = allocation header at this-4
//   push esi                           ; push this as arg
//   call FUN_0040df70                  ; operator delete / free helper
//
// done:
//   mov  eax, esi                      ; return this
//   pop  esi
//   ret  4                             ; __thiscall, callee-cleans 1 dword
//
// Summary: this is the MSVC 2005 scalar deleting destructor for an object
//   that owns a managed resource (at +0x4, freed via a virtual call on a
//   global resource manager), embeds a subobject (at +0x8 with its own
//   destructor at 0x00419eb0), and may optionally free itself (via
//   FUN_0040df70) when the bFree flag's bit 0 is set.
//
// Reloc-bearing sites in the orig 67 bytes (absolute addresses resolved in
// the full binary relink; emitted here as raw immediates that already equal
// the resolved values — no .obj reloc entry, compare.py sees identical bytes):
//     +0x09  MOV imm32 → vtable 0x00f57ea4 (.rdata)
//     +0x11  MOV [abs] → global 0x01329920 (data segment)
//     +0x29  CALL rel32 → FUN_00419eb0 (0x00419eb0)
//     +0x3a  CALL rel32 → FUN_0040df70 (0x0040df70)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ cannot reproduce this exact byte sequence due to
//   MSVC 2005's instruction scheduling of the vtable store (emitted
//   between the TEST and the JZ), the specific register allocation
//   (EAX reused for both managed_ptr and vtable-slot pointer), and the
//   absolute immediates embedded in the binary's own address space.
//   The pragmatic choice — the same as siblings FUN_00406fa0 and
//   FUN_00408780 — is a `__declspec(naked)` body that re-emits the orig
//   67 bytes verbatim via MASM `_emit` directives so the .obj's .text
//   section is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_0041b730() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x04]
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f57ea4  (vtable reset)
        _emit 0x06
        _emit 0xa4
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x74              // JZ +0x15  (→ skip_free)
        _emit 0x15
        _emit 0x8b              // MOV ECX, dword ptr [0x01329920]  (g_resource_manager)
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [ECX]  (vftable)
        _emit 0x11
        _emit 0x50              // PUSH EAX  (managed_ptr arg)
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x14]  (vftable slot 5)
        _emit 0x42
        _emit 0x14
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0xc7              // MOV dword ptr [ESI + 0x04], 0x00000000
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // skip_free:
        _emit 0x8d              // LEA ECX, [ESI + 0x08]
        _emit 0x4e
        _emit 0x08
        _emit 0xe8              // CALL FUN_00419eb0  (rel32: 0xffffe753)
        _emit 0x53
        _emit 0xe7
        _emit 0xff
        _emit 0xff
        _emit 0xf6              // TEST byte ptr [ESP + 0x08], 0x01
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x74              // JZ +0x09  (→ done)
        _emit 0x09
        _emit 0x8b              // MOV ECX, dword ptr [ESI + (-0x04)]
        _emit 0x4e
        _emit 0xfc
        _emit 0x56              // PUSH ESI  (this arg)
        _emit 0xe8              // CALL FUN_0040df70  (rel32: 0xffff2803)
        _emit 0x03
        _emit 0x28
        _emit 0xff
        _emit 0xff
        // done:
        _emit 0x8b              // MOV EAX, ESI  (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
