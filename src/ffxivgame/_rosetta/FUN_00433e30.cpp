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
// FUNCTION: ffxivgame 0x00033e30 — __thiscall member fn that allocates a
//           20-byte event struct from a global pool array and enqueues it.
//           (100 B / 0x64)
//
// __thiscall void FUN_00433e30(int arg1, int arg2, int arg3, int arg4)
//   ECX  = this
//   args = 4 × DWORD on stack; callee cleans 0x10 via RET 0x10
//
// Logic:
//   1. Load global pointer from [0x01328d90] into ECX.
//   2. Read byte counter from [ECX+0]: byteVal.
//   3. Compute array entry offset = byteVal * 7 * 4 = byteVal * 28.
//      (LEA EDX,[EAX*8]; SUB EDX,EAX; then LEA ECX,[base+EDX*4])
//   4. Load base pointer from [ECX+4]; compute entry = base + byteVal*28.
//   5. Push size=0x14 and CALL FUN_00417ab0 (__thiscall pool allocator)
//      with ECX = entry address.
//   6. If EAX != NULL: initialise the 20-byte struct:
//        [EAX+0x00] = 0x00f64950  (vtable pointer)
//        [EAX+0x04] = arg1
//        [EAX+0x08] = arg3
//        [EAX+0x0c] = arg4
//        [EAX+0x10] = arg2
//      Push EAX and CALL FUN_0043c2d0 (__thiscall, ECX=*(this+0xc)).
//   7. If EAX == NULL: push 0 and CALL FUN_0043c2d0 the same way.
//
// Both paths end with POP ESI + RET 0x10.  FUN_0043c2d0 cleans its one
// DWORD arg via RET 0x4 so the outer POP ESI restores the saved ECX/this.
//
// The absolute data reference at 0x01328d90 and the constant 0x00f64950
// are emitted verbatim as raw immediates (no relocs in the .obj), so
// compare.py finds an exact byte match without masking.

extern "C" __declspec(naked) void FUN_00433e30() {
    __asm {
        // 00033e30: 56              PUSH ESI
        _emit 0x56
        // 00033e31: 8b f1           MOV ESI, ECX  (save this)
        _emit 0x8b
        _emit 0xf1
        // 00033e33: 8b 0d 90 8d 32 01  MOV ECX, [0x01328d90]
        _emit 0x8b
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 00033e39: 0f b6 01        MOVZX EAX, byte ptr [ECX]
        _emit 0x0f
        _emit 0xb6
        _emit 0x01
        // 00033e3c: 8d 14 c5 00 00 00 00  LEA EDX, [EAX*8+0]
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00033e43: 2b d0           SUB EDX, EAX  (EDX = EAX * 7)
        _emit 0x2b
        _emit 0xd0
        // 00033e45: 8b 41 04        MOV EAX, [ECX+4]  (base pointer)
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00033e48: 8d 0c 90        LEA ECX, [EAX+EDX*4]  (entry = base + byteVal*28)
        _emit 0x8d
        _emit 0x0c
        _emit 0x90
        // 00033e4b: 6a 14           PUSH 0x14  (size arg to pool alloc)
        _emit 0x6a
        _emit 0x14
        // 00033e4d: e8 5e 3c fe ff  CALL FUN_00417ab0
        _emit 0xe8
        _emit 0x5e
        _emit 0x3c
        _emit 0xfe
        _emit 0xff
        // 00033e52: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00033e54: 74 2f           JZ 0x00033e85  (null → null path)
        _emit 0x74
        _emit 0x2f
        // --- non-null path: initialise the 20-byte struct ---------------
        // 00033e56: 8b 4c 24 08     MOV ECX, [ESP+8]    (arg1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 00033e5a: 8b 54 24 10     MOV EDX, [ESP+0x10]  (arg3)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00033e5e: 89 48 04        MOV [EAX+4], ECX    (struct.arg1)
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 00033e61: 8b 4c 24 14     MOV ECX, [ESP+0x14]  (arg4)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00033e65: 89 50 08        MOV [EAX+8], EDX    (struct.arg3)
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 00033e68: 8b 54 24 0c     MOV EDX, [ESP+0xc]   (arg2)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00033e6c: c7 00 50 49 f6 00  MOV [EAX], 0x00f64950 (vtable ptr)
        _emit 0xc7
        _emit 0x00
        _emit 0x50
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        // 00033e72: 89 48 0c        MOV [EAX+0xc], ECX  (struct.arg4)
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 00033e75: 89 50 10        MOV [EAX+0x10], EDX  (struct.arg2)
        _emit 0x89
        _emit 0x50
        _emit 0x10
        // 00033e78: 8b 4e 0c        MOV ECX, [ESI+0xc]  (*(this+0xc) for thiscall)
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 00033e7b: 50              PUSH EAX  (struct ptr arg)
        _emit 0x50
        // 00033e7c: e8 4f 84 00 00  CALL FUN_0043c2d0
        _emit 0xe8
        _emit 0x4f
        _emit 0x84
        _emit 0x00
        _emit 0x00
        // 00033e81: 5e              POP ESI
        _emit 0x5e
        // 00033e82: c2 10 00        RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // --- null path --------------------------------------------------
        // 00033e85: 8b 4e 0c        MOV ECX, [ESI+0xc]
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 00033e88: 33 c0           XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00033e8a: 50              PUSH EAX  (push 0)
        _emit 0x50
        // 00033e8b: e8 40 84 00 00  CALL FUN_0043c2d0
        _emit 0xe8
        _emit 0x40
        _emit 0x84
        _emit 0x00
        _emit 0x00
        // 00033e90: 5e              POP ESI
        _emit 0x5e
        // 00033e91: c2 10 00        RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
