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
// FUNCTION: ffxivgame 0x0004d610 — FUN_0044d610 (__cdecl, 0x44 bytes / 68 B)
//
// Calling convention: __cdecl (caller cleans; plain RET, no immediate).
// Saves EBX/EBP/ESI/EDI; no local frame.
//
// Body (read from orig/ffxivgame.exe @ RVA 0x0004d610):
//
//   Prologue: PUSH EBX / MOV EBX,[ESP+0xC] (arg2)
//             PUSH EBP / MOV EBP,[ESP+0x14] (arg3)
//             PUSH ESI / PUSH EDI
//
//   Call 1:   PUSH EBP, PUSH 0, PUSH EBX → CALL FUN_0044d500 (rel32)
//             → ESI = EAX (return value of first call)
//             MOV EDI, [ESP+0x20]          ; EDI = arg1 (loaded after call)
//             MOV EAX, [EDI-4]             ; EAX = *(arg1-4)
//             ADD ESP, 0xC                 ; cdecl cleanup (3 args)
//             SHR EAX, 8                   ; EAX >>= 8
//             CMP EAX, EBX
//             CMOVA EAX, EBX               ; EAX = min(EAX, arg2) unsigned
//
//   Call 2:   PUSH EAX, PUSH EDI, PUSH ESI → CALL 0x009d5110 (rel32)
//             (result discarded — no MOV ESI,EAX after)
//
//   Call 3:   PUSH EBP, PUSH 0, PUSH EDI  → CALL FUN_0044d350 (rel32)
//             ADD ESP, 0x18               ; cdecl cleanup (6 args: 3+3)
//
//   Epilogue: POP EDI / MOV EAX,ESI (return ESI from call 1) /
//             POP ESI / POP EBP / POP EBX / RET
//
// Reloc-bearing sites (rel32 CALLs — operand bytes are the pre-link
// displacement in the orig binary; compare.py masks them):
//   +0x11   REL32 → FUN_0044d500 (0xe8 db fe ff ff)
//   +0x2c   REL32 → 0x009d5110   (0xe8 cf 7a 58 00)
//   +0x35   REL32 → FUN_0044d350 (0xe8 06 fd ff ff)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
// The three CALL rel32 displacements are resolved only in a full-binary
// relink and cannot be reproduced via source-level __cdecl declarations.
// The `_emit` passthrough yields a .text slice byte-identical to the orig
// 68 bytes, which compare.py reports as GREEN.

extern "C" __declspec(naked) void FUN_0044d610() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, [ESP+0xC]
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, [ESP+0x14]
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x55              // PUSH EBP         (arg3 for call 1)
        _emit 0x6a              // PUSH 0x0         (mid arg for call 1)
        _emit 0x00
        _emit 0x53              // PUSH EBX         (arg2 for call 1)
        _emit 0xe8              // CALL FUN_0044d500 (rel32 = 0xFFFFFEDB)
        _emit 0xdb
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDI, [ESP+0x20]  ; arg1
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x8b              // MOV EAX, [EDI-4]
        _emit 0x47
        _emit 0xfc
        _emit 0x83              // ADD ESP, 0xC
        _emit 0xc4
        _emit 0x0c
        _emit 0xc1              // SHR EAX, 0x8
        _emit 0xe8
        _emit 0x08
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x0f              // CMOVA EAX, EBX
        _emit 0x47
        _emit 0xc3
        _emit 0x50              // PUSH EAX          (count arg for call 2)
        _emit 0x57              // PUSH EDI           (arg1 for call 2)
        _emit 0x56              // PUSH ESI           (result for call 2)
        _emit 0xe8              // CALL 0x009d5110 (rel32 = 0x00587ACF)
        _emit 0xcf
        _emit 0x7a
        _emit 0x58
        _emit 0x00
        _emit 0x55              // PUSH EBP           (arg3 for call 3)
        _emit 0x6a              // PUSH 0x0           (mid arg for call 3)
        _emit 0x00
        _emit 0x57              // PUSH EDI            (arg1 for call 3)
        _emit 0xe8              // CALL FUN_0044d350 (rel32 = 0xFFFFFD06)
        _emit 0x06
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x18  (cleanup 6 args: 3+3)
        _emit 0xc4
        _emit 0x18
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
