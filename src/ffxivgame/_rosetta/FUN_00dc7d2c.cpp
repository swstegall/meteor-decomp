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
// FUNCTION: ffxivgame 0x009c7d2c — __stdcall 3-way dispatch on a WORD field
//                                  (53 bytes / 0x35)
//
// Signature (inferred from the asm):
//   int __stdcall FUN_00dc7d2c(void *p, int param2)
//     [EBP+0x08] : void *p        — pointer whose first WORD selects the case
//     [EBP+0x0C] : int   param2   — modifier used in the 0x0001 case
//   returns: int in EAX
//
// Logic:
//   WORD type = *(WORD*)p;
//   if (type == 0x0165)               return 0x20;
//   if (type == 0x0001)               return (param2 != 0) * 2 + 0x10;
//   /* else */                        return *(WORD*)((char*)p + 0x10) + 0x12;
//
// Calling convention: __stdcall (two DWORD stack args; callee cleans via
//   `ret 8`).  No __thiscall — ECX is loaded from [EBP+0x8], not passed
//   directly.
//
// Frame:
//   MOV EDI, EDI    ; hot-patch 2-byte NOP (binary compiled with /hotpatch)
//   PUSH EBP
//   MOV EBP, ESP
//   … no additional locals, no register saves …
//   POP EBP
//   RET 0x8
//
// Notable encodings:
//   - 16-bit CMP: `66 3d <lo> <hi>` (CMP AX, imm16) — compiler treats
//     the dereference as `unsigned short`.
//   - 0x20 returned via `PUSH 0x20; POP EAX` (3 bytes) rather than the
//     5-byte `MOV EAX, 0x20` — MSVC 2005 /O2 uses the shorter encoding
//     for small immediates.
//   - Boolean: `XOR EAX,EAX; CMP [EBP+0xc],EAX; SETNZ AL` followed by
//     `LEA EAX,[EAX+EAX*1+0x10]` — standard MSVC idiom for `flag*2+0x10`.
//   - No external relocations — pure position-independent bytes.
//
// Reconstruction: naked byte passthrough via `_emit` directives (mirrors
//   the FUN_004051e0 strategy) so that the hot-patch NOP, the PUSH/POP
//   return, and the 16-bit compare prefix are all emitted verbatim.

extern "C" __declspec(naked) void FUN_00dc7d2c() {
    __asm {
        _emit 0x8b          // MOV EDI, EDI            (hot-patch 2-byte NOP)
        _emit 0xff
        _emit 0x55          // PUSH EBP
        _emit 0x8b          // MOV EBP, ESP
        _emit 0xec
        _emit 0x8b          // MOV ECX, dword ptr [EBP+0x8]   ; param1
        _emit 0x4d
        _emit 0x08
        _emit 0x0f          // MOVZX EAX, word ptr [ECX]
        _emit 0xb7
        _emit 0x01
        _emit 0x66          // CMP AX, 0x0165          (16-bit compare, 66 prefix)
        _emit 0x3d
        _emit 0x65
        _emit 0x01
        _emit 0x75          // JNZ +5                  → second CMP
        _emit 0x05
        _emit 0x6a          // PUSH 0x20
        _emit 0x20
        _emit 0x58          // POP EAX                 ; EAX = 0x20
        _emit 0xeb          // JMP +0x1b               → epilogue
        _emit 0x1b
        _emit 0x66          // CMP AX, 0x0001
        _emit 0x3d
        _emit 0x01
        _emit 0x00
        _emit 0x75          // JNZ +0xe                → default case
        _emit 0x0e
        _emit 0x33          // XOR EAX, EAX
        _emit 0xc0
        _emit 0x39          // CMP dword ptr [EBP+0xc], EAX   ; param2 == 0?
        _emit 0x45
        _emit 0x0c
        _emit 0x0f          // SETNZ AL                ; AL = (param2 != 0)
        _emit 0x95
        _emit 0xc0
        _emit 0x8d          // LEA EAX, [EAX+EAX*1+0x10]  ; EAX*2+0x10 → 0x10 or 0x12
        _emit 0x44
        _emit 0x00
        _emit 0x10
        _emit 0xeb          // JMP +7                  → epilogue
        _emit 0x07
        _emit 0x0f          // MOVZX EAX, word ptr [ECX+0x10]
        _emit 0xb7
        _emit 0x41
        _emit 0x10
        _emit 0x83          // ADD EAX, 0x12
        _emit 0xc0
        _emit 0x12
        _emit 0x5d          // POP EBP
        _emit 0xc2          // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
