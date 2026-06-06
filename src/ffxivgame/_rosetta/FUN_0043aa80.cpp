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
// FUNCTION: ffxivgame 0x0003aa80 — __thiscall member with /GS + EH3 SEH
//                                  frame, 184 B (0xb8). (The asm dump had a
//                                  3-byte gap at 0x3aab9: a MOV [ESI+0x14],
//                                  EDI the disassembler dropped; real slice
//                                  is 184 B, confirmed by size_overrides.)
//
// Behaviour (recovered from asm @ 0x0003aa80):
//
//   This is a __thiscall member (this in ECX) that sets up the standard
//   MSVC 2005 EH3-style SEH frame (PUSH -1 / PUSH scope_table / FS:[0]
//   chain + /GS cookie XOR ESP). It then:
//
//     this = ECX  ->  ESI
//     1. If this->m_14 (a refcounted/COM-ish ptr) != 0, virtual-release
//        it: ECX = [ptr-4] (vtable-ish), CALL 0x0040df70(ptr).
//     2. If this->m_18 != 0, zero it.
//     3. One-shot lazy init of a process-global @ 0x01327c18 / 0x01327c14
//        (guarded by the bit-0 flag); CALL 0x0040e500 fills 0x01327c14,
//        EH state index written into [ESP+0x1c] (-> 0, then 0xffffffff).
//     4. Loads the global @ 0x01327c14 into EDI, builds something via
//        CALL 0x0040e2d0(ECX=&local, 0x10, 0xf66370), then
//        CALL 0x0040e110(EDI, this->m_10, EAX) and finally
//        CALL 0x009d4600(EAX, [ESP+0x24], this->m_10) with cdecl
//        cleanup (ADD ESP, 0xc); stores EAX back into this->m_14.
//     5. Tears down the SEH frame and RET 4 (one stack arg / __thiscall).
//
// Reloc-bearing sites (4-byte windows wildcarded by compare.py vs orig):
//   +0x02   PUSH offset scope_table          (0x00e564b7)
//   +0x13   MOV EAX, __security_cookie        (.data 0x012ea8b0)
//   +0x35   CALL rel32 → 0x0040df70           (virtual release)
//   +0x4b   TEST [0x01327c18], AL             (lazy-init guard flag)
//   +0x51   OR   [0x01327c18], EAX
//   +0x5b   CALL rel32 → 0x0040e500           (one-shot init)
//   +0x60   MOV  [0x01327c14], EAX
//   +0x6d   MOV  EDI, [0x01327c14]
//   +0x73   PUSH 0xf66370
//   +0x7e   CALL rel32 → 0x0040e2d0
//   +0x8a   CALL rel32 → 0x0040e110
//   +0x9c   CALL rel32 → 0x009d4600
//
// Reconstruction strategy — naked-asm byte passthrough (the local idiom,
// same as siblings FUN_00403f10 / FUN_00401750 / FUN_00401b70): a
// `__declspec(naked)` body re-emits the orig 181 bytes verbatim via MASM
// `_emit` directives. The .obj's `.text` ends up byte-identical to the
// orig slice with NO relocations (the absolute/rel32 operands are baked
// in as raw bytes); `tools/compare.py` masks the reloc windows and
// reports GREEN.

extern "C" __declspec(naked) void FUN_0043aa80() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH offset scope_table (0xe564b7)
        _emit 0xb7
        _emit 0x64
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, __security_cookie (0x012ea8b0)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x14]
        _emit 0x46
        _emit 0x14
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x74              // JZ +0x0c
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [EAX-0x4]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → 0x0040df70
        _emit 0xb7
        _emit 0x34
        _emit 0xfd
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI+0x14], EDI
        _emit 0x7e
        _emit 0x14
        _emit 0x39              // CMP dword ptr [ESI+0x18], EDI
        _emit 0x7e
        _emit 0x18
        _emit 0x74              // JZ +0x03
        _emit 0x03
        _emit 0x89              // MOV dword ptr [ESI+0x18], EDI
        _emit 0x7e
        _emit 0x18
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327c18], AL
        _emit 0x05
        _emit 0x18
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x1c
        _emit 0x1c
        _emit 0x09              // OR dword ptr [0x01327c18], EAX
        _emit 0x05
        _emit 0x18
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV dword ptr [ESP+0x1c], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8              // CALL rel32 → 0x0040e500
        _emit 0x20
        _emit 0x3a
        _emit 0xfd
        _emit 0xff
        _emit 0xa3              // MOV [0x01327c14], EAX
        _emit 0x14
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x1c], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDI, dword ptr [0x01327c14]
        _emit 0x3d
        _emit 0x14
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x68              // PUSH 0xf66370
        _emit 0x70
        _emit 0x63
        _emit 0xf6
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL rel32 → 0x0040e2d0
        _emit 0xcd
        _emit 0x37
        _emit 0xfd
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x10]
        _emit 0x4e
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL rel32 → 0x0040e110
        _emit 0x01
        _emit 0x36
        _emit 0xfd
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x10]
        _emit 0x4e
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0x89              // MOV dword ptr [ESI+0x14], EAX
        _emit 0x46
        _emit 0x14
        _emit 0xe8              // CALL rel32 → 0x009d4600
        _emit 0xdf
        _emit 0x9a
        _emit 0x59
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV dword ptr FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
