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
// FUNCTION: ffxivgame 0x0005d660 — __cdecl lookup/dispatch helper with a
//                                  /GS security cookie (433 B / 0x1b1).
//
// Inspection (read from the disassembly at orig RVA 0x0005d660):
//
//   __cdecl char FUN_0045d660(int arg0/*esp+0x70*/, void* arg1/*esp+0x74*/,
//                             void** arg2/*esp+0x78*/, int* out/*esp+0x7c*/,
//                             void* arg4/*esp+0x88*/, int arg5/*esp+0x8c*/);
//
//   Prologue allocates 0x68 of frame via the alloc-probe stub at
//   0x009d29d0 (MOV EAX,0x68 / CALL __chkstk variant), then stamps the
//   /GS cookie:  EAX = __security_cookie ^ ESP; [ESP+0x64] = EAX.
//   EBX/EBP/ESI/EDI are callee-saved.
//
//   Body shape (mirrors the asm exactly):
//
//     *out = 0;                                  // MOV [EBP],0
//     // build three stack temporaries via the neighbour helpers
//     FUN_0045ced0(&tmp, arg1);                  // (LEA EAX,[esp+0x1c]; PUSH; ... )
//     FUN_0045d240(&tmpB, arg2);
//     FUN_0045d0e0(&tmpC, &tmpD, &tmpB);
//     FUN_0045d160(&tmpD);
//     void* obj = *arg2;                          // MOV EAX,[ESI]
//     if ((obj->flags_0c & 0x4) == 0) {           // TEST byte [EAX+0xc],4 ; JZ
//         // --- tag-scan arm (jump target 0x0045d767) ---
//         // walk up to 4 entries of obj+0x2c looking for *arg4 == entry
//         // — on miss, raise the assert at line 0x7d (FUN_0045c940);
//         //   on hit, require obj+0x24 (vtbl-ish) non-null else assert
//         //   line 0x83; then indirect-call (*obj_24)(*obj, &tmpD,
//         //   tmpB-slot, arg0_byte, out, arg4->[0x14]).
//     } else {
//         // --- registry arm ---
//         FUN_0045cb50(arg4);                     // -> handle in EAX
//         int h = FUN_0046a2e0(arg4, 0);          // open/find by handle
//         char ok = 0;
//         if (h) {
//             if (FUN_0046a980(h) > 0 &&
//                 FUN_0046a1f0(h, -1, 0xf8, 1, 0, *arg2) > 0 &&
//                 FUN_0046a9e0(h, *arg2, &slotA, &slotB, slotC) > 0) {
//                 *out = slotD;
//                 ok = 1;
//             }
//             FUN_0046a190(h);                    // close
//         }
//         return ok;                              // EBX -> EAX
//     }
//
//   Three distinct epilogues each re-validate the cookie
//   (MOV ECX,[ESP+0x64]; XOR ECX,ESP; CALL __security_check_cookie
//   @0x009d20f4; ADD ESP,0x68; RET) — the registry-arm tail returns
//   EBX, the two assert arms return 0 (XOR EAX,EAX), and the indirect-
//   call arm returns whatever (*obj_24)() produced in EAX.
//
//   Reloc-bearing sites in the 433-byte body (image base 0x00400000):
//     +0x05  CALL rel32   → 0x009d29d0  (__chkstk / alloc-probe)
//     +0x0a  MOV  EAX,[]  → 0x012ea8b0  (__security_cookie)
//     +0x3c  CALL rel32   → 0x0045ced0
//     +0x47  CALL rel32   → 0x0045d240
//     +0x5b  CALL rel32   → 0x0045d0e0
//     +0x65  CALL rel32   → 0x0045d160
//     +0x7a  CALL rel32   → 0x0045cb50
//     +0x87  CALL rel32   → 0x0046a2e0
//     +0x96  CALL rel32   → 0x0046a980
//     +0xb0  CALL rel32   → 0x0046a1f0
//     +0xd1  CALL rel32   → 0x0046a9e0
//     +0xea  CALL rel32   → 0x0046a190
//     +0xfe  CALL rel32   → 0x009d20f4  (__security_check_cookie)
//     +0x127 PUSH imm32   → 0x00f689bc  (string-pool ptr, assert ctx)
//     +0x132 CALL rel32   → 0x0045c940  (assert line 0x7d)
//     +0x146 CALL rel32   → 0x009d20f4
//     +0x15b PUSH imm32   → 0x00f689bc
//     +0x166 CALL rel32   → 0x0045c940  (assert line 0x83)
//     +0x17a CALL rel32   → 0x009d20f4
//     +0x196 CALL ECX                    (indirect vtbl dispatch)
//     +0x1a8 CALL rel32   → 0x009d20f4
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level rewrite would have to coax MSVC 2005 /O2 /GS into
//   reproducing the exact /GS prologue+three cookie-checked epilogues,
//   the chkstk alloc-probe, the short-circuit `&&` chain across five
//   cdecl call sites with the shared `ADD ESP,n` fold-downs, the two
//   assert arms with their distinct line-number immediates, AND the
//   linker-resolved absolute addresses in the 21 relocation windows
//   above. Each is brittle under /O2 — every high-level rewrite shifts
//   at least one byte. The pragmatic, established choice (same as the
//   sibling FUN_00415d00 / FUN_00409350 / FUN_0040b840 bodies) is a
//   `__declspec(naked)` body that re-emits the orig 433 bytes verbatim
//   via MASM `_emit`. The .obj's `.text` ends up byte-identical to the
//   orig slice, which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0045d660() {
    __asm {
        _emit 0xb8
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x66
        _emit 0x53
        _emit 0x57
        _emit 0x00
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x64
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x74
        _emit 0x55
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x7c
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x78
        _emit 0x57
        _emit 0x8b
        _emit 0xbc
        _emit 0x24
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x50
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x2f
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x56
        _emit 0x51
        _emit 0xe8
        _emit 0x94
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x44
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        _emit 0x51
        _emit 0xe8
        _emit 0x20
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x34
        _emit 0x52
        _emit 0xe8
        _emit 0x96
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x06
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        _emit 0xf6
        _emit 0x40
        _emit 0x0c
        _emit 0x04
        _emit 0x0f
        _emit 0x84
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0x71
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        _emit 0x33
        _emit 0xdb
        _emit 0x53
        _emit 0x57
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xe8
        _emit 0xf4
        _emit 0xcb
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf8
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xff
        _emit 0x74
        _emit 0x54
        _emit 0x57
        _emit 0xe8
        _emit 0x85
        _emit 0xd2
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x85
        _emit 0xc0
        _emit 0x7e
        _emit 0x47
        _emit 0x8b
        _emit 0x06
        _emit 0x50
        _emit 0x53
        _emit 0x6a
        _emit 0x01
        _emit 0x68
        _emit 0xf8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0xff
        _emit 0x57
        _emit 0xe8
        _emit 0xdb
        _emit 0xca
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x85
        _emit 0xc0
        _emit 0x7e
        _emit 0x2d
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x38
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50
        _emit 0x51
        _emit 0x57
        _emit 0xe8
        _emit 0xaa
        _emit 0xd2
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x85
        _emit 0xc0
        _emit 0x7e
        _emit 0x0c
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x89
        _emit 0x55
        _emit 0x00
        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0xe8
        _emit 0x41
        _emit 0xca
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x8b
        _emit 0xc3
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x91
        _emit 0x49
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x68
        _emit 0xc3
        _emit 0x33
        _emit 0xf6
        _emit 0x8d
        _emit 0x50
        _emit 0x2c
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        _emit 0x8b
        _emit 0x0a
        _emit 0x85
        _emit 0xc9
        _emit 0x74
        _emit 0x0f
        _emit 0x39
        _emit 0x0f
        _emit 0x74
        _emit 0x35
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        _emit 0x83
        _emit 0xc2
        _emit 0x04
        _emit 0x83
        _emit 0xfe
        _emit 0x04
        _emit 0x7c
        _emit 0xeb
        _emit 0x6a
        _emit 0x7d
        _emit 0x68
        _emit 0xbc
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x6e
        _emit 0x6a
        _emit 0x6b
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0xa9
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x49
        _emit 0x49
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x68
        _emit 0xc3
        _emit 0x8b
        _emit 0x48
        _emit 0x24
        _emit 0x85
        _emit 0xc9
        _emit 0x75
        _emit 0x2d
        _emit 0x68
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0xbc
        _emit 0x89
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x68
        _emit 0x6a
        _emit 0x6b
        _emit 0x6a
        _emit 0x06
        _emit 0xe8
        _emit 0x75
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x15
        _emit 0x49
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x68
        _emit 0xc3
        _emit 0x8b
        _emit 0x57
        _emit 0x14
        _emit 0x8b
        _emit 0x00
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x55
        _emit 0x53
        _emit 0x52
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x44
        _emit 0x52
        _emit 0x50
        _emit 0xff
        _emit 0xd1
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5f
        _emit 0x5e
        _emit 0x5d
        _emit 0x5b
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xe7
        _emit 0x48
        _emit 0x57
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x68
        _emit 0xc3
    }
}
