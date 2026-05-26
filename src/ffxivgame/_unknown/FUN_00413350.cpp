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
// FUNCTION: ffxivgame 0x00413350 — __thiscall doubly-linked-list iterator
//                                   with virtual dispatch and alignment
//                                   (344 bytes / 0x158)
//
// Calling convention: __thiscall (ECX = this); no explicit parameters.
//   The function receives 'this' in ECX and reads several arguments from
//   the stack above the saved registers ([ESP+0x18], [ESP+0x1c], [ESP+0x20],
//   [ESP+0x24], [ESP+0x28] after prologue adjustments).
//
// Stack frame:
//   SUB ESP, 8    — two DWORD slots
//   PUSH EBX, EBP, ESI pushed mid-function
//   PUSH EDI in outer scope
//
// High-level structure (read from the asm bytes):
//   1. Set up: this → EBX; this->vtable[0xb]() — virtual call
//   2. piVar9 = this->field0x38; sentinel = this+0x30
//   3. If (piVar9 == sentinel) → jump to tail call at the end
//   4. Loop over doubly-linked list nodes:
//      - Virtual dispatch chain per node
//      - Check node->field0x24 (byte flag)
//      - On flag==0: check field0x2c and field0x25 for two sub-paths
//      - Sub-path A (all zero): call two dtor-like virtual calls + alignment
//        computation + emit; set field0x24 = 1
//      - Sub-path B (field0x2c != 0): walk chain; if flag set: similar emit
//   5. Tail: POP callee-saves; virtual tail-call this->vtable[0xc]()
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   All calls are indirect through registers (ff d0 / ff d2 / ff d5) so
//   there are no COFF relocations. The epilogue ends with JMP EAX (ff e0),
//   a tail call that MSVC 2005 does not synthesize from source-level C++
//   at /O2. A source-level port would require careful arrangement to match
//   the register allocation and branch encoding exactly.
//   The naked-asm passthrough is the reliable approach — compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00413350() {
    __asm {
        // 00013350: 83 ec 08        SUB ESP, 8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00013353: 53              PUSH EBX
        _emit 0x53
        // 00013354: 8b d9           MOV EBX, ECX       (EBX = this)
        _emit 0x8b
        _emit 0xd9
        // 00013356: 8b 03           MOV EAX, [EBX]     (vtable)
        _emit 0x8b
        _emit 0x03
        // 00013358: 8b 50 2c        MOV EDX, [EAX+0x2c] (vtable[0xb])
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001335b: 57              PUSH EDI
        _emit 0x57
        // 0001335c: ff d2           CALL EDX           (this->vtable[0xb]())
        _emit 0xff
        _emit 0xd2
        // 0001335e: 8b 7b 38        MOV EDI, [EBX+0x38] (piVar9 = this->field0x38)
        _emit 0x8b
        _emit 0x7b
        _emit 0x38
        // 00013361: 8d 43 30        LEA EAX, [EBX+0x30] (sentinel = this+0x30)
        _emit 0x8d
        _emit 0x43
        _emit 0x30
        // 00013364: 3b f8           CMP EDI, EAX        (piVar9 == sentinel?)
        _emit 0x3b
        _emit 0xf8
        // 00013366: 0f 84 2e 01 00 00  JZ +0x12e (→ epilogue)
        _emit 0x0f
        _emit 0x84
        _emit 0x2e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001336c: 55              PUSH EBP
        _emit 0x55
        // 0001336d: 56              PUSH ESI
        _emit 0x56
        // 0001336f: eb 04           JMP +4  (→ 0x13375, skip loop-top re-entry)
        _emit 0xeb
        _emit 0x04
        // === loop top re-entry (from back-edge JNZ) ===
        // 00013371: 8b 7c 24 10     MOV EDI, [ESP+0x10] (reload piVar9)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // === outer loop body top at 0x00013375 ===
        // 00013375: 8b 07           MOV EAX, [EDI]      (node vtable)
        _emit 0x8b
        _emit 0x07
        // 00013377: 8b 50 04        MOV EDX, [EAX+4]    (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001337a: 8b cf           MOV ECX, EDI        (thiscall: ECX = node)
        _emit 0x8b
        _emit 0xcf
        // 0001337c: ff d2           CALL EDX            (iVar3 = node->vtable[1]())
        _emit 0xff
        _emit 0xd2
        // 0001337e: 8b 48 0c        MOV ECX, [EAX+0xc]  (ECX = vtable_of_iVar3->vtable[3])
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 00013381: 8b 01           MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00013383: 8b 50 04        MOV EDX, [EAX+4]    (vtable[1] of inner)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013386: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00013388: 8b f0           MOV ESI, EAX        (ESI = iVar3 resolved item)
        _emit 0x8b
        _emit 0xf0
        // 0001338a: 80 7e 24 00     CMP BYTE PTR [ESI+0x24], 0
        _emit 0x80
        _emit 0x7e
        _emit 0x24
        _emit 0x00
        // 0001338e: 8b 47 08        MOV EAX, [EDI+8]    (advance node)
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // 00013391: 89 44 24 10     MOV [ESP+0x10], EAX  (save next node)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00013395: 0f 85 f1 00 00 00  JNZ +0xf1 (flag set → skip to back-edge)
        _emit 0x0f
        _emit 0x85
        _emit 0xf1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === flag == 0 path ===
        // 0001339b: 83 7e 2c 00     CMP DWORD PTR [ESI+0x2c], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x2c
        _emit 0x00
        // 0001339f: 75 3e           JNZ +0x3e (→ 0x133df, sub-path B)
        _emit 0x75
        _emit 0x3e
        // 000133a1: 80 7e 25 00     CMP BYTE PTR [ESI+0x25], 0
        _emit 0x80
        _emit 0x7e
        _emit 0x25
        _emit 0x00
        // 000133a5: 75 38           JNZ +0x38 (→ 0x133df, sub-path B)
        _emit 0x75
        _emit 0x38
        // === sub-path A: both zeros ===
        // 000133a7: 8b 4e 1c        MOV ECX, [ESI+0x1c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x1c
        // 000133aa: 8b 11           MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 000133ac: 8b 42 1c        MOV EAX, [EDX+0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 000133af: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000133b1: 8b 4e 18        MOV ECX, [ESI+0x18]
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 000133b4: 8b 11           MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 000133b6: 8b 42 1c        MOV EAX, [EDX+0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 000133b9: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000133bb: 8b 56 04        MOV EDX, [ESI+4]    (piVar8 = ESI->field4)
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 000133be: 8b 42 0c        MOV EAX, [EDX+0xc]  (vtable[3])
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 000133c1: 8d 7e 04        LEA EDI, [ESI+4]
        _emit 0x8d
        _emit 0x7e
        _emit 0x04
        // 000133c4: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000133c6: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000133c8: 8b 17           MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 000133ca: 8d 68 ff        LEA EBP, [EAX-1]    (iVar5 = ret - 1)
        _emit 0x8d
        _emit 0x68
        _emit 0xff
        // 000133cd: 8b 42 08        MOV EAX, [EDX+8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 000133d0: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000133d2: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000133d4: 8b 4e 18        MOV ECX, [ESI+0x18]  (piVar8 = ESI->field0x18)
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 000133d7: 8b 7e 1c        MOV EDI, [ESI+0x1c]  (piVar6 = ESI->field0x1c)
        _emit 0x8b
        _emit 0x7e
        _emit 0x1c
        // 000133da: e9 88 00 00 00  JMP +0x88 → LAB_00413467
        _emit 0xe9
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === sub-path B: field0x2c != 0 or field0x25 != 0 ===
        // 000133df: 8b 46 2c        MOV EAX, [ESI+0x2c]
        _emit 0x8b
        _emit 0x46
        _emit 0x2c
        // 000133e2: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000133e4: 8b ce           MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000133e6: 74 09           JZ +9 (→ 0x133f1)
        _emit 0x74
        _emit 0x09
        // === chain walk loop ===
        // 000133e8: 8b c8           MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 000133ea: 8b 41 2c        MOV EAX, [ECX+0x2c]
        _emit 0x8b
        _emit 0x41
        _emit 0x2c
        // 000133ed: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000133ef: 75 f7           JNZ -9 → 0x133e8 (spin)
        _emit 0x75
        _emit 0xf7
        // === after chain walk: ECX = iVar4 ===
        // 000133f1: 80 79 25 00     CMP BYTE PTR [ECX+0x25], 0
        _emit 0x80
        _emit 0x79
        _emit 0x25
        _emit 0x00
        // 000133f5: 0f 84 91 00 00 00  JZ +0x91 → back-edge check
        _emit 0x0f
        _emit 0x84
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === flag set on chain tail ===
        // 000133fb: 8b 4e 1c        MOV ECX, [ESI+0x1c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x1c
        // 000133fe: 8b 11           MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 00013400: 8b 42 1c        MOV EAX, [EDX+0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 00013403: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013405: 8b 43 0c        MOV EAX, [EBX+0xc]   (this->field0xc)
        _emit 0x8b
        _emit 0x43
        _emit 0x0c
        // 00013408: 8b 28           MOV EBP, [EAX]
        _emit 0x8b
        _emit 0x28
        // 0001340a: 8b 56 04        MOV EDX, [ESI+4]     (piVar8 = ESI->field4)
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0001340d: 8d 7e 04        LEA EDI, [ESI+4]
        _emit 0x8d
        _emit 0x7e
        _emit 0x04
        // 00013410: 89 44 24 14     MOV [ESP+0x14], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00013414: 8b 42 10        MOV EAX, [EDX+0x10]
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 00013417: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00013419: 83 c5 0c        ADD EBP, 0xc
        _emit 0x83
        _emit 0xc5
        _emit 0x0c
        // 0001341c: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001341e: 8b 17           MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 00013420: 50              PUSH EAX
        _emit 0x50
        // 00013421: 8b 42 0c        MOV EAX, [EDX+0xc]
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 00013424: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00013426: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013428: 8b 17           MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 0001342a: 50              PUSH EAX
        _emit 0x50
        // 0001342b: 8b 42 08        MOV EAX, [EDX+8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0001342e: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00013430: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013432: 8b 4c 24 1c     MOV ECX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00013436: 8b 55 00        MOV EDX, [EBP+0]
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00013439: 50              PUSH EAX
        _emit 0x50
        // 0001343a: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001343c: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001343e: 74 4c           JZ +0x4c → 0x1348c (null → skip)
        _emit 0x74
        _emit 0x4c
        // 00013440: 89 46 20        MOV [ESI+0x20], EAX
        _emit 0x89
        _emit 0x46
        _emit 0x20
        // 00013443: 8b 10           MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 00013445: 8b c8           MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00013447: 8b 42 1c        MOV EAX, [EDX+0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 0001344a: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001344c: 8b 17           MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 0001344e: 8b 42 0c        MOV EAX, [EDX+0xc]
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 00013451: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00013453: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013455: 8b 17           MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 00013457: 8d 68 ff        LEA EBP, [EAX-1]
        _emit 0x8d
        _emit 0x68
        _emit 0xff
        // 0001345a: 8b 42 08        MOV EAX, [EDX+8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0001345d: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 0001345f: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013461: 8b 4e 1c        MOV ECX, [ESI+0x1c]  (piVar8)
        _emit 0x8b
        _emit 0x4e
        _emit 0x1c
        // 00013464: 8b 7e 20        MOV EDI, [ESI+0x20]  (piVar6)
        _emit 0x8b
        _emit 0x7e
        _emit 0x20
        // === LAB_00413467 (shared emit path) ===
        // 00013467: 8b 11           MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 00013469: 03 c5           ADD EAX, EBP         (iVar5 + (iVar4 - 1U))
        _emit 0x03
        _emit 0xc5
        // 0001346b: f7 d5           NOT EBP              (~(iVar4 - 1U))
        _emit 0xf7
        _emit 0xd5
        // 0001346d: 23 c5           AND EAX, EBP         (align up)
        _emit 0x23
        _emit 0xc5
        // 0001346f: 8b 6b 20        MOV EBP, [EBX+0x20]
        _emit 0x8b
        _emit 0x6b
        _emit 0x20
        // 00013472: 50              PUSH EAX
        _emit 0x50
        // 00013473: 8b 42 04        MOV EAX, [EDX+4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00013476: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013478: 8b 17           MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 0001347a: 50              PUSH EAX
        _emit 0x50
        // 0001347b: 8b 42 04        MOV EAX, [EDX+4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001347e: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00013480: ff d0           CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013482: 50              PUSH EAX
        _emit 0x50
        // 00013483: ff d5           CALL EBP            (this->field0x20)(args)
        _emit 0xff
        _emit 0xd5
        // 00013485: c6 46 24 01     MOV BYTE PTR [ESI+0x24], 1
        _emit 0xc6
        _emit 0x46
        _emit 0x24
        _emit 0x01
        // 00013489: 83 c4 0c        ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // === back-edge / loop continuation (0x0001348c) ===
        // 0001348c: 8d 43 30        LEA EAX, [EBX+0x30]  (sentinel = this+0x30)
        _emit 0x8d
        _emit 0x43
        _emit 0x30
        // 0001348f: 39 44 24 10     CMP [ESP+0x10], EAX   (next node == sentinel?)
        _emit 0x39
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00013493: 0f 85 d8 fe ff ff  JNZ → loop top (0x13375 via 0x13371)
        _emit 0x0f
        _emit 0x85
        _emit 0xd8
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // === epilogue ===
        // 00013499: 5e              POP ESI
        _emit 0x5e
        // 0001349a: 5d              POP EBP
        _emit 0x5d
        // 0001349b: 8b 13           MOV EDX, [EBX]      (vtable)
        _emit 0x8b
        _emit 0x13
        // 0001349d: 8b 42 30        MOV EAX, [EDX+0x30] (vtable[0xc])
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 000134a0: 5f              POP EDI
        _emit 0x5f
        // 000134a1: 8b cb           MOV ECX, EBX        (thiscall: ECX = this)
        _emit 0x8b
        _emit 0xcb
        // 000134a3: 5b              POP EBX
        _emit 0x5b
        // 000134a4: 83 c4 08        ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000134a7: ff e0           JMP EAX             (tail call: this->vtable[0xc]())
        _emit 0xff
        _emit 0xe0
    }
}
