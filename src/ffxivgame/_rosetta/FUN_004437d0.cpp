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
// FUNCTION: ffxivgame 0x000437d0 — quest flag-conditional virtual dispatch
//                                  (__thiscall, 3 stack args, 40 B / 0x28)
//
// Guards a 4-arg virtual call (vtable slot 5) on the object at arg2 behind
// an early-exit byte flag. Returns the flag regardless of whether the call
// was made.
//
// High-level logic:
//   bool FUN_004437d0(bool flag, SomeObj* obj, int param):
//       if (!flag)
//           obj->vtable[5](this, param, 0, 0);
//       return flag;
//
// Calling convention: __thiscall — ECX = this (outer), 3 stack args, RET 0xC.
// Frame: none (/Oy). ESI saved/restored INSIDE the false-flag branch only
// (/O2 deferred-save idiom), EBX saved at function entry for byte-register
// access to the flag arg.
//
// No relocations — all calls are indirect through registers. Encoded as
// __declspec(naked) to preserve the deferred-ESI-save layout exactly.
//
// Asm (40 bytes @ orig RVA 0x000437d0):
//   53                  PUSH EBX
//   8a 5c 24 08         MOV BL, byte ptr [ESP+0x8]    ; flag
//   84 db               TEST BL, BL
//   75 19               JNZ short -> epilogue
//   8b 44 24 0c         MOV EAX, dword ptr [ESP+0xc]  ; obj
//   8b 10               MOV EDX, dword ptr [EAX]       ; vtable
//   56                  PUSH ESI                       ; deferred save
//   8b 74 24 14         MOV ESI, dword ptr [ESP+0x14]  ; param
//   6a 00               PUSH 0x0
//   6a 00               PUSH 0x0
//   56                  PUSH ESI
//   51                  PUSH ECX                       ; outer this
//   8b c8               MOV ECX, EAX                   ; this = obj
//   8b 42 14            MOV EAX, dword ptr [EDX+0x14]  ; vtable[5]
//   ff d0               CALL EAX
//   5e                  POP ESI
//   8a c3               MOV AL, BL                     ; return flag
//   5b                  POP EBX
//   c2 0c 00            RET 0xc

extern "C" __declspec(naked) void FUN_004437d0()
{
    __asm {
        // 000437d0: 53                PUSH EBX
        _emit 0x53
        // 000437d1: 8a 5c 24 08       MOV BL, byte ptr [ESP+0x8]
        _emit 0x8a
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 000437d5: 84 db             TEST BL, BL
        _emit 0x84
        _emit 0xdb
        // 000437d7: 75 19             JNZ +0x19
        _emit 0x75
        _emit 0x19
        // 000437d9: 8b 44 24 0c       MOV EAX, dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 000437dd: 8b 10             MOV EDX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 000437df: 56                PUSH ESI
        _emit 0x56
        // 000437e0: 8b 74 24 14       MOV ESI, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 000437e4: 6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000437e6: 6a 00             PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 000437e8: 56                PUSH ESI
        _emit 0x56
        // 000437e9: 51                PUSH ECX
        _emit 0x51
        // 000437ea: 8b c8             MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 000437ec: 8b 42 14          MOV EAX, dword ptr [EDX+0x14]
        _emit 0x8b
        _emit 0x42
        _emit 0x14
        // 000437ef: ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000437f1: 5e                POP ESI
        _emit 0x5e
        // 000437f2: 8a c3             MOV AL, BL
        _emit 0x8a
        _emit 0xc3
        // 000437f4: 5b                POP EBX
        _emit 0x5b
        // 000437f5: c2 0c 00          RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
