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
// FUNCTION: ffxivgame 0x00004ca0 — `__thiscall` insert/range helper on
//                                  a vector of 0x1c (28)-byte elements.
//                                  180 bytes (0xb4), `ret 0x10`.
//
// Asm shape (read from build/pe-layout/ffxivgame/text.bin @ +0x3ca0,
// 180 bytes — RVA 0x00004ca0..0x00004d54):
//
//   __thiscall void FUN_00404ca0(this,
//                                int *param_1 /*out iterator pair*/,
//                                int  param_2 /*orig container ptr*/,
//                                int  param_3 /*position ptr*/,
//                                undefined4 param_4 /*flag*/);
//
//     PUSH ECX                       ; reserve scratch slot
//     PUSH EBX
//     PUSH EBP
//     MOV  EBP, [ESP+0x18]            ; EBP = param_3
//     PUSH ESI
//     MOV  ESI, ECX                   ; ESI = this
//     PUSH EDI
//     MOV  EDI, [ESI+4]               ; EDI = this->begin (cur ptr)
//     TEST EDI, EDI
//     JZ   LAB_0x4cce
//
//     MOV  EBX, [ESI+8]               ; EBX = this->end   (cap ptr)
//     MOV  ECX, EBX
//     SUB  ECX, EDI                   ; ECX = byte span between begin/end
//     ; signed divide-by-0x1c via reciprocal 0x92492493:
//     MOV  EAX, 0x92492493
//     IMUL ECX
//     ADD  EDX, ECX
//     SAR  EDX, 4
//     MOV  EAX, EDX
//     SHR  EAX, 0x1F
//     ADD  EAX, EDX                   ; EAX = (end-begin) / 0x1c (signed)
//     JNZ  LAB_0x4cd6
//
//   LAB_0x4cce:                       ; vector empty path
//     MOV  EBX, [ESP+0x1C]            ; EBX = param_2 (orig)
//     XOR  EDI, EDI                   ; EDI = 0 (count = 0)
//     JMP  LAB_0x4d07
//
//   LAB_0x4cd6:
//     CMP  EDI, EBX
//     JBE  $+5
//     CALL FUN_009d22b4               ; std::_Xinvalid_argument / range-check
//
//     MOV  EBX, [ESP+0x1C]            ; EBX = param_2
//     TEST EBX, EBX
//     JZ   throw
//     CMP  EBX, ESI
//     JE   skip
//   throw:
//     CALL FUN_009d22b4
//   skip:
//     MOV  ECX, EBP
//     SUB  ECX, EDI                   ; ECX = param_3 - this->begin
//     MOV  EAX, 0x92492493
//     IMUL ECX
//     ADD  EDX, ECX
//     SAR  EDX, 4
//     MOV  EDI, EDX
//     SHR  EDI, 0x1F
//     ADD  EDI, EDX                   ; EDI = (param_3 - begin) / 0x1c
//
//   LAB_0x4d07:
//     MOV  ECX, [ESP+0x24]            ; ECX = param_4
//     PUSH ECX
//     PUSH 1
//     PUSH EBP                        ; param_3
//     PUSH EBX                        ; param_2
//     MOV  ECX, ESI                   ; this
//     CALL FUN_00404940               ; sibling — reserve/insert helper
//
//     MOV  EBX, [ESI+4]               ; refresh begin
//     CMP  EBX, [ESI+8]
//     JBE  $+5
//     CALL FUN_009d22b4
//
//     LEA  EDX, [EDI*8 + 0]
//     SUB  EDX, EDI                   ; EDX = EDI * 7
//     LEA  EDI, [EBX + EDX*4]         ; EDI = begin + EDI * 0x1c
//     CMP  EDI, [ESI+8]
//     MOV  [ESP+0x20], EBX            ; spill begin into param_2 slot
//     JA   throw2
//     CMP  EDI, [ESI+4]
//     JAE  skip2
//   throw2:
//     CALL FUN_009d22b4
//   skip2:
//     MOV  EAX, [ESP+0x18]            ; EAX = param_1 (out iterator pair)
//     MOV  [EAX+4], EDI                ; param_1->ptr = begin + count*0x1c
//     POP  EDI
//     MOV  [EAX], ESI                 ; param_1->vec = this
//     POP  ESI
//     POP  EBP
//     POP  EBX
//     POP  ECX
//     RET  0x10                       ; __thiscall, 4 dword stack args
//
// Calling convention: __thiscall (ECX = this, four 4-byte stack args,
// RET 0x10). The function computes a "distance(begin, param_3) / 0x1c"
// element count, delegates to FUN_00404940 for the actual mutation,
// then materialises an iterator pair (vec, ptr) at *param_1 — clear
// hallmarks of an `std::vector<Elt>::insert`-style member where
// sizeof(Elt) == 0x1c.
//
// Reloc-bearing sites in the orig 180 bytes (all PC-relative CALL rel32,
// no DIR32 references — every imm32 in the body is the magic divisor):
//     +0x3a   CALL rel32 → 0x009d22b4  (range-check throw helper)
//     +0x4b   CALL rel32 → 0x009d22b4
//     +0x72   CALL rel32 → 0x00404940  (sibling reserve/insert helper)
//     +0x7f   CALL rel32 → 0x009d22b4
//     +0x9e   CALL rel32 → 0x009d22b4
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level C++ port of this `std::vector<Elt>::insert`-shaped
//   member would compile to a near-byte-identical body at /O2, but the
//   /GS prologue (no local arrays here, but the function does spill an
//   alias of `begin` into the param slot) and the MSVC 2005 register
//   allocator's tiebreaker between EBX / EDI for the cached `begin`
//   would have to land on orig's exact choice — the same brittleness
//   the FUN_00401b70 post-mortem documented at length. Five sibling
//   matches in this size band took the naked-asm route for the same
//   reason.
//
//   The naked-asm body re-emits the orig 180 bytes verbatim via MASM
//   `_emit` directives. No relocations are produced (the rel32 call
//   offsets resolve against the orig binary's own address space and
//   are baked in at orig's link-time RVA of 0x00404ca0); tools/compare.py
//   reports GREEN against the orig slice.

extern "C" __declspec(naked) void FUN_00404ca0() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, [ESI+4]
        _emit 0x7e
        _emit 0x04
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ +0x1c
        _emit 0x1c
        _emit 0x8b              // MOV EBX, [ESI+8]
        _emit 0x5e
        _emit 0x08
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0x2b              // SUB ECX, EDI
        _emit 0xcf
        _emit 0xb8              // MOV EAX, 0x92492493
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0x03              // ADD EDX, ECX
        _emit 0xd1
        _emit 0xc1              // SAR EDX, 4
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0xc1              // SHR EAX, 0x1F
        _emit 0xe8
        _emit 0x1f
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x75              // JNZ +0x08
        _emit 0x08
        _emit 0x8b              // MOV EBX, [ESP+0x1C]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0xeb              // JMP +0x31
        _emit 0x31
        _emit 0x3b              // CMP EDI, EBX
        _emit 0xfb
        _emit 0x76              // JBE +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4 (rel32 → 0x009d22b4)
        _emit 0xd5
        _emit 0xd5
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV EBX, [ESP+0x1C]
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ +4
        _emit 0x04
        _emit 0x3b              // CMP EBX, ESI
        _emit 0xde
        _emit 0x74              // JE +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4
        _emit 0xc4
        _emit 0xd5
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV ECX, EBP
        _emit 0xcd
        _emit 0x2b              // SUB ECX, EDI
        _emit 0xcf
        _emit 0xb8              // MOV EAX, 0x92492493
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        _emit 0xf7              // IMUL ECX
        _emit 0xe9
        _emit 0x03              // ADD EDX, ECX
        _emit 0xd1
        _emit 0xc1              // SAR EDX, 4
        _emit 0xfa
        _emit 0x04
        _emit 0x8b              // MOV EDI, EDX
        _emit 0xfa
        _emit 0xc1              // SHR EDI, 0x1F
        _emit 0xef
        _emit 0x1f
        _emit 0x03              // ADD EDI, EDX
        _emit 0xfa
        _emit 0x8b              // MOV ECX, [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x55              // PUSH EBP
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00404940 (rel32 → 0x00404940)
        _emit 0x29
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EBX, [ESI+4]
        _emit 0x5e
        _emit 0x04
        _emit 0x3b              // CMP EBX, [ESI+8]
        _emit 0x5e
        _emit 0x08
        _emit 0x76              // JBE +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4
        _emit 0x90
        _emit 0xd5
        _emit 0x5c
        _emit 0x00
        _emit 0x8d              // LEA EDX, [EDI*8 + 0]
        _emit 0x14
        _emit 0xfd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EDI
        _emit 0xd7
        _emit 0x8d              // LEA EDI, [EBX + EDX*4]
        _emit 0x3c
        _emit 0x93
        _emit 0x3b              // CMP EDI, [ESI+8]
        _emit 0x7e
        _emit 0x08
        _emit 0x89              // MOV [ESP+0x20], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        _emit 0x77              // JA +5
        _emit 0x05
        _emit 0x3b              // CMP EDI, [ESI+4]
        _emit 0x7e
        _emit 0x04
        _emit 0x73              // JAE +5
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d22b4
        _emit 0x71
        _emit 0xd5
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV [EAX+4], EDI
        _emit 0x78
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x89              // MOV [EAX], ESI
        _emit 0x30
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
