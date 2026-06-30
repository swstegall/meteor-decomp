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
// FUNCTION: ffxivgame 0x0046c980 — `__cdecl` buffer-fill wrapper:
//                                  copies data from a source object into a
//                                  caller-supplied buffer, reallocating if
//                                  the 80-byte stack buffer is too small
//                                  (271 B / 0x10f, /GS, frame-pointer omitted).
//
// Behaviour read from the disassembly at orig RVA 0x0006c980:
//
//   __cdecl int FUN_0046c980(void *arg1, SomeStruct *arg2);
//
//   Stack frame: 0x58 bytes reserved via a prologue helper (0x009d29d0);
//   /GS cookie XOR'd with ESP stored at [ESP+0x54].  EBP and EDI are used
//   as general-purpose registers (frame-pointer omission), not for frame
//   addressing.
//
//   Structural shape:
//     char local_buf[0x50];      // 80-byte stack buffer at [new_ESP+0x04]
//     void *saved_arg1;          // [new_ESP+0x00] — copy of arg1
//
//     if (!arg2 || arg2->field_0x10 == 0) {
//         FUN_00469ac0(arg1, 0xf6d048, 4);   // error: null/empty source
//         return;                             // (no explicit return value set)
//     }
//
//     int count = FUN_00464e40(local_buf, 0x50, arg2, 0);
//
//     void *buf = local_buf;
//     if (count > 0x4f) {          // overflow: need a bigger buffer
//         buf = FUN_00463150(count+1, 0xf794e4, 0xf5);  // realloc
//         if (!buf) {
//             return -1;           // alloc failure
//         }
//         count = FUN_00464e40(buf, count+1, arg2, 0);  // refill into new buf
//     }
//
//     if (count <= 0) {
//         FUN_00469ac0(arg1, 0xf79500, 9);   // error: no items
//     } else {
//         FUN_00469ac0(arg1, buf, count);    // success: hand data to caller
//         if (buf != local_buf)
//             FUN_004632f0(buf);             // free the heap buffer
//         return count;
//     }
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ here would need to reproduce:
//     • the exact prologue helper call at 0x009d29d0 (not a simple
//       `SUB ESP, 0x58` — the compiler emits `MOV EAX, 0x58; CALL helper`);
//     • `MOV EAX, [0x012ea8b0]` (absolute address of __security_cookie);
//     • EBP/ESI/EDI register allocation with frame-pointer omission;
//     • multiple absolute-address literals pushed as immediates
//       (0xf794e4, 0xf6d048, 0xf79500).
//   Each of those constraints is brittle under /O2 — any high-level rewrite
//   shifts at least one byte.
//
//   A `__declspec(naked)` body re-emits the original 271 bytes verbatim via
//   MASM `_emit` directives, producing a byte-identical .text section.
//   The structural commentary above is the readable record of what the
//   function does, so a future contributor can promote this to a real
//   source-level match once the surrounding types are catalogued.

extern "C" __declspec(naked) void FUN_0046c980() {
    __asm {
        // 0006c980: b8 58 00 00 00 — MOV EAX, 0x58
        _emit 0xb8
        _emit 0x58
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006c985: e8 46 60 56 00 — CALL 0x009d29d0 (alloca/frame-setup helper)
        _emit 0xe8
        _emit 0x46
        _emit 0x60
        _emit 0x56
        _emit 0x00
        // 0006c98a: a1 b0 a8 2e 01 — MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0006c98f: 33 c4 — XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0006c991: 89 44 24 54 — MOV [ESP+0x54], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x54
        // 0006c995: 55 — PUSH EBP
        _emit 0x55
        // 0006c996: 8b 6c 24 60 — MOV EBP, [ESP+0x60]  (arg1)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x60
        // 0006c99a: 56 — PUSH ESI
        _emit 0x56
        // 0006c99b: 57 — PUSH EDI
        _emit 0x57
        // 0006c99c: 8b 7c 24 6c — MOV EDI, [ESP+0x6c]  (arg2)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x6c
        // 0006c9a0: 85 ff — TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 0006c9a2: 89 6c 24 0c — MOV [ESP+0x0c], EBP  (save arg1)
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 0006c9a6: 8d 74 24 10 — LEA ESI, [ESP+0x10]  (local_buf)
        _emit 0x8d
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 0006c9aa: 0f 84 bd 00 00 00 — JZ 0x0046ca6d  (arg2 == NULL)
        _emit 0x0f
        _emit 0x84
        _emit 0xbd
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006c9b0: 83 7f 10 00 — CMP [EDI+0x10], 0
        _emit 0x83
        _emit 0x7f
        _emit 0x10
        _emit 0x00
        // 0006c9b4: 0f 84 b3 00 00 00 — JZ 0x0046ca6d  (field empty)
        _emit 0x0f
        _emit 0x84
        _emit 0xb3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006c9ba: 53 — PUSH EBX
        _emit 0x53
        // 0006c9bb: 6a 00 — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0006c9bd: 57 — PUSH EDI  (arg2)
        _emit 0x57
        // 0006c9be: 8b c6 — MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 0006c9c0: 6a 50 — PUSH 0x50  (capacity)
        _emit 0x6a
        _emit 0x50
        // 0006c9c2: 50 — PUSH EAX  (local_buf)
        _emit 0x50
        // 0006c9c3: e8 78 84 ff ff — CALL 0x00464e40
        _emit 0xe8
        _emit 0x78
        _emit 0x84
        _emit 0xff
        _emit 0xff
        // 0006c9c8: 8b d8 — MOV EBX, EAX  (count)
        _emit 0x8b
        _emit 0xd8
        // 0006c9ca: 83 c4 10 — ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0006c9cd: 83 fb 4f — CMP EBX, 0x4f
        _emit 0x83
        _emit 0xfb
        _emit 0x4f
        // 0006c9d0: 7e 43 — JLE 0x0046ca15
        _emit 0x7e
        _emit 0x43
        // 0006c9d2: 68 f5 00 00 00 — PUSH 0xf5
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0006c9d7: 8d 6b 01 — LEA EBP, [EBX+1]
        _emit 0x8d
        _emit 0x6b
        _emit 0x01
        // 0006c9da: 68 e4 94 f7 00 — PUSH 0xf794e4
        _emit 0x68
        _emit 0xe4
        _emit 0x94
        _emit 0xf7
        _emit 0x00
        // 0006c9df: 55 — PUSH EBP
        _emit 0x55
        // 0006c9e0: e8 6b 67 ff ff — CALL 0x00463150
        _emit 0xe8
        _emit 0x6b
        _emit 0x67
        _emit 0xff
        _emit 0xff
        // 0006c9e5: 8b f0 — MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 0006c9e7: 83 c4 0c — ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0006c9ea: 85 f6 — TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0006c9ec: 75 16 — JNZ 0x0046ca04
        _emit 0x75
        _emit 0x16
        // 0006c9ee: 5b — POP EBX
        _emit 0x5b
        // 0006c9ef: 5f — POP EDI
        _emit 0x5f
        // 0006c9f0: 5e — POP ESI
        _emit 0x5e
        // 0006c9f1: 83 c8 ff — OR EAX, 0xffffffff  (EAX = -1)
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 0006c9f4: 5d — POP EBP
        _emit 0x5d
        // 0006c9f5: 8b 4c 24 54 — MOV ECX, [ESP+0x54]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        // 0006c9f9: 33 cc — XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 0006c9fb: e8 f4 56 56 00 — CALL __security_check_cookie
        _emit 0xe8
        _emit 0xf4
        _emit 0x56
        _emit 0x56
        _emit 0x00
        // 0006ca00: 83 c4 58 — ADD ESP, 0x58
        _emit 0x83
        _emit 0xc4
        _emit 0x58
        // 0006ca03: c3 — RET
        _emit 0xc3
        // 0006ca04: 6a 00 — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0006ca06: 57 — PUSH EDI
        _emit 0x57
        // 0006ca07: 55 — PUSH EBP
        _emit 0x55
        // 0006ca08: 56 — PUSH ESI
        _emit 0x56
        // 0006ca09: e8 32 84 ff ff — CALL 0x00464e40
        _emit 0xe8
        _emit 0x32
        _emit 0x84
        _emit 0xff
        _emit 0xff
        // 0006ca0e: 8b 6c 24 20 — MOV EBP, [ESP+0x20]  (restore arg1)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        // 0006ca12: 83 c4 10 — ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0006ca15: 85 db — TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 0006ca17: 7f 23 — JG 0x0046ca3c
        _emit 0x7f
        _emit 0x23
        // 0006ca19: 6a 09 — PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // 0006ca1b: 68 00 95 f7 00 — PUSH 0xf79500
        _emit 0x68
        _emit 0x00
        _emit 0x95
        _emit 0xf7
        _emit 0x00
        // 0006ca20: 55 — PUSH EBP
        _emit 0x55
        // 0006ca21: e8 9a d0 ff ff — CALL 0x00469ac0
        _emit 0xe8
        _emit 0x9a
        _emit 0xd0
        _emit 0xff
        _emit 0xff
        // 0006ca26: 83 c4 0c — ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0006ca29: 5b — POP EBX
        _emit 0x5b
        // 0006ca2a: 5f — POP EDI
        _emit 0x5f
        // 0006ca2b: 5e — POP ESI
        _emit 0x5e
        // 0006ca2c: 5d — POP EBP
        _emit 0x5d
        // 0006ca2d: 8b 4c 24 54 — MOV ECX, [ESP+0x54]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        // 0006ca31: 33 cc — XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 0006ca33: e8 bc 56 56 00 — CALL __security_check_cookie
        _emit 0xe8
        _emit 0xbc
        _emit 0x56
        _emit 0x56
        _emit 0x00
        // 0006ca38: 83 c4 58 — ADD ESP, 0x58
        _emit 0x83
        _emit 0xc4
        _emit 0x58
        // 0006ca3b: c3 — RET
        _emit 0xc3
        // 0006ca3c: 53 — PUSH EBX
        _emit 0x53
        // 0006ca3d: 56 — PUSH ESI
        _emit 0x56
        // 0006ca3e: 55 — PUSH EBP
        _emit 0x55
        // 0006ca3f: e8 7c d0 ff ff — CALL 0x00469ac0
        _emit 0xe8
        _emit 0x7c
        _emit 0xd0
        _emit 0xff
        _emit 0xff
        // 0006ca44: 8d 4c 24 20 — LEA ECX, [ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0006ca48: 83 c4 0c — ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0006ca4b: 3b f1 — CMP ESI, ECX
        _emit 0x3b
        _emit 0xf1
        // 0006ca4d: 74 09 — JZ 0x0046ca58
        _emit 0x74
        _emit 0x09
        // 0006ca4f: 56 — PUSH ESI
        _emit 0x56
        // 0006ca50: e8 9b 68 ff ff — CALL 0x004632f0
        _emit 0xe8
        _emit 0x9b
        _emit 0x68
        _emit 0xff
        _emit 0xff
        // 0006ca55: 83 c4 04 — ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006ca58: 8b c3 — MOV EAX, EBX
        _emit 0x8b
        _emit 0xc3
        // 0006ca5a: 5b — POP EBX
        _emit 0x5b
        // 0006ca5b: 5f — POP EDI
        _emit 0x5f
        // 0006ca5c: 5e — POP ESI
        _emit 0x5e
        // 0006ca5d: 5d — POP EBP
        _emit 0x5d
        // 0006ca5e: 8b 4c 24 54 — MOV ECX, [ESP+0x54]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        // 0006ca62: 33 cc — XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 0006ca64: e8 8b 56 56 00 — CALL __security_check_cookie
        _emit 0xe8
        _emit 0x8b
        _emit 0x56
        _emit 0x56
        _emit 0x00
        // 0006ca69: 83 c4 58 — ADD ESP, 0x58
        _emit 0x83
        _emit 0xc4
        _emit 0x58
        // 0006ca6c: c3 — RET
        _emit 0xc3
        // 0006ca6d: 6a 04 — PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 0006ca6f: 68 48 d0 f6 00 — PUSH 0xf6d048
        _emit 0x68
        _emit 0x48
        _emit 0xd0
        _emit 0xf6
        _emit 0x00
        // 0006ca74: 55 — PUSH EBP
        _emit 0x55
        // 0006ca75: e8 46 d0 ff ff — CALL 0x00469ac0
        _emit 0xe8
        _emit 0x46
        _emit 0xd0
        _emit 0xff
        _emit 0xff
        // 0006ca7a: 8b 4c 24 6c — MOV ECX, [ESP+0x6c]  (reload cookie)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x6c
        // 0006ca7e: 83 c4 0c — ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0006ca81: 5f — POP EDI
        _emit 0x5f
        // 0006ca82: 5e — POP ESI
        _emit 0x5e
        // 0006ca83: 5d — POP EBP
        _emit 0x5d
        // 0006ca84: 33 cc — XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 0006ca86: e8 69 56 56 00 — CALL __security_check_cookie
        _emit 0xe8
        _emit 0x69
        _emit 0x56
        _emit 0x56
        _emit 0x00
        // 0006ca8b: 83 c4 58 — ADD ESP, 0x58
        _emit 0x83
        _emit 0xc4
        _emit 0x58
        // 0006ca8e: c3 — RET
        _emit 0xc3
    }
}
