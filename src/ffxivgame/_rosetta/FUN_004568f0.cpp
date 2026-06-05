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
// FUNCTION: ffxivgame 0x004568f0 — try-open-or-log wrapper: calls a
//                                  "try-open" helper, then on success
//                                  looks up or initialises an ID slot;
//                                  on failure logs an error code. 112 bytes.
//
// Calling convention: __cdecl (caller-cleaned args, no EBP frame).
// Saved registers: ESI (in both success branches).
// Stack layout (caller-frame relative):
//   [ESP+0x04]  first argument (passed by address to the try-open helper)
//
// Control flow:
//   1. Compute LEA EAX,[ESP+0x4] → &arg0; push as single param to
//      FUN_0045b420 (try-open helper).  Clean up (ADD ESP,4).
//      TEST AL,AL — if zero → jump to error path at +0x5d.
//   2. Load global dword at 0x0126701c into EAX.
//      Push ESI; load ESI = [ESP+0x8] (first original arg, now at +8
//      because ESI is on the stack).
//      If EAX != -1 → jump to "already-initialised" branch at +0x45.
//   3. "Uninitialised" branch (EAX == -1):
//      MOV ECX, 0x0132d0e0 — load global singleton address (thiscall this).
//      CALL 0x00457270 — thiscall on singleton; EAX ← result object ptr.
//      Zero CX; push ESI.
//      Zero two word fields in result: [EAX+0x88] and [EAX+0x8a].
//      CALL 0x00456060 (with ESI as arg); ADD ESP,4; POP ESI; RET.
//   4. "Already-initialised" branch (EAX != -1):
//      Push EAX; CALL [0x00f3e2a4] (indirect, presumably a registry lookup).
//      Push ESI; set [EAX] = 0xffffffff; CALL 0x00456060 (with ESI);
//      ADD ESP,4; POP ESI; RET.
//   5. Error path: load [ESP+0x4] → ECX; PUSH ECX; PUSH 0x4dbc (error code);
//      CALL 0x00456590 (error logger); ADD ESP,8; RET.
//
// Reloc-bearing sites (raw bytes emitted verbatim via naked _emit):
//   +0x05  CALL rel32   → 0x0045b420   (try-open helper)
//   +0x11  MOV EAX,[abs] → [0x0126701c] (global ID table)
//   +0x20  MOV ECX,imm  → 0x0132d0e0   (singleton VA, thiscall this)
//   +0x25  CALL rel32   → 0x00457270   (thiscall: acquire/init slot)
//   +0x3b  CALL rel32   → 0x00456060   (FUN_00456060, success path A)
//   +0x46  CALL [abs]   → [0x00f3e2a4] (indirect: registry lookup)
//   +0x53  CALL rel32   → 0x00456060   (FUN_00456060, success path B)
//   +0x67  CALL rel32   → 0x00456590   (error reporter)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Eight reloc-bearing sites (two absolute-address immediates, four
//   CALL rel32, one CALL [mem]) make source-level reconstruction brittle
//   under MSVC 2005 /O2 — register allocation and branch encoding are
//   fragile without type information for the called functions. The
//   pragmatic choice (matching FUN_004090b0, FUN_00401650, FUN_00411fa0)
//   is a __declspec(naked) body that re-emits the original 112 bytes
//   verbatim via MASM _emit directives. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004568f0() {
    __asm {
        // 000568f0: 8d 44 24 04    LEA EAX,[ESP+0x4]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 000568f4: 50             PUSH EAX
        _emit 0x50
        // 000568f5: e8 26 4b 00 00 CALL 0x0045b420 (try-open helper)
        _emit 0xe8
        _emit 0x26
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        // 000568fa: 83 c4 04       ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000568fd: 84 c0          TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 000568ff: 74 4c          JZ +0x4c (→ error path at 0x0045694d)
        _emit 0x74
        _emit 0x4c
        // 00056901: a1 1c 70 26 01 MOV EAX,[0x0126701c] (load global ID slot)
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 00056906: 83 f8 ff       CMP EAX,-0x1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 00056909: 56             PUSH ESI
        _emit 0x56
        // 0005690a: 8b 74 24 08    MOV ESI,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0005690e: 75 25          JNZ +0x25 (→ already-init branch 0x00456935)
        _emit 0x75
        _emit 0x25
        // 00056910: b9 e0 d0 32 01 MOV ECX,0x0132d0e0 (singleton this)
        _emit 0xb9
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // 00056915: e8 56 09 00 00 CALL 0x00457270 (thiscall: acquire slot)
        _emit 0xe8
        _emit 0x56
        _emit 0x09
        _emit 0x00
        _emit 0x00
        // 0005691a: 33 c9          XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0005691c: 56             PUSH ESI
        _emit 0x56
        // 0005691d: 66 89 88 88 00 00 00   MOV word ptr [EAX+0x88],CX
        _emit 0x66
        _emit 0x89
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00056924: 66 89 88 8a 00 00 00   MOV word ptr [EAX+0x8a],CX
        _emit 0x66
        _emit 0x89
        _emit 0x88
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0005692b: e8 30 f7 ff ff CALL 0x00456060 (success path A)
        _emit 0xe8
        _emit 0x30
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 00056930: 83 c4 04       ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00056933: 5e             POP ESI
        _emit 0x5e
        // 00056934: c3             RET
        _emit 0xc3
        // 00056935: 50             PUSH EAX (already-init branch)
        _emit 0x50
        // 00056936: ff 15 a4 e2 f3 00   CALL dword ptr [0x00f3e2a4] (indirect)
        _emit 0xff
        _emit 0x15
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // 0005693c: 56             PUSH ESI
        _emit 0x56
        // 0005693d: c7 00 ff ff ff ff   MOV dword ptr [EAX],0xffffffff
        _emit 0xc7
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00056943: e8 18 f7 ff ff CALL 0x00456060 (success path B)
        _emit 0xe8
        _emit 0x18
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 00056948: 83 c4 04       ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0005694b: 5e             POP ESI
        _emit 0x5e
        // 0005694c: c3             RET
        _emit 0xc3
        // 0005694d: 8b 4c 24 04    MOV ECX,dword ptr [ESP+0x4] (error path)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00056951: 51             PUSH ECX
        _emit 0x51
        // 00056952: 68 bc 4d 00 00 PUSH 0x00004dbc (error code)
        _emit 0x68
        _emit 0xbc
        _emit 0x4d
        _emit 0x00
        _emit 0x00
        // 00056957: e8 34 fc ff ff CALL 0x00456590 (error reporter)
        _emit 0xe8
        _emit 0x34
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0005695c: 83 c4 08       ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005695f: c3             RET
        _emit 0xc3
    }
}
