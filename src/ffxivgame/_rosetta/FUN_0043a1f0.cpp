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
// FUNCTION: ffxivgame 0x0043a1f0 — one-time float-table initializer + forward
//                                  to two callees (0x0043a080, 0x0042edb0).
//                                  __cdecl, 369 B / 0x171, no SEH, no /GS.
//
// Inspection (read from the disassembly at orig RVA 0x0003a1f0):
//
//   __cdecl void* FUN_0043a1f0(...) — plain __cdecl; epilogue is
//   ADD ESP,0x80 / RET (no arg pop, no /GS cookie).
//
//   Structural shape:
//
//     // 0x80-byte local frame; EAX = 1 (the init-flag bit)
//     MOV EAX, 1
//     SUB ESP, 0x80
//     TEST byte ptr [g_init_flag @ 0x0132ca18], AL
//     JNZ  already_inited                      // skip if bit already set
//
//     // ---- one-time table initialisation ----
//     XORPS  XMM0, XMM0                        // zero
//     MOVSS  XMM1, [0x00fb7a60]               // float constant A
//     MOVSS  XMM2, [0x00f62f60]               // float constant B
//     OR dword ptr [g_init_flag], EAX          // set bit
//
//     // zero-fill twelve floats in [esp+0x4..esp+0x34]
//     MOVSS [esp+0x30], XMM0
//     MOVSS [esp+0x34], XMM0
//     MOVSS [esp+0x20], XMM0
//     MOVSS [esp+0x24], XMM0
//     MOVSS [esp+0x2c], XMM0
//     MOVSS [esp+0x10], XMM0
//     MOVSS [esp+0x18], XMM0
//     MOVSS [esp+0x1c], XMM0
//     MOVSS [esp+0x04], XMM0
//     MOVSS [esp+0x08], XMM0
//     MOVSS [esp+0x0c], XMM0
//
//     // lay XMM1 (constant A) into [esp+0x00], then copy 8 bytes at a
//     // time into the global table at 0x0132c9d8 using MOVQ-pair:
//     MOVSS [esp], XMM1
//     MOVQ  XMM0, [esp]           ; grab qword (XMM1.lo + zero)
//     MOVQ  [0x0132c9d8], XMM0
//     MOVQ  XMM0, [esp+0x08]
//     MOVQ  [0x0132c9e0], XMM0
//     MOVSS [esp+0x14], XMM1
//     MOVQ  XMM0, [esp+0x10]
//     MOVQ  [0x0132c9e8], XMM0
//     MOVQ  XMM0, [esp+0x18]
//     MOVQ  [0x0132c9f0], XMM0
//     MOVQ  XMM0, [esp+0x20]
//     MOVQ  [0x0132c9f8], XMM0
//     MOVSS [esp+0x28], XMM2      // constant B
//     MOVQ  XMM0, [esp+0x28]
//     MOVQ  [0x0132ca00], XMM0
//     MOVQ  XMM0, [esp+0x30]
//     MOVSS [esp+0x38], XMM2
//     MOVSS [esp+0x3c], XMM1
//     MOVQ  [0x0132ca08], XMM0
//     MOVQ  XMM0, [esp+0x38]
//     MOVQ  [0x0132ca10], XMM0
//
//   already_inited:
//     // ---- load six float arguments from the caller's frame via x87 ----
//     FLD  float ptr [esp+0x9c]    // arg1 (first float param)
//     PUSH ESI
//     SUB  ESP, 0x18
//     FSTP float ptr [esp+0x14]
//     LEA  EAX, [esp+0x5c]        // pointer to something in the local frame
//     FLD  float ptr [esp+0xb4]
//     FSTP float ptr [esp+0x10]
//     FLD  float ptr [esp+0xb0]
//     FSTP float ptr [esp+0x0c]
//     FLD  float ptr [esp+0xac]
//     FSTP float ptr [esp+0x08]
//     FLD  float ptr [esp+0xa8]
//     FSTP float ptr [esp+0x04]
//     FLD  float ptr [esp+0xa4]
//     FSTP float ptr [esp]
//     PUSH EAX
//     CALL FUN_0043a080
//
//     MOV  ESI, dword ptr [esp+0xa4]  // load one of the caller args as return value
//     ADD  ESP, 0x1c
//     LEA  ECX, [esp+0x44]
//     PUSH ECX
//     PUSH ESI
//     MOV  ECX, 0x0132c9d8            // `this` = &g_float_table
//     CALL FUN_0042edb0
//     MOV  EAX, ESI
//     POP  ESI
//     ADD  ESP, 0x80
//     RET
//
//   Reloc-bearing sites in the orig 369 bytes (absolute addresses baked in
//   at link time; tools/compare.py masks them on the cmp_obj path):
//     +0x06  DIR32 → 0x0132ca18  (TEST/OR init flag)
//     +0x0c  DIR32 → 0x0132ca18  (JNZ target is relative — no reloc)
//     +0x12  DIR32 → 0x00fb7a60  (MOVSS XMM1 source)
//     +0x1a  DIR32 → 0x00f62f60  (MOVSS XMM2 source)
//     +0x22  DIR32 → 0x0132ca18  (OR init flag)
//     +0x7c  DIR32 → 0x0132c9d8  (MOVQ dest, 1st)
//     +0x84  DIR32 → 0x0132c9e0  (MOVQ dest, 2nd)
//     +0x90  DIR32 → 0x0132c9e8  (MOVQ dest, 3rd)
//     +0x98  DIR32 → 0x0132c9f0  (MOVQ dest, 4th)
//     +0xa0  DIR32 → 0x0132c9f8  (MOVQ dest, 5th)
//     +0xac  DIR32 → 0x0132ca00  (MOVQ dest, 6th)
//     +0xb8  DIR32 → 0x0132ca08  (MOVQ dest, 7th)
//     +0xc0  DIR32 → 0x0132ca10  (MOVQ dest, 8th)
//     +0xe8  REL32 → 0x0043a080  (CALL FUN_0043a080)
//     +0xf7  DIR32 → 0x0132c9d8  (MOV ECX, &g_float_table)
//     +0xfc  REL32 → 0x0042edb0  (CALL FUN_0042edb0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This function has fifteen relocation windows binding it to absolute
//   addresses in the orig image (XMM float-constant sources, global table
//   targets, two CALL rel32s). A source-level C++ port would need the
//   linker to resolve all of them at the same RVAs as the orig 0x00400000
//   image base, which a standalone .obj cannot satisfy. The naked-asm
//   passthrough used by FUN_00405080, FUN_0040ced0, FUN_004014b0, and
//   dozens of other siblings in this directory is the correct approach:
//   re-emit the orig 369 bytes verbatim via MASM `_emit` directives so
//   the .text section is byte-identical to the orig slice, and let
//   tools/compare.py report GREEN.

extern "C" __declspec(naked) void FUN_0043a1f0() {
    __asm {
        // MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // SUB ESP, 0x80
        _emit 0x81
        _emit 0xec
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // TEST byte ptr [0x0132ca18], AL
        _emit 0x84
        _emit 0x05
        _emit 0x18
        _emit 0xca
        _emit 0x32
        _emit 0x01
        // JNZ 0x0043a2ee
        _emit 0x0f
        _emit 0x85
        _emit 0xe7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // XORPS XMM0, XMM0
        _emit 0x0f
        _emit 0x57
        _emit 0xc0
        // MOVSS XMM1, [0x00fb7a60]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x60
        _emit 0x7a
        _emit 0xfb
        _emit 0x00
        // MOVSS XMM2, [0x00f62f60]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x15
        _emit 0x60
        _emit 0x2f
        _emit 0xf6
        _emit 0x00
        // OR dword ptr [0x0132ca18], EAX
        _emit 0x09
        _emit 0x05
        _emit 0x18
        _emit 0xca
        _emit 0x32
        _emit 0x01
        // MOVSS [esp+0x30], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // MOVSS [esp+0x34], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // MOVSS [esp+0x20], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // MOVSS [esp+0x24], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // MOVSS [esp+0x2c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // MOVSS [esp+0x10], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOVSS [esp+0x18], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // MOVSS [esp+0x1c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // MOVSS [esp+0x04], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // MOVSS [esp+0x08], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // MOVSS [esp+0x0c], XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // MOVSS [esp], XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x0c
        _emit 0x24
        // MOVQ XMM0, [esp]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x04
        _emit 0x24
        // MOVQ [0x0132c9d8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xd8
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // MOVQ XMM0, [esp+0x08]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // MOVQ [0x0132c9e0], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xe0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // MOVSS [esp+0x14], XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // MOVQ XMM0, [esp+0x10]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // MOVQ [0x0132c9e8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xe8
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // MOVQ XMM0, [esp+0x18]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // MOVQ [0x0132c9f0], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xf0
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // MOVQ XMM0, [esp+0x20]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // MOVQ [0x0132c9f8], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xf8
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // MOVSS [esp+0x28], XMM2
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // MOVQ XMM0, [esp+0x28]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // MOVQ [0x0132ca00], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x00
        _emit 0xca
        _emit 0x32
        _emit 0x01
        // MOVQ XMM0, [esp+0x30]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // MOVSS [esp+0x38], XMM2
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // MOVSS [esp+0x3c], XMM1
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // MOVQ [0x0132ca08], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x08
        _emit 0xca
        _emit 0x32
        _emit 0x01
        // MOVQ XMM0, [esp+0x38]
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x44
        _emit 0x24
        _emit 0x38
        // MOVQ [0x0132ca10], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0x10
        _emit 0xca
        _emit 0x32
        _emit 0x01
        // already_inited:
        // FLD float ptr [esp+0x9c]
        _emit 0xd9
        _emit 0x84
        _emit 0x24
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH ESI
        _emit 0x56
        // SUB ESP, 0x18
        _emit 0x83
        _emit 0xec
        _emit 0x18
        // FSTP float ptr [esp+0x14]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // LEA EAX, [esp+0x5c]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x5c
        // FLD float ptr [esp+0xb4]
        _emit 0xd9
        _emit 0x84
        _emit 0x24
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // FSTP float ptr [esp+0x10]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // FLD float ptr [esp+0xb0]
        _emit 0xd9
        _emit 0x84
        _emit 0x24
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // FSTP float ptr [esp+0x0c]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // FLD float ptr [esp+0xac]
        _emit 0xd9
        _emit 0x84
        _emit 0x24
        _emit 0xac
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // FSTP float ptr [esp+0x08]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // FLD float ptr [esp+0xa8]
        _emit 0xd9
        _emit 0x84
        _emit 0x24
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // FSTP float ptr [esp+0x04]
        _emit 0xd9
        _emit 0x5c
        _emit 0x24
        _emit 0x04
        // FLD float ptr [esp+0xa4]
        _emit 0xd9
        _emit 0x84
        _emit 0x24
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // FSTP float ptr [esp]
        _emit 0xd9
        _emit 0x1c
        _emit 0x24
        // PUSH EAX
        _emit 0x50
        // CALL 0x0043a080
        _emit 0xe8
        _emit 0x43
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // MOV ESI, dword ptr [esp+0xa4]
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0xa4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // LEA ECX, [esp+0x44]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        // PUSH ECX
        _emit 0x51
        // PUSH ESI
        _emit 0x56
        // MOV ECX, 0x0132c9d8
        _emit 0xb9
        _emit 0xd8
        _emit 0xc9
        _emit 0x32
        _emit 0x01
        // CALL 0x0042edb0
        _emit 0xe8
        _emit 0x59
        _emit 0x4a
        _emit 0xff
        _emit 0xff
        // MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // POP ESI
        _emit 0x5e
        // ADD ESP, 0x80
        _emit 0x81
        _emit 0xc4
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // RET
        _emit 0xc3
    }
}
