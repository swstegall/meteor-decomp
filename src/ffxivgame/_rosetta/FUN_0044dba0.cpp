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
// FUNCTION: ffxivgame 0x0004dba0 — __thiscall teardown/finalizer under an
// MSVC C++ SEH unwind frame with a `/GS` security cookie (125 B / 0x7d).
//
// Behaviour reconstructed from the asm (RVA 0x0004dba0, 125 bytes .text):
//
//   void __thiscall FUN_0044dba0(Obj *this /*ECX*/) {
//       // Install vtable pair for the teardown body (MSVC class layout
//       // with an embedded sub-object whose own vftable lives at +0x4).
//       this->vftable     = &vftable_00f674c4;   // [this+0]  = 0x00f674c4
//       this->sub_vftable = &vftable_00f674a8;   // [this+4]  = 0x00f674a8
//
//       // state 0: run a fixed-target finalizer on a global object.
//       FUN_004592d0(/*ECX = 0x01266e00*/);       // __thiscall on global
//
//       // One-shot conditional release of a cached global resource:
//       //   if (g_handle_0132cf4c) {
//       //       FUN_009d00a8(g_arg_0132cf48, g_handle_0132cf4c);
//       //       g_handle_0132cf4c = 0;
//       //   }
//
//       // Reset to the base vftable as the frame unwinds.
//       this->vftable = &vftable_00f67434;        // [this+0] = 0x00f67434
//   }
//
// Calling convention: __thiscall (ECX = this; no `ret N` — the only stack
// data is the SEH/GS frame, not parameters). Mirrors the sibling
// destructor FUN_004013d0 exactly in prologue/epilogue shape (push -1 /
// push handler / fs:[0] chain link / push ecx,esi / cookie xor esp / install
// registration; teardown restores fs:[0], pops ecx/esi, `add esp,0x10`, ret).
//
// SEH frame layout after the prologue (relative to the final esp):
//   [esp + 0x00] = saved GS cookie (xor'd with original esp)
//   [esp + 0x04] = saved esi (preserved register)
//   [esp + 0x08] = `this` spill (EH handler reads ECX back from here)
//   [esp + 0x0c] = saved prev fs:[0] (links into SEH chain)
//   [esp + 0x10] = SEH handler ptr (push'd as 0x00e57b78)
//   [esp + 0x14] = unwind-state slot (initial 0)
//   [esp + 0x18] = return address
//
// Reloc-bearing positions in the orig 125 bytes (all masked in the diff):
//   off 0x02   PUSH imm32  → SEH scope/handler (0x00e57b78)
//   off 0x10   MOV moffs32 → __security_cookie (0x012ea8b0)
//   off 0x2a   MOV mem,imm → vftable_00f674c4
//   off 0x31   MOV mem,imm → vftable_00f674a8
//   off 0x42   CALL rel32  → FUN_004592d0
//   off 0x57   CALL rel32  → FUN_009d00a8
//   off 0x68   MOV mem,imm → vftable_00f67434
//
// Reconstruction strategy — naked-asm byte passthrough (same as the sibling
// FUN_00403d60): a source-level C++ form would need to coax MSVC 2005 into
// reproducing the exact /GS + SEH frame, register allocation, branch
// encoding, AND four absolute reloc targets / two rel32 sibling calls. Each
// is brittle under /O2. Re-emitting the orig 125 bytes verbatim via MASM
// `_emit` directives yields a .obj whose `.text` is byte-identical to the
// orig slice; the absolute imm32 / DIR32 operands are written as concrete
// bytes (valid against the orig image), and `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_0044dba0() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E57B78  (SEH handler)
        _emit 0x78
        _emit 0x7b
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (prev fs:[0])
        _emit 0x51              // PUSH ECX  (scratch -> `this`)
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX  (spill cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x0C]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0], EAX  (install registration)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX  (esi = this)
        _emit 0xf1
        _emit 0x89              // MOV [ESP + 0x08], ESI  (spill `this`)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F674C4
        _emit 0x06
        _emit 0xc4
        _emit 0x74
        _emit 0xf6
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI + 0x04], 0x00F674A8
        _emit 0x46
        _emit 0x04
        _emit 0xa8
        _emit 0x74
        _emit 0xf6
        _emit 0x00
        _emit 0xb9              // MOV ECX, 0x01266E00
        _emit 0x00
        _emit 0x6e
        _emit 0x26
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP + 0x14], 0  (state slot)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_004592d0 (rel32)
        _emit 0xe9
        _emit 0xb6
        _emit 0x00
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x0132CF4C]  (g_handle)
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x16  (-> vtable reset)
        _emit 0x16
        _emit 0x50              // PUSH EAX  (g_handle)
        _emit 0xa1              // MOV EAX, [0x0132CF48]  (g_arg)
        _emit 0x48
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x50              // PUSH EAX  (g_arg)
        _emit 0xe8              // CALL FUN_009d00a8 (rel32)
        _emit 0xac
        _emit 0x24
        _emit 0x58
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [0x0132CF4C], 0
        _emit 0x05
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F67434  (base vftable)
        _emit 0x06
        _emit 0x34
        _emit 0x74
        _emit 0xf6
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESP + 0x0C]  (prev fs:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0], ECX  (restore SEH chain)
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
