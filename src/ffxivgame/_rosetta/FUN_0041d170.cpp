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
// FUNCTION: ffxivgame 0x0001d170 — three-call attribute setter for a global
//           object, dispatching table-indexed values from two parallel arrays
//           (__cdecl, 100 B / 0x64)
//
// __cdecl void FUN_0041d170(int arg0, int arg1, int arg2, int arg3)
//
// Calls FUN_004236e0 (a __thiscall setter on the object at [0x132987c])
// three times, each time passing:
//   - a "key"   drawn from the array at 0xf597dc/0xf597e4/0xf597ec
//               indexed by arg0  (stride-8 sequence; three consecutive arrays)
//   - a "value" drawn from the common array at 0xf59730
//               indexed by arg1, arg2, and arg3 respectively
//
// Calling convention analysis:
//   - Outer: __cdecl (plain `ret`; no stack cleanup)
//   - FUN_004236e0: __thiscall (ECX = `this`, two stack args; callee cleans
//     8 bytes — confirmed by [esp+0x10]/[esp+0x14] accesses after each call
//     with only `push esi` (-4) on the outer frame)
//
// Stack frame (no EBP frame, no /GS cookie — ESP-relative throughout):
//   [entry_esp + 0x00]  return address
//   [entry_esp + 0x04]  arg0   (index into 0xf597dc/0xf597e4/0xf597ec arrays)
//   [entry_esp + 0x08]  arg1   (index into 0xf59730 for first call)
//   [entry_esp + 0x0c]  arg2   (index into 0xf59730 for second call)
//   [entry_esp + 0x10]  arg3   (index into 0xf59730 for third call)
//
// MSVC 2005 register-allocation note:
//   ESI is saved/restored; arg1 is loaded into EAX first (before `push esi`)
//   so the compiler can re-index 0xf59730 immediately while arg0 has not yet
//   displaced the stack offset.  After `push esi`, ESI = arg0, EAX is reloaded
//   from [esp+0x10/0x14] for subsequent calls.

extern "C" __declspec(naked) void FUN_0041d170() {
    __asm {
        // 0001d170: 8b 44 24 08           MOV EAX, [ESP+0x8]             ; arg1
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001d174: 8b 0c 85 30 97 f5 00  MOV ECX, [EAX*4+0xf59730]     ; g_table0[arg1]
        _emit 0x8b
        _emit 0x0c
        _emit 0x85
        _emit 0x30
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // 0001d17b: 56                    PUSH ESI
        _emit 0x56
        // 0001d17c: 8b 74 24 08           MOV ESI, [ESP+0x8]             ; arg0 (after push)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0001d180: 8b 14 b5 dc 97 f5 00  MOV EDX, [ESI*4+0xf597dc]     ; g_table1[arg0]
        _emit 0x8b
        _emit 0x14
        _emit 0xb5
        _emit 0xdc
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // 0001d187: 51                    PUSH ECX                       ; push g_table0[arg1]
        _emit 0x51
        // 0001d188: 8b 0d 7c 98 32 01     MOV ECX, [0x132987c]           ; this pointer
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d18e: 52                    PUSH EDX                       ; push g_table1[arg0]
        _emit 0x52
        // 0001d18f: e8 4c 65 00 00        CALL 0x4236e0                  ; FUN_004236e0
        _emit 0xe8
        _emit 0x4c
        _emit 0x65
        _emit 0x00
        _emit 0x00
        // 0001d194: 8b 44 24 10           MOV EAX, [ESP+0x10]            ; arg2
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0001d198: 8b 0c 85 30 97 f5 00  MOV ECX, [EAX*4+0xf59730]     ; g_table0[arg2]
        _emit 0x8b
        _emit 0x0c
        _emit 0x85
        _emit 0x30
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // 0001d19f: 8b 14 b5 e4 97 f5 00  MOV EDX, [ESI*4+0xf597e4]     ; g_table2[arg0]
        _emit 0x8b
        _emit 0x14
        _emit 0xb5
        _emit 0xe4
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // 0001d1a6: 51                    PUSH ECX                       ; push g_table0[arg2]
        _emit 0x51
        // 0001d1a7: 8b 0d 7c 98 32 01     MOV ECX, [0x132987c]           ; this pointer
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d1ad: 52                    PUSH EDX                       ; push g_table2[arg0]
        _emit 0x52
        // 0001d1ae: e8 2d 65 00 00        CALL 0x4236e0                  ; FUN_004236e0
        _emit 0xe8
        _emit 0x2d
        _emit 0x65
        _emit 0x00
        _emit 0x00
        // 0001d1b3: 8b 44 24 14           MOV EAX, [ESP+0x14]            ; arg3
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001d1b7: 8b 0c 85 30 97 f5 00  MOV ECX, [EAX*4+0xf59730]     ; g_table0[arg3]
        _emit 0x8b
        _emit 0x0c
        _emit 0x85
        _emit 0x30
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // 0001d1be: 8b 14 b5 ec 97 f5 00  MOV EDX, [ESI*4+0xf597ec]     ; g_table3[arg0]
        _emit 0x8b
        _emit 0x14
        _emit 0xb5
        _emit 0xec
        _emit 0x97
        _emit 0xf5
        _emit 0x00
        // 0001d1c5: 51                    PUSH ECX                       ; push g_table0[arg3]
        _emit 0x51
        // 0001d1c6: 8b 0d 7c 98 32 01     MOV ECX, [0x132987c]           ; this pointer
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d1cc: 52                    PUSH EDX                       ; push g_table3[arg0]
        _emit 0x52
        // 0001d1cd: e8 0e 65 00 00        CALL 0x4236e0                  ; FUN_004236e0
        _emit 0xe8
        _emit 0x0e
        _emit 0x65
        _emit 0x00
        _emit 0x00
        // 0001d1d2: 5e                    POP ESI
        _emit 0x5e
        // 0001d1d3: c3                    RET
        _emit 0xc3
    }
}
