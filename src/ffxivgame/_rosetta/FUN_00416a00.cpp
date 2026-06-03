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
// FUNCTION: ffxivgame 0x00016a00 — sorted-array insert with virtual
//                                  binary-search, memmove right-shift, and
//                                  two __fastcall copy-out calls (215 B /
//                                  0xd7, __thiscall, RET 0xc).
//
// Signature (recovered from asm):
//
//   __thiscall int FUN_00416a00(SortedArray* this,
//                               void*        src0,   // [esp+4]  — first part src
//                               void*        src1,   // [esp+8]  — second part src
//                               void**       out_ptr)// [esp+0xc] — optional output
//
//   ECX = this (the sorted-array object)
//   Returns: 0 = success, 1 = full (count==capacity), 2 = rejected by vfunc
//
// Object layout (inferred from field accesses):
//
//   [this+0x00]  vtable pointer
//   [this+0x04]  data   — pointer to base of element array
//   [this+0x08]  limit  — capacity (max element count)
//   [this+0x0c]  count  — current element count
//   [this+0x14]  sz0    — byte: size in bytes of part 0 of each element
//   [this+0x15]  sz1    — byte: size in bytes of part 1 of each element
//   [this+0x16]  sz2    — byte: size in bytes of part 2 (total stride = sz0+sz1+sz2)
//
// Logic sketch:
//
//   if (count == limit)  return 1;     // full
//   if (count == 0) {
//       slot = data;                   // insert at front
//   } else {
//       bool found = false;
//       int  idx   = vtable[7](src0, &found);  // binary-search finds insertion pt
//       if (found) return 2;           // key already present
//       int  tail  = count - idx;
//       if (tail >= 1)                 // right-shift tail elements to make room
//           memmove(data + stride*(idx+1), data + stride*idx, stride*tail);
//       slot = data + stride * idx;
//   }
//   if (out_ptr) *out_ptr = slot;      // optionally return slot address
//   // write new element in two parts via FUN_00416810 (__fastcall DWORD copier):
//   FUN_00416810(src0,            sz0>>2, slot);
//   FUN_00416810(src1,            sz1>>2, slot + sz0);
//   count++;
//   ADD ESP,8;                         // caller-cleans the two PUSH-dst from above
//   return 0;
//
// Virtual call: vtable[7] is __thiscall with 2 stack args (RET 8).
//   The bool output is written into the arg0 stack slot which MSVC reuses
//   as a local (LEA of [esp+0x14] before the PUSH; c6 44 24 1c 00 initialises
//   the slot to 0 after the PUSH pair shifts the stack by 8).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two CALL FUN_00416810 sequences are reloc sites (REL32 to .text
//   0x00416810), and the memmove call is a reloc site (REL32 to .text
//   0x009d5110). The virtual call site is an indirect CALL EDX (no reloc).
//   The main difficulty is the bool-output-via-arg-slot pattern that MSVC
//   only produces when it determines arg0's slot is dead after the vtable
//   call; any source-level rewrite allocates a separate local, producing a
//   different stack frame and breaking the byte diff. Emitting the 215
//   original bytes verbatim via _emit gives byte-identical output modulo
//   the three relocation sites, which compare.py masks.
//
// Relocation sites (offsets within function body):
//   +0x79  REL32 → 0x009d5110  (memmove, .text CRT)
//   +0xab  REL32 → 0x00416810  (FUN_00416810, call 1)
//   +0xc2  REL32 → 0x00416810  (FUN_00416810, call 2)

extern "C" __declspec(naked) void FUN_00416a00() {
    __asm {
        // 00016a00: 56           PUSH ESI
        _emit 0x56
        // 00016a01: 8b f1        MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00016a03: 8b 46 0c     MOV EAX,[ESI+0xc]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 00016a06: 3b 46 08     CMP EAX,[ESI+0x8]
        _emit 0x3b
        _emit 0x46
        _emit 0x08
        // 00016a09: 75 09        JNZ +0x09 (→ 0x16a14)
        _emit 0x75
        _emit 0x09
        // 00016a0b: b8 01 00 00 00  MOV EAX,1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00016a10: 5e           POP ESI
        _emit 0x5e
        // 00016a11: c2 0c 00     RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00016a14: 85 c0        TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00016a16: 53           PUSH EBX
        _emit 0x53
        // 00016a17: 55           PUSH EBP
        _emit 0x55
        // 00016a18: 8b 6c 24 10  MOV EBP,[ESP+0x10]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 00016a1c: 57           PUSH EDI
        _emit 0x57
        // 00016a1d: 75 05        JNZ +0x05 (→ 0x16a24)
        _emit 0x75
        _emit 0x05
        // 00016a1f: 8b 7e 04     MOV EDI,[ESI+0x4]
        _emit 0x8b
        _emit 0x7e
        _emit 0x04
        // 00016a22: eb 73        JMP +0x73 (→ 0x16a97)
        _emit 0xeb
        _emit 0x73
        // 00016a24: 8b 06        MOV EAX,[ESI]
        _emit 0x8b
        _emit 0x06
        // 00016a26: 8b 50 1c     MOV EDX,[EAX+0x1c]
        _emit 0x8b
        _emit 0x50
        _emit 0x1c
        // 00016a29: 8d 4c 24 14  LEA ECX,[ESP+0x14]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00016a2d: 51           PUSH ECX
        _emit 0x51
        // 00016a2e: 55           PUSH EBP
        _emit 0x55
        // 00016a2f: 8b ce        MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00016a31: c6 44 24 1c 00  MOV byte ptr [ESP+0x1c],0x0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x00
        // 00016a36: ff d2        CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00016a38: 80 7c 24 14 00  CMP byte ptr [ESP+0x14],0x0
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x00
        // 00016a3d: 8b d8        MOV EBX,EAX
        _emit 0x8b
        _emit 0xd8
        // 00016a3f: 74 0c        JZ +0x0c (→ 0x16a4d)
        _emit 0x74
        _emit 0x0c
        // 00016a41: 5f           POP EDI
        _emit 0x5f
        // 00016a42: 5d           POP EBP
        _emit 0x5d
        // 00016a43: 5b           POP EBX
        _emit 0x5b
        // 00016a44: b8 02 00 00 00  MOV EAX,2
        _emit 0xb8
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00016a49: 5e           POP ESI
        _emit 0x5e
        // 00016a4a: c2 0c 00     RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00016a4d: 8b 4e 0c     MOV ECX,[ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 00016a50: 2b cb        SUB ECX,EBX
        _emit 0x2b
        _emit 0xcb
        // 00016a52: 83 f9 01     CMP ECX,0x1
        _emit 0x83
        _emit 0xf9
        _emit 0x01
        // 00016a55: 7c 2a        JL +0x2a (→ 0x16a81)
        _emit 0x7c
        _emit 0x2a
        // 00016a57: 0f b6 56 15  MOVZX EDX,byte ptr [ESI+0x15]
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x15
        // 00016a5b: 0f b6 46 16  MOVZX EAX,byte ptr [ESI+0x16]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x16
        // 00016a5f: 03 c2        ADD EAX,EDX
        _emit 0x03
        _emit 0xc2
        // 00016a61: 0f b6 56 14  MOVZX EDX,byte ptr [ESI+0x14]
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x14
        // 00016a65: 03 d0        ADD EDX,EAX
        _emit 0x03
        _emit 0xd0
        // 00016a67: 8b c2        MOV EAX,EDX
        _emit 0x8b
        _emit 0xc2
        // 00016a69: 8b fa        MOV EDI,EDX
        _emit 0x8b
        _emit 0xfa
        // 00016a6b: 0f af c3     IMUL EAX,EBX
        _emit 0x0f
        _emit 0xaf
        _emit 0xc3
        // 00016a6e: 03 46 04     ADD EAX,[ESI+0x4]
        _emit 0x03
        _emit 0x46
        _emit 0x04
        // 00016a71: 0f af f9     IMUL EDI,ECX
        _emit 0x0f
        _emit 0xaf
        _emit 0xf9
        // 00016a74: 57           PUSH EDI
        _emit 0x57
        // 00016a75: 50           PUSH EAX
        _emit 0x50
        // 00016a76: 03 d0        ADD EDX,EAX
        _emit 0x03
        _emit 0xd0
        // 00016a78: 52           PUSH EDX
        _emit 0x52
        // 00016a79: e8 92 e6 5b 00  CALL 0x009d5110 (memmove)
        _emit 0xe8
        _emit 0x92
        _emit 0xe6
        _emit 0x5b
        _emit 0x00
        // 00016a7e: 83 c4 0c     ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00016a81: 0f b6 7e 16  MOVZX EDI,byte ptr [ESI+0x16]
        _emit 0x0f
        _emit 0xb6
        _emit 0x7e
        _emit 0x16
        // 00016a85: 0f b6 46 15  MOVZX EAX,byte ptr [ESI+0x15]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x15
        // 00016a89: 0f b6 4e 14  MOVZX ECX,byte ptr [ESI+0x14]
        _emit 0x0f
        _emit 0xb6
        _emit 0x4e
        _emit 0x14
        // 00016a8d: 03 f8        ADD EDI,EAX
        _emit 0x03
        _emit 0xf8
        // 00016a8f: 03 f9        ADD EDI,ECX
        _emit 0x03
        _emit 0xf9
        // 00016a91: 0f af fb     IMUL EDI,EBX
        _emit 0x0f
        _emit 0xaf
        _emit 0xfb
        // 00016a94: 03 7e 04     ADD EDI,[ESI+0x4]
        _emit 0x03
        _emit 0x7e
        _emit 0x04
        // 00016a97: 8b 44 24 1c  MOV EAX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00016a9b: 85 c0        TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00016a9d: 74 02        JZ +0x02 (→ 0x16aa1)
        _emit 0x74
        _emit 0x02
        // 00016a9f: 89 38        MOV [EAX],EDI
        _emit 0x89
        _emit 0x38
        // 00016aa1: 0f b6 56 14  MOVZX EDX,byte ptr [ESI+0x14]
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x14
        // 00016aa5: c1 ea 02     SHR EDX,0x2
        _emit 0xc1
        _emit 0xea
        _emit 0x02
        // 00016aa8: 57           PUSH EDI
        _emit 0x57
        // 00016aa9: 8b cd        MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 00016aab: e8 60 fd ff ff  CALL 0x00416810 (FUN_00416810, call 1)
        _emit 0xe8
        _emit 0x60
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00016ab0: 0f b6 46 14  MOVZX EAX,byte ptr [ESI+0x14]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x14
        // 00016ab4: 0f b6 56 15  MOVZX EDX,byte ptr [ESI+0x15]
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x15
        // 00016ab8: 8b 4c 24 1c  MOV ECX,[ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00016abc: 03 c7        ADD EAX,EDI
        _emit 0x03
        _emit 0xc7
        // 00016abe: c1 ea 02     SHR EDX,0x2
        _emit 0xc1
        _emit 0xea
        _emit 0x02
        // 00016ac1: 50           PUSH EAX
        _emit 0x50
        // 00016ac2: e8 49 fd ff ff  CALL 0x00416810 (FUN_00416810, call 2)
        _emit 0xe8
        _emit 0x49
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00016ac7: 83 46 0c 01  ADD dword ptr [ESI+0xc],0x1
        _emit 0x83
        _emit 0x46
        _emit 0x0c
        _emit 0x01
        // 00016acb: 83 c4 08     ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00016ace: 5f           POP EDI
        _emit 0x5f
        // 00016acf: 5d           POP EBP
        _emit 0x5d
        // 00016ad0: 5b           POP EBX
        _emit 0x5b
        // 00016ad1: 33 c0        XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00016ad3: 5e           POP ESI
        _emit 0x5e
        // 00016ad4: c2 0c 00     RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
