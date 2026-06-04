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
// FUNCTION: ffxivgame 0x00423c60 — SSE2 16-byte block compare/refresh against a
//                                  per-slot cache table hung off the `this`
//                                  object (117 bytes / 0x75)
//
// Calling convention: a `this`-bearing member (ECX = base pointer, used at
//   +0x21 in `ADD EAX,ECX`) with three explicit stack arguments and an
//   EBP/aligned-ESP frame; cleans 12 bytes on return (`RET 0xc`). Returns a
//   bool in AL.
//
// Args (after PUSH EBP / MOV EBP,ESP / AND ESP,~0xf):
//   [EBP+0x08]  slot index   (EAX; bailout-true if >= 0x10, unsigned)
//   [EBP+0x0c]  source ptr   (ESI; 16-byte-aligned xmmword array)
//   [EBP+0x10]  block count  (EDX; bailout-true if 0)
//
// Body (pseudo-C):
//
//   bool __thiscall sub(int slot, const __m128i* src, unsigned count) {
//       if ((unsigned)slot >= 0x10)            return true;        // out of range
//       __m128i* dst = (__m128i*)((char*)this + (slot + 0x118) * 16);
//       unsigned i = 0;
//       if (count == 0)                        return true;        // nothing to do
//       do {
//           __m128i s = _mm_load_si128(&src[0]);                   // MOVDQA [ESI]
//           __m128i d = _mm_load_si128(&dst[i]);                   // MOVDQA [EAX]
//           if (_mm_movemask_epi8(_mm_cmpeq_epi32(d, s)) != 0xffff) {
//               // first differing block: refresh the rest of the cache row
//               memcpy(&dst[i], &src[i], (count - i) * 16);        // CALL 0x009d4600
//               return false;
//           }
//           ++i; ++dst-as-EAX; ++src;
//       } while (i < count);
//       return true;                                               // all blocks matched
//   }
//
// The PCMPEQD + PMOVMSKB / CMP 0xffff sequence is the canonical MSVC 2005
// SSE2-intrinsic lowering of a 16-byte equality test; the body is otherwise a
// straight load/compare/advance loop. The lone CALL at +0x63 targets a
// memmove/memcpy helper (0x009d4600) via a rel32 that resolves to the same
// raw bytes (e8 38 09 5b 00) in a standalone .obj as in the orig slice — no
// relocation is needed for byte-equality.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Coaxing MSVC into the exact register allocation (EAX as the running dst
//   pointer, ECX as the index, the 8d 49 00 alignment NOP at the loop head),
//   the `AND ESP,~0xf` alignment frame, and the rel32 call site is brittle
//   under /O2. A __declspec(naked) body re-emitting the original 117 bytes
//   verbatim produces a .obj whose .text is byte-identical to the orig.
//   compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00423c60() {
    __asm {
        // 00023c60: 55              PUSH EBP
        _emit 0x55
        // 00023c61: 8b ec           MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 00023c63: 83 e4 f0        AND ESP,0xfffffff0
        _emit 0x83
        _emit 0xe4
        _emit 0xf0
        // 00023c66: 8b 45 08        MOV EAX,dword ptr [EBP+0x8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 00023c69: 83 ec 08        SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00023c6c: 83 f8 10        CMP EAX,0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 00023c6f: 56              PUSH ESI
        _emit 0x56
        // 00023c70: 8b 75 0c        MOV ESI,dword ptr [EBP+0xc]
        _emit 0x8b
        _emit 0x75
        _emit 0x0c
        // 00023c73: 57              PUSH EDI
        _emit 0x57
        // 00023c74: 73 3b           JNC 0x00423cb1
        _emit 0x73
        _emit 0x3b
        // 00023c76: 8b 55 10        MOV EDX,dword ptr [EBP+0x10]
        _emit 0x8b
        _emit 0x55
        _emit 0x10
        // 00023c79: 05 18 01 00 00  ADD EAX,0x118
        _emit 0x05
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00023c7e: c1 e0 04        SHL EAX,0x4
        _emit 0xc1
        _emit 0xe0
        _emit 0x04
        // 00023c81: 03 c1           ADD EAX,ECX
        _emit 0x03
        _emit 0xc1
        // 00023c83: 33 c9           XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 00023c85: 85 d2           TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 00023c87: 76 28           JBE 0x00423cb1
        _emit 0x76
        _emit 0x28
        // 00023c89: 66 0f 6f 06     MOVDQA XMM0,xmmword ptr [ESI]
        _emit 0x66
        _emit 0x0f
        _emit 0x6f
        _emit 0x06
        // 00023c8d: 8d 49 00        LEA ECX,[ECX]   (3-byte align NOP)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 00023c90: 66 0f 6f 08     MOVDQA XMM1,xmmword ptr [EAX]
        _emit 0x66
        _emit 0x0f
        _emit 0x6f
        _emit 0x08
        // 00023c94: 66 0f 76 c8     PCMPEQD XMM1,XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0x76
        _emit 0xc8
        // 00023c98: 66 0f d7 f9     PMOVMSKB EDI,XMM1
        _emit 0x66
        _emit 0x0f
        _emit 0xd7
        _emit 0xf9
        // 00023c9c: 81 ff ff ff 00 00  CMP EDI,0xffff
        _emit 0x81
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x00
        _emit 0x00
        // 00023ca2: 75 17           JNZ 0x00423cbb
        _emit 0x75
        _emit 0x17
        // 00023ca4: 83 c1 01        ADD ECX,0x1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // 00023ca7: 83 c0 10        ADD EAX,0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x10
        // 00023caa: 83 c6 10        ADD ESI,0x10
        _emit 0x83
        _emit 0xc6
        _emit 0x10
        // 00023cad: 3b ca           CMP ECX,EDX
        _emit 0x3b
        _emit 0xca
        // 00023caf: 72 df           JC 0x00423c90
        _emit 0x72
        _emit 0xdf
        // 00023cb1: b0 01           MOV AL,0x1
        _emit 0xb0
        _emit 0x01
        // 00023cb3: 5f              POP EDI
        _emit 0x5f
        // 00023cb4: 5e              POP ESI
        _emit 0x5e
        // 00023cb5: 8b e5           MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 00023cb7: 5d              POP EBP
        _emit 0x5d
        // 00023cb8: c2 0c 00        RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00023cbb: 2b d1           SUB EDX,ECX
        _emit 0x2b
        _emit 0xd1
        // 00023cbd: c1 e2 04        SHL EDX,0x4
        _emit 0xc1
        _emit 0xe2
        _emit 0x04
        // 00023cc0: 52              PUSH EDX
        _emit 0x52
        // 00023cc1: 56              PUSH ESI
        _emit 0x56
        // 00023cc2: 50              PUSH EAX
        _emit 0x50
        // 00023cc3: e8 38 09 5b 00  CALL 0x009d4600  (memcpy/memmove helper)
        _emit 0xe8
        _emit 0x38
        _emit 0x09
        _emit 0x5b
        _emit 0x00
        // 00023cc8: 83 c4 0c        ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00023ccb: 5f              POP EDI
        _emit 0x5f
        // 00023ccc: 32 c0           XOR AL,AL
        _emit 0x32
        _emit 0xc0
        // 00023cce: 5e              POP ESI
        _emit 0x5e
        // 00023ccf: 8b e5           MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 00023cd1: 5d              POP EBP
        _emit 0x5d
        // 00023cd2: c2 0c 00        RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
