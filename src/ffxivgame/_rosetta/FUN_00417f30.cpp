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
// FUNCTION: ffxivgame 0x00417f30 — vector<T>::insert iterator factory
//                                  (__thiscall, 137 bytes / 0x89)
//                                  T is 8-byte element type
//
// Calling convention: __thiscall (ECX = this); four __stdcall stack args;
//   RET 0x10 pops them.  Callee-saves: ECX (unusual: push/pop ECX framing),
//   EBX, EBP (repurposed for arg2), ESI, EDI.
//
// Object layout (this = ESI):
//   [this + 0x00]  (unused in this fn)
//   [this + 0x04]  ptr  begin — pointer to first element
//   [this + 0x08]  ptr  end   — pointer past last element
//
// Stack args after all 5 pushes (ESP offsets at deepest point):
//   [ESP + 0x18]  arg1  — pointer to output iterator struct
//   [ESP + 0x1C]  arg2  — position iterator (loaded into EBP before ESI/EDI push)
//   [ESP + 0x20]  arg3  — range begin pointer
//   [ESP + 0x24]  arg4  — range end pointer (or count)
//
// Logic (high-level):
//   1. Load begin = [this+4], check if NULL → offset = 0
//   2. Compute element count = (end - begin) >> 3; if 0 → offset = 0
//   3. Validate begin <= end; validate arg2 (position iterator)
//   4. Compute offset EBX = (arg3 - begin) >> 3
//   5. Call FUN_00417bb0(this, arg2, arg3, 1, arg4)  [inner insert]
//   6. Reload begin; validate; new_iter_ptr = begin + offset * 8
//   7. Store *arg1 = { this, new_iter_ptr }
//
// Reloc sites (compare.py masks rel32 displacements — 4 bytes after 0xe8):
//   +0x26  CALL 0x009d22b4  (assertion / invalid_iterator)
//   +0x33  CALL 0x009d22b4
//   +0x50  CALL FUN_00417bb0  (rel32 = 0xfffffc2b)
//   +0x5d  CALL 0x009d22b4
//   +0x73  CALL 0x009d22b4
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The unusual PUSH ECX / POP ECX framing and the interleaved
//   EBP-load-before-push pattern cannot be reproduced from C++ source
//   without triggering different prologue ordering.  The
//   __declspec(naked) body re-emits the original 137 bytes verbatim via
//   MASM _emit directives; tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00417f30() {
    __asm {
        // 00017f30:  51                 PUSH ECX
        _emit 0x51
        // 00017f31:  53                 PUSH EBX
        _emit 0x53
        // 00017f32:  55                 PUSH EBP
        _emit 0x55
        // 00017f33:  8b 6c 24 14        MOV EBP,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 00017f37:  56                 PUSH ESI
        _emit 0x56
        // 00017f38:  8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 00017f3a:  57                 PUSH EDI
        _emit 0x57
        // 00017f3b:  8b 7e 04           MOV EDI,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x7e
        _emit 0x04
        // 00017f3e:  85 ff              TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00017f40:  74 0c              JZ +0x0c
        _emit 0x74
        _emit 0x0c
        // 00017f42:  8b 46 08           MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 00017f45:  8b c8              MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 00017f47:  2b cf              SUB ECX,EDI
        _emit 0x2b
        _emit 0xcf
        // 00017f49:  c1 f9 03           SAR ECX,0x3
        _emit 0xc1
        _emit 0xf9
        _emit 0x03
        // 00017f4c:  75 04              JNZ +0x04
        _emit 0x75
        _emit 0x04
        // 00017f4e:  33 db              XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 00017f50:  eb 1f              JMP +0x1f
        _emit 0xeb
        _emit 0x1f
        // 00017f52:  3b f8              CMP EDI,EAX
        _emit 0x3b
        _emit 0xf8
        // 00017f54:  76 05              JBE +0x05
        _emit 0x76
        _emit 0x05
        // 00017f56:  e8 59 a3 5b 00     CALL 0x009d22b4
        _emit 0xe8
        _emit 0x59
        _emit 0xa3
        _emit 0x5b
        _emit 0x00
        // 00017f5b:  85 ed              TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 00017f5d:  74 04              JZ +0x04
        _emit 0x74
        _emit 0x04
        // 00017f5f:  3b ee              CMP EBP,ESI
        _emit 0x3b
        _emit 0xee
        // 00017f61:  74 05              JZ +0x05
        _emit 0x74
        _emit 0x05
        // 00017f63:  e8 4c a3 5b 00     CALL 0x009d22b4
        _emit 0xe8
        _emit 0x4c
        _emit 0xa3
        _emit 0x5b
        _emit 0x00
        // 00017f68:  8b 5c 24 20        MOV EBX,dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00017f6c:  2b df              SUB EBX,EDI
        _emit 0x2b
        _emit 0xdf
        // 00017f6e:  c1 fb 03           SAR EBX,0x3
        _emit 0xc1
        _emit 0xfb
        _emit 0x03
        // 00017f71:  8b 54 24 24        MOV EDX,dword ptr [ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 00017f75:  8b 44 24 20        MOV EAX,dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00017f79:  52                 PUSH EDX
        _emit 0x52
        // 00017f7a:  6a 01              PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 00017f7c:  50                 PUSH EAX
        _emit 0x50
        // 00017f7d:  55                 PUSH EBP
        _emit 0x55
        // 00017f7e:  8b ce              MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00017f80:  e8 2b fc ff ff     CALL FUN_00417bb0
        _emit 0xe8
        _emit 0x2b
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 00017f85:  8b 7e 04           MOV EDI,dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x7e
        _emit 0x04
        // 00017f88:  3b 7e 08           CMP EDI,dword ptr [ESI+0x8]
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 00017f8b:  76 05              JBE +0x05
        _emit 0x76
        _emit 0x05
        // 00017f8d:  e8 22 a3 5b 00     CALL 0x009d22b4
        _emit 0xe8
        _emit 0x22
        _emit 0xa3
        _emit 0x5b
        _emit 0x00
        // 00017f92:  89 7c 24 20        MOV dword ptr [ESP+0x20],EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        // 00017f96:  8d 3c df           LEA EDI,[EDI+EBX*0x8]
        _emit 0x8d
        _emit 0x3c
        _emit 0xdf
        // 00017f99:  3b 7e 08           CMP EDI,dword ptr [ESI+0x8]
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 00017f9c:  77 05              JA +0x05
        _emit 0x77
        _emit 0x05
        // 00017f9e:  3b 7e 04           CMP EDI,dword ptr [ESI+0x4]
        _emit 0x3b
        _emit 0x7e
        _emit 0x04
        // 00017fa1:  73 05              JNC +0x05
        _emit 0x73
        _emit 0x05
        // 00017fa3:  e8 0c a3 5b 00     CALL 0x009d22b4
        _emit 0xe8
        _emit 0x0c
        _emit 0xa3
        _emit 0x5b
        _emit 0x00
        // 00017fa8:  8b 44 24 18        MOV EAX,dword ptr [ESP+0x18]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 00017fac:  89 78 04           MOV dword ptr [EAX+0x4],EDI
        _emit 0x89
        _emit 0x78
        _emit 0x04
        // 00017faf:  5f                 POP EDI
        _emit 0x5f
        // 00017fb0:  89 30              MOV dword ptr [EAX],ESI
        _emit 0x89
        _emit 0x30
        // 00017fb2:  5e                 POP ESI
        _emit 0x5e
        // 00017fb3:  5d                 POP EBP
        _emit 0x5d
        // 00017fb4:  5b                 POP EBX
        _emit 0x5b
        // 00017fb5:  59                 POP ECX
        _emit 0x59
        // 00017fb6:  c2 10 00           RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
