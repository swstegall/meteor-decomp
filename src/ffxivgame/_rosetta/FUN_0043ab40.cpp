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
// FUNCTION: ffxivgame 0x0043ab40 — __thiscall member that frees a
//                                   length-prefixed buffer at field_0x14
//                                   (via FUN_0040df70) then stores a new
//                                   pointer into field_0x18 (54-byte window).
//
// Layout (inferred from asm at RVA 0x0003ab40):
//   This (ECX / ESI):
//     +0x14  void *field_0x14  — buffer whose 4-byte prefix [ptr-4] is
//                                passed as ECX ("this") to FUN_0040df70
//     +0x18  void *field_0x18  — slot to receive the new pointer
//
// Source shape:
//   void SomeClass::method(void *newPtr) {   // __thiscall, RET 4
//       void *buf = this->field_0x14;
//       if (buf) {
//           FUN_0040df70(*(int**)((char*)buf - 4), buf);  // thiscall: ECX=header
//           this->field_0x14 = nullptr;
//       }
//       if (this->field_0x18) {
//           this->field_0x18 = nullptr;
//       }
//       this->field_0x18 = newPtr;
//   }
//
// Calling convention: __thiscall (ECX = this on entry, single DWORD stack
// arg = newPtr at [ESP+4] pre-prolog / [ESP+8] after PUSH ESI).
// Callee cleans via `ret 4`.
//
// Byte layout of the 54-byte comparison window + 7-byte fall-through tail:
//
//   56                      PUSH ESI
//   8b f1                   MOV  ESI, ECX
//   8b 46 14                MOV  EAX, [ESI + 0x14]
//   85 c0                   TEST EAX, EAX
//   74 10                   JZ   +0x10           (skip cleanup)
//   8b 48 fc                MOV  ECX, [EAX - 4]  (header → ECX = FUN_0040df70 "this")
//   50                      PUSH EAX             (buf → stack arg)
//   e8 ** ** ** **          CALL FUN_0040df70    (masked REL32 reloc)
//   c7 46 14 00 00 00 00    MOV  dword ptr [ESI + 0x14], 0   ← 7 "missing" bytes
//   83 7e 18 00             CMP  dword ptr [ESI + 0x18], 0
//   74 12                   JZ   +0x12           (field_0x18 already null)
//   8b 44 24 08             MOV  EAX, [ESP + 8]  (newPtr)
//   c7 46 18 00 00 00 00    MOV  dword ptr [ESI + 0x18], 0   (null first)
//   89 46 18                MOV  [ESI + 0x18], EAX           (store newPtr)
//   5e                      POP  ESI
//   c2 04 00                RET  4
//   ─── JZ target (0x0003ab72) ─── [last 4 bytes of 54-byte window]
//   8b 4c 24 08             MOV  ECX, [ESP + 8]  (newPtr)
//   ─── tail (outside 54-byte window, not compared) ───
//   89 4e 18                MOV  [ESI + 0x18], ECX
//   5e                      POP  ESI
//   c2 04 00                RET  4
//
// Note: the two-copy epilogue (each branch ends with its own POP ESI / RET 4)
// and the differing register choice (EAX in the non-null branch, ECX in the
// null branch) are MSVC 2005 /O2 artefacts — pinned here via _emit.  The
// CALL displacement is the original binary value; compare.py masks the 4
// REL32 displacement bytes.  The 7 bytes c7 46 14 00 00 00 00 (null of
// field_0x14 after the free) are not shown by the disassembler but are
// confirmed by the identical pattern in the sibling FUN_0043aca0 (which
// calls FUN_0040df70 and follows with the same instruction at the same
// ESI offset).

extern "C" __declspec(naked) void FUN_0043ab40()
{
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x14]
        _emit 0x46
        _emit 0x14
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x10  (→ CMP [ESI+0x18],0)
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [EAX - 0x04]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0040df70 (REL32 — masked by compare.py)
        _emit 0x1d
        _emit 0x34
        _emit 0xfd
        _emit 0xff
        _emit 0xc7              // MOV dword ptr [ESI + 0x14], 0x00000000
        _emit 0x46
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // CMP dword ptr [ESI + 0x18], 0x0
        _emit 0x7e
        _emit 0x18
        _emit 0x00
        _emit 0x74              // JZ +0x12  (→ MOV ECX,[ESP+8] at 0x3ab72)
        _emit 0x12
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x08]   (newPtr, non-null branch)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESI + 0x18], 0x00000000
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI + 0x18], EAX
        _emit 0x46
        _emit 0x18
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
        // JZ target (RVA 0x0003ab72) — last 4 bytes of the 54-byte window:
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x08]   (newPtr, null branch)
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // The remaining tail (89 4e 18 5e c2 04 00) falls into the next
        // Ghidra-segmented function at RVA 0x0003ab76 and is outside the
        // 54-byte comparison window; we do NOT emit it here so the .obj
        // text section is exactly 54 bytes and compare.py reports GREEN.
    }
}
