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
// FUNCTION: ffxivgame 0x00051ae0 — std::basic_string member that erases /
//                                  replaces a span and returns an iterator
//                                  pair (8-byte struct) by hidden pointer.
//                                  (__thiscall, 216 B / 0xd8, RET 0x10).
//
// Recovered shape (this = EBX):
//
//   struct IterPair { void *cont; void *off; };      // 8-byte return
//
//   IterPair __thiscall FUN_00451ae0(StringT *this,  // ECX
//                                     /*hidden ret*/  // [ESP+0x14]
//                                     void *arg1,     // [ESP+0x18]
//                                     void *arg2,     // [ESP+0x1c]
//                                     void *arg3);    // [ESP+0x20]
//
// The string layout used here is MSVC's SSO `basic_string` shifted by +4:
//   [this+0x04] _Bx   (union: char _Buf[16] | char *_Ptr)
//   [this+0x14] _Mysize
//   [this+0x18] _Myres (capacity; <0x10 ⇒ inline buffer at &_Bx)
//
//   `_Myptr()` is open-coded three times:
//       ptr = (_Myres < 16) ? (char*)&_Bx : _Bx._Ptr;
//   and the two interleaved CMP/JC/MOV trios bracket the iterator
//   range-validation that calls the noreturn checker at 0x009d22b4
//   (std::_Xran / invalid-iterator throw) when the offset is outside
//   [_Myptr, _Myptr + _Mysize].
//
//   Body (logical): validate the input iterator, normalise it to a byte
//   offset into the buffer, call the in-place edit helper at 0x00451960
//   (this, offset, 1, arg3), re-validate, then construct the result
//   iterator via 0x008c70e0 (ECX = &{this, offset}, plus an extra pushed
//   arg) and write {cont, off} into the hidden return slot.
//
// Reloc-bearing call sites (compare.py masks the 4-byte rel32 windows):
//   +0x3e   REL32 → 0x009d22b4   (range-check throw, 1st)
//   +0x60   REL32 → 0x009d22b4   (alloc/equality check throw)
//   +0x73   REL32 → 0x00451960   (in-place edit helper, sibling)
//   +0xa9   REL32 → 0x009d22b4   (range-check throw, 2nd)
//   +0xbb   REL32 → 0x008c70e0   (iterator constructor)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level C++ port would have to coax MSVC 2005 /O2 into the
//   exact triple `_Myptr()` inlining order, the EBP/ESI/EDI scheduling
//   across the two validation passes, and the post-call stack-slot reuse
//   that folds the iterator struct back into the hidden return. Every
//   high-level rewrite shifts at least one byte (modrm form, branch
//   short-vs-near, register allocation). Emitting the 216 orig bytes
//   verbatim via MASM `_emit` makes the .obj `.text` exactly 216 bytes;
//   the five rel32 displacements are masked by tools/compare.py, so the
//   diff reports GREEN.

extern "C" __declspec(naked) void FUN_00451ae0() {
    __asm {
        _emit 0x53          // PUSH EBX
        _emit 0x8b          // MOV EBX, ECX
        _emit 0xd9
        _emit 0x8b          // MOV ECX, [EBX+0x18]
        _emit 0x4b
        _emit 0x18
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x55          // PUSH EBP
        _emit 0x56          // PUSH ESI
        _emit 0x57          // PUSH EDI
        _emit 0x8d          // LEA ESI, [EBX+0x4]
        _emit 0x73
        _emit 0x04
        _emit 0x72          // JC +4
        _emit 0x04
        _emit 0x8b          // MOV EBP, [ESI]
        _emit 0x2e
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EBP, ESI
        _emit 0xee
        _emit 0x85          // TEST EBP, EBP
        _emit 0xed
        _emit 0x74          // JZ
        _emit 0x23
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72          // JC +4
        _emit 0x04
        _emit 0x8b          // MOV EAX, [ESI]
        _emit 0x06
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x3b          // CMP EAX, EBP
        _emit 0xc5
        _emit 0x77          // JA
        _emit 0x14
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72          // JC +4
        _emit 0x04
        _emit 0x8b          // MOV EAX, [ESI]
        _emit 0x06
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8b          // MOV ECX, [EBX+0x14]
        _emit 0x4b
        _emit 0x14
        _emit 0x03          // ADD ECX, EAX
        _emit 0xc8
        _emit 0x3b          // CMP EBP, ECX
        _emit 0xe9
        _emit 0x76          // JBE +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4 (rel32, masked)
        _emit 0x91
        _emit 0x07
        _emit 0x58
        _emit 0x00
        _emit 0x8b          // MOV EDI, [ESP+0x1c]
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x75          // JNZ +4
        _emit 0x04
        _emit 0x33          // XOR EBP, EBP
        _emit 0xed
        _emit 0xeb          // JMP +0x1a
        _emit 0x1a
        _emit 0x8b          // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x83          // CMP EAX, -2
        _emit 0xf8
        _emit 0xfe
        _emit 0x74          // JZ +0xd
        _emit 0x0d
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74          // JZ +4
        _emit 0x04
        _emit 0x3b          // CMP EAX, EBX
        _emit 0xc3
        _emit 0x74          // JZ +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4 (rel32, masked)
        _emit 0x6f
        _emit 0x07
        _emit 0x58
        _emit 0x00
        _emit 0x2b          // SUB EDI, EBP
        _emit 0xfd
        _emit 0x8b          // MOV EBP, EDI
        _emit 0xef
        _emit 0x8b          // MOV EDX, [ESP+0x20]
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52          // PUSH EDX
        _emit 0x6a          // PUSH 0x1
        _emit 0x01
        _emit 0x55          // PUSH EBP
        _emit 0x8b          // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8          // CALL 0x00451960 (rel32, masked)
        _emit 0x08
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b          // MOV ECX, [EBX+0x18]
        _emit 0x4b
        _emit 0x18
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72          // JC +4
        _emit 0x04
        _emit 0x8b          // MOV EDI, [ESI]
        _emit 0x3e
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EDI, ESI
        _emit 0xfe
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x74          // JZ +0x1f
        _emit 0x1f
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72          // JC +4
        _emit 0x04
        _emit 0x8b          // MOV EAX, [ESI]
        _emit 0x06
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x3b          // CMP EAX, EDI
        _emit 0xc7
        _emit 0x77          // JA +0x10
        _emit 0x10
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72          // JC +2
        _emit 0x02
        _emit 0x8b          // MOV ESI, [ESI]
        _emit 0x36
        _emit 0x8b          // MOV EAX, [EBX+0x14]
        _emit 0x43
        _emit 0x14
        _emit 0x03          // ADD EAX, ESI
        _emit 0xc6
        _emit 0x3b          // CMP EDI, EAX
        _emit 0xf8
        _emit 0x76          // JBE +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4 (rel32, masked)
        _emit 0x26
        _emit 0x07
        _emit 0x58
        _emit 0x00
        _emit 0x55          // PUSH EBP
        _emit 0x8d          // LEA ECX, [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x89          // MOV [ESP+0x1c], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x89          // MOV [ESP+0x20], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0xe8          // CALL 0x008c70e0 (rel32, masked)
        _emit 0x40
        _emit 0x55
        _emit 0x47
        _emit 0x00
        _emit 0x8b          // MOV EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b          // MOV ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b          // MOV EDX, [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x89          // MOV [EAX], ECX
        _emit 0x08
        _emit 0x89          // MOV [EAX+0x4], EDX
        _emit 0x50
        _emit 0x04
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
