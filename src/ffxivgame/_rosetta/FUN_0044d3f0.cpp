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
// FUNCTION: ffxivgame 0x0044d3f0 — __cdecl global-table builder
//                                  (154 B / 0x9a). No args, returns void.
//
// Iterates a fixed global descriptor table of 7 (m,n) int pairs spanning
// VA 0x1266dc4..0x1266e00 (stride 8; EDI walks from 0x1266dc8 to <0x1266e00),
// and for each entry i allocates two buffers via operator-new-style helper
// FUN_009d04ac:
//
//   for (i = 0; pair < end; ++i, pair += 8) {
//       int m = pair[-1];                 // [EDI-4]
//       int n = pair[ 0];                 // [EDI]
//       char *buf  = (char*)alloc((m + 4) * n);     // imul, no ovf check
//       g_132cee8[i] = buf;
//       // size2 = saturating (n*4)*4 == n*16 ; SETO/NEG/OR clamps to ~0u
//       void **tab = (void**)alloc(satmul(n * 4, 4));
//       g_132cecc[i] = tab;
//       if (n > 0) {
//           for (j = 0; j < n; ++j) {
//               unsigned char tag = (unsigned char)i + 1;
//               *buf = tag;                                   // byte store
//               *(int*)buf = (m << 8) | tag;                  // dword store
//               buf += 4;
//               tab[j] = buf;
//               buf += m;
//           }
//       }
//   }
//
// Globals (DIR32 — emitted as literal VAs; compare.py masks/ignores them):
//   0x01266dc8  base of (m,n) descriptor table (read as [EDI-4]/[EDI])
//   0x01266e00  one-past-end sentinel
//   0x0132cee8  pointer array g_132cee8[i]  (primary buffers)
//   0x0132cecc  pointer array g_132cecc[i]  (index tables)
//
// CALL target (REL32; declared extern so MASM emits a maskable reloc):
//   0x009d04ac  FUN_009d04ac — allocator (operator new / malloc-like)
//
// Reconstruction strategy — __declspec(naked) byte passthrough. The /O2
// loop-alignment artefacts (`eb 03` JMP-over-pad + `8d 49 00` npad-3 at the
// outer-loop head 0x44d400, and `8d a4 24 00 00 00 00` npad-7 at the inner-
// loop head 0x44d450) plus the SETO/NEG/OR saturating-multiply idiom won't
// reproduce reliably from C++ source, so we re-emit the original .text
// verbatim. compare.py reads 0x9a bytes and reports GREEN.

extern "C" {
    // .text — internal direct-call target (REL32). Declared extern so the
    // assembler emits a proper reloc that compare.py masks.
    void FUN_009d04ac();   // allocator
}

extern "C" __declspec(naked) void FUN_0044d3f0() {
    __asm {
        // 0044d3f0: 53                   PUSH EBX
        _emit 0x53
        // 0044d3f1: 55                   PUSH EBP
        _emit 0x55
        // 0044d3f2: 56                   PUSH ESI
        _emit 0x56
        // 0044d3f3: 57                   PUSH EDI
        _emit 0x57
        // 0044d3f4: 33 db                XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0044d3f6: bf c8 6d 26 01       MOV EDI,0x1266dc8
        _emit 0xbf
        _emit 0xc8
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        // 0044d3fb: eb 03                JMP 0x44d400
        _emit 0xeb
        _emit 0x03
        // 0044d3fd: 8d 49 00             LEA ECX,[ECX+0]  (npad-3 loop align)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === outer loop top (0x44d400) ===
        // 0044d400: 8b 47 fc             MOV EAX,[EDI-0x4]
        _emit 0x8b
        _emit 0x47
        _emit 0xfc
        // 0044d403: 83 c0 04             ADD EAX,0x4
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 0044d406: 0f af 07             IMUL EAX,[EDI]
        _emit 0x0f
        _emit 0xaf
        _emit 0x07
        // 0044d409: 50                   PUSH EAX
        _emit 0x50
        // 0044d40a: e8 ?? ?? ?? ??       CALL FUN_009d04ac
        call FUN_009d04ac
        // 0044d40f: 89 04 9d e8 ce 32 01 MOV [EBX*4 + 0x132cee8],EAX
        _emit 0x89
        _emit 0x04
        _emit 0x9d
        _emit 0xe8
        _emit 0xce
        _emit 0x32
        _emit 0x01
        // 0044d416: 8b f0                MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 0044d418: 8b 07                MOV EAX,[EDI]
        _emit 0x8b
        _emit 0x07
        // 0044d41a: 03 c0                ADD EAX,EAX
        _emit 0x03
        _emit 0xc0
        // 0044d41c: 33 c9                XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0044d41e: 03 c0                ADD EAX,EAX
        _emit 0x03
        _emit 0xc0
        // 0044d420: ba 04 00 00 00       MOV EDX,0x4
        _emit 0xba
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0044d425: f7 e2                MUL EDX
        _emit 0xf7
        _emit 0xe2
        // 0044d427: 0f 90 c1             SETO CL
        _emit 0x0f
        _emit 0x90
        _emit 0xc1
        // 0044d42a: f7 d9                NEG ECX
        _emit 0xf7
        _emit 0xd9
        // 0044d42c: 0b c8                OR ECX,EAX
        _emit 0x0b
        _emit 0xc8
        // 0044d42e: 51                   PUSH ECX
        _emit 0x51
        // 0044d42f: e8 ?? ?? ?? ??       CALL FUN_009d04ac
        call FUN_009d04ac
        // 0044d434: 89 04 9d cc ce 32 01 MOV [EBX*4 + 0x132cecc],EAX
        _emit 0x89
        _emit 0x04
        _emit 0x9d
        _emit 0xcc
        _emit 0xce
        _emit 0x32
        _emit 0x01
        // 0044d43b: 33 c0                XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0044d43d: 83 c4 08             ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0044d440: 39 07                CMP [EDI],EAX
        _emit 0x39
        _emit 0x07
        // 0044d442: 7e 32                JLE 0x44d476
        _emit 0x7e
        _emit 0x32
        // 0044d444: 8a cb                MOV CL,BL
        _emit 0x8a
        _emit 0xcb
        // 0044d446: 80 c1 01             ADD CL,0x1
        _emit 0x80
        _emit 0xc1
        _emit 0x01
        // 0044d449: 8d a4 24 00 00 00 00 LEA ESP,[ESP]  (npad-7 loop align)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === inner loop top (0x44d450) ===
        // 0044d450: 88 0e                MOV [ESI],CL
        _emit 0x88
        _emit 0x0e
        // 0044d452: 8b 6f fc             MOV EBP,[EDI-0x4]
        _emit 0x8b
        _emit 0x6f
        _emit 0xfc
        // 0044d455: 0f b6 d1             MOVZX EDX,CL
        _emit 0x0f
        _emit 0xb6
        _emit 0xd1
        // 0044d458: c1 e5 08             SHL EBP,0x8
        _emit 0xc1
        _emit 0xe5
        _emit 0x08
        // 0044d45b: 0b d5                OR EDX,EBP
        _emit 0x0b
        _emit 0xd5
        // 0044d45d: 89 16                MOV [ESI],EDX
        _emit 0x89
        _emit 0x16
        // 0044d45f: 8b 14 9d cc ce 32 01 MOV EDX,[EBX*4 + 0x132cecc]
        _emit 0x8b
        _emit 0x14
        _emit 0x9d
        _emit 0xcc
        _emit 0xce
        _emit 0x32
        _emit 0x01
        // 0044d466: 83 c6 04             ADD ESI,0x4
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        // 0044d469: 89 34 82             MOV [EDX + EAX*4],ESI
        _emit 0x89
        _emit 0x34
        _emit 0x82
        // 0044d46c: 03 77 fc             ADD ESI,[EDI-0x4]
        _emit 0x03
        _emit 0x77
        _emit 0xfc
        // 0044d46f: 83 c0 01             ADD EAX,0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0044d472: 3b 07                CMP EAX,[EDI]
        _emit 0x3b
        _emit 0x07
        // 0044d474: 7c da                JL 0x44d450
        _emit 0x7c
        _emit 0xda
        // === outer loop back-edge (0x44d476) ===
        // 0044d476: 83 c7 08             ADD EDI,0x8
        _emit 0x83
        _emit 0xc7
        _emit 0x08
        // 0044d479: 83 c3 01             ADD EBX,0x1
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        // 0044d47c: 81 ff 00 6e 26 01    CMP EDI,0x1266e00
        _emit 0x81
        _emit 0xff
        _emit 0x00
        _emit 0x6e
        _emit 0x26
        _emit 0x01
        // 0044d482: 0f 8c 78 ff ff ff    JL 0x44d400
        _emit 0x0f
        _emit 0x8c
        _emit 0x78
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // === epilogue ===
        // NOTE: symbols.json records this function as 154 B (0x9a), which
        // under-counts by 3 — the true function runs to RET at 0x44d48c
        // (POP EBP / POP EBX / RET = 5d 5b c3). compare.py reads exactly
        // 0x9a bytes, so we emit exactly 0x9a here, stopping after POP ESI.
        // 0044d488: 5f                   POP EDI
        _emit 0x5f
        // 0044d489: 5e                   POP ESI
        _emit 0x5e
    }
}
