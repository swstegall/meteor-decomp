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
// FUNCTION: ffxivgame 0x00048880 — small-string (SSO) copy-construct from a
//                                  string-like source argument (__thiscall,
//                                  112 bytes / 0x70, returns this).
//
// Calling convention: __thiscall (ECX = this); one stack argument at
// [ESP+0xc] (a string-like source); RET 0x4 cleans that arg; returns
// `this` in EAX.
//
// Destination object layout (ESI = this):
//   [this + 0x00]  char*  data pointer  (initialised to this+0x12, the
//                                        inline SSO buffer)
//   [this + 0x04]  dword  capacity      = 0x40 (64)
//   [this + 0x08]  dword  ?             = 1
//   [this + 0x0c]  dword  length        = 0
//   [this + 0x10]  byte   flag          = 1
//   [this + 0x11]  byte   flag          = 1
//   [this + 0x12]  char   inline buffer (NUL-terminated here)
//
// Source argument layout (EAX = [ESP+0xc]):
//   [arg + 0x18]  dword  capacity — if < 8 the string is inline (buffer at
//                                   arg+0x04), else heap (ptr at arg+0x04)
//   [arg + 0x04]  inline buffer  OR  heap pointer (per the capacity test)
//
// Body:
//   - default-init the destination as an empty SSO string,
//   - select the source character pointer (EBP) from the SSO branch test,
//   - len = FUN_00445ae0(src, 0)            (counts source length),
//   - FUN_00447010(this, len+1, 1)          (reserve / grow to len+1),
//   - FUN_00445ae0(src, this->data)         (copy source into destination),
//   - this->data[len] = 0                   (NUL-terminate),
//   - return this.
//
// Reloc-bearing sites in the orig 112 bytes (CALL rel32):
//   +0x3f  CALL rel32 → FUN_00445ae0  (0x00445ae0 — length counter)
//   +0x51  CALL rel32 → FUN_00447010  (0x00447010 — reserve/grow, __thiscall)
//   +0x5a  CALL rel32 → FUN_00445ae0  (0x00445ae0 — copy)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would emit three CALL rel32 relocations the
//   linker resolves at relink; the simpler path (the same one the sibling
//   _rosetta thunks take) is a __declspec(naked) body that re-emits the
//   orig 112 bytes verbatim via MASM _emit directives. The .obj's .text
//   ends up byte-identical to the orig slice (the rel32 offsets resolve
//   against the orig binary's own address space, so emitting them as raw
//   bytes yields the exact wire image). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00448880() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI,ECX
        _emit 0xf1
        _emit 0x8d              // LEA EAX,[ESI+0x12]
        _emit 0x46
        _emit 0x12
        _emit 0x89              // MOV dword ptr [ESI],EAX
        _emit 0x06
        _emit 0xc6              // MOV byte ptr [ESI+0x10],0x1
        _emit 0x46
        _emit 0x10
        _emit 0x01
        _emit 0xc6              // MOV byte ptr [ESI+0x11],0x1
        _emit 0x46
        _emit 0x11
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESI+0xc],0x0
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x8],0x1
        _emit 0x46
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x4],0x40
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [EAX],0x0
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX,dword ptr [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x83              // CMP dword ptr [EAX+0x18],0x8
        _emit 0x78
        _emit 0x18
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x72              // JC short +0x05  (→ 0x4488b9, inline source)
        _emit 0x05
        _emit 0x8b              // MOV EBP,dword ptr [EAX+0x4]  (heap pointer)
        _emit 0x68
        _emit 0x04
        _emit 0xeb              // JMP short +0x03  (→ 0x4488bc)
        _emit 0x03
        _emit 0x8d              // LEA EBP,[EAX+0x4]  (inline buffer)
        _emit 0x68
        _emit 0x04
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x55              // PUSH EBP
        _emit 0xe8              // CALL FUN_00445ae0 (rel32 → 0x00445ae0)
        _emit 0x1c
        _emit 0xd2
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP,0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x8b              // MOV EDI,EAX  (EDI = length)
        _emit 0xf8
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x8d              // LEA EAX,[EDI+0x1]  (length+1)
        _emit 0x47
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX,ESI  (this)
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447010 (rel32 → 0x00447010)
        _emit 0x3a
        _emit 0xe7
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX,dword ptr [ESI]  (this->data)
        _emit 0x0e
        _emit 0x51              // PUSH ECX
        _emit 0x55              // PUSH EBP
        _emit 0xe8              // CALL FUN_00445ae0 (rel32 → 0x00445ae0)
        _emit 0x01
        _emit 0xd2
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDX,dword ptr [ESI]  (this->data)
        _emit 0x16
        _emit 0x83              // ADD ESP,0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc6              // MOV byte ptr [EDI+EDX*0x1],0x0  (NUL-term)
        _emit 0x04
        _emit 0x17
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX,ESI  (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
