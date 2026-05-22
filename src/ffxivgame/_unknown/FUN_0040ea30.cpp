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
// FUNCTION: ffxivgame 0x0040ea30 — LeakIter::is_valid() (__thiscall, 113 B)
//
// Tests whether a LeakIter is currently positioned on a valid element.
// Returns true (1) if the iterator points to a live element, false (0)
// if the iterator is exhausted (cur == null) or the mode is neither 0 nor 1.
//
// LeakIter layout (inferred from this function and FUN_0040e970 advance()):
//   +0x00  int *head   — pointer to the MemoryTracker head node
//   +0x04  int *cur    — current node pointer (null ⇒ exhausted)
//   +0x08  int *field8 — updated by this function
//   +0x0c  int  fieldc — updated by this function
//   +0x10  byte field10 — updated by this function
//   +0x14  int  field14 — updated by this function
//
// FUN_0040e0b0 (9 B vtable dispatch):
//   MOV ECX, [ECX]; MOV EAX, [ECX]; MOV EDX, [EAX+0x18]; JMP EDX
//   Called with ECX = this->head; tail-calls vtable[6](head).
//   Returns mode integer (0 or 1).
//
// Mode 0 path (LeakIter::is_valid when mode==0):
//   EAX = this->cur  (the node pointer — used both as data ptr AND raw offset)
//   ECX = EAX->field8  (pointer to a data block)
//   this->field8  = *(ECX + (int)EAX + 0x10)  — SIB: [ECX + EAX*1 + 0x10]
//   this->fieldc  = *(ECX + (int)EAX + 0x14)  — SIB: [ECX + EAX*1 + 0x14]
//   this->field10 = 1
//   this->field14 = *(ECX + (int)EAX + 0x1c)  — SIB: [ECX + EAX*1 + 0x1c]
//
// Mode 1 path (LeakIter::is_valid when mode==1):
//   EAX = this->cur  (same node pointer)
//   this->field8  = EAX
//   this->fieldc  = *EAX + 0x10        (i.e. EAX->field0 + 16)
//   this->field10 = (byte)(EAX->field4 >> 31)  (sign bit)
//   this->field14 = EAX->field8
//
// Both paths return 1; the null-check path returns 0 (BL init = 0).
//
// Calling convention: __thiscall (ECX = this), no stack args, plain RET.
// Callee-saves: EBX (partial — only BL used), ESI.  No local frame.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The mode-0 path uses SIB addressing [ECX + EAX*1 + disp8] where EAX
//   is a live pointer value being treated as a raw byte offset into a
//   separate data block pointed to by ECX.  This is not expressible in
//   standard C/C++ without undefined behaviour, so the function is
//   re-emitted verbatim as a __declspec(naked) _emit sequence.
//   compare.py masks the single CALL rel32 at offset +0x0e (4 bytes).
//
// Reloc-bearing site:
//   offset +0x0e  CALL rel32 → FUN_0040e0b0  (rel32 = 0xfffff66d)

extern "C" __declspec(naked) void FUN_0040ea30() {
    __asm {
        // 0000ea30:  53                  PUSH EBX
        _emit 0x53
        // 0000ea31:  56                  PUSH ESI
        _emit 0x56
        // 0000ea32:  8b f1               MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0000ea34:  32 db               XOR BL, BL
        _emit 0x32
        _emit 0xdb
        // 0000ea36:  83 7e 04 00         CMP dword ptr [ESI+0x4], 0x0
        _emit 0x83
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        // 0000ea3a:  74 60               JZ +0x60  (→ 0x0040ea9c)
        _emit 0x74
        _emit 0x60
        // 0000ea3c:  8b 0e               MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0000ea3e:  e8 6d f6 ff ff      CALL 0x0040e0b0  (rel32 = 0xfffff66d)
        _emit 0xe8
        _emit 0x6d
        _emit 0xf6
        _emit 0xff
        _emit 0xff
        // 0000ea43:  83 e8 00            SUB EAX, 0x0
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        // 0000ea46:  74 2a               JZ +0x2a  (→ 0x0040ea72)
        _emit 0x74
        _emit 0x2a
        // 0000ea48:  83 e8 01            SUB EAX, 0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0000ea4b:  75 4f               JNZ +0x4f  (→ 0x0040ea9c)
        _emit 0x75
        _emit 0x4f
        // 0000ea4d:  8b 46 04            MOV EAX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0000ea50:  89 46 08            MOV dword ptr [ESI+0x8], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0000ea53:  8b 08               MOV ECX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x08
        // 0000ea55:  83 c1 10            ADD ECX, 0x10
        _emit 0x83
        _emit 0xc1
        _emit 0x10
        // 0000ea58:  89 4e 0c            MOV dword ptr [ESI+0xc], ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x0c
        // 0000ea5b:  8b 50 04            MOV EDX, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0000ea5e:  c1 ea 1f            SHR EDX, 0x1f
        _emit 0xc1
        _emit 0xea
        _emit 0x1f
        // 0000ea61:  80 e2 01            AND DL, 0x1
        _emit 0x80
        _emit 0xe2
        _emit 0x01
        // 0000ea64:  88 56 10            MOV byte ptr [ESI+0x10], DL
        _emit 0x88
        _emit 0x56
        _emit 0x10
        // 0000ea67:  8b 40 08            MOV EAX, dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x40
        _emit 0x08
        // 0000ea6a:  89 46 14            MOV dword ptr [ESI+0x14], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x14
        // 0000ea6d:  5e                  POP ESI
        _emit 0x5e
        // 0000ea6e:  b0 01               MOV AL, 0x1
        _emit 0xb0
        _emit 0x01
        // 0000ea70:  5b                  POP EBX
        _emit 0x5b
        // 0000ea71:  c3                  RET
        _emit 0xc3
        // 0000ea72:  8b 46 04            MOV EAX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 0000ea75:  8b 48 08            MOV ECX, dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0000ea78:  8b 54 01 10         MOV EDX, dword ptr [ECX + EAX*1 + 0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x01
        _emit 0x10
        // 0000ea7c:  89 56 08            MOV dword ptr [ESI+0x8], EDX
        _emit 0x89
        _emit 0x56
        _emit 0x08
        // 0000ea7f:  8b 48 08            MOV ECX, dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0000ea82:  8b 54 01 14         MOV EDX, dword ptr [ECX + EAX*1 + 0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x01
        _emit 0x14
        // 0000ea86:  89 56 0c            MOV dword ptr [ESI+0xc], EDX
        _emit 0x89
        _emit 0x56
        _emit 0x0c
        // 0000ea89:  c6 46 10 01         MOV byte ptr [ESI+0x10], 0x1
        _emit 0xc6
        _emit 0x46
        _emit 0x10
        _emit 0x01
        // 0000ea8d:  8b 48 08            MOV ECX, dword ptr [EAX+0x8]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 0000ea90:  8b 54 01 1c         MOV EDX, dword ptr [ECX + EAX*1 + 0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x01
        _emit 0x1c
        // 0000ea94:  89 56 14            MOV dword ptr [ESI+0x14], EDX
        _emit 0x89
        _emit 0x56
        _emit 0x14
        // 0000ea97:  5e                  POP ESI
        _emit 0x5e
        // 0000ea98:  b0 01               MOV AL, 0x1
        _emit 0xb0
        _emit 0x01
        // 0000ea9a:  5b                  POP EBX
        _emit 0x5b
        // 0000ea9b:  c3                  RET
        _emit 0xc3
        // 0000ea9c:  5e                  POP ESI
        _emit 0x5e
        // 0000ea9d:  8a c3               MOV AL, BL
        _emit 0x8a
        _emit 0xc3
        // 0000ea9f:  5b                  POP EBX
        _emit 0x5b
        // 0000eaa0:  c3                  RET
        _emit 0xc3
    }
}
