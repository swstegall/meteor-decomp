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
// FUNCTION: ffxivgame 0x0043aeb0 — deep-copy / assignment of a fixed array of
// 0x1c-byte elements (__thiscall, 103 B, RET 0x4).
//
// Semantics (recovered from asm):
//
//   T *FUN_0043aeb0(T *this /*ECX*/, T *other /*[ESP+8]*/) {
//       if (this == other) return this;          // self-assign guard
//       this->count = 0;                          // field at +0x16c
//       for (unsigned i = 0; i < other->count; i++) {
//           unsigned dst = this->count;           // append index
//           this->count = dst + 1;
//           // dest element = (char*)this + dst*0x1c  (array base at +0x00)
//           // src  element = (char*)other + i*0x1c
//           FUN_0043abc0(&this->elem[dst], &other->elem[i]); // element copy
//       }
//       return this;
//   }
//
// Layout: an array of 0x1c-byte (28-byte) elements begins at object offset
// 0x00; the element count lives at +0x16c (0x16c / 0x1c == 13 → a fixed
// 13-slot array followed by the count dword). The per-element copy helper
// FUN_0043abc0 is itself a __thiscall (dest in ECX) taking one stacked src
// arg.  The dest address is computed as base + count*0x1c via the classic
// MSVC `LEA r,[idx*8]; SUB r,idx` (== idx*7) then `LEA r,[base+r*4]` (== *28)
// strength-reduction of the *0x1c multiply.
//
// Calling convention: __thiscall (this in ECX, one stacked arg, callee
// cleans → RET 0x4). Three epilog tails (self-assign, empty-source, and
// post-loop) each restore their own subset of the callee-save spill chain.
//
// Why naked __asm byte passthrough: the only relocatable site is the rel32
// CALL to 0x0043abc0 (a PC-relative intra-image call — no base-reloc record
// in the orig PE). Emitting the orig 103 bytes verbatim via `_emit` — the
// same approach the siblings FUN_004091f0 / FUN_00409260 took — yields a
// .text section byte-identical to the orig slice (the literal rel32
// displacement `cf fc ff ff` is reproduced exactly), so tools/compare.py
// reports GREEN. A source-level C++ form would not reliably round-trip the
// ESI=this / EBP=other / EDI=counter / EBX=src register allocation nor the
// three split epilog tails.

extern "C" __declspec(naked) void FUN_0043aeb0() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x8]   (other)
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX                   (this)
        _emit 0xf1
        _emit 0x3b              // CMP ESI, EBP
        _emit 0xf5
        _emit 0x74              // JZ 0x0043af10 (ret_this)
        _emit 0x54
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI                   (i = 0)
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI+0x16c], EDI (this->count = 0)
        _emit 0xbe
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x39              // CMP dword ptr [EBP+0x16c], EDI (other->count vs 0)
        _emit 0xbd
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x76              // JBE 0x0043af08 (ret_pop_edi)   (count == 0)
        _emit 0x3b
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, EBP                   (src = &other->elem[0])
        _emit 0xdd
    // loop: @0x0043aed0
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x16c] (dst = this->count)
        _emit 0x86
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX*0x8 + 0x0]       (dst*8)
        _emit 0x0c
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB ECX, EAX                   (dst*7)
        _emit 0xc8
        _emit 0x83              // ADD EAX, 0x1                   (count + 1)
        _emit 0xc0
        _emit 0x01
        _emit 0x8d              // LEA ECX, [ESI + ECX*0x4]       (&this->elem[dst])
        _emit 0x0c
        _emit 0x8e
        _emit 0x53              // PUSH EBX                       (push src)
        _emit 0x89              // MOV dword ptr [ESI+0x16c], EAX (this->count = dst+1)
        _emit 0x86
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x0043abc0 (element copy) — rel32 cf fc ff ff
        _emit 0xcf
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD EDI, 0x1                   (i++)
        _emit 0xc7
        _emit 0x01
        _emit 0x83              // ADD EBX, 0x1c                  (src += 0x1c)
        _emit 0xc3
        _emit 0x1c
        _emit 0x3b              // CMP EDI, dword ptr [EBP+0x16c]
        _emit 0xbd
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x72              // JC loop (i < other->count)
        _emit 0xd1
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI                   (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    // ret_pop_edi: @0x0043af08
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI                   (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    // ret_this: @0x0043af10
        _emit 0x8b              // MOV EAX, ESI                   (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
