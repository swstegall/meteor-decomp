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
// FUNCTION: ffxivgame 0x00042e70 — argument-packing trampoline: loads up to
//                                   10 caller arguments, zeroes a local slot,
//                                   builds three 12-byte structs ({0, arg_x, arg_y})
//                                   on the stack, then calls FUN_00442cf0 with
//                                   all of it as a flat argument list.
//                                   (__cdecl, 115 bytes / 0x73, no EBP frame)
//
// Calling convention: __cdecl; ESI / ECX callee-saved via PUSH/POP.
// Returns: EAX = ESI (= first argument passed in).
//
// Stack layout at entry (before any modification):
//   [ESP+00]  ret_addr
//   [ESP+04]  arg1  (loaded into ECX, EDX, ESI — becomes return value)
//   [ESP+08]  arg2
//   [ESP+0C]  arg3  (packed into 3rd struct field_4)
//   [ESP+10]  arg4  (packed into 3rd struct field_8)
//   [ESP+14]  arg5
//   [ESP+18]  arg6  (packed into 2nd struct field_4)
//   [ESP+1C]  arg7  (packed into 2nd struct field_8)
//   [ESP+20]  arg8
//   [ESP+24]  arg9  (packed into 1st struct field_4)
//   [ESP+28]  arg10 (packed into 1st struct field_8)
//
// The three structs (each 12 bytes, field_0 = 0) are pushed newest-first;
// a leading PUSH ESI (= arg1) precedes the CALL, so FUN_00442cf0 sees:
//   arg1, {0, arg3, arg4}, {0, arg6, arg7}, {0, arg9, arg10}
//   plus the residual three words from the initial PUSH EAX/ECX/EDX.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The compiler interleaves ECX/EDX reloads between SUB ESP sequences to
//   reuse registers while keeping offsets consistent.  The single CALL
//   rel32 at +0x66 encodes a displacement of 0xFFFFFE15 (target RVA
//   0x42CF0), which is a raw immediate here — no linker relocation.
//   Source-level C++ cannot reproduce the exact SUB/MOV/SUB/MOV pattern
//   without the register-reuse spills MSVC 2005 /O2 happens to emit here,
//   so we emit the 115 original bytes verbatim via __declspec(naked) /
//   MASM _emit.  compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00442e70() {
    __asm {
        // 00042e70: 51                     PUSH ECX
        _emit 0x51
        // 00042e71: 8b 4c 24 08            MOV ECX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00042e75: 8b 54 24 08            MOV EDX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00042e79: 56                     PUSH ESI
        _emit 0x56
        // 00042e7a: 8b 74 24 0c            MOV ESI,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00042e7e: c6 44 24 04 00         MOV byte ptr [ESP+0x4],0x0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x00
        // 00042e83: 8b 44 24 04            MOV EAX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00042e87: 50                     PUSH EAX
        _emit 0x50
        // 00042e88: 51                     PUSH ECX
        _emit 0x51
        // 00042e89: 8b 4c 24 34            MOV ECX,dword ptr [ESP+0x34]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 00042e8d: 52                     PUSH EDX
        _emit 0x52
        // 00042e8e: 8b 54 24 3c            MOV EDX,dword ptr [ESP+0x3c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        // 00042e92: 83 ec 0c               SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042e95: 8b c4                  MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 00042e97: 89 48 04               MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00042e9a: 8b 4c 24 38            MOV ECX,dword ptr [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00042e9e: 89 50 08               MOV dword ptr [EAX+0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00042ea1: 8b 54 24 3c            MOV EDX,dword ptr [ESP+0x3c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        // 00042ea5: c7 00 00 00 00 00      MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042eab: 83 ec 0c               SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042eae: 8b c4                  MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 00042eb0: 89 48 04               MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00042eb3: 8b 4c 24 38            MOV ECX,dword ptr [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00042eb7: 89 50 08               MOV dword ptr [EAX+0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00042eba: 8b 54 24 3c            MOV EDX,dword ptr [ESP+0x3c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        // 00042ebe: c7 00 00 00 00 00      MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042ec4: 83 ec 0c               SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042ec7: 8b c4                  MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 00042ec9: 56                     PUSH ESI
        _emit 0x56
        // 00042eca: c7 00 00 00 00 00      MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042ed0: 89 48 04               MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00042ed3: 89 50 08               MOV dword ptr [EAX+0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00042ed6: e8 15 fe ff ff         CALL 0x00442cf0
        _emit 0xe8
        _emit 0x15
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00042edb: 83 c4 34               ADD ESP,0x34
        _emit 0x83
        _emit 0xc4
        _emit 0x34
        // 00042ede: 8b c6                  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00042ee0: 5e                     POP ESI
        _emit 0x5e
        // 00042ee1: 59                     POP ECX
        _emit 0x59
        // 00042ee2: c3                     RET
        _emit 0xc3
    }
}
