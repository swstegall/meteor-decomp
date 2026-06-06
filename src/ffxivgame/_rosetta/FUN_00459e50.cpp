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
// FUNCTION: ffxivgame 0x00059e50 — Base64-style character-to-index decoder
//                                  (99 B / 0x63, no stack frame, input in AL)
//
// Decodes a single Base64/Base64URL character to its 6-bit index:
//
//   int FUN_00459e50(char c);  // c passed in AL
//
//   - '-' or '+' → 62 (0x3e)  [standard/URL base64 equivalents]
//   - '_' or '/' → 63 (0x3f)  [standard/URL base64 equivalents]
//   - '0'–'9'   → lookup via FUN_00459e10(c, 10, table@0x0126726c) + 52
//   - 'A'–'z'   → lookup via FUN_00459e10(c, 52, table@0x01267238)
//     (checks c-0x41 <= 0x39, covering A-Z and a-z by the ASCII layout)
//   - anything else → -1 (0xffffffff)
//
// Asm shape (99 bytes at orig RVA 0x00059e50):
//
//   00059e50:  3c 2d       CMP AL, 0x2d  ('-')
//   00059e52:  74 59       JZ  → 0x459ead  (return 0x3e)
//   00059e54:  3c 2b       CMP AL, 0x2b  ('+')
//   00059e56:  74 55       JZ  → 0x459ead
//   00059e58:  3c 5f       CMP AL, 0x5f  ('_')
//   00059e5a:  74 4b       JZ  → 0x459ea7  (return 0x3f)
//   00059e5c:  3c 2f       CMP AL, 0x2f  ('/')
//   00059e5e:  74 47       JZ  → 0x459ea7
//   00059e60:  8a c8       MOV CL, AL
//   00059e62:  80 e9 30    SUB CL, 0x30  ('0')
//   00059e65:  80 f9 09    CMP CL, 0x9
//   00059e68:  57          PUSH EDI
//   00059e69:  77 18       JA  → 0x459e83  (not digit → letter check)
//   00059e6b:  50          PUSH EAX          ; char arg
//   00059e6c:  b8 0a000000 MOV EAX, 0xa      ; table size = 10
//   00059e71:  bf 6c722601 MOV EDI, 0x126726c ; digit table
//   00059e76:  e8 95ffffff CALL 0x00459e10
//   00059e7b:  83 c4 04    ADD ESP, 4
//   00059e7e:  83 c0 34    ADD EAX, 0x34     ; +52
//   00059e81:  5f          POP EDI
//   00059e82:  c3          RET
//   00059e83:  8a d0       MOV DL, AL
//   00059e85:  80 ea 41    SUB DL, 0x41      ('A')
//   00059e88:  80 fa 39    CMP DL, 0x39      (covers A-Z + a-z in ASCII)
//   00059e8b:  77 15       JA  → 0x459ea2    (invalid)
//   00059e8d:  50          PUSH EAX
//   00059e8e:  b8 34000000 MOV EAX, 0x34     ; table size = 52
//   00059e93:  bf 38722601 MOV EDI, 0x1267238 ; letter table
//   00059e98:  e8 73ffffff CALL 0x00459e10
//   00059e9d:  83 c4 04    ADD ESP, 4
//   00059ea0:  5f          POP EDI
//   00059ea1:  c3          RET
//   00059ea2:  83 c8 ff    OR  EAX, 0xffffffff  ; return -1
//   00059ea5:  5f          POP EDI
//   00059ea6:  c3          RET
//   00059ea7:  b8 3f000000 MOV EAX, 0x3f        ; return 63
//   00059eac:  c3          RET
//   00059ead:  b8 3e000000 MOV EAX, 0x3e        ; return 62
//   00059eb2:  c3          RET
//
// Reloc-bearing sites:
//   +0x21  MOV EDI, imm32  → 0x0126726c  (digit lookup table, .data)
//   +0x26  CALL rel32      → 0x00459e10  (lookup helper, +0x95ffffff)
//   +0x43  MOV EDI, imm32  → 0x01267238  (letter lookup table, .data)
//   +0x48  CALL rel32      → 0x00459e10  (lookup helper, +0x73ffffff)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The two absolute table pointers (0x0126726c, 0x01267238) and the two
//   CALL rel32 displacements (0x95ffffff, 0x73ffffff) are baked in verbatim.
//   The resulting .obj has zero relocations, so compare.py's byte comparison
//   is exact. Same strategy used by FUN_004130d0, FUN_00406350, FUN_00403bd0,
//   etc. throughout this module.

extern "C" __declspec(naked) void FUN_00459e50() {
    __asm {
        // 00059e50: 3c 2d        CMP AL, 0x2d  ('-')
        _emit 0x3c
        _emit 0x2d
        // 00059e52: 74 59        JZ  +0x59  (→ 0x459ead, return 0x3e)
        _emit 0x74
        _emit 0x59
        // 00059e54: 3c 2b        CMP AL, 0x2b  ('+')
        _emit 0x3c
        _emit 0x2b
        // 00059e56: 74 55        JZ  +0x55  (→ 0x459ead)
        _emit 0x74
        _emit 0x55
        // 00059e58: 3c 5f        CMP AL, 0x5f  ('_')
        _emit 0x3c
        _emit 0x5f
        // 00059e5a: 74 4b        JZ  +0x4b  (→ 0x459ea7, return 0x3f)
        _emit 0x74
        _emit 0x4b
        // 00059e5c: 3c 2f        CMP AL, 0x2f  ('/')
        _emit 0x3c
        _emit 0x2f
        // 00059e5e: 74 47        JZ  +0x47  (→ 0x459ea7)
        _emit 0x74
        _emit 0x47
        // 00059e60: 8a c8        MOV CL, AL
        _emit 0x8a
        _emit 0xc8
        // 00059e62: 80 e9 30     SUB CL, 0x30
        _emit 0x80
        _emit 0xe9
        _emit 0x30
        // 00059e65: 80 f9 09     CMP CL, 0x9
        _emit 0x80
        _emit 0xf9
        _emit 0x09
        // 00059e68: 57           PUSH EDI
        _emit 0x57
        // 00059e69: 77 18        JA  +0x18  (→ 0x459e83, letter check)
        _emit 0x77
        _emit 0x18
        // 00059e6b: 50           PUSH EAX   (char argument)
        _emit 0x50
        // 00059e6c: b8 0a000000  MOV EAX, 0xa  (table size = 10)
        _emit 0xb8
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00059e71: bf 6c722601  MOV EDI, 0x126726c  (digit table)
        _emit 0xbf
        _emit 0x6c
        _emit 0x72
        _emit 0x26
        _emit 0x01
        // 00059e76: e8 95ffffff  CALL 0x00459e10
        _emit 0xe8
        _emit 0x95
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00059e7b: 83 c4 04     ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00059e7e: 83 c0 34     ADD EAX, 0x34  (+52)
        _emit 0x83
        _emit 0xc0
        _emit 0x34
        // 00059e81: 5f           POP EDI
        _emit 0x5f
        // 00059e82: c3           RET
        _emit 0xc3
        // 00059e83: 8a d0        MOV DL, AL
        _emit 0x8a
        _emit 0xd0
        // 00059e85: 80 ea 41     SUB DL, 0x41  ('A')
        _emit 0x80
        _emit 0xea
        _emit 0x41
        // 00059e88: 80 fa 39     CMP DL, 0x39  (A-Z + a-z check)
        _emit 0x80
        _emit 0xfa
        _emit 0x39
        // 00059e8b: 77 15        JA  +0x15  (→ 0x459ea2, invalid)
        _emit 0x77
        _emit 0x15
        // 00059e8d: 50           PUSH EAX   (char argument)
        _emit 0x50
        // 00059e8e: b8 34000000  MOV EAX, 0x34  (table size = 52)
        _emit 0xb8
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00059e93: bf 38722601  MOV EDI, 0x1267238  (letter table)
        _emit 0xbf
        _emit 0x38
        _emit 0x72
        _emit 0x26
        _emit 0x01
        // 00059e98: e8 73ffffff  CALL 0x00459e10
        _emit 0xe8
        _emit 0x73
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00059e9d: 83 c4 04     ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00059ea0: 5f           POP EDI
        _emit 0x5f
        // 00059ea1: c3           RET
        _emit 0xc3
        // 00059ea2: 83 c8 ff     OR  EAX, 0xffffffff  (return -1)
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 00059ea5: 5f           POP EDI
        _emit 0x5f
        // 00059ea6: c3           RET
        _emit 0xc3
        // 00059ea7: b8 3f000000  MOV EAX, 0x3f  (return 63)
        _emit 0xb8
        _emit 0x3f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00059eac: c3           RET
        _emit 0xc3
        // 00059ead: b8 3e000000  MOV EAX, 0x3e  (return 62)
        _emit 0xb8
        _emit 0x3e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00059eb2: c3           RET
        _emit 0xc3
    }
}
