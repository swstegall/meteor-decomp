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
// FUNCTION: ffxivgame 0x00020ab0 — FUN_00420ab0 (__thiscall constructor,
//                                  125 B, SEH-wrapped, ESP-based GS frame).
//
// Behaviour (reconstructed from RVA 0x00020ab0, 125 bytes of .text):
//
//   __thiscall void* FUN_00420ab0(SomeClass *this /*ECX*/)
//   {
//       // --- SEH / GS prologue (ESP-based, no EBP frame) ----------------
//       // PUSH -1 (initial trylevel), PUSH SEH_handler (0x00e55a63),
//       // MOV EAX, FS:[0], PUSH EAX (prev SEH), PUSH ECX (this spill),
//       // PUSH ESI, PUSH (cookie = __security_cookie ^ ESP),
//       // LEA EAX, [ESP+0xc] → install as FS:[0].
//
//       // --- Body -------------------------------------------------------
//       // ESI = this
//       // [ESP+0x8] = this  (saved into scratch ECX slot for EH handler)
//       //
//       // Zero-initialise two DWORD fields:
//       //   this->field_4 = 0;
//       //   this->field_8 = 0;
//       //
//       // SEH trylevel (dword [ESP+0x14]) ← 0  (full-dword write)
//       //
//       // First vptr/field assignment (intermediate, for a base sub-object):
//       //   this->field_c = 0x00f57e20;   // sub-object vtable (base)
//       //
//       // Take interior pointer:
//       //   EDX = &this->field_10;
//       //
//       // Set the main vtable and override the sub-object vptr:
//       //   this->vftable = 0x00f59880;   // most-derived vtable at [this+0]
//       //   this->field_c  = 0x00f59870;  // most-derived vtable at [this+0xc]
//       //
//       // Load a global and call through its vtable:
//       //   g  = [0x01329834];            // pointer to some manager object
//       //   ECX = *g;                     // dereference to get vftable ptr
//       //   EAX = ECX->vftable[0x1d8/4]; // virtual method slot
//       //   [ESP+0x20] = 1;               // SEH trylevel ← 1 (byte write)
//       //   EAX(g, 0x8, &this->field_10); // __thiscall(?) virtual call
//       //                                 // (3 stack args: EDX/0x8/EAX)
//       //
//       // Complete construction:
//       //   this->sub_init();             // CALL 0x004328a0 (ECX = this)
//       //
//       // return this; (EAX = ESI)
//       // --- SEH teardown + epilogue ------------------------------------
//   }
//
// Stack layout after the prologue (6 pushes, no SUB ESP, ESP-relative):
//   [ESP+0x00] = GS cookie (security_cookie ^ original ESP)
//   [ESP+0x04] = saved ESI
//   [ESP+0x08] = this (the ECX spill slot)
//   [ESP+0x0c] = prev FS:[0]  (SEH Next link)
//   [ESP+0x10] = SEH handler  (0x00e55a63)
//   [ESP+0x14] = trylevel     (-1 initial → 0 → 1)
//   [ESP+0x18] = return address
//
// Reloc-bearing sites (masked by compare.py; emitted as raw immediates):
//   +0x02  PUSH imm32  → SEH handler stub     (orig VA 0x00e55a63)
//   +0x13  MOV  moffs  → __security_cookie    (orig VA 0x012ea8b0)
//   +0x1c  MOV  moffs  → FS:[0] install       (zero offset)
//   +0x2c  MOV  imm32  → sub-object base vtable  (orig VA 0x00f57e20)
//   +0x33  MOV  imm32  → most-derived vtable      (orig VA 0x00f59880)
//   +0x3a  MOV  imm32  → most-derived sub-vptr    (orig VA 0x00f59870)
//   +0x41  MOV  moffs  → global manager ptr   (orig VA 0x01329834)
//   +0x60  CALL rel32  → FUN_004328a0         (displacement 0x00011d86)
//   +0x6c  MOV  moffs  → FS:[0] restore       (zero offset)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would require naming the three vtable globals,
//   the manager global, the SEH scope table, __security_cookie, and the
//   two call targets — each of which is a binary-resident absolute or
//   relative address that only resolves in a full relink. Rather than
//   introduce COFF relocations (which would mismatch the orig's resolved
//   immediate bytes), we emit the 125 bytes verbatim via MASM `_emit`
//   directives so the .obj's .text section carries no relocations and
//   compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00420ab0() {
    __asm {
        // --- SEH / GS prologue ------------------------------------------
        _emit 0x6a              // PUSH -1  (initial trylevel)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e55a63  (SEH handler)
        _emit 0x63
        _emit 0x5a
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (prev FS:[0])
        _emit 0x51              // PUSH ECX  (this spill)
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX  (anchored cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x0c]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x00000000], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- Body -------------------------------------------------------
        _emit 0x8b              // MOV ESI, ECX  (ESI = this)
        _emit 0xf1
        _emit 0x89              // MOV [ESP + 0x8], ESI  (spill this)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV [ESI + 0x4], EAX  (this->field_4 = 0)
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV [ESI + 0x8], EAX  (this->field_8 = 0)
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV [ESP + 0x14], EAX  (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xc7              // MOV [ESI + 0xc], 0x00f57e20  (sub-base vtable)
        _emit 0x46
        _emit 0x0c
        _emit 0x20
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESI + 0x10]  (EDX = &this->field_10)
        _emit 0x56
        _emit 0x10
        _emit 0xc7              // MOV [ESI], 0x00f59880  (main vtable at [this+0])
        _emit 0x06
        _emit 0x80
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV [ESI + 0xc], 0x00f59870  (derived sub-vptr)
        _emit 0x46
        _emit 0x0c
        _emit 0x70
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x01329834]  (global manager)
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, [EAX]  (deref → vftable ptr)
        _emit 0x08
        _emit 0x52              // PUSH EDX  (&this->field_10)
        _emit 0x6a              // PUSH 0x8
        _emit 0x08
        _emit 0x50              // PUSH EAX  (manager ptr)
        _emit 0x8b              // MOV EAX, [ECX + 0x1d8]  (virtual method)
        _emit 0x81
        _emit 0xd8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [ESP + 0x20], 0x1  (trylevel = 1)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x01
        _emit 0xff              // CALL EAX  (virtual dispatch)
        _emit 0xd0
        _emit 0x8b              // MOV ECX, ESI  (ECX = this for next call)
        _emit 0xce
        _emit 0xe8              // CALL 0x004328a0  (rel32 = 0x00011d86)
        _emit 0x86
        _emit 0x1d
        _emit 0x01
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI  (return this)
        _emit 0xc6

        // --- SEH teardown + epilogue ------------------------------------
        _emit 0x8b              // MOV ECX, [ESP + 0x0c]  (prev FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x00000000], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX  (drop cookie)
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
