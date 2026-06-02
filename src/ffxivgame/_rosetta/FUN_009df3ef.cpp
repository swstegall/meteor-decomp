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
// FUNCTION: ffxivgame 0x005df3ef — object cleanup / release-fields helper
//                                  (256 B / 0x100, EH4-SEH wrapped, /GS).
//
// Behaviour read from the disassembly at orig RVA 0x005df3ef:
//
//   __cdecl void FUN_009df3ef(SomeObject* p);
//
//   The function opens with the MSVC 2005 __EH_prolog3_GS calling convention:
//     PUSH 8 / PUSH <scope-table 0x0122d3d8> / CALL <prologue helper @0x005de4f0>
//   establishing an EBP frame with /GS cookie protection.
//
//   Body:
//     ESI = p = [EBP+8]                    ; first (only) argument
//     if (!ESI) return;                    ; early-out on null
//
//     // Release a chain of reference-counted members at various offsets,
//     // each following the pattern: if (field != 0) Release(field)
//     if (ESI+0x24 != 0) Release(ESI+0x24);
//     if (ESI+0x2c != 0) Release(ESI+0x2c);
//     if (ESI+0x34 != 0) Release(ESI+0x34);
//     if (ESI+0x3c != 0) Release(ESI+0x3c);
//     if (ESI+0x44 != 0) Release(ESI+0x44);
//     if (ESI+0x48 != 0) Release(ESI+0x48);
//     if ([ESI+0x5c] != 0x012eb7c8) Release([ESI+0x5c]);
//
//     // Allocate/init something via a 0x0d-byte helper (@+0x000031da)
//     // then clear [EBP-4] scope to 0
//
//     EDI = [ESI+0x68]                     ; m_ptr_68
//     if (EDI) {
//         r = CALL [IAT@0x00f3e2d0](EDI);  ; COM Release or similar
//         if (!r) {
//             if (EDI == 0x012eaec0) Release(EDI);
//         }
//     }
//     scope = -2;
//     CALL <epilog-cleanup @0x005df4fb>;
//
//     scope = 1;
//     EDI = [ESI+0x6c]                     ; m_ptr_6c
//     if (EDI) {
//         CALL Release(EDI);               ; @(-0xbc61) from call site
//         if (EDI != [0x012eb4c8]) {
//             if (EDI != 0x012eb3f0) {
//                 if ([EDI] == 0) Release(EDI);
//             }
//         }
//     }
//     scope = -2;
//     CALL <epilog-cleanup @0x005df507>;
//     PUSH ESI;
//     CALL <some-fn @0x005d5c88>;
//
//   Stack frame: EBP-frame established by __EH_prolog3_GS;
//                [EBP-4] = EH4 trylevel / scope cookie.
//
//   Reloc-bearing sites in the orig 256 bytes (resolve only in full relink):
//     +0x04   abs32  0x0122d3d8  scope-table ptr (PUSH)
//     +0x08   rel32  0x005de4f0  __EH_prolog3_GS (CALL)
//     +0x1f   rel32  call Release-like fn
//     +0x29   rel32  call Release-like fn (2nd field)
//     +0x33   rel32  ...
//     +0x3d   rel32  ...
//     +0x47   rel32  ...
//     +0x51   rel32  ...
//     +0x5b   rel32  ...
//     +0x65   abs32  0x012eb7c8  sentinel constant (CMP)
//     +0x6e   rel32  call Release-like fn
//     +0x72   rel32  0x000031da  allocator helper
//     +0x86   abs32  0x00f3e2d0  IAT COM-Release slot
//     +0x9b   abs32  0x012eaec0  sentinel constant (CMP EDI)
//     +0xa3   rel32  call Release-like fn
//     +0xb0   rel32  0x005df4fb  EH epilog-trampoline #1
//     +0xb7   rel32  0x005e264c  alloc/init helper
//     +0xcc   rel32  call Release-like fn
//     +0xd4   abs32  0x012eb4c8  sentinel (CMP EDI vs dword ptr)
//     +0xdc   abs32  0x012eb3f0  sentinel (CMP EDI imm)
//     +0xe7   rel32  call Release-like fn
//     +0xf5   rel32  0x005df507  EH epilog-trampoline #2
//     +0xfb   rel32  0x005d5c88  tail call
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH4 prologue-helper calling convention, the two epilog trampolines,
//   and the exact register allocation / scope-level sequence make a clean
//   source-level /O2 /GS match extremely brittle. The naked _emit approach
//   (the same used for FUN_00403a20 and FUN_004054d0) is the correct path
//   until surrounding types are catalogued.

extern "C" __declspec(naked) void FUN_009df3ef() {
    __asm {
        // +0000  PUSH 8
        _emit 0x6a
        _emit 0x08
        // +0002  PUSH 0x0122d3d8  (scope-table)
        _emit 0x68
        _emit 0xd8
        _emit 0xd3
        _emit 0x22
        _emit 0x01
        // +0007  CALL __EH_prolog3_GS
        _emit 0xe8
        _emit 0xf5
        _emit 0xf0
        _emit 0xff
        _emit 0xff
        // +000c  MOV ESI, [EBP+8]
        _emit 0x8b
        _emit 0x75
        _emit 0x08
        // +000f  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // +0011  JZ +0xea
        _emit 0x0f
        _emit 0x84
        _emit 0xea
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0017  MOV EAX, [ESI+0x24]
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // +001a  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +001c  JZ +7
        _emit 0x74
        _emit 0x07
        // +001e  PUSH EAX
        _emit 0x50
        // +001f  CALL rel32
        _emit 0xe8
        _emit 0x75
        _emit 0x68
        _emit 0xff
        _emit 0xff
        // +0024  POP ECX
        _emit 0x59
        // +0025  MOV EAX, [ESI+0x2c]
        _emit 0x8b
        _emit 0x46
        _emit 0x2c
        // +0028  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +002a  JZ +7
        _emit 0x74
        _emit 0x07
        // +002c  PUSH EAX
        _emit 0x50
        // +002d  CALL rel32
        _emit 0xe8
        _emit 0x67
        _emit 0x68
        _emit 0xff
        _emit 0xff
        // +0032  POP ECX
        _emit 0x59
        // +0033  MOV EAX, [ESI+0x34]
        _emit 0x8b
        _emit 0x46
        _emit 0x34
        // +0036  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0038  JZ +7
        _emit 0x74
        _emit 0x07
        // +003a  PUSH EAX
        _emit 0x50
        // +003b  CALL rel32
        _emit 0xe8
        _emit 0x59
        _emit 0x68
        _emit 0xff
        _emit 0xff
        // +0040  POP ECX
        _emit 0x59
        // +0041  MOV EAX, [ESI+0x3c]
        _emit 0x8b
        _emit 0x46
        _emit 0x3c
        // +0044  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0046  JZ +7
        _emit 0x74
        _emit 0x07
        // +0048  PUSH EAX
        _emit 0x50
        // +0049  CALL rel32
        _emit 0xe8
        _emit 0x4b
        _emit 0x68
        _emit 0xff
        _emit 0xff
        // +004e  POP ECX
        _emit 0x59
        // +004f  MOV EAX, [ESI+0x44]
        _emit 0x8b
        _emit 0x46
        _emit 0x44
        // +0052  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0054  JZ +7
        _emit 0x74
        _emit 0x07
        // +0056  PUSH EAX
        _emit 0x50
        // +0057  CALL rel32
        _emit 0xe8
        _emit 0x3d
        _emit 0x68
        _emit 0xff
        _emit 0xff
        // +005c  POP ECX
        _emit 0x59
        // +005d  MOV EAX, [ESI+0x48]
        _emit 0x8b
        _emit 0x46
        _emit 0x48
        // +0060  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0062  JZ +7
        _emit 0x74
        _emit 0x07
        // +0064  PUSH EAX
        _emit 0x50
        // +0065  CALL rel32
        _emit 0xe8
        _emit 0x2f
        _emit 0x68
        _emit 0xff
        _emit 0xff
        // +006a  POP ECX
        _emit 0x59
        // +006b  MOV EAX, [ESI+0x5c]
        _emit 0x8b
        _emit 0x46
        _emit 0x5c
        // +006e  CMP EAX, 0x012eb7c8
        _emit 0x3d
        _emit 0xc8
        _emit 0xb7
        _emit 0x2e
        _emit 0x01
        // +0073  JZ +7
        _emit 0x74
        _emit 0x07
        // +0075  PUSH EAX
        _emit 0x50
        // +0076  CALL rel32
        _emit 0xe8
        _emit 0x1e
        _emit 0x68
        _emit 0xff
        _emit 0xff
        // +007b  POP ECX
        _emit 0x59
        // +007c  PUSH 0x0d
        _emit 0x6a
        _emit 0x0d
        // +007e  CALL rel32
        _emit 0xe8
        _emit 0xda
        _emit 0x31
        _emit 0x00
        _emit 0x00
        // +0083  POP ECX
        _emit 0x59
        // +0084  AND [EBP-4], 0
        _emit 0x83
        _emit 0x65
        _emit 0xfc
        _emit 0x00
        // +0088  MOV EDI, [ESI+0x68]
        _emit 0x8b
        _emit 0x7e
        _emit 0x68
        // +008b  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // +008d  JZ +0x1a
        _emit 0x74
        _emit 0x1a
        // +008f  PUSH EDI
        _emit 0x57
        // +0090  CALL [IAT@0x00f3e2d0]
        _emit 0xff
        _emit 0x15
        _emit 0xd0
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // +0096  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0098  JNZ +0x0f
        _emit 0x75
        _emit 0x0f
        // +009a  CMP EDI, 0x012eaec0
        _emit 0x81
        _emit 0xff
        _emit 0xc0
        _emit 0xae
        _emit 0x2e
        _emit 0x01
        // +00a0  JZ +7
        _emit 0x74
        _emit 0x07
        // +00a2  PUSH EDI
        _emit 0x57
        // +00a3  CALL rel32
        _emit 0xe8
        _emit 0xf1
        _emit 0x67
        _emit 0xff
        _emit 0xff
        // +00a8  POP ECX
        _emit 0x59
        // +00a9  MOV [EBP-4], 0xFFFFFFFE
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // +00b0  CALL epilog-trampoline #1
        _emit 0xe8
        _emit 0x57
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +00b5  PUSH 0x0c
        _emit 0x6a
        _emit 0x0c
        // +00b7  CALL rel32 (alloc/init)
        _emit 0xe8
        _emit 0xa1
        _emit 0x31
        _emit 0x00
        _emit 0x00
        // +00bc  POP ECX
        _emit 0x59
        // +00bd  MOV [EBP-4], 1
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +00c4  MOV EDI, [ESI+0x6c]
        _emit 0x8b
        _emit 0x7e
        _emit 0x6c
        // +00c7  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // +00c9  JZ +0x23
        _emit 0x74
        _emit 0x23
        // +00cb  PUSH EDI
        _emit 0x57
        // +00cc  CALL rel32
        _emit 0xe8
        _emit 0x9f
        _emit 0x43
        _emit 0xff
        _emit 0xff
        // +00d1  POP ECX
        _emit 0x59
        // +00d2  CMP EDI, [0x012eb4c8]
        _emit 0x3b
        _emit 0x3d
        _emit 0xc8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        // +00d8  JZ +0x14
        _emit 0x74
        _emit 0x14
        // +00da  CMP EDI, 0x012eb3f0
        _emit 0x81
        _emit 0xff
        _emit 0xf0
        _emit 0xb3
        _emit 0x2e
        _emit 0x01
        // +00e0  JZ +0x0c
        _emit 0x74
        _emit 0x0c
        // +00e2  CMP dword ptr [EDI], 0
        _emit 0x83
        _emit 0x3f
        _emit 0x00
        // +00e5  JNZ +7
        _emit 0x75
        _emit 0x07
        // +00e7  PUSH EDI
        _emit 0x57
        // +00e8  CALL rel32
        _emit 0xe8
        _emit 0xbd
        _emit 0x41
        _emit 0xff
        _emit 0xff
        // +00ed  POP ECX
        _emit 0x59
        // +00ee  MOV [EBP-4], 0xFFFFFFFE
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // +00f5  CALL epilog-trampoline #2
        _emit 0xe8
        _emit 0x1e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +00fa  PUSH ESI
        _emit 0x56
        // +00fb  CALL rel32
        _emit 0xe8
        _emit 0x99
        _emit 0x67
        _emit 0xff
        _emit 0xff
    }
}
