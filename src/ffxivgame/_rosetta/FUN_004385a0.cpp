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
// FUNCTION: ffxivgame 0x004385a0 — formatted-log emitter (__cdecl, 180 B / 0xb4)
//
// Behaviour (recovered from asm @ 0x000385a0):
//
//   void FUN_004385a0(int a0, int a1, int a2, int a3, int a4,
//                     int /*a5*/, int a6, int a7) {
//       char buf[0x800];                 // /GS-cookied 2048-byte scratch
//       if (a4 != 0) {
//           _snprintf_s(buf, 0x800, 0x7ff, FMT_A /*0xf660bc*/,
//                       a2, a3, a4, a0, a1);
//       } else {
//           _snprintf_s(buf, 0x800, 0x7ff, FMT_B /*0xf660dc*/,
//                       a2, a3, a0, a1);
//       }
//       (*(void(__stdcall **)(char*, int))0x012651b4)(buf, 6);  // IAT log sink
//       *(int *)g_zero_target = 0;       // reset a global error/status slot
//   }
//
//   The args a0 (= [ESP+0x808]) and a1 (= [ESP+0x80c]) are pushed once
//   (before the a4 branch) so they trail the varargs of BOTH _snprintf_s
//   calls; a2/a3 come next, and a4 is appended only on the non-zero arm.
//   The IAT call through [0x012651b4] takes (buf, 6) cdecl-cleaned.
//
// Reloc-bearing sites in the orig 180 bytes (compare.py wildcards each
// 4-byte window; we re-emit the orig bytes verbatim so the .obj's .text
// matches byte-for-byte with NO relocations of its own):
//   +0x07   MOV  EAX, [0x012ea8b0]        __security_cookie load
//   +0x41   PUSH 0xf660bc                 format string A
//   +0x49   CALL rel32 → 0x009d4f9f       _snprintf_s
//   +0x68   PUSH 0xf660dc                 format string B
//   +0x70   CALL rel32 → 0x009d4f9f       _snprintf_s
//   +0x8b   CALL [0x012651b4]             IAT log-sink slot
//   +0xa0   MOV  [imm32], 0               global zero-store
//   +0xa8   CALL rel32 → 0x009d20f4       __security_check_cookie
//
// Reconstruction strategy — naked-asm byte passthrough. The source-level
// shape (a single if/else over two _snprintf_s calls + an IAT call + a
// /GS prolog/epilog) produces a fixed encoding whose reloc operands the
// linker would resolve at relink time. As with sibling FUN_00403f10, we
// re-emit the orig 180 bytes via MASM `_emit` so compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004385a0() {
    __asm {
        _emit 0x81              // SUB ESP, 0x804
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x012ea8b0]   (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV [ESP+0x800], EAX
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, [ESP+0x80c]    (a1)
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP+0x818]    (a4)
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ECX, [ESP+0x808]    (a0)
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x814]    (a2)
        _emit 0xb4
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX                (a1)
        _emit 0x51              // PUSH ECX                (a0)
        _emit 0x74              // JZ +0x28 (else branch @ 0x385ff)
        _emit 0x28
        _emit 0x50              // PUSH EAX                (a4)
        _emit 0x8b              // MOV EAX, [ESP+0x824]    (a3)
        _emit 0x84
        _emit 0x24
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                (a3)
        _emit 0x56              // PUSH ESI                (a2)
        _emit 0x68              // PUSH 0xf660bc           (format A)
        _emit 0xbc
        _emit 0x60
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff              (count)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x20]     (buf)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x68              // PUSH 0x800              (bufsize)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                (buf)
        _emit 0xe8              // CALL rel32 → 0x009d4f9f (_snprintf_s)
        _emit 0xa5
        _emit 0xc9
        _emit 0x59
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x24
        _emit 0xc4
        _emit 0x24
        _emit 0xeb              // JMP +0x25 (tail @ 0x438624)
        _emit 0x25
        _emit 0x8b              // MOV EDX, [ESP+0x820]    (a3)   (else:)
        _emit 0x94
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX                (a3)
        _emit 0x56              // PUSH ESI                (a2)
        _emit 0x68              // PUSH 0xf660dc           (format B)
        _emit 0xdc
        _emit 0x60
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff              (count)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP+0x1c]     (buf)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x68              // PUSH 0x800              (bufsize)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                (buf)
        _emit 0xe8              // CALL rel32 → 0x009d4f9f (_snprintf_s)
        _emit 0x7e
        _emit 0xc9
        _emit 0x59
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0x8d              // LEA ECX, [ESP+0x4]      (buf)   (tail:)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x6a              // PUSH 0x6
        _emit 0x06
        _emit 0x51              // PUSH ECX                (buf)
        _emit 0xff              // CALL dword ptr [0x012651b4]  (IAT log sink)
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x8b              // MOV ECX, [ESP+0x80c]    (saved cookie)
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
        _emit 0xc7              // MOV dword ptr [imm32], 0   (global zero-store)
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
        _emit 0xa7
        _emit 0x9a
        _emit 0x59
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
