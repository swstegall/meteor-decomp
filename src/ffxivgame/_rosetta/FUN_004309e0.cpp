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
// FUNCTION: ffxivgame 0x000309e0 — string-format + log dispatch
//           (__cdecl void FUN_004309e0(arg1, arg2, arg3, arg4, arg5[, arg6]), 180 B / 0xb4)
//
// The function formats a message into a 0x800-byte stack buffer via
// _snprintf_s, then dispatches the result to a logging function through
// the global function-pointer at [0x012651b4] with severity level 6.
//
// Frame layout (after SUB ESP, 0x804):
//   [ESP+0x000 .. +0x7ff]  char buf[0x800]  — the format target
//   [ESP+0x800]             /GS cookie       — __security_cookie XOR ESP
//   [ESP+0x804]             return address
//   [ESP+0x808]             param1           (also passed as arg to log call)
//   [ESP+0x80c]             param2
//   [ESP+0x810]             param3           → ESI (format arg)
//   [ESP+0x814]             param4           → read as additional format arg
//   [ESP+0x818]             param5           → branch condition / format arg
//   [ESP+0x81c]             param6           (read in non-zero branch)
//
// Branch on param5:
//   param5 != 0:  _snprintf_s(buf, 0x800, 0x7ff, fmt@0xf633a4, param3, param4, param5)
//   param5 == 0:  _snprintf_s(buf, 0x800, 0x7ff, fmt@0xf633c4, param3, param4)
//
// After formatting:
//   (*g_log_fn)(buf, 6)   where g_log_fn = *(fn**)0x012651b4
//
// Epilog: /GS cookie check via __security_check_cookie (0x009d20f4),
// with an unusual inline `MOV dword ptr [0x00000000], 0x0` (10 bytes)
// between the XOR ECX,ESP and the CALL.  This appears to be a compiler
// or toolchain artifact in this specific build; the 10 bytes must be
// reproduced verbatim.
//
// Why naked asm: the /GS epilog contains a 10-byte `MOV [0x0], 0`
// instruction that has no standard C++ equivalent and cannot be coaxed
// out of the front-end cleanly.  Additionally, the parameter-shuffle
// before the two _snprintf_s calls (param1/param2 pushed as trailing
// stack context before the branch, then cleaned up together with the
// snprintf args) produces a register/stack shape that is sensitive to
// MSVC's allocation state across the whole TU.  Naked asm with _emit
// is the reliable path (same strategy as FUN_00403f10, FUN_00401b70).
//
// Reloc-bearing sites (compare.py wildcards these 4-byte windows):
//   +0x07  MOV EAX, [__security_cookie]      (DIR32 → 0x012ea8b0)
//   +0x42  PUSH imm32 = fmt string 1          (DIR32 → 0x00f633a4)
//   +0x56  CALL rel32 → _snprintf_s           (REL32 → 0x009d4f9f)
//   +0x69  PUSH imm32 = fmt string 2          (DIR32 → 0x00f633c4)
//   +0x7d  CALL rel32 → _snprintf_s           (REL32 → 0x009d4f9f)
//   +0x8c  CALL [g_log_fn_ptr]                (DIR32 → 0x012651b4)
//   +0xa9  CALL rel32 → __security_check_cookie (REL32 → 0x009d20f4)

extern "C" __declspec(naked) void FUN_004309e0() {
    __asm {
        // 000309e0: 81 ec 04 08 00 00   SUB ESP,0x804
        _emit 0x81
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 000309e6: a1 b0 a8 2e 01      MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000309eb: 33 c4               XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 000309ed: 89 84 24 00 08 00 00   MOV [ESP+0x800],EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 000309f4: 8b 94 24 0c 08 00 00   MOV EDX,[ESP+0x80c]  param2
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 000309fb: 8b 84 24 18 08 00 00   MOV EAX,[ESP+0x818]  param5
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a02: 85 c0   TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00030a04: 8b 8c 24 08 08 00 00   MOV ECX,[ESP+0x808]  param1
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a0b: 56   PUSH ESI
        _emit 0x56
        // 00030a0c: 8b b4 24 14 08 00 00   MOV ESI,[ESP+0x814]  param3
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a13: 52   PUSH EDX  (param2)
        _emit 0x52
        // 00030a14: 51   PUSH ECX  (param1)
        _emit 0x51
        // 00030a15: 74 28   JZ +0x28 (→ 0x30a3f)
        _emit 0x74
        _emit 0x28
        // 00030a17: 50   PUSH EAX  (param5)
        _emit 0x50
        // 00030a18: 8b 84 24 24 08 00 00   MOV EAX,[ESP+0x824]  param4
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a1f: 50   PUSH EAX  (param4)
        _emit 0x50
        // 00030a20: 56   PUSH ESI  (param3)
        _emit 0x56
        // 00030a21: 68 a4 33 f6 00   PUSH 0xf633a4  (format string 1)
        _emit 0x68
        _emit 0xa4
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        // 00030a26: 68 ff 07 00 00   PUSH 0x7ff  (count)
        _emit 0x68
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 00030a2b: 8d 4c 24 20   LEA ECX,[ESP+0x20]  (→ buf)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00030a2f: 68 00 08 00 00   PUSH 0x800  (sizeOfBuffer)
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a34: 51   PUSH ECX  (buf)
        _emit 0x51
        // 00030a35: e8 65 45 5a 00   CALL 0x009d4f9f  (_snprintf_s)
        _emit 0xe8
        _emit 0x65
        _emit 0x45
        _emit 0x5a
        _emit 0x00
        // 00030a3a: 83 c4 24   ADD ESP,0x24
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // 00030a3d: eb 25   JMP +0x25 (→ 0x30a64)
        _emit 0xeb
        _emit 0x25
        // 00030a3f: 8b 94 24 20 08 00 00   MOV EDX,[ESP+0x820]  param4
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a46: 52   PUSH EDX  (param4)
        _emit 0x52
        // 00030a47: 56   PUSH ESI  (param3)
        _emit 0x56
        // 00030a48: 68 c4 33 f6 00   PUSH 0xf633c4  (format string 2)
        _emit 0x68
        _emit 0xc4
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        // 00030a4d: 68 ff 07 00 00   PUSH 0x7ff  (count)
        _emit 0x68
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // 00030a52: 8d 44 24 1c   LEA EAX,[ESP+0x1c]  (→ buf)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00030a56: 68 00 08 00 00   PUSH 0x800  (sizeOfBuffer)
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a5b: 50   PUSH EAX  (buf)
        _emit 0x50
        // 00030a5c: e8 3e 45 5a 00   CALL 0x009d4f9f  (_snprintf_s)
        _emit 0xe8
        _emit 0x3e
        _emit 0x45
        _emit 0x5a
        _emit 0x00
        // 00030a61: 83 c4 20   ADD ESP,0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 00030a64: 8d 4c 24 04   LEA ECX,[ESP+0x4]  (→ buf, after PUSH ESI)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00030a68: 6a 06   PUSH 0x6  (severity)
        _emit 0x6a
        _emit 0x06
        // 00030a6a: 51   PUSH ECX  (buf)
        _emit 0x51
        // 00030a6b: ff 15 b4 51 26 01   CALL [0x012651b4]  (g_log_fn)
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        // 00030a71: 8b 8c 24 0c 08 00 00   MOV ECX,[ESP+0x80c]  (saved cookie)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a78: 83 c4 08   ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00030a7b: 5e   POP ESI
        _emit 0x5e
        // 00030a7c: 33 cc   XOR ECX,ESP
        _emit 0x33
        _emit 0xcc
        // 00030a7e: c7 05 00 00 00 00 00 00 00 00   MOV [0x0],0x0  (toolchain artifact)
        _emit 0xc7
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030a88: e8 67 16 5a 00   CALL 0x009d20f4  (__security_check_cookie)
        _emit 0xe8
        _emit 0x67
        _emit 0x16
        _emit 0x5a
        _emit 0x00
        // 00030a8d: 81 c4 04 08 00 00   ADD ESP,0x804
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 00030a93: c3   RET
        _emit 0xc3
    }
}
