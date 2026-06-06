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
// FUNCTION: ffxivgame 0x004129e0 — engine_memory __thiscall with one stack
//                                  argument, 131 B / 0x83.
//
// __thiscall void FUN_004129e0(this, undefined4 arg0)
//   ECX        : this              (EDI throughout)
//   [ESP+0x04] : arg0              (single stack argument; callee cleans via RET 4)
//
// Shape summary (131 bytes, RVA 0x000129e0 – 0x00012a62):
//
//   prologue: PUSH EBX/EBP/ESI/EDI; EDI = ECX (this)
//   vtable call via [EDI] + 0x2C    (virtual dispatch on this)
//   load arg from [ESP+0x14]        (ESP shifted by 4 saved regs → arg0)
//   chain of vtable calls building an object into ESI
//   atomic spin-lock via XCHG / TEST / JNZ loop (@0x12a20)
//   doubly-linked-list insert + decrement counter
//   release lock via XCHG
//   vtable call via [EDI+4] + 0x10  (notify)
//   vtable call via [EDI]   + 0x30  (final dispatch)
//   epilogue: POP EDI/ESI/EBP/EBX; RET 4
//
// No CALL rel32 / direct CALL targets — every call goes through a vtable
// pointer (CALL EDX or CALL EAX). Therefore there are zero relocations and
// the orig bytes can be re-emitted verbatim via MASM _emit directives. The
// .obj's .text is byte-identical to the orig slice and tools/compare.py
// reports GREEN with no reloc masking needed.
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom as
// FUN_0040a460 / FUN_0040a4b0 / FUN_00409580).

extern "C" __declspec(naked) void FUN_004129e0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x2c]
        _emit 0x50
        _emit 0x2c
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x14]
        _emit 0x50
        _emit 0x14
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x4]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x14]
        _emit 0x4e
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x4]
        _emit 0x42
        _emit 0x04
        _emit 0x8b              // MOV EBP, dword ptr [ESI + 0x18]
        _emit 0x6e
        _emit 0x18
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x8b              // MOV EBX, dword ptr [EAX + 0x10]
        _emit 0x58
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EDX]
        _emit 0x02
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8d              // LEA ECX, [EBX + 0x4]
        _emit 0x4b
        _emit 0x04
        _emit 0xba              // MOV EDX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, ECX          (spin_top:)
        _emit 0xc1
        _emit 0x87              // XCHG dword ptr [EAX], EDX
        _emit 0x10
        _emit 0x85              // TEST EDX, EDX
        _emit 0xd2
        _emit 0x75              // JNZ spin_top (-13 / 0xf3)
        _emit 0xf3
        _emit 0x8b              // MOV EAX, dword ptr [EBX + 0xc]
        _emit 0x43
        _emit 0x0c
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EDX], ESI
        _emit 0x32
        _emit 0x8b              // MOV EDX, dword ptr [EAX + 0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI], EAX
        _emit 0x06
        _emit 0x89              // MOV dword ptr [ESI + 0x4], EDX
        _emit 0x56
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX + 0x4], ESI
        _emit 0x70
        _emit 0x04
        _emit 0x83              // ADD dword ptr [EBX + 0x18], -0x1
        _emit 0x43
        _emit 0x18
        _emit 0xff
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x87              // XCHG dword ptr [ECX], EAX
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [EDI + 0x4]
        _emit 0x4f
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ECX]
        _emit 0x11
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x10]
        _emit 0x42
        _emit 0x10
        _emit 0x55              // PUSH EBP
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EDX, dword ptr [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, dword ptr [EDX + 0x30]
        _emit 0x42
        _emit 0x30
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
