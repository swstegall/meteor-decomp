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
// FUNCTION: ffxivgame 0x000515d0 — __thiscall std::basic_string range-validated
//                                  iterator-pair forwarder (151 B / 0x97, ret 0).
//
// Calling convention: __thiscall (ECX = this); no stack args; returns void
// (plain RET — callee pops nothing).
//
// Object layout (MSVC 2005 std::basic_string SSO, this = ECX = EDI):
//     +0x04  union { char _Buf[16]; char *_Ptr; } _Bx
//     +0x14  size_type _Mysize     (current length)
//     +0x18  size_type _Myres      (capacity; < 16 → inline SSO buffer)
//
// Behaviour (recovered from asm @ 0x004515d0):
//
//   Block 1 (bytes 0x00–0x43): compute EBP = end-of-string (data_ptr + _Mysize).
//     Three consecutive SSO-dereference sequences on the same string, with
//     range guards calling FUN_009d22b4 (_invalid_parameter) if:
//       (a) end_ptr == NULL
//       (b) data_ptr > end_ptr
//       (c) end_ptr > data_ptr + size   (tautologically false — guard is still emitted)
//
//   Block 2 (bytes 0x49–0x79): compute EBX = begin-of-string (= data_ptr).
//     Another three SSO-dereference sequences with guards:
//       (a) begin_ptr == NULL
//       (b) data_ptr > begin_ptr   (= begin_ptr > begin_ptr, always false)
//       (c) begin_ptr > data_ptr + size  (= false for valid string)
//
//   Final call (bytes 0x7f–0x96):
//     Passes two MSVC 2005 checked iterators (container*, raw_ptr) to
//     FUN_00451470 via five stack pushes plus ECX=this:
//       arg1 = &local_8bytes   (sret-style output area on our frame)
//       arg2 = this            (= begin_iterator.container)
//       arg3 = EBX             (= begin_iterator.ptr  = data_ptr)
//       arg4 = this            (= end_iterator.container)
//       arg5 = EBP             (= end_iterator.ptr    = data_ptr + size)
//
// CALL targets (all REL32 — wildcarded by tools/compare.py):
//   +0x44   CALL FUN_009d22b4   — _invalid_parameter (block-1 error gate)
//   +0x7a   CALL FUN_009d22b4   — _invalid_parameter (block-2 error gate)
//   +0x8a   CALL FUN_00451470   — actual work (__thiscall, 5 stack args)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   MSVC 2005 emits the SSO-check idiom (cmp Myres,0x10 / jc small /
//   mov eax,[esi] / jmp done / small: mov eax,esi / done:) three times in
//   block 1 and three times in block 2, never caching the result in a
//   register across checks.  This produces twelve jc/jmp pairs and two
//   separate FUN_009d22b4 call sites that source-level C++ at /O2 will
//   not reproduce without precisely-redundant expressions.  Raw _emit is
//   used for every instruction (one _emit per line per MSVC inline asm
//   rules) so the displacements in the 12 short-branch pairs are
//   guaranteed exact.  The three `call FUN_xxx` forms produce COFF REL32
//   reloc entries that compare.py masks during the byte diff.

extern "C" void FUN_009d22b4();   // _invalid_parameter (REL32)
extern "C" void FUN_00451470();   // actual work function, __thiscall (REL32)

extern "C" __declspec(naked) void FUN_004515d0() {
    __asm {
        // +0x00: 83 ec 08   SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // +0x03: 53         PUSH EBX
        _emit 0x53
        // +0x04: 55         PUSH EBP
        _emit 0x55
        // +0x05: 56         PUSH ESI
        _emit 0x56
        // +0x06: 57         PUSH EDI
        _emit 0x57
        // +0x07: 8b f9      MOV EDI, ECX
        _emit 0x8b
        _emit 0xf9
        // +0x09: 8b 57 18   MOV EDX, [EDI+0x18]
        _emit 0x8b
        _emit 0x57
        _emit 0x18
        // +0x0c: 83 fa 10   CMP EDX, 0x10
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        // +0x0f: 8d 77 04   LEA ESI, [EDI+4]
        _emit 0x8d
        _emit 0x77
        _emit 0x04
        // +0x12: 72 04      JC +4
        _emit 0x72
        _emit 0x04
        // +0x14: 8b 06      MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // +0x16: eb 02      JMP +2
        _emit 0xeb
        _emit 0x02
        // +0x18: 8b c6      MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // +0x1a: 8b 4f 14   MOV ECX, [EDI+0x14]
        _emit 0x8b
        _emit 0x4f
        _emit 0x14
        // +0x1d: 8d 2c 01   LEA EBP, [ECX+EAX]
        _emit 0x8d
        _emit 0x2c
        _emit 0x01
        // +0x20: 85 ed      TEST EBP, EBP
        _emit 0x85
        _emit 0xed
        // +0x22: 74 20      JZ +0x20
        _emit 0x74
        _emit 0x20
        // +0x24: 83 fa 10   CMP EDX, 0x10
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        // +0x27: 72 04      JC +4
        _emit 0x72
        _emit 0x04
        // +0x29: 8b 06      MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // +0x2b: eb 02      JMP +2
        _emit 0xeb
        _emit 0x02
        // +0x2d: 8b c6      MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // +0x2f: 3b c5      CMP EAX, EBP
        _emit 0x3b
        _emit 0xc5
        // +0x31: 77 11      JA +0x11
        _emit 0x77
        _emit 0x11
        // +0x33: 83 fa 10   CMP EDX, 0x10
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        // +0x36: 72 04      JC +4
        _emit 0x72
        _emit 0x04
        // +0x38: 8b 06      MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // +0x3a: eb 02      JMP +2
        _emit 0xeb
        _emit 0x02
        // +0x3c: 8b c6      MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // +0x3e: 03 c8      ADD ECX, EAX
        _emit 0x03
        _emit 0xc8
        // +0x40: 3b e9      CMP EBP, ECX
        _emit 0x3b
        _emit 0xe9
        // +0x42: 76 05      JBE +5
        _emit 0x76
        _emit 0x05
        // +0x44: e8 ...     CALL FUN_009d22b4  (REL32, compare.py masks)
        call FUN_009d22b4
        // +0x49: 8b 4f 18   MOV ECX, [EDI+0x18]
        _emit 0x8b
        _emit 0x4f
        _emit 0x18
        // +0x4c: 83 f9 10   CMP ECX, 0x10
        _emit 0x83
        _emit 0xf9
        _emit 0x10
        // +0x4f: 72 04      JC +4
        _emit 0x72
        _emit 0x04
        // +0x51: 8b 1e      MOV EBX, [ESI]
        _emit 0x8b
        _emit 0x1e
        // +0x53: eb 02      JMP +2
        _emit 0xeb
        _emit 0x02
        // +0x55: 8b de      MOV EBX, ESI
        _emit 0x8b
        _emit 0xde
        // +0x57: 85 db      TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // +0x59: 74 1f      JZ +0x1f
        _emit 0x74
        _emit 0x1f
        // +0x5b: 83 f9 10   CMP ECX, 0x10
        _emit 0x83
        _emit 0xf9
        _emit 0x10
        // +0x5e: 72 04      JC +4
        _emit 0x72
        _emit 0x04
        // +0x60: 8b 06      MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // +0x62: eb 02      JMP +2
        _emit 0xeb
        _emit 0x02
        // +0x64: 8b c6      MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // +0x66: 3b c3      CMP EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // +0x68: 77 10      JA +0x10
        _emit 0x77
        _emit 0x10
        // +0x6a: 83 f9 10   CMP ECX, 0x10
        _emit 0x83
        _emit 0xf9
        _emit 0x10
        // +0x6d: 72 02      JC +2
        _emit 0x72
        _emit 0x02
        // +0x6f: 8b 36      MOV ESI, [ESI]
        _emit 0x8b
        _emit 0x36
        // +0x71: 8b 47 14   MOV EAX, [EDI+0x14]
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        // +0x74: 03 c6      ADD EAX, ESI
        _emit 0x03
        _emit 0xc6
        // +0x76: 3b d8      CMP EBX, EAX
        _emit 0x3b
        _emit 0xd8
        // +0x78: 76 05      JBE +5
        _emit 0x76
        _emit 0x05
        // +0x7a: e8 ...     CALL FUN_009d22b4  (REL32, compare.py masks)
        call FUN_009d22b4
        // +0x7f: 55         PUSH EBP
        _emit 0x55
        // +0x80: 57         PUSH EDI
        _emit 0x57
        // +0x81: 53         PUSH EBX
        _emit 0x53
        // +0x82: 57         PUSH EDI
        _emit 0x57
        // +0x83: 8d 4c 24 20   LEA ECX, [ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // +0x87: 51         PUSH ECX
        _emit 0x51
        // +0x88: 8b cf      MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // +0x8a: e8 ...     CALL FUN_00451470  (REL32, compare.py masks)
        call FUN_00451470
        // +0x8f: 5f         POP EDI
        _emit 0x5f
        // +0x90: 5e         POP ESI
        _emit 0x5e
        // +0x91: 5d         POP EBP
        _emit 0x5d
        // +0x92: 5b         POP EBX
        _emit 0x5b
        // +0x93: 83 c4 08   ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // +0x96: c3         RET
        _emit 0xc3
    }
}
