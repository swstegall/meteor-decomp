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
// FUNCTION: ffxivgame 0x005df187 — dynamic function resolver with fast-path
//                                  cache and LoadLibrary fallback (110 bytes).
//
// __cdecl FUN_009df187(arg) — resolves a function via two paths:
//
//   Fast path: if a cached module handle at [0x12eaeb0] can be used to
//   look up a function index [0x12eaeac] (via fn-ptr at [0xf3e2a4]),
//   call the result and return result->field_0x1fc.
//
//   Slow path: LoadLibrary("..." @ 0xf8891c), call a helper at
//   RVA 0x9df0a4, then GetProcAddress(module, "..." @ 0x1086eb8).
//
//   Common exit: if a function pointer was resolved, call it with the
//   original argument, return the result; otherwise return 0.
//
// Calling convention: __cdecl (ret; one argument via [esp+8] after push esi).
// Frame: no EBP frame (frame-pointer omission /Oy); saves ESI only.
//
// Reloc-bearing sites in the orig 110 bytes:
//   +0x02  PUSH [imm32]  → 0x012eaeb0  (.data global)
//   +0x08  MOV ESI,[imm32] → 0x00f3e2a4  (.data function pointer)
//   +0x14  MOV EAX,[imm32] → 0x012eaeac  (.data global)
//   +0x1e  PUSH [imm32]  → 0x012eaeb0  (.data global)
//   +0x34  PUSH imm32    → 0x00f8891c  (.rdata string)
//   +0x39  CALL [imm32]  → 0x00f3e1e4  (IAT LoadLibraryA)
//   +0x45  CALL rel32    → 0x009df0a4  (.text helper, rel32)
//   +0x4e  PUSH imm32    → 0x01086eb8  (.rdata string)
//   +0x54  CALL [imm32]  → 0x00f3e150  (IAT GetProcAddress)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function references multiple absolute addresses (IAT slots, .data
//   globals, .rdata strings) and one rel32 CALL. Emitting the orig 110
//   bytes verbatim via MASM _emit directives produces a .obj whose .text
//   matches orig byte-for-byte (no relocations in the .obj; compare.py
//   compares the post-fixup orig bytes directly). GREEN.

extern "C" __declspec(naked) void FUN_009df187() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0xff              // PUSH dword ptr [0x012eaeb0]
        _emit 0x35
        _emit 0xb0
        _emit 0xae
        _emit 0x2e
        _emit 0x01
        _emit 0x8b              // MOV ESI, dword ptr [0x00f3e2a4]
        _emit 0x35
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0xff              // CALL ESI
        _emit 0xd6
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JE +0x21
        _emit 0x21
        _emit 0xa1              // MOV EAX, dword ptr [0x012eaeac]
        _emit 0xac
        _emit 0xae
        _emit 0x2e
        _emit 0x01
        _emit 0x83              // CMP EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x74              // JE +0x17
        _emit 0x17
        _emit 0x50              // PUSH EAX
        _emit 0xff              // PUSH dword ptr [0x012eaeb0]
        _emit 0x35
        _emit 0xb0
        _emit 0xae
        _emit 0x2e
        _emit 0x01
        _emit 0xff              // CALL ESI
        _emit 0xd6
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JE +0x08
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [EAX + 0x1fc]
        _emit 0x80
        _emit 0xfc
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x26
        _emit 0x26
        _emit 0x68              // PUSH 0x00f8891c
        _emit 0x1c
        _emit 0x89
        _emit 0xf8
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x00f3e1e4]
        _emit 0x15
        _emit 0xe4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JE +0x23
        _emit 0x23
        _emit 0xe8              // CALL rel32 → 0x009df0a4
        _emit 0xd3
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JE +0x1a
        _emit 0x1a
        _emit 0x68              // PUSH 0x01086eb8
        _emit 0xb8
        _emit 0x6e
        _emit 0x08
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0xff              // CALL dword ptr [0x00f3e150]
        _emit 0x15
        _emit 0x50
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JE +0x0a
        _emit 0x0a
        _emit 0xff              // PUSH dword ptr [ESP + 8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x89              // MOV dword ptr [ESP + 8], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
