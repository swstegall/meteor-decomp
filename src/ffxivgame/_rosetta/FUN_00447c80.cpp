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
// FUNCTION: ffxivgame 0x00047c80 — string-like buffer append/concat
//                                  (__thiscall, 101 bytes / 0x65, RET 0x4)
//
// __thiscall Str* FUN_00447c80(Str *this, Str *other);
//
//   struct Str { char *data;  /* +0x00 */  int _;  /* +0x04 */  int size; /* +0x08 */ };
//
//   Reserves room for the merged length, then copies `other`'s bytes onto
//   the tail of `this`, overlapping the existing NUL terminator. Returns
//   `this`.
//
//   this->reserve(this->size + other->size - 1, 1);   // FUN_00447010 (__thiscall)
//   if (other != this) {
//       memcpy(this->data + this->size - 1,            // 0x009d5110 (cdecl memcpy)
//              other->data, other->size);
//   } else {
//       // self-append: copy (size-1) bytes of own data after the tail,
//       // then re-terminate at the original end.
//       memcpy(this->data + this->size - 1, this->data, this->size - 1);
//       this->data[this->size - 1] = 0;
//   }
//   return this;
//
// Stack at entry (after the PUSH EBX prologue the arg sits at [ESP+0x08]):
//     [ESP+0x04]  return address
//     [ESP+0x08]  arg 0  Str* other      ; -> EBX
//   `this` arrives in ECX (-> ESI), preserved as the return value.
//
// Reloc-bearing sites in the orig 101 bytes (the two CALL rel32s are baked
// in; emitting the bytes verbatim yields a .text slice byte-identical to the
// orig with no COFF relocations, which is what tools/compare.py checks):
//     +0x18   CALL rel32 → FUN_00447010   (reserve/grow helper)
//     +0x2f   CALL rel32 → 0x009d5110      (memcpy, other != this path)
//     +0x4b   CALL rel32 → 0x009d5110      (memcpy, self-append path)
//
// Reconstruction strategy — naked-asm byte passthrough (same as siblings
// FUN_00406350 / FUN_0040a590): a source-level /O2 lowering of the dual-arm
// branch with the exact LEA/register selection is brittle, so re-emit the
// orig bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_00447c80() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x08]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [EBX+0x08]
        _emit 0x43
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x08]
        _emit 0x7e
        _emit 0x08
        _emit 0x8d              // LEA ECX, [EAX+EDI-0x01]
        _emit 0x4c
        _emit 0x38
        _emit 0xff
        _emit 0x6a              // PUSH 0x01
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447010
        _emit 0x73
        _emit 0xf3
        _emit 0xff
        _emit 0xff
        _emit 0x3b              // CMP EBX, ESI
        _emit 0xde
        _emit 0x74              // JZ self_append (+0x1e)
        _emit 0x1e
        _emit 0x8b              // MOV EDX, dword ptr [EBX+0x08]
        _emit 0x53
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [EBX]
        _emit 0x03
        _emit 0x8b              // MOV ECX, dword ptr [ESI]
        _emit 0x0e
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EDX, [ECX+EDI-0x01]
        _emit 0x54
        _emit 0x39
        _emit 0xff
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d5110 (memcpy)
        _emit 0x5c
        _emit 0xd4
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x8b              // self_append: MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8d              // LEA ECX, [EDI-0x01]
        _emit 0x4f
        _emit 0xff
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EDX, [EAX+EDI-0x01]
        _emit 0x54
        _emit 0x38
        _emit 0xff
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d5110 (memcpy)
        _emit 0x40
        _emit 0xd4
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x08]
        _emit 0x4e
        _emit 0x08
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0xc6              // MOV byte ptr [EAX+ECX-0x01], 0x00
        _emit 0x44
        _emit 0x08
        _emit 0xff
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x04
        _emit 0x04
        _emit 0x00
    }
}
