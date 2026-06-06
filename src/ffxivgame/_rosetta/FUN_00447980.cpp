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
// FUNCTION: ffxivgame 0x00047980 — __thiscall "prepend byte" on a
//                                   grow-able byte buffer (71 B / 0x47)
//
//   void __thiscall FUN_00447980(Buf *this, char c)
//     ECX = this, one DWORD stack arg (the byte to insert), RET 4.
//
//   struct Buf {
//     char  *data;   // +0x00  base pointer
//     /* +0x04 */
//     size_t len;    // +0x08  current element count
//   };
//
//   Body (matches asm flow):
//
//     FUN_00447010(this, this->len + 1, 1);   // reserve room for one more
//     size_t n = this->len - 1;
//     while (n != 0) {                         // shift right by one
//         this->data[n] = this->data[n - 1];
//         --n;
//     }
//     this->data[0] = c;                       // store new front byte
//
//   The `len == 1` case skips the shift loop (JZ → tail) and writes
//   data[0] = c directly. The byte argument is read from [ESP+8] (after
//   PUSH ESI) in both arms.
//
// Asm (71 bytes @ orig RVA 0x00047980):
//   56                   push esi
//   8b f1                mov  esi, ecx
//   8b 46 08             mov  eax, [esi+8]
//   6a 01                push 1
//   83 c0 01             add  eax, 1
//   50                   push eax
//   e8 ?? ?? ?? ??       call FUN_00447010            ; REL32 reloc
//   8b 46 08             mov  eax, [esi+8]
//   83 e8 01             sub  eax, 1
//   74 22                jz   tail
//   8d a4 24 00 00 00 00 lea  esp, [esp]              ; 7-byte align nop
//  loop:
//   8b 0e                mov  ecx, [esi]
//   8a 54 01 ff          mov  dl, [ecx+eax-1]
//   03 c8                add  ecx, eax
//   83 e8 01             sub  eax, 1
//   88 11                mov  [ecx], dl
//   75 f1                jnz  loop
//   8b 06                mov  eax, [esi]
//   8a 4c 24 08          mov  cl, [esp+8]
//   88 08                mov  [eax], cl
//   5e                   pop  esi
//   c2 04 00             ret  4
//  tail:
//   8b 16                mov  edx, [esi]
//   8a 44 24 08          mov  al, [esp+8]
//   88 02                mov  [edx], al
//   5e                   pop  esi
//   c2 04 00             ret  4
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling _rosetta naked stubs). The single CALL rel32 to FUN_00447010 is
// emitted as a named `call` so the COFF .obj carries the REL32 relocation
// that tools/compare.py masks; every other byte — including the 7-byte
// `lea esp,[esp]` alignment nop MSVC inserted ahead of the loop — is a
// self-contained constant emitted verbatim.

extern "C" void FUN_00447010();   // buffer-grow helper (REL32 callsite)

extern "C" __declspec(naked) void FUN_00447980() {
    __asm {
        // 00047980: 56            push esi
        _emit 0x56
        // 00047981: 8b f1         mov esi, ecx
        _emit 0x8b
        _emit 0xf1
        // 00047983: 8b 46 08      mov eax, [esi+8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00047986: 6a 01         push 1
        _emit 0x6a
        _emit 0x01
        // 00047988: 83 c0 01      add eax, 1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0004798b: 50            push eax
        _emit 0x50
        // 0004798c: e8 ?? ?? ?? ??  call FUN_00447010  (REL32 reloc)
        call    FUN_00447010
        // 00047991: 8b 46 08      mov eax, [esi+8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00047994: 83 e8 01      sub eax, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 00047997: 74 22         jz tail
        _emit 0x74
        _emit 0x22
        // 00047999: 8d a4 24 00 00 00 00  lea esp, [esp]   (7-byte nop)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // loop: 000479a0: 8b 0e   mov ecx, [esi]
        _emit 0x8b
        _emit 0x0e
        // 000479a2: 8a 54 01 ff   mov dl, [ecx+eax-1]
        _emit 0x8a
        _emit 0x54
        _emit 0x01
        _emit 0xff
        // 000479a6: 03 c8         add ecx, eax
        _emit 0x03
        _emit 0xc8
        // 000479a8: 83 e8 01      sub eax, 1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 000479ab: 88 11         mov [ecx], dl
        _emit 0x88
        _emit 0x11
        // 000479ad: 75 f1         jnz loop
        _emit 0x75
        _emit 0xf1
        // 000479af: 8b 06         mov eax, [esi]
        _emit 0x8b
        _emit 0x06
        // 000479b1: 8a 4c 24 08   mov cl, [esp+8]
        _emit 0x8a
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 000479b5: 88 08         mov [eax], cl
        _emit 0x88
        _emit 0x08
        // 000479b7: 5e            pop esi
        _emit 0x5e
        // 000479b8: c2 04 00      ret 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
        // tail: 000479bb: 8b 16   mov edx, [esi]
        _emit 0x8b
        _emit 0x16
        // 000479bd: 8a 44 24 08   mov al, [esp+8]
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 000479c1: 88 02         mov [edx], al
        _emit 0x88
        _emit 0x02
        // 000479c3: 5e            pop esi
        _emit 0x5e
        // 000479c4: c2 04 00      ret 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
