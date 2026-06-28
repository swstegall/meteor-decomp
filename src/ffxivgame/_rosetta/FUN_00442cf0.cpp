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
// FUNCTION: ffxivgame 0x00042cf0 — span/range constructor helper (__cdecl, 187 B / 0xbb)
//
// Behaviour read from asm/ffxivgame/00042cf0_FUN_00442cf0.s:
//
//   __cdecl void* FUN_00442cf0(
//       arg1  @ [ESP+0x04] — result struct pointer (returned in EAX)
//       arg2  @ [ESP+0x08] — (unused directly)
//       arg3  @ [ESP+0x0c] — forwarded to inner call
//       arg4  @ [ESP+0x10] — range begin (loaded into ECX before frame save)
//       arg5  @ [ESP+0x14] — forwarded via pointer to inner call
//       arg6  @ [ESP+0x18] — forwarded to inner call
//       arg7  @ [ESP+0x1c] — range end (loaded into EAX before frame save)
//       arg8  @ [ESP+0x20] — cleared to 0
//       arg9  @ [ESP+0x24] — bounds-check object pointer (must be non-null)
//       arg10 @ [ESP+0x28] — base offset value
//   )
//
//   Summary:
//     1. Loads arg4 (ECX) and arg7 (EAX) before saving callee-saved regs.
//     2. Computes ESI = NEG(arg7 - arg4) = arg4 - arg7 (signed delta).
//     3. Saves EBX, EBP, ESI, EDI.
//     4. Loads EDI = arg9; asserts non-null via CALL 0x009d22b4.
//     5. Loads EBX = arg10; computes EBP = EBX + ESI.
//     6. Bounds-checks EBP in [EDI+0xc, EDI+0xc+EDI+0x10); calls 0x009d22b4 on fail.
//     7. Builds three 12-byte structs on the stack via SUB ESP,0xc sequences,
//        filling fields with {0, ptr, value} triples for sub-function args.
//     8. Fills result struct at ESI: *(ESI)=0, *(ESI+4)=arg9, *(ESI+8)=EBP.
//     9. Calls FUN_00442b50 (0x00442b50).
//    10. Cleans stack 0x34 bytes, pops frame, returns ESI in EAX.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function builds its argument frame with three sequential `SUB ESP,0xc`
//   blocks followed by direct `MOV [EAX+N],reg` field writes. No high-level
//   C++ layout can reproduce this exact byte sequence under MSVC 2005.
//   Same strategy as FUN_00401750 / FUN_00403f10 / FUN_00406680.
//
// Reloc-bearing sites (wildcarded by compare.py):
//   +0x1a  CALL rel32 → 0x009d22b4  (null-pointer assert handler)
//   +0x36  CALL rel32 → 0x009d22b4  (bounds check failure)
//   +0xac  CALL rel32 → 0x00442b50  (FUN_00442b50 — inner constructor)

extern "C" __declspec(naked) void FUN_00442cf0() {
    __asm {
        // 00042cf0: 8b 4c 24 10  MOV ECX, [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00042cf4: 8b 44 24 1c  MOV EAX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00042cf8: 53           PUSH EBX
        _emit 0x53
        // 00042cf9: 55           PUSH EBP
        _emit 0x55
        // 00042cfa: 56           PUSH ESI
        _emit 0x56
        // 00042cfb: 2b c1        SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 00042cfd: 57           PUSH EDI
        _emit 0x57
        // 00042cfe: 8b 7c 24 34  MOV EDI, [ESP+0x34]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        // 00042d02: f7 d8        NEG EAX
        _emit 0xf7
        _emit 0xd8
        // 00042d04: 85 ff        TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00042d06: 8b f0        MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00042d08: 75 05        JNZ +5
        _emit 0x75
        _emit 0x05
        // 00042d0a: e8 a5 f5 58 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0xa5
        _emit 0xf5
        _emit 0x58
        _emit 0x00
        // 00042d0f: 8b 47 0c     MOV EAX, [EDI+0xc]
        _emit 0x8b
        _emit 0x47
        _emit 0x0c
        // 00042d12: 8b 5c 24 38  MOV EBX, [ESP+0x38]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x38
        // 00042d16: 8b 4f 10     MOV ECX, [EDI+0x10]
        _emit 0x8b
        _emit 0x4f
        _emit 0x10
        // 00042d19: 8d 2c 33     LEA EBP, [EBX+ESI*1]
        _emit 0x8d
        _emit 0x2c
        _emit 0x33
        // 00042d1c: 03 c8        ADD ECX, EAX
        _emit 0x03
        _emit 0xc8
        // 00042d1e: 3b e9        CMP EBP, ECX
        _emit 0x3b
        _emit 0xe9
        // 00042d20: 77 04        JA +4
        _emit 0x77
        _emit 0x04
        // 00042d22: 3b e8        CMP EBP, EAX
        _emit 0x3b
        _emit 0xe8
        // 00042d24: 73 05        JNC +5
        _emit 0x73
        _emit 0x05
        // 00042d26: e8 89 f5 58 00  CALL 0x009d22b4
        _emit 0xe8
        _emit 0x89
        _emit 0xf5
        _emit 0x58
        _emit 0x00
        // 00042d2b: 8b 74 24 14  MOV ESI, [ESP+0x14]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00042d2f: c6 44 24 14 00  MOV byte ptr [ESP+0x14], 0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        // 00042d34: 8b 54 24 14  MOV EDX, [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00042d38: 8b 4c 24 14  MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00042d3c: 52           PUSH EDX
        _emit 0x52
        // 00042d3d: 8b 54 24 2c  MOV EDX, [ESP+0x2c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x2c
        // 00042d41: c6 44 24 34 00  MOV byte ptr [ESP+0x34], 0
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x34
        _emit 0x00
        // 00042d46: 8b 44 24 34  MOV EAX, [ESP+0x34]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 00042d4a: 50           PUSH EAX
        _emit 0x50
        // 00042d4b: 51           PUSH ECX
        _emit 0x51
        // 00042d4c: 8b 4c 24 38  MOV ECX, [ESP+0x38]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        // 00042d50: 83 ec 0c     SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042d53: 8b c4        MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 00042d55: c7 00 00 00 00 00  MOV dword ptr [EAX], 0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042d5b: 89 78 04     MOV [EAX+4], EDI
        _emit 0x89
        _emit 0x78
        _emit 0x04
        // 00042d5e: 89 58 08     MOV [EAX+8], EBX
        _emit 0x89
        _emit 0x58
        _emit 0x08
        // 00042d61: 83 ec 0c     SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042d64: 8b c4        MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 00042d66: 89 50 04     MOV [EAX+4], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 00042d69: 8b 54 24 40  MOV EDX, [ESP+0x40]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x40
        // 00042d6d: 89 48 08     MOV [EAX+8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00042d70: 8b 4c 24 44  MOV ECX, [ESP+0x44]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        // 00042d74: c7 00 00 00 00 00  MOV dword ptr [EAX], 0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042d7a: 83 ec 0c     SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042d7d: 8b c4        MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 00042d7f: 89 50 04     MOV [EAX+4], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 00042d82: 8d 54 24 54  LEA EDX, [ESP+0x54]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x54
        // 00042d86: 52           PUSH EDX
        _emit 0x52
        // 00042d87: c7 06 00 00 00 00  MOV dword ptr [ESI], 0
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042d8d: 89 7e 04     MOV [ESI+4], EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        // 00042d90: 89 6e 08     MOV [ESI+8], EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x08
        // 00042d93: c7 00 00 00 00 00  MOV dword ptr [EAX], 0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042d99: 89 48 08     MOV [EAX+8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00042d9c: e8 af fd ff ff  CALL 0x00442b50
        _emit 0xe8
        _emit 0xaf
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00042da1: 83 c4 34     ADD ESP, 0x34
        _emit 0x83
        _emit 0xc4
        _emit 0x34
        // 00042da4: 5f           POP EDI
        _emit 0x5f
        // 00042da5: 8b c6        MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00042da7: 5e           POP ESI
        _emit 0x5e
        // 00042da8: 5d           POP EBP
        _emit 0x5d
        // 00042da9: 5b           POP EBX
        _emit 0x5b
        // 00042daa: c3           RET
        _emit 0xc3
    }
}
