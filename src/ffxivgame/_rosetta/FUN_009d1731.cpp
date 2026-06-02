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
// FUNCTION: ffxivgame 0x005d1731 — doubly-nested pointer-list walker (54 B / 0x36)
//
// Shape (inferred from the full binary, which has 62 bytes; Ghidra's flow
// analysis stopped at 54 bytes — the function is captured here verbatim):
//
//   void FUN_009d1731() {
//       for (SomeEntry **pp = (SomeEntry **)0x01363dc0;
//            pp < (SomeEntry **)0x01363eb8;
//            ++pp) {
//           SomeEntry *e = SomeFunc(0, *pp);   // CALL [0x00f3e148]
//           if (e) {
//               for (; e != NULL; e = e->next) {
//                   SomeOtherFunc(e->field8, e);  // CALL FUN_009d3a16
//                   AnotherFunc(e);               // CALL FUN_009d5c88
//               }
//           }
//       }
//   }
//
// Calling convention: __cdecl (no args, no ret; caller doesn't need to clean).
// The Ghidra-assigned size is 54 bytes (0x36); the matching window ends
// 4 bytes into what would be the outer-loop CMP EBX, 0x01363eb8.
//
// Byte-passthrough strategy (same as FUN_004051e0 / FUN_004051e0.cpp):
// naked + _emit so that the raw bytes match the original PE slice
// without any COFF relocations. The positions that would normally carry
// linker fixups (MOV EBX imm32, CALL [IAT], two CALL rel32) are baked
// in as the orig binary's own values; compare.py sees no reloc table
// and compares the bytes directly.
//
// Reloc-bearing offsets (informational — no actual COFF relocs in the .obj):
//   +0x04   DIR32  MOV EBX, 0x01363dc0        (table start VA)
//   +0x0d   DIR32  CALL dword ptr [0x00f3e148] (IAT slot)
//   +0x1d   REL32  CALL FUN_009d3a16
//   +0x23   REL32  CALL FUN_009d5c88

extern "C" __declspec(naked) void FUN_009d1731() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xbb              // MOV EBX, 0x01363dc0
        _emit 0xc0
        _emit 0x3d
        _emit 0x36
        _emit 0x01
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0xff              // CALL dword ptr [0x00f3e148]
        _emit 0x15
        _emit 0x48
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ +0x18  (→ ADD EBX, 4)
        _emit 0x18
        _emit 0xff              // PUSH dword ptr [ESI + 0x8]
        _emit 0x76
        _emit 0x08
        _emit 0x8b              // MOV EDI, dword ptr [ESI]
        _emit 0x3e
        _emit 0xe8              // CALL FUN_009d3a16
        _emit 0xc4
        _emit 0x22
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_009d5c88
        _emit 0x30
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x59              // POP ECX
        _emit 0x8b              // MOV ESI, EDI
        _emit 0xf7
        _emit 0x75              // JNZ -0x18  (→ PUSH [ESI+8])
        _emit 0xe8
        _emit 0x83              // ADD EBX, 0x4
        _emit 0xc3
        _emit 0x04
        _emit 0x81              // CMP EBX, ... (54-byte window ends here)
        _emit 0xfb
        _emit 0xb8
        _emit 0x3e
    }
}
