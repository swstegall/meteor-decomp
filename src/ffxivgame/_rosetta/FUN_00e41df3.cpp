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
// FUNCTION: ffxivgame 0x00a41df3 — `__cdecl` 3-arg forwarding wrapper (29 B).
//
// Forwards three caller args plus two constants into a 5-arg `__cdecl`
// callee at FUN_00e41cb6, using the address of the caller's third argument
// (passed by pointer) as the fifth arg.  The caller cleans up 5 dwords
// (0x14 bytes) via `ADD ESP, 0x14` before returning.
//
// Asm shape (29 bytes — RVA 0x00a41df3..0x00a41e10):
//
//     00a41df3:  8d 44 24 0c          LEA  EAX, [ESP+0xC]    ; &arg3
//     00a41df7:  50                   PUSH EAX               ; push &arg3 (5th arg of callee)
//     00a41df8:  6a 00                PUSH 0                 ; 4th arg of callee = 0
//     00a41dfa:  ff 74 24 10          PUSH [ESP+0x10]        ; 3rd arg of callee = arg2
//     00a41dfe:  ff 74 24 10          PUSH [ESP+0x10]        ; 2nd arg of callee = arg1
//     00a41e02:  68 99 a8 9e 00       PUSH 0x9ea899          ; 1st arg of callee
//     00a41e07:  e8 aa fe ff ff       CALL FUN_00e41cb6      ; rel32 = -0x156
//     00a41e0c:  83 c4 14             ADD  ESP, 0x14         ; cdecl cleanup (5 args)
//     00a41e0f:  c3                   RET
//
// Stack offsets for the two `PUSH [ESP+0x10]` reloads:
//   - At the first reload (after 2 pushes: EAX and 0), arg2 has shifted
//     from [ESP+8] to [ESP+0x10] (+8 offset from 2 dword pushes).
//   - At the second reload (after 3 pushes), arg1 has shifted from
//     [ESP+4] to [ESP+0x10] (+12 offset from 3 dword pushes → original
//     [ESP+4] is now at [ESP+0x10]).
//
// Reloc-bearing site in the orig 29 bytes:
//     +0x14   CALL rel32 → FUN_00e41cb6 (rel32 = 0xfffffeaa)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.

extern "C" __declspec(naked) void FUN_00e41df3() {
    __asm {
        _emit 0x8d    // LEA  EAX, [ESP+0xC]      ; &arg3
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x50    // PUSH EAX                 ; push &arg3 (5th callee arg)
        _emit 0x6a    // PUSH 0                   ; 4th callee arg = 0
        _emit 0x00
        _emit 0xff    // PUSH [ESP+0x10]           ; 3rd callee arg = arg2
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0xff    // PUSH [ESP+0x10]           ; 2nd callee arg = arg1
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x68    // PUSH 0x9ea899             ; 1st callee arg
        _emit 0x99
        _emit 0xa8
        _emit 0x9e
        _emit 0x00
        _emit 0xe8    // CALL FUN_00e41cb6         ; rel32 = 0xfffffeaa
        _emit 0xaa
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83    // ADD  ESP, 0x14            ; cdecl cleanup (5 args)
        _emit 0xc4
        _emit 0x14
        _emit 0xc3    // RET
    }
}
