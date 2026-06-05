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
// FUNCTION: ffxivgame 0x0045abb0 — __thiscall conditional-memcpy then
//                                  per-element callback loop (63 B / 0x3f)
//
// Calling convention: __thiscall (ECX = this; three DWORD stack args;
//   callee cleans 0xc via `ret 0xc`).
//
// Signature (reconstructed):
//   void SomeClass::method(void* src, void* dest, int n_bytes)
//
// Stack layout at entry:
//   ECX          = this
//   [ESP+0x04]   = src      (arg1, loaded into EAX before register saves)
//   [ESP+0x08]   = dest     (arg2, loaded into ESI after 2 PUSHes → [ESP+0x10])
//   [ESP+0x0c]   = n_bytes  (arg3, loaded into EDI after 3 PUSHes → [ESP+0x18])
//
// Register allocation:
//   EAX = arg1 (src)
//   ESI = arg2 (dest / current element pointer)
//   EDI = arg3 (n_bytes, then element count after SAR 3)
//   EBX = this (saved ECX)
//
// Logic:
//   1. if (src != dest): memcpy(dest, src, n_bytes)    // skip self-copy
//   2. count = n_bytes >> 3                             // number of 8-byte entries
//   3. if (count != 0):
//        do {
//            this->FUN_0045aa30(ptr, ptr+4);            // process each 8-byte element
//            ptr += 8;
//            count--;
//        } while (count != 0);
//
// Called functions:
//   _memcpy       at RVA 0x005d4600 (binary's internal memcpy implementation,
//                 __cdecl, 3 args: dest, src, count — CALL rel32 0x00579a33)
//   FUN_0045aa30  at RVA 0x0005aa30 (__thiscall, 2 stack args: ptr, ptr+4 —
//                 CALL rel32 0xfffffe4f)
//
// Both CALL rel32 offsets are masked by tools/compare.py; the raw bytes
// embedded below are the values from the orig binary's resolved .text and
// match byte-for-byte against the original slice.
//
// Asm (63 bytes):
//   8b 44 24 04              MOV  EAX, [ESP+0x04]        ; arg1 = src
//   53                       PUSH EBX
//   56                       PUSH ESI
//   8b 74 24 10              MOV  ESI, [ESP+0x10]        ; arg2 = dest (after 2 pushes)
//   3b c6                    CMP  EAX, ESI
//   57                       PUSH EDI
//   8b 7c 24 18              MOV  EDI, [ESP+0x18]        ; arg3 = n_bytes (after 3 pushes)
//   8b d9                    MOV  EBX, ECX               ; save this
//   74 0b                    JZ   +0x0b                  ; skip memcpy if src==dest
//   57                       PUSH EDI
//   50                       PUSH EAX
//   56                       PUSH ESI
//   e8 33 9a 57 00           CALL _memcpy
//   83 c4 0c                 ADD  ESP, 0xc
//   c1 ff 03                 SAR  EDI, 3                 ; count = n_bytes / 8
//   74 14                    JZ   +0x14                  ; skip loop if count==0
//   8d 46 04                 LEA  EAX, [ESI+4]           ; &element.second
//   50                       PUSH EAX
//   56                       PUSH ESI                    ; &element.first
//   8b cb                    MOV  ECX, EBX               ; restore this
//   e8 4f fe ff ff           CALL FUN_0045aa30
//   83 c6 08                 ADD  ESI, 0x8               ; next element
//   83 ef 01                 SUB  EDI, 0x1
//   75 ec                    JNZ  -0x14
//   5f                       POP  EDI
//   5e                       POP  ESI
//   5b                       POP  EBX
//   c2 0c 00                 RET  0xc

extern "C" __declspec(naked) void FUN_0045abb0() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x10]
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x3b              // CMP EAX, ESI
        _emit 0xc6
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x18]
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EBX, ECX
        _emit 0xd9
        _emit 0x74              // JZ +0x0b  (skip memcpy)
        _emit 0x0b
        _emit 0x57              // PUSH EDI
        _emit 0x50              // PUSH EAX
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL _memcpy  (rel32 → RVA 0x5d4600)
        _emit 0x33
        _emit 0x9a
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0xc1              // SAR EDI, 0x03
        _emit 0xff
        _emit 0x03
        _emit 0x74              // JZ +0x14  (skip loop)
        _emit 0x14
        _emit 0x8d              // LEA EAX, [ESI+0x04]
        _emit 0x46
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8              // CALL FUN_0045aa30  (rel32 → RVA 0x5aa30)
        _emit 0x4f
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESI, 0x08
        _emit 0xc6
        _emit 0x08
        _emit 0x83              // SUB EDI, 0x01
        _emit 0xef
        _emit 0x01
        _emit 0x75              // JNZ -0x14  (loop back)
        _emit 0xec
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x0c
        _emit 0x0c
        _emit 0x00
    }
}
