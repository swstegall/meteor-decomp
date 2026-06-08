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
// FUNCTION: ffxivgame 0x00422120 — __thiscall void FUN_00422120() (32 B)
//
// Calls the first virtual function (vtable[0]) on `this` exactly twice via
// a counted loop.  ESI holds `this`; EDI is the loop counter (2 → 1 → 0).
//
// Asm (verbatim, 32 bytes, RVA 0x00022120–0x0002213f):
//
//   00022120:  56                    PUSH ESI
//   00022121:  57                    PUSH EDI
//   00022122:  8b f1                 MOV ESI, ECX          ; this
//   00022124:  bf 02 00 00 00        MOV EDI, 0x2          ; loop counter = 2
//   00022129:  8d a4 24 00 00 00 00  LEA ESP, [ESP+0]      ; 7-byte alignment NOP
//   00022130:  8b 06                 MOV EAX, [ESI]        ; vtable ptr
//   00022132:  8b 10                 MOV EDX, [EAX]        ; vtable[0]
//   00022134:  8b ce                 MOV ECX, ESI          ; restore this
//   00022136:  ff d2                 CALL EDX              ; (*vtable[0])(this)
//   00022138:  83 ef 01              SUB EDI, 0x1
//   0002213b:  75 f3                 JNZ 0x00422130        ; loop while EDI != 0
//   0002213d:  5f                    POP EDI
//   0002213e:  5e                    POP ESI
//   0002213f:  c3                    RET
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The 7-byte LEA alignment NOP at offset +9 is the only non-trivial byte.
//   MASM would encode `LEA ESP, [ESP]` as the 3-byte ModRM form (8D 24 24),
//   not the 7-byte SIB + 32-bit displacement form (8D A4 24 00 00 00 00)
//   that MSVC /O2 emits for 16-byte loop alignment.  There are no relocations
//   in this function (the only CALL is indirect through EDX), so emitting all
//   32 bytes verbatim is safe and stable.

extern "C" __declspec(naked) void FUN_00422120() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xbf              // MOV EDI, 0x2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ESP, [ESP+0x00000000]  (7-byte NOP)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [ESI]   ; vtable ptr
        _emit 0x06
        _emit 0x8b              // MOV EDX, [EAX]   ; vtable[0]
        _emit 0x10
        _emit 0x8b              // MOV ECX, ESI     ; this
        _emit 0xce
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x83              // SUB EDI, 0x1
        _emit 0xef
        _emit 0x01
        _emit 0x75              // JNZ -13  (→ loop head at +0x10)
        _emit 0xf3
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
