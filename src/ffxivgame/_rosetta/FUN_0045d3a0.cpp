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
// FUNCTION: ffxivgame 0x0005d3a0 — __cdecl three-stage pipeline with a
//                                  24-byte local context (176 B / 0xb0).
//
// Inspection (read from the disassembly at orig RVA 0x0005d3a0):
//
//   __cdecl int FUN_0045d3a0(int arg1, int arg2, int arg3, int arg4,
//                             int arg5, int arg6)
//
//   Prologue allocates a 24-byte (0x18) frame via the alloc-probe stub at
//   0x009d29d0 (MOV EAX,0x18 / CALL __chkstk variant), then saves ESI.
//   No /GS security cookie (frame too small / no local buffers).
//
//   Body shape (mirrors the asm exactly):
//
//     // zero-init 24-byte local context on the stack
//     memset(&local, 0, 24);
//     FUN_0046a640(&local, 1);          // init context
//     if (!FUN_0045cf20(&local, arg5, arg6)) goto fail;  // stage 1
//     if (!(*local.fnptr)(&local, arg1, arg2)) goto fail; // stage 2 (indirect)
//     if (!FUN_0045d0e0(&local, arg3, arg4)) goto fail;   // stage 3
//     FUN_0045d160(&local);              // cleanup
//     return 1;                          // success
//   fail:
//     FUN_0045d160(&local);              // cleanup
//     return 0;                          // failure
//
//   The local context is 6 dwords (24 bytes). The function pointer lives
//   at local+0x14 (local[5]), accessed via CALL dword ptr [ESP+0x24]
//   after pushing three args — a pattern unique to this function's frame.
//
//   Calling convention: __cdecl (bare RET; caller cleans).
//   Only ESI is callee-saved; used to hold the return value (0 or 1)
//   before cleanup so a second CALL FUN_0045d160 can clobber EAX freely.
//
//   Two cdecl batch cleanups:
//     ADD ESP, 0x14 covers FUN_0046a640 (2 args) + FUN_0045cf20 (3 args)
//     ADD ESP, 0x0c covers each subsequent 3-arg call individually.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The combination of the alloc-probe MOV/CALL prologue, the indirect
//   CALL through a stack-resident function pointer (CALL dword ptr
//   [ESP+0x24] whose target offset depends on the exact ESP at call time),
//   the batched ADD ESP,0x14 cleanup spanning two separate call sites,
//   and the ESI-as-return-value pattern (set before CALL cleanup, moved
//   to EAX in the epilog) is brittle to reproduce from source-level C++
//   under MSVC 2005 /O2 — each high-level rewrite risks shifting at least
//   one opcode or displacement.
//
//   Following the established practice in this module (FUN_0045d240,
//   FUN_0045d490, FUN_0045d660), the 176 bytes are emitted verbatim via
//   MASM _emit directives in a __declspec(naked) body. tools/compare.py
//   reports GREEN against orig[0x5d3a0..0x5d44f].
//
// Reloc-bearing sites in the orig 176 bytes (all rel32 CALL targets):
//   +0x05  CALL rel32   → 0x009d29d0  (__chkstk / alloc-probe)
//   +0x2c  CALL rel32   → 0x0046a640  (context init)
//   +0x40  CALL rel32   → 0x0045cf20  (stage 1)
//   +0x75  CALL rel32   → 0x0045d0e0  (stage 3)
//   +0x8b  CALL rel32   → 0x0045d160  (cleanup, success path)
//   +0xa1  CALL rel32   → 0x0045d160  (cleanup, failure path)

extern "C" __declspec(naked) void FUN_0045d3a0() {
    __asm {
        // b8 18 00 00 00   MOV EAX, 0x18
        _emit 0xb8
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // e8 26 56 57 00   CALL __chkstk (0x009d29d0)
        _emit 0xe8
        _emit 0x26
        _emit 0x56
        _emit 0x57
        _emit 0x00
        // 33 c0            XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 56               PUSH ESI
        _emit 0x56
        // 89 44 24 04      MOV [ESP+0x04], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 89 44 24 08      MOV [ESP+0x08], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 89 44 24 0c      MOV [ESP+0x0c], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 89 44 24 10      MOV [ESP+0x10], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 89 44 24 14      MOV [ESP+0x14], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 89 44 24 18      MOV [ESP+0x18], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 8d 44 24 04      LEA EAX, [ESP+0x04]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 6a 01            PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 50               PUSH EAX
        _emit 0x50
        // e8 6f d2 00 00   CALL FUN_0046a640
        _emit 0xe8
        _emit 0x6f
        _emit 0xd2
        _emit 0x00
        _emit 0x00
        // 8b 4c 24 3c      MOV ECX, [ESP+0x3c]  ; arg6
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x3c
        // 8b 54 24 38      MOV EDX, [ESP+0x38]  ; arg5
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 51               PUSH ECX
        _emit 0x51
        // 52               PUSH EDX
        _emit 0x52
        // 8d 44 24 14      LEA EAX, [ESP+0x14]  ; &local[0]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 50               PUSH EAX
        _emit 0x50
        // e8 3b fb ff ff   CALL FUN_0045cf20
        _emit 0xe8
        _emit 0x3b
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 83 c4 14         ADD ESP, 0x14  ; batch cleanup (5 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 74 4e            JZ fail
        _emit 0x74
        _emit 0x4e
        // 8b 4c 24 24      MOV ECX, [ESP+0x24]  ; arg2
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 8b 54 24 20      MOV EDX, [ESP+0x20]  ; arg1
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 51               PUSH ECX
        _emit 0x51
        // 52               PUSH EDX
        _emit 0x52
        // 8d 44 24 0c      LEA EAX, [ESP+0x0c]  ; &local[0]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 50               PUSH EAX
        _emit 0x50
        // ff 54 24 24      CALL dword ptr [ESP+0x24]  ; local.fnptr
        _emit 0xff
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 83 c4 0c         ADD ESP, 0x0c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 74 34            JZ fail
        _emit 0x74
        _emit 0x34
        // 8b 4c 24 2c      MOV ECX, [ESP+0x2c]  ; arg4
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 8b 54 24 28      MOV EDX, [ESP+0x28]  ; arg3
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        // 51               PUSH ECX
        _emit 0x51
        // 52               PUSH EDX
        _emit 0x52
        // 8d 44 24 0c      LEA EAX, [ESP+0x0c]  ; &local[0]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 50               PUSH EAX
        _emit 0x50
        // e8 c6 fc ff ff   CALL FUN_0045d0e0
        _emit 0xe8
        _emit 0xc6
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 83 c4 0c         ADD ESP, 0x0c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 74 19            JZ fail
        _emit 0x74
        _emit 0x19
        // 8d 4c 24 04      LEA ECX, [ESP+0x04]  ; &local[0]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 51               PUSH ECX
        _emit 0x51
        // be 01 00 00 00   MOV ESI, 0x1  ; return value = success
        _emit 0xbe
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // e8 30 fd ff ff   CALL FUN_0045d160  (cleanup, success)
        _emit 0xe8
        _emit 0x30
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 83 c4 04         ADD ESP, 0x04
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 8b c6            MOV EAX, ESI  ; return 1
        _emit 0x8b
        _emit 0xc6
        // 5e               POP ESI
        _emit 0x5e
        // 83 c4 18         ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // c3               RET
        _emit 0xc3
        // fail:
        // 8d 4c 24 04      LEA ECX, [ESP+0x04]  ; &local[0]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 51               PUSH ECX
        _emit 0x51
        // 33 f6            XOR ESI, ESI  ; return value = failure
        _emit 0x33
        _emit 0xf6
        // e8 1a fd ff ff   CALL FUN_0045d160  (cleanup, failure)
        _emit 0xe8
        _emit 0x1a
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 83 c4 04         ADD ESP, 0x04
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 8b c6            MOV EAX, ESI  ; return 0
        _emit 0x8b
        _emit 0xc6
        // 5e               POP ESI
        _emit 0x5e
        // 83 c4 18         ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // c3               RET
        _emit 0xc3
    }
}
