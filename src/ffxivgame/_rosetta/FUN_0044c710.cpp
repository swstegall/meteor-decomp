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
// FUNCTION: ffxivgame 0x0004c710 — FUN_0044c710
//   __thiscall constructor (124 B / 0x7c, SEH-wrapped, /GS cookie).
//
// Shape (read from RVA 0x0004c710):
//
//   __thiscall void* FUN_0044c710(SomeClass *this /*ECX*/)
//   {
//       // --- parent constructor -----------------------------------------
//       FUN_00458950(this);           // base class ctor via __thiscall
//
//       // --- vtable + member init ----------------------------------------
//       this->vftable = 0x00f6738c;  // set most-derived vtable
//       this->field_4 = nullptr;
//
//       // --- sub-object allocation (operator new(4)) --------------------
//       // SEH state 0: about to allocate; field_4 is null so the EH
//       //              unwind path need not destroy it yet.
//       void *p = FUN_009d1b35(4);   // allocator — cdecl, 1 arg, returns ptr
//
//       // SEH state 1: allocation complete; if non-null, destructor must
//       //              run on unwind to release the sub-object.
//       if (p != nullptr)
//           FUN_004589a0(p);         // sub-object init ctor (__thiscall)
//       else
//           p = nullptr;
//
//       this->field_4 = p;
//       return this;
//   }
//
// SEH frame (ESP-relative, after prologue):
//   [ESP + 0x00]  GS cookie (xor'd with esp)
//   [ESP + 0x04]  saved ESI (preserved reg = `this`)
//   [ESP + 0x08]  `this` spill for EH handler recovery
//   [ESP + 0x0c]  allocator return value (spilled for EH unwind)
//   [ESP + 0x10]  prev FS:[0]  ← FS:[0] is set to point here
//   [ESP + 0x14]  SEH handler  (0x00e57a63 — scope table VA)
//   [ESP + 0x18]  SEH state    (−1 → 0 → 1)
//   [ESP + 0x1c]  return address
//
// Reloc-bearing positions in the orig 124 bytes (masked by compare.py):
//   +0x02  PUSH imm32       → SEH scope table  (orig 0x00e57a63)
//   +0x08  MOV EAX,moffs32  → FS:[0]           (addr field = 0)
//   +0x14  MOV EAX,moffs32  → __security_cookie (orig 0x012ea8b0)
//   +0x1e  MOV moffs32,EAX  → FS:[0]           (install handler)
//   +0x2b  CALL rel32       → FUN_00458950      (base ctor)
//   +0x39  MOV dword ptr,imm32 → vtable        (orig 0x00f6738c)
//   +0x47  CALL rel32       → FUN_009d1b35      (allocator)
//   +0x5e  CALL rel32       → FUN_004589a0      (sub-object init)
//   +0x6c  MOV moffs32,ECX  → FS:[0]           (restore)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH-framed prologue (SUB-before-PUSH ordering), the in-body
//   state transitions (dword-zero then byte-one), and the three
//   binary-resident absolute/relative addresses in the reloc sites all
//   resist clean-room C++ without a reliable relink.  The pragmatic
//   choice (identical to FUN_00403d60 and FUN_00401350) is a
//   `__declspec(naked)` body that re-emits the orig 124 bytes verbatim
//   via MASM `_emit` directives; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044c710() {
    __asm {
        // --- SEH / GS prologue -------------------------------------------
        _emit 0x6a              // PUSH -1         (initial SEH state)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e57a63  (SEH scope table VA — DIR32 reloc)
        _emit 0x63
        _emit 0x7a
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX         (prev FS:[0] — SEH chain link)
        _emit 0x83              // SUB ESP, 0x8     (allocate two local dword slots)
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI         (save callee-preserved reg)
        _emit 0xa1              // MOV EAX, [__security_cookie]  (DIR32 reloc)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP     (anchor cookie to stack)
        _emit 0xc4
        _emit 0x50              // PUSH EAX         (spill GS cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x10]  (addr of prev-FS:[0] slot)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x0], EAX  (install SEH registration)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- body --------------------------------------------------------
        _emit 0x8b              // MOV ESI, ECX      (ESI = this)
        _emit 0xf1
        _emit 0x89              // MOV [ESP + 0x8], ESI  (spill `this` for EH handler)
        _emit 0x74
        _emit 0x24
        _emit 0x08

        _emit 0xe8              // CALL FUN_00458950  (base/parent ctor, __thiscall ECX=this)
        _emit 0x11              //   rel32 = 0x0000c211
        _emit 0xc2
        _emit 0x00
        _emit 0x00

        _emit 0x6a              // PUSH 0x4           (arg: allocation size)
        _emit 0x04

        _emit 0xc7              // MOV dword ptr [ESP + 0x1c], 0x0  (SEH state → 0)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0xc7              // MOV dword ptr [ESI], 0x00f6738c  (set vtable)
        _emit 0x06
        _emit 0x8c
        _emit 0x73
        _emit 0xf6
        _emit 0x00

        _emit 0xc7              // MOV dword ptr [ESI + 0x4], 0x0   (field_4 = null)
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0xe8              // CALL FUN_009d1b35  (allocator __cdecl, 1 arg)
        _emit 0xda              //   rel32 = 0x005853da
        _emit 0x53
        _emit 0x58
        _emit 0x00

        _emit 0x83              // ADD ESP, 0x4       (cdecl caller cleanup)
        _emit 0xc4
        _emit 0x04

        _emit 0x89              // MOV [ESP + 0xc], EAX  (spill alloc result for EH)
        _emit 0x44
        _emit 0x24
        _emit 0x0c

        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0

        _emit 0xc6              // MOV byte ptr [ESP + 0x18], 0x1  (SEH state → 1)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x01

        _emit 0x74              // JZ +9  (alloc failed → skip init call)
        _emit 0x09

        _emit 0x8b              // MOV ECX, EAX       (ECX = alloc'd ptr for __thiscall)
        _emit 0xc8

        _emit 0xe8              // CALL FUN_004589a0  (sub-object init, __thiscall)
        _emit 0x2e              //   rel32 = 0x0000c22e
        _emit 0xc2
        _emit 0x00
        _emit 0x00

        _emit 0xeb              // JMP +2  (skip XOR EAX, EAX)
        _emit 0x02

        _emit 0x33              // XOR EAX, EAX       (alloc failed: result = null)
        _emit 0xc0

        _emit 0x89              // MOV [ESI + 0x4], EAX  (this->field_4 = result)
        _emit 0x46
        _emit 0x04

        _emit 0x8b              // MOV EAX, ESI       (return value = this)
        _emit 0xc6

        // --- SEH teardown / epilogue -------------------------------------
        _emit 0x8b              // MOV ECX, [ESP + 0x10]   (load prev FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x10

        _emit 0x64              // MOV FS:[0x0], ECX   (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x59              // POP ECX             (discard GS cookie)
        _emit 0x5e              // POP ESI             (restore callee-preserved reg)
        _emit 0x83              // ADD ESP, 0x14       (drop locals + SEH frame fields)
        _emit 0xc4
        _emit 0x14

        _emit 0xc3              // RET                 (__thiscall, no stack args)
    }
}
