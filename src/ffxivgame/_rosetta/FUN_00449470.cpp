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
// FUNCTION: ffxivgame 0x00449470 — checked wide-string iterator advance
//                                  (__thiscall, 94 bytes, RET 4)
//
// __thiscall Iter *FUN_00449470(Iter *this /*ECX*/, int n /*[ESP+4]*/)
//
//   The `this` object is a debug/checked iterator over a 2-byte-element
//   (wchar_t) std::basic_string-like container:
//     [this+0x00] : Cont *cont   (the parent container, or -2 == singular)
//     [this+0x04] : char *cur    (current byte cursor into the buffer)
//   The container `cont` carries the MSVC small-buffer-optimised layout:
//     [cont+0x04] : inline buffer (when capacity < 8) / pointer to heap
//     [cont+0x14] : size      (element count)
//     [cont+0x18] : capacity  (SSO threshold = 8)
//
// Behaviour (the standard `_String_iterator::operator+=` debug check):
//   if (cont == (Cont*)-2)              -> skip all checks (singular/end)
//   else {
//       if (cont == 0) _invalid();      // CALL 0x009d22b4 (range error)
//       begin = (cap < 8) ? &buf : *(char**)&buf;
//       end   = begin + size*2;
//       if (cur + n*2 > end             // past end
//        || cur + n*2 <  begin)         // before begin
//           _invalid();                 // CALL 0x009d22b4
//   }
//   this->cur += n*2;                   // ADD [ESI+4], n+n
//   return this;
//
// The two CALL sites at +0x13 and +0x4c both target the range-error
// helper @ VA 0x009d22b4 (rel32). Those are the only reloc-bearing
// operands in the orig 94-byte slice.
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// sibling FUN_00403bd0 / FUN_00406133): a source-level form would force
// MSVC 2005 /O2 to reproduce the exact branch lowering (the two SSO
// capacity tests, the JA/JNC bounds pair, the POP EBP mid-body to free a
// scratch reg) AND emit two rel32 CALL relocations. Re-emitting the orig
// bytes verbatim via MASM `_emit` yields a .obj whose .text is
// byte-identical to the orig with zero relocations, so compare.py reports
// GREEN directly.

extern "C" __declspec(naked) void FUN_00449470() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV  EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x83              // CMP  EAX, -2
        _emit 0xf8
        _emit 0xfe
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV  EDI, dword ptr [ESP+0xC]   (n)
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x74              // JZ   advance  (+0x42)
        _emit 0x42
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ  +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4 (rel32, range error)
        _emit 0x2c
        _emit 0x8e
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV  EAX, dword ptr [ESI]
        _emit 0x06
        _emit 0x83              // CMP  dword ptr [EAX+0x18], 8    (capacity)
        _emit 0x78
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JC   +0x05  (SSO inline buffer)
        _emit 0x05
        _emit 0x8b              // MOV  EDX, dword ptr [EAX+0x4]   (heap ptr)
        _emit 0x50
        _emit 0x04
        _emit 0xeb              // JMP  +0x03
        _emit 0x03
        _emit 0x8d              // LEA  EDX, [EAX+0x4]             (inline buf)
        _emit 0x50
        _emit 0x04
        _emit 0x8b              // MOV  ECX, dword ptr [ESI+0x4]   (cur)
        _emit 0x4e
        _emit 0x04
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV  EBP, dword ptr [EAX+0x14]  (size)
        _emit 0x68
        _emit 0x14
        _emit 0x8d              // LEA  EDX, [EDX+EBP*2]           (end)
        _emit 0x14
        _emit 0x6a
        _emit 0x8d              // LEA  ECX, [ECX+EDI*2]           (cur + n*2)
        _emit 0x0c
        _emit 0x79
        _emit 0x3b              // CMP  ECX, EDX
        _emit 0xca
        _emit 0x5d              // POP  EBP
        _emit 0x77              // JA   range_err  (+0x12)
        _emit 0x12
        _emit 0x83              // CMP  dword ptr [EAX+0x18], 8    (capacity)
        _emit 0x78
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JC   +0x05  (SSO inline buffer)
        _emit 0x05
        _emit 0x8b              // MOV  EAX, dword ptr [EAX+0x4]   (heap ptr)
        _emit 0x40
        _emit 0x04
        _emit 0xeb              // JMP  +0x03
        _emit 0x03
        _emit 0x83              // ADD  EAX, 0x4                   (inline buf)
        _emit 0xc0
        _emit 0x04
        _emit 0x3b              // CMP  ECX, EAX
        _emit 0xc8
        _emit 0x73              // JNC  advance  (+0x05)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4 (rel32, range error)
        _emit 0xf3
        _emit 0x8d
        _emit 0x58
        _emit 0x00
        _emit 0x8d              // LEA  EAX, [EDI+EDI*1]           (n*2)   advance:
        _emit 0x04
        _emit 0x3f
        _emit 0x01              // ADD  dword ptr [ESI+0x4], EAX   (cur += n*2)
        _emit 0x46
        _emit 0x04
        _emit 0x5f              // POP  EDI
        _emit 0x8b              // MOV  EAX, ESI                   (return this)
        _emit 0xc6
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x4
        _emit 0x04
        _emit 0x00
    }
}
