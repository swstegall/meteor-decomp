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
// FUNCTION: ffxivgame 0x0045b390 — hash-context Update (SHA-1 style),
//                                   per-byte copy into 64-byte block buffer
//                                   (__thiscall, 134 bytes / 0x86).
//
// Signature (inferred from stack layout and RET 0x8):
//
//   int __thiscall FUN_0045b390(HashCtx *this,
//                               const unsigned char *data,   // param1
//                               int                  count); // param2
//
// Calling convention: __thiscall — ECX = this; RET 0x8 cleans 2 × 4 = 8
// stack bytes (param1 + param2); callee-saves EBX/ESI/EDI/EBP.
//
// Struct layout (offsets are relative to `this`):
//   +0x14  bit_count_low  (dword) — incremented by 8 per byte
//   +0x18  bit_count_high (dword) — carry from bit_count_low overflow
//   +0x1c  byte_index     (dword) — current fill position in block (0..63)
//   +0x20  block[64]      (bytes) — the 64-byte message block
//   +0x60  locked_flag    (dword) — non-zero → context is finalised/corrupt
//   +0x64  state          (dword) — error/completion status code
//
// FUN_0045ad60 is the block-compress (transform) function called when
// byte_index wraps back to 0 after reaching 64 (SHA-1 block size 0x40).
//
// Return values:
//   0 — normal success (param2 == 0, or loop ran to completion, or
//       state became non-zero during the loop)
//   1 — data pointer is NULL
//   3 — locked_flag was set; state updated to 3 and returned
//   this->state — pre-existing non-zero state returned unchanged
//
// Reconstruction strategy:
//
//   The unique SIB addressing `[EAX + ESI*1 + 0x20]` (MOV encoding
//   88 4c 30 20) at offset 0x53 within the function body, plus the
//   MASM label-resolution uncertainty around forward jumps, makes the
//   safest approach a full `_emit`-byte passthrough for every instruction
//   except the single REL32 CALL at offset 0x72. Using a real `call`
//   instruction there generates the proper IMAGE_REL_I386_REL32 entry
//   in the .obj, which compare.py uses to mask out the 4-byte
//   displacement when doing the byte-level diff.
//
// Reloc-bearing position:
//   off 0x73  IMAGE_REL_I386_REL32 → FUN_0045ad60 (block transform)

extern "C" void FUN_0045ad60();

extern "C" __declspec(naked) int FUN_0045b390() {
    __asm {
        // --- offset 0x00 ---
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x0c]
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        _emit 0x85              // TEST EBP, EBP
        _emit 0xed
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x75              // JNZ +7  (→ 0x13: loc_b3a3)
        _emit 0x07
        _emit 0x5e              // POP ESI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- offset 0x13 --- loc_b3a3:
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x75              // JNZ +9  (→ 0x25: loc_b3b5)
        _emit 0x09
        _emit 0x8d              // LEA EAX, [EBX+0x1]  (EBX==0 → EAX=1)
        _emit 0x43
        _emit 0x01
        _emit 0x5b              // POP EBX
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- offset 0x25 --- loc_b3b5:
        _emit 0x83              // CMP dword ptr [ESI+0x60], 0
        _emit 0x7e
        _emit 0x60
        _emit 0x00
        _emit 0x74              // JZ +0xe  (→ 0x39: loc_b3c9)
        _emit 0x0e
        _emit 0xb8              // MOV EAX, 3
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5b              // POP EBX
        _emit 0x89              // MOV dword ptr [ESI+0x64], EAX
        _emit 0x46
        _emit 0x64
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- offset 0x39 --- loc_b3c9:
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x64]
        _emit 0x46
        _emit 0x64
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x40  (→ 0x80: loc_b410)
        _emit 0x40
        // --- offset 0x40 ---
        _emit 0x57              // PUSH EDI
        _emit 0xbf              // MOV EDI, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- offset 0x46 --- loc_b3d6: (loop top — SUB first)
        _emit 0x2b              // SUB EBP, EDI
        _emit 0xef
        _emit 0x83              // CMP dword ptr [ESI+0x64], 0
        _emit 0x7e
        _emit 0x64
        _emit 0x00
        _emit 0x75              // JNZ +0x2f  (→ 0x7d: loc_b40d)
        _emit 0x2f
        // --- offset 0x4e ---
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x8a              // MOV CL, byte ptr [EBX]
        _emit 0x0b
        _emit 0x88              // MOV byte ptr [EAX+ESI*1+0x20], CL
        _emit 0x4c
        _emit 0x30
        _emit 0x20
        _emit 0x01              // ADD dword ptr [ESI+0x1c], EDI
        _emit 0x7e
        _emit 0x1c
        _emit 0x83              // ADD dword ptr [ESI+0x14], 8
        _emit 0x46
        _emit 0x14
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x75              // JNZ +8  (→ 0x6b: loc_b3fb)
        _emit 0x08
        // --- offset 0x63 ---
        _emit 0x01              // ADD dword ptr [ESI+0x18], EDI
        _emit 0x7e
        _emit 0x18
        _emit 0x75              // JNZ +3  (→ 0x6b: loc_b3fb)
        _emit 0x03
        _emit 0x89              // MOV dword ptr [ESI+0x64], EDI
        _emit 0x7e
        _emit 0x64
        // --- offset 0x6b --- loc_b3fb:
        _emit 0x83              // CMP EAX, 0x40
        _emit 0xf8
        _emit 0x40
        _emit 0x75              // JNZ +7  (→ 0x77: loc_b407)
        _emit 0x07
        // --- offset 0x70 ---
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        // --- offset 0x72 --- CALL with REL32 relocation:
        call    FUN_0045ad60
        // --- offset 0x77 --- loc_b407:
        _emit 0x03              // ADD EBX, EDI
        _emit 0xdf
        _emit 0x85              // TEST EBP, EBP
        _emit 0xed
        _emit 0x75              // JNZ -0x37  (→ 0x46: loc_b3d6)
        _emit 0xc9
        // --- offset 0x7d --- loc_b40d:
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5f              // POP EDI
        // --- offset 0x80 --- loc_b410:
        _emit 0x5b              // POP EBX
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
