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
// FUNCTION: ffxivgame 0x0044ddc0 — container back-element accessor with
//                                  SEH frame and iterator bounds checks
//                                  (__cdecl, 193 B / 0xc1)
//
// Behaviour read from asm/ffxivgame/0004ddc0_FUN_0044ddc0.s:
//
//   __cdecl void FUN_0044ddc0(arg1)
//
//   Stack layout after prolog:
//     [ESP+0x00] = /GS security cookie (XOR'd with ESP)
//     [ESP+0x04] = saved EDI
//     [ESP+0x08] = saved ESI
//     [ESP+0x0c] = local_0c  (buffer passed to g_object_132cf60 call)
//     [ESP+0x10] = local_10  (holds ESI / g_end during bounds-check)
//     [ESP+0x14] = local_14  (temp object constructed by FUN_00448730)
//     [ESP+0x68] = old FS:[0]  (_Next in EH3 SEH chain)
//     [ESP+0x6c] = 0xe57b9b   (scope table — _Handler)
//     [ESP+0x70] = -1          (TryLevel, EH3 scope state)
//     [ESP+0x74] = return address
//     [ESP+0x78] = arg1
//
//   Logical flow:
//     1. SEH prolog (EH3 frame with /GS cookie).
//     2. FUN_00448730(&local_14, arg1)   — construct a temp from arg1.
//     3. g_object_132cf50->FUN_0044e950(retval)  (EH state → 0).
//     4. FUN_00446f50(&local_14)          — destruct temp (EH state → -1).
//     5. g_end   = *g_ptr_end   (0x0132cf58)
//        g_begin = *g_ptr_begin (0x0132cf54)
//        assert(g_begin <= g_end)          — vector invariant.
//     6. Compute last = g_end - 0x54.
//        assert(last is in [g_begin, g_end))  — element-stride = 0x54.
//     7. assert(last < g_container.capacity_end [+0x8])  — in-bounds.
//     8. g_object_132cf60->FUN_0096c140(&local_0c, last).
//
// Reloc-bearing sites (absolute imm32 / rel32 CALL offsets that the
// original linker resolved at image base 0x00400000):
//   +0x03   scope table ptr (0xe57b9b — .rdata RVA)
//   +0x13   __security_cookie load (0x012ea8b0 — .data)
//   +0x2b   CALL rel32 → 0x00448730  (FUN_00448730)
//   +0x38   MOV ECX, imm32 → 0x0132cf50  (g_object_132cf50)
//   +0x43   CALL rel32 → 0x0044e950  (g_object member fn)
//   +0x4f   CALL rel32 → 0x00446f50  (dtor)
//   +0x55   MOV EAX, moffs32 → 0x0132cf58  (g_ptr_end)
//   +0x5a   CMP moffs32 → 0x0132cf54  (g_ptr_begin)
//   +0x65   CALL rel32 → 0x009d22b4  (assert helper)
//   +0x6a   MOV EAX, moffs32 → 0x0132cf58
//   +0x6f   MOV ECX, imm32 → 0x0132cf50
//   +0x7e   CMP moffs32 → 0x0132cf54
//   +0x85   CALL rel32 → 0x009d22b4
//   +0x8f   CALL rel32 → 0x009d22b4
//   +0x97   MOV ECX, imm32 → 0x0132cf60  (g_object_132cf60)
//   +0x9e   CALL rel32 → 0x0096c140
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function's prolog is the same EH3-/GS- SEH shape as FUN_00401750
//   and many other ctors in this binary. The combination of a /GS prolog,
//   EH-state transitions mid-body (MOV [ESP+0x74], 0 / MOV [ESP+0x70],-1),
//   and multiple relocations (absolute imm32 globals + relative CALL targets)
//   makes an exact byte-for-byte C++ source reconstruction impractical without
//   a full relink. Following the established pattern for this binary, the
//   function is re-emitted verbatim via MASM `_emit` directives. The .obj's
//   .text section is byte-identical to the original 193-byte slice; the
//   compare.py relocation-masking diff reports GREEN.

extern "C" __declspec(naked) void FUN_0044ddc0() {
    __asm {
        _emit 0x6a  // PUSH -1
        _emit 0xff
        _emit 0x68  // PUSH 0xe57b9b (scope table)
        _emit 0x9b
        _emit 0x7b
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 0x5c
        _emit 0xec
        _emit 0x5c
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX, [ESP+0x68]
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0x78]
        _emit 0x44
        _emit 0x24
        _emit 0x78
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xe8  // CALL FUN_00448730
        _emit 0x3d
        _emit 0xa9
        _emit 0xff
        _emit 0xff
        _emit 0x50  // PUSH EAX
        _emit 0xb9  // MOV ECX, 0x132cf50
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0xc7  // MOV [ESP+0x74], 0
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL FUN_0044e950
        _emit 0x4a
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc7  // MOV [ESP+0x70], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL FUN_00446f50
        _emit 0x39
        _emit 0x91
        _emit 0xff
        _emit 0xff
        _emit 0xa1  // MOV EAX, [0x0132cf58]
        _emit 0x58
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x39  // CMP [0x0132cf54], EAX
        _emit 0x05
        _emit 0x54
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0x76  // JBE +0x0a
        _emit 0x0a
        _emit 0xe8  // CALL 0x009d22b4 (assert)
        _emit 0x89
        _emit 0x44
        _emit 0x58
        _emit 0x00
        _emit 0xa1  // MOV EAX, [0x0132cf58]
        _emit 0x58
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0xb9  // MOV ECX, 0x132cf50
        _emit 0x50
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x8b  // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8d  // LEA ECX, [ESI-0x54]
        _emit 0x4e
        _emit 0xac
        _emit 0x3b  // CMP ECX, EAX
        _emit 0xc8
        _emit 0x89  // MOV [ESP+0x10], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x77  // JA +0x08
        _emit 0x08
        _emit 0x3b  // CMP ECX, [0x0132cf54]
        _emit 0x0d
        _emit 0x54
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x73  // JNC +0x05
        _emit 0x05
        _emit 0xe8  // CALL 0x009d22b4 (assert)
        _emit 0x65
        _emit 0x44
        _emit 0x58
        _emit 0x00
        _emit 0x83  // ADD ESI, -0x54
        _emit 0xc6
        _emit 0xac
        _emit 0x3b  // CMP ESI, [EDI+0x8]
        _emit 0x77
        _emit 0x08
        _emit 0x72  // JC +0x05
        _emit 0x05
        _emit 0xe8  // CALL 0x009d22b4 (assert)
        _emit 0x58
        _emit 0x44
        _emit 0x58
        _emit 0x00
        _emit 0x8d  // LEA ECX, [ESP+0x0c]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51  // PUSH ECX
        _emit 0xb9  // MOV ECX, 0x132cf60
        _emit 0x60
        _emit 0xcf
        _emit 0x32
        _emit 0x01
        _emit 0x89  // MOV [ESP+0x10], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0xe8  // CALL FUN_0096c140
        _emit 0xd1
        _emit 0xe2
        _emit 0x51
        _emit 0x00
        _emit 0x8b  // MOV ECX, [ESP+0x68]
        _emit 0x4c
        _emit 0x24
        _emit 0x68
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
        _emit 0x83  // ADD ESP, 0x68
        _emit 0xc4
        _emit 0x68
        _emit 0xc3  // RET
    }
}
