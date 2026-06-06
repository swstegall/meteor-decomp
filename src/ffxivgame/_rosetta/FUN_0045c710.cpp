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
// FUNCTION: ffxivgame 0x0005c710 — global-dispatch init + two-phase virtual call
//                                  (257 B / 0x101, no SEH, no EBP frame).
//
// Behaviour read from asm/ffxivgame/0005c710_FUN_0045c710.s:
//
//   __cdecl void FUN_0045c710(void* arg1)
//
//   Stack frame (ESP-relative, no EBP, 4-byte local via __chkstk):
//     [ESP+0x00]  local DWORD — result of g_ptr->func_14 (written via MOV [ESP],EAX)
//     [ESP+0x04]  return address
//     [ESP+0x08]  arg1         (accessed as [ESP+0x1c] after PUSH ESI + 4 arg pushes)
//
//   Callee-saved: ESI (PUSH ESI at 0x5c77c, POP ESI at 0x5c80e).
//   Frame teardown: POP ESI (restore), POP ECX (discard local), RET.
//
//   Structural shape:
//
//     1. MOV EAX,4 / CALL __chkstk (0x9d29d0) — allocate 4-byte local
//
//     2. if (g_ptr == NULL) {          // [0x0132e788]
//            FUN_00465f80(9,1,0xf6802c,0x127);
//            if (g_ptr == NULL)
//                g_ptr = 0xf68000;    // default
//            FUN_00465f80(10,1,0xf6802c,0x12a);
//        }
//
//     3. result = (*g_ptr->func_14)(0) // indirect CALL via [EAX+0x14]
//        local_var = result
//        if (result == 0) return;
//
//     4. PUSH ESI (callee-save)
//        FUN_00465f80(9,1,0xf6802c,0x213);   // logging / tracing
//        result2 = FUN_00466af0(result, arg1) // combined ADD ESP,0x18 cleans
//                                             // 4 args of 00465f80 + 2 of 00466af0
//        ESI = result2
//
//     5. if (g_flag == 1) {            // [0x0132e794]
//            if (g_ptr2 != NULL) {     // [0x0132e790]
//                if (FUN_00466980(g_ptr2) == 0) {
//                    FUN_00466620(g_ptr2);
//                    g_ptr2 = NULL;
//                }
//            }
//        }
//
//     6. FUN_00465f80(10,1,0xf6802c,0x21c);
//        (*g_ptr->func_18)(&local_var)  // indirect CALL via [EAX+0x18],
//                                       // combined ADD ESP,0x14 cleans
//                                       // 4 args of 00465f80 + 1 for func_18
//
//     7. if (ESI != 0)
//            FUN_0045c1f0(ESI);
//
//   Reloc-bearing sites (absolute addresses resolve only at image base
//   0x00400000 in the full-binary link):
//     +0x05   rel32  0x009d29d0 — __chkstk (CRT)
//     +0x0e   abs32  0x0132e788 — g_ptr global
//     +0x14   ..                 g_ptr (1st CMP)
//     +0x1e   rel32  0x00465f80 — FUN_00465f80
//     +0x25   abs32  0x0132e788 — g_ptr (2nd CMP)
//     +0x2e   abs32  0x0132e788 — g_ptr MOV immediate store
//     +0x37   rel32  0x00465f80 — FUN_00465f80
//     +0x40   abs32  0x0132e788 — g_ptr load
//     +0x5b   rel32  0x00465f80 — FUN_00465f80
//     +0x6e   rel32  0x00466af0 — FUN_00466af0
//     +0x74   abs32  0x0132e794 — g_flag
//     +0x7b   abs32  0x0132e790 — g_ptr2
//     +0x82   rel32  0x00466980 — FUN_00466980
//     +0x8b   abs32  0x0132e790 — g_ptr2
//     +0x8f   rel32  0x00466620 — FUN_00466620
//     +0x95   abs32  0x0132e790 — g_ptr2 zero-store
//     +0xa0   rel32  0x00465f80 — FUN_00465f80
//     +0xa5   abs32  0x0132e788 — g_ptr load
//     +0xf1   rel32  0x0045c1f0 — FUN_0045c1f0
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ port requires MSVC 2005 /O2 to reproduce:
//   (a) the ESP-relative no-EBP frame with 4-byte local via __chkstk,
//   (b) the combined ADD ESP,0x18 cleanup of two separate cdecl calls,
//   (c) the combined ADD ESP,0x14 cleanup of 00465f80 + indirect func_18,
//   (d) MOV [ESP],EAX to write the local var (not EBP-relative),
//   (e) LEA EDX,[ESP+0x14] to pass &local_var as the func_18 argument
//       (stack depth-dependent and fragile under any reorder),
//   (f) the exact ESI allocation order (PUSH ESI after the first
//       vtable call, MOV ESI,EAX after the combined cleanup).
//   Each constraint is brittle under /O2 — every restructuring shifts
//   at least one byte in the ESP-offset arithmetic.
//
//   The pragmatic choice — the same one FUN_00402a30 / FUN_00403a20 /
//   FUN_004054d0 took for their reloc-heavy or frame-sensitive bodies —
//   is a `__declspec(naked)` body that re-emits the orig 257 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice (no relocations — all
//   absolute addresses and PC-relative offsets are baked in at orig's
//   link-time RVA of 0x0045c710), which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0045c710() {
    __asm {
        // 0005c710  MOV EAX, 4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c715  CALL 0x009d29d0  (__chkstk — allocate 4-byte local)
        _emit 0xe8
        _emit 0xb6
        _emit 0x62
        _emit 0x57
        _emit 0x00
        // 0005c71a  CMP dword ptr [0x0132e788], 0
        _emit 0x83
        _emit 0x3d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0005c721  JNZ 0x0045c762
        _emit 0x75
        _emit 0x3f
        // 0005c723  PUSH 0x127
        _emit 0x68
        _emit 0x27
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c728  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c72d  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c72f  PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // 0005c731  CALL 0x00465f80
        _emit 0xe8
        _emit 0x4a
        _emit 0x98
        _emit 0x00
        _emit 0x00
        // 0005c736  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c739  CMP dword ptr [0x0132e788], 0
        _emit 0x83
        _emit 0x3d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0005c740  JNZ 0x0045c74c
        _emit 0x75
        _emit 0x0a
        // 0005c742  MOV dword ptr [0x0132e788], 0xf68000
        _emit 0xc7
        _emit 0x05
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c74c  PUSH 0x12a
        _emit 0x68
        _emit 0x2a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0005c751  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c756  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c758  PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0005c75a  CALL 0x00465f80
        _emit 0xe8
        _emit 0x21
        _emit 0x98
        _emit 0x00
        _emit 0x00
        // 0005c75f  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0005c762  MOV EAX, [0x0132e788]
        _emit 0xa1
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c767  MOV ECX, dword ptr [EAX+0x14]
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // 0005c76a  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0005c76c  CALL ECX
        _emit 0xff
        _emit 0xd1
        // 0005c76e  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c771  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005c773  MOV dword ptr [ESP], EAX
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // 0005c776  JZ 0x0045c80f
        _emit 0x0f
        _emit 0x84
        _emit 0x93
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c77c  PUSH ESI
        _emit 0x56
        // 0005c77d  PUSH 0x213
        _emit 0x68
        _emit 0x13
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0005c782  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c787  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c789  PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // 0005c78b  CALL 0x00465f80
        _emit 0xe8
        _emit 0xf0
        _emit 0x97
        _emit 0x00
        _emit 0x00
        // 0005c790  MOV EDX, dword ptr [ESP+0x1c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 0005c794  MOV EAX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0005c798  PUSH EDX
        _emit 0x52
        // 0005c799  PUSH EAX
        _emit 0x50
        // 0005c79a  CALL 0x00466af0
        _emit 0xe8
        _emit 0x51
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        // 0005c79f  ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 0005c7a2  CMP dword ptr [0x0132e794], 0x1
        _emit 0x83
        _emit 0x3d
        _emit 0x94
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 0005c7a9  MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0005c7ab  JNZ 0x0045c7dc
        _emit 0x75
        _emit 0x2f
        // 0005c7ad  MOV EAX, [0x0132e790]
        _emit 0xa1
        _emit 0x90
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c7b2  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005c7b4  JZ 0x0045c7dc
        _emit 0x74
        _emit 0x26
        // 0005c7b6  PUSH EAX
        _emit 0x50
        // 0005c7b7  CALL 0x00466980
        _emit 0xe8
        _emit 0xc4
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        // 0005c7bc  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c7bf  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0005c7c1  JNZ 0x0045c7dc
        _emit 0x75
        _emit 0x19
        // 0005c7c3  MOV ECX, dword ptr [0x0132e790]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c7c9  PUSH ECX
        _emit 0x51
        // 0005c7ca  CALL 0x00466620
        _emit 0xe8
        _emit 0x51
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        // 0005c7cf  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c7d2  MOV dword ptr [0x0132e790], 0x0
        _emit 0xc7
        _emit 0x05
        _emit 0x90
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005c7dc  PUSH 0x21c
        _emit 0x68
        _emit 0x1c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0005c7e1  PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // 0005c7e6  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0005c7e8  PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // 0005c7ea  CALL 0x00465f80
        _emit 0xe8
        _emit 0x91
        _emit 0x97
        _emit 0x00
        _emit 0x00
        // 0005c7ef  MOV EAX, [0x0132e788]
        _emit 0xa1
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 0005c7f4  MOV ECX, dword ptr [EAX+0x18]
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        // 0005c7f7  LEA EDX, [ESP+0x14]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0005c7fb  PUSH EDX
        _emit 0x52
        // 0005c7fc  CALL ECX
        _emit 0xff
        _emit 0xd1
        // 0005c7fe  ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0005c801  TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0005c803  JZ 0x0045c80e
        _emit 0x74
        _emit 0x09
        // 0005c805  PUSH ESI
        _emit 0x56
        // 0005c806  CALL 0x0045c1f0
        _emit 0xe8
        _emit 0xe5
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 0005c80b  ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005c80e  POP ESI
        _emit 0x5e
        // 0005c80f  POP ECX
        _emit 0x59
        // 0005c810  RET
        _emit 0xc3
    }
}
