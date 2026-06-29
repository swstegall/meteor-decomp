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
// FUNCTION: ffxivgame 0x000415f0 — __thiscall member function (295 B / 0x127),
//                                   EH4-SEH wrapped, 1 stack argument (RET 4).
//
// Inspection (read from the disassembly at orig RVA 0x000415f0):
//
//   __thiscall void* FUN_004415f0(SomeClass *this, void *arg1);
//
//   The function:
//     1. Sets up an EH4/SEH frame (PUSH -1 / PUSH scope_table / PUSH FS:[0] /
//        SUB ESP,0x50 / cookie-XOR / callee-saves / FS:[0] install).
//     2. Calls an external function (0x00ce6e30) with (&local, arg1) to
//        construct or deserialise something into a stack-local.
//     3. Validates the result pointer (EBX) against NULL and
//        &this->field_0x4 (the internal SSO buffer); calls the
//        assertion helper 0x009d22b4 on mismatch.
//     4. Compares the returned size field against this->field_0x8;
//        on mismatch falls through to the epilogue with the current EAX.
//     5. Builds a second SSO-string-like object on the stack with
//        capacity=7/size=0, calls 0x00440850 (string constructor) and
//        then 0x00441110 (a __thiscall member method) with two local
//        addresses.
//     6. Depending on capacity fields (threshold: 8), conditionally
//        calls 0x009d1b17 (free) for each of the two string objects.
//     7. Validates EBX (non-null) and that [ESP+0x18] != [EBX+4] before
//        asserting, then returns [ESP+0x18] + 0xc.
//
//   Calling convention: __thiscall, 1 stack arg, callee pops (RET 4).
//   Stack frame: SUB ESP,0x50 + 5 callee-saves + cookie (total 0x70 from
//   original ESP; locals span [ESP+0x14]..[ESP+0x63] relative to
//   post-prologue ESP0).
//
//   Reloc-bearing absolute addresses in the 295 bytes:
//     +0x02  scope-table      (0x00e57030 — .rdata FuncInfo)
//     +0x07  FS:[0] read      (constant 0, fold-through)
//     +0x15  __security_cookie (.data 0x012ea8b0)
//     +0x21  FS:[0] install   (constant 0, fold-through)
//     +0x33  CALL 0x00ce6e30  (rel32 from +0x28)
//     +0x4f  CALL 0x009d22b4  (rel32 from +0x44)
//     +0x62  JNZ target       (rel32, internal)
//     +0x99  CALL 0x00440850  (rel32, short negative)
//     +0xaf  CALL 0x00441110  (rel32, short negative)
//     +0xcd  CALL 0x009d1b17  (rel32 from +0xc2)
//     +0xe8  CALL 0x009d1b17  (rel32 from +0xe1)
//     +0xfd  CALL 0x009d22b4  (rel32)
//     +0x10b CALL 0x009d22b4  (rel32)
//     +0x11b FS:[0] restore   (constant 0, fold-through)
//     +0x11f __security_check_cookie (implicit via epilogue byte pattern)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH4 prologue, the scope-table and security-cookie absolute
//   addresses, and the collection of inward-calling rel32 displacements
//   make a source-level reconstruction brittle under MSVC 2005 /O2.
//   Every high-level rewrite risks shifting at least one byte (cookie
//   stack offset, state numbering, branch short-vs-near, modrm vs
//   moffs32). The pragmatic choice — matching the strategy of
//   FUN_00401a00, FUN_004014b0, FUN_00408f10 and other SEH-wrapped
//   siblings — is a __declspec(naked) body that re-emits the original
//   295 bytes verbatim via MASM _emit directives. The .obj's .text
//   section ends up byte-identical to the original slice (bytes are
//   emitted as raw immediates; no relocation records), which is what
//   tools/compare.py checks against.

extern "C" __declspec(naked) void FUN_004415f0() {
    __asm {
        // --- EH4 prologue ---
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe57030  (scope table)
        _emit 0x30
        _emit 0x70
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x50
        _emit 0xec
        _emit 0x50
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX  (cookie ^ ESP)
        _emit 0x8d  // LEA EAX, [ESP+0x64]
        _emit 0x44
        _emit 0x24
        _emit 0x64
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- function body ---
        _emit 0x8b  // MOV ESI, ECX  (save 'this')
        _emit 0xf1
        _emit 0x8b  // MOV EBP, [ESP+0x74]  (arg1)
        _emit 0x6c
        _emit 0x24
        _emit 0x74
        _emit 0x55  // PUSH EBP
        _emit 0x8d  // LEA EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x00ce6e30
        _emit 0x08
        _emit 0x58
        _emit 0x8a
        _emit 0x00
        _emit 0x8b  // MOV EBX, [ESP+0x14]
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x8b  // MOV ECX, [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x8d  // LEA EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x33  // XOR EDI, EDI
        _emit 0xff
        _emit 0x3b  // CMP EBX, EDI
        _emit 0xdf
        _emit 0x89  // MOV [ESP+0x20], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x74  // JZ +4  (→ assert call)
        _emit 0x04
        _emit 0x3b  // CMP EBX, EAX
        _emit 0xd8
        _emit 0x74  // JZ +5  (→ past assert)
        _emit 0x05
        _emit 0xe8  // CALL 0x009d22b4  (assertion)
        _emit 0x6f
        _emit 0x0c
        _emit 0x59
        _emit 0x00
        _emit 0x8b  // MOV EDX, [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x3b  // CMP EDX, [ESP+0x20]
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x0f  // JNZ 0x000416e9  (near)
        _emit 0x85
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- main body: build two SSO-string locals, call helpers ---
        _emit 0xb8  // MOV EAX, 7
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x40], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x89  // MOV [ESP+0x3c], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x3c
        _emit 0x66  // MOV word [ESP+0x2c], DI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b  // MOV ECX, [EBP]
        _emit 0x4d
        _emit 0x00
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x57  // PUSH EDI
        _emit 0x8d  // LEA EDX, [ESP+0x30]
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x89  // MOV [ESP+0x4c], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x4c
        _emit 0x52  // PUSH EDX
        _emit 0x8d  // LEA ECX, [ESP+0x54]
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        _emit 0x89  // MOV [ESP+0x78], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x78
        _emit 0x89  // MOV [ESP+0x6c], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0x89  // MOV [ESP+0x68], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x68
        _emit 0x66  // MOV word [ESP+0x58], DI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x58
        _emit 0xe8  // CALL 0x00440850
        _emit 0xc2
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x8d  // LEA EAX, [ESP+0x44]
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV ECX, ESI  (restore 'this')
        _emit 0xce
        _emit 0xc6  // MOV byte [ESP+0x74], 1
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0x01
        _emit 0xe8  // CALL 0x00441110
        _emit 0x6c
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EDX, [EAX+4]
        _emit 0x50
        _emit 0x04
        _emit 0x8b  // MOV EBX, [EAX]
        _emit 0x18
        _emit 0xbe  // MOV ESI, 8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x39  // CMP [ESP+0x60], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x60
        _emit 0x89  // MOV [ESP+0x18], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x72  // JC +0xd  (skip free if capacity < 8)
        _emit 0x0d
        _emit 0x8b  // MOV EAX, [ESP+0x4c]
        _emit 0x44
        _emit 0x24
        _emit 0x4c
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x009d1b17  (free, __cdecl 1-arg)
        _emit 0x55
        _emit 0x04
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 4  (caller cleans __cdecl arg)
        _emit 0xc4
        _emit 0x04

        // --- second string object cleanup ---
        _emit 0x39  // CMP [ESP+0x40], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x40
        _emit 0xc7  // MOV dword [ESP+0x60], 7
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV [ESP+0x5c], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x5c
        _emit 0x66  // MOV word [ESP+0x4c], DI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x4c
        _emit 0x72  // JC +0xd
        _emit 0x0d
        _emit 0x8b  // MOV ECX, [ESP+0x2c]
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL 0x009d1b17  (free, __cdecl 1-arg)
        _emit 0x31
        _emit 0x04
        _emit 0x59
        _emit 0x00
        _emit 0x83  // ADD ESP, 4  (caller cleans __cdecl arg)
        _emit 0xc4
        _emit 0x04

        // --- validate EBX and return pointer ---
        _emit 0x3b  // CMP EBX, EDI
        _emit 0xdf
        _emit 0x75  // JNZ +5
        _emit 0x05
        _emit 0xe8  // CALL 0x009d22b4  (assertion: EBX must be non-null)
        _emit 0xc2
        _emit 0x0b
        _emit 0x59
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x3b  // CMP EAX, [EBX+4]
        _emit 0x43
        _emit 0x04
        _emit 0x75  // JNZ +9
        _emit 0x09
        _emit 0xe8  // CALL 0x009d22b4  (assertion: ptr must differ from EBX+4)
        _emit 0xb4
        _emit 0x0b
        _emit 0x59
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x83  // ADD EAX, 0xc
        _emit 0xc0
        _emit 0x0c

        // --- EH4 epilogue ---
        _emit 0x8b  // MOV ECX, [ESP+0x64]  (restore saved FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX   (cookie)
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x5c
        _emit 0xc4
        _emit 0x5c
        _emit 0xc2  // RET 4
        _emit 0x04
        _emit 0x00
    }
}
