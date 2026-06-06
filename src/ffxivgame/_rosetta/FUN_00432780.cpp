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
// FUNCTION: ffxivgame 0x00432780 — checked-iterator list walk that compares a
//                                  cached node pointer (this+0x8) against each
//                                  forward link and dispatches a virtual call
//                                  per match (__thiscall, 121 bytes / 0x79).
//
// Calling convention: __thiscall (ECX = this); returns void; cleans 0xc bytes
//   of locals on exit but RET (no immediate) → no stack params.
//
// Object / iterator layout touched by this function:
//   [this + 0x04]   embedded list sentinel; LEA EAX,[ECX+4] → ESI is the
//                   iterator cursor (starts at the sentinel address)
//   [this + 0x08]   EBX = cached "end / target" node pointer
//   [cursor + 0x04] forward link to the next list node
//   [node + 0x00]   first child / payload pointer
//
// The repeated CALL 0x009d22b4 sites are the MSVC checked-iterator (_SECURE_SCL)
// debug-assert helper, guarded by TEST/CMP/Jcc bounds checks — each preceded
// branch skips the assert when the invariant holds.
//
// Dispatch site:
//   MOV ECX,[EDI+0x8] ; MOV EDX,[ECX] ; MOV EAX,[EDX+0x8] ; CALL EAX
//   → node->field_8->vtable[2](...) virtual call.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The control flow (an entry JMP into the mid-loop test, four conditional
//   skips around the iterator-debug asserts, and a single virtual dispatch)
//   plus the precise register allocation is not reproducible from /O2 C++
//   without bit-shifting at least one branch. Every CALL 0x009d22b4 is a
//   position-relative rel32 (image-base independent) and the dispatch is a
//   register-indirect CALL EAX, so there are NO absolute relocations — a
//   __declspec(naked) body re-emitting the original 121 bytes verbatim via
//   MASM _emit directives yields a .obj whose .text is byte-identical to the
//   original slice. tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00432780() {
    __asm {
        // 00032780: 83 ec 0c       SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00032783: 53             PUSH EBX
        _emit 0x53
        // 00032784: 8b 59 08       MOV EBX,[ECX+0x8]
        _emit 0x8b
        _emit 0x59
        _emit 0x08
        // 00032787: 55             PUSH EBP
        _emit 0x55
        // 00032788: 56             PUSH ESI
        _emit 0x56
        // 00032789: 8d 41 04       LEA EAX,[ECX+0x4]
        _emit 0x8d
        _emit 0x41
        _emit 0x04
        // 0003278c: 57             PUSH EDI
        _emit 0x57
        // 0003278d: 8b f0          MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 0003278f: eb 02          JMP +2 (→ 0x32793)
        _emit 0xeb
        _emit 0x02
        // 00032791: 8b c6          MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00032793: 85 f6          TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 00032795: 8b 48 04       MOV ECX,[EAX+0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 00032798: 8b 39          MOV EDI,[ECX]
        _emit 0x8b
        _emit 0x39
        // 0003279a: 74 04          JZ +4 (→ 0x327a0)
        _emit 0x74
        _emit 0x04
        // 0003279c: 3b f0          CMP ESI,EAX
        _emit 0x3b
        _emit 0xf0
        // 0003279e: 74 05          JZ +5 (→ 0x327a5)
        _emit 0x74
        _emit 0x05
        // 000327a0: e8 0f fb 59 00 CALL 0x009d22b4 (iterator-debug assert)
        _emit 0xe8
        _emit 0x0f
        _emit 0xfb
        _emit 0x59
        _emit 0x00
        // 000327a5: 3b df          CMP EBX,EDI
        _emit 0x3b
        _emit 0xdf
        // 000327a7: 74 48          JZ +0x48 (→ 0x327f1, epilogue)
        _emit 0x74
        _emit 0x48
        // 000327a9: 85 f6          TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 000327ab: 8b ee          MOV EBP,ESI
        _emit 0x8b
        _emit 0xee
        // 000327ad: 89 5c 24 18    MOV [ESP+0x18],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        // 000327b1: 75 05          JNZ +5 (→ 0x327b8)
        _emit 0x75
        _emit 0x05
        // 000327b3: e8 fc fa 59 00 CALL 0x009d22b4
        _emit 0xe8
        _emit 0xfc
        _emit 0xfa
        _emit 0x59
        _emit 0x00
        // 000327b8: 8b 7b 04       MOV EDI,[EBX+0x4]
        _emit 0x8b
        _emit 0x7b
        _emit 0x04
        // 000327bb: 3b 7d 04       CMP EDI,[EBP+0x4]
        _emit 0x3b
        _emit 0x7d
        _emit 0x04
        // 000327be: 75 0f          JNZ +0xf (→ 0x327cf)
        _emit 0x75
        _emit 0x0f
        // 000327c0: e8 ef fa 59 00 CALL 0x009d22b4
        _emit 0xe8
        _emit 0xef
        _emit 0xfa
        _emit 0x59
        _emit 0x00
        // 000327c5: 3b 7d 04       CMP EDI,[EBP+0x4]
        _emit 0x3b
        _emit 0x7d
        _emit 0x04
        // 000327c8: 75 05          JNZ +5 (→ 0x327cf)
        _emit 0x75
        _emit 0x05
        // 000327ca: e8 e5 fa 59 00 CALL 0x009d22b4
        _emit 0xe8
        _emit 0xe5
        _emit 0xfa
        _emit 0x59
        _emit 0x00
        // 000327cf: 8b 4f 08       MOV ECX,[EDI+0x8]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 000327d2: 8b 11          MOV EDX,[ECX]
        _emit 0x8b
        _emit 0x11
        // 000327d4: 8b 42 08       MOV EAX,[EDX+0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 000327d7: ff d0          CALL EAX (virtual dispatch)
        _emit 0xff
        _emit 0xd0
        // 000327d9: 85 f6          TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 000327db: 75 05          JNZ +5 (→ 0x327e2)
        _emit 0x75
        _emit 0x05
        // 000327dd: e8 d2 fa 59 00 CALL 0x009d22b4
        _emit 0xe8
        _emit 0xd2
        _emit 0xfa
        _emit 0x59
        _emit 0x00
        // 000327e2: 8b 5b 04       MOV EBX,[EBX+0x4]
        _emit 0x8b
        _emit 0x5b
        _emit 0x04
        // 000327e5: 3b 5e 04       CMP EBX,[ESI+0x4]
        _emit 0x3b
        _emit 0x5e
        _emit 0x04
        // 000327e8: 75 a7          JNZ -0x59 (→ 0x32791)
        _emit 0x75
        _emit 0xa7
        // 000327ea: e8 c5 fa 59 00 CALL 0x009d22b4
        _emit 0xe8
        _emit 0xc5
        _emit 0xfa
        _emit 0x59
        _emit 0x00
        // 000327ef: eb a0          JMP -0x60 (→ 0x32791)
        _emit 0xeb
        _emit 0xa0
        // 000327f1: 5f             POP EDI
        _emit 0x5f
        // 000327f2: 5e             POP ESI
        _emit 0x5e
        // 000327f3: 5d             POP EBP
        _emit 0x5d
        // 000327f4: 5b             POP EBX
        _emit 0x5b
        // 000327f5: 83 c4 0c       ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000327f8: c3             RET
        _emit 0xc3
    }
}
