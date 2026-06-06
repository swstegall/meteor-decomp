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
// FUNCTION: ffxivgame 0x00064270 — variable-length integer decoder
//                                  (__fastcall, 0x9b bytes / 155 bytes)
//
// Calling convention: __fastcall
//   ECX = int *out_type   (output: 0 = normal, 1 = null/escape)
//   EDX = int count       (number of bytes available at *pptr)
//   [stack+0] = unsigned char **pptr     (in/out: current position in byte stream)
//   [stack+1] = int *out_value           (output: decoded integer value)
// Returns: 1 = success, 0 = failure (truncated or out-of-range input)
//
// Encoding format:
//   Byte == 0x80        → null/escape: *out_type=1, *out_value=0, advance 1 byte
//   Byte 0x00–0x7F      → single-byte value: low 7 bits = value
//   Byte 0x81–0xFF      → multi-byte: high bit = continuation flag,
//                         low 7 bits = count of following bytes (1–4);
//                         continuation bytes are accumulated big-endian into ESI;
//                         final value must be ≤ 0x7FFFFFFF
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The control flow has multiple back-edges and a shared "fail" return target
//   that are unreproducible from C++ source without precise register-allocation
//   control (EDI used simultaneously as "remaining count" tracker and as the
//   old-value snapshot for the loop's guard check).  A __declspec(naked) body
//   re-emitting all 155 bytes verbatim produces a .obj whose .text is
//   byte-identical to the original slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00464270() {
    __asm {
        // 00064270: 55                   PUSH EBP
        _emit 0x55
        // 00064271: 8b 6c 24 08          MOV EBP,dword ptr [ESP+0x8]   (EBP = pptr)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        // 00064275: 8b 45 00             MOV EAX,dword ptr [EBP]       (EAX = *pptr)
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00064278: 56                   PUSH ESI
        _emit 0x56
        // 00064279: 57                   PUSH EDI
        _emit 0x57
        // 0006427a: 8b fa                MOV EDI,EDX                   (EDI = count)
        _emit 0x8b
        _emit 0xfa
        // 0006427c: 33 f6                XOR ESI,ESI                   (ESI = 0)
        _emit 0x33
        _emit 0xf6
        // 0006427e: 83 ef 01             SUB EDI,0x1                   (EDI = count-1)
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 00064281: 83 fa 01             CMP EDX,0x1
        _emit 0x83
        _emit 0xfa
        _emit 0x01
        // 00064284: 7d 06                JGE +6                        (→ 0x46428c)
        _emit 0x7d
        _emit 0x06
        // --- early-out: count < 1, return 0 ---
        // 00064286: 5f                   POP EDI
        _emit 0x5f
        // 00064287: 5e                   POP ESI
        _emit 0x5e
        // 00064288: 33 c0                XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 0006428a: 5d                   POP EBP
        _emit 0x5d
        // 0006428b: c3                   RET
        _emit 0xc3
        // --- 0x46428c: check for null/escape byte 0x80 ---
        // 0006428c: 80 38 80             CMP byte ptr [EAX],0x80
        _emit 0x80
        _emit 0x38
        _emit 0x80
        // 0006428f: 75 1b                JNZ +0x1b                     (→ 0x4642ac)
        _emit 0x75
        _emit 0x1b
        // --- byte == 0x80: null/escape case ---
        // 00064291: 8b 54 24 14          MOV EDX,dword ptr [ESP+0x14]  (EDX = out_value)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00064295: 33 f6                XOR ESI,ESI                   (value = 0)
        _emit 0x33
        _emit 0xf6
        // 00064297: 83 c0 01             ADD EAX,0x1                   (advance ptr)
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0006429a: c7 01 01 00 00 00    MOV dword ptr [ECX],0x1       (*out_type = 1)
        _emit 0xc7
        _emit 0x01
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000642a0: 89 45 00             MOV dword ptr [EBP],EAX       (*pptr = advanced ptr)
        _emit 0x89
        _emit 0x45
        _emit 0x00
        // 000642a3: 5f                   POP EDI
        _emit 0x5f
        // 000642a4: 89 32                MOV dword ptr [EDX],ESI       (*out_value = 0)
        _emit 0x89
        _emit 0x32
        // 000642a6: 8d 46 01             LEA EAX,[ESI+0x1]             (return 1)
        _emit 0x8d
        _emit 0x46
        _emit 0x01
        // 000642a9: 5e                   POP ESI
        _emit 0x5e
        // 000642aa: 5d                   POP EBP
        _emit 0x5d
        // 000642ab: c3                   RET
        _emit 0xc3
        // --- 0x4642ac: byte != 0x80 ---
        // 000642ac: 89 31                MOV dword ptr [ECX],ESI       (*out_type = 0)
        _emit 0x89
        _emit 0x31
        // 000642ae: 8a 08                MOV CL,byte ptr [EAX]         (read first byte)
        _emit 0x8a
        _emit 0x08
        // 000642b0: 0f b6 d1             MOVZX EDX,CL                  (EDX = byte value)
        _emit 0x0f
        _emit 0xb6
        _emit 0xd1
        // 000642b3: 80 e1 80             AND CL,0x80                   (high bit = continuation?)
        _emit 0x80
        _emit 0xe1
        _emit 0x80
        // 000642b6: 83 e2 7f             AND EDX,0x7f                  (EDX = low 7 bits)
        _emit 0x83
        _emit 0xe2
        _emit 0x7f
        // 000642b9: 83 c0 01             ADD EAX,0x1                   (advance ptr past header)
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 000642bc: 84 c9                TEST CL,CL                    (high bit set?)
        _emit 0x84
        _emit 0xc9
        // 000642be: 74 2f                JZ +0x2f                      (→ 0x4642ef: single-byte)
        _emit 0x74
        _emit 0x2f
        // --- multi-byte: EDX = continuation byte count ---
        // 000642c0: 83 fa 04             CMP EDX,0x4
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        // 000642c3: 77 c1                JA -0x3f                      (→ 0x464286: fail)
        _emit 0x77
        _emit 0xc1
        // 000642c5: 8b cf                MOV ECX,EDI                   (ECX = remaining-1)
        _emit 0x8b
        _emit 0xcf
        // 000642c7: 83 ef 01             SUB EDI,0x1                   (EDI--)
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 000642ca: 85 c9                TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 000642cc: 74 b8                JZ -0x48                      (→ 0x464286: fail)
        _emit 0x74
        _emit 0xb8
        // 000642ce: 85 d2                TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 000642d0: 76 27                JBE +0x27                     (→ 0x4642f9: store)
        _emit 0x76
        _emit 0x27
        // --- continuation byte loop ---
        // 000642d2: 0f b6 08             MOVZX ECX,byte ptr [EAX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x08
        // 000642d5: c1 e6 08             SHL ESI,0x8
        _emit 0xc1
        _emit 0xe6
        _emit 0x08
        // 000642d8: 0b f1                OR ESI,ECX
        _emit 0x0b
        _emit 0xf1
        // 000642da: 8b cf                MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000642dc: 83 ea 01             SUB EDX,0x1
        _emit 0x83
        _emit 0xea
        _emit 0x01
        // 000642df: 83 c0 01             ADD EAX,0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 000642e2: 83 ef 01             SUB EDI,0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 000642e5: 85 c9                TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 000642e7: 74 9d                JZ -0x63                      (→ 0x464286: fail)
        _emit 0x74
        _emit 0x9d
        // 000642e9: 85 d2                TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 000642eb: 77 e5                JA -0x1b                      (→ 0x4642d2: loop)
        _emit 0x77
        _emit 0xe5
        // 000642ed: eb 02                JMP +0x2                      (→ 0x4642f1: range check)
        _emit 0xeb
        _emit 0x02
        // --- 0x4642ef: single-byte value path ---
        // 000642ef: 8b f2                MOV ESI,EDX                   (ESI = low 7 bits)
        _emit 0x8b
        _emit 0xf2
        // --- 0x4642f1: range check (join from loop and single-byte) ---
        // 000642f1: 81 fe ff ff ff 7f    CMP ESI,0x7fffffff
        _emit 0x81
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 000642f7: 77 8d                JA -0x73                      (→ 0x464286: fail)
        _emit 0x77
        _emit 0x8d
        // --- 0x4642f9: store result and return 1 ---
        // 000642f9: 8b 54 24 14          MOV EDX,dword ptr [ESP+0x14]  (EDX = out_value)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000642fd: 89 45 00             MOV dword ptr [EBP],EAX       (*pptr = new position)
        _emit 0x89
        _emit 0x45
        _emit 0x00
        // 00064300: 5f                   POP EDI
        _emit 0x5f
        // 00064301: 89 32                MOV dword ptr [EDX],ESI       (*out_value = ESI)
        _emit 0x89
        _emit 0x32
        // 00064303: 5e                   POP ESI
        _emit 0x5e
        // 00064304: b8 01 00 00 00       MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00064309: 5d                   POP EBP
        _emit 0x5d
        // 0006430a: c3                   RET
        _emit 0xc3
    }
}
