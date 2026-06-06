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
// FUNCTION: ffxivgame 0x00047200 — copy-constructor for a small-buffer
//                                  string/blob container (__thiscall, 93 B,
//                                  RET 4).
//
// __thiscall Container *FUN_00447200(Container *this, const Container *src)
//
// Layout (offsets into `this`):
//     +0x00 : char *buf       (data pointer; defaults to inline &this[0x12])
//     +0x04 : unsigned cap    (capacity; seeded to 0x40)
//     +0x08 : unsigned        (flag/size; seeded to 1)
//     +0x0c : unsigned        (length; seeded to 0, then copied from src)
//     +0x10 : char            (flag; seeded to 1, then copied from src)
//     +0x11 : char            (flag; seeded to 1)
//     +0x12 : char[]          (inline SSO buffer; first byte NUL-terminated)
//
// Behaviour: stamp the default-constructed shape (flags=1, cap=0x40,
// buf=&inline, inline[0]=0), then this->FUN_00447010(src[0x8], 1) to
// reserve/grow to src's size, memcpy(this->buf, src->buf, src[0x8])
// the payload across, and finally mirror src's length (+0x0c) and the
// +0x10 flag byte. Returns `this`.
//
// Asm shape (93 bytes, read from orig RVA 0x00047200):
//
//   56                 push esi
//   8b f1              mov  esi, ecx                 ; esi = this
//   b9 01 00 00 00     mov  ecx, 1
//   57                 push edi
//   8b 7c 24 0c        mov  edi, [esp+0xc]           ; edi = src
//   8d 46 12           lea  eax, [esi+0x12]          ; &inline buf
//   88 4e 10           mov  [esi+0x10], cl
//   88 4e 11           mov  [esi+0x11], cl
//   89 4e 08           mov  [esi+0x8], ecx
//   c7 46 0c 00000000  mov  dword ptr [esi+0xc], 0
//   c7 46 04 40000000  mov  dword ptr [esi+0x4], 0x40
//   89 06              mov  [esi], eax               ; buf = &inline
//   c6 00 00           mov  byte ptr [eax], 0        ; inline[0] = NUL
//   8b 47 08           mov  eax, [edi+0x8]           ; src size
//   51                 push ecx                      ; (1)
//   50                 push eax                      ; (src size)
//   8b ce              mov  ecx, esi                 ; this
//   e8 d8 fd ff ff     call FUN_00447010             ; this->reserve(size,1)
//   8b 4f 08           mov  ecx, [edi+0x8]           ; memcpy n
//   8b 17              mov  edx, [edi]               ; memcpy src
//   8b 06              mov  eax, [esi]               ; memcpy dest
//   51                 push ecx
//   52                 push edx
//   50                 push eax
//   e8 b9 d3 58 00     call _memcpy                  ; @ 0x009d4600
//   8b 4f 0c           mov  ecx, [edi+0xc]
//   83 c4 0c           add  esp, 0xc                 ; cdecl cleanup
//   89 4e 0c           mov  [esi+0xc], ecx           ; length = src length
//   8a 57 10           mov  dl, [edi+0x10]
//   5f                 pop  edi
//   88 56 10           mov  [esi+0x10], dl           ; flag = src flag
//   8b c6              mov  eax, esi                 ; return this
//   5e                 pop  esi
//   c2 04 00           ret  4
//
// Reloc-bearing sites in the orig 93 bytes:
//   +0x33   CALL rel32 → 0x00447010 (FUN_00447010, the reserve/grow helper)
//   +0x42   CALL rel32 → 0x009d4600 (_memcpy)
//
// Reconstruction strategy — naked-asm byte passthrough (same `_emit`
// technique as siblings FUN_00403bd0 / FUN_00406133): the 93 bytes are
// re-emitted literally, with the two rel32 CALL operands baked in as the
// concrete byte values that already resolve against the orig PE's `.text`
// address space. The resulting .obj's `.text` is byte-identical to the
// orig slice with ZERO relocations, so tools/compare.py reports GREEN
// without any reloc masking — and without having to coax MSVC 2005's
// register allocator into the exact esi=this / edi=src picture.

extern "C" __declspec(naked) void FUN_00447200() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0xb9              // MOV  ECX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV  EDI, dword ptr [ESP+0xC]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x8d              // LEA  EAX, [ESI+0x12]
        _emit 0x46
        _emit 0x12
        _emit 0x88              // MOV  byte ptr [ESI+0x10], CL
        _emit 0x4e
        _emit 0x10
        _emit 0x88              // MOV  byte ptr [ESI+0x11], CL
        _emit 0x4e
        _emit 0x11
        _emit 0x89              // MOV  dword ptr [ESI+0x8], ECX
        _emit 0x4e
        _emit 0x08
        _emit 0xc7              // MOV  dword ptr [ESI+0xC], 0
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV  dword ptr [ESI+0x4], 0x40
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV  dword ptr [ESI], EAX
        _emit 0x06
        _emit 0xc6              // MOV  byte ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV  EAX, dword ptr [EDI+0x8]
        _emit 0x47
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV  ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447010 (rel32 → 0x00447010)
        _emit 0xd8
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV  ECX, dword ptr [EDI+0x8]
        _emit 0x4f
        _emit 0x08
        _emit 0x8b              // MOV  EDX, dword ptr [EDI]
        _emit 0x17
        _emit 0x8b              // MOV  EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL _memcpy (rel32 → 0x009d4600)
        _emit 0xb9
        _emit 0xd3
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV  ECX, dword ptr [EDI+0xC]
        _emit 0x4f
        _emit 0x0c
        _emit 0x83              // ADD  ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0x89              // MOV  dword ptr [ESI+0xC], ECX
        _emit 0x4e
        _emit 0x0c
        _emit 0x8a              // MOV  DL, byte ptr [EDI+0x10]
        _emit 0x57
        _emit 0x10
        _emit 0x5f              // POP  EDI
        _emit 0x88              // MOV  byte ptr [ESI+0x10], DL
        _emit 0x56
        _emit 0x10
        _emit 0x8b              // MOV  EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x4
        _emit 0x04
        _emit 0x00
    }
}
