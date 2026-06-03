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
// FUNCTION: ffxivgame 0x00018740 — log-format dispatcher (__cdecl, 180 B / 0xb4)
//
// Formats a message into a 0x800-byte (2048-byte) stack buffer and then
// passes the result to a global log-writer function pointer (at .data
// 0x012651b4) with severity level 6.  Two format strings select the
// message shape:
//   0x00f57dd8 — "with-ID" variant, three format args (p3, p4, p5)
//   0x00f57df8 — "without-ID" variant, two format args (p3, p7[≡p4_of_false])
// Selection is driven by whether the fifth parameter (p5) is non-zero.
//
// After the write, a global variable (reloc placeholder 0x00000000 in the
// orig binary) is zeroed, then the /GS security cookie is validated via
// __security_check_cookie (0x009d20f4) before the frame is torn down.
//
// Calling convention: __cdecl (RET, no stack cleanup in callee).
// Frame: SUB ESP, 0x804  →  char buf[0x800] at [ESP+0], cookie at [ESP+0x800].
//
// Reloc-bearing sites (function-relative offsets, wildcarded by compare.py):
//   +0x07   MOV EAX, [__security_cookie]          (.data 0x012ea8b0)
//   +0x42   PUSH fmt_with_id                      (.rdata 0x00f57dd8)
//   +0x56   CALL _snprintf_s-like                 (rel32 → 0x009d4f9f)
//   +0x69   PUSH fmt_no_id                        (.rdata 0x00f57df8)
//   +0x7d   CALL _snprintf_s-like                 (rel32 → 0x009d4f9f)
//   +0x8d   CALL [log_fn_ptr]                     (.data 0x012651b4)
//   +0xa0   MOV [g_zero_target], 0                (.data reloc placeholder 0x00000000)
//   +0xa9   CALL __security_check_cookie          (rel32 → 0x009d20f4)

extern "C" __declspec(naked) void FUN_00418740() {
    __asm {
        // --- frame allocation + /GS cookie setup --------------------------
        _emit 0x81  // SUB ESP, 0x804
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xa1  // MOV EAX, [0x012ea8b0]  (reloc: __security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89  // MOV dword ptr [ESP+0x800], EAX  (save XOR'd cookie)
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00

        // --- load / test parameters ----------------------------------------
        _emit 0x8b  // MOV EDX, dword ptr [ESP+0x80c]  (param2)
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x818]  (param5)
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x8b  // MOV ECX, dword ptr [ESP+0x808]  (param1)
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x56  // PUSH ESI  (save ESI)
        _emit 0x8b  // MOV ESI, dword ptr [ESP+0x814]  (param3)
        _emit 0xb4
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52  // PUSH EDX  (param2 onto stack for snprintf)
        _emit 0x51  // PUSH ECX  (param1 onto stack for snprintf)
        _emit 0x74  // JZ  +0x28  (param5 == 0 → false branch)
        _emit 0x28

        // --- true branch (param5 != 0): snprintf(buf, 0x800, 0x7ff, fmt1, p3, p4, p5) ---
        _emit 0x50  // PUSH EAX  (param5 — rightmost vararg)
        _emit 0x8b  // MOV EAX, dword ptr [ESP+0x824]  (param4)
        _emit 0x84
        _emit 0x24
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX  (param4)
        _emit 0x56  // PUSH ESI  (param3 = ESI)
        _emit 0x68  // PUSH 0xf57dd8  (reloc: fmt_with_id)
        _emit 0xd8
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        _emit 0x68  // PUSH 0x7ff  (maxCount)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA ECX, [ESP+0x20]  (buf = bottom of frame)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x68  // PUSH 0x800  (bufSize)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51  // PUSH ECX  (buf ptr)
        _emit 0xe8  // CALL 0x009d4f9f  (reloc: _snprintf_s-like)
        _emit 0x05
        _emit 0xc8
        _emit 0x5b
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x24  (pop 9 args)
        _emit 0xc4
        _emit 0x24
        _emit 0xeb  // JMP +0x25  (→ common epilogue)
        _emit 0x25

        // --- false branch (param5 == 0): snprintf(buf, 0x800, 0x7ff, fmt2, p3, p7) ---
        _emit 0x8b  // MOV EDX, dword ptr [ESP+0x820]  (param4-of-false = orig param7)
        _emit 0x94
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52  // PUSH EDX
        _emit 0x56  // PUSH ESI  (param3 = ESI)
        _emit 0x68  // PUSH 0xf57df8  (reloc: fmt_no_id)
        _emit 0xf8
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        _emit 0x68  // PUSH 0x7ff  (maxCount)
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA EAX, [ESP+0x1c]  (buf = bottom of frame)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x68  // PUSH 0x800  (bufSize)
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX  (buf ptr)
        _emit 0xe8  // CALL 0x009d4f9f  (reloc: _snprintf_s-like)
        _emit 0xde
        _emit 0xc7
        _emit 0x5b
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x20  (pop 8 args)
        _emit 0xc4
        _emit 0x20

        // --- common: call log writer with (buf, 6) -------------------------
        _emit 0x8d  // LEA ECX, [ESP+0x4]  (buf ptr; ESP = frame_base - 4 here)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x6a  // PUSH 0x6  (log level)
        _emit 0x06
        _emit 0x51  // PUSH ECX  (buf ptr)
        _emit 0xff  // CALL dword ptr [0x012651b4]  (reloc: log_fn_ptr)
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01

        // --- /GS epilogue --------------------------------------------------
        _emit 0x8b  // MOV ECX, dword ptr [ESP+0x80c]  (load saved XOR'd cookie)
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8  (pop 2 args from log call)
        _emit 0xc4
        _emit 0x08
        _emit 0x5e  // POP ESI  (restore ESI)
        _emit 0x33  // XOR ECX, ESP  (reconstruct original cookie)
        _emit 0xcc
        _emit 0xc7  // MOV dword ptr [0x00000000], 0x0  (reloc: zero global)
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x009d20f4  (reloc: __security_check_cookie)
        _emit 0x07
        _emit 0x99
        _emit 0x5b
        _emit 0x00
        _emit 0x81  // ADD ESP, 0x804  (deallocate frame)
        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
    }
}
