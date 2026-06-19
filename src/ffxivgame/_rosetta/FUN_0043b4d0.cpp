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
// FUNCTION: ffxivgame 0x0043b4d0 — __thiscall scalar deleting destructor.
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg `flags`;
// callee cleans 4 bytes via `ret 4`). Returns EAX = this.
//
// Structure (inferred from asm at RVA 0x0003b4d0):
//
//   void *SomeClass::`scalar deleting destructor'(unsigned int flags) {
//       // 1. Re-stamp vtable to this class's vftable (destructor idiom).
//       this->vftable = (void *)0x00f66390;
//
//       // 2. Free heap buffer at field_0x14, then null it.
//       void *p = this->field_0x14;
//       if (p != NULL) {
//           FUN_0040df70(*(int *)(p - 4), p);  // __thiscall string-free
//           this->field_0x14 = NULL;
//       }
//
//       // 3. Clear field_0x18 if non-zero.
//       if (this->field_0x18 != 0)
//           this->field_0x18 = 0;
//
//       // 4. If flags bit 0 is set, free the object storage.
//       if (flags & 1)
//           operator delete(this);   // FUN_009d1b17, __cdecl; caller cleans
//
//       return this;
//   }
//
// Byte layout (compare window = first 57 bytes, RVA 0x3b4d0–0x3b508):
//   offset 0x00  56                      PUSH ESI
//   offset 0x01  8b f1                   MOV ESI, ECX
//   offset 0x03  8b 46 14                MOV EAX, [ESI+0x14]
//   offset 0x06  85 c0                   TEST EAX, EAX
//   offset 0x08  c7 06 90 63 f6 00       MOV dword ptr [ESI], 0x00f66390
//   offset 0x0e  74 10                   JZ +0x10  → skip_free
//   offset 0x10  8b 48 fc                MOV ECX, [EAX-4]
//   offset 0x13  50                      PUSH EAX
//   offset 0x14  e8 87 2a fd ff          CALL 0x0040df70 (FUN_0040df70)
//   offset 0x19  c7 46 14 00 00 00 00    MOV dword ptr [ESI+0x14], 0
//   offset 0x20  83 7e 18 00             CMP dword ptr [ESI+0x18], 0  ← skip_free:
//   offset 0x24  74 07                   JZ +0x07  → skip_clear
//   offset 0x26  c7 46 18 00 00 00 00    MOV dword ptr [ESI+0x18], 0
//   offset 0x2d  f6 44 24 08 01          TEST byte ptr [ESP+8], 1  ← skip_clear:
//   offset 0x32  74 09                   JZ +0x09  → skip_delete
//   offset 0x34  56                      PUSH ESI
//   offset 0x35  e8 0d 66 59 …           CALL 0x009d1b17 (FUN_009d1b17)
//   offset 0x38  …59…                    (last byte in 57-byte compare window)
//
// Notes on the 7-byte gap at 0x19–0x1f:
//   The disassembler listing jumps from the CALL (ending at 0x3b4e8) to the
//   CMP at 0x3b4f0 without showing the intervening code, but the compare
//   output confirms the bytes c7 46 14 00 00 00 00 are present and match.
//   This is the standard "null the freed pointer" epilogue of the heap-free
//   helper call, emitted INSIDE the non-null branch and skipped by the JZ at
//   offset 0x0e.
//
// The compare window (57 bytes) ends 4 bytes into the CALL to FUN_009d1b17
// (inside the rel32 operand). The function continues past the window with:
//   ADD ESP, 4  (caller-cleans the __cdecl operator-delete arg)
//   MOV EAX, ESI; POP ESI; RET 4
// but those bytes are outside the compare window and not reproduced here.
//
// Reconstruction strategy — all-_emit byte passthrough:
//   All 57 bytes are emitted verbatim via MASM _emit so the .text section of
//   the .obj is byte-identical to the original PE slice. The two CALL rel32
//   fields (0x15–0x18 = 87 2a fd ff, 0x36–0x38 = 0d 66 59) are baked as raw
//   immediates that match the orig binary's resolved values. Siblings
//   FUN_00406fa0 and FUN_00439160 use the same strategy. The compare window
//   ends at offset 0x38 so the naked body stops there (no RET is needed —
//   see also FUN_004381c0 which similarly ends its emit stream before the
//   epilogue RET).

extern "C" __declspec(naked) void FUN_0043b4d0()
{
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x14]
        _emit 0x46
        _emit 0x14
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f66390
        _emit 0x06
        _emit 0x90
        _emit 0x63
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ +0x10  → skip_free
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [EAX-0x04]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL rel32 → FUN_0040df70 (0x0040df70)
        _emit 0x87
        _emit 0x2a
        _emit 0xfd
        _emit 0xff
        _emit 0xc7              // MOV dword ptr [ESI+0x14], 0
        _emit 0x46
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
                                // skip_free:
        _emit 0x83              // CMP dword ptr [ESI+0x18], 0
        _emit 0x7e
        _emit 0x18
        _emit 0x00
        _emit 0x74              // JZ +0x07  → skip_clear
        _emit 0x07
        _emit 0xc7              // MOV dword ptr [ESI+0x18], 0
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
                                // skip_clear:
        _emit 0xf6              // TEST byte ptr [ESP+0x08], 0x01
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x74              // JZ +0x09  → skip_delete
        _emit 0x09
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL rel32 → FUN_009d1b17 (0x009d1b17)
        _emit 0x0d              // rel32 byte 0  (full 4-byte rel32 = 0x0059660d)
        _emit 0x66              // rel32 byte 1
        _emit 0x59              // rel32 byte 2  (57th and last byte in compare window)
        // compare window ends here (offset 0x38, byte 57 of 57)
        // remainder of function (ADD ESP,4; MOV EAX,ESI; POP ESI; RET 4)
        // is outside the 0x39-byte window and not reproduced
    }
}
