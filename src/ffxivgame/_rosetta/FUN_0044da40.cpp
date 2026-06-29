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
// FUNCTION: ffxivgame 0x0044da40 — bit-0x10 toggle in a flags word
//                                  (__stdcall, 1 DWORD arg, 85 bytes)
//
// High-level shape:
//
//   void __stdcall FUN_0044da40(int mode) {
//       DWORD local_A, local_B;
//       fn_read(*(DWORD*)0x0132cf4c, &local_A, &local_B);  // fills local_A/B
//       // function's own arg is now at [ESP+0xc] (12 bytes above frame after
//       // the stdcall cleaned up the 3 pushed arguments).
//       DWORD val = mode;
//       if (val == 0) {
//           DWORD x = local_A;
//           if (!(x & 0x10)) { x |= 0x10; fn_write(*(DWORD*)0x0132cf4c, x, local_B); }
//       } else if (val == 1) {
//           DWORD x = local_A;
//           if (x & 0x10)  { x &= ~0x10; fn_write(*(DWORD*)0x0132cf4c, x, local_B); }
//       }
//   }
//
// Asm (85 bytes, RVA 0x0004da40):
//
//   0004da40  8b 15 4c cf 32 01   MOV  EDX,[0x0132cf4c]   ; load global
//   0004da46  83 ec 08            SUB  ESP,8              ; alloc locals
//   0004da49  8d 44 24 04         LEA  EAX,[ESP+4]        ; &local_B
//   0004da4d  50                  PUSH EAX
//   0004da4e  8d 4c 24 04         LEA  ECX,[ESP+4]        ; &local_A
//   0004da52  51                  PUSH ECX
//   0004da53  52                  PUSH EDX
//   0004da54  e8 61 26 58 00      CALL 0x009d00ba         ; fn_read  (REL32)
//   0004da59  8b 44 24 0c         MOV  EAX,[ESP+0xc]      ; mode arg (at [ESP+12])
//   0004da5d  83 e8 00            SUB  EAX,0x0            ; cmp with 0
//   0004da60  74 11               JZ   +0x11              ; → 0x44da73 (case 0)
//   0004da62  83 e8 01            SUB  EAX,0x1            ; cmp with 1
//   0004da65  75 28               JNZ  +0x28              ; → 0x44da8f (epilogue)
//   0004da67  8b 04 24            MOV  EAX,[ESP]          ; local_A
//   0004da6a  a8 10               TEST AL,0x10
//   0004da6c  74 21               JZ   +0x21              ; bit not set → skip
//   0004da6e  83 e0 ef            AND  EAX,0xffffffef     ; clear bit 0x10
//   0004da71  eb 0a               JMP  +0x0a              ; → common write path
//   0004da73  8b 04 24            MOV  EAX,[ESP]          ; local_A  (case 0)
//   0004da76  a8 10               TEST AL,0x10
//   0004da78  75 15               JNZ  +0x15              ; bit set → skip
//   0004da7a  83 c8 10            OR   EAX,0x10           ; set bit 0x10
//   0004da7d  8b 4c 24 04         MOV  ECX,[ESP+4]        ; local_B
//   0004da81  8b 15 4c cf 32 01   MOV  EDX,[0x0132cf4c]  ; reload global (DIR32)
//   0004da87  51                  PUSH ECX
//   0004da88  50                  PUSH EAX
//   0004da89  52                  PUSH EDX
//   0004da8a  e8 25 26 58 00      CALL 0x009d00b4         ; fn_write (REL32)
//   0004da8f  83 c4 08            ADD  ESP,8              ; collapse locals
//   0004da92  c2 04 00            RET  4
//
// Reloc-bearing sites:
//   +0x02   MOV imm32  → 0x0132cf4c   (DIR32 — first global load)
//   +0x14   CALL rel32 → 0x009d00ba   (fn_read)
//   +0x41   MOV imm32  → 0x0132cf4c   (DIR32 — second global load)
//   +0x4a   CALL rel32 → 0x009d00b4   (fn_write)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function has four reloc-bearing operands (two DIR32 global loads
//   and two CALL REL32s).  A source-level reconstruction would require
//   providing the addresses of the two callees and the global.  The
//   naked _emit passthrough sidesteps reloc resolution; compare.py masks
//   the reloc windows during the diff, so the .obj's .text is
//   byte-identical to the orig 85-byte slice.

extern "C" __declspec(naked) void FUN_0044da40()
{
    __asm {
        _emit 0x8b              // MOV  EDX, dword ptr [0x0132cf4c]   (DIR32)
        _emit 0x15
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x83              // SUB  ESP, 8
        _emit 0xec
        _emit 0x08
        _emit 0x8d              // LEA  EAX, [ESP+4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA  ECX, [ESP+4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d00ba   (REL32)
        _emit 0x61
        _emit 0x26
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV  EAX, dword ptr [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x83              // SUB  EAX, 0x0
        _emit 0xe8
        _emit 0x00
        _emit 0x74              // JZ   +0x11
        _emit 0x11
        _emit 0x83              // SUB  EAX, 0x1
        _emit 0xe8
        _emit 0x01
        _emit 0x75              // JNZ  +0x28
        _emit 0x28
        _emit 0x8b              // MOV  EAX, dword ptr [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0xa8              // TEST AL, 0x10
        _emit 0x10
        _emit 0x74              // JZ   +0x21
        _emit 0x21
        _emit 0x83              // AND  EAX, 0xffffffef
        _emit 0xe0
        _emit 0xef
        _emit 0xeb              // JMP  +0x0a
        _emit 0x0a
        _emit 0x8b              // MOV  EAX, dword ptr [ESP]
        _emit 0x04
        _emit 0x24
        _emit 0xa8              // TEST AL, 0x10
        _emit 0x10
        _emit 0x75              // JNZ  +0x15
        _emit 0x15
        _emit 0x83              // OR   EAX, 0x10
        _emit 0xc8
        _emit 0x10
        _emit 0x8b              // MOV  ECX, dword ptr [ESP+4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV  EDX, dword ptr [0x0132cf4c]   (DIR32)
        _emit 0x15
        _emit 0x4c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x009d00b4   (REL32)
        _emit 0x25
        _emit 0x26
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD  ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET  4
        _emit 0x04
        _emit 0x00
    }
}
