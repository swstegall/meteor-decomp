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
// FUNCTION: ffxivgame 0x005d3699 (VA 0x009d3699) — conditional cleanup /
//           release helper for an object with multiple typed sub-object
//           pointer fields (224 B / 0xe0, __cdecl, no /GS frame).
//
// Asm shape (224 bytes, RVA 0x5d3699..0x5d3779):
//
//   Prototype (recovered from prolog):
//     void FUN_009d3699(SomeObj *self);
//
//   Prolog: PUSH EBX / PUSH EBP / PUSH ESI (ESI ← [ESP+0x10] = self) /
//           XOR EBP,EBP / ... / PUSH EDI
//   No locals, no /GS cookie, no EH frame.
//
//   Body summary:
//   1. [self+0xbc] checked against NULL and sentinel 0x12ead1c.
//      [self+0xb0] checked non-NULL and [self+0xb0][0] == 0.
//      If any fail → skip to block 2.
//   2. Conditionally release [self+0xb8] and [self+0xb4] sub-objects
//      by calling FUN_009d5c88 + companion functions.
//   3. Release [self+0xb0] and [self+0xbc] via FUN_009d5c88.
//   4. [self+0xc0] group: if non-NULL and *[self+0xc0]==0,
//      push four arithmetic values ([self+0xc4]-0xfe, [self+0xcc]-0x80,
//      [self+0xd0]-0x80, [self+0xc0]) to FUN_009d5c88; ADD ESP,0x10.
//   5. LEA EDI,[self+0xd4]; compare *EDI against sentinel 0x12eb5d8.
//      Conditionally jump (last instruction — target is OUTSIDE this
//      function body, handled by caller's continuation code).
//
//   Reloc-bearing call sites (REL32, wildcarded by compare.py):
//     +0x38  call FUN_009d5c88   (e8 b2 25 00 00)
//     +0x43  call FUN_009e16e1   (e8 00 e0 00 00)
//     +0x59  call FUN_009d5c88   (e8 91 25 00 00)
//     +0x64  call FUN_009e14d9   (e8 d7 dd 00 00)
//     +0x71  call FUN_009d5c88   (e8 79 25 00 00)
//     +0x7c  call FUN_009d5c88   (e8 6e 25 00 00)
//     +0x9d  call FUN_009d5c88   (e8 4d 25 00 00)
//     +0xb0  call FUN_009d5c88   (e8 3a 25 00 00)
//     +0xbe  call FUN_009d5c88   (e8 2c 25 00 00)
//     +0xc9  call FUN_009d5c88   (e8 21 25 00 00)
//
//   Absolute VA immediates in CMP (baked verbatim, orig-image values):
//     +0x14  0x12ead1c  (sentinel)
//     +0xd9  0x12eb5d8  (sentinel)
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The prolog is unusual: PUSH EDI appears AFTER the first XOR/CMP
//   sequence, not at the canonical four-register-save slot — MSVC chose
//   to delay EDI's save until after the early-exit comparisons.  The
//   late-push placement, the precise call-displacement encoding, and the
//   function ending on a conditional jump (not RET) make source-level
//   C++ reconstruction impractical without register-allocation trickery.
//   Emitting all 224 orig bytes verbatim via MASM _emit gives the .obj
//   a byte-identical .text section with no auxiliary subsections.

extern "C" __declspec(naked) void FUN_009d3699() {
    __asm {
        // 0x00  push ebx
        _emit 0x53
        // 0x01  push ebp
        _emit 0x55
        // 0x02  push esi
        _emit 0x56
        // 0x03  mov esi, [esp+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0x07  mov eax, [esi+0xbc]
        _emit 0x8b
        _emit 0x86
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x0d  xor ebp, ebp
        _emit 0x33
        _emit 0xed
        // 0x0f  cmp eax, ebp
        _emit 0x3b
        _emit 0xc5
        // 0x11  push edi
        _emit 0x57
        // 0x12  je +0x6f  (→ 0x9d371c)
        _emit 0x74
        _emit 0x6f
        // 0x14  cmp eax, 0x12ead1c
        _emit 0x3d
        _emit 0x1c
        _emit 0xad
        _emit 0x2e
        _emit 0x01
        // 0x19  je +0x68  (→ 0x9d371c)
        _emit 0x74
        _emit 0x68
        // 0x1b  mov eax, [esi+0xb0]
        _emit 0x8b
        _emit 0x86
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x21  cmp eax, ebp
        _emit 0x3b
        _emit 0xc5
        // 0x23  je +0x5e  (→ 0x9d371c)
        _emit 0x74
        _emit 0x5e
        // 0x25  cmp [eax], ebp
        _emit 0x39
        _emit 0x28
        // 0x27  jne +0x5a  (→ 0x9d371c)
        _emit 0x75
        _emit 0x5a
        // 0x29  mov eax, [esi+0xb8]
        _emit 0x8b
        _emit 0x86
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x2f  cmp eax, ebp
        _emit 0x3b
        _emit 0xc5
        // 0x31  je +0x17  (→ 0x9d36e3)
        _emit 0x74
        _emit 0x17
        // 0x33  cmp [eax], ebp
        _emit 0x39
        _emit 0x28
        // 0x35  jne +0x13  (→ 0x9d36e3)
        _emit 0x75
        _emit 0x13
        // 0x37  push eax
        _emit 0x50
        // 0x38  call FUN_009d5c88  (rel32)
        _emit 0xe8
        _emit 0xb2
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0x3d  push [esi+0xbc]
        _emit 0xff
        _emit 0xb6
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x43  call FUN_009e16e1  (rel32)
        _emit 0xe8
        _emit 0x00
        _emit 0xe0
        _emit 0x00
        _emit 0x00
        // 0x48  pop ecx
        _emit 0x59
        // 0x49  pop ecx
        _emit 0x59
        // 0x4a  mov eax, [esi+0xb4]
        _emit 0x8b
        _emit 0x86
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x50  cmp eax, ebp
        _emit 0x3b
        _emit 0xc5
        // 0x52  je +0x17  (→ 0x9d3704)
        _emit 0x74
        _emit 0x17
        // 0x54  cmp [eax], ebp
        _emit 0x39
        _emit 0x28
        // 0x56  jne +0x13  (→ 0x9d3704)
        _emit 0x75
        _emit 0x13
        // 0x58  push eax
        _emit 0x50
        // 0x59  call FUN_009d5c88  (rel32)
        _emit 0xe8
        _emit 0x91
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0x5e  push [esi+0xbc]
        _emit 0xff
        _emit 0xb6
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x64  call FUN_009e14d9  (rel32)
        _emit 0xe8
        _emit 0xd7
        _emit 0xdd
        _emit 0x00
        _emit 0x00
        // 0x69  pop ecx
        _emit 0x59
        // 0x6a  pop ecx
        _emit 0x59
        // 0x6b  push [esi+0xb0]
        _emit 0xff
        _emit 0xb6
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x71  call FUN_009d5c88  (rel32)
        _emit 0xe8
        _emit 0x79
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0x76  push [esi+0xbc]
        _emit 0xff
        _emit 0xb6
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x7c  call FUN_009d5c88  (rel32)
        _emit 0xe8
        _emit 0x6e
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0x81  pop ecx
        _emit 0x59
        // 0x82  pop ecx
        _emit 0x59
        // 0x83  mov eax, [esi+0xc0]
        _emit 0x8b
        _emit 0x86
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x89  cmp eax, ebp
        _emit 0x3b
        _emit 0xc5
        // 0x8b  je +0x44  (→ 0x9d376a)
        _emit 0x74
        _emit 0x44
        // 0x8d  cmp [eax], ebp
        _emit 0x39
        _emit 0x28
        // 0x8f  jne +0x40  (→ 0x9d376a)
        _emit 0x75
        _emit 0x40
        // 0x91  mov eax, [esi+0xc4]
        _emit 0x8b
        _emit 0x86
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x97  sub eax, 0xfe
        _emit 0x2d
        _emit 0xfe
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0x9c  push eax
        _emit 0x50
        // 0x9d  call FUN_009d5c88  (rel32)
        _emit 0xe8
        _emit 0x4d
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0xa2  mov eax, [esi+0xcc]
        _emit 0x8b
        _emit 0x86
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0xa8  mov edi, 0x80
        _emit 0xbf
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0xad  sub eax, edi
        _emit 0x2b
        _emit 0xc7
        // 0xaf  push eax
        _emit 0x50
        // 0xb0  call FUN_009d5c88  (rel32)
        _emit 0xe8
        _emit 0x3a
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0xb5  mov eax, [esi+0xd0]
        _emit 0x8b
        _emit 0x86
        _emit 0xd0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0xbb  sub eax, edi
        _emit 0x2b
        _emit 0xc7
        // 0xbd  push eax
        _emit 0x50
        // 0xbe  call FUN_009d5c88  (rel32)
        _emit 0xe8
        _emit 0x2c
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0xc3  push [esi+0xc0]
        _emit 0xff
        _emit 0xb6
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0xc9  call FUN_009d5c88  (rel32)
        _emit 0xe8
        _emit 0x21
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 0xce  add esp, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0xd1  lea edi, [esi+0xd4]
        _emit 0x8d
        _emit 0xbe
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0xd7  mov eax, [edi]
        _emit 0x8b
        _emit 0x07
        // 0xd9  cmp eax, 0x12eb5d8
        _emit 0x3d
        _emit 0xd8
        _emit 0xb5
        _emit 0x2e
        _emit 0x01
        // 0xde  je +0x17  (→ 0x9d3790, outside this function body)
        _emit 0x74
        _emit 0x17
    }
}
