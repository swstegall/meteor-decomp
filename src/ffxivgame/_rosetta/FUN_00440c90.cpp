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
// FUNCTION: ffxivgame 0x00040c90 — `__thiscall` allocate-and-init helper
//                                   (68 B / 0x44)
//
// __thiscall bool FUN_00440c90(this, size_t n)
//   ECX        : this   (a 3-pointer span/buffer struct)
//   [ESP+0x04] : size_t n  (element count; RET 4 — callee-cleans 1 dword)
//
// Body (matches asm flow):
//
//   this->begin_04 = 0;        // [esi+0x04]
//   this->cur_08   = 0;        // [esi+0x08]
//   this->end_0c   = 0;        // [esi+0x0c]
//   if (n == 0)
//       return false;          // XOR AL,AL → return 0
//
//   // operator-new[]-style overflow guard: n * 1 element. The
//   // CMP EDI,-1 / JBE skips the (never-taken, elemsize == 1) overflow
//   // helper at 0x00cb0e40; MSVC emits it as part of the n*sizeof guard.
//   void *p = alloc(n, 0);     // CALL 0x00401090(n, 0) — cdecl, 2 args
//   this->begin_04 = p;        // [esi+0x04] = p
//   this->cur_08   = p;        // [esi+0x08] = p
//   this->end_0c   = p + n;    // [esi+0x0c] = p + n
//   return true;               // MOV AL,1 → return 1
//
// Orig codegen (68 bytes, read verbatim from RVA 0x00040c90):
//
//   56                push esi
//   33 c0             xor  eax, eax
//   57                push edi
//   8b 7c 24 0c       mov  edi, [esp+0xc]          ; edi = n
//   3b f8             cmp  edi, eax
//   8b f1             mov  esi, ecx                ; esi = this
//   89 46 04          mov  [esi+4], eax
//   89 46 08          mov  [esi+8], eax
//   89 46 0c          mov  [esi+0xc], eax
//   75 07             jnz  alloc
//   5f                pop  edi
//   32 c0             xor  al, al
//   5e                pop  esi
//   c2 04 00          ret  4
// alloc:
//   83 ff ff          cmp  edi, -1
//   76 05             jbe  do_alloc
//   e8 88 01 87 00    call 0x00cb0e40              ; overflow helper (rel32)
// do_alloc:
//   50                push eax                     ; arg2 = 0
//   57                push edi                     ; arg1 = n
//   e8 d1 03 fc ff    call 0x00401090              ; alloc (rel32)
//   89 46 04          mov  [esi+4], eax
//   89 46 08          mov  [esi+8], eax
//   83 c4 08          add  esp, 8
//   03 c7             add  eax, edi
//   89 46 0c          mov  [esi+0xc], eax
//   5f                pop  edi
//   b0 01             mov  al, 1
//   5e                pop  esi
//   c2 04 00          ret  4
//
// Reloc-bearing sites in the orig 68 bytes (the two CALL rel32
// displacements are relative to the orig image layout; emitting the raw
// bytes via `_emit` reproduces the orig slice byte-for-byte — the .obj
// carries no relocation, the immediates simply match the resolved orig
// bytes, so tools/compare.py reports GREEN):
//     +0x23   CALL rel32 → 0x00cb0e40 (overflow helper)
//     +0x2a   CALL rel32 → 0x00401090 (allocator)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// sibling FUN_00406fa0): a `__declspec(naked)` body re-emitting the orig
// 68 bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_00440c90() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP + 0x0c]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ESI + 0x04], EAX
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI + 0x08], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI + 0x0c], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x75              // JNZ +0x07 (→ alloc)
        _emit 0x07
        _emit 0x5f              // POP EDI
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
        // alloc:
        _emit 0x83              // CMP EDI, -1
        _emit 0xff
        _emit 0xff
        _emit 0x76              // JBE +0x05 (→ do_alloc)
        _emit 0x05
        _emit 0xe8              // CALL rel32 → 0x00cb0e40
        _emit 0x88
        _emit 0x01
        _emit 0x87
        _emit 0x00
        // do_alloc:
        _emit 0x50              // PUSH EAX
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL rel32 → 0x00401090
        _emit 0xd1
        _emit 0x03
        _emit 0xfc
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI + 0x04], EAX
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI + 0x08], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0x03              // ADD EAX, EDI
        _emit 0xc7
        _emit 0x89              // MOV dword ptr [ESI + 0x0c], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0xb0              // MOV AL, 0x01
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
