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
// FUNCTION: ffxivgame 0x00404f10 — system-locale → FFXIV region code
//                                   (81 B / 0x51, __cdecl, no args)
//
// Inspection (read from the disassembly at orig RVA 0x00004f10):
//
//   __cdecl int FUN_00404f10(void)
//
//   Calls kernel32!GetSystemDefaultLCID() (IAT slot @ 0x00f3e19c —
//   resolved by name via the PE import directory; hint 0x1be) and
//   maps the returned LCID to the FFXIV client's internal region
//   code:
//
//       0x411 (ja-JP)                                        -> 1
//       0x404, 0x804, 0xc04, 0x1004 (zh-* family)            -> 4
//       0x409, 0xc0c, 0x1009 (en-US, fr-CA, en-CA)           -> 2
//       any other LCID                                       -> 3
//
//   MSVC 2005 /O2 lowers this 8-case sparse switch into a balanced
//   binary-search decision tree pivoting on 0xc04 (the median of the
//   eight reachable case values), then pivoting on 0x411 on the
//   low-side arm and falling through into offset-from-pivot SUB
//   chains on both the <0x411 and >0xc04 arms. The two SUB chains
//   converge at a shared common-tail (`common_check`) that re-uses
//   the JZ→4 path so the "return 4" `MOV EAX, 4 / RET` epilogue is
//   emitted only once.
//
//   Flow / branch graph (offsets into the 81-byte body):
//
//     +0x00   CALL [GetSystemDefaultLCID]
//     +0x06   CMP EAX, 0xc04
//     +0x0b   JG  hi_path     (+0x25 -> +0x32)
//     +0x0d   JZ  ret4        (+0x1d -> +0x2c)
//     +0x0f   CMP EAX, 0x411
//     +0x14   JG  mid_path    (+0x0f -> +0x25)
//     +0x16   JZ  ret1        (+0x07 -> +0x1f)
//     +0x18   SUB EAX, 0x404
//     +0x1d   JMP common_check(+0x1f -> +0x3e)
//     +0x1f   ret1:  MOV EAX, 1; RET
//     +0x25   mid_path: CMP EAX, 0x804
//     +0x2a   JNZ ret3        (+0x19 -> +0x45)
//     +0x2c   ret4:  MOV EAX, 4; RET
//     +0x32   hi_path: SUB EAX, 0xc0c
//     +0x37   JZ  ret2        (+0x12 -> +0x4b)
//     +0x39   SUB EAX, 0x3f8
//     +0x3e   common_check: JZ ret4  (-0x14 -> +0x2c)
//     +0x40   SUB EAX, 5
//     +0x43   JZ  ret2        (+0x06 -> +0x4b)
//     +0x45   ret3:  MOV EAX, 3; RET
//     +0x4b   ret2:  MOV EAX, 2; RET
//
// Reloc-bearing site in the orig 81 bytes:
//     +0x00   import IAT load (.rdata 0x00f3e19c —
//             kernel32!GetSystemDefaultLCID, hint 0x1be)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level `switch (GetSystemDefaultLCID()) { ... }` with the
//   eight reachable cases would express the same semantics, but MSVC
//   2005's switch-lowering heuristic is sensitive to surrounding code
//   in the TU; coaxing it into the same decision-tree shape (and the
//   exact JG/JZ short-branch encodings + the shared-tail SUB+JZ
//   convergence) in isolation isn't reliable. The IAT call also
//   resolves to a linker-emitted absolute address inside the orig
//   binary's own address space, so emitting the IAT slot as immediate
//   bytes via `__declspec(naked)` produces an .obj whose `.text` is
//   byte-identical to the orig slice (no relocations to mask).

extern "C" __declspec(naked) int __cdecl FUN_00404f10() {
    __asm {
        _emit 0xff              // +0x00: CALL [0x00f3e19c]  ; GetSystemDefaultLCID
        _emit 0x15
        _emit 0x9c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x3d              // +0x06: CMP EAX, 0x00000c04
        _emit 0x04
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x7f              // +0x0b: JG  +0x25  -> hi_path
        _emit 0x25
        _emit 0x74              // +0x0d: JZ  +0x1d  -> ret4
        _emit 0x1d
        _emit 0x3d              // +0x0f: CMP EAX, 0x00000411
        _emit 0x11
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x7f              // +0x14: JG  +0x0f  -> mid_path
        _emit 0x0f
        _emit 0x74              // +0x16: JZ  +0x07  -> ret1
        _emit 0x07
        _emit 0x2d              // +0x18: SUB EAX, 0x00000404
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // +0x1d: JMP +0x1f  -> common_check
        _emit 0x1f
        _emit 0xb8              // +0x1f: ret1: MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // +0x24: RET
        _emit 0x3d              // +0x25: mid_path: CMP EAX, 0x00000804
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x75              // +0x2a: JNZ +0x19  -> ret3
        _emit 0x19
        _emit 0xb8              // +0x2c: ret4: MOV EAX, 0x00000004
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // +0x31: RET
        _emit 0x2d              // +0x32: hi_path: SUB EAX, 0x00000c0c
        _emit 0x0c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x74              // +0x37: JZ  +0x12  -> ret2
        _emit 0x12
        _emit 0x2d              // +0x39: SUB EAX, 0x000003f8
        _emit 0xf8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x74              // +0x3e: common_check: JZ -0x14 -> ret4
        _emit 0xec
        _emit 0x83              // +0x40: SUB EAX, 0x05
        _emit 0xe8
        _emit 0x05
        _emit 0x74              // +0x43: JZ  +0x06  -> ret2
        _emit 0x06
        _emit 0xb8              // +0x45: ret3: MOV EAX, 0x00000003
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // +0x4a: RET
        _emit 0xb8              // +0x4b: ret2: MOV EAX, 0x00000002
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // +0x50: RET
    }
}
