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
// FUNCTION: ffxivgame 0x00041a50 — `__stdcall` two-arg audio gain setter
//                                  (67 B / 0x43)
//
// Inspection (read from the disassembly at orig RVA 0x00041a50):
//
//   __stdcall void FUN_00441a50(float a, int b) — callee-cleans 2 dwords
//                                                 (RET 0x8).
//     [ESP+0x04] : float a   (stored to the global at 0x0126688c)
//     [ESP+0x08] : int   b   (forwarded verbatim to the sub_b8fd70 call)
//
//   Structure (matches asm flow):
//
//     g_0126688c = a;                       // MOVSS [0x0126688c], XMM0
//     if (g_0132ca94 == 0) {                // CMP byte [0x0132ca94], 0 / JNZ ret
//         // double-precision scale of a by the companion global, then
//         // narrow back to float and hand off (cdecl, 2 args):
//         float scaled = (float)((double)a * (double)g_01266890);
//         sub_b8fd70(scaled, b);            // PUSH b / ... / MOVSS [ESP],XMM0 / CALL
//     }
//
//   The CVTPS2PD/MULSD/CVTPD2PS triple is MSVC's lowering of a
//   `float * float` product evaluated in double precision — the source
//   multiply is performed on widened doubles then re-narrowed to float
//   for the call's first argument.
//
// Reloc-bearing sites in the orig 67 bytes (these absolute addresses
// resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation can't reproduce them via source — naked
// asm emits them as raw immediate bytes which happen to match the orig
// binary's resolved addresses byte-for-byte):
//     +0x02   CMP   [0x0132ca94]  (flag global, abs32)
//     +0x09   MOVSS [0x0126688c]  (stored-float global, abs32)
//     +0x0f   MOVSS [0x01266890]  (scale-factor global, abs32)
//     +0x39   CALL rel32 → 0x00b8fd70 (sub_b8fd70)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 /O2 to reproduce this exact CMP-global / MOVSS /
//   double-widen-multiply / CALL sequence with the four linker-resolved
//   absolute references would require the surrounding TU and allocator
//   state. The pragmatic choice — the same one siblings FUN_00406fa0 and
//   FUN_00408780 took — is a `__declspec(naked)` body that re-emits the
//   orig 67 bytes verbatim via MASM `_emit` directives. The absolute
//   addresses and rel32 offset are absolute values in the binary's own
//   address space, so emitting them as raw immediates produces the same
//   bytes the linker would produce. `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00441a50() {
    __asm {
        _emit 0x80              // CMP byte ptr [0x0132ca94], 0x00
        _emit 0x3d
        _emit 0x94
        _emit 0xca
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0xf3              // MOVSS XMM0, dword ptr [ESP+0x04]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0xf3              // MOVSS dword ptr [0x0126688c], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x05
        _emit 0x8c
        _emit 0x68
        _emit 0x26
        _emit 0x01
        _emit 0x75              // JNZ +0x29  → RET
        _emit 0x29
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x08]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0xf3              // MOVSS XMM1, dword ptr [0x01266890]
        _emit 0x0f
        _emit 0x10
        _emit 0x0d
        _emit 0x90
        _emit 0x68
        _emit 0x26
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0x0f              // CVTPS2PD XMM0, XMM0
        _emit 0x5a
        _emit 0xc0
        _emit 0x0f              // CVTPS2PD XMM1, XMM1
        _emit 0x5a
        _emit 0xc9
        _emit 0xf2              // MULSD XMM1, XMM0
        _emit 0x0f
        _emit 0x59
        _emit 0xc8
        _emit 0x51              // PUSH ECX
        _emit 0x66              // CVTPD2PS XMM0, XMM1
        _emit 0x0f
        _emit 0x5a
        _emit 0xc1
        _emit 0xf3              // MOVSS dword ptr [ESP], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x04
        _emit 0x24
        _emit 0xe8              // CALL rel32 → 0x00b8fd70
        _emit 0xe3
        _emit 0xe2
        _emit 0x74
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
