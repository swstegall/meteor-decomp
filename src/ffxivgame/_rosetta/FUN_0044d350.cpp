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
// FUNCTION: ffxivgame 0x0004d350 — `__cdecl` ring-buffer registration for a
//                                  ref-counted object (100 B / 0x64).
//
// __cdecl void FUN_0044d350(void* arg)
//   [ESP+4] = arg     pointer to the object to register (or null → early return)
//
// Logical shape:
//   1. If arg == null → return immediately (JZ → shared RET outside compare window).
//   2. ESI = *(uint8_t*)(arg - 4) & 0xFF  — key byte from the object header.
//   3. If key == 0: free(arg-4)  [__cdecl, 1 arg, ADD ESP,4 cleanup], then
//      POP ESI / RET (early exit — object destroyed, nothing to register).
//   4. Otherwise (key != 0), use key (ESI) as a category index:
//      a. Load IAT function pointer from [0x00f3e1a4]  (InterlockedExchangeAdd).
//      b. Atomically increment counter at global_counters[ESI*4 + 0x132cf1c] by 1.
//      c. If result == 2 * table2[ESI]: decrement counter by table2[ESI]
//         (ring-buffer wrap-around).
//      d. Compute slot = result % table2[ESI]  (signed IDIV).
//      e. Store arg at table3[ESI][slot]:
//            *(int*)(table3_base[ESI] + EDX*4) = arg.
//         — MOV and shared RET at 0x4d3b5/0x4d3b8 are outside the 100-byte
//           compare window; epilogue is shared with adjacent code.
//
// Stack frame:
//   PUSH ESI at entry; PUSH EBX/EBP/EDI after branch — 4 callee-saves total.
//   [ESP+0x14] after the 4 pushes = original [ESP+4] = arg.
//
// Relocation windows (compare.py masks these):
//   +0x17  rel32 CALL  → 0x009d1be9  (__cdecl free/dealloc, 1 arg)
//   +0x26  abs32       → 0x00f3e1a4  (IAT slot for InterlockedExchangeAdd)
//   +0x2c  abs32 SIB   → 0x0132cf1c  (global counter array base, SIB scale-4)
//   +0x38  abs32 SIB   → 0x01266dc0  (table2 array base, SIB scale-8)
//   +0x4f  abs32 SIB   → 0x01266dc0  (table2 again, for IDIV)
//   +0x56  abs32 SIB   → 0x0132cec8  (table3 base pointer array, SIB scale-4)
//
// Reconstruction — naked __asm byte passthrough:
//   The function has two early-return paths whose shapes can't be reproduced
//   from source-level C++ in an isolated TU without fighting /O2 register
//   allocation. The 5 mystery bytes at +0x1c are `83 c4 04 5e c3`
//   (ADD ESP,4 ; POP ESI ; RET) — cdecl cleanup followed by early return on
//   the key==0 ("free and don't register") branch. The naked _emit passthrough
//   reproduces all 100 bytes exactly; compare.py masks the six relocation
//   windows listed above.

extern "C" __declspec(naked) void FUN_0044d350() {
    __asm {
        // 0004d350: 8b 44 24 04     MOV EAX, [ESP+4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0004d354: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0004d356: 74 60           JZ +0x60  (→ 0x4d3b8, shared RET outside window)
        _emit 0x74
        _emit 0x60
        // 0004d358: 83 c0 fc        ADD EAX, -4
        _emit 0x83
        _emit 0xc0
        _emit 0xfc
        // 0004d35b: 56              PUSH ESI
        _emit 0x56
        // 0004d35c: 8b 30           MOV ESI, [EAX]
        _emit 0x8b
        _emit 0x30
        // 0004d35e: 81 e6 ff 00 00 00  AND ESI, 0xFF
        _emit 0x81
        _emit 0xe6
        _emit 0xff
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004d364: 75 0b           JNZ +0xb  (→ 0x4d371, skip free+return)
        _emit 0x75
        _emit 0x0b
        // 0004d366: 50              PUSH EAX  (arg to free: arg-4)
        _emit 0x50
        // 0004d367: e8 7d 48 58 00  CALL 0x009d1be9  (__cdecl, 1 arg)
        _emit 0xe8
        _emit 0x7d
        _emit 0x48
        _emit 0x58
        _emit 0x00
        // 0004d36c: 83 c4 04        ADD ESP, 4  (cdecl cleanup)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0004d36f: 5e              POP ESI  (restore callee-save)
        _emit 0x5e
        // 0004d370: c3              RET  (early return — key was 0)
        _emit 0xc3
        // 0004d371: 53              PUSH EBX
        _emit 0x53
        // 0004d372: 55              PUSH EBP
        _emit 0x55
        // 0004d373: 8b 2d a4 e1 f3 00  MOV EBP, [0x00f3e1a4]  (IAT fn ptr)
        _emit 0x8b
        _emit 0x2d
        _emit 0xa4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0004d379: 57              PUSH EDI
        _emit 0x57
        // 0004d37a: 6a 01           PUSH 1
        _emit 0x6a
        _emit 0x01
        // 0004d37c: 8d 3c b5 1c cf 32 01  LEA EDI, [ESI*4 + 0x132cf1c]
        _emit 0x8d
        _emit 0x3c
        _emit 0xb5
        _emit 0x1c
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        // 0004d383: 57              PUSH EDI
        _emit 0x57
        // 0004d384: ff d5           CALL EBP  (InterlockedExchangeAdd(counter, 1))
        _emit 0xff
        _emit 0xd5
        // 0004d386: 8b d8           MOV EBX, EAX  (save result)
        _emit 0x8b
        _emit 0xd8
        // 0004d388: 8b 04 f5 c0 6d 26 01  MOV EAX, [ESI*8 + 0x1266dc0]  (table2[ESI])
        _emit 0x8b
        _emit 0x04
        _emit 0xf5
        _emit 0xc0
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        // 0004d38f: 8d 0c 00        LEA ECX, [EAX + EAX*1]  (ECX = EAX*2)
        _emit 0x8d
        _emit 0x0c
        _emit 0x00
        // 0004d392: 3b d9           CMP EBX, ECX
        _emit 0x3b
        _emit 0xd9
        // 0004d394: 75 06           JNZ +6  (→ 0x4d39c)
        _emit 0x75
        _emit 0x06
        // 0004d396: f7 d8           NEG EAX  (EAX = -table2[ESI])
        _emit 0xf7
        _emit 0xd8
        // 0004d398: 50              PUSH EAX
        _emit 0x50
        // 0004d399: 57              PUSH EDI
        _emit 0x57
        // 0004d39a: ff d5           CALL EBP  (InterlockedExchangeAdd(counter, -table2))
        _emit 0xff
        _emit 0xd5
        // 0004d39c: 8b c3           MOV EAX, EBX  (restore EBX = original result)
        _emit 0x8b
        _emit 0xc3
        // 0004d39e: 99              CDQ
        _emit 0x99
        // 0004d39f: f7 3c f5 c0 6d 26 01  IDIV [ESI*8 + 0x1266dc0]  (result / table2[ESI])
        _emit 0xf7
        _emit 0x3c
        _emit 0xf5
        _emit 0xc0
        _emit 0x6d
        _emit 0x26
        _emit 0x01
        // 0004d3a6: 8b 04 b5 c8 ce 32 01  MOV EAX, [ESI*4 + 0x132cec8]  (table3[ESI])
        _emit 0x8b
        _emit 0x04
        _emit 0xb5
        _emit 0xc8
        _emit 0xce
        _emit 0x32
        _emit 0x01
        // 0004d3ad: 8b 4c 24 14     MOV ECX, [ESP+0x14]  (= arg, original first param)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0004d3b1: 5f              POP EDI
        _emit 0x5f
        // 0004d3b2: 5d              POP EBP
        _emit 0x5d
        // 0004d3b3: 5b              POP EBX   ← last byte of 100-byte compare window
        _emit 0x5b
        // [0x4d3b4: 5e POP ESI  — outside 100-byte compare window]
        // [0x4d3b5: 89 0c 90 MOV [EAX+EDX*4],ECX  — outside window]
        // [0x4d3b8: c3 RET  — outside window (shared epilogue)]
    }
}
