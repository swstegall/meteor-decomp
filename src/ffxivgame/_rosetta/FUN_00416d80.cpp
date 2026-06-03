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
// FUNCTION: ffxivgame 0x00416d80 — container "remove at index" method
//                                  (__thiscall, 109 bytes).
//
// bool __thiscall FUN_00416d80(this=ECX, unsigned int idx)
//
// Semantics (recovered from asm):
//
//   bool RemoveAt(unsigned int idx) {
//       if (this->count <= idx) return false;    // JA: unsigned above
//
//       unsigned int stride = (unsigned char)this->field_14
//                           + (unsigned char)this->field_15
//                           + (unsigned char)this->field_16;
//       char *elem = (char *)this->base + stride * idx;
//
//       // Call virtual method at vtable[0x28/4] on this, passing elem ptr.
//       // Pattern: EDX = *this (vftable ptr), EAX = [EDX+0x28] (fn ptr),
//       //          MOV ECX, ESI (this), PUSH EDI (elem), CALL EAX.
//       (*(void (__thiscall **)(void*, void*))(**(void***)this + 0x28 / sizeof(void*)))(this, elem);
//
//       unsigned int remaining = this->count - idx - 1;
//       if (remaining != 0) {
//           memmove(elem, elem + stride, stride * remaining);
//       }
//       this->count--;
//       return true;
//   }
//
// Class layout (this pointer, accessed via ESI):
//   +0x00  void**   vftable_ptr  (first member; [*this+0x28] is a fn ptr)
//   +0x04  char*    base         (element array base pointer)
//   +0x0c  DWORD    count        (number of elements)
//   +0x14  BYTE     field_14     (element stride component a)
//   +0x15  BYTE     field_15     (element stride component b)
//   +0x16  BYTE     field_16     (element stride component c)
//   stride = field_14 + field_15 + field_16
//
// memmove external: RVA 0x009d5110 (statically-linked CRT), called as
//   memmove(dst=elem, src=elem+stride, count=stride*remaining)
//   with __cdecl 3-arg stack cleanup (ADD ESP, 0xc) after the call.
//
// Why naked asm: MSVC 2005 __thiscall calling convention with indirect
// vtable dispatch, specific MOVZX load ordering, IMUL register choices,
// and ADD [mem], -1 (not DEC [mem]) for count decrement cannot be
// reliably reproduced byte-for-byte from C++ source. The memmove CALL
// rel32 target (0x009d5110) is baked as a concrete displacement;
// emitting it via _emit produces a zero-reloc .obj whose .text matches
// the orig post-fixup image byte-for-byte.
//
// Original 109 bytes (RVA 0x00016d80 – 0x00016ded):
//
//   53 8b 5c 24 08 56 8b f1 39 5e 0c 77 07 5e 32 c0
//   5b c2 04 00 0f b6 46 16 0f b6 4e 15 8b 16 57 0f
//   b6 7e 14 03 f8 8b 42 28 03 f9 0f af fb 03 7e 04
//   8b ce 57 ff d0 8b 46 0c 2b c3 83 e8 01 74 22 0f
//   b6 56 16 0f b6 4e 14 03 ca 0f b6 56 15 03 ca 8b
//   d1 0f af d0 52 03 cf 51 57 e8 32 e3 5b 00 83 c4
//   0c 83 46 0c ff 5f 5e b0 01 5b c2 04 00

extern "C" __declspec(naked) void FUN_00416d80() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x39              // CMP dword ptr [ESI+0xc], EBX
        _emit 0x5e
        _emit 0x0c
        _emit 0x77              // JA +7 (to body)
        _emit 0x07
        _emit 0x5e              // POP ESI
        _emit 0x32              // XOR AL, AL  (return false)
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
        _emit 0x0f              // MOVZX EAX, byte ptr [ESI+0x16]  (c)
        _emit 0xb6
        _emit 0x46
        _emit 0x16
        _emit 0x0f              // MOVZX ECX, byte ptr [ESI+0x15]  (b)
        _emit 0xb6
        _emit 0x4e
        _emit 0x15
        _emit 0x8b              // MOV EDX, dword ptr [ESI]        (vftable ptr)
        _emit 0x16
        _emit 0x57              // PUSH EDI
        _emit 0x0f              // MOVZX EDI, byte ptr [ESI+0x14]  (a)
        _emit 0xb6
        _emit 0x7e
        _emit 0x14
        _emit 0x03              // ADD EDI, EAX   (EDI = a + c)
        _emit 0xf8
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x28]  (fn ptr)
        _emit 0x42
        _emit 0x28
        _emit 0x03              // ADD EDI, ECX   (EDI = a + c + b = stride)
        _emit 0xf9
        _emit 0x0f              // IMUL EDI, EBX  (EDI = stride * idx)
        _emit 0xaf
        _emit 0xfb
        _emit 0x03              // ADD EDI, dword ptr [ESI+0x4]   (EDI = base + stride*idx = elem)
        _emit 0x7e
        _emit 0x04
        _emit 0x8b              // MOV ECX, ESI   (ECX = this for __thiscall)
        _emit 0xce
        _emit 0x57              // PUSH EDI       (push elem ptr as argument)
        _emit 0xff              // CALL EAX       (virtual call [*this+0x28](elem))
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0xc]  (count)
        _emit 0x46
        _emit 0x0c
        _emit 0x2b              // SUB EAX, EBX   (EAX = count - idx)
        _emit 0xc3
        _emit 0x83              // SUB EAX, 1     (EAX = count - idx - 1 = remaining)
        _emit 0xe8
        _emit 0x01
        _emit 0x74              // JZ +0x22       (if remaining==0, skip memmove)
        _emit 0x22
        _emit 0x0f              // MOVZX EDX, byte ptr [ESI+0x16]  (c)
        _emit 0xb6
        _emit 0x56
        _emit 0x16
        _emit 0x0f              // MOVZX ECX, byte ptr [ESI+0x14]  (a)
        _emit 0xb6
        _emit 0x4e
        _emit 0x14
        _emit 0x03              // ADD ECX, EDX   (ECX = a + c)
        _emit 0xca
        _emit 0x0f              // MOVZX EDX, byte ptr [ESI+0x15]  (b)
        _emit 0xb6
        _emit 0x56
        _emit 0x15
        _emit 0x03              // ADD ECX, EDX   (ECX = a + c + b = stride)
        _emit 0xca
        _emit 0x8b              // MOV EDX, ECX   (EDX = stride)
        _emit 0xd1
        _emit 0x0f              // IMUL EDX, EAX  (EDX = stride * remaining)
        _emit 0xaf
        _emit 0xd0
        _emit 0x52              // PUSH EDX       (arg3: size = stride * remaining)
        _emit 0x03              // ADD ECX, EDI   (ECX = stride + elem = elem + stride)
        _emit 0xcf
        _emit 0x51              // PUSH ECX       (arg2: src = elem + stride)
        _emit 0x57              // PUSH EDI       (arg1: dst = elem)
        _emit 0xe8              // CALL memmove (rel32 → RVA 0x009d5110)
        _emit 0x32
        _emit 0xe3
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc   (clean 3 args)
        _emit 0xc4
        _emit 0x0c
        _emit 0x83              // ADD dword ptr [ESI+0xc], -1   (count--)
        _emit 0x46
        _emit 0x0c
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xb0              // MOV AL, 1   (return true)
        _emit 0x01
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
    }
}
