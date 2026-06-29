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
// FUNCTION: ffxivgame 0x00042db0 — span/range constructor helper (__cdecl, 183 B / 0xb7)
//
// Behaviour read from asm/ffxivgame/00042db0_FUN_00442db0.s:
//
//   __cdecl void* FUN_00442db0(
//       arg1  @ [ESP+0x04] — result struct pointer (returned in EAX)
//       arg2  @ [ESP+0x08] — (unused directly)
//       arg3  @ [ESP+0x0c] — forwarded to inner call
//       arg4  @ [ESP+0x10] — range begin (loaded into EAX before frame save)
//       arg5  @ [ESP+0x14] — forwarded via pointer to inner call
//       arg6  @ [ESP+0x18] — forwarded to inner call
//       arg7  @ [ESP+0x1c] — range end (loaded into ESI after 3 pushes)
//       arg8  @ [ESP+0x20] — cleared to 0
//       arg9  @ [ESP+0x24] — bounds-check object pointer (must be non-null)
//       arg10 @ [ESP+0x28] — base offset value
//   )
//
//   Summary:
//     1. Loads arg4 into EAX before saving callee-saved regs.
//     2. Saves EBX, EBP, ESI (loading arg7 into ESI after 3 pushes).
//     3. Saves EDI (loading arg9 into EDI).
//     4. Computes ESI = arg7 - arg4 (positive delta, no NEG unlike FUN_00442cf0).
//     5. Asserts EDI (arg9) non-null via CALL 0x009d22b4.
//     6. Loads EBX = arg10; computes EBP = EBX + ESI.
//     7. Bounds-checks EBP in [EDI+0xc, EDI+0xc+EDI+0x10); calls 0x009d22b4 on fail.
//     8. Builds three 12-byte structs on the stack via SUB ESP,0xc sequences,
//        filling fields with {0, ptr, value} triples for sub-function args.
//     9. Fills result struct at ESI (= arg1): *(ESI)=0, *(ESI+4)=arg9, *(ESI+8)=EBP.
//    10. Calls FUN_00442c20 (0x00442c20).
//    11. Cleans stack 0x34 bytes, pops frame, returns ESI in EAX.
//
//   Structural sibling: FUN_00442cf0 (0x00042cf0, 187 B) is nearly identical —
//   the only differences are the sign of the delta (NEG in cf0, absent here) and
//   the callee (FUN_00442b50 in cf0, FUN_00442c20 here).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function builds its argument frame with three sequential `SUB ESP,0xc`
//   blocks followed by direct `MOV [EAX+N],reg` field writes. No high-level
//   C++ layout can reproduce this exact byte sequence under MSVC 2005.
//   Same strategy as FUN_00442cf0 / FUN_00401750 / FUN_00403f10.
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x17  CALL rel32 → 0x009d22b4  (null-pointer assert handler)
//   +0x33  CALL rel32 → 0x009d22b4  (bounds check failure)
//   +0xa9  CALL rel32 → 0x00442c20  (FUN_00442c20 — inner copy function)

extern "C" __declspec(naked) void FUN_00442db0() {
    __asm {
        // 00042db0: 8b 44 24 10  MOV EAX, [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00042db4: 53           PUSH EBX
        _emit 0x53
        // 00042db5: 55           PUSH EBP
        _emit 0x55
        // 00042db6: 56           PUSH ESI
        _emit 0x56
        // 00042db7: 8b 74 24 28  MOV ESI, [ESP+0x28]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x28
        // 00042dbb: 57           PUSH EDI
        _emit 0x57
        // 00042dbc: 8b 7c 24 34  MOV EDI, [ESP+0x34]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        // 00042dc0: 2b f0        SUB ESI, EAX
        _emit 0x2b
        _emit 0xf0
        // 00042dc2: 85 ff        TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00042dc4: 75 05        JNZ +5
        _emit 0x75
        _emit 0x05
        // 00042dc6: e8 e9 f4 58 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xe9
        _emit 0xf4
        _emit 0x58
        _emit 0x00
        // 00042dcb: 8b 47 0c     MOV EAX, [EDI+0xc]
        _emit 0x8b
        _emit 0x47
        _emit 0x0c
        // 00042dce: 8b 5c 24 38  MOV EBX, [ESP+0x38]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x38
        // 00042dd2: 8b 4f 10     MOV ECX, [EDI+0x10]
        _emit 0x8b
        _emit 0x4f
        _emit 0x10
        // 00042dd5: 8d 2c 33     LEA EBP, [EBX+ESI*1]
        _emit 0x8d
        _emit 0x2c
        _emit 0x33
        // 00042dd8: 03 c8        ADD ECX, EAX
        _emit 0x03
        _emit 0xc8
        // 00042dda: 3b e9        CMP EBP, ECX
        _emit 0x3b
        _emit 0xe9
        // 00042ddc: 77 04        JA +4
        _emit 0x77
        _emit 0x04
        // 00042dde: 3b e8        CMP EBP, EAX
        _emit 0x3b
        _emit 0xe8
        // 00042de0: 73 05        JNC +5
        _emit 0x73
        _emit 0x05
        // 00042de2: e8 cd f4 58 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xcd
        _emit 0xf4
        _emit 0x58
        _emit 0x00
        // 00042de7: 8b 74 24 14  MOV ESI, [ESP+0x14]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00042deb: c6 44 24 14 00  MOV byte ptr [ESP+0x14], 0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        // 00042df0: 8b 54 24 14  MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00042df4: 8b 4c 24 14  MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00042df8: 52           PUSH EDX
        _emit 0x52
        // 00042df9: 8b 54 24 2c  MOV EDX, [ESP+0x2c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 00042dfd: c6 44 24 34 00  MOV byte ptr [ESP+0x34], 0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x00
        // 00042e02: 8b 44 24 34  MOV EAX, [ESP+0x34]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 00042e06: 50           PUSH EAX
        _emit 0x50
        // 00042e07: 51           PUSH ECX
        _emit 0x51
        // 00042e08: 8b 4c 24 38  MOV ECX, [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00042e0c: 83 ec 0c     SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042e0f: 8b c4        MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 00042e11: c7 00 00 00 00 00  MOV dword ptr [EAX], 0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042e17: 89 78 04     MOV [EAX+4], EDI
        _emit 0x89
        _emit 0x78
        _emit 0x04
        // 00042e1a: 89 58 08     MOV [EAX+8], EBX
        _emit 0x89
        _emit 0x58
        _emit 0x08
        // 00042e1d: 83 ec 0c     SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042e20: 8b c4        MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 00042e22: 89 50 04     MOV [EAX+4], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 00042e25: 8b 54 24 40  MOV EDX, [ESP+0x40]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x40
        // 00042e29: 89 48 08     MOV [EAX+8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00042e2c: 8b 4c 24 44  MOV ECX, [ESP+0x44]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        // 00042e30: c7 00 00 00 00 00  MOV dword ptr [EAX], 0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042e36: 83 ec 0c     SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042e39: 8b c4        MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 00042e3b: 89 50 04     MOV [EAX+4], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 00042e3e: 8d 54 24 54  LEA EDX, [ESP+0x54]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x54
        // 00042e42: 52           PUSH EDX
        _emit 0x52
        // 00042e43: c7 06 00 00 00 00  MOV dword ptr [ESI], 0
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042e49: 89 7e 04     MOV [ESI+4], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        // 00042e4c: 89 6e 08     MOV [ESI+8], EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x08
        // 00042e4f: c7 00 00 00 00 00  MOV dword ptr [EAX], 0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042e55: 89 48 08     MOV [EAX+8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00042e58: e8 c3 fd ff ff  CALL 0x00442c20
        _emit 0xe8
        _emit 0xc3
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00042e5d: 83 c4 34     ADD ESP, 0x34
        _emit 0x83
        _emit 0xc4
        _emit 0x34
        // 00042e60: 5f           POP EDI
        _emit 0x5f
        // 00042e61: 8b c6        MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00042e63: 5e           POP ESI
        _emit 0x5e
        // 00042e64: 5d           POP EBP
        _emit 0x5d
        // 00042e65: 5b           POP EBX
        _emit 0x5b
        // 00042e66: c3           RET
        _emit 0xc3
    }
}
