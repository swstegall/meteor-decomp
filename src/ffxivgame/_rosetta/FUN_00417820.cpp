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
// FUNCTION: ffxivgame 0x00417820 — __thiscall vector-of-8B-elements destructor
//                                   (109 bytes / 0x6d).
//
// void __thiscall FUN_00417820(this)
//   this (ECX) → struct { void* unk0; T* begin; T* end; T* cap_end; }
//
// Semantics:
//   1. Compute count = (end - begin) >> 3  (8-byte elements, or 0 if begin==NULL)
//   2. For each i in [0, count):
//        - Inline bounds-check: if begin==NULL || i>=count → call 0x9d22b4
//        - Load element = begin[i*8]  (first DWORD of 8-byte slot)
//        - Push element and call 0x9d1be9 (per-element destructor)
//        - ADD EDI,1 (index++) then ADD ESP,4 (caller-cleans __cdecl call)
//   3. If begin != NULL:
//        - ECX = *(begin − 4)  (allocator header / block descriptor)
//        - Push begin, call FUN_0040df70 (buffer free, __thiscall on allocator)
//        - POP EDI
//   4. begin = end = 0  (zeroed here; cap_end zeroed by fall-through to next fn)
//
// NOTE: this function has NO epilogue — it falls through directly into the
// next function at RVA 0x1788d (FUN_0041788d), which zeros [ESI+0xc] and
// executes POP ESI / POP EBX / RET.  This is a standard MSVC 2005 shared-
// epilogue optimisation.  The .obj must therefore contain exactly these
// 109 bytes with no terminating ret.
//
// Byte map (109 bytes, RVA 0x17820 … 0x1788c inclusive):
//
//   +0x00  53 56 8b f1 8b 46 04 85 c0 57 75 04 33 db eb 08
//   +0x10  8b 5e 08 2b d8 c1 fb 03 33 ff 85 db 76 30 8b ff
//   +0x20  8b 4e 04 85 c9 74 0c 8b 46 08 2b c1 c1 f8 03 3b
//   +0x30  f8 72 05 e8 5c aa 5b 00 8b 46 04 8b 0c f8 51 e8
//   +0x40  85 a3 5b 00 83 c7 01 83 c4 04 3b fb 72 d2 8b 46
//   +0x50  04 85 c0 74 09 8b 48 fc 50 e8 f2 66 ff ff 5f c7
//   +0x60  46 04 00 00 00 00 c7 46 08 00 00 00 00
//
// Reloc-bearing sites (rel32 CALL targets baked into the orig binary):
//   +0x33  CALL rel32 → 0x9d22b4  (bounds-fail handler / __range_check)
//   +0x40  CALL rel32 → 0x9d1be9  (per-element destructor)
//   +0x59  CALL rel32 → 0x40df70  (buffer free, FUN_0040df70)

extern "C" __declspec(naked) void FUN_00417820() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x57              // PUSH EDI
        _emit 0x75              // JNZ +0x4
        _emit 0x04
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0xeb              // JMP +0x8
        _emit 0x08
        _emit 0x8b              // MOV EBX, dword ptr [ESI+0x8]
        _emit 0x5e
        _emit 0x08
        _emit 0x2b              // SUB EBX, EAX
        _emit 0xd8
        _emit 0xc1              // SAR EBX, 0x3
        _emit 0xfb
        _emit 0x03
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x76              // JBE +0x30
        _emit 0x30
        _emit 0x8b              // MOV EDI, EDI  (NOP / align)
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x4]   (loop:)
        _emit 0x4e
        _emit 0x04
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ +0xc
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x2b              // SUB EAX, ECX
        _emit 0xc1
        _emit 0xc1              // SAR EAX, 0x3
        _emit 0xf8
        _emit 0x03
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0x72              // JB +0x5
        _emit 0x05
        _emit 0xe8              // CALL 0x9d22b4  (rel32 = 0x5baa5c, bounds-fail)
        _emit 0x5c
        _emit 0xaa
        _emit 0x5b
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [EAX+EDI*8]
        _emit 0x0c
        _emit 0xf8
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL 0x9d1be9  (rel32 = 0x5ba385, per-elem dtor)
        _emit 0x85
        _emit 0xa3
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD EDI, 0x1
        _emit 0xc7
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x3b              // CMP EDI, EBX
        _emit 0xfb
        _emit 0x72              // JB -0x2e  (back to loop)
        _emit 0xd2
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x9
        _emit 0x09
        _emit 0x8b              // MOV ECX, dword ptr [EAX-0x4]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x40df70  (rel32 = 0xffff66f2, buffer free)
        _emit 0xf2
        _emit 0x66
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0xc7              // MOV dword ptr [ESI+0x4], 0x0
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x8], 0x0
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // falls through into FUN_0041788d (no ret here)
    }
}
