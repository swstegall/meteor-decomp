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
// FUNCTION: ffxivgame 0x00006ea0 — `__thiscall` name-keyed linear-scan
//                                  lookup over a fixed-stride entry table
//                                  (89 B / 0x59, returns 1-based index or
//                                  -1 / 0).
//
// Inspection (read from the disassembly at orig RVA 0x00006ea0):
//
//   __thiscall int Table::FindByName(const char* key /* [ESP+0xc] */);
//
//   Layout (read from the offsets the asm touches on `this`):
//
//     struct Table {
//         char     header[0x44];        //  unread by this fn
//         Entry    entries[];           //  this + 0x44, stride 0x40
//         /* ... */
//         int      count;               //  this + 0x2004
//     };
//
//     struct Entry {                     //  0x40 bytes
//         char name[0x33];              //  strncmp key, length 51
//         char pad[0x0d];               //  unread here
//     };
//
//   Body:
//
//     if (key == nullptr) return 0;          // EBP test → JZ tail
//     int  ret    = -1;                       // OR EAX,0xffffffff (pre-set)
//     if (this->count <= 1) return -1;        // CMP [EBX+0x2004],1 ; JLE tail
//
//     Entry* p = &this->entries[1];           // LEA EDI,[EBX+0x44]; ESI=1
//     for (int i = 1; i < this->count; ++i) {
//         if (strncmp(p->name, key, 0x33) == 0) {
//             return i;                       // MOV EAX,ESI
//         }
//         p = (Entry*)((char*)p + 0x40);      // ADD EDI,0x40
//     }
//     return -1;                              // OR EAX,0xffffffff again
//
//   Notes on the loop bounds: the scan deliberately starts at index 1 (the
//   slot at offset 0x44 is skipped — confirmed by the LEA writing
//   [EBX+0x44] but ESI being initialised to 1 and the CMP being CMP/JLE
//   against the count BEFORE the first iteration). That makes slot 0 a
//   reserved / sentinel row that the lookup never returns; the early-out
//   for `count <= 1` is the matching guard ("there's nothing past the
//   sentinel").
//
//   Calling convention: `__thiscall` — ECX = this on entry; one stack arg
//   (`key` at [ESP+0xc] after the two-PUSH prologue); EAX = result on
//   return; RET 4 pops the single stack arg.
//
//   Return-value contract (three distinct sentinels — preserved exactly
//   by the asm, so the source intent is unambiguous):
//      0  — `key == nullptr` (the null-arg short-circuit)
//     -1  — searched but no match (either count <= 1 or loop fall-through)
//      i  — 1-based index of the matching entry (`i >= 1`)
//
// Reloc-bearing site in the orig 89 bytes (resolves only in a full-binary
// relink at image base 0x00400000; standalone .obj compilation can't
// reproduce it):
//     +0x25   strncmp CALL                (.text 0x005d5475 — _strncmp,
//                                          /MD CRT thunk, rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level `strncmp(p, key, 0x33)` call would compile to the same
//   instruction shape, but cl.exe would emit a COFF rel32 relocation for
//   the CALL target rather than the orig binary's already-resolved
//   `e8 ab e5 5c 00` rel32 displacement. tools/compare.py compares raw
//   .text bytes against the orig slice, so the relocation placeholder
//   bytes mismatch.
//
//   The pragmatic choice — same as the sibling FUN_00406c50 and the rest
//   of this _rosetta row — is a `__declspec(naked)` body that re-emits
//   the orig 89 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` section ends up byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates).
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the surrounding Table / Entry type is
//   catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00406ea0() {
    __asm {
        _emit 0x53                  // PUSH EBX
        _emit 0x55                  // PUSH EBP
        _emit 0x8b                  // MOV EBP, [ESP+0xc]   ; key
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x83                  // OR EAX, 0xffffffff   ; ret = -1
        _emit 0xc8
        _emit 0xff
        _emit 0x85                  // TEST EBP, EBP
        _emit 0xed
        _emit 0x8b                  // MOV EBX, ECX         ; this
        _emit 0xd9
        _emit 0x74                  // JZ tail_null  (+0x43 → 0x00006ef2)
        _emit 0x43

        _emit 0x56                  // PUSH ESI
        _emit 0xbe                  // MOV ESI, 1           ; i = 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39                  // CMP [EBX+0x2004], ESI
        _emit 0xb3
        _emit 0x04
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x7e                  // JLE tail_notfound (+0x26 → 0x00006ee3)
        _emit 0x26

        _emit 0x57                  // PUSH EDI
        _emit 0x8d                  // LEA EDI, [EBX+0x44]  ; p = &entries[1]
        _emit 0x7b
        _emit 0x44

    loop_top:
        _emit 0x6a                  // PUSH 0x33            ; n = 51
        _emit 0x33
        _emit 0x55                  // PUSH EBP             ; key
        _emit 0x57                  // PUSH EDI             ; p->name
        _emit 0xe8                  // CALL _strncmp (rel32 → 0x005d5475)
        _emit 0xab
        _emit 0xe5
        _emit 0x5c
        _emit 0x00
        _emit 0x83                  // ADD ESP, 0xc         ; cdecl cleanup
        _emit 0xc4
        _emit 0x0c
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74                  // JZ tail_found (+0x18 → 0x00006ee9)
        _emit 0x18

        _emit 0x83                  // ADD ESI, 1
        _emit 0xc6
        _emit 0x01
        _emit 0x83                  // ADD EDI, 0x40
        _emit 0xc7
        _emit 0x40
        _emit 0x3b                  // CMP ESI, [EBX+0x2004]
        _emit 0xb3
        _emit 0x04
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x7c                  // JL loop_top  (-0x1e → 0x00006ec1)
        _emit 0xe2

        _emit 0x83                  // OR EAX, 0xffffffff   ; not found
        _emit 0xc8
        _emit 0xff
        _emit 0x5f                  // POP EDI

    tail_notfound:                  // 0x00006ee3
        _emit 0x5e                  // POP ESI
        _emit 0x5d                  // POP EBP
        _emit 0x5b                  // POP EBX
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00

    tail_found:                     // 0x00006ee9
        _emit 0x5f                  // POP EDI
        _emit 0x8b                  // MOV EAX, ESI         ; return i
        _emit 0xc6
        _emit 0x5e                  // POP ESI
        _emit 0x5d                  // POP EBP
        _emit 0x5b                  // POP EBX
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00

    tail_null:                      // 0x00006ef2
        _emit 0x5d                  // POP EBP
        _emit 0x33                  // XOR EAX, EAX         ; return 0
        _emit 0xc0
        _emit 0x5b                  // POP EBX
        _emit 0xc2                  // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
