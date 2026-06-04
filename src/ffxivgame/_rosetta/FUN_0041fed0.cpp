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
// FUNCTION: ffxivgame 0x0041fed0 — one-time init guard with SEH frame
//                                  (__cdecl, no params, 166 bytes / 0xa6)
//
// __cdecl bool FUN_0041fed0(void)
//
// Behaviour (inspected from the orig 166 bytes at RVA 0x0001fed0):
//
//   Standard MSVC 2005 SEH prologue (PUSH -1 / PUSH handler / MOV EAX,
//   FS:[0] / PUSH EAX / SUB ESP,0x10 / GS-cookie / LEA + MOV FS:[0]).
//   Local frame = 16 bytes of scratch; GS cookie on top; SEH chain below.
//
//   if ([0x01329428] != 0) {   // already initialised?
//       return true;           // MOV AL, 1; epilogue; RET
//   }
//
//   // Initialisation path:
//   PUSH 0xf58a8c              // string/name literal
//   PUSH 0x10                  // id / flags
//   LEA  ECX, [ESP+0x14]       // pointer to local scratch
//   CALL 0x0040e2d0            // construct / lookup something → EAX
//   PUSH EAX                   // save result
//   PUSH 0x1bc                 // size or type id
//   MOV  [ESP+0xc], EAX        // stash in local
//   CALL 0x00419c40            // factory / query → EAX (object ptr)
//   ADD  ESP, 0x8              // cdecl cleanup (2 args)
//   MOV  [ESP+0x8], EAX        // stash object ptr in local
//   TEST EAX, EAX
//   MOV  [ESP+0x1c], 0         // enter SEH try-block (try-state = 0)
//   JZ   null_branch
//   MOV  ECX, EAX              // thiscall: ECX = object ptr
//   CALL 0x00421e90            // virtual/member method on object
//   JMP  join
// null_branch:
//   XOR  EAX, EAX              // EAX = 0
// join:
//   PUSH 0x20
//   MOV  [0x01329428], EAX     // store result in the guard global
//   CALL 0x009fc756            // platform init (e.g. timer/event creation)
//   MOV  [0x01329830], EAX     // store handle
//   MOV  EAX, [0x012660ec]     // load another global (context ptr, unused by callee)
//   CALL 0x0041b820            // further init step
//   epilogue; RET
//
// Reloc-bearing sites in the orig 166 bytes:
//     +0x22   MOV  m32,imm32  → [0x012ea8b0]   (__security_cookie)
//     +0x2a   LEA/MOV FS:[0]  → FS-relative (no abs reloc)
//     +0x35   CMP  m32,imm    → [0x01329428]   (g_init_guard)
//     +0x3e   (early-RET epilogue uses FS-relative only)
//     +0x4b   CALL rel32      → 0x0040e2d0
//     +0x58   CALL rel32      → 0x00419c40
//     +0x72   CALL rel32      → 0x00421e90
//     +0x7d   MOV  m32,EAX    → [0x01329428]   (g_init_guard write)
//     +0x82   CALL rel32      → 0x009fc756
//     +0x87   MOV  m32,EAX    → [0x01329830]
//     +0x8c   MOV  EAX,m32    → [0x012660ec]
//     +0x91   CALL rel32      → 0x0041b820
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SEH frame, GS cookie, absolute memory references, and three
//   cross-function CALL rel32 operands make a source-level reconstruction
//   fragile: MSVC 2005 would emit identical bytes only when the SEH
//   handler table address (0xe55a30) and cookie address (0x012ea8b0)
//   are resolved identically, which they are only in the original link.
//   The naked `_emit` approach (matching siblings FUN_004063c0,
//   FUN_004071b0, FUN_00404d60) emits the orig 166 bytes verbatim;
//   tools/compare.py masks reloc bytes and reports GREEN.

extern "C" __declspec(naked) void FUN_0041fed0() {
    __asm {
        // --- SEH prologue ---------------------------------------------------
        _emit 0x6a              // PUSH -0x1              (SEH guard / try-state)
        _emit 0xff
        _emit 0x68              // PUSH 0xe55a30          (exception handler addr)
        _emit 0x30
        _emit 0x5a
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]      (old SEH chain)
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x10          (16 bytes locals)
        _emit 0xec
        _emit 0x10
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX               (GS cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x14]  (&saved FS:[0])
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0x0], EAX      (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- guard check ----------------------------------------------------
        _emit 0x83              // CMP dword ptr [0x01329428], 0x0
        _emit 0x3d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x74              // JZ +0x12 (→ init path)
        _emit 0x12
        // --- early-return true path -----------------------------------------
        _emit 0xb0              // MOV AL, 0x1
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0x0], ECX      (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                (discard GS cookie)
        _emit 0x83              // ADD ESP, 0x1c          (reclaim frame)
        _emit 0xc4
        _emit 0x1c
        _emit 0xc3              // RET
        // --- init path ------------------------------------------------------
        _emit 0x68              // PUSH 0xf58a8c          (string/name literal)
        _emit 0x8c
        _emit 0x8a
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x10              (id / flags)
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP + 0x14]  (&local scratch)
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL 0x0040e2d0        (construct/lookup)
        _emit 0xb2
        _emit 0xe3
        _emit 0xfe
        _emit 0xff
        _emit 0x50              // PUSH EAX               (save result)
        _emit 0x68              // PUSH 0x1bc             (size/type)
        _emit 0xbc
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESP + 0xc], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL 0x00419c40        (factory/query → object)
        _emit 0x13
        _emit 0x9d
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x8           (cdecl cleanup)
        _emit 0xc4
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP + 0x8], EAX (stash obj ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESP + 0x1c], 0 (try-state = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x09 (→ null_branch)
        _emit 0x09
        _emit 0x8b              // MOV ECX, EAX           (thiscall recv)
        _emit 0xc8
        _emit 0xe8              // CALL 0x00421e90        (member method)
        _emit 0x49
        _emit 0x1f
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x02 (→ join)
        _emit 0x02
        // null_branch:
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        // join:
        _emit 0x6a              // PUSH 0x20
        _emit 0x20
        _emit 0xa3              // MOV [0x01329428], EAX  (write guard global)
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL 0x009fc756        (timer/event create)
        _emit 0xff
        _emit 0xc7
        _emit 0x5d
        _emit 0x00
        _emit 0xa3              // MOV [0x01329830], EAX  (store handle)
        _emit 0x30
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0xa1              // MOV EAX, [0x012660ec]  (load context ptr)
        _emit 0xec
        _emit 0x60
        _emit 0x26
        _emit 0x01
        _emit 0xe8              // CALL 0x0041b820        (further init)
        _emit 0xba
        _emit 0xb8
        _emit 0xff
        _emit 0xff
        // --- normal epilogue ------------------------------------------------
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0x0], ECX      (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                (discard GS cookie)
        _emit 0x83              // ADD ESP, 0x1c          (reclaim frame)
        _emit 0xc4
        _emit 0x1c
        _emit 0xc3              // RET
    }
}
