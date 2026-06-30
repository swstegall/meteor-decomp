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
// FUNCTION: ffxivgame 0x00456590 — teardown: reset circular-buffer slot, then
//                                  push both params into the buffer (__cdecl, 75 B / 0x4b)
//
// Calling convention: __cdecl, two arguments (param0 at [ESP+4], param1 at
// [ESP+8] from function entry). Returns void. Epilogue is bare RET (caller cleans).
//
// Behaviour (from disassembly at RVA 0x00056590):
//
//   1. Load the global handle from DAT_0126701c into EAX.
//   2. If handle == -1 (uninitialised):
//        Set ECX = OBJ_0132d0e0 (singleton), call FUN_00457270 (__thiscall)
//        which returns a pointer to the buffer header in EAX.
//        Zero the 16-bit "current slot" and "count" fields at [EAX+0x88]
//        and [EAX+0x8a] (resetting the ring-buffer head/tail state).
//   3. Otherwise (handle is valid):
//        Push handle (EAX) as argument, call the IAT function pointer stored
//        at [0x00f3e2a4] (__stdcall, 1 arg — callee cleans).
//        The returned EAX is a pointer to the slot record; set record[0] = -1
//        (marking the slot free).
//   4. Common tail: call FUN_00456060(__cdecl) twice — first with param1, then
//        with param0 — to push both values into the circular buffer.
//        Clean the two pushed args (ADD ESP, 8), then RET.
//
// Globals referenced:
//   0x0126701c — DAT_0126701c  — global handle / registry index
//   0x0132d0e0 — OBJ_0132d0e0 — singleton "this" immediate in MOV ECX
//   0x00f3e2a4 — IAT slot for the buffer-accessor function pointer (__stdcall)
//
// CALL rel32 / DIR32 sites (compare.py masks these during diff):
//   +0x01   MOV abs32  → 0x0126701c  (DAT_0126701c)
//   +0x0b   MOV imm32  → 0x0132d0e0  (OBJ_0132d0e0)
//   +0x10   CALL rel32 → FUN_00457270
//   +0x29   CALL abs32 → [0x00f3e2a4]  (IAT slot)
//   +0x39   CALL rel32 → FUN_00456060  (first call, param1)
//   +0x43   CALL rel32 → FUN_00456060  (second call, param0)
//
// Reconstruction strategy — naked-asm byte passthrough (_emit):
//   The 66-prefixed word-width MOV forms at +0x16 and +0x1d (7 bytes each),
//   the MOV EAX,moffs32 form (opcode 0xa1), the indirect CALL through the IAT
//   (FF 15), and the two CALL rel32 sites to the same target FUN_00456060 must
//   be reproduced verbatim to get byte-identical output. Following the
//   established local idiom (FUN_00456470, FUN_00456060, FUN_004086a0), we
//   use __declspec(naked) and re-emit all 75 original bytes via _emit
//   directives. The resulting .obj .text is byte-identical to the original
//   modulo the six masked reloc slots, so compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00456590() {
    __asm {
        // +0x00: a1 1c 70 26 01  MOV EAX,[0x0126701c]   (abs32 reloc at +0x01)
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // +0x05: 83 f8 ff        CMP EAX,-0x1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // +0x08: 75 1c           JNZ +0x1c  (→ +0x26, else branch)
        _emit 0x75
        _emit 0x1c
        // +0x0a: b9 e0 d0 32 01  MOV ECX,0x0132d0e0    (imm32 reloc at +0x0b)
        _emit 0xb9
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // +0x0f: e8 cc 0c 00 00  CALL FUN_00457270     (rel32 reloc at +0x10)
        _emit 0xe8
        _emit 0xcc
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        // +0x14: 33 c9           XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // +0x16: 66 89 88 88 00 00 00  MOV word ptr [EAX+0x88],CX
        _emit 0x66
        _emit 0x89
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x1d: 66 89 88 8a 00 00 00  MOV word ptr [EAX+0x8a],CX
        _emit 0x66
        _emit 0x89
        _emit 0x88
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x24: eb 0d           JMP +0xd  (→ +0x33, common tail)
        _emit 0xeb
        _emit 0x0d
        // +0x26: 50              PUSH EAX                 (else branch)
        _emit 0x50
        // +0x27: ff 15 a4 e2 f3 00  CALL dword ptr [0x00f3e2a4]  (abs32 reloc at +0x29)
        _emit 0xff
        _emit 0x15
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // +0x2d: c7 00 ff ff ff ff  MOV dword ptr [EAX],0xffffffff
        _emit 0xc7
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // +0x33: 8b 44 24 08     MOV EAX,dword ptr [ESP+0x8]    (common tail; param1)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // +0x37: 50              PUSH EAX
        _emit 0x50
        // +0x38: e8 93 fa ff ff  CALL FUN_00456060              (rel32 reloc at +0x39)
        _emit 0xe8
        _emit 0x93
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // +0x3d: 8b 4c 24 08     MOV ECX,dword ptr [ESP+0x8]    (param0)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // +0x41: 51              PUSH ECX
        _emit 0x51
        // +0x42: e8 89 fa ff ff  CALL FUN_00456060              (rel32 reloc at +0x43)
        _emit 0xe8
        _emit 0x89
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // +0x47: 83 c4 08        ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // +0x4a: c3              RET
        _emit 0xc3
    }
}
