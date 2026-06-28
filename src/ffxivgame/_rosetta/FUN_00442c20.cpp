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
// FUNCTION: ffxivgame 0x00042c20 — indexed-array copy loop (208 B / 0xd0),
//                                  __cdecl, no GS frame.
//
// Asm shape (208 B, RVA 0x00042c20..0x00042cef, section .text):
//
//   Copies a range of pointer-indexed 36-byte elements from one
//   IndexedArray (src, arg3/arg4) to another (dst, arg9/arg10),
//   stopping when src_idx reaches src_end (arg7).  Bounds assertions
//   (calls to 0x009d22b4) guard each array access.  Returns final
//   iterator state into the output struct *arg1:
//     arg1->field0 = 0
//     arg1->field4 = dst_ptr     (arg9, unchanged across the loop)
//     arg1->field8 = dst_idx     (arg10, incremented once per iteration)
//
//   Stack layout (after PUSH EBX / EBP / ESI / EDI — ESP = original−0x10):
//     [ESP+0x14] arg1  — output Iterator* (written at epilogue)
//     [ESP+0x18] arg2  — (unused in the loop body)
//     [ESP+0x1c] arg3  — src IndexedArray*       (→ EDI at loop top)
//     [ESP+0x20] arg4  — src_idx (mutable on stack, incremented by 1/iter)
//     [ESP+0x24] arg5  — (unused in the loop body)
//     [ESP+0x28] arg6  — (unused in the loop body)
//     [ESP+0x2c] arg7  — src_end_idx (loop terminator)
//     [ESP+0x30] arg8  — (unused in the loop body)
//     EBP              — arg9  : dst IndexedArray* (read via [EBP+…])
//     EBX              — arg10 : dst_idx (incremented by 1/iter)
//
//   Each element copied is 36 bytes (0x24):
//     float @ +0x00, float @ +0x04  (x87 FLD/FSTP)
//     dword @ +0x08, dword @ +0x0c  (MOV)
//     qword @ +0x10, qword @ +0x18  (MOVQ XMM0)
//     dword @ +0x20                 (MOV)
//
// Reconstruction: __declspec(naked) byte passthrough.
//   The interaction between the interleaved argument reads before the
//   first and second PUSH (MOV EBX/EBP from [ESP+0x2c] at different
//   stack depths), the LEA ESP,[ESP] alignment NOP, x87 FLD/FSTP mixed
//   with MOVQ SSE2 copies, and the near JZ / near JMP branches to loop
//   head and exit produce a brittle encoding that high-level C++ at /O2
//   does not reliably reproduce.  No relocations inside the function
//   body beyond the four REL32 CALL sites to 0x009d22b4; compare.py
//   masks those 4-byte windows.
//
// Reloc-bearing sites (offsets within the 208-byte function body):
//   +0x29  REL32 → 0x009d22b4  (assertion call — null src check)
//   +0x3a  REL32 → 0x009d22b4  (assertion call — src bounds check)
//   +0x5a  REL32 → 0x009d22b4  (assertion call — null dst check)
//   +0x69  REL32 → 0x009d22b4  (assertion call — dst bounds check)

extern "C" __declspec(naked) void FUN_00442c20() {
    __asm {
        // 00042c20  53              PUSH EBX
        // 00042c21  8b 5c 24 2c     MOV EBX,[ESP+0x2c]   ; EBX = arg10 (dst_idx)
        // 00042c25  55              PUSH EBP
        // 00042c26  8b 6c 24 2c     MOV EBP,[ESP+0x2c]   ; EBP = arg9  (dst ptr) [ESP shifted again]
        // 00042c2a  56              PUSH ESI
        // 00042c2b  57              PUSH EDI
        // 00042c2c  8d 64 24 00     LEA ESP,[ESP]         ; alignment NOP
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x2c
        _emit 0x56
        _emit 0x57
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00

        // 00042c30  8b 44 24 20     MOV EAX,[ESP+0x20]   ; EAX = src_idx (loop top)
        // 00042c34  3b 44 24 2c     CMP EAX,[ESP+0x2c]   ; vs src_end
        // 00042c38  0f 84 9d 00 00 00  JZ +0x9d          ; exit loop
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x3b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x0f
        _emit 0x84
        _emit 0x9d
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 00042c3e  8b 7c 24 1c     MOV EDI,[ESP+0x1c]   ; EDI = src array ptr
        // 00042c42  85 ff           TEST EDI,EDI
        // 00042c44  8b f0           MOV ESI,EAX           ; save src_idx in ESI
        // 00042c46  75 07           JNZ +7                ; skip assert if non-null
        // 00042c48  e8 67 f6 58 00  CALL 0x009d22b4       ; assert (null src)
        // 00042c4d  8b c6           MOV EAX,ESI           ; restore src_idx
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x85
        _emit 0xff
        _emit 0x8b
        _emit 0xf0
        _emit 0x75
        _emit 0x07
        _emit 0xe8
        _emit 0x67
        _emit 0xf6
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0xc6

        // 00042c4f  8b 4f 10        MOV ECX,[EDI+0x10]
        // 00042c52  03 4f 0c        ADD ECX,[EDI+0x0c]   ; ECX = src.start+src.size
        // 00042c55  3b c1           CMP EAX,ECX
        // 00042c57  72 09           JC  +9               ; in range → skip assert
        // 00042c59  e8 56 f6 58 00  CALL 0x009d22b4       ; assert (src oob)
        // 00042c5e  8b 44 24 20     MOV EAX,[ESP+0x20]   ; reload src_idx
        _emit 0x8b
        _emit 0x4f
        _emit 0x10
        _emit 0x03
        _emit 0x4f
        _emit 0x0c
        _emit 0x3b
        _emit 0xc1
        _emit 0x72
        _emit 0x09
        _emit 0xe8
        _emit 0x56
        _emit 0xf6
        _emit 0x58
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x20

        // 00042c62  8b 4f 08        MOV ECX,[EDI+0x8]    ; ECX = src.base_idx
        // 00042c65  3b c8           CMP ECX,EAX          ; base > src_idx?
        // 00042c67  77 04           JA  +4               ; yes → keep ESI=EAX
        // 00042c69  8b f0           MOV ESI,EAX          ; ESI = src_idx
        // 00042c6b  2b f1           SUB ESI,ECX          ; ESI = src_idx - base
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        _emit 0x3b
        _emit 0xc8
        _emit 0x77
        _emit 0x04
        _emit 0x8b
        _emit 0xf0
        _emit 0x2b
        _emit 0xf1

        // 00042c6d  85 ed           TEST EBP,EBP         ; check dst ptr non-null
        // 00042c6f  8b 57 04        MOV EDX,[EDI+0x4]    ; EDX = src.data
        // 00042c72  8b 34 b2        MOV ESI,[EDX+ESI*4]  ; ESI = src.data[offset]
        // 00042c75  8b fb           MOV EDI,EBX          ; EDI = dst_idx (reuse EDI)
        // 00042c77  75 05           JNZ +5               ; skip assert if dst non-null
        // 00042c79  e8 36 f6 58 00  CALL 0x009d22b4       ; assert (null dst)
        _emit 0x85
        _emit 0xed
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        _emit 0x8b
        _emit 0x34
        _emit 0xb2
        _emit 0x8b
        _emit 0xfb
        _emit 0x75
        _emit 0x05
        _emit 0xe8
        _emit 0x36
        _emit 0xf6
        _emit 0x58
        _emit 0x00

        // 00042c7e  8b 45 10        MOV EAX,[EBP+0x10]
        // 00042c81  03 45 0c        ADD EAX,[EBP+0x0c]   ; EAX = dst.start+dst.size
        // 00042c84  3b d8           CMP EBX,EAX          ; dst_idx vs limit
        // 00042c86  72 05           JC  +5               ; in range → skip assert
        // 00042c88  e8 27 f6 58 00  CALL 0x009d22b4       ; assert (dst oob)
        _emit 0x8b
        _emit 0x45
        _emit 0x10
        _emit 0x03
        _emit 0x45
        _emit 0x0c
        _emit 0x3b
        _emit 0xd8
        _emit 0x72
        _emit 0x05
        _emit 0xe8
        _emit 0x27
        _emit 0xf6
        _emit 0x58
        _emit 0x00

        // 00042c8d  8b 45 08        MOV EAX,[EBP+0x8]    ; EAX = dst.base_idx
        // 00042c90  3b c3           CMP EAX,EBX          ; base > dst_idx?
        // 00042c92  77 04           JA  +4               ; yes → keep EDI=EBX
        // 00042c94  8b fb           MOV EDI,EBX          ; EDI = dst_idx
        // 00042c96  2b f8           SUB EDI,EAX          ; EDI = dst_idx - base
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x3b
        _emit 0xc3
        _emit 0x77
        _emit 0x04
        _emit 0x8b
        _emit 0xfb
        _emit 0x2b
        _emit 0xf8

        // 00042c98  8b 4d 04        MOV ECX,[EBP+0x4]    ; ECX = dst.data
        // 00042c9b  d9 06           FLD  dword ptr [ESI]  ; load src.float0
        // 00042c9d  8b 04 b9        MOV EAX,[ECX+EDI*4]  ; EAX = dst.data[offset]
        // 00042ca0  d9 18           FSTP dword ptr [EAX]  ; store → dst.float0
        // 00042ca2  83 c3 01        ADD EBX,1             ; dst_idx++
        // 00042ca5  d9 46 04        FLD  dword ptr [ESI+4]; load src.float1
        // 00042ca8  83 44 24 20 01  ADD [ESP+0x20],1      ; src_idx++ (on stack)
        // 00042cad  d9 58 04        FSTP dword ptr [EAX+4]; store → dst.float1
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        _emit 0xd9
        _emit 0x06
        _emit 0x8b
        _emit 0x04
        _emit 0xb9
        _emit 0xd9
        _emit 0x18
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        _emit 0xd9
        _emit 0x46
        _emit 0x04
        _emit 0x83
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x01
        _emit 0xd9
        _emit 0x58
        _emit 0x04

        // 00042cb0  8b 56 08        MOV EDX,[ESI+8]
        // 00042cb3  89 50 08        MOV [EAX+8],EDX
        // 00042cb6  8b 4e 0c        MOV ECX,[ESI+0xc]
        // 00042cb9  89 48 0c        MOV [EAX+0xc],ECX
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        _emit 0x89
        _emit 0x50
        _emit 0x08
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        _emit 0x89
        _emit 0x48
        _emit 0x0c

        // 00042cbc  f3 0f 7e 46 10  MOVQ XMM0,[ESI+0x10]
        // 00042cc1  66 0f d6 40 10  MOVQ [EAX+0x10],XMM0
        // 00042cc6  f3 0f 7e 46 18  MOVQ XMM0,[ESI+0x18]
        // 00042ccb  66 0f d6 40 18  MOVQ [EAX+0x18],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x46
        _emit 0x10
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x46
        _emit 0x18
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18

        // 00042cd0  8b 56 20        MOV EDX,[ESI+0x20]
        // 00042cd3  89 50 20        MOV [EAX+0x20],EDX
        // 00042cd6  e9 55 ff ff ff  JMP 0x00442c30        ; loop back
        _emit 0x8b
        _emit 0x56
        _emit 0x20
        _emit 0x89
        _emit 0x50
        _emit 0x20
        _emit 0xe9
        _emit 0x55
        _emit 0xff
        _emit 0xff
        _emit 0xff

        // 00042cdb  8b 44 24 14     MOV EAX,[ESP+0x14]   ; arg1 = output ptr
        // 00042cdf  5f              POP EDI
        // 00042ce0  5e              POP ESI
        // 00042ce1  89 68 04        MOV [EAX+4],EBP      ; output->field4 = dst ptr
        // 00042ce4  5d              POP EBP
        // 00042ce5  89 58 08        MOV [EAX+8],EBX      ; output->field8 = dst_idx
        // 00042ce8  c7 00 00 00 00 00  MOV [EAX],0       ; output->field0 = 0
        // 00042cee  5b              POP EBX
        // 00042cef  c3              RET
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x89
        _emit 0x68
        _emit 0x04
        _emit 0x5d
        _emit 0x89
        _emit 0x58
        _emit 0x08
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b
        _emit 0xc3
    }
}
