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
// FUNCTION: ffxivgame 0x0005c0f0 — build_SYS_str_reasons
//                                  (248 B / 0xf8, __cdecl, no SEH).
//
// Behaviour read from asm/ffxivgame/0005c0f0_build_SYS_str_reasons.s:
//
//   __cdecl void build_SYS_str_reasons(void);
//
//   1. Calls a "register string reason" helper (0x00465f80) with:
//        push 0x244, push 0xf6802c, push 1, push 5
//   2. Checks global flag at [0x01268534]. If zero:
//        calls helper(6, 1, 0xf6802c, 0x247) and returns early.
//   3. If non-zero:
//        calls helper(6, 1, 0xf6802c, 0x24b)
//        calls helper(9, 1, 0xf6802c, 0x24c)
//   4. Checks global flag again. If zero:
//        calls helper(0xa, 1, 0xf6802c, 0x24f) and returns early.
//   5. If still non-zero: enters a loop over a table of (value, ptr) pairs:
//        ESI = 0x132e0fc (ptr table), EDI = 0x132d118 (string buf base)
//        EBP = 1 (loop counter / index)
//        Loop:
//          [ESI-4] = EBP
//          if [ESI] == 0:
//            eax = malloc(EBP)       ; 0x009d8494
//            if eax != 0:
//              memcpy(EDI, eax, 0x20); 0x009d5540
//              [EDI+0x1f] = 0
//              [ESI] = EDI
//            if [ESI] == 0:
//              [ESI] = 0xf6806c      ; fallback string
//          ESI += 8, EBP += 1, EDI += 0x20
//          loop while ESI <= 0x132e4ec
//   6. Calls helper(0xa, 1, 0xf6802c, 0x26c), clears flag ([0x01268534]=0),
//      returns.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body contains absolute 32-bit addresses used as immediate
//   operands (CMP [0x01268534], MOV EDI/ESI/ESI from globals, CMP ESI vs
//   0x132e4ec, MOV [ESI]/[0xf6806c] etc.) that standalone .obj compilation
//   cannot reproduce — they are link-time constants baked into the orig
//   image at base 0x00400000. The `__declspec(naked)` byte passthrough is
//   the standard approach used by every reloc-bearing function in this
//   _rosetta collection.

extern "C" __declspec(naked) void FUN_0045c0f0() {
    __asm {
        // 0005c0f0  PUSH 0x244
        _emit 0x68
        _emit 0x44
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0005c0f5  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c0fa  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c0fc  PUSH 0x5
        _emit 0x6a
        _emit 0x05
        // 0005c0fe  CALL 0x00465f80
        _emit 0xe8
        _emit 0x7d
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        // 0005c103  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c106  CMP dword ptr [0x01268534],0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x34
        _emit 0x85
        _emit 0x26
        _emit 0x01
        _emit 0x00
        // 0005c10d  JNZ 0x0045c126
        _emit 0x75
        _emit 0x17
        // 0005c10f  PUSH 0x247
        _emit 0x68
        _emit 0x47
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0005c114  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c119  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c11b  PUSH 0x6
        _emit 0x6a
        _emit 0x06
        // 0005c11d  CALL 0x00465f80
        _emit 0xe8
        _emit 0x5e
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        // 0005c122  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c125  RET
        _emit 0xc3
        // 0005c126  PUSH 0x24b
        _emit 0x68
        _emit 0x4b
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0005c12b  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c130  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c132  PUSH 0x6
        _emit 0x6a
        _emit 0x06
        // 0005c134  CALL 0x00465f80
        _emit 0xe8
        _emit 0x47
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        // 0005c139  PUSH 0x24c
        _emit 0x68
        _emit 0x4c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0005c13e  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c143  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c145  PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // 0005c147  CALL 0x00465f80
        _emit 0xe8
        _emit 0x34
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        // 0005c14c  ADD ESP,0x20
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 0005c14f  CMP dword ptr [0x01268534],0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x34
        _emit 0x85
        _emit 0x26
        _emit 0x01
        _emit 0x00
        // 0005c156  JNZ 0x0045c16f
        _emit 0x75
        _emit 0x17
        // 0005c158  PUSH 0x24f
        _emit 0x68
        _emit 0x4f
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0005c15d  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c162  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c164  PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0005c166  CALL 0x00465f80
        _emit 0xe8
        _emit 0x15
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        // 0005c16b  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c16e  RET
        _emit 0xc3
        // 0005c16f  PUSH EBP
        _emit 0x55
        // 0005c170  PUSH ESI
        _emit 0x56
        // 0005c171  PUSH EDI
        _emit 0x57
        // 0005c172  MOV EBP,0x1
        _emit 0xbd
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c177  MOV EDI,0x132d118
        _emit 0xbf
        _emit 0x18
        _emit 0xd1
        _emit 0x32
        _emit 0x01
        // 0005c17c  MOV ESI,0x132e0fc
        _emit 0xbe
        _emit 0xfc
        _emit 0xe0
        _emit 0x32
        _emit 0x01
        // 0005c181  CMP dword ptr [ESI],0x0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 0005c184  MOV dword ptr [ESI-0x4],EBP
        _emit 0x89
        _emit 0x6e
        _emit 0xfc
        // 0005c187  JNZ 0x0045c1b3
        _emit 0x75
        _emit 0x2a
        // 0005c189  PUSH EBP
        _emit 0x55
        // 0005c18a  CALL 0x009d8494
        _emit 0xe8
        _emit 0x05
        _emit 0xc3
        _emit 0x57
        _emit 0x00
        // 0005c18f  ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c192  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0005c194  JZ 0x0045c1a8
        _emit 0x74
        _emit 0x12
        // 0005c196  PUSH 0x20
        _emit 0x6a
        _emit 0x20
        // 0005c198  PUSH EAX
        _emit 0x50
        // 0005c199  PUSH EDI
        _emit 0x57
        // 0005c19a  CALL 0x009d5540
        _emit 0xe8
        _emit 0xa1
        _emit 0x93
        _emit 0x57
        _emit 0x00
        // 0005c19f  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005c1a2  MOV byte ptr [EDI+0x1f],0x0
        _emit 0xc6
        _emit 0x47
        _emit 0x1f
        _emit 0x00
        // 0005c1a6  MOV dword ptr [ESI],EDI
        _emit 0x89
        _emit 0x3e
        // 0005c1a8  CMP dword ptr [ESI],0x0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 0005c1ab  JNZ 0x0045c1b3
        _emit 0x75
        _emit 0x06
        // 0005c1ad  MOV dword ptr [ESI],0xf6806c
        _emit 0xc7
        _emit 0x06
        _emit 0x6c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c1b3  ADD ESI,0x8
        _emit 0x83
        _emit 0xc6
        _emit 0x08
        // 0005c1b6  ADD EBP,0x1
        _emit 0x83
        _emit 0xc5
        _emit 0x01
        // 0005c1b9  ADD EDI,0x20
        _emit 0x83
        _emit 0xc7
        _emit 0x20
        // 0005c1bc  CMP ESI,0x132e4ec
        _emit 0x81
        _emit 0xfe
        _emit 0xec
        _emit 0xe4
        _emit 0x32
        _emit 0x01
        // 0005c1c2  JLE 0x0045c181
        _emit 0x7e
        _emit 0xbd
        // 0005c1c4  PUSH 0x26c
        _emit 0x68
        _emit 0x6c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0005c1c9  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c1ce  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c1d0  PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0005c1d2  MOV dword ptr [0x01268534],0x0
        _emit 0xc7
        _emit 0x05
        _emit 0x34
        _emit 0x85
        _emit 0x26
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c1dc  CALL 0x00465f80
        _emit 0xe8
        _emit 0x9f
        _emit 0x9d
        _emit 0x00
        _emit 0x00
        // 0005c1e1  ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c1e4  POP EDI
        _emit 0x5f
        // 0005c1e5  POP ESI
        _emit 0x5e
        // 0005c1e6  POP EBP
        _emit 0x5d
        // 0005c1e7  RET
        _emit 0xc3
    }
}
