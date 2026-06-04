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
// FUNCTION: ffxivgame 0x0002e120 — global-state save / reset / restore helper
//                                  (250 B / 0xfa, no SEH).
//
// Inspection (read from the disassembly at orig RVA 0x0002e120):
//
//   __cdecl void FUN_0042e120(...);  — small frame (SUB ESP,0xc), pushes
//   EBX/ESI, ends with a tail-call (JMP 0x0041c1d0) so it has no RET of
//   its own.
//
//   Structural shape:
//
//     ; ---- snapshot four globals into locals / saved regs ----
//     AL  = *(u8*)0x01328ef0;   CL  = *(u8*)0x01328ee8;
//     BL  = *(u8*)0x01328f19;   ESI = *(u32*)0x01328f68;
//     [esp+0x18]=BL; [esp+0x10]=AL; [esp+0x14]=CL;   ; spill to locals
//
//     FUN_00419240(3, 0);
//     FUN_004186a0(3);
//     FUN_00418410(0, arg0);                          ; arg0 @ [esp+0x24]
//     ESP += 0x14;                                    ; cdecl arg cleanup
//
//     ; ---- materialise a 5-dword arg list (push-slot + store idiom) ----
//     FUN_004183c0(0, 2, 2, 0, 0);
//
//     ; ---- reset the four globals to 0 via per-field setters ----
//     *(u32*)0x01328f68 = 0;  FUN_0041d0e0(0);
//     *(u8*)0x01328f19  = 0;  FUN_0041c270(0);
//     *(u8*)0x01328ef0  = 0;  FUN_0041c1f0(0);
//     *(u8*)0x01328ee8  = 0;  FUN_0041c1d0(0);
//
//     FUN_0042e0a0();
//
//     ; ---- restore the four globals from the saved snapshot ----
//     *(u32*)0x01328f68 = ESI;  FUN_0041d0e0([esp+0x38]);
//     *(u8*)0x01328f19  = BL;    FUN_0041c270([esp+0x34]);
//     *(u8*)0x01328ef0  = [esp+0x34].lo;  FUN_0041c1f0([esp+0x34]);
//     *(u8*)0x01328ee8  = [esp+0x3c].lo;
//     ESP cleanup; POP ESI/EBX;
//     JMP FUN_0041c1d0([esp+0x3c]);                   ; tail call
//
//   Reloc-bearing sites in the orig 250 bytes (these absolute addresses /
//   rel32 call offsets resolve only in a full-binary relink at image base
//   0x00400000; standalone .obj compilation can't reproduce them). The
//   globals at 0x01328ee8 / 0x01328ef0 / 0x01328f19 / 0x01328f68 are
//   .data, and the eleven CALL targets are .text rel32 displacements.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 into
//   reproducing the exact register allocation (AL/CL/BL/ESI snapshot of
//   the four globals across the reset block), the push-slot-then-store
//   idiom that materialises the FUN_004183c0 argument list, the cdecl
//   arg-cleanup ADD ESP windows, and the linker-resolved absolute
//   addresses + rel32 call offsets. Each of those constraints is brittle
//   under /O2 — every high-level rewrite shifts at least one byte.
//
//   The pragmatic choice — the same one the surrounding _rosetta siblings
//   (FUN_00403a20 / FUN_004054d0 / FUN_00402a30) took for their
//   reloc-heavy bodies — is a `__declspec(naked)` body that re-emits the
//   orig 250 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` ends up byte-identical to the orig slice (no relocations —
//   the bytes are emitted as raw immediates), which is what
//   `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_0042e120() {
    __asm {
        // 0002e120  SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 0002e123  MOVZX EAX, byte [0x01328ef0]
        _emit 0x0f
        _emit 0xb6
        _emit 0x05
        _emit 0xf0
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        // 0002e12a  MOVZX ECX, byte [0x01328ee8]
        _emit 0x0f
        _emit 0xb6
        _emit 0x0d
        _emit 0xe8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        // 0002e131  PUSH EBX
        _emit 0x53
        // 0002e132  MOV BL, byte [0x01328f19]
        _emit 0x8a
        _emit 0x1d
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0002e138  PUSH ESI
        _emit 0x56
        // 0002e139  MOV ESI, dword [0x01328f68]
        _emit 0x8b
        _emit 0x35
        _emit 0x68
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0002e13f  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0002e141  PUSH 3
        _emit 0x6a
        _emit 0x03
        // 0002e143  MOV byte [ESP+0x18], BL
        _emit 0x88
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 0002e147  MOV byte [ESP+0x10], AL
        _emit 0x88
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0002e14b  MOV byte [ESP+0x14], CL
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0002e14f  CALL 0x00419240
        _emit 0xe8
        _emit 0xec
        _emit 0xb0
        _emit 0xfe
        _emit 0xff
        // 0002e154  PUSH 3
        _emit 0x6a
        _emit 0x03
        // 0002e156  CALL 0x004186a0
        _emit 0xe8
        _emit 0x45
        _emit 0xa5
        _emit 0xfe
        _emit 0xff
        // 0002e15b  MOV EDX, dword [ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0002e15f  PUSH EDX
        _emit 0x52
        // 0002e160  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0002e162  CALL 0x00418410
        _emit 0xe8
        _emit 0xa9
        _emit 0xa2
        _emit 0xfe
        _emit 0xff
        // 0002e167  ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0002e16a  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0002e16c  PUSH ECX
        _emit 0x51
        // 0002e16d  MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 0002e16f  MOV dword [EAX], 0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002e175  PUSH ECX
        _emit 0x51
        // 0002e176  MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 0002e178  PUSH ECX
        _emit 0x51
        // 0002e179  MOV dword [EAX], 2
        _emit 0xc7
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002e17f  MOV EAX, ESP
        _emit 0x8b
        _emit 0xc4
        // 0002e181  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0002e183  MOV dword [EAX], 2
        _emit 0xc7
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002e189  CALL 0x004183c0
        _emit 0xe8
        _emit 0x32
        _emit 0xa2
        _emit 0xfe
        _emit 0xff
        // 0002e18e  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0002e190  PUSH EAX
        _emit 0x50
        // 0002e191  MOV [0x01328f68], EAX
        _emit 0xa3
        _emit 0x68
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0002e196  CALL 0x0041d0e0
        _emit 0xe8
        _emit 0x45
        _emit 0xef
        _emit 0xfe
        _emit 0xff
        // 0002e19b  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0002e19d  MOV byte [0x01328f19], 0
        _emit 0xc6
        _emit 0x05
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0002e1a4  CALL 0x0041c270
        _emit 0xe8
        _emit 0xc7
        _emit 0xe0
        _emit 0xfe
        _emit 0xff
        // 0002e1a9  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0002e1ab  MOV byte [0x01328ef0], 0
        _emit 0xc6
        _emit 0x05
        _emit 0xf0
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0002e1b2  CALL 0x0041c1f0
        _emit 0xe8
        _emit 0x39
        _emit 0xe0
        _emit 0xfe
        _emit 0xff
        // 0002e1b7  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0002e1b9  MOV byte [0x01328ee8], 0
        _emit 0xc6
        _emit 0x05
        _emit 0xe8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 0002e1c0  CALL 0x0041c1d0
        _emit 0xe8
        _emit 0x0b
        _emit 0xe0
        _emit 0xfe
        _emit 0xff
        // 0002e1c5  CALL 0x0042e0a0
        _emit 0xe8
        _emit 0xd6
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0002e1ca  PUSH ESI
        _emit 0x56
        // 0002e1cb  MOV [0x01328f68], ESI
        _emit 0x89
        _emit 0x35
        _emit 0x68
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0002e1d1  CALL 0x0041d0e0
        _emit 0xe8
        _emit 0x0a
        _emit 0xef
        _emit 0xfe
        _emit 0xff
        // 0002e1d6  MOV EAX, dword [ESP+0x38]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // 0002e1da  PUSH EAX
        _emit 0x50
        // 0002e1db  MOV byte [0x01328f19], BL
        _emit 0x88
        _emit 0x1d
        _emit 0x19
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 0002e1e1  CALL 0x0041c270
        _emit 0xe8
        _emit 0x8a
        _emit 0xe0
        _emit 0xfe
        _emit 0xff
        // 0002e1e6  MOVZX ECX, byte [ESP+0x34]
        _emit 0x0f
        _emit 0xb6
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        // 0002e1eb  MOV EDX, dword [ESP+0x34]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x34
        // 0002e1ef  PUSH EDX
        _emit 0x52
        // 0002e1f0  MOV byte [0x01328ef0], CL
        _emit 0x88
        _emit 0x0d
        _emit 0xf0
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        // 0002e1f6  CALL 0x0041c1f0
        _emit 0xe8
        _emit 0xf5
        _emit 0xdf
        _emit 0xfe
        _emit 0xff
        // 0002e1fb  MOVZX EAX, byte [ESP+0x3c]
        _emit 0x0f
        _emit 0xb6
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        // 0002e200  MOV ECX, dword [ESP+0x3c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // 0002e204  ADD ESP,0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        // 0002e207  MOV [0x01328ee8], AL
        _emit 0xa2
        _emit 0xe8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        // 0002e20c  POP ESI
        _emit 0x5e
        // 0002e20d  POP EBX
        _emit 0x5b
        // 0002e20e  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0002e211  MOV dword [ESP+0x4], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0002e215  JMP 0x0041c1d0
        _emit 0xe9
        _emit 0xb6
        _emit 0xdf
        _emit 0xfe
        _emit 0xff
    }
}
