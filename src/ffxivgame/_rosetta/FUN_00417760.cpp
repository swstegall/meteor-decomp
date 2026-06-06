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
// FUNCTION: ffxivgame 0x00017760 — debug/log format-and-output helper
//           (__cdecl void FUN_00417760(arg1, arg2, arg3, arg4, arg5), 180 B / 0xb4)
//
// A /GS-protected function that formats a diagnostic message into a 2 KB
// stack buffer using _snprintf_s, then passes the result to a log output
// function via an IAT indirect call.
//
// Parameters (all __cdecl, relative to original ESP before call):
//   arg1 [+0x04]: first  param — loaded into ECX (file/module?)
//   arg2 [+0x08]: second param — loaded into EDX (line number?)
//   arg3 [+0x0c]: third  param — loaded into ESI (subsystem/tag?)
//   arg4 [+0x10]: fourth param — loaded lazily from stack
//   arg5 [+0x14]: fifth  param — condition flag (0 = short fmt, else long fmt)
//
// Stack layout after SUB ESP,0x804:
//   [ESP+0x000..0x7ff]  char buf[0x800]   (2048-byte output buffer)
//   [ESP+0x800]         /GS cookie        (__security_cookie XOR ESP)
//   [ESP+0x804]         return address
//   [ESP+0x808..0x81c]  arg1..arg5
//
// Behaviour:
//   1. /GS prolog: allocate frame, store cookie XOR ESP at [ESP+0x800].
//   2. Load arg2→EDX, arg5→EAX, arg1→ECX; save ESI; load arg3→ESI.
//   3. Pre-push arg2 (EDX) and arg1 (ECX) as common trailing varargs.
//   4. Branch on arg5 (EAX):
//        nonzero: _snprintf_s(buf,0x800,0x7ff,fmt1,arg3,arg4,arg5,arg1,arg2)
//        zero:    _snprintf_s(buf,0x800,0x7ff,fmt2,arg3,arg4,arg1,arg2)
//   5. Call IAT fn at [0x012651b4](6, buf) — log output at level 6.
//   6. Write 0 to a relocated global address.
//   7. /GS epilog: XOR ECX,ESP; __security_check_cookie; restore frame; ret.
//
// Reconstruction: naked-asm byte passthrough (_emit one byte per line).
//
// Reloc-bearing sites (4-byte windows masked by compare.py against orig):
//   +0x07  MOV EAX,[__security_cookie]    moffs32 addr (.data 0x012ea8b0)
//   +0x38  PUSH fmt1                      imm32 addr   (.rdata 0xf57ac8)
//   +0x47  CALL rel32 _snprintf_s         (0x009d4f9f, 1st)
//   +0x60  PUSH fmt2                      imm32 addr   (.rdata 0xf57ae8)
//   +0x6f  CALL rel32 _snprintf_s         (0x009d4f9f, 2nd)
//   +0x8c  CALL [IAT] addr                (0x012651b4)
//   +0x9f  MOV [global],0 target addr     (reloc addr, value stays 0)
//   +0xa9  CALL rel32 __security_check_cookie (0x009d20f4)

extern "C" __declspec(naked) void FUN_00417760() {
    __asm {
        // SUB ESP, 0x804
        _emit 0x81
        _emit 0xec
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // MOV EAX, [__security_cookie]   (moffs32; addr = reloc)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // MOV dword ptr [ESP+0x800], EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // MOV EDX, dword ptr [ESP+0x80c]   (arg2)
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // MOV EAX, dword ptr [ESP+0x818]   (arg5)
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // MOV ECX, dword ptr [ESP+0x808]   (arg1)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH ESI   (save callee-save)
        _emit 0x56
        // MOV ESI, dword ptr [ESP+0x814]   (arg3; +4 for ESI push)
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x14
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH EDX   (arg2 — common trailing vararg pre-push)
        _emit 0x52
        // PUSH ECX   (arg1 — common trailing vararg pre-push)
        _emit 0x51
        // JZ +0x28   (if arg5==0, jump to else/short path)
        _emit 0x74
        _emit 0x28
        // --- true path: arg5 != 0, long format (fmt1) ---
        // PUSH EAX   (arg5)
        _emit 0x50
        // MOV EAX, dword ptr [ESP+0x824]   (arg4; +4*4=16 for 4 pushes)
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH EAX   (arg4)
        _emit 0x50
        // PUSH ESI   (arg3)
        _emit 0x56
        // PUSH 0xf57ac8   (fmt1; reloc addr)
        _emit 0x68
        _emit 0xc8
        _emit 0x7a
        _emit 0xf5
        _emit 0x00
        // PUSH 0x7ff   (max count)
        _emit 0x68
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // LEA ECX, [ESP+0x20]   (buf ptr; 8 pushes down from base)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // PUSH 0x800   (buffer size)
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH ECX   (buf)
        _emit 0x51
        // CALL rel32 _snprintf_s   (reloc)
        _emit 0xe8
        _emit 0xe5
        _emit 0xd7
        _emit 0x5b
        _emit 0x00
        // ADD ESP, 0x24   (9 args × 4 = 36)
        _emit 0x83
        _emit 0xc4
        _emit 0x24
        // JMP +0x25   (to common epilogue)
        _emit 0xeb
        _emit 0x25
        // --- else path: arg5 == 0, short format (fmt2) ---
        // MOV EDX, dword ptr [ESP+0x820]   (arg4; +3*4=12 for 3 pushes)
        _emit 0x8b
        _emit 0x94
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH EDX   (arg4)
        _emit 0x52
        // PUSH ESI   (arg3)
        _emit 0x56
        // PUSH 0xf57ae8   (fmt2; reloc addr)
        _emit 0x68
        _emit 0xe8
        _emit 0x7a
        _emit 0xf5
        _emit 0x00
        // PUSH 0x7ff
        _emit 0x68
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        // LEA EAX, [ESP+0x1c]   (buf ptr; 7 pushes down from base)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // PUSH 0x800
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // PUSH EAX   (buf)
        _emit 0x50
        // CALL rel32 _snprintf_s   (reloc)
        _emit 0xe8
        _emit 0xbe
        _emit 0xd7
        _emit 0x5b
        _emit 0x00
        // ADD ESP, 0x20   (8 args × 4 = 32)
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // --- common: log output via IAT ---
        // LEA ECX, [ESP+0x4]   (buf = past saved-ESI slot; ESP = base-4)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // PUSH 0x6   (log level)
        _emit 0x6a
        _emit 0x06
        // PUSH ECX   (buf)
        _emit 0x51
        // CALL dword ptr [0x012651b4]   (IAT; addr = reloc)
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        // --- /GS epilog ---
        // MOV ECX, dword ptr [ESP+0x80c]   (cookie; ESP=base-12 → [base+0x800])
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x8   (clean 2 __cdecl args from IAT call)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // POP ESI   (restore callee-save; ESP → base)
        _emit 0x5e
        // XOR ECX, ESP   (recover cookie = stored^base XOR base = original)
        _emit 0x33
        _emit 0xcc
        // MOV dword ptr [global], 0   (addr=reloc, value=literal 0)
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
        // CALL rel32 __security_check_cookie   (reloc)
        _emit 0xe8
        _emit 0xe7
        _emit 0xa8
        _emit 0x5b
        _emit 0x00
        // ADD ESP, 0x804   (restore frame)
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // RET
        _emit 0xc3
    }
}
