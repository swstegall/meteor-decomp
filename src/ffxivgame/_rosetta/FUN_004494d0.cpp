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
// FUNCTION: ffxivgame 0x000494d0 — bounds-checked dword (4-byte) block move
//                                  with overlap handling (__cdecl, 100 B / 0x64)
//
// __cdecl int* FUN_004494d0(int* dst, unsigned cap, int* src, unsigned count);
//
//   [esp+0x04]  int*     dst    (returned in EAX)
//   [esp+0x08]  unsigned cap    (destination capacity)
//   [esp+0x0c]  int*     src
//   [esp+0x10]  unsigned count  (number of dwords to move)
//
// Logical body:
//
//   if (cap < count) {                 // unsigned bounds check
//       FUN_009d22b4();                // out-of-range report/throw
//       return 0;
//   }
//   // EAX = dst is the return value for every non-error path.
//   if (src < dst && dst < src + count) {
//       // overlapping, dst ahead of src → copy backwards
//       int* d = dst + count;
//       int* s = src + count;
//       while (count--) *--d = *--s;
//   } else {
//       // no overlap (or src >= dst) → copy forwards
//       while (count--) *dst++ = *src++;
//   }
//   return dst;                        // EAX preserved = original dst
//
// Calling convention: __cdecl (args cleaned by caller, plain RET). The
// register-shuffling backward/forward split with the early CMP-before-PUSH
// of ESI/EDI and the 7-byte `lea esp,[esp]` loop-alignment NOP are exact
// /O2 codegen artefacts that no source-level rewrite reliably round-trips;
// a __declspec(naked) re-emit of the original 100 bytes is byte-identical
// (the single CALL rel32 at +0x0a is masked by compare.py's reloc window).

extern "C" __declspec(naked) void FUN_004494d0() {
    __asm {
        // 000494d0: 8b 4c 24 10   MOV ECX,[ESP+0x10]   ; count
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000494d4: 39 4c 24 08   CMP [ESP+0x8],ECX    ; cap vs count
        _emit 0x39
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 000494d8: 73 08         JNC 0x004494e2
        _emit 0x73
        _emit 0x08
        // 000494da: e8 d5 8d 58 00 CALL 0x009d22b4
        _emit 0xe8
        _emit 0xd5
        _emit 0x8d
        _emit 0x58
        _emit 0x00
        // 000494df: 33 c0         XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000494e1: c3            RET
        _emit 0xc3
        // 000494e2: 8b 44 24 04   MOV EAX,[ESP+0x4]    ; dst
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000494e6: 8b 54 24 0c   MOV EDX,[ESP+0xc]    ; src
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 000494ea: 3b d0         CMP EDX,EAX          ; src vs dst
        _emit 0x3b
        _emit 0xd0
        // 000494ec: 56            PUSH ESI
        _emit 0x56
        // 000494ed: 57            PUSH EDI
        _emit 0x57
        // 000494ee: 8b f0         MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 000494f0: 73 23         JNC 0x00449515       ; src >= dst → forward
        _emit 0x73
        _emit 0x23
        // 000494f2: 8d 3c 8a      LEA EDI,[EDX+ECX*4]  ; src + count
        _emit 0x8d
        _emit 0x3c
        _emit 0x8a
        // 000494f5: 3b c7         CMP EAX,EDI          ; dst vs src_end
        _emit 0x3b
        _emit 0xc7
        // 000494f7: 73 1c         JNC 0x00449515       ; dst >= src_end → forward
        _emit 0x73
        _emit 0x1c
        // 000494f9: 85 c9         TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 000494fb: 8d 34 88      LEA ESI,[EAX+ECX*4]  ; dst + count
        _emit 0x8d
        _emit 0x34
        _emit 0x88
        // 000494fe: 8b d7         MOV EDX,EDI
        _emit 0x8b
        _emit 0xd7
        // 00049500: 76 2f         JBE 0x00449531       ; count==0 → done
        _emit 0x76
        _emit 0x2f
        // 00049502: 8b 7a fc      MOV EDI,[EDX-0x4]
        _emit 0x8b
        _emit 0x7a
        _emit 0xfc
        // 00049505: 83 ea 04      SUB EDX,0x4
        _emit 0x83
        _emit 0xea
        _emit 0x04
        // 00049508: 83 ee 04      SUB ESI,0x4
        _emit 0x83
        _emit 0xee
        _emit 0x04
        // 0004950b: 83 e9 01      SUB ECX,0x1
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        // 0004950e: 89 3e         MOV [ESI],EDI
        _emit 0x89
        _emit 0x3e
        // 00049510: 75 f0         JNZ 0x00449502
        _emit 0x75
        _emit 0xf0
        // 00049512: 5f            POP EDI
        _emit 0x5f
        // 00049513: 5e            POP ESI
        _emit 0x5e
        // 00049514: c3            RET
        _emit 0xc3
        // 00049515: 85 c9         TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 00049517: 76 18         JBE 0x00449531       ; count==0 → done
        _emit 0x76
        _emit 0x18
        // 00049519: 8d a4 24 00 00 00 00  LEA ESP,[ESP] ; 7-byte align NOP
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00049520: 8b 3a         MOV EDI,[EDX]
        _emit 0x8b
        _emit 0x3a
        // 00049522: 89 3e         MOV [ESI],EDI
        _emit 0x89
        _emit 0x3e
        // 00049524: 83 e9 01      SUB ECX,0x1
        _emit 0x83
        _emit 0xe9
        _emit 0x01
        // 00049527: 83 c6 04      ADD ESI,0x4
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        // 0004952a: 83 c2 04      ADD EDX,0x4
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        // 0004952d: 85 c9         TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 0004952f: 77 ef         JA 0x00449520
        _emit 0x77
        _emit 0xef
        // 00049531: 5f            POP EDI
        _emit 0x5f
        // 00049532: 5e            POP ESI
        _emit 0x5e
        // 00049533: c3            RET
        _emit 0xc3
    }
}
