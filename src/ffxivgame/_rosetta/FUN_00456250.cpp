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
// FUNCTION: ffxivgame 0x00056250 — guarded one-shot reset / error path (51 B / 0x33)
//
// Reads the global init/handle slot at 0x0126701c. If it still holds the
// sentinel -1 (uninitialised), it fetches a singleton via a __thiscall
// getter (ECX = &g_singleton at 0x0132d0e0) and zeroes the two 16-bit
// fields at +0x88 / +0x8a of the returned object, then returns. Otherwise
// the slot holds a real value: it is pushed to an imported CRT helper
// (call through the IAT slot at 0x00f3e2a4 — returns a pointer, e.g. the
// per-thread errno cell), whose target is then stamped with 0xffffffff.
//
// Asm (51 bytes @ orig RVA 0x00056250):
//   a1 1c 70 26 01           MOV  EAX, [0x0126701c]
//   83 f8 ff                 CMP  EAX, -1
//   75 1b                    JNZ  err_path (+0x1b)
//   b9 e0 d0 32 01           MOV  ECX, 0x0132d0e0     ; this
//   e8 0c 10 00 00           CALL 0x00457270          ; rel32 getter
//   33 c9                    XOR  ECX, ECX
//   66 89 88 88 00 00 00     MOV  word ptr [EAX+0x88], CX
//   66 89 88 8a 00 00 00     MOV  word ptr [EAX+0x8a], CX
//   c3                       RET
// err_path:
//   50                       PUSH EAX
//   ff 15 a4 e2 f3 00        CALL dword ptr [0x00f3e2a4]  ; IAT import
//   c7 00 ff ff ff ff        MOV  dword ptr [EAX], 0xffffffff
//   c3                       RET
//
// Calling convention: __cdecl — no args, void return.
// Frame: none.
//
// Reconstruction: __declspec(naked) _emit byte passthrough (mirrors the
// sibling FUN_004051e0 / FUN_00416320). The rel32 CALL and the IAT-
// indirect CALL immediates are baked into the orig binary's address space
// and re-emitted as raw bytes, so the .obj's .text is byte-identical with
// no relocations; tools/compare.py masks reloc bytes and reports GREEN.

extern "C" __declspec(naked) void FUN_00456250() {
    __asm {
        // 00056250: a1 1c 70 26 01   MOV EAX, [0x0126701c]
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 00056255: 83 f8 ff         CMP EAX, -1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 00056258: 75 1b            JNZ err_path (+0x1b)
        _emit 0x75
        _emit 0x1b
        // 0005625a: b9 e0 d0 32 01   MOV ECX, 0x0132d0e0
        _emit 0xb9
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // 0005625f: e8 0c 10 00 00   CALL 0x00457270 (rel32)
        _emit 0xe8
        _emit 0x0c
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 00056264: 33 c9            XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 00056266: 66 89 88 88 00 00 00  MOV word ptr [EAX+0x88], CX
        _emit 0x66
        _emit 0x89
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005626d: 66 89 88 8a 00 00 00  MOV word ptr [EAX+0x8a], CX
        _emit 0x66
        _emit 0x89
        _emit 0x88
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00056274: c3               RET
        _emit 0xc3
        // 00056275: 50               PUSH EAX
        _emit 0x50
        // 00056276: ff 15 a4 e2 f3 00  CALL dword ptr [0x00f3e2a4]
        _emit 0xff
        _emit 0x15
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // 0005627c: c7 00 ff ff ff ff  MOV dword ptr [EAX], 0xffffffff
        _emit 0xc7
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00056282: c3               RET
        _emit 0xc3
    }
}
