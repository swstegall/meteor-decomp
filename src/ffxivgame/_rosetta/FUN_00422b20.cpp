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
// FUNCTION: ffxivgame 0x00022b20 — class constructor (355 B / 0x163),
//                                  `__thiscall void()`, SEH4-wrapped.
//
// Inspection (read from disassembly at orig RVA 0x00022b20):
//
//   __thiscall void SomeClass::ctor(this) — ECX = this; returns void.
//
//   Structure:
//     1. SEH4 prologue: PUSH -1 / PUSH 0xe55d5e / PUSH FS:[0] /
//        PUSH ECX / PUSH ESI / PUSH EDI / XOR cookie^ESP / PUSH cookie /
//        LEA EAX,[ESP+0x10] / MOV FS:[0],EAX
//     2. ESI = this; save this at [ESP+0xc]; set vtable: [this] = 0xf59a64
//     3. Set SEH state to 0x9: MOV dword [ESP+0x18], 9
//     4. EDI = 2; loop: MOV ECX,ESI / CALL FUN_00422370 (sub-object ctor),
//        SUB EDI,1 / JNZ (calls twice total)
//     5. Read global [0x0132991c] → save to [0x01329920]
//     6. Call FUN_009d1c4c (4-arg __stdcall init) 10 times on members
//        at offsets 0x16c, 0x144, 0x11c, 0xf4, 0xcc, 0xa4, 0x7c, 0x54, 0x2c,
//        and (this+4), with the string at 0xc51ab0, args 0x14/0x2.
//        Between each call the SEH state slot [ESP+0x28] is updated
//        (8, 7, 6, 5, 4, 3, 2, 1, 0, -1) for partial-unwind safety.
//     7. SEH4 epilogue: restore FS:[0], POP ECX/EDI/ESI, ADD ESP,0x10, RET
//
// Reloc-bearing sites (absolute VA immediates in the 355-byte body):
//     +0x02  DIR32 → 0x00e55d5e  (PUSH EH handler)
//     +0x07  DIR32 → FS:[0] (constant 0)
//     +0x11  DIR32 → 0x012ea8b0  (__security_cookie)
//     +0x19  DIR32 → FS:[0] (MOV FS:[0], EAX)
//     +0x29  DIR32 → 0x00f59a64  (vtable, MOV dword [ESI])
//     +0x43  REL32 → 0x00022370  (CALL FUN_00422370)
//     +0x4c  DIR32 → 0x0132991c  (global read)
//     +0x51  DIR32 → 0x00c51ab0  (init string)
//     +0x5a  DIR32 → this+0x16c  (ModRM+SIB+disp32)
//     +0x60  DIR32 → 0x01329920  (global write)
//     +0x6b  REL32 → 0x005d1c4c  (CALL FUN_009d1c4c — 1st)
//     ... (repeat for each of the 10 calls)
//     +0x151 DIR32 → FS:[0] (MOV FS:[0], ECX restore)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   Source-level C++ here would need MSVC 2005 /O2 to reproduce the exact
//   SEH4 prologue layout (cookie placement, ECX save ordering, state-slot
//   scheduling around the loop), the precise register allocation cycling
//   ECX/EDX/EAX for the 10 member pushes, and all linker-resolved
//   absolute addresses. This is the same pragmatic approach taken by
//   FUN_00405080, FUN_0040ced0, and FUN_004014b0 for the same reasons.

extern "C" __declspec(naked) void FUN_00422b20() {
    __asm {
        // SEH4 prologue
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe55d5e (EH handler)
        _emit 0x5e
        _emit 0x5d
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX (cookie^ESP)
        _emit 0x8d  // LEA EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // body
        _emit 0x8b  // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89  // MOV [ESP+0xc], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0xc7  // MOV dword [ESI], 0xf59a64
        _emit 0x06
        _emit 0x64
        _emit 0x9a
        _emit 0xf5
        _emit 0x00
        _emit 0xc7  // MOV dword [ESP+0x18], 9
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xbf  // MOV EDI, 2
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA ESP, [ESP] (4-byte NOP)
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // loop: call FUN_00422370 twice
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8  // CALL FUN_00422370
        _emit 0x09
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x83  // SUB EDI, 1
        _emit 0xef
        _emit 0x01
        _emit 0x75  // JNZ loop
        _emit 0xf4
        // call 1: member at [this+0x16c], state=8
        _emit 0xa1  // MOV EAX, [0x0132991c]
        _emit 0x1c
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA ECX, [ESI+0x16c]
        _emit 0x8e
        _emit 0x6c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x51  // PUSH ECX
        _emit 0xa3  // MOV [0x01329920], EAX
        _emit 0x20
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xc6  // MOV byte [ESP+0x28], 8
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x08
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0xbc
        _emit 0xf0
        _emit 0x5a
        _emit 0x00
        // call 2: member at [this+0x144], state=7
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA EDX, [ESI+0x144]
        _emit 0x96
        _emit 0x44
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x52  // PUSH EDX
        _emit 0xc6  // MOV byte [ESP+0x28], 7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x07
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0xa2
        _emit 0xf0
        _emit 0x5a
        _emit 0x00
        // call 3: member at [this+0x11c], state=6
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA EAX, [ESI+0x11c]
        _emit 0x86
        _emit 0x1c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0xc6  // MOV byte [ESP+0x28], 6
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x06
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0x88
        _emit 0xf0
        _emit 0x5a
        _emit 0x00
        // call 4: member at [this+0xf4], state=5
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA ECX, [ESI+0xf4]
        _emit 0x8e
        _emit 0xf4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51  // PUSH ECX
        _emit 0xc6  // MOV byte [ESP+0x28], 5
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x05
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0x6e
        _emit 0xf0
        _emit 0x5a
        _emit 0x00
        // call 5: member at [this+0xcc], state=4
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA EDX, [ESI+0xcc]
        _emit 0x96
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x52  // PUSH EDX
        _emit 0xc6  // MOV byte [ESP+0x28], 4
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x04
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0x54
        _emit 0xf0
        _emit 0x5a
        _emit 0x00
        // call 6: member at [this+0xa4], state=3
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA EAX, [ESI+0xa4]
        _emit 0x86
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0xc6  // MOV byte [ESP+0x28], 3
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x03
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0x3a
        _emit 0xf0
        _emit 0x5a
        _emit 0x00
        // call 7: member at [this+0x7c], state=2
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA ECX, [ESI+0x7c]
        _emit 0x4e
        _emit 0x7c
        _emit 0x51  // PUSH ECX
        _emit 0xc6  // MOV byte [ESP+0x28], 2
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x02
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0x23
        _emit 0xf0
        _emit 0x5a
        _emit 0x00
        // call 8: member at [this+0x54], state=1
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA EDX, [ESI+0x54]
        _emit 0x56
        _emit 0x54
        _emit 0x52  // PUSH EDX
        _emit 0xc6  // MOV byte [ESP+0x28], 1
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x01
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0x0c
        _emit 0xf0
        _emit 0x5a
        _emit 0x00
        // call 9: member at [this+0x2c], state=0
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x8d  // LEA EAX, [ESI+0x2c]
        _emit 0x46
        _emit 0x2c
        _emit 0x50  // PUSH EAX
        _emit 0xc6  // MOV byte [ESP+0x28], 0
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x00
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0xf5
        _emit 0xef
        _emit 0x5a
        _emit 0x00
        // call 10: member at [this+4], state=-1
        _emit 0x68  // PUSH 0xc51ab0
        _emit 0xb0
        _emit 0x1a
        _emit 0xc5
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x14
        _emit 0x14
        _emit 0x83  // ADD ESI, 4
        _emit 0xc6
        _emit 0x04
        _emit 0x56  // PUSH ESI
        _emit 0xc7  // MOV dword [ESP+0x28], -1
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL FUN_009d1c4c
        _emit 0xdb
        _emit 0xef
        _emit 0x5a
        _emit 0x00
        // SEH4 epilogue
        _emit 0x8b  // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3  // RET
    }
}
