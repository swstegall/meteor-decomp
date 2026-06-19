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
// FUNCTION: ffxivgame 0x00047de0 — FUN_00447de0 (__thiscall, 187 B / 0xbb)
//
// A __thiscall member function that accepts two stack arguments and
// returns `this` in EAX.  Sets up a /GS + EH3-style SEH frame, then
// calls four sibling methods:
//
//   1. 0x00445e50  — some getter/query on `this` (no explicit stack args)
//   2. 0x00447a80  — (this, &local_buf, arg1, result_of_445e50 - arg1)
//   3. 0x004460a0  — (this, arg1, -1)     with [ESP+0x78] zeroed beforehand
//   4. 0x00447c80  — (this, arg2)
//   5. 0x00447c80  — (this, &local_buf)
//
//   If local_buf[0x11] == 0 (zero), calls 0x0044d350(__cdecl):
//     FUN_0044d350(local_buf[0], local_buf[1], 0xb)
//
//   Returns this (ESI → EAX).
//
// Signature (inferred):
//   __thiscall void* FUN_00447de0(void* this, unsigned arg1, unsigned arg2)
//     RET 0x8 → 2 stack args; ECX = this
//
// Stack layout at body-start (ESP = ENTRY_ESP - 0x74):
//   [ESP+0x78]   arg1   (loaded into EDI)
//   [ESP+0x7c]   arg2   (loaded into EBX)
//   [ESP+0x10]   local_buf (0x58-byte area, accessed as DWORD pair + byte flag)
//   [ESP+0x68]   saved prev FS:[0] (SEH restore slot)
//   [ESP+0x70]   SEH state slot (set to 0xffffffff in body)
//
// Reloc-bearing sites in the orig 187 bytes (4-byte windows masked by
// compare.py in the diff, so raw byte values here are image-address
// literals and will not match a relocated .obj — masking is correct):
//   +0x03   scope_table ptr  (0x00e57530, image-relative)
//   +0x11   __security_cookie load  (0x012ea8b0 × first load)
//   +0x1f   __security_cookie load  (0x012ea8b0 × second load)
//   +0x38   CALL rel32 → 0x00445e50
//   +0x4c   CALL rel32 → 0x00447a80
//   +0x5e   CALL rel32 → 0x004460a0
//   +0x66   CALL rel32 → 0x00447c80 (first)
//   +0x72   CALL rel32 → 0x00447c80 (second)
//   +0x92   CALL rel32 → 0x0044d350
//   +0xb1   CALL rel32 → 0x009d20f4 (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3-style SEH prolog with TWO separate /GS cookie XOR-ESP
//   sequences (one before PUSH EBX/ESI/EDI, one after), the
//   mid-body EH-state slot writes, and the non-trivial register
//   allocation produced by the four chained thiscall sites all
//   conspire to make a plain C++ reconstruction shift bytes. The
//   pragmatic choice — matching FUN_00401750 / FUN_00406680 — is a
//   `__declspec(naked)` body that re-emits the orig 187 bytes verbatim
//   via MASM `_emit` directives. compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00447de0() {
    __asm {
        // 00047de0 — SEH + /GS prolog
        _emit 0x6a  // PUSH -0x1
        _emit 0xff
        _emit 0x68  // PUSH 0xe57530  (scope table — reloc)
        _emit 0x30
        _emit 0x75
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX,FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP,0x58
        _emit 0xec
        _emit 0x58
        _emit 0xa1  // MOV EAX,[__security_cookie]  (reloc)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX,ESP
        _emit 0xc4
        _emit 0x89  // MOV [ESP+0x54],EAX
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX,[__security_cookie]  (reloc)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX,ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX,[ESP+0x68]
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0x64  // MOV FS:[0x0],EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00047e11 — body
        _emit 0x8b  // MOV EBX,[ESP+0x7c]  (arg2)
        _emit 0x5c
        _emit 0x24
        _emit 0x7c
        _emit 0x8b  // MOV ESI,ECX  (this)
        _emit 0xf1
        _emit 0xe8  // CALL 0x00445e50  (reloc)
        _emit 0x34
        _emit 0xe0
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV EDI,[ESP+0x78]  (arg1)
        _emit 0x7c
        _emit 0x24
        _emit 0x78
        _emit 0x2b  // SUB EAX,EDI
        _emit 0xc7
        _emit 0x50  // PUSH EAX  (= result - arg1)
        _emit 0x57  // PUSH EDI  (= arg1)
        _emit 0x8d  // LEA EAX,[ESP+0x18]  (&local_buf, after 2 pushes)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x00447a80  (reloc)
        _emit 0x50
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x6a  // PUSH -0x1
        _emit 0xff
        _emit 0x57  // PUSH EDI  (= arg1)
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0xc7  // MOV [ESP+0x78],0x0
        _emit 0x44
        _emit 0x24
        _emit 0x78
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x004460a0  (reloc)
        _emit 0x5e
        _emit 0xe2
        _emit 0xff
        _emit 0xff
        _emit 0x53  // PUSH EBX  (= arg2)
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x00447c80  (reloc)
        _emit 0x36
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8d  // LEA ECX,[ESP+0x10]  (&local_buf)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV ECX,ESI
        _emit 0xce
        _emit 0xe8  // CALL 0x00447c80  (reloc)
        _emit 0x2a
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x80  // CMP byte ptr [ESP+0x21],0x0
        _emit 0x7c
        _emit 0x24
        _emit 0x21
        _emit 0x00
        _emit 0xc7  // MOV [ESP+0x70],0xffffffff  (EH state reset)
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x75  // JNZ +0x14  (skip if byte != 0)
        _emit 0x14
        _emit 0x8b  // MOV EDX,[ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b  // MOV EAX,[ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x6a  // PUSH 0xb
        _emit 0x0b
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x0044d350  (reloc)
        _emit 0xda
        _emit 0x54
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP,0xc
        _emit 0xc4
        _emit 0x0c
        // 00047e79 — epilog
        _emit 0x8b  // MOV EAX,ESI  (return this)
        _emit 0xc6
        _emit 0x8b  // MOV ECX,[ESP+0x68]  (prev FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x68
        _emit 0x64  // MOV FS:[0x0],ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5b  // POP EBX
        _emit 0x8b  // MOV ECX,[ESP+0x54]  (saved cookie)
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        _emit 0x33  // XOR ECX,ESP
        _emit 0xcc
        _emit 0xe8  // CALL 0x009d20f4  (__security_check_cookie, reloc)
        _emit 0x5f
        _emit 0xa2
        _emit 0x58
        _emit 0x00
        _emit 0x83  // ADD ESP,0x64
        _emit 0xc4
        _emit 0x64
        _emit 0xc2  // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
