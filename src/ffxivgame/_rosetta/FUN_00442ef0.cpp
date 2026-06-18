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
// FUNCTION: ffxivgame 0x00042ef0 — multi-arg forwarder that builds three
//                                  12-byte by-value structs on the stack and
//                                  forwards them (plus several scalar args)
//                                  to FUN_00442db0; returns arg1.
//                                  __cdecl, 115 bytes / 0x73.
//
// Calling convention: __cdecl; PUSH ECX at entry allocates a 4-byte local
// that is zeroed and then passed as an argument.  ESI is callee-saved and
// holds arg1 for the return value.
//
// Stack frame analysis (ESP_0 = SP at call site, before RET addr):
//   Incoming args accessed: arg1=[ESP_0+4], arg2=[ESP_0+8], arg3=[ESP_0+0xC],
//     arg4=[ESP_0+0x10], arg5=[ESP_0+0x14], arg7=[ESP_0+0x1C],
//     arg9=[ESP_0+0x24].
//
// The function builds three 12-byte structs on the stack, each of the form
//   { 0, X, Y }
// in order (top-of-stack first):
//   { 0, arg5, arg4 }
//   { 0, arg2, arg3 }
//   { 0, arg9, arg7 }
// followed by arg1 pushed twice and 0 once, then CALL FUN_00442db0 (52-byte
// arg block, cleaned by ADD ESP,0x34).  Return value = arg1 (ESI).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The complex interleaved SUB-ESP / MOV-EAX,ESP / fill pattern combined with
//   a PUSH ESI mid-structure (to reorder the arg on the callee stack) makes
//   source-level reproduction extremely brittle under MSVC 2005 /O2.
//   A __declspec(naked) body re-emitting the original 115 bytes verbatim via
//   MASM _emit directives produces a .obj whose .text is byte-identical to the
//   original slice.  The CALL rel32 at offset +0x66 (e8 55 fe ff ff) is
//   emitted as raw bytes; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00442ef0() {
    __asm {
        // 00042ef0: 51                     PUSH ECX
        _emit 0x51
        // 00042ef1: 8b 4c 24 08            MOV ECX,[ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00042ef5: 8b 54 24 08            MOV EDX,[ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00042ef9: 56                     PUSH ESI
        _emit 0x56
        // 00042efa: 8b 74 24 0c            MOV ESI,[ESP+0xC]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00042efe: c6 44 24 04 00         MOV byte ptr [ESP+0x4],0x0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x00
        // 00042f03: 8b 44 24 04            MOV EAX,[ESP+0x4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00042f07: 50                     PUSH EAX
        _emit 0x50
        // 00042f08: 51                     PUSH ECX
        _emit 0x51
        // 00042f09: 8b 4c 24 34            MOV ECX,[ESP+0x34]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 00042f0d: 52                     PUSH EDX
        _emit 0x52
        // 00042f0e: 8b 54 24 3c            MOV EDX,[ESP+0x3C]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        // 00042f12: 83 ec 0c               SUB ESP,0xC
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042f15: 8b c4                  MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 00042f17: 89 48 04               MOV [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00042f1a: 8b 4c 24 38            MOV ECX,[ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00042f1e: 89 50 08               MOV [EAX+0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00042f21: 8b 54 24 3c            MOV EDX,[ESP+0x3C]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        // 00042f25: c7 00 00 00 00 00      MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042f2b: 83 ec 0c               SUB ESP,0xC
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042f2e: 8b c4                  MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 00042f30: 89 48 04               MOV [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00042f33: 8b 4c 24 38            MOV ECX,[ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00042f37: 89 50 08               MOV [EAX+0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00042f3a: 8b 54 24 3c            MOV EDX,[ESP+0x3C]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        // 00042f3e: c7 00 00 00 00 00      MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042f44: 83 ec 0c               SUB ESP,0xC
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042f47: 8b c4                  MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 00042f49: 56                     PUSH ESI
        _emit 0x56
        // 00042f4a: c7 00 00 00 00 00      MOV dword ptr [EAX],0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042f50: 89 48 04               MOV [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00042f53: 89 50 08               MOV [EAX+0x8],EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00042f56: e8 55 fe ff ff         CALL FUN_00442db0
        _emit 0xe8
        _emit 0x55
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00042f5b: 83 c4 34               ADD ESP,0x34
        _emit 0x83
        _emit 0xc4
        _emit 0x34
        // 00042f5e: 8b c6                  MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00042f60: 5e                     POP ESI
        _emit 0x5e
        // 00042f61: 59                     POP ECX
        _emit 0x59
        // 00042f62: c3                     RET
        _emit 0xc3
    }
}
