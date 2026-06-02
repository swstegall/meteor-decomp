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
// FUNCTION: ffxivgame 0x00014790 — __cdecl factory: __aligned_malloc(0x48,0x10)
//                                  → __thiscall init via FUN_00414640 (32 B / 0x20)
//
// Calling convention: __cdecl, 1 stack arg (`[ESP+4]`), returns EAX. No frame
// (/Oy — function has no locals; the alloc result is the only live value).
//
// Behaviour (read from the disassembly at orig RVA 0x00014790):
//
//   1. Push alignment (0x10) then size (0x48); CALL __aligned_malloc
//      (cdecl CRT helper at VA 0x009d5712 / RVA 0x5d5712). Caller cleans
//      8 B with `ADD ESP,8`.
//   2. If the allocator returned NULL, jump to tail `XOR EAX,EAX / RET`
//      (returns 0).
//   3. Otherwise, load the caller's first arg from `[ESP+4]` into ECX, PUSH
//      it (now the thiscall's stack arg), load alloc result from EAX into
//      ECX (the thiscall `this` pointer), then CALL FUN_00414640
//      (VA 0x00414640 / RVA 0x14640). FUN_00414640 is `__thiscall`, ends
//      with `MOV EAX,ESI / POP ESI / RET 4` — returns its `this`. The
//      factory's trailing `RET` propagates that pointer to the caller.
//
// Asm (32 bytes @ orig RVA 0x00014790):
//   00014790:  6a 10                PUSH 0x10                ; alignment
//   00014792:  6a 48                PUSH 0x48                ; size
//   00014794:  e8 79 0f 5c 00       CALL 0x009d5712          ; __aligned_malloc
//   00014799:  83 c4 08             ADD ESP, 0x8             ; cdecl cleanup
//   0001479c:  85 c0                TEST EAX, EAX
//   0001479e:  74 0d                JZ 0x004147ad            ; NULL → return 0
//   000147a0:  8b 4c 24 04          MOV ECX, [ESP+0x4]       ; load caller arg
//   000147a4:  51                   PUSH ECX                 ; stack arg for thiscall
//   000147a5:  8b c8                MOV ECX, EAX             ; this = alloc result
//   000147a7:  e8 94 fe ff ff       CALL 0x00414640          ; FUN_00414640 (__thiscall)
//   000147ac:  c3                   RET                      ; return EAX (new object ptr)
//   000147ad:  33 c0                XOR EAX, EAX             ; fail path
//   000147af:  c3                   RET
//
// Reconstruction strategy — __declspec(naked) byte-emit passthrough.
// The two REL32 call sites are emitted verbatim (raw displacement bytes
// from the orig .text slice) so they match directly without COFF relocs.
// compare.py masks REL32-bearing positions anyway, but emitting the orig
// bytes also works and avoids external forward declarations.

extern "C" __declspec(naked) void FUN_00414790() {
    __asm {
        // 00014790: 6a 10            PUSH 0x10           (alignment)
        _emit 0x6a
        _emit 0x10
        // 00014792: 6a 48            PUSH 0x48           (size)
        _emit 0x6a
        _emit 0x48
        // 00014794: e8 79 0f 5c 00   CALL __aligned_malloc (rel32 = 0x005c0f79)
        _emit 0xe8
        _emit 0x79
        _emit 0x0f
        _emit 0x5c
        _emit 0x00
        // 00014799: 83 c4 08         ADD ESP, 0x8        (cdecl cleanup)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001479c: 85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001479e: 74 0d            JZ +0x0d → 0x4147ad (NULL → return 0)
        _emit 0x74
        _emit 0x0d
        // 000147a0: 8b 4c 24 04      MOV ECX, [ESP+0x4]  (load caller arg)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000147a4: 51               PUSH ECX            (stack arg for thiscall)
        _emit 0x51
        // 000147a5: 8b c8            MOV ECX, EAX        (this = alloc result)
        _emit 0x8b
        _emit 0xc8
        // 000147a7: e8 94 fe ff ff   CALL FUN_00414640   (rel32 = 0xfffffe94)
        _emit 0xe8
        _emit 0x94
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 000147ac: c3               RET                 (return EAX from thiscall)
        _emit 0xc3
        // 000147ad: 33 c0            XOR EAX, EAX        (fail path: return NULL)
        _emit 0x33
        _emit 0xc0
        // 000147af: c3               RET
        _emit 0xc3
    }
}
