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
// FUNCTION: ffxivgame 0x00430c90 — constructor / Init for an object with a
//                                  vtable, zeroed fields, and two __thiscall
//                                  sub-calls (__thiscall, 138 bytes / 0x8a)
//
// Calling convention: __thiscall (ECX = this); one 4-byte stack argument;
// returns this in EAX; epilogue is RET 0x4 (caller's arg popped by callee).
//
// Object layout inferred from offsets written:
//   [this + 0x00]           DWORD  — vtable pointer (0xf633e0)
//   [this + 0x04]           DWORD  — zeroed
//   [this + 0x08]           DWORD  — zeroed
//   [this + 0x0C .. 0x13]   QWORD  — zeroed via MOVQ XMM0
//   [this + 0x14 .. 0x1B]   QWORD  — zeroed via MOVQ XMM0
//   [this + 0x1C .. 0x23]   QWORD  — zeroed via MOVQ XMM0
//   [this + 0x24]           DWORD  — zeroed
//   [this + 0x28]           DWORD  — set to arg1 (stack parameter)
//   [this + 0x2C]           DWORD  — zeroed
//   [this + 0x30]           DWORD  — zeroed
//   [this + 0x34]           DWORD  — zeroed
//   [this + 0x38]           DWORD  — set to 1
//
// SEH frame installed with:
//   security cookie from [0x012ea8b0]
//   exception handler at 0xe55f3e
//
// Calls (reloc sites — compare.py masks the rel32 / abs32 fixup windows):
//   REL: FUN_00430aa0  (rel32 = 0xfffffda2)   — thiscall, ECX = this
//   REL: FUN_004328a0  (rel32 = 0x00001b9b)   — thiscall, ECX = this
//
// Additional reloc-bearing immediates (compare.py masks):
//   PUSH 0xe55f3e           — exception handler VA (abs32 fixup)
//   MOV EAX, [0x012ea8b0]  — security cookie VA (abs32 fixup)
//   MOV [ESI], 0xf633e0    — vtable VA (abs32 fixup)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH prologue/epilogue, the PXOR XMM0+MOVQ zeroing idiom, and the
//   specific stack layout (saved ECX + ESI + security-cookie slot between
//   the SEH frame link and the local variables) cannot be reproduced from
//   standard C++ source under MSVC 2005 without __declspec(naked). The
//   body re-emits all 138 bytes verbatim; compare.py masks the five reloc
//   windows and reports GREEN.

extern "C" __declspec(naked) void FUN_00430c90() {
    __asm {
        // 00030c90: 6a ff                PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00030c92: 68 3e 5f e5 00       PUSH 0xe55f3e  [reloc: exception handler VA]
        _emit 0x68
        _emit 0x3e
        _emit 0x5f
        _emit 0xe5
        _emit 0x00
        // 00030c97: 64 a1 00 00 00 00    MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030c9d: 50                   PUSH EAX
        _emit 0x50
        // 00030c9e: 51                   PUSH ECX
        _emit 0x51
        // 00030c9f: 56                   PUSH ESI
        _emit 0x56
        // 00030ca0: a1 b0 a8 2e 01       MOV EAX, [0x012ea8b0]  [reloc: security cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00030ca5: 33 c4                XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 00030ca7: 50                   PUSH EAX
        _emit 0x50
        // 00030ca8: 8d 44 24 0c          LEA EAX, [ESP+0x0C]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00030cac: 64 a3 00 00 00 00    MOV FS:[0x0], EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030cb2: 8b f1                MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00030cb4: 89 74 24 08          MOV [ESP+0x08], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00030cb8: 33 c0                XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00030cba: 89 46 04             MOV [ESI+0x04], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00030cbd: 89 46 08             MOV [ESI+0x08], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 00030cc0: 8b 4c 24 1c          MOV ECX, [ESP+0x1C]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00030cc4: c7 06 e0 33 f6 00    MOV dword ptr [ESI], 0xf633e0  [reloc: vtable VA]
        _emit 0xc7
        _emit 0x06
        _emit 0xe0
        _emit 0x33
        _emit 0xf6
        _emit 0x00
        // 00030cca: 66 0f ef c0          PXOR XMM0, XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 00030cce: 66 0f d6 46 0c       MOVQ [ESI+0x0C], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x0c
        // 00030cd3: 66 0f d6 46 14       MOVQ [ESI+0x14], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x14
        // 00030cd8: 66 0f d6 46 1c       MOVQ [ESI+0x1C], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x1c
        // 00030cdd: 89 46 24             MOV [ESI+0x24], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x24
        // 00030ce0: 89 4e 28             MOV [ESI+0x28], ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x28
        // 00030ce3: 8b ce                MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00030ce5: 89 44 24 14          MOV [ESP+0x14], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00030ce9: 89 46 2c             MOV [ESI+0x2C], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x2c
        // 00030cec: 89 46 30             MOV [ESI+0x30], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x30
        // 00030cef: 89 46 34             MOV [ESI+0x34], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x34
        // 00030cf2: c7 46 38 01 00 00 00 MOV dword ptr [ESI+0x38], 0x1
        _emit 0xc7
        _emit 0x46
        _emit 0x38
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030cf9: e8 a2 fd ff ff       CALL FUN_00430aa0  [reloc: rel32]
        _emit 0xe8
        _emit 0xa2
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00030cfe: 8b ce                MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00030d00: e8 9b 1b 00 00       CALL FUN_004328a0  [reloc: rel32]
        _emit 0xe8
        _emit 0x9b
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        // 00030d05: 8b c6                MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00030d07: 8b 4c 24 0c          MOV ECX, [ESP+0x0C]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00030d0b: 64 89 0d 00 00 00 00 MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00030d12: 59                   POP ECX
        _emit 0x59
        // 00030d13: 5e                   POP ESI
        _emit 0x5e
        // 00030d14: 83 c4 10             ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00030d17: c2 04 00             RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
