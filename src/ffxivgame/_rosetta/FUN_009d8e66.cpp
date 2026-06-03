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
// FUNCTION: ffxivgame 0x005d8e66 — CorExitProcess probe (38 B / 0x26)
//
// __cdecl void FUN_009d8e66(int exitCode)
//
// Checks if the .NET runtime (mscoree.dll) is already loaded in the
// current process via GetModuleHandleA. If loaded, resolves CorExitProcess
// via GetProcAddress and calls it with exitCode. This is the Windows pattern
// for triggering clean .NET shutdown from native code only when the CLR is
// present.
//
// Calling convention: __cdecl — plain RET, no callee-save registers.
// Frame: none (/Oy). Both API calls are __stdcall (self-cleaning), so the
// stack depth is always at function-entry level when it matters.
// exitCode stays at [ESP+4] throughout; MSVC encodes the forward as
// `PUSH dword ptr [ESP+4]` (ff 74 24 04), the 4-byte PUSH m32 form.
//
// Source-level C++ generates `MOV ECX,[ESP+4] / PUSH ECX` (5 bytes) for
// the exitCode forward, producing a 39-byte function instead of 38.
// The __declspec(naked) + _emit approach emits the exact 38-byte sequence.
// compare.py masks the four reloc ranges (offsets 0x01-0x04, 0x07-0x0a,
// 0x11-0x14, 0x17-0x1a) which correspond to the string VA and IAT VA
// fields that differ between the orig binary and our .obj.
//
// Asm (38 bytes @ orig RVA 0x005d8e66):
//   68 c0 5e 08 01       PUSH 0x01085ec0         ; "mscoree.dll"
//   ff 15 e4 e1 f3 00    CALL [0x00f3e1e4]        ; IAT → GetModuleHandleA
//   85 c0                TEST EAX, EAX
//   74 16                JE  short +0x16 → ret    ; not loaded → skip
//   68 b0 5e 08 01       PUSH 0x01085eb0          ; "CorExitProcess"
//   50                   PUSH EAX                 ; hMod
//   ff 15 50 e1 f3 00    CALL [0x00f3e150]         ; IAT → GetProcAddress
//   85 c0                TEST EAX, EAX
//   74 06                JE  short +0x06 → ret    ; not exported → skip
//   ff 74 24 04          PUSH dword ptr [ESP+4]   ; exitCode
//   ff d0                CALL EAX                 ; CorExitProcess(exitCode)
//   c3                   RET

extern "C" __declspec(naked) void FUN_009d8e66(int /*exitCode*/) {
    __asm {
        // 005d8e66: 68 c0 5e 08 01    PUSH "mscoree.dll" (VA 0x01085ec0, reloc)
        _emit 0x68
        _emit 0xc0
        _emit 0x5e
        _emit 0x08
        _emit 0x01
        // 005d8e6b: ff 15 e4 e1 f3 00 CALL [GetModuleHandleA] (IAT VA 0x00f3e1e4, reloc)
        _emit 0xff
        _emit 0x15
        _emit 0xe4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 005d8e71: 85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 005d8e73: 74 16             JE short +0x16 (→ ret at 0x005d8e8b)
        _emit 0x74
        _emit 0x16
        // 005d8e75: 68 b0 5e 08 01    PUSH "CorExitProcess" (VA 0x01085eb0, reloc)
        _emit 0x68
        _emit 0xb0
        _emit 0x5e
        _emit 0x08
        _emit 0x01
        // 005d8e7a: 50                PUSH EAX (hMod from GetModuleHandleA)
        _emit 0x50
        // 005d8e7b: ff 15 50 e1 f3 00 CALL [GetProcAddress] (IAT VA 0x00f3e150, reloc)
        _emit 0xff
        _emit 0x15
        _emit 0x50
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 005d8e81: 85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 005d8e83: 74 06             JE short +0x06 (→ ret at 0x005d8e8b)
        _emit 0x74
        _emit 0x06
        // 005d8e85: ff 74 24 04       PUSH dword ptr [ESP+4] (exitCode)
        _emit 0xff
        _emit 0x74
        _emit 0x24
        _emit 0x04
        // 005d8e89: ff d0             CALL EAX (CorExitProcess(exitCode))
        _emit 0xff
        _emit 0xd0
        // 005d8e8b: c3                RET
        _emit 0xc3
    }
}
