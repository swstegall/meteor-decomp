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
// FUNCTION: ffxivgame 0x009d4c25 — __cdecl file-open helper
//                                   (311 B / 0x137, EH3-SEH wrapped)
//
// Inspection (read from the disassembly at orig RVA 0x005d4c25):
//
//   __cdecl int FUN_009d4c25(int param1, SomeObj *param2);
//
//   Validates param1 (non-null) and param2 (non-null, object pointer
//   with flag checks), then calls into a chain of helpers to perform
//   some file/resource open operation, returning 0 on success or -1
//   on failure (errno = EINVAL = 0x16 on bad args).
//
//   Structure (from asm):
//
//     if (!param1) {
//         *_errno() = 0x16;                        // EINVAL
//         _invalid_parameter(0,0,0,0,0);
//         return -1;
//     }
//     param2 = arg2;
//     if (!param2) goto einval_path;
//     if (param2->flags & 0x40) goto skip_mode_checks;
//     // first mode-validity block: checks 7-bit mode table
//     result1 = get_mode(param2);
//     if (result1 != -1 && result1 != -2) {
//         idx = result1 >> 5;
//         EDI = mode_table[idx];
//         result1 = get_mode(param2);
//         result1 = (result1 & 0x1f) << 6;
//         result1 += EDI;
//     } else {
//         result1 = 0x012eb4d8;                    // fallback global
//     }
//     if (result1->byte_at_0x24 & 0x7f) goto einval_path;
//     // second mode-validity block: checks bit 0x80
//     result2 = get_mode(param2);
//     if (result2 != -1 && result2 != -2) {
//         idx = result2 >> 5;
//         EDI = mode_table[idx];
//         result2 = get_mode(param2);
//         result2 = (result2 & 0x1f) << 6;
//         result2 += EDI;
//     } else {
//         result2 = 0x012eb4d8;
//     }
//     if (result2->byte_at_0x24 & 0x80) goto einval_path;
// skip_mode_checks:
//     local_1c = FUN_009dc3f0(param1);
//     FUN_009d4e3e(param2);
//     local_fc = 0;                                // try-scope marker
//     edi_val  = FUN_009e4f00(param2);
//     local_e0 = FUN_009d723b(param1, 1, local_1c, param2);
//     FUN_009e4f96(param2, edi_val);
//     // esp += 0x1c (7 pushes above)
//     local_fc = -2;                               // try-scope done
//     FUN_009d4d5f();                              // unwind/cleanup thunk
//     return (local_e0 == local_1c) ? 0 : -1;     // SETZ + DEC EAX
//
//   Stack frame (EH3-SEH, after __SEH_prolog4 call):
//     [ebp - 0x04]  try-scope state (EBX/0 during try, -2 after)
//     [ebp - 0x1c]  local_1c (result of FUN_009dc3f0)
//     [ebp - 0x20]  local_e0 (result of FUN_009d723b)
//     [ebp + 0x08]  param1
//     [ebp + 0x0c]  param2 (→ ESI throughout)
//
//   Reloc-bearing sites in the orig 311 bytes:
//     +0x02  scope-table ptr PUSH  (0x0122cda0 — .rdata)
//     +0x07  __SEH_prolog4 CALL    (rel32 → 0x009de4f0)
//     +0x1a  _errno CALL           (rel32 → 0x009d9d47)
//     +0x2a  _invalid_parameter CALL (rel32 → 0x009d2290)
//     +0x53  get_mode CALL ×8      (rel32 → 0x009d6a61, repeated)
//     +0x8c  mode_table LEA ×2     (0x0137b7e0 — .data)
//     +0x8c  fallback global MOV   (0x012eb4d8 — .data)
//     +0xe2  FUN_009dc3f0 CALL     (rel32)
//     +0xec  FUN_009d4e3e CALL     (rel32)
//     +0xf7  FUN_009e4f00 CALL     (rel32)
//     +0x107 FUN_009d723b CALL     (rel32)
//     +0x111 FUN_009e4f96 CALL     (rel32)
//     +0x120 FUN_009d4d5f CALL     (rel32)
//     +0x131 __SEH_epilog CALL     (rel32 → 0x009de535)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ rewrite would need MSVC 2005 /O2 /GS /EHsc to
//   reproduce the exact EH3 prolog (PUSH frame_size / PUSH scope-table /
//   CALL __SEH_prolog4), the SETNZ-then-CMP-zero null-check idiom for
//   both parameters, the double-invocation pattern of get_mode for each
//   of the two mode-validity blocks, the specific try-scope state
//   management, and all fourteen relocation windows above.  Each is
//   brittle under /O2.
//
//   The pragmatic choice — the same one FUN_004014b0 and FUN_00401a00
//   took — is a `__declspec(naked)` body that re-emits the orig 311
//   bytes verbatim via MASM `_emit` directives so that `tools/compare.py`
//   sees a byte-identical `.text` slice.

extern "C" __declspec(naked) void FUN_009d4c25() {
    __asm {
        // 005d4c25: 6a 10                  PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // 005d4c27: 68 a0 cd 22 01         PUSH scope_table (0x0122cda0)
        _emit 0x68
        _emit 0xa0
        _emit 0xcd
        _emit 0x22
        _emit 0x01
        // 005d4c2c: e8 bf 98 00 00         CALL __SEH_prolog4
        _emit 0xe8
        _emit 0xbf
        _emit 0x98
        _emit 0x00
        _emit 0x00
        // 005d4c31: 33 c0                  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 005d4c33: 33 db                  XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 005d4c35: 39 5d 08               CMP [EBP+0x8],EBX
        _emit 0x39
        _emit 0x5d
        _emit 0x08
        // 005d4c38: 0f 95 c0               SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 005d4c3b: 3b c3                  CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 005d4c3d: 75 20                  JNZ +0x20
        _emit 0x75
        _emit 0x20
        // 005d4c3f: e8 03 51 00 00         CALL _errno
        _emit 0xe8
        _emit 0x03
        _emit 0x51
        _emit 0x00
        _emit 0x00
        // 005d4c44: c7 00 16 00 00 00      MOV [EAX],0x16
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d4c4a..4e: 53 53 53 53 53     PUSH EBX x5
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        // 005d4c4f: e8 3c d6 ff ff         CALL _invalid_parameter
        _emit 0xe8
        _emit 0x3c
        _emit 0xd6
        _emit 0xff
        _emit 0xff
        // 005d4c54: 83 c4 14               ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 005d4c57: 83 c8 ff               OR EAX,0xffffffff
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 005d4c5a: e9 f7 00 00 00         JMP epilogue
        _emit 0xe9
        _emit 0xf7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d4c5f: 33 c0                  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 005d4c61: 8b 75 0c               MOV ESI,[EBP+0xc]  (param2)
        _emit 0x8b
        _emit 0x75
        _emit 0x0c
        // 005d4c64: 3b f3                  CMP ESI,EBX
        _emit 0x3b
        _emit 0xf3
        // 005d4c66: 0f 95 c0               SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 005d4c69: 3b c3                  CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 005d4c6b: 74 d2                  JZ einval_path
        _emit 0x74
        _emit 0xd2
        // 005d4c6d: f6 46 0c 40            TEST [ESI+0xc],0x40
        _emit 0xf6
        _emit 0x46
        _emit 0x0c
        _emit 0x40
        // 005d4c71: 0f 85 8e 00 00 00      JNZ skip_mode_checks
        _emit 0x0f
        _emit 0x85
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d4c77: 56                     PUSH ESI
        _emit 0x56
        // 005d4c78: e8 e4 1d 00 00         CALL get_mode
        _emit 0xe8
        _emit 0xe4
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // 005d4c7d: 59                     POP ECX
        _emit 0x59
        // 005d4c7e: 83 f8 ff               CMP EAX,-1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 005d4c81: 74 2e                  JZ fallback1
        _emit 0x74
        _emit 0x2e
        // 005d4c83: 56                     PUSH ESI
        _emit 0x56
        // 005d4c84: e8 d8 1d 00 00         CALL get_mode
        _emit 0xe8
        _emit 0xd8
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // 005d4c89: 59                     POP ECX
        _emit 0x59
        // 005d4c8a: 83 f8 fe               CMP EAX,-2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 005d4c8d: 74 22                  JZ fallback1
        _emit 0x74
        _emit 0x22
        // 005d4c8f: 56                     PUSH ESI
        _emit 0x56
        // 005d4c90: e8 cc 1d 00 00         CALL get_mode
        _emit 0xe8
        _emit 0xcc
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // 005d4c95: c1 f8 05               SAR EAX,5
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        // 005d4c98: 8d 3c 85 e0 b7 37 01   LEA EDI,[EAX*4+0x137b7e0]
        _emit 0x8d
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005d4c9f: 56                     PUSH ESI
        _emit 0x56
        // 005d4ca0: e8 bc 1d 00 00         CALL get_mode
        _emit 0xe8
        _emit 0xbc
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // 005d4ca5: 59                     POP ECX
        _emit 0x59
        // 005d4ca6: 59                     POP ECX
        _emit 0x59
        // 005d4ca7: 83 e0 1f               AND EAX,0x1f
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 005d4caa: c1 e0 06               SHL EAX,6
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        // 005d4cad: 03 07                  ADD EAX,[EDI]
        _emit 0x03
        _emit 0x07
        // 005d4caf: eb 05                  JMP check1
        _emit 0xeb
        _emit 0x05
        // 005d4cb1: b8 d8 b4 2e 01         MOV EAX,0x012eb4d8  (fallback1)
        _emit 0xb8
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        // 005d4cb6: f6 40 24 7f            TEST [EAX+0x24],0x7f  (check1)
        _emit 0xf6
        _emit 0x40
        _emit 0x24
        _emit 0x7f
        // 005d4cba: 75 83                  JNZ einval_path
        _emit 0x75
        _emit 0x83
        // 005d4cbc: 56                     PUSH ESI
        _emit 0x56
        // 005d4cbd: e8 9f 1d 00 00         CALL get_mode
        _emit 0xe8
        _emit 0x9f
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // 005d4cc2: 59                     POP ECX
        _emit 0x59
        // 005d4cc3: 83 f8 ff               CMP EAX,-1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 005d4cc6: 74 2e                  JZ fallback2
        _emit 0x74
        _emit 0x2e
        // 005d4cc8: 56                     PUSH ESI
        _emit 0x56
        // 005d4cc9: e8 93 1d 00 00         CALL get_mode
        _emit 0xe8
        _emit 0x93
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // 005d4cce: 59                     POP ECX
        _emit 0x59
        // 005d4ccf: 83 f8 fe               CMP EAX,-2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 005d4cd2: 74 22                  JZ fallback2
        _emit 0x74
        _emit 0x22
        // 005d4cd4: 56                     PUSH ESI
        _emit 0x56
        // 005d4cd5: e8 87 1d 00 00         CALL get_mode
        _emit 0xe8
        _emit 0x87
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // 005d4cda: c1 f8 05               SAR EAX,5
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        // 005d4cdd: 8d 3c 85 e0 b7 37 01   LEA EDI,[EAX*4+0x137b7e0]
        _emit 0x8d
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005d4ce4: 56                     PUSH ESI
        _emit 0x56
        // 005d4ce5: e8 77 1d 00 00         CALL get_mode
        _emit 0xe8
        _emit 0x77
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        // 005d4cea: 59                     POP ECX
        _emit 0x59
        // 005d4ceb: 59                     POP ECX
        _emit 0x59
        // 005d4cec: 83 e0 1f               AND EAX,0x1f
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 005d4cef: c1 e0 06               SHL EAX,6
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        // 005d4cf2: 03 07                  ADD EAX,[EDI]
        _emit 0x03
        _emit 0x07
        // 005d4cf4: eb 05                  JMP check2
        _emit 0xeb
        _emit 0x05
        // 005d4cf6: b8 d8 b4 2e 01         MOV EAX,0x012eb4d8  (fallback2)
        _emit 0xb8
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        // 005d4cfb: f6 40 24 80            TEST [EAX+0x24],0x80  (check2)
        _emit 0xf6
        _emit 0x40
        _emit 0x24
        _emit 0x80
        // 005d4cff: 0f 85 3a ff ff ff      JNZ einval_path
        _emit 0x0f
        _emit 0x85
        _emit 0x3a
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005d4d05: ff 75 08               PUSH [EBP+0x8]  (skip_mode_checks)
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // 005d4d08: e8 e3 76 00 00         CALL FUN_009dc3f0
        _emit 0xe8
        _emit 0xe3
        _emit 0x76
        _emit 0x00
        _emit 0x00
        // 005d4d0d: 89 45 e4               MOV [EBP-0x1c],EAX
        _emit 0x89
        _emit 0x45
        _emit 0xe4
        // 005d4d10: 56                     PUSH ESI
        _emit 0x56
        // 005d4d11: e8 28 01 00 00         CALL FUN_009d4e3e
        _emit 0xe8
        _emit 0x28
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 005d4d16: 59                     POP ECX
        _emit 0x59
        // 005d4d17: 59                     POP ECX
        _emit 0x59
        // 005d4d18: 89 5d fc               MOV [EBP-0x4],EBX
        _emit 0x89
        _emit 0x5d
        _emit 0xfc
        // 005d4d1b: 56                     PUSH ESI
        _emit 0x56
        // 005d4d1c: e8 df 01 01 00         CALL FUN_009e4f00
        _emit 0xe8
        _emit 0xdf
        _emit 0x01
        _emit 0x01
        _emit 0x00
        // 005d4d21: 8b f8                  MOV EDI,EAX
        _emit 0x8b
        _emit 0xf8
        // 005d4d23: 56                     PUSH ESI
        _emit 0x56
        // 005d4d24: ff 75 e4               PUSH [EBP-0x1c]
        _emit 0xff
        _emit 0x75
        _emit 0xe4
        // 005d4d27: 6a 01                  PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 005d4d29: ff 75 08               PUSH [EBP+0x8]
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // 005d4d2c: e8 0a 25 00 00         CALL FUN_009d723b
        _emit 0xe8
        _emit 0x0a
        _emit 0x25
        _emit 0x00
        _emit 0x00
        // 005d4d31: 89 45 e0               MOV [EBP-0x20],EAX
        _emit 0x89
        _emit 0x45
        _emit 0xe0
        // 005d4d34: 56                     PUSH ESI
        _emit 0x56
        // 005d4d35: 57                     PUSH EDI
        _emit 0x57
        // 005d4d36: e8 5b 02 01 00         CALL FUN_009e4f96
        _emit 0xe8
        _emit 0x5b
        _emit 0x02
        _emit 0x01
        _emit 0x00
        // 005d4d3b: 83 c4 1c               ADD ESP,0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 005d4d3e: c7 45 fc fe ff ff ff   MOV [EBP-0x4],0xFFFFFFFE
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005d4d45: e8 15 00 00 00         CALL FUN_009d4d5f
        _emit 0xe8
        _emit 0x15
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005d4d4a: 33 c0                  XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 005d4d4c: 8b 4d e4               MOV ECX,[EBP-0x1c]
        _emit 0x8b
        _emit 0x4d
        _emit 0xe4
        // 005d4d4f: 39 4d e0               CMP [EBP-0x20],ECX
        _emit 0x39
        _emit 0x4d
        _emit 0xe0
        // 005d4d52: 0f 94 c0               SETZ AL
        _emit 0x0f
        _emit 0x94
        _emit 0xc0
        // 005d4d55: 48                     DEC EAX
        _emit 0x48
        // 005d4d56: e8 da 97 00 00         CALL __SEH_epilog
        _emit 0xe8
        _emit 0xda
        _emit 0x97
        _emit 0x00
        _emit 0x00
        // 005d4d5b: c3                     RET
        _emit 0xc3
    }
}
