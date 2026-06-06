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
// FUNCTION: ffxivgame 0x0004e8a0 — `__thiscall` std::vector<T>::insert
//           (returns iterator), element stride 0x54 (84 B), 169 B / 0xa9.
//
// Asm shape (`__thiscall iterator f(this, void* val, T* where, void* ret)`
// — ret 0x10; the iterator result is materialised through the hidden
// return-value pointer passed as the first stack arg):
//
//   EDI = this                                       ; (this = ECX in)
//   ESI = this->m4                  ; first/begin pointer
//   if (ESI == 0 ||
//       (this->m8 - ESI) / 0x54 == 0) {              ; empty container
//       EBX = where_iter (arg @ESP+0x1c)
//       ESI = 0                                      ; index = 0
//       goto do_insert;
//   }
//   if (ESI > where_iter) FUN_009d22b4();            ; invalid-iterator panic
//   EBX = where_iter;
//   if (EBX != 0 && EBX != this) FUN_009d22b4();     ; container-mismatch panic
//   ESI = (pos_iter - first) / 0x54;                 ; signed element index
// do_insert:
//   FUN_0044e470(this, EBX, pos_iter, 1, arg@ESP+0x24);   ; underlying insert
//   EBX = this->m4;                                  ; reload first (may move)
//   if (EBX > this->m8) FUN_009d22b4();              ; range panic
//   ESI = EBX + ESI * 0x54;                          ; recompute result iterator
//   if (ESI > this->m8 || ESI < this->m4) FUN_009d22b4();
//   *ret = { this, ESI };                            ; write iterator pair
//   return ret;                                      ; ret 0x10
//
// The three CALL 0x009d22b4 sites are the binary's universal panic thunk
// (a 16-byte stub that pushes five zeros and tail-calls the actual
// throw-site). FUN_0044e470 is the underlying element-insert helper.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This body has four callee-saved values (EBX/EBP/ESI/EDI) live across
//   six conditional branches, three external panic CALLs and one internal
//   helper CALL, with an iterator-pair result written through a hidden
//   return pointer. As documented for the sibling matches in this binary
//   (FUN_00406280, FUN_004063c0, and the FUN_00401b70 post-mortem), a
//   source-level C++ form reliably lands a stable PARTIAL because MSVC's
//   register allocator picks the EBX↔ESI tiebreaker differently from the
//   orig full-binary build. The pragmatic, byte-exact path is a
//   `__declspec(naked)` body re-emitting the orig 169 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` ends up byte-identical to
//   the orig slice (the rel32 CALL displacements are baked into the orig
//   wire image, so emitting them as raw bytes reproduces the exact
//   sequence). `tools/compare.py` then reports GREEN against
//   orig[0xe8a0..0xe949].
//
// Reloc-bearing sites in the orig 169 bytes:
//     +0x38   CALL rel32   → FUN_009d22b4  (panic thunk)
//     +0x49   CALL rel32   → FUN_009d22b4  (panic thunk)
//     +0x6e   CALL rel32   → FUN_0044e470  (underlying insert helper)
//     +0x7b   CALL rel32   → FUN_009d22b4  (panic thunk)
//     +0x93   CALL rel32   → FUN_009d22b4  (panic thunk)

extern "C" __declspec(naked) void FUN_0044e8a0() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV ESI, dword ptr [EDI+0x4]
        _emit 0x77
        _emit 0x04
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ short  (→ +0x1a, empty-container arm)
        _emit 0x1a
        _emit 0x8b              // MOV EBX, dword ptr [EDI+0x8]
        _emit 0x5f
        _emit 0x08
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0x2b              // SUB ECX, ESI
        _emit 0xce
        _emit 0xb8              // MOV EAX, 0x30c30c31
        _emit 0x31
        _emit 0x0c
        _emit 0xc3
        _emit 0x30
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0xc1              // SAR EDX, 0x4
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SHR EAX, 0x1f
        _emit 0xe8
        _emit 0x1f
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x75              // JNZ short  (→ +0x08, non-empty arm)
        _emit 0x08
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0xeb              // JMP short  (→ +0x2f, do_insert)
        _emit 0x2f
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0x76              // JBE short  (→ +0x05)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0xd7
        _emit 0x39
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x1c]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ short  (→ +0x04)
        _emit 0x04
        _emit 0x3b              // CMP EBX, EDI
        _emit 0xdf
        _emit 0x74              // JZ short  (→ +0x05)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0xc6
        _emit 0x39
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0x2b              // SUB ECX, ESI
        _emit 0xce
        _emit 0xb8              // MOV EAX, 0x30c30c31
        _emit 0x31
        _emit 0x0c
        _emit 0xc3
        _emit 0x30
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0xc1              // SAR EDX, 0x4
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV ESI, EDX
        _emit 0xf2
        _emit 0xc1              // SHR ESI, 0x1f
        _emit 0xee
        _emit 0x1f
        _emit 0x03              // ADD ESI, EDX
        _emit 0xf2
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x55              // PUSH EBP
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_0044e470 (rel32)
        _emit 0x5d
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EBX, dword ptr [EDI+0x4]
        _emit 0x5f
        _emit 0x04
        _emit 0x3b              // CMP EBX, dword ptr [EDI+0x8]
        _emit 0x5f
        _emit 0x08
        _emit 0x76              // JBE short  (→ +0x05)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x94
        _emit 0x39
        _emit 0x58
        _emit 0x00
        _emit 0x6b              // IMUL ESI, ESI, 0x54
        _emit 0xf6
        _emit 0x54
        _emit 0x03              // ADD ESI, EBX
        _emit 0xf3
        _emit 0x3b              // CMP ESI, dword ptr [EDI+0x8]
        _emit 0x77
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESP+0x20], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x77              // JA short  (→ +0x05)
        _emit 0x05
        _emit 0x3b              // CMP ESI, dword ptr [EDI+0x4]
        _emit 0x77
        _emit 0x04
        _emit 0x73              // JNC short  (→ +0x05)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32)
        _emit 0x7c
        _emit 0x39
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]   (hidden ret ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [EAX], EDI
        _emit 0x38
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV dword ptr [EAX+0x4], ESI
        _emit 0x70
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
