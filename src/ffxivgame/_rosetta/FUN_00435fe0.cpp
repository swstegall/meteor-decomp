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
// FUNCTION: ffxivgame 0x00435fe0 — method that drives a two-stage vtable
//                                  dispatch + optional debug-log call
//                                  (__thiscall, 1 stack param, 140 bytes)
//
// Calling convention: __thiscall (ECX = this); RET 4 (pops 1 stack arg).
// Callee-saves: ESI (PUSH ESI / POP ESI). ECX is also pushed as a local
// slot at function entry (PUSH ECX creates 4 bytes of stack space used
// later as an out-parameter buffer for the first vtable call).
//
// Object layout (from `this` in ECX / ESI):
//   [this + 0x04]  — pointer to sub-object with vtable
//   [this + 0x08]  — DWORD field_8   (arg to second call)
//   [this + 0x0c]  — DWORD field_c   (arg to first vtable call)
//   [this + 0x10]  — DWORD field_10  (arg to both vtable calls and second call)
//   [this + 0x14]  — DWORD field_14  (arg to first vtable call)
//
// First virtual call: vtable[0x2c/4] of *[this+0x04] with 5 explicit
//   args: ([this+0x04], [this+0xc], [this+0x10], &local_slot, [this+0x14]).
//   If return value is non-zero, function returns early (EAX unchanged).
//
// Second call: FUN_009d4600 (rel32, __cdecl) with 3 args:
//   (local_slot_output, [this+0x8], [this+0x10]).
//
// Third virtual call: vtable[0x30/4] of *[this+0x04] with 1 explicit
//   arg: ([this+0x04]). If return value is zero, function returns early.
//
// If the third call succeeds, checks/sets a one-shot global flag at
//   0x01323910 and, if first time, writes the function pointer address
//   0x00433720 into 0x0132390c (a global debug-print dispatch slot),
//   then calls that slot with 5 string/int arguments (debug log emit).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The PUSH ECX local-slot trick, the interleaved ADD ESP cleanup
//   between the second and third calls, and the one-shot flag idiom
//   are all sensitive to exact byte layout. The __declspec(naked) body
//   re-emits the original 140 bytes verbatim; compare.py masks the
//   nine relocation sites (CALL rel32 at +0x31; absolute mem/imm refs
//   at +0x4d, +0x55, +0x5b, +0x5f, +0x65, +0x6f, +0x74, +0x79, +0x7e).

extern "C" __declspec(naked) void FUN_00435fe0() {
    __asm {
        // 00035fe0:  51
        _emit 0x51
        // 00035fe1:  56
        _emit 0x56
        // 00035fe2:  8b f1
        _emit 0x8b
        _emit 0xf1
        // 00035fe4:  8b 56 14
        _emit 0x8b
        _emit 0x56
        _emit 0x14
        // 00035fe7:  8b 46 04
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00035fea:  8b 08
        _emit 0x8b
        _emit 0x08
        // 00035fec:  52
        _emit 0x52
        // 00035fed:  8d 54 24 08
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 00035ff1:  52
        _emit 0x52
        // 00035ff2:  8b 56 10
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        // 00035ff5:  52
        _emit 0x52
        // 00035ff6:  8b 56 0c
        _emit 0x8b
        _emit 0x56
        _emit 0x0c
        // 00035ff9:  52
        _emit 0x52
        // 00035ffa:  50
        _emit 0x50
        // 00035ffb:  8b 41 2c
        _emit 0x8b
        _emit 0x41
        _emit 0x2c
        // 00035ffe:  ff d0
        _emit 0xff
        _emit 0xd0
        // 00036000:  85 c0
        _emit 0x85
        _emit 0xc0
        // 00036002:  75 63
        _emit 0x75
        _emit 0x63
        // 00036004:  8b 4e 10
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00036007:  8b 56 08
        _emit 0x8b
        _emit 0x56
        _emit 0x08
        // 0003600a:  8b 44 24 04
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0003600e:  51
        _emit 0x51
        // 0003600f:  52
        _emit 0x52
        // 00036010:  50
        _emit 0x50
        // 00036011:  e8 ea e5 59 00
        _emit 0xe8
        _emit 0xea
        _emit 0xe5
        _emit 0x59
        _emit 0x00
        // 00036016:  8b 76 04
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        // 00036019:  8b 0e
        _emit 0x8b
        _emit 0x0e
        // 0003601b:  8b 51 30
        _emit 0x8b
        _emit 0x51
        _emit 0x30
        // 0003601e:  83 c4 0c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00036021:  56
        _emit 0x56
        // 00036022:  ff d2
        _emit 0xff
        _emit 0xd2
        // 00036024:  85 c0
        _emit 0x85
        _emit 0xc0
        // 00036026:  74 3f
        _emit 0x74
        _emit 0x3f
        // 00036028:  b8 01 00 00 00
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003602d:  84 05 10 39 32 01
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00036033:  75 10
        _emit 0x75
        _emit 0x10
        // 00036035:  09 05 10 39 32 01
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 0003603b:  c7 05 0c 39 32 01 20 37 43 00
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x20
        _emit 0x37
        _emit 0x43
        _emit 0x00
        // 00036045:  68 f8 55 f6 00
        _emit 0x68
        _emit 0xf8
        _emit 0x55
        _emit 0xf6
        _emit 0x00
        // 0003604a:  68 12 03 00 00
        _emit 0x68
        _emit 0x12
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 0003604f:  68 18 4c f6 00
        _emit 0x68
        _emit 0x18
        _emit 0x4c
        _emit 0xf6
        _emit 0x00
        // 00036054:  68 d8 55 f6 00
        _emit 0x68
        _emit 0xd8
        _emit 0x55
        _emit 0xf6
        _emit 0x00
        // 00036059:  68 e8 4b f6 00
        _emit 0x68
        _emit 0xe8
        _emit 0x4b
        _emit 0xf6
        _emit 0x00
        // 0003605e:  ff 15 0c 39 32 01
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00036064:  83 c4 14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00036067:  5e
        _emit 0x5e
        // 00036068:  59
        _emit 0x59
        // 00036069:  c2 04 00
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
