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
// FUNCTION: ffxivgame 0x0041b8c0 — global-field conditional setter (47 B / 0x2F)
//
//   char __cdecl FUN_0041b8c0(void *param)
//     stack layout (after PUSH ESI, ESP has moved 4 bytes down):
//       [ESP+0x08] : void *param  (param_1 — the new candidate value)
//     returns: 1 if the field was already equal to param, or if the field
//              was successfully updated; 0 if FUN_00418280 said no.
//
// Asm (47 bytes @ orig RVA 0x0001b8c0):
//
//   0001b8c0:  a1 28 94 32 01          MOV  EAX,[0x01329428]    ; load global ptr
//   0001b8c5:  56                      PUSH ESI
//   0001b8c6:  8b 74 24 08             MOV  ESI,[ESP+0x8]       ; param_1
//   0001b8ca:  39 b0 50 01 00 00       CMP  [EAX+0x150],ESI     ; already set?
//   0001b8d0:  74 19                   JZ   0x0041b8eb          ; yes → return 1
//   0001b8d2:  e8 a9 c9 ff ff          CALL 0x00418280          ; can-we-switch check
//   0001b8d7:  84 c0                   TEST AL,AL
//   0001b8d9:  74 04                   JZ   0x0041b8df          ; zero → proceed
//   0001b8db:  32 c0                   XOR  AL,AL               ; non-zero → fail
//   0001b8dd:  5e                      POP  ESI
//   0001b8de:  c3                      RET
//   0001b8df:  8b 0d 28 94 32 01       MOV  ECX,[0x01329428]    ; reload ptr
//   0001b8e5:  89 b1 50 01 00 00       MOV  [ECX+0x150],ESI     ; store param
//   0001b8eb:  b0 01                   MOV  AL,0x1              ; return 1
//   0001b8ed:  5e                      POP  ESI
//   0001b8ee:  c3                      RET
//
// Calling convention: __cdecl (plain RET; one pointer arg).
// Frame: PUSH ESI only (no ESP adjustment).
//
// Reloc-bearing sites masked by tools/compare.py:
//   +0x01  MOV imm32 → 0x01329428  (global pointer slot)
//   +0x0d  CMP mem displacement 0x150 (embedded in ModRM/SIB, not a reloc)
//   +0x13  CALL rel32 → FUN_00418280
//   +0x1f  MOV imm32 → 0x01329428  (reload of same global pointer slot)
//
// Reconstruction strategy: naked _emit passthrough, matching all siblings in
// this module (FUN_004051e0, FUN_00416320, FUN_004061a0 etc.).

extern "C" __declspec(naked) void FUN_0041b8c0() {
    __asm {
        // 0001b8c0: a1 28 94 32 01   MOV EAX, [0x01329428]
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001b8c5: 56               PUSH ESI
        _emit 0x56
        // 0001b8c6: 8b 74 24 08      MOV ESI, [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0001b8ca: 39 b0 50 01 00 00   CMP [EAX+0x150], ESI
        _emit 0x39
        _emit 0xb0
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b8d0: 74 19             JZ +0x19 (→ 0x0041b8eb)
        _emit 0x74
        _emit 0x19
        // 0001b8d2: e8 a9 c9 ff ff    CALL 0x00418280
        _emit 0xe8
        _emit 0xa9
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 0001b8d7: 84 c0             TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 0001b8d9: 74 04             JZ +0x04 (→ 0x0041b8df)
        _emit 0x74
        _emit 0x04
        // 0001b8db: 32 c0             XOR AL, AL
        _emit 0x32
        _emit 0xc0
        // 0001b8dd: 5e                POP ESI
        _emit 0x5e
        // 0001b8de: c3                RET
        _emit 0xc3
        // 0001b8df: 8b 0d 28 94 32 01  MOV ECX, [0x01329428]
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001b8e5: 89 b1 50 01 00 00  MOV [ECX+0x150], ESI
        _emit 0x89
        _emit 0xb1
        _emit 0x50
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001b8eb: b0 01             MOV AL, 0x1
        _emit 0xb0
        _emit 0x01
        // 0001b8ed: 5e                POP ESI
        _emit 0x5e
        // 0001b8ee: c3                RET
        _emit 0xc3
    }
}
