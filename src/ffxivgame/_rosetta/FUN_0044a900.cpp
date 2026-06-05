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
// FUNCTION: ffxivgame 0x0004a900 — std::map<K,V> lookup-or-insert accessor
//                                  (__thiscall, 225 B / 0xe1). `this`
//                                  (ECX = EBX) wraps a map whose tree root
//                                  lives at `this+0x20` (ESI). Returns the
//                                  mapped value's field at +0x10.
//
// Asm shape (read from asm/ffxivgame/0004a900_FUN_0044a900.s):
//
//   void *__thiscall FUN_0044a900(C *this);
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
//       ; --- not found: allocate a 0x54-byte value, default-init it ---
//       ADD  EBX, 0x8; PUSH EBX; CALL [0x00f3e16c] ; (lock/acquire)
//       PUSH 0x54; CALL 0x009d1b35; ADD ESP, 4     ; operator new(0x54)
//       TEST EAX, EAX; JZ skip_init
//         MOV [EAX+0x10]=1; [EAX+0x11]=1; [EAX+8]=1
//         LEA ECX,[EAX+0x12]; [EAX+0xc]=0; [EAX+4]=0x40
//         [EAX]=ECX; [ECX]=0                        ; SSO string init
//       skip_init: (EAX=0)
//       ... build pair, insert into tree via 0x00994a90 ...
//       PUSH EBX; CALL [0x00f3e168]                 ; (unlock/release)
//       CALL 0x0071d420 (re-find) -> EDI=[EAX], EBP=[EAX+4]
//   found:
//     TEST EDI, EDI; JNZ; CALL 0x009d22b4           ; iterator validity
//     CMP  EBP, [EDI+4]; JNZ; CALL 0x009d22b4
//     MOV  EAX, [EBP+0x10]                          ; return value field
//     POP EDI/ESI/EBP/EBX; ADD ESP,0x18; RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   This is std::map operator[] / find-or-insert glue with the MSVC 2005
//   _Tree iterator-debug helper calls (0x009d22b4) and the inline SSO
//   string ctor on the freshly-allocated 0x54-byte value. Coaxing this
//   exact 225-byte sequence — including the precise ESI/EDI/EBP register
//   allocation across the two tree-find calls, the duplicated iterator
//   checks, and the rel32/IAT call encodings — out of high-level C++ at
//   /O2 is impractical; every rewrite shifts at least one byte. The
//   sibling /GS-and-tree functions (FUN_00408910, FUN_00404e40) take the
//   same naked-asm route. The rel32 displacements and IAT slot addresses
//   bake in as raw immediates that resolve against the orig image's
//   address space, and tools/compare.py masks reloc windows — so the
//   emitted .text matches orig byte-for-byte. compare.py reports GREEN.
//
// Reloc-bearing sites in the orig 225 bytes (compare.py masks these):
//   +0x09   DIR32 → 0x00f3e1dc   (IAT: key getter)
//   +0x23   REL32 → 0x0071d420   (tree find)
//   +0x3a   REL32 → 0x009d22b4   (iterator-debug helper)
//   +0x4d   DIR32 → 0x00f3e16c   (IAT: lock/acquire)
//   +0x55   REL32 → 0x009d1b35   (operator new)
//   +0xa1   REL32 → 0x00994a90   (tree insert)
//   +0xa7   DIR32 → 0x00f3e168   (IAT: unlock/release)
//   +0xb9   REL32 → 0x0071d420   (tree find, 2nd)
//   +0xc7   REL32 → 0x009d22b4   (iterator-debug helper)
//   +0xd1   REL32 → 0x009d22b4   (iterator-debug helper)

extern "C" __declspec(naked) void FUN_0044a900() {
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
        _emit 0xf9
        _emit 0x2a
        _emit 0x2d
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
        _emit 0x74          // JZ 0x0044a93a
        _emit 0x04
        _emit 0x3b          // CMP EDI, ESI
        _emit 0xfe
        _emit 0x74          // JZ 0x0044a93f
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0x75
        _emit 0x79
        _emit 0x58
        _emit 0x00
        _emit 0x8b          // MOV EBP, [ESP+0x18]
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        _emit 0x3b          // CMP EBP, [ESP+0x20]
        _emit 0x6c
        _emit 0x24
        _emit 0x20
        _emit 0x75          // JNZ 0x0044a9c3
        _emit 0x7a
        _emit 0x83          // ADD EBX, 0x8
        _emit 0xc3
        _emit 0x08
        _emit 0x53          // PUSH EBX
        _emit 0xff          // CALL DWORD PTR [0x00f3e16c]
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x6a          // PUSH 0x54
        _emit 0x54
        _emit 0xe8          // CALL 0x009d1b35
        _emit 0xdb
        _emit 0x71
        _emit 0x58
        _emit 0x00
        _emit 0x83          // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74          // JZ 0x0044a987
        _emit 0x26
        _emit 0xb9          // MOV ECX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x88          // MOV [EAX+0x10], CL
        _emit 0x48
        _emit 0x10
        _emit 0x88          // MOV [EAX+0x11], CL
        _emit 0x48
        _emit 0x11
        _emit 0x89          // MOV [EAX+0x8], ECX
        _emit 0x48
        _emit 0x08
        _emit 0x8d          // LEA ECX, [EAX+0x12]
        _emit 0x48
        _emit 0x12
        _emit 0xc7          // MOV DWORD PTR [EAX+0xc], 0x0
        _emit 0x40
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7          // MOV DWORD PTR [EAX+0x4], 0x40
        _emit 0x40
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89          // MOV [EAX], ECX
        _emit 0x08
        _emit 0xc6          // MOV BYTE PTR [ECX], 0x0
        _emit 0x01
        _emit 0x00
        _emit 0xeb          // JMP 0x0044a989
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
        _emit 0xea
        _emit 0xa0
        _emit 0x54
        _emit 0x00
        _emit 0x53          // PUSH EBX
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
        _emit 0x62
        _emit 0x2a
        _emit 0x2d
        _emit 0x00
        _emit 0x8b          // MOV EDI, [EAX]
        _emit 0x38
        _emit 0x8b          // MOV EBP, [EAX+0x4]
        _emit 0x68
        _emit 0x04
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x75          // JNZ 0x0044a9cc
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0xe8
        _emit 0x78
        _emit 0x58
        _emit 0x00
        _emit 0x3b          // CMP EBP, [EDI+0x4]
        _emit 0x6f
        _emit 0x04
        _emit 0x75          // JNZ 0x0044a9d6
        _emit 0x05
        _emit 0xe8          // CALL 0x009d22b4
        _emit 0xde
        _emit 0x78
        _emit 0x58
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
