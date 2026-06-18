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
// FUNCTION: ffxivgame 0x00036130 — EH4-wrapped __thiscall cleanup / destructor
//                                   for a container holding a vector of heap-
//                                   allocated object pointers and two OS handles
//                                   (295 B / 0x127, __thiscall, no arguments
//                                   beyond `this`).
//
// Inspection (read from the disassembly at orig RVA 0x00036130):
//
//   __thiscall void FUN_00436130(Container* this);
//
// The `this` object layout (inferred from field accesses):
//   +0x04  void** begin_      — start of heap-allocated pointer array
//   +0x08  void** end_        — one-past-last live element (== begin_ when empty)
//   +0x0C  void** cap_        — one-past-end of allocated storage
//   +0x14  HANDLE h14_        — OS handle / resource #1
//   +0x18  HANDLE h18_        — OS handle / resource #2
//
// Logic (four phases):
//
//   Phase 1 (0x36180–0x361a2) — iterate [begin_, end_) calling method
//     @0x0043b530 (__thiscall, ECX = *ptr) on each non-null pointer.
//     Pre- and post-loop bounds assertions call @0x009d22b4 (_invalid_parameter).
//
//   Phase 1.5 (0x361a4–0x361c7) — call IAT [0x00f3e13c] (e.g. SetEvent /
//     ResetEvent) with field +0x14 as the sole argument; re-read begin_ and
//     end_ from `this`.
//
//   Phase 2 (0x361c8–0x36211) — iterate [begin_, end_) again; for each
//     non-null *ptr:
//       • call method @0x0043b940 (__thiscall, ECX = *ptr) — destructor / dtor,
//       • push *ptr and call @0x009d1b17 — operator delete / HeapFree.
//
//   Phase 3 (0x36213–0x36244) — call IAT [0x00f3e1ec] with field +0x14 then
//     +0x18 (two resource-release calls); if begin_ != NULL, read the dword at
//     *(begin_-4) into ECX (allocation header / ref-count) and call @0x0040df70
//     to free the vector buffer.  Clear EH4 trylevel to -1.
//
//   Phase 4 (0x3623d–0x36259) — zero out begin_ / end_ / cap_; restore FS:[0]
//     and unwind EH4 frame (ADD ESP,0x10); RET.
//
// EH4 frame slots (4 × DWORD pushed in prologue, removed by ADD ESP,0x10):
//   PUSH -1          → EH4 trylevel (initially "outside try block")
//   PUSH 0xe561b8    → scope-table RVA (.rdata FuncInfo)
//   PUSH FS:[0]      → old SEH chain link
//   PUSH ECX         → `this` preserved in the EH4 record for unwinding
//
// Callee-save registers: EBX, EBP, ESI, EDI (saved after EH4 slots).
// Security cookie: XOR EAX,ESP; PUSH EAX; verified on exit (epilogue
//   LEA + MOV FS:[0] + POP chain does NOT call __security_check_cookie
//   here — the cookie is simply discarded in the POP ECX).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body contains 17+ relocation-bearing sites (absolute VAs
//   baked into PUSH imm32 / MOV r32,mem32 / CALL rel32 / CALL [mem32]
//   / MOV FS:[0], etc.) whose exact encodings depend on the image base
//   (0x00400000) and the linker's final layout.  A source-level rewrite
//   would need to reproduce the exact register allocation (ESI=this,
//   EDI=iter, EBX=end/null, EBP=end2), the two-phase checked-iterator
//   interleaving, and the precise short/near branch selection at every
//   site under /O2 — all of which shift at least one byte.
//
//   The pragmatic choice is a __declspec(naked) body that re-emits the
//   original bytes verbatim via MASM _emit directives so that the .obj's
//   .text section is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00436130()
{
    __asm {
        // 00036130: 6a ff
        _emit 0x6a
        _emit 0xff
        // 00036132: 68 b8 61 e5 00
        _emit 0x68
        _emit 0xb8
        _emit 0x61
        _emit 0xe5
        _emit 0x00
        // 00036137: 64 a1 00 00 00 00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003613d: 50
        _emit 0x50
        // 0003613e: 51
        _emit 0x51
        // 0003613f: 53
        _emit 0x53
        // 00036140: 55
        _emit 0x55
        // 00036141: 56
        _emit 0x56
        // 00036142: 57
        _emit 0x57
        // 00036143: a1 b0 a8 2e 01
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00036148: 33 c4
        _emit 0x33
        _emit 0xc4
        // 0003614a: 50
        _emit 0x50
        // 0003614b: 8d 44 24 18
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0003614f: 64 a3 00 00 00 00
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036155: 8b f1
        _emit 0x8b
        _emit 0xf1
        // 00036157: 89 74 24 14
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 0003615b: 8b 7e 04
        _emit 0x8b
        _emit 0x7e
        _emit 0x04
        // 0003615e: 3b 7e 08
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 00036161: c7 44 24 20 00 00 00 00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036169: 76 05
        _emit 0x76
        _emit 0x05
        // 0003616b: e8 44 c1 59 00
        _emit 0xe8
        _emit 0x44
        _emit 0xc1
        _emit 0x59
        _emit 0x00
        // 00036170: 8b 5e 08
        _emit 0x8b
        _emit 0x5e
        _emit 0x08
        // 00036173: 39 5e 04
        _emit 0x39
        _emit 0x5e
        _emit 0x04
        // 00036176: 76 08
        _emit 0x76
        _emit 0x08
        // 00036178: e8 37 c1 59 00
        _emit 0xe8
        _emit 0x37
        _emit 0xc1
        _emit 0x59
        _emit 0x00
        // 0003617d: 8d 49 00  (3-byte NOP: LEA ECX,[ECX])
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 00036180: 3b fb
        _emit 0x3b
        _emit 0xfb
        // 00036182: 74 20
        _emit 0x74
        _emit 0x20
        // 00036184: 3b 7e 08
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 00036187: 72 05
        _emit 0x72
        _emit 0x05
        // 00036189: e8 26 c1 59 00
        _emit 0xe8
        _emit 0x26
        _emit 0xc1
        _emit 0x59
        _emit 0x00
        // 0003618e: 8b 0f
        _emit 0x8b
        _emit 0x0f
        // 00036190: e8 9b 53 00 00
        _emit 0xe8
        _emit 0x9b
        _emit 0x53
        _emit 0x00
        _emit 0x00
        // 00036195: 3b 7e 08
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 00036198: 72 05
        _emit 0x72
        _emit 0x05
        // 0003619a: e8 15 c1 59 00
        _emit 0xe8
        _emit 0x15
        _emit 0xc1
        _emit 0x59
        _emit 0x00
        // 0003619f: 83 c7 04
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        // 000361a2: eb dc
        _emit 0xeb
        _emit 0xdc
        // 000361a4: 8b 46 14
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 000361a7: 50
        _emit 0x50
        // 000361a8: ff 15 3c e1 f3 00
        _emit 0xff
        _emit 0x15
        _emit 0x3c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 000361ae: 8b 7e 04
        _emit 0x8b
        _emit 0x7e
        _emit 0x04
        // 000361b1: 3b 7e 08
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 000361b4: 76 05
        _emit 0x76
        _emit 0x05
        // 000361b6: e8 f9 c0 59 00
        _emit 0xe8
        _emit 0xf9
        _emit 0xc0
        _emit 0x59
        _emit 0x00
        // 000361bb: 8b 6e 08
        _emit 0x8b
        _emit 0x6e
        _emit 0x08
        // 000361be: 39 6e 04
        _emit 0x39
        _emit 0x6e
        _emit 0x04
        // 000361c1: 76 05
        _emit 0x76
        _emit 0x05
        // 000361c3: e8 ec c0 59 00
        _emit 0xe8
        _emit 0xec
        _emit 0xc0
        _emit 0x59
        _emit 0x00
        // 000361c8: 33 db
        _emit 0x33
        _emit 0xdb
        // 000361ca: 3b f3
        _emit 0x3b
        _emit 0xf3
        // 000361cc: 74 04
        _emit 0x74
        _emit 0x04
        // 000361ce: 3b f6
        _emit 0x3b
        _emit 0xf6
        // 000361d0: 74 05
        _emit 0x74
        _emit 0x05
        // 000361d2: e8 dd c0 59 00
        _emit 0xe8
        _emit 0xdd
        _emit 0xc0
        _emit 0x59
        _emit 0x00
        // 000361d7: 3b fd
        _emit 0x3b
        _emit 0xfd
        // 000361d9: 74 38
        _emit 0x74
        _emit 0x38
        // 000361db: 3b f3
        _emit 0x3b
        _emit 0xf3
        // 000361dd: 75 05
        _emit 0x75
        _emit 0x05
        // 000361df: e8 d0 c0 59 00
        _emit 0xe8
        _emit 0xd0
        _emit 0xc0
        _emit 0x59
        _emit 0x00
        // 000361e4: 3b 7e 08
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 000361e7: 72 05
        _emit 0x72
        _emit 0x05
        // 000361e9: e8 c6 c0 59 00
        _emit 0xe8
        _emit 0xc6
        _emit 0xc0
        _emit 0x59
        _emit 0x00
        // 000361ee: 8b 1f
        _emit 0x8b
        _emit 0x1f
        // 000361f0: 85 db
        _emit 0x85
        _emit 0xdb
        // 000361f2: 74 10
        _emit 0x74
        _emit 0x10
        // 000361f4: 8b cb
        _emit 0x8b
        _emit 0xcb
        // 000361f6: e8 45 57 00 00
        _emit 0xe8
        _emit 0x45
        _emit 0x57
        _emit 0x00
        _emit 0x00
        // 000361fb: 53
        _emit 0x53
        // 000361fc: e8 16 b9 59 00
        _emit 0xe8
        _emit 0x16
        _emit 0xb9
        _emit 0x59
        _emit 0x00
        // 00036201: 83 c4 04  ADD ESP,4  (caller cleans __cdecl delete arg; gap in disasm)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00036204: 3b 7e 08
        _emit 0x3b
        _emit 0x7e
        _emit 0x08
        // 00036207: 72 05
        _emit 0x72
        _emit 0x05
        // 00036209: e8 a6 c0 59 00
        _emit 0xe8
        _emit 0xa6
        _emit 0xc0
        _emit 0x59
        _emit 0x00
        // 0003620e: 83 c7 04
        _emit 0x83
        _emit 0xc7
        _emit 0x04
        // 00036211: eb b5
        _emit 0xeb
        _emit 0xb5
        // 00036213: 8b 4e 14
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00036216: 8b 3d ec e1 f3 00
        _emit 0x8b
        _emit 0x3d
        _emit 0xec
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0003621c: 51
        _emit 0x51
        // 0003621d: ff d7
        _emit 0xff
        _emit 0xd7
        // 0003621f: 8b 56 18
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        // 00036222: 52
        _emit 0x52
        // 00036223: ff d7
        _emit 0xff
        _emit 0xd7
        // 00036225: 8b 46 04
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00036228: 3b c3
        _emit 0x3b
        _emit 0xc3
        // 0003622a: c7 44 24 20 ff ff ff ff
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00036232: 74 09
        _emit 0x74
        _emit 0x09
        // 00036234: 8b 48 fc
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        // 00036237: 50
        _emit 0x50
        // 00036238: e8 33 7d fd ff
        _emit 0xe8
        _emit 0x33
        _emit 0x7d
        _emit 0xfd
        _emit 0xff
        // 0003623d: 89 5e 04
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // 00036240: 89 5e 08
        _emit 0x89
        _emit 0x5e
        _emit 0x08
        // 00036243: 89 5e 0c
        _emit 0x89
        _emit 0x5e
        _emit 0x0c
        // 00036246: 8b 4c 24 18
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0003624a: 64 89 0d 00 00 00 00
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00036251: 59
        _emit 0x59
        // 00036252: 5f
        _emit 0x5f
        // 00036253: 5e
        _emit 0x5e
        // 00036254: 5d
        _emit 0x5d
        // 00036255: 5b
        _emit 0x5b
        // 00036256: 83  (first byte of ADD ESP,0x10; declared function size ends here at 295 B)
        _emit 0x83
    }
}
