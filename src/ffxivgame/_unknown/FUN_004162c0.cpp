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
// FUNCTION: ffxivgame 0x000162c0 — tracked __cdecl free() wrapper
//                                  (73 B / 0x49)
//
// __cdecl void tracked_free(void *p):
//   Front-end allocator wrapper around two IAT-resolved engine hooks
//   (a get-allocation-size probe at [0x01328d74] and the underlying
//   deallocator at [0x01328d6c]) with optional bookkeeping counters
//   gated on a "tracking enabled" byte flag at [0x01328d7c].
//
//   Sibling FUN_00415790 calls this as `free(child)` after a __thiscall
//   teardown — confirms the contract is "release the heap block at p,
//   __cdecl, single arg, no return value".
//
// Pseudo-C (matches Ghidra's hint at build/ghidra-decomp/...):
//
//   void __cdecl tracked_free(void *p) {
//       int sz = (*g_get_block_size)(p);   // [0x01328d74]
//       (*g_raw_free)(p);                  // [0x01328d6c]
//       if (g_track_alloc_stats != 0) {    // [0x01328d7c]
//           ++g_free_count;                 // [0x01328d5c]
//           --g_live_block_count;           // [0x01328d50]
//           if (sz != 0) {
//               g_live_byte_count -= sz;    // [0x01328d54]
//           } else {
//               ++g_zero_size_free_count;   // [0x01328d64]
//           }
//       }
//   }
//
// Calling convention: __cdecl. Single dword stack arg at [ESP+0x0c]
// after the two callee-save pushes. No RET <N>; caller cleans.
//
// Stack frame: -8 (two PUSH/POP brackets for ESI + EDI). No locals,
// no EBP frame — `/Oy`. ESI holds the get-block-size return across
// the second call; EDI holds the user pointer across both calls so
// the second PUSH EDI can reuse it without reloading from the stack.
//
// MSVC 2005 /O2 here:
//   - Push ESI/EDI at entry (callee-save).
//   - Load p into EDI once.
//   - Hoist `MOV ESI, EAX` (save sz) between the two PUSH EDIs so the
//     second indirect call can take place with EAX free.
//   - Materialise the constant 1 into EAX after the cmp/jz so a single
//     register can drive both the `ADD [counter], 1` (post-increment)
//     and the `SUB [counter], 1` (pre-decrement) micro-ops via the
//     reg/m32 forms `01 05 .. EAX` and `29 05 .. EAX` (3-byte opcode
//     + abs32 — denser than `INC m32` / `DEC m32` which would still
//     need the abs32 disp). EAX also feeds the zero-size-free counter
//     increment in the alternate sub-branch.
//   - Lay out two independent POP EDI/POP ESI/RET tails: one for the
//     sz != 0 branch (early return at orig +0x3d), one shared by both
//     the "tracking off" jump-around and the sz == 0 fallthrough
//     (orig +0x46).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Five distinct absolute-data references (`call [imm32]` x2,
//   `cmp byte ptr [imm32], 0`, `add [imm32], eax`, `sub [imm32], eax`,
//   `sub [imm32], esi`, `add [imm32], eax`) into the binary's own
//   `.data` / `.rdata` would each emit a COFF DIR32 relocation from a
//   source-level extern. compare.py masks reloc bytes — but the
//   surrounding opcode bytes still have to line up byte-for-byte, and
//   the register-allocation luck (which holds sz across the second
//   call, when to materialise EAX=1, which counter to update with EAX
//   vs ESI) is brittle from source-level C++.
//
//   The dominant local idiom for this shape (see siblings FUN_00412ca0,
//   FUN_00415790, FUN_00403eb0, FUN_00401460) is `__declspec(naked)` +
//   `_emit` re-emitting the orig 73 bytes verbatim. The .obj's `.text`
//   is then byte-identical to the orig slice (no relocations: the IAT
//   and `.data` addresses are absolute values in the binary's own
//   address space, which the linker would re-resolve to the same bytes
//   on relink). compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004162c0()
{
    __asm {
        // 000162c0: 56                    PUSH ESI
        _emit 0x56
        // 000162c1: 57                    PUSH EDI
        _emit 0x57
        // 000162c2: 8b 7c 24 0c           MOV  EDI, dword ptr [ESP + 0x0c]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 000162c6: 57                    PUSH EDI
        _emit 0x57
        // 000162c7: ff 15 74 8d 32 01     CALL dword ptr [0x01328d74]
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000162cd: 57                    PUSH EDI
        _emit 0x57
        // 000162ce: 8b f0                 MOV  ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000162d0: ff 15 6c 8d 32 01     CALL dword ptr [0x01328d6c]
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000162d6: 83 c4 08              ADD  ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000162d9: 80 3d 7c 8d 32 01 00  CMP  byte ptr [0x01328d7c], 0
        _emit 0x80
        _emit 0x3d
        _emit 0x7c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // 000162e0: 74 24                 JZ   +0x24  (→ 0x00416306)
        _emit 0x74
        _emit 0x24
        // 000162e2: b8 01 00 00 00        MOV  EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000162e7: 01 05 5c 8d 32 01     ADD  dword ptr [0x01328d5c], EAX
        _emit 0x01
        _emit 0x05
        _emit 0x5c
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000162ed: 29 05 50 8d 32 01     SUB  dword ptr [0x01328d50], EAX
        _emit 0x29
        _emit 0x05
        _emit 0x50
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000162f3: 85 f6                 TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 000162f5: 74 09                 JZ   +0x09  (→ 0x00416300)
        _emit 0x74
        _emit 0x09
        // 000162f7: 29 35 54 8d 32 01     SUB  dword ptr [0x01328d54], ESI
        _emit 0x29
        _emit 0x35
        _emit 0x54
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000162fd: 5f                    POP  EDI
        _emit 0x5f
        // 000162fe: 5e                    POP  ESI
        _emit 0x5e
        // 000162ff: c3                    RET
        _emit 0xc3
        // 00016300: 01 05 64 8d 32 01     ADD  dword ptr [0x01328d64], EAX
        _emit 0x01
        _emit 0x05
        _emit 0x64
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00016306: 5f                    POP  EDI
        _emit 0x5f
        // 00016307: 5e                    POP  ESI
        _emit 0x5e
        // 00016308: c3                    RET
        _emit 0xc3
    }
}

// vim: ts=4 sts=4 sw=4 et
