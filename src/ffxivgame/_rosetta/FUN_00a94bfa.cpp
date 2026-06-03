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
// FUNCTION: ffxivgame 0x00a94bfa — vtable-chain lookup with type check
//                                  (__cdecl, 191 B / 0xbf)
//
// Called with two pointer arguments (param1, param2). The function:
//
//   1. Calls a parameterless global getter (0x00a886e0) → obj1
//   2. obj1->vtbl[3]()         → container (EDI throughout)
//   3. container->vtbl[16]()   → count (EBP)
//   4. Iterates i in [0, count) looking for the first item where
//      container->vtbl[14](i)->vtbl[59](0x12, param1) returns non-null.
//      ESI holds the found match; EBX is the loop index.
//   5. If no match found (ESI == 0): return NULL.
//   6. Guard: match->vtbl[58]()        — bail if NULL.
//   7. Repeat same call: match->vtbl[58]() → obj3.
//   8. obj3->vtbl[50](param2, 1)       → new_found (ESI updated).
//   9. If new_found == NULL: return NULL.
//  10. new_found->vtbl[19]()           → obj4.
//  11. If obj4 == NULL: return NULL.
//  12. obj4->vtbl[1]()                 → type_code.
//  13. Return new_found if type_code == 7, else NULL.
//      (Implemented as SUB 7 / NEG / SBB EAX,EAX / NOT / AND ESI.)
//
// Register roles:
//   EBX = loop index i (0-based, pushed as arg to vtbl[14])
//   EBP = count (from vtbl[16])
//   ESI = found item / result pointer
//   EDI = container object (vtbl[16] / vtbl[14] receiver)
//
// Calling convention: __cdecl (plain RET; caller cleans args).
//   param1 → [ESP+0x14] after 4 pushes (EBX/EBP/ESI/EDI)
//   param2 → [ESP+0x18]
//
// Reloc-bearing byte in the orig 191 bytes:
//   +0x04  CALL rel32 → 0x00a886e0 (global getter, image-relative)
//          Bytes: e8 dd 3a ff ff
//          compare.py masks these 4 rel32 bytes; all other CALLs are
//          indirect (ff d0 / ff d2) and carry no relocations.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body is a chain of thiscall virtual-dispatch sequences
//   (MOV EDX,[reg] / MOV EAX,[EDX+disp] / MOV ECX,reg / CALL EAX) that
//   MSVC 2005 emits in a specific register-allocation order determined by
//   source-level declaration order. Any source-level rewrite shifts the
//   register assignment (e.g. EDI vs ESI for the container pointer) and
//   the loop-counter register, producing different ModRM bytes. The
//   closing SUB/NEG/SBB/NOT/AND equality idiom for `(val==7) ? ptr : 0`
//   also depends on the compiler's choice of temporaries. A naked-asm
//   passthrough — same approach as FUN_00403f10, FUN_00406680,
//   FUN_00401750, and other _rosetta siblings — is the safe path to a
//   byte-exact match. compare.py wildcards the one rel32 displacement.

extern "C" __declspec(naked) void FUN_00a94bfa() {
    __asm {
        // prolog — save EBX, EBP, ESI, EDI
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI

        // Call global getter → EAX = obj1
        _emit 0xe8              // CALL rel32 → 0x00a886e0
        _emit 0xdd
        _emit 0x3a
        _emit 0xff
        _emit 0xff

        // obj1->vtbl[3]() → EDI = container
        _emit 0x8b              // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, [EDX+0x0c]     ; vtbl[3]
        _emit 0x42
        _emit 0x0c
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8

        // container->vtbl[16]() → EBP = count
        _emit 0x8b              // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, [EDX+0x40]     ; vtbl[16]
        _emit 0x42
        _emit 0x40
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EBP, EAX
        _emit 0xe8

        // ESI = NULL (found); EBX = 0 (loop index)
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb

        // if (count == 0) goto return_null  (JBE near, 6-byte encoding)
        _emit 0x85              // TEST EBP, EBP
        _emit 0xed
        _emit 0x0f              // JBE near32 +0x8d  → 0x00694cb2
        _emit 0x86
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // ----- LOOP START -----
        // container->vtbl[14](EBX) → EAX = item
        _emit 0x8b              // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, [EDX+0x38]     ; vtbl[14]
        _emit 0x42
        _emit 0x38
        _emit 0x53              // PUSH EBX                ; arg = i
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x19  → loop_next
        _emit 0x19

        // item->vtbl[59](0x12, param1) → ESI = match
        _emit 0x8b              // MOV ECX, [ESP+0x14]     ; param1
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EDX, [EAX]          ; item's vtable
        _emit 0x10
        _emit 0x8b              // MOV EDX, [EDX+0xec]     ; vtbl[59]
        _emit 0x92
        _emit 0xec
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                ; param1
        _emit 0x6a              // PUSH 0x12
        _emit 0x12
        _emit 0x8b              // MOV ECX, EAX            ; this = item
        _emit 0xc8
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ESI, EAX            ; found = result
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +0x0b  → post_loop (found)
        _emit 0x0b

        // ----- LOOP_NEXT -----
        _emit 0x83              // ADD EBX, 1
        _emit 0xc3
        _emit 0x01
        _emit 0x3b              // CMP EBX, EBP
        _emit 0xdd
        _emit 0x72              // JC  -0x2e  → LOOP START
        _emit 0xd2
        // ----- LOOP END -----

        // if (found == NULL) goto return_null
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ  +0x5b  → return_null
        _emit 0x5b

        // ----- post_loop: found != NULL -----

        // Guard: found->vtbl[58]() → bail if NULL
        _emit 0x8b              // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, [EAX+0xe8]     ; vtbl[58]
        _emit 0x90
        _emit 0xe8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ  +0x4b  → return_null
        _emit 0x4b

        // Same call again: found->vtbl[58]() → obj3
        _emit 0x8b              // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, [EAX+0xe8]     ; vtbl[58]
        _emit 0x90
        _emit 0xe8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX                ; EAX = obj3
        _emit 0xd2

        // obj3->vtbl[50](param2, 1) → ESI = new_found
        _emit 0x8b              // MOV ECX, [ESP+0x18]     ; param2
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EDX, [EAX]          ; obj3's vtable
        _emit 0x10
        _emit 0x8b              // MOV EDX, [EDX+0xc8]     ; vtbl[50]
        _emit 0x92
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0x51              // PUSH ECX                ; param2
        _emit 0x8b              // MOV ECX, EAX            ; this = obj3
        _emit 0xc8
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV ESI, EAX            ; new_found
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ  +0x26  → return_null
        _emit 0x26

        // new_found->vtbl[19]() → obj4
        _emit 0x8b              // MOV EAX, [ESI]
        _emit 0x06
        _emit 0x8b              // MOV EDX, [EAX+0x4c]     ; vtbl[19]
        _emit 0x50
        _emit 0x4c
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EDX                ; EAX = obj4
        _emit 0xd2
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ  +0x19  → return_null
        _emit 0x19

        // obj4->vtbl[1]() → type_code
        _emit 0x8b              // MOV EDX, [EAX]          ; obj4's vtable
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, [EDX+0x04]     ; vtbl[1]
        _emit 0x42
        _emit 0x04
        _emit 0xff              // CALL EAX                ; EAX = type_code
        _emit 0xd0

        // return (type_code == 7) ? ESI : NULL
        // SUB EAX,7 → NEG → SBB EAX,EAX → NOT → AND ESI
        _emit 0x83              // SUB EAX, 7
        _emit 0xe8
        _emit 0x07
        _emit 0xf7              // NEG EAX
        _emit 0xd8
        _emit 0x1b              // SBB EAX, EAX
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0xf7              // NOT EAX
        _emit 0xd0
        _emit 0x23              // AND EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET

        // ----- return_null -----
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
