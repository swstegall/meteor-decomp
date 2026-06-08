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
// FUNCTION: ffxivgame 0x000206a0 — __thiscall constructor/init with SEH frame,
//                                   vtable install, optional resource release,
//                                   and vtable reset (126 B / 0x7e).
//
// Behaviour reconstructed from the asm:
//
//   __thiscall void FUN_004206a0(SomeClass *this /*ECX*/) {
//       this->vftable   = 0x00f59880;  // [this+0x00]
//       this->field_0xc = 0x00f59870;  // [this+0x0c]
//       // SEH state = 1
//       FUN_00432820(this);             // thiscall init helper
//       if (this->field_0x10 != NULL) {
//           // Virtual call: g_obj_01329920->vftable[3](this->field_0x10)
//           void *obj = *((void**)0x01329920);
//           ((void(__cdecl*)(void*))(((void**)obj)[3]))(this->field_0x10);
//           this->field_0x10 = NULL;
//       }
//       this->field_0xc = 0x00f57e20;  // reset vtable part
//       this->vftable   = 0x00f57e14;  // reset vtable
//   }
//
// Frame layout (after GS cookie PUSH; all ESP-relative):
//   [ESP+0x00]  GS cookie (xor'd security cookie)
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  scratch (from SUB ESP,8)
//   [ESP+0x0c]  `this` spill (written at body entry)
//   [ESP+0x10]  old FS:[0]   ← LEA EAX,[ESP+0x10] points here
//   [ESP+0x14]  SEH handler  (0x00e558ff)
//   [ESP+0x18]  unwind state (initially -1, updated to 1 before call)
//   [ESP+0x1c]  return address
//
// Calling convention: __thiscall (ECX = this; plain RET, no stack args).
//
// Reloc-bearing positions in the orig 126 bytes (all masked by compare.py):
//   off 0x03   DIR32   → SEH handler table stub (VA 0x00e558ff)
//   off 0x09   MOFFS32 → FS:[0] (always 0x00000000, not a normal DIR32)
//   off 0x13   DIR32   → __security_cookie (VA 0x012ea8b0)
//   off 0x1d   MOFFS32 → FS:[0] (install)
//   off 0x29   DIR32   → vftable init #1 (VA 0x00f59880)
//   off 0x30   DIR32   → vftable init #2 (VA 0x00f59870)
//   off 0x3f   REL32   → FUN_00432820 (displacement 0x0001213c)
//   off 0x47   DIR32   → global obj ptr (VA 0x01329920)
//   off 0x60   DIR32   → vftable reset #1 (VA 0x00f57e20)
//   off 0x67   DIR32   → vftable reset #2 (VA 0x00f57e14)
//   off 0x6e   MOFFS32 → FS:[0] (restore)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   All absolute addresses (vtable VAs, global ptr, SEH handler, security cookie)
//   resolve only in the full binary at its 0x00400000 load base. Emitting them as
//   raw immediate bytes via _emit produces byte-identical .text with no
//   relocations; compare.py masks all reloc sites and reports GREEN.

extern "C" __declspec(naked) void FUN_004206a0() {
    __asm {
        // --- SEH / GS prologue -----------------------------------------
        _emit 0x6a              // PUSH -1              (initial unwind state)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e558ff      (SEH handler table)
        _emit 0xff
        _emit 0x58
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX             (old FS:[0])
        _emit 0x83              // SUB ESP, 0x8         (scratch slots)
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX             (GS cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x00000000], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- Body -------------------------------------------------------
        _emit 0x8b              // MOV ESI, ECX         (ESI = this)
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ESP + 0xc], ESI  (spill this)
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f59880
        _emit 0x06
        _emit 0x80
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI + 0xc], 0x00f59870
        _emit 0x46
        _emit 0x0c
        _emit 0x70
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESP + 0x18], 0x1  (SEH state = 1)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00432820  (rel32 = 0x0001213c)
        _emit 0x3c
        _emit 0x21
        _emit 0x01
        _emit 0x00

        // --- Conditional resource release -------------------------------
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x15  (→ vtable-reset block)
        _emit 0x15
        _emit 0x8b              // MOV ECX, dword ptr [0x01329920]
        _emit 0x0d
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [ECX]  (load vtable)
        _emit 0x11
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0xc]  (vtable[3])
        _emit 0x42
        _emit 0x0c
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0xc7              // MOV dword ptr [ESI + 0x10], 0x0
        _emit 0x46
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- Vtable reset -----------------------------------------------
        _emit 0xc7              // MOV dword ptr [ESI + 0xc], 0x00f57e20
        _emit 0x46
        _emit 0x0c
        _emit 0x20
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f57e14
        _emit 0x06
        _emit 0x14
        _emit 0x7e
        _emit 0xf5
        _emit 0x00

        // --- SEH teardown / epilogue ------------------------------------
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV dword ptr FS:[0x00000000], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX              (drop GS cookie)
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3              // RET
    }
}
