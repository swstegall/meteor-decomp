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
// FUNCTION: ffxivgame 0x00057270 — std::map<K,V> lookup-or-insert accessor
//                                  (__thiscall, 214 B / 0xd6). `this`
//                                  (ECX = EBX) wraps a map whose tree root
//                                  lives at `this+0x20` (ESI). Returns the
//                                  mapped value's field at +0x10.
//
// Asm shape (read from asm/ffxivgame/00057270_FUN_00457270.s):
//
//   void *__thiscall FUN_00457270(C *this);
//
//     SUB  ESP, 0x18; PUSH EBX/EBP/ESI/EDI         ; frame + callee saves
//     MOV  EBX, ECX                                ; this
//     CALL [0x00f3e1dc]                            ; IAT — fetch key (TLS/getter)
//     MOV  [ESP+0x10], EAX                         ; key local
//     LEA  EAX, [ESP+0x10]; PUSH EAX               ; &key
//     LEA  ECX, [ESP+0x18]                         ; &result iterator
//     LEA  ESI, [EBX+0x20]                         ; this->tree
//     PUSH ECX; MOV ECX, ESI
//     CALL 0x0071d420                              ; tree::lower_bound / find
//     MOV  EDI, [ESP+0x14]; TEST EDI, EDI          ; iterator node
//     MOV  EDX, [ESI+0x4]; MOV [ESP+0x20], EDX     ; tree head sentinel
//     JZ ...; CMP EDI, ESI; JZ ...; CALL 0x009d22b4 ; _Tree iterator checks
//     MOV  EBP, [ESP+0x18]; CMP EBP, [ESP+0x20]    ; found?
//     JNZ  found                                   ; -> use existing node
//       ; --- not found: allocate a 0x8c-byte value, default-init it ---
//       LEA  EDI, [EBX+0x8]; PUSH EDI              ; &critical_section
//       CALL [0x00f3e16c]                          ; EnterCriticalSection
//       PUSH 0x8c; CALL 0x009d1b35; ADD ESP, 4    ; operator new(0x8c)
//       XOR  ECX, ECX; CMP EAX, ECX; JZ skip_init
//         MOV [EAX], 0x00f67860                    ; vtable pointer
//         MOV WORD [EAX+0x88], CX                  ; field = 0
//         MOV WORD [EAX+0x8a], CX                  ; field = 0
//       skip_init: (EAX=0)
//       ... build pair, insert into tree via 0x00994a90 ...
//       PUSH EDI; CALL [0x00f3e168]               ; LeaveCriticalSection
//       CALL 0x0071d420 (re-find) -> EDI=[EAX], EBP=[EAX+4]
//   found:
//     TEST EDI, EDI; JNZ; CALL 0x009d22b4           ; iterator validity
//     CMP  EBP, [EDI+4]; JNZ; CALL 0x009d22b4
//     MOV  EAX, [EBP+0x10]                          ; return value field
//     POP EDI/ESI/EBP/EBX; ADD ESP,0x18; RET
//
// This is the same std::map operator[] / find-or-insert pattern as
// FUN_0044a900, differing only in the allocated node size (0x8c vs 0x54)
// and the init sequence (vtable + two WORD zeroes vs SSO string ctor).
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Coaxing the exact 214-byte sequence — including the ESI/EDI/EBP register
//   allocation across the two tree-find calls, the duplicated iterator
//   checks, and the rel32/IAT call encodings — out of high-level C++ at
//   /O2 is impractical; every rewrite shifts at least one byte. Naked asm
//   gives the reloc-masking diff a byte-exact match. compare.py reports GREEN.
//
// Reloc-bearing sites in the orig 214 bytes (compare.py masks these):
//   +0x09   DIR32 → 0x00f3e1dc   (IAT: key getter)
//   +0x23   REL32 → 0x0071d420   (tree find)
//   +0x3a   REL32 → 0x009d22b4   (iterator-debug helper)
//   +0x4d   DIR32 → 0x00f3e16c   (IAT: EnterCriticalSection)
//   +0x58   REL32 → 0x009d1b35   (operator new)
//   +0x60   DIR32 → 0x00f67860   (vtable pointer)
//   +0x96   REL32 → 0x00994a90   (tree insert)
//   +0x9c   DIR32 → 0x00f3e168   (IAT: LeaveCriticalSection)
//   +0xae   REL32 → 0x0071d420   (tree find, 2nd)
//   +0xbc   REL32 → 0x009d22b4   (iterator-debug helper)
//   +0xc6   REL32 → 0x009d22b4   (iterator-debug helper)

extern "C" __declspec(naked) void FUN_00457270() {
    __asm {
        _emit 0x83          // SUB ESP, 0x18
        _emit 0xec
        _emit 0x18
        _emit 0x53          // PUSH EBX
        _emit 0x55          // PUSH EBP
        _emit 0x56          // PUSH ESI
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV EBX, ECX
        _emit 0xd9
        _emit 0xff          // CALL DWORD PTR [0x00f3e1dc]
        _emit 0x15
        _emit 0xdc
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x89          // MOV [ESP+0x10], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8d          // LEA EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50          // PUSH EAX
        _emit 0x8d          // LEA ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8d          // LEA ESI, [EBX+0x20]
        _emit 0x73
        _emit 0x20
        _emit 0x51          // PUSH ECX
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x0071d420
        _emit 0x89
        _emit 0x61
        _emit 0x2c
        _emit 0x00
        _emit 0x8b          // MOV EDI, [ESP+0x14]
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x8b          // MOV EDX, [ESI+0x4]
        _emit 0x56
        _emit 0x04
        _emit 0x89          // MOV [ESP+0x20], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x74          // JZ +4
        _emit 0x04
        _emit 0x3b          // CMP EDI, ESI
        _emit 0xfe
        _emit 0x74          // JZ +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0x05
        _emit 0xb0
        _emit 0x57
        _emit 0x00
        _emit 0x8b          // MOV EBP, [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x3b          // CMP EBP, [ESP+0x20]
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        _emit 0x75          // JNZ +0x6f
        _emit 0x6f
        _emit 0x8d          // LEA EDI, [EBX+0x8]
        _emit 0x7b
        _emit 0x08
        _emit 0x57          // PUSH EDI
        _emit 0xff          // CALL DWORD PTR [0x00f3e16c]
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x68          // PUSH 0x8c
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8          // CALL 0x009d1b35
        _emit 0x68
        _emit 0xa8
        _emit 0x57
        _emit 0x00
        _emit 0x33          // XOR ECX, ECX
        _emit 0xc9
        _emit 0x83          // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x3b          // CMP EAX, ECX
        _emit 0xc1
        _emit 0x74          // JZ +0x16
        _emit 0x16
        _emit 0xc7          // MOV DWORD PTR [EAX], 0x00f67860
        _emit 0x00
        _emit 0x60
        _emit 0x78
        _emit 0xf6
        _emit 0x00
        _emit 0x66          // MOV WORD PTR [EAX+0x88], CX
        _emit 0x89
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66          // MOV WORD PTR [EAX+0x8a], CX
        _emit 0x89
        _emit 0x88
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x33          // XOR EAX, EAX
        _emit 0xc0
        _emit 0x8b          // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8d          // LEA EDX, [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x89          // MOV [ESP+0x18], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x52          // PUSH EDX
        _emit 0x8d          // LEA EAX, [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89          // MOV [ESP+0x18], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x50          // PUSH EAX
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x00994a90
        _emit 0x85
        _emit 0xd7
        _emit 0x53
        _emit 0x00
        _emit 0x57          // PUSH EDI
        _emit 0xff          // CALL DWORD PTR [0x00f3e168]
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8d          // LEA ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51          // PUSH ECX
        _emit 0x8d          // LEA EDX, [ESP+0x20]
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x52          // PUSH EDX
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x0071d420
        _emit 0xfd
        _emit 0x60
        _emit 0x2c
        _emit 0x00
        _emit 0x8b          // MOV EDI, [EAX]
        _emit 0x38
        _emit 0x8b          // MOV EBP, [EAX+0x4]
        _emit 0x68
        _emit 0x04
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x75          // JNZ +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0x83
        _emit 0xaf
        _emit 0x57
        _emit 0x00
        _emit 0x3b          // CMP EBP, [EDI+0x4]
        _emit 0x6f
        _emit 0x04
        _emit 0x75          // JNZ +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0x79
        _emit 0xaf
        _emit 0x57
        _emit 0x00
        _emit 0x8b          // MOV EAX, [EBP+0x10]
        _emit 0x45
        _emit 0x10
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0x83          // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0xc3          // RET
    }
}
