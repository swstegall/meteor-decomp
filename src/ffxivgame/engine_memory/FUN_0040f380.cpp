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
// FUNCTION: ffxivgame 0x0000f380 — __stdcall 3-arg: one-time-init assert-callback stub
//                                  (SQEX::CDev::Engine::Memory::ComplexLink::Realloc)
//
// Asm (65 bytes):
//   b8 01 00 00 00              MOV EAX, 0x1
//   84 05 10 39 32 01           TEST byte ptr [0x01323910], AL
//   75 10                       JNZ +0x10  (skip init, to PUSH block)
//   09 05 10 39 32 01           OR dword ptr [0x01323910], EAX
//   c7 05 0c 39 32 01 d0 f2 40 00  MOV dword ptr [0x0132390c], 0x40f2d0
//   68 78 66 f5 00              PUSH 0xf56678          ; "false"
//   6a 79                       PUSH 0x79              ; 121 (line)
//   68 5c 66 f5 00              PUSH 0xf5665c          ; ".\\modules\\ComplexLink.cpp"
//   68 48 4d f5 00              PUSH 0xf54d48          ; condition string
//   68 10 65 f5 00              PUSH 0xf56510          ; function name
//   ff 15 0c 39 32 01           CALL dword ptr [0x0132390c]
//   83 c4 14                    ADD ESP, 0x14
//   33 c0                       XOR EAX, EAX
//   c2 0c 00                    RET 0xc
//
// Pattern: one-time initialization of a global assert-handler function pointer
// (stored at 0x0132390c), guarded by a flag at 0x01323910. After init the handler
// is called with 5 args. Returns 0. __stdcall with 3 stack args (RET 0xC).

extern "C" __declspec(naked) void __stdcall FUN_0040f380(int, int, int)
{
    __asm {
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x10
        _emit 0x10
        _emit 0x09              // OR dword ptr [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x0040f2d0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xd0
        _emit 0xf2
        _emit 0x40
        _emit 0x00
        _emit 0x68              // PUSH 0x00f56678
        _emit 0x78
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x79
        _emit 0x79
        _emit 0x68              // PUSH 0x00f5665c
        _emit 0x5c
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f56510
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
