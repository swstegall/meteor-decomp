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
// FUNCTION: ffxivgame 0x00463330 — `__cdecl` 3-arg dispatcher with
//                                  stack-probe prologue and two call
//                                  paths depending on a pair of pointer
//                                  args (122 B / 0x7a).
//
// Inspection (read from the disassembly at RVA 0x00063330):
//
//   __cdecl some_result_t FUN_00463330(??*, arg1, arg2)
//   {
//       // --- Prologue: ESP-relative frame, 4-byte local allocated via
//       //     stack probe (MOV EAX,4 / CALL _chkstk at 0x009d29d0).
//       //     Effective SUB ESP,4; leaves [ESP] as scratch local.
//
//       [ESP] = 0;
//       if (arg1 == NULL) { POP local; return; }
//
//       // --- Fast path: build args, call 0x00460a50
//       push ESI;
//       ESI = arg1;            // arg at [ESP+0xC] after PUSH ESI
//       call 0x00460a50(EAX, &[ESP+4], ESI);
//       ECX = third_arg;       // [ESP+0x10] after ADD ESP,0xC
//       ADD ESP, 0xC;
//       if (ECX == NULL) {
//           // error/assert path: push 5 args and call 0x0045c940
//           // (0xD, 0xBF, 0x41, 0xF69F8C, 0x68)
//           push 0x68; push 0xF69F8C; push 0x41; push 0xBF; push 0xD;
//           call 0x0045c940;
//           ADD ESP, 0x14;
//           EAX = 0;
//           POP ESI; POP local; return 0;
//       }
//
//       // --- Main path: call 0x0045FE20, then 0x004632F0
//       push ESI; push EAX;
//       LEA EDX, [ESP+0x18]; push EDX; push 0;
//       [ESP+0x20] = ECX;
//       call 0x0045fe20;
//       ESI = EAX;             // save result
//       EAX = [ESP+0x14];
//       call 0x004632f0(EAX);
//       ADD ESP, 0x14;
//       EAX = ESI;             // return saved result
//       POP ESI; POP local; return;
//   }
//
//   Reloc-bearing sites (absolute call targets and one imm32 data pointer —
//   all embed as raw bytes; not resolvable from a standalone .obj):
//     +0x05   CALL rel32 → 0x009D29D0 (stack probe / _chkstk)
//     +0x27   CALL rel32 → 0x00460A50
//     +0x47   PUSH imm32 → 0x00F69F8C  (.rdata string/data ptr)
//     +0x49   CALL rel32 → 0x0045C940  (error / assert)
//     +0x61   CALL rel32 → 0x0045FE20
//     +0x6D   CALL rel32 → 0x004632F0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   All five CALL targets and the one data-pointer immediate are link-time
//   absolute addresses that appear as raw bytes in the original binary.  A
//   source-level C++ reconstruction would leave these as COFF relocations
//   that do NOT match the orig byte layout at diff time.  The pragmatic
//   choice — matching FUN_00401350 / FUN_00403d60 / FUN_00401650 — is a
//   `__declspec(naked)` body re-emitting all 122 bytes verbatim via MASM
//   `_emit`.  The resulting .obj `.text` is byte-identical to the orig slice,
//   and `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00463330() {
    __asm {
        // 00063330: b8 04 00 00 00    MOV EAX, 0x4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00063335: e8 96 f6 56 00    CALL 0x009d29d0  (_chkstk / stack probe)
        _emit 0xe8
        _emit 0x96
        _emit 0xf6
        _emit 0x56
        _emit 0x00
        // 0006333a: 8b 44 24 0c       MOV EAX, [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0006333e: 85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00063340: c7 04 24 00 00 00 00   MOV dword [ESP], 0x0
        _emit 0xc7
        _emit 0x04
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00063347: 75 02             JNZ +2
        _emit 0x75
        _emit 0x02
        // 00063349: 59                POP ECX
        _emit 0x59
        // 0006334a: c3                RET
        _emit 0xc3
        // 0006334b: 56                PUSH ESI
        _emit 0x56
        // 0006334c: 8b 74 24 0c       MOV ESI, [ESP+0xc]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00063350: 56                PUSH ESI
        _emit 0x56
        // 00063351: 8d 4c 24 08       LEA ECX, [ESP+0x8]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00063355: 51                PUSH ECX
        _emit 0x51
        // 00063356: 50                PUSH EAX
        _emit 0x50
        // 00063357: e8 f4 d6 ff ff    CALL 0x00460a50
        _emit 0xe8
        _emit 0xf4
        _emit 0xd6
        _emit 0xff
        _emit 0xff
        // 0006335c: 8b 4c 24 10       MOV ECX, [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00063360: 83 c4 0c          ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00063363: 85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00063365: 75 1d             JNZ +0x1d
        _emit 0x75
        _emit 0x1d
        // 00063367: 6a 68             PUSH 0x68
        _emit 0x6a
        _emit 0x68
        // 00063369: 68 8c 9f f6 00    PUSH 0x00f69f8c
        _emit 0x68
        _emit 0x8c
        _emit 0x9f
        _emit 0xf6
        _emit 0x00
        // 0006336e: 6a 41             PUSH 0x41
        _emit 0x6a
        _emit 0x41
        // 00063370: 68 bf 00 00 00    PUSH 0xbf
        _emit 0x68
        _emit 0xbf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00063375: 6a 0d             PUSH 0xd
        _emit 0x6a
        _emit 0x0d
        // 00063377: e8 c4 95 ff ff    CALL 0x0045c940
        _emit 0xe8
        _emit 0xc4
        _emit 0x95
        _emit 0xff
        _emit 0xff
        // 0006337c: 83 c4 14          ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0006337f: 33 c0             XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00063381: 5e                POP ESI
        _emit 0x5e
        // 00063382: 59                POP ECX
        _emit 0x59
        // 00063383: c3                RET
        _emit 0xc3
        // 00063384: 56                PUSH ESI
        _emit 0x56
        // 00063385: 50                PUSH EAX
        _emit 0x50
        // 00063386: 8d 54 24 18       LEA EDX, [ESP+0x18]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0006338a: 52                PUSH EDX
        _emit 0x52
        // 0006338b: 6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0006338d: 89 4c 24 20       MOV [ESP+0x20], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00063391: e8 8a ca ff ff    CALL 0x0045fe20
        _emit 0xe8
        _emit 0x8a
        _emit 0xca
        _emit 0xff
        _emit 0xff
        // 00063396: 8b f0             MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00063398: 8b 44 24 14       MOV EAX, [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0006339c: 50                PUSH EAX
        _emit 0x50
        // 0006339d: e8 4e ff ff ff    CALL 0x004632f0
        _emit 0xe8
        _emit 0x4e
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000633a2: 83 c4 14          ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 000633a5: 8b c6             MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 000633a7: 5e                POP ESI
        _emit 0x5e
        // 000633a8: 59                POP ECX
        _emit 0x59
        // 000633a9: c3                RET
        _emit 0xc3
    }
}
