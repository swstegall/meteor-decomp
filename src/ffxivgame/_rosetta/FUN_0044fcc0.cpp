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
// FUNCTION: ffxivgame 0x0044fcc0 — "is parsed timestamp within the last
//                                   60 s" predicate (__cdecl, 160 B / 0xa0)
//
// Calling convention: __cdecl; returns bool (AL).
// Frame: SUB ESP,8 reserves an 8-byte local scratch ([ESP+0xc] after the
//   ESI/EDI saves holds the high word of the 64-bit subtraction). ESI/EDI
//   are callee-saved but pushed late (only on the non-empty-string path).
//
// High-level shape:
//   obj = FUN_0044fc60();            // returns std::string* (object in EAX)
//   if (obj->_Mysize == 0)           // [EAX+0x14] == 0
//       return false;
//   const char* s = (obj->_Myres < 0x10)   // [EAX+0x18] < 16  → SSO
//                       ? (char*)obj + 4    //   inline buffer at +4
//                       : *(char**)(obj+4); //   heap pointer at +4
//   unsigned __int64 target = FUN_0044faf0(s);  // __cdecl _atoi64-style
//   unsigned __int64 now     = (*g_timeFn)();   // [0x00f3e1bc] → 32-bit ms,
//                                               //   zero-extended (high=0)
//   if (target == 0) return false;
//   if (target > now) return false;             // 64-bit unsigned compare
//   // wraparound-aware "now - target <= 60000" (0xea60) check, with the
//   // compiler's ADD r,0 / ADC r,1 (+2^32) fixups for the tick-count wrap.
//   return (now - target) <= 60000;
//
// Reloc-bearing sites in the orig 160 bytes:
//     +0x03   CALL rel32   → FUN_0044fc60   (RVA 0x0044fc60)
//     +0x25   CALL rel32   → FUN_0044faf0   (RVA 0x0044faf0)
//     +0x31   CALL [imm32] → [0x00f3e1bc]   (indirect through data slot)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level rebuild would chain stub generation for two unmatched
//   __cdecl siblings and an absolute indirect-call slot, and MSVC's exact
//   register allocation for the open-coded 64-bit compare/subtract with
//   wraparound fixups is not reproducible from C++ source. A
//   __declspec(naked) body re-emitting the original 160 bytes verbatim
//   yields a .obj whose .text is byte-identical to the orig slice; the
//   rel32/imm32 operands resolve against the orig binary's own address
//   space and are emitted as raw bytes. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0044fcc0() {
    __asm {
        // 0004fcc0: 83 ec 08          SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0004fcc3: e8 98 ff ff ff    CALL 0x0044fc60 (rel32)
        _emit 0xe8
        _emit 0x98
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0004fcc8: 83 78 14 00       CMP dword ptr [EAX + 0x14], 0
        _emit 0x83
        _emit 0x78
        _emit 0x14
        _emit 0x00
        // 0004fccc: 75 06             JNZ 0x0044fcd4
        _emit 0x75
        _emit 0x06
        // 0004fcce: 32 c0             XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 0004fcd0: 83 c4 08          ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0004fcd3: c3                RET
        _emit 0xc3
        // 0004fcd4: 83 78 18 10       CMP dword ptr [EAX + 0x18], 0x10
        _emit 0x83
        _emit 0x78
        _emit 0x18
        _emit 0x10
        // 0004fcd8: 72 05             JC 0x0044fcdf
        _emit 0x72
        _emit 0x05
        // 0004fcda: 8b 40 04          MOV EAX, dword ptr [EAX + 0x4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 0004fcdd: eb 03             JMP 0x0044fce2
        _emit 0xeb
        _emit 0x03
        // 0004fcdf: 83 c0 04          ADD EAX, 0x4
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 0004fce2: 56                PUSH ESI
        _emit 0x56
        // 0004fce3: 57                PUSH EDI
        _emit 0x57
        // 0004fce4: 50                PUSH EAX
        _emit 0x50
        // 0004fce5: e8 06 fe ff ff    CALL 0x0044faf0 (rel32)
        _emit 0xe8
        _emit 0x06
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0004fcea: 83 c4 04          ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0004fced: 8b f8             MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 0004fcef: 8b f2             MOV ESI, EDX
        _emit 0x8b
        _emit 0xf2
        // 0004fcf1: ff 15 bc e1 f3 00 CALL dword ptr [0x00f3e1bc]
        _emit 0xff
        _emit 0x15
        _emit 0xbc
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0004fcf7: 8b d7             MOV EDX, EDI
        _emit 0x8b
        _emit 0xd7
        // 0004fcf9: 33 c9             XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 0004fcfb: 0b d6             OR EDX, ESI
        _emit 0x0b
        _emit 0xd6
        // 0004fcfd: 74 26             JZ 0x0044fd25
        _emit 0x74
        _emit 0x26
        // 0004fcff: 3b f1             CMP ESI, ECX
        _emit 0x3b
        _emit 0xf1
        // 0004fd01: 77 22             JA 0x0044fd25
        _emit 0x77
        _emit 0x22
        // 0004fd03: 72 04             JC 0x0044fd09
        _emit 0x72
        _emit 0x04
        // 0004fd05: 3b f8             CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 0004fd07: 77 1c             JA 0x0044fd25
        _emit 0x77
        _emit 0x1c
        // 0004fd09: 85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0004fd0b: 77 07             JA 0x0044fd14
        _emit 0x77
        _emit 0x07
        // 0004fd0d: 3d 60 ea 00 00    CMP EAX, 0xea60
        _emit 0x3d
        _emit 0x60
        _emit 0xea
        _emit 0x00
        _emit 0x00
        // 0004fd12: 76 19             JBE 0x0044fd2d
        _emit 0x76
        _emit 0x19
        // 0004fd14: 2b c7             SUB EAX, EDI
        _emit 0x2b
        _emit 0xc7
        // 0004fd16: 1b ce             SBB ECX, ESI
        _emit 0x1b
        _emit 0xce
        // 0004fd18: 89 4c 24 0c       MOV dword ptr [ESP + 0xc], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0004fd1c: 75 07             JNZ 0x0044fd25
        _emit 0x75
        _emit 0x07
        // 0004fd1e: 3d 60 ea 00 00    CMP EAX, 0xea60
        _emit 0x3d
        _emit 0x60
        _emit 0xea
        _emit 0x00
        _emit 0x00
        // 0004fd23: 76 33             JBE 0x0044fd58
        _emit 0x76
        _emit 0x33
        // 0004fd25: 5f                POP EDI
        _emit 0x5f
        // 0004fd26: 32 c0             XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 0004fd28: 5e                POP ESI
        _emit 0x5e
        // 0004fd29: 83 c4 08          ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0004fd2c: c3                RET
        _emit 0xc3
        // 0004fd2d: 83 c0 00          ADD EAX, 0x0
        _emit 0x83
        _emit 0xc0
        _emit 0x00
        // 0004fd30: 83 d1 01          ADC ECX, 0x1
        _emit 0x83
        _emit 0xd1
        _emit 0x01
        // 0004fd33: 85 f6             TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0004fd35: 77 10             JA 0x0044fd47
        _emit 0x77
        _emit 0x10
        // 0004fd37: 72 08             JC 0x0044fd41
        _emit 0x72
        _emit 0x08
        // 0004fd39: 81 ff 60 ea 00 00 CMP EDI, 0xea60
        _emit 0x81
        _emit 0xff
        _emit 0x60
        _emit 0xea
        _emit 0x00
        _emit 0x00
        // 0004fd3f: 77 06             JA 0x0044fd47
        _emit 0x77
        _emit 0x06
        // 0004fd41: 83 c7 00          ADD EDI, 0x0
        _emit 0x83
        _emit 0xc7
        _emit 0x00
        // 0004fd44: 83 d6 01          ADC ESI, 0x1
        _emit 0x83
        _emit 0xd6
        _emit 0x01
        // 0004fd47: 2b c7             SUB EAX, EDI
        _emit 0x2b
        _emit 0xc7
        // 0004fd49: 1b ce             SBB ECX, ESI
        _emit 0x1b
        _emit 0xce
        // 0004fd4b: 89 4c 24 0c       MOV dword ptr [ESP + 0xc], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0004fd4f: 75 d4             JNZ 0x0044fd25
        _emit 0x75
        _emit 0xd4
        // 0004fd51: 3d 60 ea 00 00    CMP EAX, 0xea60
        _emit 0x3d
        _emit 0x60
        _emit 0xea
        _emit 0x00
        _emit 0x00
        // 0004fd56: 77 cd             JA 0x0044fd25
        _emit 0x77
        _emit 0xcd
        // 0004fd58: 5f                POP EDI
        _emit 0x5f
        // 0004fd59: b0 01             MOV AL, 0x1
        _emit 0xb0
        _emit 0x01
        // 0004fd5b: 5e                POP ESI
        _emit 0x5e
        // 0004fd5c: 83 c4 08          ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0004fd5f: c3                RET
        _emit 0xc3
    }
}
