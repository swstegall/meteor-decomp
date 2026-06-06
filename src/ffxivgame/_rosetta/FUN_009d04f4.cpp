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
// FUNCTION: ffxivgame 0x005d04f4 — FUN_009d04f4 (41 B / 0x29)
//
//   void __cdecl FUN_009d04f4()
//
// Calls an import at [0x00f3e2d0] with arg 0x12ea664. If the return value
// is >= 0, returns immediately. Otherwise iterates over a fixed-size array
// at [0x1363c40, 0x1363ca0) with stride 0x18, calling FUN_009d178d for
// each element.
//
// Calling convention: __cdecl — no args, void return, plain RET.
// Frame: none (/Oy — function uses ESI but no frame pointer).
//
// Asm (41 bytes @ orig RVA 0x005d04f4):
//   68 64 a6 2e 01          PUSH 0x12ea664
//   ff 15 d0 e2 f3 00       CALL dword ptr [0x00f3e2d0]
//   85 c0                   TEST EAX, EAX
//   7d 19                   JGE +0x19 (to 005d051c)
//   56                      PUSH ESI
//   be 40 3c 36 01          MOV ESI, 0x1363c40
//   56                      PUSH ESI              (loop top: 005d0509)
//   e8 7e 12 00 00          CALL 0x009d178d        (reloc)
//   83 c6 18                ADD ESI, 0x18
//   81 fe a0 3c 36 01       CMP ESI, 0x1363ca0
//   59                      POP ECX
//   7c ee                   JL -0x12 (back to 005d0509)
//   5e                      POP ESI
//   c3                      RET

extern "C" void FUN_009d178d();

extern "C" __declspec(naked) void FUN_009d04f4() {
    __asm {
        // 005d04f4: 68 64 a6 2e 01   PUSH 0x12ea664
        _emit 0x68
        _emit 0x64
        _emit 0xa6
        _emit 0x2e
        _emit 0x01
        // 005d04f9: ff 15 d0 e2 f3 00  CALL dword ptr [0x00f3e2d0]
        _emit 0xff
        _emit 0x15
        _emit 0xd0
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // 005d04ff: 85 c0   TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 005d0501: 7d 19   JGE +0x19
        _emit 0x7d
        _emit 0x19
        // 005d0503: 56   PUSH ESI
        _emit 0x56
        // 005d0504: be 40 3c 36 01   MOV ESI, 0x1363c40
        _emit 0xbe
        _emit 0x40
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        // 005d0509: 56   PUSH ESI
        _emit 0x56
        // 005d050a: e8 7e 12 00 00   CALL FUN_009d178d  (reloc)
        call FUN_009d178d
        // 005d050f: 83 c6 18   ADD ESI, 0x18
        _emit 0x83
        _emit 0xc6
        _emit 0x18
        // 005d0512: 81 fe a0 3c 36 01   CMP ESI, 0x1363ca0
        _emit 0x81
        _emit 0xfe
        _emit 0xa0
        _emit 0x3c
        _emit 0x36
        _emit 0x01
        // 005d0518: 59   POP ECX
        _emit 0x59
        // 005d0519: 7c ee   JL -0x12
        _emit 0x7c
        _emit 0xee
        // 005d051b: 5e   POP ESI
        _emit 0x5e
        // 005d051c: c3   RET
        _emit 0xc3
    }
}
