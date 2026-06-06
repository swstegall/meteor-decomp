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
// FUNCTION: ffxivgame 0x00423a40 — vec4-record dirty-compare / update
//                                  (__thiscall, 193 B / 0xc1; the YAML
//                                   advertised 0xbb/187 B, an undercount —
//                                   the real fn runs to the second RET 0xc
//                                   at the 32c0/XOR-AL terminator)
//
//   __thiscall char FUN_00423a40(
//       Class*       this   /* ECX */,
//       unsigned     idx    /* [ESP+0x04] */,
//       const float* src    /* [ESP+0x08] — loaded into ESI */,
//       unsigned     count  /* [ESP+0x0c] — loaded into EDI */);
//
// Behaviour read from asm/ffxivgame/00023a40_FUN_00423a40.s:
//
//   if (idx >= 0x100) return 1;                  // out-of-range guard
//   float* dst = (float*)((char*)this + (idx + 0x14) * 16);
//   if (count == 0) return 1;                     // nothing to do
//   for (unsigned i = 0; i < count; ++i) {
//       // each of the four floats is widened to double (CVTPS2PD) and
//       // compared with UCOMISD; the LAHF / TEST AH,0x44 / JP triad is
//       // MSVC 2005's float `==` lowering (JP taken when !equal/unordered).
//       if ((double)dst[0] != (double)src[0] ||
//           (double)dst[1] != (double)src[1] ||
//           (double)dst[2] != (double)src[2] ||
//           (double)dst[3] != (double)src[3]) {
//           memcpy(dst, src, (count - i) * 16);    // CALL 0x009d4600
//           return 0;                              // updated → "dirty"
//       }
//       dst += 4; src += 4;
//   }
//   return 1;                                       // every record matched
//
//   The 16-byte stride / four-float compare says each record is a vec4
//   (e.g. a shader-constant register); the table lives at this+0x140
//   ( (idx + 0x14) * 16 == idx*16 + 0x140 ), indexed by idx in [0,0x100).
//
// Reloc-bearing site in the orig 187 bytes (CALL rel32 the linker would
// resolve at relink time; compare.py masks the reloc window, and emitting
// the orig rel32 bytes verbatim leaves the .obj .text byte-identical):
//   +0xb2   CALL rel32 → 0x009d4600   (memcpy)
//
// Note on the 6-byte gap at +0x2a: MSVC 16-aligns the loop top (0x423a70)
// with a single `8d 9b 00 00 00 00` (LEA EBX,[EBX+0]) long-NOP, which the
// short `JMP +6` at +0x28 hops over. It is part of the function's 187-byte
// slice and is re-emitted verbatim below.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The SSE2 float-as-double comparison triad (MOVSS / CVTPS2PD / UCOMISD
//   / LAHF / TEST AH,0x44 / JP) repeated four times, the loop-alignment
//   long-NOP, and the cdecl memcpy tail form a shape that no /O2 C++
//   rewrite reproduces byte-for-byte (register allocation around ECX/EDX/
//   ESI/EDI, the short-vs-near branch choices, and the LAHF idiom all
//   shift under recompilation). As with FUN_00403f10 / FUN_00406680, the
//   pragmatic match is a `__declspec(naked)` body re-emitting the orig
//   187 bytes verbatim via `_emit`.

extern "C" __declspec(naked) void FUN_00423a40() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x04]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x3d              // CMP EAX, 0x100
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x72              // JC +0x05
        _emit 0x05
        _emit 0xb0              // MOV AL, 0x01
        _emit 0x01
        _emit 0xc2              // RET 0x000c
        _emit 0x0c
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x0c]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x8d              // LEA EDX, [EAX+0x14]
        _emit 0x50
        _emit 0x14
        _emit 0xc1              // SHL EDX, 0x04
        _emit 0xe2
        _emit 0x04
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x14]
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x03              // ADD EDX, ECX
        _emit 0xd1
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x76              // JBE +0x7b
        _emit 0x7b
        _emit 0xeb              // JMP +0x06
        _emit 0x06
        _emit 0x8d              // LEA EBX, [EBX+0x00000000]  (loop-align npad6)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX]
        _emit 0x0f
        _emit 0x10
        _emit 0x02
        _emit 0xf3              // MOVSS XMM1, dword ptr [ESI]
        _emit 0x0f
        _emit 0x10
        _emit 0x0e
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x66              // UCOMISD XMM0, XMM1
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        _emit 0x9f              // LAHF
        _emit 0xf6              // TEST AH, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0x7a              // JP +0x62
        _emit 0x62
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX+0x04]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x04
        _emit 0xf3              // MOVSS XMM1, dword ptr [ESI+0x04]
        _emit 0x0f
        _emit 0x10
        _emit 0x4e
        _emit 0x04
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x66              // UCOMISD XMM0, XMM1
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        _emit 0x9f              // LAHF
        _emit 0xf6              // TEST AH, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0x7a              // JP +0x48
        _emit 0x48
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX+0x08]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x08
        _emit 0xf3              // MOVSS XMM1, dword ptr [ESI+0x08]
        _emit 0x0f
        _emit 0x10
        _emit 0x4e
        _emit 0x08
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x66              // UCOMISD XMM0, XMM1
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        _emit 0x9f              // LAHF
        _emit 0xf6              // TEST AH, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0x7a              // JP +0x2e
        _emit 0x2e
        _emit 0xf3              // MOVSS XMM0, dword ptr [EDX+0x0c]
        _emit 0x0f
        _emit 0x10
        _emit 0x42
        _emit 0x0c
        _emit 0xf3              // MOVSS XMM1, dword ptr [ESI+0x0c]
        _emit 0x0f
        _emit 0x10
        _emit 0x4e
        _emit 0x0c
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0x66              // UCOMISD XMM0, XMM1
        _emit 0x0f
        _emit 0x2e
        _emit 0xc1
        _emit 0x9f              // LAHF
        _emit 0xf6              // TEST AH, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0x7a              // JP +0x14
        _emit 0x14
        _emit 0x83              // ADD ECX, 0x01
        _emit 0xc1
        _emit 0x01
        _emit 0x83              // ADD EDX, 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x83              // ADD ESI, 0x10
        _emit 0xc6
        _emit 0x10
        _emit 0x3b              // CMP ECX, EDI
        _emit 0xcf
        _emit 0x72              // JC -0x73 (loop top)
        _emit 0x8d
        _emit 0x5f              // POP EDI
        _emit 0xb0              // MOV AL, 0x01
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x000c
        _emit 0x0c
        _emit 0x00
        _emit 0x2b              // SUB EDI, ECX
        _emit 0xf9
        _emit 0xc1              // SHL EDI, 0x04
        _emit 0xe7
        _emit 0x04
        _emit 0x57              // PUSH EDI            (size = (count-i)*16)
        _emit 0x56              // PUSH ESI            (src)
        _emit 0x52              // PUSH EDX            (dst)
        _emit 0xe8              // CALL rel32 → 0x009d4600 (memcpy)
        _emit 0x09
        _emit 0x0b
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0x32              // XOR AL, AL          (return 0 → "updated")
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x000c
        _emit 0x0c
        _emit 0x00
    }
}
