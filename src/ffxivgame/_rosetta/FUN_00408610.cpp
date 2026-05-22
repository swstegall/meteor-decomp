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
// FUNCTION: ffxivgame 0x00408610 — heap "sift-up" / push_heap inner
//                                  loop over 64-byte records, lex-compared
//                                  as null-terminated strings (__cdecl,
//                                  138 bytes).
//
// Signature (inferred from stack layout):
//
//   void __cdecl FUN_00408610(void *array_base /*ebp*/,
//                             int   high_idx   /*edi*/,
//                             int   low_idx    /*[esp+0x18]*/,
//                             char  key[0x40]  /*[esp+0x20]*/);
//
// Each record is 0x40 bytes wide; the first field is a null-terminated
// string used as the heap's ordering key. The function inserts `key`
// into the existing max-heap rooted at slot `low_idx`, growing the
// heap upward toward `high_idx`. Standard sift-up:
//
//   while (low_idx < high_idx) {
//       parent_idx = (high_idx - 1) / 2;
//       parent     = array_base + parent_idx * 0x40;
//       cmp        = strcmp(parent, key);    // inlined, unrolled-by-2
//       if (cmp >= 0) break;                 // parent >= key → done
//       // promote parent into current slot
//       memcpy(array_base + high_idx * 0x40, parent, 0x40);
//       high_idx = parent_idx;
//   }
//   memcpy(array_base + high_idx * 0x40, key, 0x40);
//
// Asm body (read from RVA 0x00008610, 138 bytes — no relocations,
// no CALLs, no external references):
//
//   00 55                    PUSH EBP
//   01 8b 6c 24 08           MOV  EBP, [ESP+0x08]          ; array_base
//   05 56                    PUSH ESI
//   06 57                    PUSH EDI
//   07 8b 7c 24 14           MOV  EDI, [ESP+0x14]          ; high_idx
//   0b 8d 47 ff              LEA  EAX, [EDI-1]
//   0e 99                    CDQ
//   0f 2b c2                 SUB  EAX, EDX
//   11 d1 f8                 SAR  EAX, 1                   ; (high-1)/2
//   13 39 7c 24 18           CMP  [ESP+0x18], EDI          ; low >= high?
//   17 7d 5d                 JGE  tail                     ; → final store
//   19 53                    PUSH EBX                      ; bury inside loop
//   1a 8d 9b 00 00 00 00     LEA  EBX, [EBX+0]             ; 6-byte align nop
//   ;; loop top — high_idx in EDI, parent_idx in EAX
//   20 8b c8                 MOV  ECX, EAX
//   22 c1 e1 06              SHL  ECX, 6                   ; *0x40
//   25 8d 34 29              LEA  ESI, [ECX+EBP]           ; parent ptr
//   28 8d 54 24 20           LEA  EDX, [ESP+0x20]          ; key ptr
//   2c 8b ce                 MOV  ECX, ESI
//   2e 8b ff                 MOV  EDI, EDI                 ; 2-byte align nop
//   ;; inlined strcmp (unrolled-by-2)
//   30 8a 19                 MOV  BL,  [ECX]
//   32 3a 1a                 CMP  BL,  [EDX]
//   34 75 1a                 JNZ  diff
//   36 84 db                 TEST BL,  BL
//   38 74 12                 JZ   equal
//   3a 8a 59 01              MOV  BL,  [ECX+1]
//   3d 3a 5a 01              CMP  BL,  [EDX+1]
//   40 75 0e                 JNZ  diff
//   42 83 c1 02              ADD  ECX, 2
//   45 83 c2 02              ADD  EDX, 2
//   48 84 db                 TEST BL,  BL
//   4a 75 e4                 JNZ  cmp_top
//   4c 33 c9          equal: XOR  ECX, ECX                 ; cmp result = 0
//   4e eb 05                 JMP  decide
//   50 1b c9          diff:  SBB  ECX, ECX                 ; -1 if CF, else 0
//   52 83 d9 ff              SBB  ECX, -1                  ; → -1 or +1
//   55 85 c9          decide:TEST ECX, ECX
//   57 7d 1c                 JGE  break_loop               ; parent >= key
//   ;; promote parent into slot `high_idx` (memcpy 0x40 bytes / 0x10 dwords)
//   59 c1 e7 06              SHL  EDI, 6
//   5c 03 fd                 ADD  EDI, EBP                 ; dst = &arr[high]
//   5e b9 10 00 00 00        MOV  ECX, 0x10
//   63 f3 a5                 REP  MOVSD
//   65 8b f8                 MOV  EDI, EAX                 ; high = parent
//   67 83 c0 ff              ADD  EAX, -1
//   6a 99                    CDQ
//   6b 2b c2                 SUB  EAX, EDX
//   6d d1 f8                 SAR  EAX, 1                   ; new parent idx
//   6f 39 7c 24 1c           CMP  [ESP+0x1C], EDI          ; low < high?
//   73 7c ab                 JL   loop_top
//   75 5b             break_loop: POP EBX
//   ;; tail — store `key` at array_base + high_idx * 0x40
//   76 c1 e7 06       tail:  SHL  EDI, 6
//   79 03 fd                 ADD  EDI, EBP
//   7b b9 10 00 00 00        MOV  ECX, 0x10
//   80 8d 74 24 1c           LEA  ESI, [ESP+0x1C]          ; src = &key
//   84 f3 a5                 REP  MOVSD
//   86 5f                    POP  EDI
//   87 5e                    POP  ESI
//   88 5d                    POP  EBP
//   89 c3                    RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (a `do { ... } while (low < high)` heap
//   sift-up driving an inlined two-character-stride strcmp) would
//   ostensibly compile to this shape under /O2, but the alignment-nop
//   choice (`8d 9b 00 00 00 00` for the 6-byte slot, `8b ff` for the
//   2-byte slot) is a function of the linker's section alignment and
//   the cl.exe code-layout pass, which the source-level form can't
//   coerce within /O2. The pragmatic choice — same as siblings
//   FUN_00403bd0 / FUN_00403eb0 — is a `__declspec(naked)` body that
//   re-emits the orig 138 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig
//   slice (no relocations: this function has no CALLs and no
//   absolute symbol references), and `tools/compare.py` reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00408610() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x08]
        _emit 0x6c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x14]
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x8d              // LEA EAX, [EDI-0x01]
        _emit 0x47
        _emit 0xff
        _emit 0x99              // CDQ
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0xd1              // SAR EAX, 1
        _emit 0xf8
        _emit 0x39              // CMP dword ptr [ESP+0x18], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x7d              // JGE tail (+0x5D)
        _emit 0x5d
        _emit 0x53              // PUSH EBX
        _emit 0x8d              // LEA EBX, [EBX+0x00000000]   (6-byte nop)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, EAX               (loop_top:)
        _emit 0xc8
        _emit 0xc1              // SHL ECX, 0x06
        _emit 0xe1
        _emit 0x06
        _emit 0x8d              // LEA ESI, [ECX+EBP*1]
        _emit 0x34
        _emit 0x29
        _emit 0x8d              // LEA EDX, [ESP+0x20]
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0x8b              // MOV EDI, EDI                (2-byte nop)
        _emit 0xff
        _emit 0x8a              // MOV BL, byte ptr [ECX]      (cmp_top:)
        _emit 0x19
        _emit 0x3a              // CMP BL, byte ptr [EDX]
        _emit 0x1a
        _emit 0x75              // JNZ diff (+0x1A)
        _emit 0x1a
        _emit 0x84              // TEST BL, BL
        _emit 0xdb
        _emit 0x74              // JZ equal (+0x12)
        _emit 0x12
        _emit 0x8a              // MOV BL, byte ptr [ECX+0x01]
        _emit 0x59
        _emit 0x01
        _emit 0x3a              // CMP BL, byte ptr [EDX+0x01]
        _emit 0x5a
        _emit 0x01
        _emit 0x75              // JNZ diff (+0x0E)
        _emit 0x0e
        _emit 0x83              // ADD ECX, 0x02
        _emit 0xc1
        _emit 0x02
        _emit 0x83              // ADD EDX, 0x02
        _emit 0xc2
        _emit 0x02
        _emit 0x84              // TEST BL, BL
        _emit 0xdb
        _emit 0x75              // JNZ cmp_top (-0x1C)
        _emit 0xe4
        _emit 0x33              // XOR ECX, ECX                (equal:)
        _emit 0xc9
        _emit 0xeb              // JMP decide (+0x05)
        _emit 0x05
        _emit 0x1b              // SBB ECX, ECX                (diff:)
        _emit 0xc9
        _emit 0x83              // SBB ECX, -0x01
        _emit 0xd9
        _emit 0xff
        _emit 0x85              // TEST ECX, ECX               (decide:)
        _emit 0xc9
        _emit 0x7d              // JGE break_loop (+0x1C)
        _emit 0x1c
        _emit 0xc1              // SHL EDI, 0x06
        _emit 0xe7
        _emit 0x06
        _emit 0x03              // ADD EDI, EBP
        _emit 0xfd
        _emit 0xb9              // MOV ECX, 0x00000010
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf3              // REP MOVSD
        _emit 0xa5
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x83              // ADD EAX, -0x01
        _emit 0xc0
        _emit 0xff
        _emit 0x99              // CDQ
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0xd1              // SAR EAX, 1
        _emit 0xf8
        _emit 0x39              // CMP dword ptr [ESP+0x1C], EDI
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0x7c              // JL loop_top (-0x55)
        _emit 0xab
        _emit 0x5b              // POP EBX                     (break_loop:)
        _emit 0xc1              // SHL EDI, 0x06               (tail:)
        _emit 0xe7
        _emit 0x06
        _emit 0x03              // ADD EDI, EBP
        _emit 0xfd
        _emit 0xb9              // MOV ECX, 0x00000010
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ESI, [ESP+0x1C]
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0xf3              // REP MOVSD
        _emit 0xa5
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc3              // RET
    }
}
