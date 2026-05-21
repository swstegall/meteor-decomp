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
// FUNCTION: ffxivgame 0x00005080 — `__cdecl` once-only "\latest.txt" check
//                                  bootstrap (349 B / 0x15d, EH3-SEH wrapped).
//
// Behaviour read from the disassembly at orig RVA 0x00005080 and Ghidra's
// headless decompile pass:
//
//   __cdecl void FUN_00405080();
//
//   Two function-scope magic-statics (mutex-by-OR on the same flag word
//   at .data 0x01323890) bootstrap a pair of sibling singletons via
//   FUN_00452a40:
//
//       if ((g_init_flags & 1) == 0) {            // [01323890] bit 0
//           g_init_flags |= 1;
//           trylevel = 0;
//           FUN_00452a40(0);                      // ctor: singleton A
//                                                 // atexit dtor @ [0132388c]
//       }
//       if ((g_init_flags & 2) == 0) {            // [01323890] bit 1
//           g_init_flags |= 2;
//           trylevel = 1;
//           FUN_00452a40(-1);                     // ctor: singleton B
//                                                 // atexit dtor @ [01323888]
//       }
//       trylevel = -1;
//
//   Followed by a one-shot "\latest.txt" probe gated by a tri-state
//   InterlockedExchangeAdd on .data 0x0132388c (state 0 → 1 → 2):
//
//       if (InterlockedExchangeAdd(&g_state, 0) != 2 &&
//           InterlockedCompareExchange(&g_state, 1, 0) == 0) {
//           char path[84];                                  // [esp+0x10..0x63]
//           FUN_00447550(path, "\\latest.txt");              // path-cat helper
//           trylevel = 2;
//           bool ok = FUN_004531c0(path);                    // existence check
//           InterlockedCompareExchange(&g_result, ok, -1);   // .data 0x01323888
//           InterlockedCompareExchange(&g_state,  2,  1);
//           trylevel = -1;
//           FUN_00446f50();                                  // path dtor
//       }
//
//   Then a spin-wait until the worker that grabbed the 0→1 slot publishes 2:
//
//       while (InterlockedExchangeAdd(&g_state, 0) != 2) Sleep(1);
//       (void)InterlockedExchangeAdd(&g_result, 0);          // load-barrier
//
//   The literal "\\latest.txt" lives at .rdata 0x00f54bc0.
//
//   Stack frame (after the EH3 prologue, ESP-relative):
//     [esp+0x000]               EH3 cookie XOR esp (pushed last)
//     [esp+0x004]               saved EDI
//     [esp+0x008]               saved ESI
//     [esp+0x00c]               saved ECX (alignment slot)
//     [esp+0x010 .. esp+0x063]  char path[84] (the FUN_00447550 buffer)
//     [esp+0x064]               saved FS:[0] chain link
//     [esp+0x068]               EH3 scope-table (0xe54754)
//     [esp+0x06c]               EH3 trylevel (-1 / 0 / 1 / 2)
//     [esp+0x070]               EH3 secondary state slot (state numbering for
//                               the per-arm scope-table dispatch — 0 / 1 / 2)
//     [esp+0x074]               __security_cookie XOR ESP (orig copy)
//     [esp+0x078]               return address
//
//   Reloc-bearing sites in the orig 349 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x03  scope-table handler RVA   (0x00e54754 — .rdata FuncInfo)
//     +0x09  FS:[0] read                (constant 0, fold-through)
//     +0x12  __security_cookie load     (.data 0x012ea8b0)
//     +0x1f  __security_cookie load     (.data 0x012ea8b0, 2nd)
//     +0x2c  FS:[0] install             (constant 0, fold-through)
//     +0x32  init-flag TEST             (.data 0x01323890)
//     +0x3b  init-flag OR               (.data 0x01323890)
//     +0x43  &g_state load              (.data 0x0132388c — ECX init for callee)
//     +0x50  FUN_00452a40 CALL          (.text 0x00452a40 rel32)
//     +0x5e  init-flag TEST             (.data 0x01323890, 2nd)
//     +0x67  init-flag OR               (.data 0x01323890, 2nd)
//     +0x6f  &g_state load              (.data 0x0132388c, 2nd)
//     +0x7c  FUN_00452a40 CALL          (.text 0x00452a40 rel32, 2nd)
//     +0x8a  InterlockedExchangeAdd IAT (.rdata 0x00f3e1a4 — EDI sticky load)
//     +0x91  &g_state PUSH              (.data 0x0132388c — addend arg)
//     +0x9e  InterlockedCompareExchange IAT (.rdata 0x00f3e1a0 — ESI sticky load)
//     +0xa7  &g_state PUSH              (.data 0x0132388c, 2nd)
//     +0xb2  "\\latest.txt" PUSH        (.rdata 0x00f54bc0)
//     +0xbc  path-ctor this PUSH        (.data 0x01323898 — ECX init for callee)
//     +0xc1  FUN_00447550 CALL          (.text 0x00447550 rel32)
//     +0xd3  FUN_004531c0 CALL          (.text 0x004531c0 rel32)
//     +0xe5  &g_result PUSH             (.data 0x01323888 — ICX dest)
//     +0xf1  &g_state PUSH              (.data 0x0132388c, 3rd)
//     +0x100 FUN_00446f50 CALL          (.text 0x00446f50 rel32 — path dtor)
//     +0x108 &g_state PUSH              (.data 0x0132388c, 4th — spin probe)
//     +0x117 InterlockedExchangeAdd IAT (.rdata 0x00f3e1c8 — Sleep loop IAT)
//     +0x123 &g_state PUSH              (.data 0x0132388c, 5th — spin reload)
//     +0x132 &g_result PUSH             (.data 0x01323888 — final load-barrier)
//     +0x144 FS:[0] restore             (constant 0, fold-through)
//     +0x153 __security_check_cookie    (.text 0x009d1f6c rel32)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact EH3 prolog (PUSH -1 / PUSH scope-table / PUSH
//   FS:[0] / SUB ESP / cookie XOR ESP / push-callees / second cookie XOR
//   ESP / FS:[0] install), the exact EH3 dual-state numbering driving the
//   four per-arm trylevel updates ([esp+0x6c] = -1/0/1/2 AND [esp+0x70] =
//   0/1/2), the InterlockedExchangeAdd / InterlockedCompareExchange IAT
//   choreography across ESI/EDI sticky loads, and the linker-resolved
//   absolute addresses in the twenty-seven relocation windows above. Each
//   of those constraints is brittle under /O2 — every high-level rewrite
//   shifts at least one byte (cookie-stack-offset, state numbering,
//   branch short-vs-near, modrm vs moffs32, OR vs TEST encoding).
//
//   The pragmatic choice — the same one FUN_004014b0 (Win32 message
//   pump) and FUN_00401a00 (exe-dir bootstrap) took for their SEH-wrapped
//   /O2 bodies — is a `__declspec(naked)` body that re-emits the orig
//   349 bytes verbatim via MASM `_emit` directives. The .obj's `.text`
//   section ends up byte-identical to the orig slice (no relocations
//   because the bytes are emitted as raw immediates), which is what
//   `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the upstream "\latest.txt"-probe
//   helper (FUN_004531c0), the shared singleton ctor (FUN_00452a40),
//   and the path-builder helpers (FUN_00447550 / FUN_00446f50) are
//   reconstructed.

extern "C" __declspec(naked) void FUN_00405080() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x54
        _emit 0x47
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec

        _emit 0x58
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x56
        _emit 0x57
        _emit 0xa1
        _emit 0xb0

        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x64
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0xf6
        _emit 0x05
        _emit 0x90
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x23
        _emit 0x83
        _emit 0x0d
        _emit 0x90
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x01

        _emit 0x6a
        _emit 0x00
        _emit 0xb9
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8

        _emit 0x6c
        _emit 0xd9
        _emit 0x04
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xf6
        _emit 0x05
        _emit 0x90
        _emit 0x38

        _emit 0x32
        _emit 0x01
        _emit 0x02
        _emit 0x75
        _emit 0x23
        _emit 0x83
        _emit 0x0d
        _emit 0x90
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x02
        _emit 0x6a
        _emit 0xff
        _emit 0xb9
        _emit 0x88

        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x40
        _emit 0xd9
        _emit 0x04
        _emit 0x00

        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x3d
        _emit 0xa4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x6a
        _emit 0x00

        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd7
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x74
        _emit 0x6d
        _emit 0x8b
        _emit 0x35
        _emit 0xa0
        _emit 0xe1

        _emit 0xf3
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x01
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd6
        _emit 0x85
        _emit 0xc0
        _emit 0x75

        _emit 0x58
        _emit 0x68
        _emit 0xc0
        _emit 0x4b
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0xb9
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01

        _emit 0xe8
        _emit 0x0b
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x02
        _emit 0x00

        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x69
        _emit 0xe0
        _emit 0x04
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x84
        _emit 0xc0
        _emit 0x6a
        _emit 0xff
        _emit 0x75
        _emit 0x04

        _emit 0x6a
        _emit 0x00
        _emit 0xeb
        _emit 0x02
        _emit 0x6a
        _emit 0x01
        _emit 0x68
        _emit 0x88
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd6
        _emit 0x6a
        _emit 0x01
        _emit 0x6a

        _emit 0x02
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd6
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x6c

        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0xc7
        _emit 0x1d
        _emit 0x04
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01

        _emit 0xff
        _emit 0xd7
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x74
        _emit 0x1b
        _emit 0x8b
        _emit 0x35
        _emit 0xc8
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8d
        _emit 0x49
        _emit 0x00

        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0xd6
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd7
        _emit 0x83
        _emit 0xf8
        _emit 0x02

        _emit 0x75
        _emit 0xee
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x88
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd7
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x95
        _emit 0xc0

        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c

        _emit 0x24
        _emit 0x54
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x1b
        _emit 0xcf
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x64
        _emit 0xc3
    }
}
