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
// FUNCTION: ffxivgame 0x0004dcf0 — FUN_0044dcf0 (__cdecl void, 180 B / 0xb4)
//
// Operates on two global ring-buffer / container objects at statics
// 0x0132cf50 ("buf0") and 0x0132cf60 ("buf1"). Each object carries a
// current-pointer field at +4 and an end-pointer field at +8.
//
// Pseudo-code:
//
//   void FUN_0044dcf0() {
//       // --- guard buf0 range ----------------------------------------
//       unsigned end0 = g_buf0_end;          // [0x0132cf58]
//       unsigned cur0 = g_buf0_cur;          // [0x0132cf54]
//       if (cur0 > end0) {
//           FUN_009d22b4();                  // assertion / wait
//           end0 = g_buf0_end;
//           cur0 = g_buf0_cur;
//       }
//       if (cur0 > end0)
//           FUN_009d22b4();
//       // thiscall on buf0 (5 args incl. this, cleans its stack args)
//       FUN_00444dc0(&buf0, &local, &buf0, cur0, &buf0, end0);
//       // --- guard buf1 range ----------------------------------------
//       unsigned end1 = g_buf1_end;          // [0x0132cf68]
//       unsigned cur1 = g_buf1_cur;          // [0x0132cf64]
//       if (cur1 > end1) {
//           FUN_009d22b4();
//           end1 = g_buf1_end;
//           cur1 = g_buf1_cur;
//       }
//       if (cur1 > end1)
//           FUN_009d22b4();
//       // thiscall on buf1 (5 args incl. this, cleans its stack args)
//       FUN_0071cc50(&buf1, &local, &buf1, cur1, &buf1, end1);
//       // --- dispatch with the value written into `local` by buf0 call
//       unsigned val = local;
//       FUN_0044e760(&buf0, val);
//       FUN_0093ead0(&buf1, val);
//   }
//
// Why naked asm: the compiler emits the first `MOV EAX, [global]` load
// BEFORE the `SUB ESP, 0x8` frame adjustment — an optimizer scheduling
// artifact only reproducible by re-emitting the original bytes verbatim.
// The two thiscall targets (0x00444dc0, 0x0071cc50) and the four final
// direct CALLs each carry a CALL rel32 reloc masked by compare.py.
// All global address immediates in `MOV EAX/ECX/ESI, moffs/imm32` are
// similarly reloc-masked.
//
// Reloc-bearing sites (offsets within function body):
//   +0x01   [0x0132cf58] moffs32          (g_buf0_end)
//   +0x0d   [0x0132cf54] ModRM disp32     (g_buf0_cur)
//   +0x18   CALL rel32 → 0x009d22b4       (guard helper)
//   +0x1d   [0x0132cf58] moffs32
//   +0x23   [0x0132cf54] ModRM disp32
//   +0x2a   imm32 0x0132cf50              (MOV ESI)
//   +0x31   CALL rel32 → 0x009d22b4
//   +0x39   imm32 0x0132cf50              (MOV EAX, push arg)
//   +0x44   imm32 0x0132cf50              (MOV ECX = this)
//   +0x49   CALL rel32 → 0x00444dc0
//   +0x4e   [0x0132cf68] moffs32          (g_buf1_end)
//   +0x54   [0x0132cf64] ModRM disp32     (g_buf1_cur)
//   +0x5f   CALL rel32 → 0x009d22b4
//   +0x64   [0x0132cf68] moffs32
//   +0x6a   [0x0132cf64] ModRM disp32
//   +0x71   imm32 0x0132cf60              (MOV ESI)
//   +0x7a   CALL rel32 → 0x009d22b4
//   +0x81   imm32 0x0132cf60              (MOV EAX, push arg)
//   +0x8f   CALL rel32 → 0x0071cc50
//   +0x99   imm32 0x0132cf50              (MOV ECX = this buf0)
//   +0x9e   CALL rel32 → 0x0044e760
//   +0xa4   imm32 0x0132cf60              (MOV ECX = this buf1)
//   +0xa9   CALL rel32 → 0x0093ead0

extern "C" __declspec(naked) void FUN_0044dcf0() {
    __asm {
        // --- load g_buf0_end into EAX (before frame adjust) ------------
        _emit 0xa1              // MOV EAX, [0x0132cf58]
        _emit 0x58
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // --- frame setup ------------------------------------------------
        _emit 0x83              // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        // --- load g_buf0_cur into EDI; guard check #1 -------------------
        _emit 0x8b              // MOV EDI, dword ptr [0x0132cf54]
        _emit 0x3d
        _emit 0x54
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0x76              // JBE +0x10 (skip if cur <= end)
        _emit 0x10
        _emit 0xe8              // CALL 0x009d22b4 (guard/assert helper)
        _emit 0xa8
        _emit 0x45
        _emit 0x58
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x0132cf58]   (reload end)
        _emit 0x58
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EDI, dword ptr [0x0132cf54] (reload cur)
        _emit 0x3d
        _emit 0x54
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // --- guard check #2 (after potential reload) --------------------
        _emit 0x3b              // CMP EDI, EAX
        _emit 0xf8
        _emit 0xbe              // MOV ESI, 0x0132cf50  (buf0 base addr)
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x76              // JBE +0x05 (skip if cur <= end)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4
        _emit 0x8f
        _emit 0x45
        _emit 0x58
        _emit 0x00
        // --- call FUN_00444dc0 (thiscall, 5 stack args) -----------------
        _emit 0x53              // PUSH EBX   (arg5 = end0)
        _emit 0x56              // PUSH ESI   (arg4 = &buf0)
        _emit 0x57              // PUSH EDI   (arg3 = cur0)
        _emit 0xb8              // MOV EAX, 0x0132cf50
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x50              // PUSH EAX   (arg2 = &buf0)
        _emit 0x8d              // LEA EAX, [ESP+0x1c]  (= &local)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x50              // PUSH EAX   (arg1 = &local)
        _emit 0xb9              // MOV ECX, 0x0132cf50  (this = buf0)
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL 0x00444dc0
        _emit 0x83
        _emit 0x70
        _emit 0xff
        _emit 0xff
        // --- load g_buf1_end into EAX; buf1 guard check #1 -------------
        _emit 0xa1              // MOV EAX, [0x0132cf68]
        _emit 0x68
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [0x0132cf64]
        _emit 0x0d
        _emit 0x64
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0x76              // JBE +0x10
        _emit 0x10
        _emit 0xe8              // CALL 0x009d22b4
        _emit 0x61
        _emit 0x45
        _emit 0x58
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x0132cf68]   (reload end)
        _emit 0x68
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [0x0132cf64] (reload cur)
        _emit 0x0d
        _emit 0x64
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // --- buf1 guard check #2 ----------------------------------------
        _emit 0x3b              // CMP ECX, EAX
        _emit 0xc8
        _emit 0xbe              // MOV ESI, 0x0132cf60  (buf1 base addr)
        _emit 0x60
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EDI, ECX   (EDI = cur1)
        _emit 0xf9
        _emit 0x76              // JBE +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4
        _emit 0x46
        _emit 0x45
        _emit 0x58
        _emit 0x00
        // --- call FUN_0071cc50 (thiscall, 5 stack args) -----------------
        _emit 0x53              // PUSH EBX   (arg5 = end1)
        _emit 0x56              // PUSH ESI   (arg4 = &buf1)
        _emit 0xb8              // MOV EAX, 0x0132cf60
        _emit 0x60
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x57              // PUSH EDI   (arg3 = cur1)   ← after MOV EAX
        _emit 0x50              // PUSH EAX   (arg2 = &buf1)
        _emit 0x8d              // LEA ECX, [ESP+0x1c]  (= &local)
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x51              // PUSH ECX   (arg1 = &local)
        _emit 0x8b              // MOV ECX, EAX  (this = buf1)
        _emit 0xc8
        _emit 0xe8              // CALL 0x0071cc50
        _emit 0xcd
        _emit 0xee
        _emit 0x2c
        _emit 0x00
        // --- read local written by buf0 call, dispatch ------------------
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x18]  (= local value)
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x56              // PUSH ESI
        _emit 0xb9              // MOV ECX, 0x0132cf50  (this = buf0)
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL 0x0044e760
        _emit 0xce
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0xb9              // MOV ECX, 0x0132cf60  (this = buf1)
        _emit 0x60
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL 0x0093ead0
        _emit 0x33
        _emit 0x0d
        _emit 0x4f
        _emit 0x00
        // --- epilogue ---------------------------------------------------
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
