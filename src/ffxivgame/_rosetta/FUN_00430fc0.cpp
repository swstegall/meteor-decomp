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
// FUNCTION: ffxivgame 0x00430fc0 — diagnostic/logging string formatter
//           (__cdecl, 180 B / 0xb4)
//
// Formats a message into a 2048-byte stack buffer and dispatches it to a
// log sink.  Takes 5 arguments; the conditional branch on arg5 selects
// between two /GF-pooled format strings:
//
//   arg5 != 0:  _snprintf_s(buf, 0x800, 0x7ff, fmt1, arg3, arg4, arg5, arg1, arg2)
//               where fmt1 is the rdata string at VA 0x00f63898
//   arg5 == 0:  _snprintf_s(buf, 0x800, 0x7ff, fmt2, arg3, arg4, arg1, arg2)
//               where fmt2 is the rdata string at VA 0x00f638b8
//
// The formatted buffer is then passed to the IAT function at [0x012651b4]
// as (buf, 6) — likely a structured log call (level 6 == informational).
//
// Prototype (logical, not byte-equivalent on its own):
//
//   void FUN_00430fc0(DWORD arg1, DWORD arg2, DWORD arg3,
//                     DWORD arg4, DWORD arg5);
//
// Why naked asm: the /GS epilog contains an unusual
//   MOV dword ptr [reloc_addr], 0
// write before the CALL to __security_check_cookie — a pattern that cannot
// be driven from plain C++ under /O2 /GS.  The arg1/arg2 pre-push idiom
// (both pushed before the JZ as common trailing varargs) also depends on a
// specific compiler register-allocation tiebreak that is not reproducible
// from an isolated TU.  Naked asm with _emit directives gives compare.py a
// byte-identical .text section (modulo the reloc windows it wildcards).
//
// Reloc-bearing offsets within the 180 bytes:
//   +0x07   MOV EAX, [__security_cookie]  (.data 0x012ea8b0)
//   +0x42   PUSH imm32 → fmt1             (.rdata 0x00f63898)
//   +0x56   CALL rel32 → _snprintf_s-like (.text 0x009d4f9f)
//   +0x69   PUSH imm32 → fmt2             (.rdata 0x00f638b8)
//   +0x7d   CALL rel32 → same             (.text 0x009d4f9f)
//   +0x8d   CALL [IAT]                    (.idata 0x012651b4)
//   +0xa0   MOV [reloc], 0               (unknown .data slot)
//   +0xa9   CALL rel32 → __security_check_cookie (.text 0x009d20f4)

extern "C" __declspec(naked) void FUN_00430fc0()
{
    __asm {
        // --- /GS prolog: allocate 0x804-byte frame, save cookie ------------
        _emit 0x81              // SUB ESP, 0x804
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xa1              // MOV EAX, [__security_cookie]  (reloc +0x07)
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

        // --- pre-fetch arguments -------------------------------------------
        _emit 0x8b              // MOV EDX, [ESP+0x80c]  (arg2)
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESP+0x818]  (arg5)
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ECX, [ESP+0x808]  (arg1)
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x814]  (arg3)
        _emit 0xb4
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00

        // --- pre-push common trailing varargs (arg2, arg1) ----------------
        _emit 0x52              // PUSH EDX  (arg2 — pushed deepest)
        _emit 0x51              // PUSH ECX  (arg1)

        // --- branch on arg5 -----------------------------------------------
        _emit 0x74              // JZ +0x28  (to zero-arg5 path)
        _emit 0x28

        // --- non-zero path: snprintf(buf,0x800,0x7ff,fmt1,arg3,arg4,arg5,arg1,arg2)
        _emit 0x50              // PUSH EAX  (arg5)
        _emit 0x8b              // MOV EAX, [ESP+0x824]  (arg4)
        _emit 0x84
        _emit 0x24
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (arg4)
        _emit 0x56              // PUSH ESI  (arg3)
        _emit 0x68              // PUSH imm32 → fmt1  (reloc +0x42)
        _emit 0x98
        _emit 0x38
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff  (count)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x20]  → buf
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x68              // PUSH 0x800  (bufsize)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX  (buf)
        _emit 0xe8              // CALL _snprintf_s-like  (reloc +0x56)
        _emit 0x85
        _emit 0x3f
        _emit 0x5a
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x24  (9 args × 4)
        _emit 0xc4
        _emit 0x24
        _emit 0xeb              // JMP +0x25  (to tail)
        _emit 0x25

        // --- zero path: snprintf(buf,0x800,0x7ff,fmt2,arg3,arg4,arg1,arg2)
        _emit 0x8b              // MOV EDX, [ESP+0x820]  (arg4)
        _emit 0x94
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX  (arg4)
        _emit 0x56              // PUSH ESI  (arg3)
        _emit 0x68              // PUSH imm32 → fmt2  (reloc +0x69)
        _emit 0xb8
        _emit 0x38
        _emit 0xf6
        _emit 0x00
        _emit 0x68              // PUSH 0x7ff
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP+0x1c]  → buf
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x68              // PUSH 0x800
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (buf)
        _emit 0xe8              // CALL _snprintf_s-like  (reloc +0x7d)
        _emit 0x5e
        _emit 0x3f
        _emit 0x5a
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x20  (8 args × 4)
        _emit 0xc4
        _emit 0x20

        // --- common tail: dispatch to log sink ----------------------------
        _emit 0x8d              // LEA ECX, [ESP+0x4]  → buf
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x6a              // PUSH 6  (log level)
        _emit 0x06
        _emit 0x51              // PUSH ECX  (buf)
        _emit 0xff              // CALL [0x012651b4]  (reloc +0x8d)
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01

        // --- /GS epilog ---------------------------------------------------
        _emit 0x8b              // MOV ECX, [ESP+0x80c]  (saved cookie)
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x8  (pop 2 log-sink args)
        _emit 0xc4
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0x33              // XOR ECX, ESP
        _emit 0xcc
        _emit 0xc7              // MOV dword ptr [reloc], 0  (reloc +0xa0)
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL __security_check_cookie  (reloc +0xa9)
        _emit 0x87
        _emit 0x10
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
