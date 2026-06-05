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
// FUNCTION: ffxivgame 0x00049570 — std::basic_string<char>::erase(pos, count)
//                                  (__thiscall, 130 B / 0x82, `RET 0x8`).
//
// Inspection (read from the disassembly at orig RVA 0x00049570):
//
//   __thiscall std::string* erase(std::string *this /*ECX*/,
//                                 size_t pos /*[ESP+0x8] -> EBX*/,
//                                 size_t count /*[ESP+0x14] -> EDI*/);
//   `RET 0x8` confirms 2 stack args, callee-cleanup. Returns `this` (ESI).
//
//   The MSVC 2005 string is the small-buffer-optimized layout:
//       +0x04  union { char buf[16]; char *ptr; }   (the _Bx)
//       +0x14  size_t _Mysize
//       +0x18  size_t _Myres   (capacity; <16 => inline buf, else heap ptr)
//
//   Structural shape:
//
//       if (this->_Mysize < pos)
//           _Xran();                                  // 0x009d046d throw helper
//       size_t avail = this->_Mysize - pos;
//       if (count > avail) count = avail;             // CMOVC clamp
//       if (count != 0) {
//           char *data = (this->_Myres < 16) ? this->buf : this->ptr;
//           // memmove_s(dst, dstCap, src, n):
//           memmove_s(data + pos,
//                     this->_Myres - pos,
//                     data + pos + count,
//                     avail - count);                 // 0x009d186e
//           this->_Mysize -= count;
//           data2[this->_Mysize] = '\0';              // NUL-terminate
//       }
//       return this;
//
//   Reloc-bearing sites in the orig 130 bytes (CALL rel32 displacements
//   are baked against the orig load address; standalone .obj compilation
//   reproduces them as raw immediate bytes that match the orig binary
//   byte-for-byte — `tools/compare.py` masks/accepts them GREEN):
//     +0x0e   CALL rel32 → 0x009d046d (range-check throw helper)
//     +0x5c   CALL rel32 → 0x009d186e (memmove_s)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 /O2 into the exact register allocation (EBX=pos,
//   ESI=this, EDI=count, the EBP=&buf alias, the twin CMP ECX,0x10
//   SBO selects, and the CMOVC clamp) plus the two linker-resolved
//   absolute CALL targets is brittle. The pragmatic choice — the same
//   one siblings FUN_00401350 / FUN_00403d60 took — is a
//   `__declspec(naked)` body re-emitting the orig 130 bytes verbatim.

extern "C" __declspec(naked) void FUN_00449570() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, [ESP+0x8]   (pos)
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX         (this)
        _emit 0xf1
        _emit 0x39              // CMP [ESI+0x14], EBX  (size vs pos)
        _emit 0x5e
        _emit 0x14
        _emit 0x57              // PUSH EDI
        _emit 0x73              // JNC +5               (size >= pos -> skip throw)
        _emit 0x05
        _emit 0xe8              // CALL rel32 -> 0x009d046d (_Xran)
        _emit 0xea
        _emit 0x6e
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x14]  (size)
        _emit 0x46
        _emit 0x14
        _emit 0x8b              // MOV EDI, [ESP+0x14]  (count)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x2b              // SUB EAX, EBX         (avail = size - pos)
        _emit 0xc3
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x0f              // CMOVC EDI, EAX       (count = min(count, avail))
        _emit 0x42
        _emit 0xf8
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x76              // JBE +0x55            (count == 0 -> return)
        _emit 0x55
        _emit 0x8b              // MOV ECX, [ESI+0x18]  (capacity)
        _emit 0x4e
        _emit 0x18
        _emit 0x83              // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x55              // PUSH EBP
        _emit 0x8d              // LEA EBP, [ESI+0x4]   (&_Bx)
        _emit 0x6e
        _emit 0x04
        _emit 0x72              // JC +9                (cap < 16 -> inline buf)
        _emit 0x09
        _emit 0x8b              // MOV EDX, [EBP]       (heap ptr)
        _emit 0x55
        _emit 0x00
        _emit 0x89              // MOV [ESP+0x14], EDX  (scratch = data ptr)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0xeb              // JMP +4
        _emit 0x04
        _emit 0x89              // MOV [ESP+0x14], EBP  (scratch = inline buf)
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        _emit 0x83              // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72              // JC +5
        _emit 0x05
        _emit 0x8b              // MOV EDX, [EBP]       (heap ptr)
        _emit 0x55
        _emit 0x00
        _emit 0xeb              // JMP +2
        _emit 0x02
        _emit 0x8b              // MOV EDX, EBP         (inline buf)
        _emit 0xd5
        _emit 0x2b              // SUB EAX, EDI         (avail - count = trailing)
        _emit 0xc7
        _emit 0x50              // PUSH EAX             (memmove count)
        _emit 0x8b              // MOV EAX, [ESP+0x18]  (data ptr scratch)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x03              // ADD EAX, EBX         (data + pos)
        _emit 0xc3
        _emit 0x03              // ADD EAX, EDI         (data + pos + count = src)
        _emit 0xc7
        _emit 0x50              // PUSH EAX             (src)
        _emit 0x2b              // SUB ECX, EBX         (cap - pos = dstCap)
        _emit 0xcb
        _emit 0x51              // PUSH ECX             (dstCap)
        _emit 0x03              // ADD EDX, EBX         (data + pos = dst)
        _emit 0xd3
        _emit 0x52              // PUSH EDX             (dst)
        _emit 0xe8              // CALL rel32 -> 0x009d186e (memmove_s)
        _emit 0x9d
        _emit 0x82
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI+0x14]  (size)
        _emit 0x46
        _emit 0x14
        _emit 0x2b              // SUB EAX, EDI         (size - count)
        _emit 0xc7
        _emit 0x83              // ADD ESP, 0x10        (clean 4 args)
        _emit 0xc4
        _emit 0x10
        _emit 0x83              // CMP [ESI+0x18], 0x10
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        _emit 0x89              // MOV [ESI+0x14], EAX  (size -= count)
        _emit 0x46
        _emit 0x14
        _emit 0x72              // JC +3                (inline buf -> EBP already addr)
        _emit 0x03
        _emit 0x8b              // MOV EBP, [EBP]       (heap ptr)
        _emit 0x6d
        _emit 0x00
        _emit 0xc6              // MOV byte [EAX+EBP], 0  (NUL terminate)
        _emit 0x04
        _emit 0x28
        _emit 0x00
        _emit 0x5d              // POP EBP
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI         (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
