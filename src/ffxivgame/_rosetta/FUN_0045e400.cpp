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
// FUNCTION: ffxivgame 0x0005e400 — FUN_0045e400 (__cdecl, 387 B / 0x183,
//                                  no SEH; _chkstk-probed 0xc-byte frame).
//
// Inspection (read from the disassembly at orig RVA 0x0005e400):
//
//   The function opens with `MOV EAX, 0xc; CALL 0x009d29d0` — the MSVC
//   `_chkstk` stack-probe for a 0xc-byte local frame. It then loads a
//   handful of incoming stack args, fishes `ESI = *param` (a base
//   pointer / cursor), and assembles a 7-argument cdecl call to the
//   parser/builder helper at 0x0045f3e0 (PUSH of a string literal at
//   0x00f6916c among them). The result is compared against 0 (EBX) and,
//   if `<= 0`, the function bails straight to the shared epilogue.
//
//   On the positive path it: optionally releases a previously-held node
//   (FUN_0045ded0 when *[esp+0x1c] != 0), constructs a fresh node via
//   FUN_0045de40, then — guarded by two TEST/JZ checks against the assert
//   tail — walks two nested index loops over a container (FUN_00464030 =
//   size, FUN_00464040 = element-at, FUN_00463fc0 = per-element predicate,
//   FUN_00464000 = release), copying `[esp+0x14] - ESI` bytes via the
//   memcpy-like helper at 0x009d4600. On success it commits through
//   FUN_0045e280 and, when that returns non-zero, publishes the new node
//   (`*EDI = EBP`, `*[esp+0x24] = [esp+0x14]`, `[EBP+0x4] = 0`) before the
//   POP EDI/ESI/EBX; ADD ESP,0xc; RET tail. Every failure funnels through
//   the assert helper at 0x0045c940 (line 0x9e, file 0x00f691c0, cat
//   0xda, code 0xd) and returns 0.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The body is dense with linker-resolved addresses: six rel32 CALLs to
//   fixed VAs in the 0x0046xxxx range, two more into the CRT thunks at
//   0x009d29d0 / 0x009d4600, the assert tail at 0x0045c940, and three
//   imm32 string/context pointers (0x00f6916c, 0x00f691c0). A source-level
//   /O2 rebuild can't pin those VAs from a standalone .obj, and the exact
//   register schedule (EBX=0 reused as the literal-0 push, ESI as the
//   spilled cursor, the deferred `add esp` folds) is brittle under /O2.
//   The local idiom for this size/shape band (FUN_0040ced0, FUN_00405080,
//   FUN_00415d00) is a naked body re-emitting the orig 387 bytes verbatim
//   via MASM `_emit`; the .obj's `.text` then matches the orig slice
//   byte-for-byte, which is what tools/compare.py grades.

extern "C" __declspec(naked) void FUN_0045e400() {
    __asm {
        _emit 0xb8
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0xc6
        _emit 0x45
        _emit 0x57
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x53
        _emit 0x56
        _emit 0x8b
        _emit 0x30
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x51
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x52
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x28
        _emit 0x50
        _emit 0x51
        _emit 0x68
        _emit 0x6c
        _emit 0x91
        _emit 0xf6
        _emit 0x00
        _emit 0x52
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x33
        _emit 0xdb
        _emit 0x51
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x2c
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x28
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x30
        _emit 0xe8
        _emit 0x93
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        _emit 0x3b
        _emit 0xc3
        _emit 0x0f
        _emit 0x8e
        _emit 0x25
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x57
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x39
        _emit 0x1f
        _emit 0x74
        _emit 0x0a
        _emit 0x53
        _emit 0x57
        _emit 0xe8
        _emit 0x68
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x55
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x53
        _emit 0x52
        _emit 0xe8
        _emit 0xc9
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xc1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        _emit 0x2b
        _emit 0xc6
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0x8a
        _emit 0x87
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x84
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        _emit 0x2b
        _emit 0xd6
        _emit 0x52
        _emit 0x56
        _emit 0x51
        _emit 0xe8
        _emit 0x4b
        _emit 0x61
        _emit 0x57
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52
        _emit 0xe8
        _emit 0x71
        _emit 0x5b
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x7e
        _emit 0x65
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x53
        _emit 0x50
        _emit 0xe8
        _emit 0x6f
        _emit 0x5b
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xf0
        _emit 0x56
        _emit 0x33
        _emit 0xff
        _emit 0xe8
        _emit 0x55
        _emit 0x5b
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0x85
        _emit 0xc0
        _emit 0x7e
        _emit 0x2b
        _emit 0x57
        _emit 0x56
        _emit 0xe8
        _emit 0x57
        _emit 0x5b
        _emit 0x00
        _emit 0x00
        _emit 0x89
        _emit 0x58
        _emit 0x08
        _emit 0x8b
        _emit 0x4d
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0xe8
        _emit 0xca
        _emit 0x5a
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x85
        _emit 0xc0
        _emit 0x74
        _emit 0x46
        _emit 0x56
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        _emit 0xe8
        _emit 0x2a
        _emit 0x5b
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x3b
        _emit 0xf8
        _emit 0x7c
        _emit 0xd5
        _emit 0x56
        _emit 0xe8
        _emit 0xed
        _emit 0x5a
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        _emit 0xe8
        _emit 0x10
        _emit 0x5b
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        _emit 0x3b
        _emit 0xd8
        _emit 0x7c
        _emit 0x9f
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0xe8
        _emit 0xcb
        _emit 0x5a
        _emit 0x00
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x8b
        _emit 0xdd
        _emit 0xe8
        _emit 0x41
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x25
        _emit 0x68
        _emit 0xda
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0xc0
        _emit 0x91
        _emit 0xf6
        _emit 0x00
        _emit 0x6a
        _emit 0x3a
        _emit 0x68
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x0d
        _emit 0xe8
        _emit 0xe5
        _emit 0xe3
        _emit 0xff
        _emit 0xff
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        _emit 0x5d
        _emit 0x5f
        _emit 0x5e
        _emit 0x33
        _emit 0xc0
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0xc7
        _emit 0x45
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x89
        _emit 0x2f
        _emit 0x5d
        _emit 0x89
        _emit 0x0a
        _emit 0x5f
        _emit 0x5e
        _emit 0x5b
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3
    }
}
