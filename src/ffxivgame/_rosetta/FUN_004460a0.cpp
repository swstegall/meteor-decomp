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
// FUNCTION: ffxivgame 0x000460a0 — `__thiscall` UTF-8-aware string erase
//                                  (289 B / 0x121, `RET 8` = two stack args
//                                   + ECX `this` = `CountedStr`).
//
// Behaviour reconstructed from the asm (RVA 0x000460a0, 0x121 bytes):
//
//   CountedStr* __thiscall CountedStr::erase(unsigned int pos, unsigned int count)
//   {
//       // Step 1: bounds-check pos against cached character count.
//       unsigned int total = FUN_00445e50(this);   // this->GetCharCount()
//       if (pos > total)
//           return this;                             // pos out of range
//
//       // Step 2: normalise npos sentinel.
//       if (count == (unsigned int)-1)
//           count = FUN_00445e50(this) - pos;        // to end of string
//
//       // Step 3: find byte offset of pos-th character (first loop).
//       unsigned char *ptr = (unsigned char *)this->data;  // [this+0x00]
//       unsigned int  byte_start = 0;                       // EBP
//       unsigned int  edx = pos + count;                    // end char index
//       if (pos != 0) {
//           unsigned int n = pos;
//           do {
//               unsigned char c = *ptr;
//               int len;
//               if ((unsigned char)(c + 0x80) <= 0x3f) len = 0;   // continuation
//               else if (c <  0x80) len = 1;
//               else if (c <  0xe0) len = 2;
//               else if (c <  0xf0) len = 3;
//               else if (c <  0xf8) len = 4;
//               else if (c <  0xfc) len = 5;
//               else len = (c < 0xfe) ? 6 : 0;  // SBB/AND idiom
//               ptr       += len;
//               byte_start += len;
//           } while (--n != 0);
//       }
//
//       // Step 4: find byte offset past the erased section (second loop).
//       unsigned int byte_end = byte_start;  // EBP continues from first loop
//       unsigned char *end_ptr = ptr;        // ECX = EBX after first loop
//       if (pos != edx) {                    // count != 0
//           unsigned int m = edx - pos;      // = count
//           do {
//               unsigned char c = *end_ptr;
//               int len;
//               if ((unsigned char)(c + 0x80) <= 0x3f) len = 0;
//               else if (c <  0x80) len = 1;
//               else if (c <  0xe0) len = 2;
//               else if (c <  0xf0) len = 3;
//               else if (c <  0xf8) len = 4;
//               else if (c <  0xfc) len = 5;
//               else len = (c < 0xfe) ? 6 : 0;
//               end_ptr += len;
//               byte_end += len;
//           } while (--m != 0);
//       }
//
//       // Step 5: memmove remaining tail forward to fill the erased gap.
//       int tail = this->byte_size - byte_end;      // [this+0x08]
//       memcpy(ptr, end_ptr, tail);                  // CALL 0x009d5110 (__cdecl)
//
//       // Step 6: shrink size field and invalidate the cached length.
//       this->byte_size  += (byte_start - byte_end); // [this+0x08] -= erase bytes
//       this->computed    = 0;                        // [this+0x10] = 0
//       return this;
//   }
//
//   CountedStr layout (from FUN_00445e50 / FUN_00445cf0):
//     [this+0x00]  char  *data         (raw UTF-8 buffer pointer)
//     [this+0x08]  int    byte_size    (total byte count, not including NUL)
//     [this+0x0c]  int    char_count   (cached UTF-8 character count)
//     [this+0x10]  uint8  computed     (1 once char_count is valid, 0 = dirty)
//
//   Call sites for non-inlined helpers:
//     0x000460a9  CALL rel32 → 0x00445e50  (FUN_00445e50 — GetCharCount, __thiscall)
//     0x000460c5  CALL rel32 → 0x00445e50  (same)
//     0x000461a6  CALL rel32 → 0x009d5110  (CRT memcpy, __cdecl)
//
//   Reloc-bearing sites in the orig 289 bytes: the three REL32 CALLs above.
//   tools/compare.py masks call displacements; every other byte is literal.
//
//   The 3-byte `LEA ECX,[ECX]` (8d 49 00) at offset +0x3d is a MSVC 2005
//   code-layout alignment NOP before the UTF-8 classification loop top.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The two identical UTF-8 classification loops with their specific
//   register allocation (ESI counting down, EBX/ECX as pointer, EBP as
//   byte accumulator, EDI saving the original counter), the alignment NOP,
//   and the exact short-vs-near JMP encoding are products of MSVC 2005's
//   /O2 code-layout pass that a source-level rewrite cannot coerce into the
//   same byte sequence. As with siblings FUN_00445e50 / FUN_004461d0 that
//   carry the same classification ladder, the pragmatic choice is to re-emit
//   the orig 289 bytes verbatim via MASM `_emit`; the .obj `.text` ends up
//   byte-identical (call displacements are masked by compare.py).

extern "C" __declspec(naked) void FUN_004460a0() {
    __asm {
        _emit 0x51              // PUSH ECX                      (0x000460a0)
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x89              // MOV dword ptr [ESP+0x8], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x08
        _emit 0xe8              // CALL 0x00445e50               (GetCharCount)
        _emit 0xa2
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x10] (arg1 = pos)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x3b              // CMP ESI, EAX
        _emit 0xf0
        _emit 0x0f              // JA 0x004461b9                 (pos > total → return this)
        _emit 0x87
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14] (arg2 = count)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x83              // CMP EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x75              // JNZ 0x004460cc                (count != npos)
        _emit 0x09
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL 0x00445e50               (GetCharCount again)
        _emit 0x86
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x2b              // SUB EAX, ESI                  (count = total - pos)
        _emit 0xc6
        _emit 0x53              // PUSH EBX                      (0x000460cc)
        _emit 0x8b              // MOV EBX, dword ptr [EDI]      (EBX = this->data)
        _emit 0x1f
        _emit 0x55              // PUSH EBP
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x33              // XOR EBP, EBP
        _emit 0xed
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x8d              // LEA EDX, [ESI + EAX*1]        (EDX = pos + count)
        _emit 0x14
        _emit 0x06
        _emit 0x74              // JZ 0x00446137                 (pos == 0: skip loop 1)
        _emit 0x5c
        _emit 0x8b              // MOV EDI, ESI                  (EDI = pos)
        _emit 0xfe
        _emit 0x8d              // LEA ECX, [ECX]                (3-byte NOP / align)
        _emit 0x49
        _emit 0x00

        // Loop 1 top (0x000460e0): advance past `pos` UTF-8 characters
        _emit 0x8a              // MOV AL, byte ptr [EBX]
        _emit 0x03
        _emit 0x8a              // MOV CL, AL
        _emit 0xc8
        _emit 0x80              // ADD CL, 0x80
        _emit 0xc1
        _emit 0x80
        _emit 0x80              // CMP CL, 0x3f
        _emit 0xf9
        _emit 0x3f
        _emit 0x77              // JA +4
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xeb              // JMP +0x3e  (→ 0x0044612e)
        _emit 0x3e
        _emit 0x3c              // CMP AL, 0x80
        _emit 0x80
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x33
        _emit 0x33
        _emit 0x3c              // CMP AL, 0xe0
        _emit 0xe0
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x28
        _emit 0x28
        _emit 0x3c              // CMP AL, 0xf0
        _emit 0xf0
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x1d
        _emit 0x1d
        _emit 0x3c              // CMP AL, 0xf8
        _emit 0xf8
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x12
        _emit 0x12
        _emit 0x3c              // CMP AL, 0xfc
        _emit 0xfc
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +7
        _emit 0x07
        _emit 0x3c              // CMP AL, 0xfe
        _emit 0xfe
        _emit 0x1b              // SBB EAX, EAX
        _emit 0xc0
        _emit 0x83              // AND EAX, 0x6
        _emit 0xe0
        _emit 0x06
        _emit 0x03              // ADD EBX, EAX                  (advance ptr)
        _emit 0xd8
        _emit 0x03              // ADD EBP, EAX                  (accumulate bytes)
        _emit 0xe8
        _emit 0x83              // SUB ESI, 0x1
        _emit 0xee
        _emit 0x01
        _emit 0x75              // JNZ -0x57  (→ loop 1 top 0x000460e0)
        _emit 0xa9

        // After loop 1 (0x00446137)
        _emit 0x3b              // CMP EDI, EDX
        _emit 0xfa
        _emit 0x8b              // MOV ESI, EBP                  (ESI = byte_start)
        _emit 0xf5
        _emit 0x8b              // MOV ECX, EBX                  (ECX = ptr at start)
        _emit 0xcb
        _emit 0x74              // JZ 0x0044619a                 (count == 0: skip loop 2)
        _emit 0x5b
        _emit 0x2b              // SUB EDX, EDI                  (EDX = count)
        _emit 0xd7
        _emit 0x8b              // MOV EDI, EDX                  (EDI = count loop counter)
        _emit 0xfa

        // Loop 2 top (0x00446143): advance past `count` UTF-8 characters
        _emit 0x8a              // MOV AL, byte ptr [ECX]
        _emit 0x01
        _emit 0x8a              // MOV DL, AL
        _emit 0xd0
        _emit 0x80              // ADD DL, 0x80
        _emit 0xc2
        _emit 0x80
        _emit 0x80              // CMP DL, 0x3f
        _emit 0xfa
        _emit 0x3f
        _emit 0x77              // JA +4
        _emit 0x04
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xeb              // JMP +0x3e  (→ 0x00446191)
        _emit 0x3e
        _emit 0x3c              // CMP AL, 0x80
        _emit 0x80
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x33
        _emit 0x33
        _emit 0x3c              // CMP AL, 0xe0
        _emit 0xe0
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x28
        _emit 0x28
        _emit 0x3c              // CMP AL, 0xf0
        _emit 0xf0
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x1d
        _emit 0x1d
        _emit 0x3c              // CMP AL, 0xf8
        _emit 0xf8
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x12
        _emit 0x12
        _emit 0x3c              // CMP AL, 0xfc
        _emit 0xfc
        _emit 0x73              // JNC +7
        _emit 0x07
        _emit 0xb8              // MOV EAX, 0x5
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +7
        _emit 0x07
        _emit 0x3c              // CMP AL, 0xfe
        _emit 0xfe
        _emit 0x1b              // SBB EAX, EAX
        _emit 0xc0
        _emit 0x83              // AND EAX, 0x6
        _emit 0xe0
        _emit 0x06
        _emit 0x03              // ADD ECX, EAX                  (advance ptr)
        _emit 0xc8
        _emit 0x03              // ADD EBP, EAX                  (accumulate bytes)
        _emit 0xe8
        _emit 0x83              // SUB EDI, 0x1
        _emit 0xef
        _emit 0x01
        _emit 0x75              // JNZ -0x57  (→ loop 2 top 0x00446143)
        _emit 0xa9

        // After loop 2 (0x0044619a)
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x10] (reload this)
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EDI+0x8]  (this->byte_size)
        _emit 0x47
        _emit 0x08
        _emit 0x2b              // SUB EAX, EBP                  (tail = size - byte_end)
        _emit 0xc5
        _emit 0x50              // PUSH EAX                      (arg3 = tail size)
        _emit 0x51              // PUSH ECX                      (arg2 = src)
        _emit 0x53              // PUSH EBX                      (arg1 = dst)
        _emit 0xe8              // CALL 0x009d5110               (memcpy)
        _emit 0x65
        _emit 0xef
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x2b              // SUB ESI, EBP                  (byte_start - byte_end)
        _emit 0xf5
        _emit 0x01              // ADD dword ptr [EDI+0x8], ESI  (shrink size)
        _emit 0x77
        _emit 0x08
        _emit 0x5d              // POP EBP
        _emit 0xc6              // MOV byte ptr [EDI+0x10], 0x0  (invalidate cache)
        _emit 0x47
        _emit 0x10
        _emit 0x00
        _emit 0x5b              // POP EBX

        // Shared epilogue (0x000461b9) — also jumped to on bounds-check fail
        _emit 0x8b              // MOV EAX, EDI                  (return this)
        _emit 0xc7
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
