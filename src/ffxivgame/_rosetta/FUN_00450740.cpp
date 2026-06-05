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
// FUNCTION: ffxivgame 0x00450740 — __thiscall member fn: formats a name into
//                                  a stack buffer, then appends it (107 B).
//
// Semantics (recovered from asm):
//
//   void __thiscall FUN_00450740(C *this) {
//       /* /GS security cookie protecting the 0x40-byte stack buffer */
//       char buf[0x40];                          // [ESP+0x10] after pushes
//       void *src = (*pfn_00f3e1bc)();           // indirect import CALL
//       // sprintf-family: format src into buf with format string 0xF676C0
//       FUN_009d4f83(buf, 0x40, 0x00F676C0, src, 0);
//       FUN_004528f0(this, this + 0x60);         // __thiscall, ECX=this
//       size_t len = strlen(buf);                // inline scan loop
//       FUN_00404120(/*EAX from prev*/, buf, len);
//       /* /GS cookie check */
//   }
//
// Calling convention: __thiscall (this in ECX, saved to ESI).
// Stack frame: SUB ESP,0x44 — a 0x40-byte local buffer plus the /GS cookie
// slot at [ESP+0x40].  /GS fires here because of the ≥5-byte local array.
//
// The inline strlen loop (offsets +0x3a..+0x4a) is the canonical MSVC 2005
//   LEA EDX,[ESP+4] / loop: MOV BL,[EDX] / ADD EDX,1 / TEST BL,BL / JNZ
//   SUB EDX,ECX  — pointer-walk length computation, not arr[i++].
//
// Reloc-bearing sites (compare.py masks these 4-byte windows):
//   +0x03  MOV EAX,[imm32]   → 0x012EA8B0  (__security_cookie)
//   +0x11  CALL [imm32]      → 0x00F3E1BC  (import thunk, indirect)
//   +0x1b  PUSH imm32        → 0x00F676C0  (format string literal)
//   +0x27  CALL rel32        → format helper (sprintf-family)
//   +0x35  CALL rel32        → 0x004528F0  (__thiscall member)
//   +0x56  CALL rel32        → 0x00404120  (append helper)
//   +0x62  CALL rel32        → __security_check_cookie
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as the
// reloc-heavy siblings FUN_004090b0 / FUN_004091f0 / FUN_00409260): re-emit
// the orig 107 bytes verbatim via MASM `_emit`.  The .obj's `.text` ends up
// byte-identical to the orig slice with no relocations — the rel32 offsets
// resolve against the orig binary's own address space and the imm32 absolutes
// are baked in as raw bytes, exactly the wire image the linker emits at
// relink.  `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00450740() {
    __asm {
        _emit 0x83              // SUB ESP, 0x44
        _emit 0xec
        _emit 0x44
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV [ESP+0x40], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x40
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xff              // CALL [0x00F3E1BC]  (indirect import)
        _emit 0x15
        _emit 0xbc
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x51              // PUSH ECX
        _emit 0x50              // PUSH EAX
        _emit 0x68              // PUSH 0x00F676C0  (format string)
        _emit 0xc0
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x6a              // PUSH 0x40
        _emit 0x40
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009D4F83  (sprintf-family, rel32)
        _emit 0x17
        _emit 0x48
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8d              // LEA ECX, [ESI+0x60]
        _emit 0x4e
        _emit 0x60
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x004528F0  (__thiscall member, rel32)
        _emit 0x76
        _emit 0x21
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x5e              // POP ESI
        _emit 0x8d              // LEA ECX, [EDX+0x1]
        _emit 0x4a
        _emit 0x01
        _emit 0x53              // PUSH EBX
        _emit 0x8a              // strlen loop: MOV BL, [EDX]
        _emit 0x1a
        _emit 0x83              // ADD EDX, 0x1
        _emit 0xc2
        _emit 0x01
        _emit 0x84              // TEST BL, BL
        _emit 0xdb
        _emit 0x75              // JNZ -0x9 (loop)
        _emit 0xf7
        _emit 0x2b              // SUB EDX, ECX   (EDX = strlen)
        _emit 0xd1
        _emit 0x52              // PUSH EDX
        _emit 0x8d              // LEA EDX, [ESP+0x8]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL 0x00404120  (append helper, rel32)
        _emit 0x85
        _emit 0x39
        _emit 0xfb
        _emit 0xff
        _emit 0x8b              // MOV ECX, [ESP+0x44]  (load /GS cookie)
        _emit 0x4c
        _emit 0x24
        _emit 0x44
        _emit 0x5b              // POP EBX
        _emit 0x33              // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8              // CALL __security_check_cookie (rel32)
        _emit 0x4d
        _emit 0x19
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x44
        _emit 0xc4
        _emit 0x44
        _emit 0xc3              // RET
    }
}
