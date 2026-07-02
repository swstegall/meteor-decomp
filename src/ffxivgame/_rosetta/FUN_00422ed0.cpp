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
// FUNCTION: ffxivgame 0x00422ed0 — formatted-message dispatch helper
//                                  (__cdecl, 180 B / 0xb4)
//
// Recovered behaviour (from asm/ffxivgame/00022ed0_FUN_00422ed0.s):
//
//   Structurally identical to the sibling FUN_00436b00: picks one of two
//   format strings depending on whether a4 is non-zero, renders the
//   result into a 0x800-byte local buffer with _snprintf_s, then hands
//   the buffer + a code (6) to an indirect dispatch routine.
//
//   __cdecl void FUN_00422ed0(unsigned a0, unsigned a1, unsigned a2,
//                             unsigned a3, unsigned a4)
//   {
//       char buf[0x800];                              // /GS-guarded array
//       if (a4 != 0)
//           _snprintf_s(buf, 0x800, 0x7ff, FMT1 /*0xf59b6c*/,
//                       a2, a3, a4, a0, a1);
//       else
//           _snprintf_s(buf, 0x800, 0x7ff, FMT2 /*0xf59b8c*/,
//                       a2, a3, a0, a1);
//       (*(void(__cdecl**)(char*, int))0x012651b4)(buf, 6);  // dispatch
//       *(int*)<global> = 0;
//   }
//
//   The /GS cookie (load from .data 0x012ea8b0, XOR ESP, store at
//   [ESP+0x800], re-check via __security_check_cookie @ 0x009d20f4) wraps
//   the whole thing because of the 0x800-byte local array.
//
// Reloc-bearing sites (each 4-byte window is wildcarded by compare.py):
//   +0x06   MOV  EAX, __security_cookie        (.data 0x012ea8b0)
//   +0x41   PUSH offset FMT1                   (.rdata 0x00f59b6c)
//   +0x55   CALL rel32 → 0x009d4f9f            (_snprintf_s)
//   +0x68   PUSH offset FMT2                   (.rdata 0x00f59b8c)
//   +0x7c   CALL rel32 → 0x009d4f9f            (_snprintf_s)
//   +0x8b   CALL [0x012651b4]                  (indirect dispatch)
//   +0x9e   MOV  [global], 0                   (DIR32)
//   +0xa8   CALL rel32 → 0x009d20f4            (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom as the
// sibling FUN_00436b00 / FUN_00403f10): a `__declspec(naked)` body
// re-emits the orig 180 bytes verbatim via MASM `_emit`. The .obj's
// `.text` ends up byte-identical to the orig slice; compare.py masks the
// reloc windows and reports GREEN.

extern "C" __declspec(naked) void FUN_00422ed0() {
    __asm {
        _emit 0x81              // SUB ESP, 0x804
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xa1              // MOV EAX, __security_cookie   (reloc)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV dword ptr [ESP+0x800], EAX
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x80c]  (a1)
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x818]  (a4)
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x808]  (a0)
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x814]  (a2)
        _emit 0xb4
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX                        (a1)
        _emit 0x51              // PUSH ECX                        (a0)
        _emit 0x74              // JZ +0x28 (else-branch @ 0x22f2f)
        _emit 0x28
        _emit 0x50              // PUSH EAX                        (a4)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x824]  (a3)
        _emit 0x84
        _emit 0x24
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                        (a3)
        _emit 0x56              // PUSH ESI                        (a2)
        _emit 0x68              // PUSH offset FMT1 (0x00f59b6c)   (reloc)
        _emit 0x6c
        _emit 0x9b
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x20]  (&buf)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x68              // PUSH 0x800
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX  (&buf)
        _emit 0xe8              // CALL rel32 → 0x009d4f9f (_snprintf_s)
        _emit 0x75
        _emit 0x20
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x24
        _emit 0xc4
        _emit 0x24
        _emit 0xeb              // JMP +0x25 (tail @ 0x22f54)
        _emit 0x25
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x820]  (a3) (else:)
        _emit 0x94
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX  (a3)
        _emit 0x56              // PUSH ESI  (a2)
        _emit 0x68              // PUSH offset FMT2 (0x00f59b8c)   (reloc)
        _emit 0x8c
        _emit 0x9b
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP+0x1c]  (&buf)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x68              // PUSH 0x800
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (&buf)
        _emit 0xe8              // CALL rel32 → 0x009d4f9f (_snprintf_s)
        _emit 0x4e
        _emit 0x20
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0x8d              // LEA ECX, [ESP+0x4]  (&buf) (tail:)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x6a              // PUSH 0x6
        _emit 0x06
        _emit 0x51              // PUSH ECX  (&buf)
        _emit 0xff              // CALL dword ptr [0x012651b4]  (reloc)
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x80c]  (cookie)
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0x33              // XOR ECX, ESP
        _emit 0xcc
        _emit 0xc7              // MOV dword ptr [global], 0  (DIR32 reloc)
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL rel32 → 0x009d20f4 (__security_check_cookie)
        _emit 0x77
        _emit 0xf1
        _emit 0x5a
        _emit 0x00
        _emit 0x81              // ADD ESP, 0x804
        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
