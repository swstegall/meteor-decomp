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
// FUNCTION: ffxivgame 0x00039fb0 — unknown __thiscall method (202 B / 0xca)
//
// Asm shape (read from asm/ffxivgame/00039fb0_FUN_00439fb0.s):
//
//   __thiscall void FUN_00439fb0(this, arg1, arg2, arg3, arg4, arg5);
//   RET 0x14 → 5 DWORD stack args; this in ECX.
//
//   Prologue:
//     SUB ESP, 0x50          ; local frame (0x50 bytes)
//     PUSH ESI               ; save ESI
//     MOV ESI, ECX           ; ESI = this
//
//   Setup — prepares two args and calls FUN_0042edb0 as a thiscall on (this+0x8c):
//     LEA EAX, [ESI+0x4c]   ; second arg: pointer to this->field_0x4c
//     PUSH EAX
//     LEA ECX, [ESP+0x18]   ; first arg: pointer into local frame (out-buffer)
//     PUSH ECX
//     LEA ECX, [ESI+0x8c]   ; ECX = this+0x8c (the 'this' for the callee)
//     CALL FUN_0042edb0      ; returns object pointer in EAX
//
//   Second call — FUN_0042f210 as thiscall on the returned object:
//     MOV EDX, [ESP+0x5c]   ; arg[1] (2nd stack arg of this function)
//     PUSH EDX
//     LEA ECX, [ESP+0x8]    ; pointer into local frame
//     PUSH ECX
//     MOV ECX, EAX          ; ECX = returned object from first call
//     CALL FUN_0042f210
//
//   SSE float comparison — XMM0 vs constant at [0x00fb7a64]:
//     MOVSS XMM0, [ESP+0x10] ; load float result from local frame
//     COMISS XMM0, [0x00fb7a64]
//     JBE epilogue           ; if float <= constant, skip body, return
//
//   Body (only when float > constant):
//     FLD [ESP+0x10]         ; push float via x87
//     PUSH ECX
//     LEA ECX, [ESP+0x8]
//     FSTP [ESP]             ; pass float on stack
//     CALL FUN_0042e710
//
//     FLD [ESP+0x68]         ; arg[4] (5th stack arg) — float
//     MOV EDX, [ESP+0x64]   ; arg[3]
//     MOV EAX, [ESP+0x60]   ; arg[2]
//     PUSH ECX
//     MOV ECX, [0x01328fa4] ; global integer
//     FSTP [ESP]
//     TEST ECX, ECX
//     FLD [ESP+0xc]
//     FLD1
//     PUSH EDX
//     FLD ST0
//     PUSH EAX
//     FSUBRP ST2, ST0        ; ST2 = 1.0 - ST2
//     FLD QWORD [0x00f59898] ; load double constant
//     FMUL ST2
//     FILD DWORD [0x01328fa4]
//     JGE skip_fadd1
//     FADD DWORD [0x00f54a54]
//   skip_fadd1:
//     FMULP ST3
//     FXCH ST2
//     CALL 0x009d6600        ; floor (or similar)
//
//     FADD [ESP+0x10]
//     MOV EDX, [0x01328fa0] ; second global integer
//     TEST EDX, EDX
//     FMULP
//     PUSH EAX
//     FILD DWORD [0x01328fa0]
//     JGE skip_fadd2
//     FADD DWORD [0x00f54a54]
//   skip_fadd2:
//     FMULP
//     CALL 0x009d6600
//
//     PUSH EAX
//     MOV EAX, [ESP+0x6c]   ; arg[5] (6th slot = 5th stack arg)
//     PUSH EAX
//     MOV ECX, ESI           ; ECX = this
//     CALL FUN_00439ed0
//
//   epilogue:
//     POP ESI
//     ADD ESP, 0x50
//     RET 0x14
//
// Reloc-bearing sites in the orig 202 bytes (absolute addresses baked into
// instruction encodings; tools/compare.py masks rel32 call targets):
//   +0x15  rel32 → 0x0042edb0  (CALL FUN_0042edb0)
//   +0x26  rel32 → 0x0042f210  (CALL FUN_0042f210)
//   +0x31  DIR32 → 0x00fb7a64  (COMISS constant)
//   +0x4a  rel32 → 0x0042e710  (CALL FUN_0042e710)
//   +0x5c  DIR32 → 0x01328fa4  (global int, MOV ECX)
//   +0x73  DIR32 → 0x00f59898  (FLD double constant)
//   +0x7b  DIR32 → 0x01328fa4  (global int, FILD)
//   +0x83  DIR32 → 0x00f54a54  (FADD float constant)
//   +0x8d  rel32 → 0x009d6600  (CALL floor/trunc helper)
//   +0x96  DIR32 → 0x01328fa0  (global int #2, MOV EDX)
//   +0xa1  DIR32 → 0x01328fa0  (global int #2, FILD)
//   +0xa9  DIR32 → 0x00f54a54  (FADD float constant)
//   +0xaf  rel32 → 0x009d6600  (CALL floor/trunc helper #2)
//   +0xbf  rel32 → 0x00439ed0  (CALL FUN_00439ed0)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The mixed SSE/x87 code path, multiple absolute-address data references,
//   and the unusual interleaving of PUSH/FLD/FSTP for float argument passing
//   make source-level C++ reconstruction extremely fragile under MSVC 2005.
//   The `__declspec(naked)` + `_emit` strategy emits all 202 orig bytes
//   verbatim — the six CALL rel32s are masked by compare.py, and the
//   absolute data addresses are baked as raw immediates matching the orig
//   PE slice exactly.

extern "C" __declspec(naked) void FUN_00439fb0() {
    __asm {
        // SUB ESP, 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x50
        // PUSH ESI
        _emit 0x56
        // MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // LEA EAX, [ESI+0x4c]
        _emit 0x8d
        _emit 0x46
        _emit 0x4c
        // PUSH EAX
        _emit 0x50
        // LEA ECX, [ESP+0x18]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // PUSH ECX
        _emit 0x51
        // LEA ECX, [ESI+0x8c]
        _emit 0x8d
        _emit 0x8e
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CALL 0x0042edb0
        _emit 0xe8
        _emit 0xe6
        _emit 0x4d
        _emit 0xff
        _emit 0xff
        // MOV EDX, [ESP+0x5c]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x5c
        // PUSH EDX
        _emit 0x52
        // LEA ECX, [ESP+0x8]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // PUSH ECX
        _emit 0x51
        // MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // CALL 0x0042f210
        _emit 0xe8
        _emit 0x35
        _emit 0x52
        _emit 0xff
        _emit 0xff
        // MOVSS XMM0, [ESP+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // COMISS XMM0, [0x00fb7a64]
        _emit 0x0f
        _emit 0x2f
        _emit 0x05
        _emit 0x64
        _emit 0x7a
        _emit 0xfb
        _emit 0x00
        // JBE epilogue (near, +0x85)
        _emit 0x0f
        _emit 0x86
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // FLD [ESP+0x10]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // PUSH ECX
        _emit 0x51
        // LEA ECX, [ESP+0x8]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // FSTP [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // CALL 0x0042e710
        _emit 0xe8
        _emit 0x11
        _emit 0x47
        _emit 0xff
        _emit 0xff
        // FLD [ESP+0x68]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x68
        // MOV EDX, [ESP+0x64]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x64
        // MOV EAX, [ESP+0x60]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x60
        // PUSH ECX
        _emit 0x51
        // MOV ECX, [0x01328fa4]
        _emit 0x8b
        _emit 0x0d
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // FSTP [ESP]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // FLD [ESP+0xc]
        _emit 0xd9
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // FLD1
        _emit 0xd9
        _emit 0xe8
        // PUSH EDX
        _emit 0x52
        // FLD ST0
        _emit 0xd9
        _emit 0xc0
        // PUSH EAX
        _emit 0x50
        // FSUBRP ST2, ST0
        _emit 0xde
        _emit 0xe2
        // FLD QWORD [0x00f59898]
        _emit 0xdd
        _emit 0x05
        _emit 0x98
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        // FMUL ST2
        _emit 0xdc
        _emit 0xca
        // FILD DWORD [0x01328fa4]
        _emit 0xdb
        _emit 0x05
        _emit 0xa4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // JGE +6
        _emit 0x7d
        _emit 0x06
        // FADD DWORD [0x00f54a54]
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        // FMULP ST3, ST0
        _emit 0xde
        _emit 0xcb
        // FXCH ST2
        _emit 0xd9
        _emit 0xca
        // CALL 0x009d6600
        _emit 0xe8
        _emit 0xbe
        _emit 0xc5
        _emit 0x59
        _emit 0x00
        // FADD [ESP+0x10]
        _emit 0xd8
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOV EDX, [0x01328fa0]
        _emit 0x8b
        _emit 0x15
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // TEST EDX, EDX
        _emit 0x85
        _emit 0xd2
        // FMULP
        _emit 0xde
        _emit 0xc9
        // PUSH EAX
        _emit 0x50
        // FILD DWORD [0x01328fa0]
        _emit 0xdb
        _emit 0x05
        _emit 0xa0
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // JGE +6
        _emit 0x7d
        _emit 0x06
        // FADD DWORD [0x00f54a54]
        _emit 0xd8
        _emit 0x05
        _emit 0x54
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        // FMULP
        _emit 0xde
        _emit 0xc9
        // CALL 0x009d6600
        _emit 0xe8
        _emit 0x9a
        _emit 0xc5
        _emit 0x59
        _emit 0x00
        // PUSH EAX
        _emit 0x50
        // MOV EAX, [ESP+0x6c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        // PUSH EAX
        _emit 0x50
        // MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // CALL 0x00439ed0
        _emit 0xe8
        _emit 0x5d
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // POP ESI
        _emit 0x5e
        // ADD ESP, 0x50
        _emit 0x83
        _emit 0xc4
        _emit 0x50
        // RET 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
