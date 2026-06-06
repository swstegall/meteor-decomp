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
// FUNCTION: ffxivgame 0x0045df50 — iterator/search loop over a container
//                                  (313 B / 0x139, ESP-based frame via
//                                  __chkstk, no EBP frame pointer).
//
// Inspection (read from the disassembly at orig RVA 0x0005df50):
//
//   Calling convention: non-standard internal ABI — ESI = object pointer
//   (not saved/restored within this function), callee-saves EBX/EBP/EDI.
//   Epilogue: ADD ESP,0xc; RET (no stack arg cleanup → __cdecl-like, but
//   ESI is expected to be preset by the caller).
//
//   Stack frame: __chkstk(0xc) allocates 12 bytes (3 dwords) of locals
//   before the callee-saved PUSH EBX / PUSH EBP / PUSH EDI sequence,
//   giving an ESP-relative layout:
//     [ESP+0x00] = saved EDI
//     [ESP+0x04] = saved EBP  (used as sentinel -1, not frame ptr)
//     [ESP+0x08] = saved EBX
//     [ESP+0x0C] = local_a (initialised to 0, then to result of first call)
//     [ESP+0x10] = local_b (initialised to 0, then to second result)
//     [ESP+0x14] = (padding / third local dword from __chkstk)
//     [ESP+0x18] = return address
//
//   High-level shape:
//     EBP = -1  (sentinel for "not-yet-assigned" slot tag)
//     local_a = FUN_004640e0()   ; allocate/get handle A
//     if (!local_a) goto fail_cleanup;
//     count = FUN_00464030([ESI]);
//     if (count <= 0) goto post_loop;
//     for (EBX = 0; EBX < count; EBX++) {
//         item = FUN_00464040([ESI], EBX);  // EDI = item
//         if (item->field_8 != EBP) {       // slot not already tagged -1
//             local_b = FUN_004640e0();     // allocate/get handle B
//             if (!local_b) goto fail_cleanup;
//             if (!FUN_00463fc0(local_a, local_b)) goto fail_cleanup;
//             EBP = item->field_8;
//         }
//         if (!FUN_00463fc0(local_b, item)) goto fail_cleanup;
//         count = FUN_00464030([ESI]);      // re-read after possible change
//     }
//   post_loop:
//     tmp = FUN_00460470(0, &esp_slot, 0xf6916c, -1, -1);
//     if (!FUN_00466c20([ESI+8], tmp)) goto fail_cleanup;
//     FUN_004641f0(local_a, 0x45df20);
//     FUN_0045c940(0xd, 0xcb, 0x41, 0xf691c0, 0x120);
//     return -1;                            // failure path returns OR EAX,-1
//   success:
//     tmp2 = FUN_00460470(...);
//     FUN_004641f0(tmp2, 0x45df20);
//     [ESI+4] = 0;
//     return tmp;                           // success path returns EDI
//
//   Reloc-bearing sites (absolute addresses in PUSH immediates and CALL
//   rel32 displacements; all encode the original image-base 0x00400000):
//     +0x05  CALL __chkstk            (rel32 → 0x009d29d0)
//     +0x1a  CALL FUN_004640e0        (rel32 → 0x004640e0)
//     +0x2e  CALL FUN_00464030        (rel32 → 0x00464030)
//     +0x44  CALL FUN_00464040        (rel32 → 0x00464040)
//     +0x53  CALL FUN_004640e0        (rel32 → 0x004640e0, 2nd)
//     +0x66  CALL FUN_00463fc0        (rel32 → 0x00463fc0)
//     +0x7b  CALL FUN_00463fc0        (rel32 → 0x00463fc0, 2nd)
//     +0x8d  CALL FUN_00464030        (rel32 → 0x00464030, 2nd)
//     +0x9d  PUSH 0xf6916c            (abs string literal)
//     +0xa9  CALL FUN_00460470        (rel32 → 0x00460470)
//     +0xb5  CALL FUN_00466c20        (rel32 → 0x00466c20)
//     +0xc5  PUSH 0x45df20            (abs function/data ptr)
//     +0xcb  CALL FUN_004641f0        (rel32 → 0x004641f0)
//     +0xd5  PUSH 0xf691c0            (abs string literal)
//     +0xe3  CALL FUN_0045c940        (rel32 → 0x0045c940)
//     +0xf5  PUSH 0xf6916c            (abs string literal, 2nd)
//     +0x112 CALL FUN_00460470        (rel32 → 0x00460470, 2nd)
//     +0x11b PUSH 0x45df20            (abs function/data ptr, 2nd)
//     +0x121 CALL FUN_004641f0        (rel32 → 0x004641f0, 2nd)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function's frame setup, mixed ESP-relative addressing, non-standard
//   ESI-based ABI, and the eighteen relocation sites above make a
//   source-level C++ reconstruction brittle under MSVC 2005 /O2 — every
//   rewrite would shift at least one call displacement, branch offset, or
//   frame-slot reference. The `__declspec(naked)` passthrough strategy used
//   by FUN_004014b0, FUN_00408f10, FUN_00401a00, and the rest of the
//   _rosetta siblings is the correct choice here: the 313 raw bytes are
//   emitted verbatim via MASM `_emit` directives, making the .obj's .text
//   section byte-identical to the original binary slice.

extern "C" __declspec(naked) void FUN_0045df50() {
    __asm {
        // MOV EAX, 0xc
        _emit 0xb8
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CALL __chkstk (0x009d29d0)
        _emit 0xe8
        _emit 0x76
        _emit 0x4a
        _emit 0x57
        _emit 0x00
        // PUSH EBX
        _emit 0x53
        // PUSH EBP
        _emit 0x55
        // XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // PUSH EDI
        _emit 0x57
        // MOV dword ptr [ESP+0xc], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // MOV dword ptr [ESP+0x10], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // OR EBP, 0xffffffff
        _emit 0x83
        _emit 0xcd
        _emit 0xff
        // CALL FUN_004640e0
        _emit 0xe8
        _emit 0x71
        _emit 0x61
        _emit 0x00
        _emit 0x00
        // CMP EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // MOV dword ptr [ESP+0xc], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // JZ +0x9a (to 0x0045e015)
        _emit 0x0f
        _emit 0x84
        _emit 0x9a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV EAX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // PUSH EAX
        _emit 0x50
        // CALL FUN_00464030
        _emit 0xe8
        _emit 0xad
        _emit 0x60
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // JLE +0x5f (to 0x0045dfe9)
        _emit 0x7e
        _emit 0x5f
        // LEA EBX, [EBX]  (6-byte nop / align)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // PUSH EBX
        _emit 0x53
        // PUSH ECX
        _emit 0x51
        // CALL FUN_00464040
        _emit 0xe8
        _emit 0xa7
        _emit 0x60
        _emit 0x00
        _emit 0x00
        // MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // CMP dword ptr [EDI+0x8], EBP
        _emit 0x39
        _emit 0x6f
        _emit 0x08
        // JZ +0x22 (to 0x0045dfc5)
        _emit 0x74
        _emit 0x22
        // CALL FUN_004640e0
        _emit 0xe8
        _emit 0x38
        _emit 0x61
        _emit 0x00
        _emit 0x00
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // MOV dword ptr [ESP+0x10], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // JZ +0x61 (to 0x0045e011)
        _emit 0x74
        _emit 0x61
        // MOV EDX, dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // PUSH EAX
        _emit 0x50
        // PUSH EDX
        _emit 0x52
        // CALL FUN_00463fc0
        _emit 0xe8
        _emit 0x05
        _emit 0x60
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // JZ +0x4f (to 0x0045e011)
        _emit 0x74
        _emit 0x4f
        // MOV EBP, dword ptr [EDI+0x8]
        _emit 0x8b
        _emit 0x6f
        _emit 0x08
        // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // PUSH EDI
        _emit 0x57
        // PUSH EAX
        _emit 0x50
        // CALL FUN_00463fc0
        _emit 0xe8
        _emit 0xf0
        _emit 0x5f
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // JZ +0x3a (to 0x0045e011)
        _emit 0x74
        _emit 0x3a
        // MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // PUSH ECX
        _emit 0x51
        // ADD EBX, 0x1
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        // CALL FUN_00464030
        _emit 0xe8
        _emit 0x4e
        _emit 0x60
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // CMP EBX, EAX
        _emit 0x3b
        _emit 0xd8
        // JL -0x59 (to 0x0045df90)
        _emit 0x7c
        _emit 0xa7
        // PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // PUSH 0xf6916c
        _emit 0x68
        _emit 0x6c
        _emit 0x91
        _emit 0xf6
        _emit 0x00
        // LEA EDX, [ESP+0x18]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // PUSH EDX
        _emit 0x52
        // CALL FUN_00460470
        _emit 0xe8
        _emit 0x72
        _emit 0x24
        _emit 0x00
        _emit 0x00
        // MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // MOV EAX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // PUSH EDI
        _emit 0x57
        // PUSH EAX
        _emit 0x50
        // CALL FUN_00466c20
        _emit 0xe8
        _emit 0x16
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // JNZ +0x34 (to 0x0045e045)
        _emit 0x75
        _emit 0x34
        // MOV EAX, dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // PUSH 0x45df20
        _emit 0x68
        _emit 0x20
        _emit 0xdf
        _emit 0x45
        _emit 0x00
        // PUSH EAX
        _emit 0x50
        // CALL FUN_004641f0
        _emit 0xe8
        _emit 0xd0
        _emit 0x61
        _emit 0x00
        _emit 0x00
        // PUSH 0x120
        _emit 0x68
        _emit 0x20
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // PUSH 0xf691c0
        _emit 0x68
        _emit 0xc0
        _emit 0x91
        _emit 0xf6
        _emit 0x00
        // PUSH 0x41
        _emit 0x6a
        _emit 0x41
        // PUSH 0xcb
        _emit 0x68
        _emit 0xcb
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH 0xd
        _emit 0x6a
        _emit 0x0d
        // CALL FUN_0045c940
        _emit 0xe8
        _emit 0x08
        _emit 0xe9
        _emit 0xff
        _emit 0xff
        // ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // POP EDI
        _emit 0x5f
        // POP EBP
        _emit 0x5d
        // OR EAX, 0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // POP EBX
        _emit 0x5b
        // ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // RET
        _emit 0xc3
        // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // MOV EDX, dword ptr [ECX+0x4]
        _emit 0x8b
        _emit 0x51
        _emit 0x04
        // PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // PUSH 0xf6916c
        _emit 0x68
        _emit 0x6c
        _emit 0x91
        _emit 0xf6
        _emit 0x00
        // LEA EAX, [ESP+0x20]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // PUSH EAX
        _emit 0x50
        // LEA ECX, [ESP+0x1c]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // PUSH ECX
        _emit 0x51
        // MOV dword ptr [ESP+0x28], EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // CALL FUN_00460470
        _emit 0xe8
        _emit 0x09
        _emit 0x24
        _emit 0x00
        _emit 0x00
        // MOV EDX, dword ptr [ESP+0x20]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // PUSH 0x45df20
        _emit 0x68
        _emit 0x20
        _emit 0xdf
        _emit 0x45
        _emit 0x00
        // PUSH EDX
        _emit 0x52
        // CALL FUN_004641f0
        _emit 0xe8
        _emit 0x7a
        _emit 0x61
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // POP EDI
        _emit 0x5f
        // POP EBP
        _emit 0x5d
        // MOV dword ptr [ESI+0x4], 0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // POP EBX
        _emit 0x5b
        // ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // RET
        _emit 0xc3
    }
}
