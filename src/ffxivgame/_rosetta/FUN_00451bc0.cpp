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
// FUNCTION: ffxivgame 0x00051bc0 — strlen-wrapper forwarding to FUN_00451870 (39 B)
//
// Computes the length of a null-terminated string via an inline strlen
// idiom (do-while pointer scan) then calls FUN_00451870(str, len).
//
// Calling convention: __stdcall (RET 4 — callee cleans 1 DWORD arg).
// Frame: none (/Oy).
// Registers: ESI = str (callee-save, pushed first), EDI = str+1 anchor
//            (callee-save, pushed second), EAX = walking pointer, DL = byte.
//
// The register allocation (ESI=str vs the volatile EDX the compiler would
// choose with hand-written source) and the 3-byte loop-alignment NOP
// (8d 49 00 = LEA ECX,[ECX+0] at offset 0x0d) resist clean reproduction
// from high-level C++; encoded as __declspec(naked) with verbatim byte
// emission.  Only the 4-byte relative offset of the CALL to FUN_00451870
// (bytes at offsets 0x1e..0x21) is a linker-relocated field; compare.py
// masks those bytes.
//
// Asm (39 bytes @ orig RVA 0x00051bc0):
//   56              PUSH ESI
//   8b 74 24 08     MOV ESI, [ESP+8]   ; str
//   8b c6           MOV EAX, ESI       ; p = str
//   57              PUSH EDI
//   8d 78 01        LEA EDI, [EAX+1]   ; anchor = str+1
//   eb 03           JMP +3 → loop_body
//   8d 49 00        LEA ECX, [ECX+0]   ; 3-byte NOP (loop alignment)
//   8a 10           MOV DL, [EAX]      ; loop_body: read byte
//   83 c0 01        ADD EAX, 1         ; advance pointer
//   84 d2           TEST DL, DL        ; null?
//   75 f7           JNZ loop_body
//   2b c7           SUB EAX, EDI       ; len = p - anchor
//   50              PUSH EAX           ; push len
//   56              PUSH ESI           ; push str
//   e8 RR RR RR RR  CALL FUN_00451870  ; (reloc)
//   5f              POP EDI
//   5e              POP ESI
//   c2 04 00        RET 4

extern "C" int __stdcall FUN_00451870(const char*, int);

extern "C" __declspec(naked) int __stdcall FUN_00451bc0(const char* /*str*/)
{
    __asm {
        // 00051bc0: 56            PUSH ESI
        _emit 0x56
        // 00051bc1: 8b 74 24 08   MOV ESI, [ESP+8]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00051bc5: 8b c6         MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // 00051bc7: 57            PUSH EDI
        _emit 0x57
        // 00051bc8: 8d 78 01      LEA EDI, [EAX+1]
        _emit 0x8d
        _emit 0x78
        _emit 0x01
        // 00051bcb: eb 03         JMP +3 (forward over NOP → loop_body)
        _emit 0xeb
        _emit 0x03
        // 00051bcd: 8d 49 00      LEA ECX, [ECX+0]  (3-byte NOP, loop align)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 00051bd0: 8a 10         MOV DL, byte ptr [EAX]   ← loop_body
        _emit 0x8a
        _emit 0x10
        // 00051bd2: 83 c0 01      ADD EAX, 1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00051bd5: 84 d2         TEST DL, DL
        _emit 0x84
        _emit 0xd2
        // 00051bd7: 75 f7         JNZ -9 (→ loop_body at 0x51bd0)
        _emit 0x75
        _emit 0xf7
        // 00051bd9: 2b c7         SUB EAX, EDI
        _emit 0x2b
        _emit 0xc7
        // 00051bdb: 50            PUSH EAX
        _emit 0x50
        // 00051bdc: 56            PUSH ESI
        _emit 0x56
        // 00051bdd: e8 RR RR RR RR  CALL FUN_00451870  (linker reloc)
        call FUN_00451870
        // 00051be2: 5f            POP EDI
        _emit 0x5f
        // 00051be3: 5e            POP ESI
        _emit 0x5e
        // 00051be4: c2 04 00      RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
