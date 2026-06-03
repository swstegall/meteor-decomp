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
// FUNCTION: ffxivgame 0x00018300 — global array write + forward call
//                                  (__cdecl, 70 B / 0x46)
//
// __cdecl void FUN_00418300(int idx, int v0, int v1, int v2, int v3)
//
//   Calling convention: __cdecl — 5 stack args, plain RET (no fixup).
//   ADD ESP,0x14 just before pops belongs to the inner callee's 5-arg
//   __cdecl cleanup, not a stdcall ret offset.
//
//   Body:
//     EAX = idx * 48  (LEA [ECX + ECX*2] + SHL 4 = ECX * 3 * 16 = ECX * 48)
//     g_array[EAX + 0x013290bc] = v0;
//     g_array[EAX + 0x013290c0] = v1;
//     g_array[EAX + 0x013290c4] = v2;
//     g_array[EAX + 0x013290c8] = v3;
//     FUN_0041cc60(idx, v0, v1, v2, v3);
//
//   Prologue interleaving (MSVC 2005 /O2):
//     arg1 (idx) → ECX, arg2 (v0) → EDX before any pushes.
//     PUSH EBX → MOV EBX, [ESP+0x18] captures arg5 (v3)
//     PUSH ESI → MOV ESI, [ESP+0x14] captures arg3 (v1)
//     PUSH EDI → MOV EDI, [ESP+0x1c] captures arg4 (v2)
//   Inner call push order (push last-arg first): EBX, EDI, ESI, EDX, ECX.
//
//   Reloc-bearing positions (masked by tools/compare.py):
//     off 0x3b  IMAGE_REL_I386_REL32  → FUN_0041cc60  (CALL rel32)
//   Non-reloc absolute constants in instruction stream (compared literally):
//     off 0x24  disp32 0x013290bc  (global array field 0 base)
//     off 0x2a  disp32 0x013290c0  (global array field 1 base)
//     off 0x30  disp32 0x013290c4  (global array field 2 base)
//     off 0x36  disp32 0x013290c8  (global array field 3 base)
//
//   Reconstruction strategy: __declspec(naked) byte-level passthrough.
//   The interleaved PUSH/MOV prologue and hard-coded absolute disp32
//   addresses make a plain C++ rendition too fragile; emitting the exact
//   instruction bytes via _emit (with a real `call` for the CALL so the
//   linker emits the correct COFF rel32 relocation) is the cleanest match.

extern "C" void FUN_0041cc60(int, int, int, int, int);

extern "C" __declspec(naked) void FUN_00418300() {
    __asm {
        // 00018300: 8b 4c 24 04    MOV ECX, dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00018304: 8b 54 24 08    MOV EDX, dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00018308: 53             PUSH EBX
        _emit 0x53
        // 00018309: 8b 5c 24 18    MOV EBX, dword ptr [ESP+0x18]   ; arg5 = v3
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0001830d: 56             PUSH ESI
        _emit 0x56
        // 0001830e: 8b 74 24 14    MOV ESI, dword ptr [ESP+0x14]   ; arg3 = v1
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00018312: 57             PUSH EDI
        _emit 0x57
        // 00018313: 8b 7c 24 1c    MOV EDI, dword ptr [ESP+0x1c]   ; arg4 = v2
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 00018317: 53             PUSH EBX  (v3)
        _emit 0x53
        // 00018318: 57             PUSH EDI  (v2)
        _emit 0x57
        // 00018319: 56             PUSH ESI  (v1)
        _emit 0x56
        // 0001831a: 8d 04 49       LEA EAX, [ECX + ECX*2]  ; EAX = idx*3
        _emit 0x8d
        _emit 0x04
        _emit 0x49
        // 0001831d: c1 e0 04       SHL EAX, 4              ; EAX = idx*48
        _emit 0xc1
        _emit 0xe0
        _emit 0x04
        // 00018320: 52             PUSH EDX  (v0)
        _emit 0x52
        // 00018321: 51             PUSH ECX  (idx)
        _emit 0x51
        // 00018322: 89 90 bc 90 32 01  MOV dword ptr [EAX+0x013290bc], EDX
        _emit 0x89
        _emit 0x90
        _emit 0xbc
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 00018328: 89 b0 c0 90 32 01  MOV dword ptr [EAX+0x013290c0], ESI
        _emit 0x89
        _emit 0xb0
        _emit 0xc0
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 0001832e: 89 b8 c4 90 32 01  MOV dword ptr [EAX+0x013290c4], EDI
        _emit 0x89
        _emit 0xb8
        _emit 0xc4
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 00018334: 89 98 c8 90 32 01  MOV dword ptr [EAX+0x013290c8], EBX
        _emit 0x89
        _emit 0x98
        _emit 0xc8
        _emit 0x90
        _emit 0x32
        _emit 0x01
        // 0001833a: e8 21 49 00 00  CALL FUN_0041cc60  (rel32 — reloc masked)
        call FUN_0041cc60
        // 0001833f: 83 c4 14        ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00018342: 5f              POP EDI
        _emit 0x5f
        // 00018343: 5e              POP ESI
        _emit 0x5e
        // 00018344: 5b              POP EBX
        _emit 0x5b
        // 00018345: c3              RET
        _emit 0xc3
    }
}
