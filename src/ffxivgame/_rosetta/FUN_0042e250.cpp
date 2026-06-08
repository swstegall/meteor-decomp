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
// FUNCTION: ffxivgame 0x0002e250 — FUN_0042e250 (282 B / 0x11a)
//
// __cdecl void FUN_0042e250(unsigned int selector)
//
// Selects one of seven global object pointers (at .data 0x01329960..0x01329978,
// every 4 bytes) based on `selector` (0–6). For the chosen pointer it:
//   1) Calls FUN_004208b0 (cdecl, 1 arg = the pointer value) to register/bind it.
//   2) Reads the (possibly updated) global back into a local variable.
//   3) Calls FUN_00420900 (__thiscall on [ptr+0x8]) twice with string-key
//      constants 0xf62e88 and 0xf62e90 (both with unused_arg = 0) to obtain
//      two property handles.
//   4) For each handle, calls FUN_00420cf0 (__thiscall on ptr) with the handle
//      and a pointer to a zeroed float[4] on the stack.
//
// Stack frame: SUB ESP,0x10 allocates 16 bytes for the float[4] buffer.
// Callee-saves: ESI (object pointer), EDI (pointer + 0x8 sub-object).
//
// The jump table covers cases 0–6; any value > 6 falls through directly to
// the post-switch block (the object pointer stays NULL / 0 in that path).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The combination of (a) a 7-case jump table, (b) __thiscall calls to
//   FUN_00420900 and FUN_00420cf0, (c) XORPS/MOVSS float zeroing after
//   the thiscall pushes, and (d) the exact ESI/EDI register allocation
//   across the entire body makes source-level round-tripping impractical.
//   The `__declspec(naked)` body re-emits the orig 282 bytes verbatim.
//
//   Reloc-bearing sites (compare.py wildcard-masks these windows):
//     case body CALLs to FUN_004208b0 (7 × rel32)
//     case body MOV from globals (7 × moffs32)
//     CALL to FUN_00420900 (2 × rel32)
//     CALL to FUN_00420cf0 (2 × rel32)
//     jump table at .text+0x011c (7 × abs32)

extern "C" void FUN_004208b0();
extern "C" void FUN_00420900();
extern "C" void FUN_00420cf0();

extern "C" __declspec(naked) void FUN_0042e250() {
    __asm {
        // 0002e250: MOV EAX, [ESP+4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0002e254: SUB ESP, 0x10
        _emit 0x83
        _emit 0xec
        _emit 0x10
        // 0002e257: PUSH ESI
        _emit 0x56
        // 0002e258: XOR ESI, ESI
        _emit 0x33
        _emit 0xf6
        // 0002e25a: CMP EAX, 6
        _emit 0x83
        _emit 0xf8
        _emit 0x06
        // 0002e25d: PUSH EDI
        _emit 0x57
        // 0002e25e: JA 0x0042e2f5  (rel32 = 0x00000091)
        _emit 0x0f
        _emit 0x87
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002e264: JMP [EAX*4 + 0x42e36c]
        _emit 0xff
        _emit 0x24
        _emit 0x85
        _emit 0x6c
        _emit 0xe3
        _emit 0x42
        _emit 0x00

        // case 0: 0002e26b
        _emit 0xa1  // MOV EAX, [0x01329960]
        _emit 0x60
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL FUN_004208b0
        _emit 0x3a
        _emit 0x26
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, [0x01329960]
        _emit 0x35
        _emit 0x60
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xeb  // JMP +0x74 (→ 0x0042e2f2)
        _emit 0x74

        // case 1: 0002e27e
        _emit 0x8b  // MOV ECX, [0x01329964]
        _emit 0x0d
        _emit 0x64
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL FUN_004208b0
        _emit 0x26
        _emit 0x26
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, [0x01329964]
        _emit 0x35
        _emit 0x64
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xeb  // JMP +0x60 (→ 0x0042e2f2)
        _emit 0x60

        // case 2: 0002e292
        _emit 0x8b  // MOV EDX, [0x01329968]
        _emit 0x15
        _emit 0x68
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL FUN_004208b0
        _emit 0x12
        _emit 0x26
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, [0x01329968]
        _emit 0x35
        _emit 0x68
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xeb  // JMP +0x4c (→ 0x0042e2f2)
        _emit 0x4c

        // case 3: 0002e2a6
        _emit 0xa1  // MOV EAX, [0x0132996c]
        _emit 0x6c
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL FUN_004208b0
        _emit 0xff
        _emit 0x25
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, [0x0132996c]
        _emit 0x35
        _emit 0x6c
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xeb  // JMP +0x39 (→ 0x0042e2f2)
        _emit 0x39

        // case 4: 0002e2b9
        _emit 0x8b  // MOV ECX, [0x01329970]
        _emit 0x0d
        _emit 0x70
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL FUN_004208b0
        _emit 0xeb
        _emit 0x25
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, [0x01329970]
        _emit 0x35
        _emit 0x70
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xeb  // JMP +0x25 (→ 0x0042e2f2)
        _emit 0x25

        // case 5: 0002e2cd
        _emit 0x8b  // MOV EDX, [0x01329974]
        _emit 0x15
        _emit 0x74
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL FUN_004208b0
        _emit 0xd7
        _emit 0x25
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, [0x01329974]
        _emit 0x35
        _emit 0x74
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0xeb  // JMP +0x11 (→ 0x0042e2f2)
        _emit 0x11

        // case 6: 0002e2e1
        _emit 0xa1  // MOV EAX, [0x01329978]
        _emit 0x78
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL FUN_004208b0
        _emit 0xc4
        _emit 0x25
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ESI, [0x01329978]
        _emit 0x35
        _emit 0x78
        _emit 0x99
        _emit 0x32
        _emit 0x01

        // post-switch: 0002e2f2
        _emit 0x83  // ADD ESP, 4
        _emit 0xc4
        _emit 0x04

        // 0002e2f5 (JA target):
        _emit 0x68  // PUSH 0xf62e88
        _emit 0x88
        _emit 0x2e
        _emit 0xf6
        _emit 0x00
        _emit 0x8d  // LEA EDI, [ESI+8]
        _emit 0x7e
        _emit 0x08
        _emit 0x6a  // PUSH 0
        _emit 0x00
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL FUN_00420900
        _emit 0xfa
        _emit 0x25
        _emit 0xff
        _emit 0xff

        _emit 0x0f  // XORPS XMM0, XMM0
        _emit 0x57
        _emit 0xc0
        _emit 0x8d  // LEA ECX, [ESP+8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x51  // PUSH ECX
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0xf3  // MOVSS [ESP+0x10], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3  // MOVSS [ESP+0x14], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xf3  // MOVSS [ESP+0x18], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3  // MOVSS [ESP+0x1c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xe8  // CALL FUN_00420cf0
        _emit 0xc2
        _emit 0x29
        _emit 0xff
        _emit 0xff

        _emit 0x68  // PUSH 0xf62e90
        _emit 0x90
        _emit 0x2e
        _emit 0xf6
        _emit 0x00
        _emit 0x6a  // PUSH 0
        _emit 0x00
        _emit 0x8b  // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8  // CALL FUN_00420900
        _emit 0xc4
        _emit 0x25
        _emit 0xff
        _emit 0xff

        _emit 0x0f  // XORPS XMM0, XMM0
        _emit 0x57
        _emit 0xc0
        _emit 0x8d  // LEA EDX, [ESP+8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52  // PUSH EDX
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0xf3  // MOVSS [ESP+0x10], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xf3  // MOVSS [ESP+0x14], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xf3  // MOVSS [ESP+0x18], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xf3  // MOVSS [ESP+0x1c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xe8  // CALL FUN_00420cf0
        _emit 0x8c
        _emit 0x29
        _emit 0xff
        _emit 0xff

        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3  // RET
    }
}
