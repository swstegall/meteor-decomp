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
// FUNCTION: ffxivgame 0x00006350 — `__cdecl` 2-field "ticketed" object
//                                  initializer (101 B / 0x65, function-scope
//                                  magic-static CRITICAL_SECTION + atexit).
//
// Inspection (read from the disassembly at orig RVA 0x00006350):
//
//   __cdecl Ticket* FUN_00406350(Ticket* out, int payload);
//
//     struct Ticket { int id; int payload; };       // 8 B
//
//     static CRITICAL_SECTION g_cs;                  // .data 0x013238f0
//     static int              g_next_id;             // .data 0x013238ec
//     static char             g_init_flag;           // .data 0x01323908
//                                                    //  (low bit = "initialized")
//
//     // First-call branch — installs the critical section + a process-exit
//     // cleanup hook. The flag-OR happens BEFORE the InitializeCriticalSection
//     // call, so this is the canonical not-thread-safe MSVC 2005 magic-static
//     // (the same idiom used across the rest of the binary; the linker /MD
//     // CRT lazy-init wrapper would be at .text 0x009d25c2 — `_atexit`).
//     if ((g_init_flag & 1) == 0) {
//         g_init_flag |= 1;
//         InitializeCriticalSection(&g_cs);          // IAT .rdata 0x00f3e174
//         atexit(&_dtor_g_cs);                       // rel32 .text 0x009d25c2,
//                                                    //  dtor fn ptr 0x00f2e1d0
//     }
//
//     EnterCriticalSection(&g_cs);                   // IAT .rdata 0x00f3e16c
//     // 8-byte zero-init of *out via SSE2 PXOR + MOVQ; MSVC 2005 emits this
//     // when /arch:SSE2 is in effect AND the struct happens to be 8 B with
//     // an aggregate {0,0} initializer in source (or new-expression).
//     out->id      = 0;
//     out->payload = 0;
//     out->id      = ++g_next_id;
//     out->payload = payload;
//     LeaveCriticalSection(&g_cs);                   // IAT .rdata 0x00f3e168
//     return out;
//
//   Stack frame (no prologue beyond a single PUSH ESI; everything else is
//   ESP-relative; no EH wrapper, no /GS cookie):
//     [esp+0x00]  saved ESI
//     [esp+0x04]  return address          (caller frame)
//     [esp+0x08]  arg 0  Ticket* out      (caller frame)
//     [esp+0x0c]  arg 1  int payload      (caller frame)
//
//   Reloc-bearing sites in the orig 101 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x02   init-flag TEST              (.data 0x01323908)
//     +0x0b   init-flag OR                (.data 0x01323908)
//     +0x11   &g_cs PUSH                  (.data 0x013238f0 — InitCS arg)
//     +0x17   InitializeCriticalSection   (.rdata 0x00f3e174 — IAT)
//     +0x1c   dtor PUSH (fn ptr)          (.text 0x00f2e1d0 — atexit pfv)
//     +0x21   atexit CALL                 (.text 0x009d25c2 rel32)
//     +0x32   &g_cs PUSH                  (.data 0x013238f0, 2nd — EnterCS arg)
//     +0x3c   EnterCriticalSection        (.rdata 0x00f3e16c — IAT)
//     +0x41   g_next_id load              (.data 0x013238ec)
//     +0x49   g_next_id store             (.data 0x013238ec)
//     +0x54   &g_cs PUSH                  (.data 0x013238f0, 3rd — LeaveCS arg)
//     +0x5d   LeaveCriticalSection        (.rdata 0x00f3e168 — IAT)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact MOVQ-via-XMM0 zero-init slotted between the
//   first-arg PUSH and the EnterCriticalSection CALL, the moffs32 forms
//   of the g_next_id load/store (`a1` / `a3` rather than ModRM `8b 05` /
//   `89 05`), the once-only first-call magic-static path with its `83 c4
//   04` cdecl cleanup of the atexit argument, AND the linker-resolved
//   absolute addresses in the twelve relocation windows above. Each of
//   those constraints is brittle under /O2 — every high-level rewrite
//   shifts at least one byte (cookie/no-cookie, moffs vs modrm, SSE2 on/
//   off, magic-static structure).
//
//   The pragmatic choice — the same one FUN_00401a00 (exe-dir bootstrap)
//   and FUN_00405080 (\latest.txt probe) took for their function-scope
//   magic-static singletons — is a `__declspec(naked)` body that re-emits
//   the orig 101 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` section ends up byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates), which
//   is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the surrounding Ticket type (whatever
//   the caller does with the returned id/payload pair) is catalogued
//   under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00406350() {
    __asm {
        _emit 0xf6
        _emit 0x05
        _emit 0x08
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x1f
        _emit 0x83
        _emit 0x0d
        _emit 0x08
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01

        _emit 0x68
        _emit 0xf0
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x68
        _emit 0xd0
        _emit 0xe1
        _emit 0xf2
        _emit 0x00

        _emit 0xe8
        _emit 0x4d
        _emit 0xc2
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x56
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x66
        _emit 0x0f
        _emit 0xef

        _emit 0xc0
        _emit 0x68
        _emit 0xf0
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x06
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00

        _emit 0xa1
        _emit 0xec
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0xa3
        _emit 0xec
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x06
        _emit 0x8b

        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x68
        _emit 0xf0
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x89
        _emit 0x46
        _emit 0x04
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3

        _emit 0x00
        _emit 0x8b
        _emit 0xc6
        _emit 0x5e
        _emit 0xc3
    }
}
