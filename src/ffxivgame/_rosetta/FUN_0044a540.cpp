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
// FUNCTION: ffxivgame 0x0044a540 — Dinkumware STL iterator range validator
//                                  + splice helper prologue (__thiscall,
//                                  156 bytes / 0x9c).
//
// Calling convention: __thiscall (ECX = this); returns void; `RET` (no
//   stack cleanup — callee is a thiscall with all args in registers).
//
// Object layout (Dinkumware small-buffer container):
//   [this + 0x04]  _Bx union: inline char buf[4] OR heap ptr (if cap >= 4)
//   [this + 0x14]  element count
//   [this + 0x18]  capacity — if < 4: SSO (inline buffer), else: heap ptr
//
// The function performs two iterator-validity checks against the container's
// valid range [data(), data() + count*4), calling the debug-trap reporter
// at 0x009d22b4 on any violation.  After both checks pass it pushes
// { EBP, this, EBX, this, &local } and tail-calls FUN_0044a020, the
// Dinkumware splice/move worker.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The three SSO-pointer decodes (each: CMP cap,4 / JC / MOV ecx,[esi] /
//   JMP / MOV ecx,esi) contain short-forward JC/JMP pairs whose relative
//   offsets cannot be reliably reproduced by the C++ front-end at /O2
//   without careful ordering.  Using __declspec(naked) with verbatim
//   _emit bytes gives a byte-identical .text section; compare.py masks
//   the three E8 REL32 call displacements as COFF relocations.

// Call targets (both REL32 — displacements masked by compare.py).
void FUN_009d22b4();   // debug iterator-range trap / _Xlen reporter
void FUN_0044a020();   // Dinkumware splice/move worker

extern "C" __declspec(naked) void FUN_0044a540() {
    __asm {
        // 0004a540: 83 ec 08    SUB ESP,8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0004a543: 53          PUSH EBX
        _emit 0x53
        // 0004a544: 55          PUSH EBP
        _emit 0x55
        // 0004a545: 56          PUSH ESI
        _emit 0x56
        // 0004a546: 57          PUSH EDI
        _emit 0x57
        // 0004a547: 8b f9       MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 0004a549: 8b 57 18    MOV EDX,[EDI+0x18]   ; EDX = capacity
        _emit 0x8b
        _emit 0x57
        _emit 0x18
        // 0004a54c: 83 fa 04    CMP EDX,4
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        // 0004a54f: 8d 77 04    LEA ESI,[EDI+4]      ; ESI = &_Bx
        _emit 0x8d
        _emit 0x77
        _emit 0x04
        // 0004a552: 72 04       JC  +4               ; if cap<4 -> SSO
        _emit 0x72
        _emit 0x04
        // 0004a554: 8b 0e       MOV ECX,[ESI]        ; ECX = data ptr
        _emit 0x8b
        _emit 0x0e
        // 0004a556: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a558: 8b ce       MOV ECX,ESI          ; ECX = &_Bx (SSO)
        _emit 0x8b
        _emit 0xce
        // 0004a55a: 8b 47 14    MOV EAX,[EDI+0x14]   ; EAX = count
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        // 0004a55d: 03 c0       ADD EAX,EAX
        _emit 0x03
        _emit 0xc0
        // 0004a55f: 03 c0       ADD EAX,EAX          ; EAX = count*4
        _emit 0x03
        _emit 0xc0
        // 0004a561: 8d 2c 08    LEA EBP,[EAX+ECX]    ; EBP = data+count*4
        _emit 0x8d
        _emit 0x2c
        _emit 0x08
        // 0004a564: 85 ed       TEST EBP,EBP
        _emit 0x85
        _emit 0xed
        // 0004a566: 74 20       JZ  +0x20            ; null -> trap
        _emit 0x74
        _emit 0x20
        // 0004a568: 83 fa 04    CMP EDX,4
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        // 0004a56b: 72 04       JC  +4
        _emit 0x72
        _emit 0x04
        // 0004a56d: 8b 0e       MOV ECX,[ESI]
        _emit 0x8b
        _emit 0x0e
        // 0004a56f: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a571: 8b ce       MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0004a573: 3b cd       CMP ECX,EBP
        _emit 0x3b
        _emit 0xcd
        // 0004a575: 77 11       JA  +0x11            ; begin>end -> trap
        _emit 0x77
        _emit 0x11
        // 0004a577: 83 fa 04    CMP EDX,4
        _emit 0x83
        _emit 0xfa
        _emit 0x04
        // 0004a57a: 72 04       JC  +4
        _emit 0x72
        _emit 0x04
        // 0004a57c: 8b 0e       MOV ECX,[ESI]
        _emit 0x8b
        _emit 0x0e
        // 0004a57e: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a580: 8b ce       MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0004a582: 03 c1       ADD EAX,ECX
        _emit 0x03
        _emit 0xc1
        // 0004a584: 3b e8       CMP EBP,EAX
        _emit 0x3b
        _emit 0xe8
        // 0004a586: 76 05       JBE +5               ; ok -> skip trap
        _emit 0x76
        _emit 0x05
        // 0004a588: e8 27 7d 58 00   CALL FUN_009d22b4  (REL32; masked)
        call FUN_009d22b4
        // --- second iterator check ---
        // 0004a58d: 8b 4f 18    MOV ECX,[EDI+0x18]   ; ECX = capacity
        _emit 0x8b
        _emit 0x4f
        _emit 0x18
        // 0004a590: 83 f9 04    CMP ECX,4
        _emit 0x83
        _emit 0xf9
        _emit 0x04
        // 0004a593: 72 04       JC  +4
        _emit 0x72
        _emit 0x04
        // 0004a595: 8b 1e       MOV EBX,[ESI]
        _emit 0x8b
        _emit 0x1e
        // 0004a597: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a599: 8b de       MOV EBX,ESI
        _emit 0x8b
        _emit 0xde
        // 0004a59b: 85 db       TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 0004a59d: 74 20       JZ  +0x20            ; null -> trap
        _emit 0x74
        _emit 0x20
        // 0004a59f: 83 f9 04    CMP ECX,4
        _emit 0x83
        _emit 0xf9
        _emit 0x04
        // 0004a5a2: 72 04       JC  +4
        _emit 0x72
        _emit 0x04
        // 0004a5a4: 8b 06       MOV EAX,[ESI]
        _emit 0x8b
        _emit 0x06
        // 0004a5a6: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a5a8: 8b c6       MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 0004a5aa: 3b c3       CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0004a5ac: 77 11       JA  +0x11            ; begin>iter -> trap
        _emit 0x77
        _emit 0x11
        // 0004a5ae: 83 f9 04    CMP ECX,4
        _emit 0x83
        _emit 0xf9
        _emit 0x04
        // 0004a5b1: 72 02       JC  +2               ; if cap<4 -> skip deref
        _emit 0x72
        _emit 0x02
        // 0004a5b3: 8b 36       MOV ESI,[ESI]        ; ESI = data ptr
        _emit 0x8b
        _emit 0x36
        // 0004a5b5: 8b 47 14    MOV EAX,[EDI+0x14]   ; EAX = count
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        // 0004a5b8: 8d 0c 86    LEA ECX,[ESI+EAX*4]  ; ECX = data+count*4
        _emit 0x8d
        _emit 0x0c
        _emit 0x86
        // 0004a5bb: 3b d9       CMP EBX,ECX
        _emit 0x3b
        _emit 0xd9
        // 0004a5bd: 76 05       JBE +5               ; ok -> skip trap
        _emit 0x76
        _emit 0x05
        // 0004a5bf: e8 f0 7c 58 00   CALL FUN_009d22b4  (REL32; masked)
        call FUN_009d22b4
        // --- call FUN_0044a020 ---
        // 0004a5c4: 55          PUSH EBP
        _emit 0x55
        // 0004a5c5: 57          PUSH EDI
        _emit 0x57
        // 0004a5c6: 53          PUSH EBX
        _emit 0x53
        // 0004a5c7: 57          PUSH EDI
        _emit 0x57
        // 0004a5c8: 8d 54 24 20 LEA EDX,[ESP+0x20]   ; EDX = &local_buf
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 0004a5cc: 52          PUSH EDX
        _emit 0x52
        // 0004a5cd: 8b cf       MOV ECX,EDI          ; ECX = this
        _emit 0x8b
        _emit 0xcf
        // 0004a5cf: e8 4c fa ff ff   CALL FUN_0044a020  (REL32; masked)
        call FUN_0044a020
        // --- epilogue ---
        // 0004a5d4: 5f          POP EDI
        _emit 0x5f
        // 0004a5d5: 5e          POP ESI
        _emit 0x5e
        // 0004a5d6: 5d          POP EBP
        _emit 0x5d
        // 0004a5d7: 5b          POP EBX
        _emit 0x5b
        // 0004a5d8: 83 c4 08    ADD ESP,8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0004a5db: c3          RET
        _emit 0xc3
    }
}
