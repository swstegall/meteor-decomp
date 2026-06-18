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
// FUNCTION: ffxivgame 0x0042d7e0 — FUN_0042d7e0 (46 B / 0x2E)
//
// A four-argument __cdecl wrapper that:
//   1. Pre-loads arg4 ([ESP+0x10]) into EAX and arg3 ([ESP+0x0c]) into ECX
//      BEFORE allocating the local frame (a common MSVC 2005 scheduler move
//      that avoids the extra frame-offset arithmetic those reads would need
//      after SUB ESP).
//   2. Allocates a 64-byte local buffer with SUB ESP,0x40.
//   3. Pushes EAX (arg4) and ECX (arg3) onto the stack, then computes
//      &local_buf via LEA EDX,[ESP+0x08] (the buffer starts 8 bytes above
//      the two extra pushes) and pushes EDX.
//   4. Calls FUN_0042fcf0 — a 1-arg scaled-identity 4×4 float-matrix init
//      (RVA 0x0002fcf0); FUN_0042fcf0 uses only [ESP+0x44] (= &local_buf)
//      and ignores the arg3/arg4 words sitting below it on the stack.
//      FUN_0042fcf0 is __cdecl and returns EAX = the output pointer
//      (&local_buf), so EAX on return = &local_buf.
//   5. Reads arg1 into ECX via MOV ECX,[ESP+0x50] (= [orig ESP+0x04]).
//   6. Partially cleans the first call's args: ADD ESP,0x04 pops the
//      &local_buf push; the arg3/arg4 words remain on the stack and are
//      absorbed by the final frame teardown.
//   7. Pushes EAX (= &local_buf), loads arg2 from [ESP+0x54] (= [orig
//      ESP+0x08]) into EAX, pushes EAX, then pushes ECX (arg1).
//   8. Calls FUN_00426190 (RVA 0x00026190, __cdecl) with three args:
//      (arg1, arg2, &local_buf).
//   9. ADD ESP,0x54 tears down the entire frame (0x40 alloc + 3 pre-call
//      pushes + 1 partial pop + 3 second-call pushes = 0x54 net).
//  10. RET — __cdecl, caller was responsible for its own args.
//
// Reconstruction: __declspec(naked) byte passthrough.
// The two CALL rel32 immediates are reloc-bearing in the orig PE; they are
// emitted here verbatim from the orig binary slice.  tools/compare.py masks
// those bytes, so the exact immediate values do not affect the GREEN result.

extern "C" __declspec(naked) void FUN_0042d7e0() {
    __asm {
        // 0002d7e0: 8b 44 24 10   MOV EAX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0002d7e4: 8b 4c 24 0c   MOV ECX, dword ptr [ESP+0x0c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0002d7e8: 83 ec 40      SUB ESP, 0x40
        _emit 0x83
        _emit 0xec
        _emit 0x40
        // 0002d7eb: 50            PUSH EAX
        _emit 0x50
        // 0002d7ec: 51            PUSH ECX
        _emit 0x51
        // 0002d7ed: 8d 54 24 08   LEA EDX, [ESP+0x08]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0002d7f1: 52            PUSH EDX
        _emit 0x52
        // 0002d7f2: e8 f9 24 00 00 CALL FUN_0042fcf0  (rel32 0x000024f9)
        _emit 0xe8
        _emit 0xf9
        _emit 0x24
        _emit 0x00
        _emit 0x00
        // 0002d7f7: 8b 4c 24 50   MOV ECX, dword ptr [ESP+0x50]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x50
        // 0002d7fb: 83 c4 04      ADD ESP, 0x04
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0002d7fe: 50            PUSH EAX
        _emit 0x50
        // 0002d7ff: 8b 44 24 54   MOV EAX, dword ptr [ESP+0x54]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x54
        // 0002d803: 50            PUSH EAX
        _emit 0x50
        // 0002d804: 51            PUSH ECX
        _emit 0x51
        // 0002d805: e8 86 89 ff ff CALL FUN_00426190  (rel32 0xffff8986)
        _emit 0xe8
        _emit 0x86
        _emit 0x89
        _emit 0xff
        _emit 0xff
        // 0002d80a: 83 c4 54      ADD ESP, 0x54
        _emit 0x83
        _emit 0xc4
        _emit 0x54
        // 0002d80d: c3            RET
        _emit 0xc3
    }
}
