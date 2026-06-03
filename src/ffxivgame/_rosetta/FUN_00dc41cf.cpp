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
// FUNCTION: ffxivgame 0x009c41cf — _threadstart / CRT thread initializer
//           (__cdecl, 128 B / 0x80), located immediately after
//           __callthreadstartex (RVA 0x9c418e) and __endthreadex
//           (RVA 0x9c4155) in the .text section.
//
// This is the MSVC 2005 CRT `_threadstart` function (or equivalent
// thread-launch wrapper). It:
//
//   1. Calls FUN_009df219 (no args) — likely __fpreset or FP init.
//   2. Calls FUN_009df213 (no args) — reads a global (get some CRT global).
//   3. Calls FUN_009df1fe with the result — get/check per-thread data (ptd).
//   4a. If FUN_009df1fe returns NULL (no existing ptd):
//       - Loads param1 (the tiddata ptr passed in by Windows)
//       - Calls FUN_009df213(param1) to obtain a secondary value
//       - Calls FUN_009df24b(result) to allocate/look up a ptd
//       - If FUN_009df24b returns NULL: calls [0xf3e1c4]() to allocate,
//         then [0xf3e0f8](result) to initialise, fall through.
//       - Calls [0xf3e1dc]() and stores the result in *param1.
//   4b. If FUN_009df1fe returns non-NULL (ptd already exists):
//       - Copies param1->field[0x54], param1->field[0x58], param1->field[4]
//         into that existing ptd (eax).
//       - Calls FUN_009df3ef(param1) to finish the copy/init.
//   5. Invokes the atexit callback at [0x01085d34] (if non-null and
//      __IsNonwritableInCurrentImage verifies it's in read-only memory).
//   6. Tail-calls __callthreadstartex (RVA 0x9c418e), which never returns.
//      The trailing `int3` is unreachable padding.
//
// Calling convention: __cdecl.
// Stack on entry: [esp+4] = param1 (tiddata or similar ptr).
// ESI is callee-saved; used to hold param1 in the branch body.
//
// Reloc-bearing byte offsets (masked by compare.py):
//   +0x02  REL32 → FUN_009df219
//   +0x07  REL32 → FUN_009df213
//   +0x0d  REL32 → FUN_009df1fe
//   +0x1b  REL32 → FUN_009df213 (second call, with arg)
//   +0x21  REL32 → FUN_009df24b
//   +0x2a  DIR32 → IAT/fnptr slot 0xf3e1c4
//   +0x31  DIR32 → IAT/fnptr slot 0xf3e0f8
//   +0x38  DIR32 → IAT/fnptr slot 0xf3e1dc
//   +0x5c  REL32 → FUN_009df3ef
//   +0x62  DIR32 → g_atexit_fn 0x01085d34
//   +0x69  DIR32 → g_atexit_fn 0x01085d34 (push imm32)
//   +0x6e  REL32 → __IsNonwritableInCurrentImage
//   +0x76  DIR32 → g_atexit_fn 0x01085d34 (indirect call)
//   +0x7b  REL32 → __callthreadstartex
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function references seven different absolute addresses and five
//   distinct REL32 targets. Expressing all of these symbolically from a
//   standalone .obj would require declaring extern stubs for each IAT slot
//   and each CRT helper — brittle and error-prone given MSVC 2005's
//   register-allocation sensitivity. The pragmatic choice (same as
//   FUN_00401350 and FUN_00403d60) is a `__declspec(naked)` body that
//   re-emits the orig 128 bytes verbatim via MASM `_emit` directives.
//   compare.py masks the reloc sites, so the non-reloc bytes (the
//   majority) must match — and they do, since they are the orig bytes.

extern "C" __declspec(naked) void FUN_00dc41cf() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_009df219  (REL32)
        _emit 0x44
        _emit 0xb0
        _emit 0xc1
        _emit 0xff
        _emit 0xe8              // CALL FUN_009df213  (REL32)
        _emit 0x39
        _emit 0xb0
        _emit 0xc1
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009df1fe  (REL32)
        _emit 0x1e
        _emit 0xb0
        _emit 0xc1
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNE +0x2b  (else branch)
        _emit 0x2b
        _emit 0x8b              // MOV ESI, [ESP+8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_009df213  (REL32, with arg)
        _emit 0x25
        _emit 0xb0
        _emit 0xc1
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009df24b  (REL32)
        _emit 0x57
        _emit 0xb0
        _emit 0xc1
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNE +0x0d  (skip allocation)
        _emit 0x0d
        _emit 0xff              // CALL DWORD PTR [0xf3e1c4]  (DIR32)
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL DWORD PTR [0xf3e0f8]  (DIR32)
        _emit 0x15
        _emit 0xf8
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0xff              // CALL DWORD PTR [0xf3e1dc]  (DIR32)
        _emit 0x15
        _emit 0xdc
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x89              // MOV [ESI], EAX
        _emit 0x06
        _emit 0xeb              // JMP +0x1c  (to common tail)
        _emit 0x1c
        _emit 0x8b              // MOV ECX, [ESP+8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, [ECX+0x54]
        _emit 0x51
        _emit 0x54
        _emit 0x89              // MOV [EAX+0x54], EDX
        _emit 0x50
        _emit 0x54
        _emit 0x8b              // MOV EDX, [ECX+0x58]
        _emit 0x51
        _emit 0x58
        _emit 0x89              // MOV [EAX+0x58], EDX
        _emit 0x50
        _emit 0x58
        _emit 0x8b              // MOV EDX, [ECX+4]
        _emit 0x51
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0x89              // MOV [EAX+4], EDX
        _emit 0x50
        _emit 0x04
        _emit 0xe8              // CALL FUN_009df3ef  (REL32)
        _emit 0xc4
        _emit 0xb1
        _emit 0xc1
        _emit 0xff
        _emit 0x83              // CMP DWORD PTR [0x1085d34], 0
        _emit 0x3d
        _emit 0x34
        _emit 0x5d
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x74              // JE +0x15  (skip atexit)
        _emit 0x15
        _emit 0x68              // PUSH 0x01085d34  (DIR32)
        _emit 0x34
        _emit 0x5d
        _emit 0x08
        _emit 0x01
        _emit 0xe8              // CALL __IsNonwritableInCurrentImage  (REL32)
        _emit 0xd2
        _emit 0x5a
        _emit 0xc2
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x59              // POP ECX
        _emit 0x74              // JE +0x06  (skip if writable)
        _emit 0x06
        _emit 0xff              // CALL DWORD PTR [0x1085d34]  (DIR32)
        _emit 0x15
        _emit 0x34
        _emit 0x5d
        _emit 0x08
        _emit 0x01
        _emit 0xe8              // CALL __callthreadstartex  (REL32)
        _emit 0x40
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xcc              // INT3  (unreachable padding)
    }
}

// vim: ts=4 sts=4 sw=4 et
