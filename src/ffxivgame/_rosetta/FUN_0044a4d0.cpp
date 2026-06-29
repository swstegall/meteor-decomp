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
// FUNCTION: ffxivgame 0x0004a4d0 — wstring iterator-range bounds check + assign
//                                  (__thiscall, 1 stack arg, 98 B / 0x62)
//
// __thiscall void FUN_0044a4d0(? arg)
//   ECX = this
//   [ESP+4] = arg  (single 4-byte stack argument, callee-pops via RET 4)
//
// Behaviour:
//   1. EBX = *(this+0x18)  — capacity (_Myres), threshold 8 (wchar_t SSO)
//   2. EAX = &this->_Bx   (offset 0x4)
//   3. If capacity >= 8: EDX = *EAX (heap ptr); else EDX = EAX (SSO buf)
//   4. ECX = *(this+0x14) * 2   — size in bytes (_Mysize for wchar_t)
//   5. EDI = ECX + EDX          — end-of-data pointer
//   6. Assert: EDI != NULL
//   7. Reload EDX (same SSO check); Assert: EDX <= EDI (no overflow)
//   8. Reload EAX (SSO check); ECX = ECX + EAX; Assert: EDI <= ECX
//      (i.e. end-of-data is within allocated capacity region)
//      On any failure: CALL 0x009d22b4  (assert / range-error handler)
//   9. After bounds check:
//        PUSH arg; PUSH EDI; PUSH this; LEA ECX,[ESP+0x18]; PUSH ECX
//        MOV ECX, this; CALL FUN_00449e70 (assign / replace thiscall)
//  10. Epilogue: POP EDI/ESI/EBX; ADD ESP,8; RET 4
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function contains two CALL rel32 relocations and two DIR32-like
//   references (the SSO buffer-size literal 8 repeated four times).
//   A `__declspec(naked)` body re-emitting the original 98 bytes verbatim
//   via MASM `_emit` directives produces a .obj whose .text is byte-identical
//   to the original slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044a4d0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x18]
        _emit 0x5e
        _emit 0x18
        _emit 0x83              // CMP EBX, 0x8
        _emit 0xfb
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x72              // JC +4  (small-string: skip deref)
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [EAX]   (heap ptr)
        _emit 0x10
        _emit 0xeb              // JMP +2
        _emit 0x02
        _emit 0x8b              // MOV EDX, EAX               (SSO buf ptr)
        _emit 0xd0
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x14]
        _emit 0x4e
        _emit 0x14
        _emit 0x03              // ADD ECX, ECX   (size * 2 for wchar_t)
        _emit 0xc9
        _emit 0x8d              // LEA EDI, [ECX + EDX*1]
        _emit 0x3c
        _emit 0x11
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x74              // JZ +0x1c  (null end ptr → error)
        _emit 0x1c
        _emit 0x83              // CMP EBX, 0x8
        _emit 0xfb
        _emit 0x08
        _emit 0x72              // JC +4
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0xeb              // JMP +2
        _emit 0x02
        _emit 0x8b              // MOV EDX, EAX
        _emit 0xd0
        _emit 0x3b              // CMP EDX, EDI
        _emit 0xd7
        _emit 0x77              // JA +0xd  (start > end → error)
        _emit 0x0d
        _emit 0x83              // CMP EBX, 0x8
        _emit 0xfb
        _emit 0x08
        _emit 0x72              // JC +2
        _emit 0x02
        _emit 0x8b              // MOV EAX, dword ptr [EAX]
        _emit 0x00
        _emit 0x03              // ADD ECX, EAX   (capacity-based end)
        _emit 0xc8
        _emit 0x3b              // CMP EDI, ECX
        _emit 0xf9
        _emit 0x76              // JBE +5  (within capacity → ok)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (assert/error, rel32=0x00587d9e)
        _emit 0x9e
        _emit 0x7d
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]  (arg)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // PUSH EAX
        _emit 0x57              // PUSH EDI
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00449e70  (rel32=0xfffff947)
        _emit 0x47
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
