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
// FUNCTION: ffxivgame 0x00413350 — `__thiscall` linked-list iterator / allocator
//                                   dispatch (344 B / 0x158).
//
// Inspection (read from the disassembly at orig RVA 0x00013350):
//
//   __thiscall void FUN_00413350(this);  ECX = this → EBX
//
//   Structure:
//     1. Call vtable[0x2c] on this (some pre-iteration setup).
//     2. Iterate backwards over the intrusive list whose sentinel is at
//        this+0x30 (Link #2 from DetachableHeapSpace), starting at
//        [this+0x38] (link2.prev) and stepping via node[+0x8] (prev ptr).
//        Loop exits when the current pointer equals &this->link2.
//     3. For each node EDI:
//        a. Call node->vtable[0x4] (returns some "actual" object, EAX).
//        b. From EAX[+0xc], call vtable[0x4] again → ESI = sub-object.
//        c. If ESI[+0x24] != 0 (already processed), skip to next node.
//        d. If ESI[+0x2c] == 0 AND ESI[+0x25] == 0 (no parent, no flag):
//              — "simple" path: call ESI[+0x1c]->vtable[0x1c] and
//                ESI[+0x18]->vtable[0x1c]; compute capacity-minus-1 and
//                size from the sub-iterator; jump to shared tail.
//        e. Else — "has-parent" path: walk ESI[+0x2c] chain to find the
//              root whose [+0x25] != 0. If none found, skip node.
//              Otherwise: call ESI[+0x1c]->vtable[0x1c]; load the factory
//              from this[+0xc]; call sub-iterator vtable[0x10], [0xc],
//              [0x8] for three size args; call factory with those three
//              args + current node ECX; if factory returns null, skip.
//              Store result in ESI[+0x20]; initialise it via vtable[0x1c];
//              compute capacity-1 and size from sub-iterator.
//        f. Shared tail: round size down to a power-of-two aligned block,
//              push it; call ECX->vtable[0x4] on ESI[+0x1c] to get one
//              address; call EDI->vtable[0x4] to get another; call the
//              function pointer at this[+0x20] with both addresses + size;
//              set ESI[+0x24] = 1; ADD ESP, 0xc.
//     4. After loop, tail-call this->vtable[0x30].
//
//   No relocations: every CALL is through a vtable-loaded register; all
//   branches are self-contained relative displacements. The 344 bytes are
//   fully position-independent.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.

extern "C" __declspec(naked) void FUN_00413350() {
    __asm {
        // 00013350  SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00013353  PUSH EBX
        _emit 0x53
        // 00013354  MOV EBX,ECX
        _emit 0x8b
        _emit 0xd9
        // 00013356  MOV EAX,dword ptr [EBX]
        _emit 0x8b
        _emit 0x03
        // 00013358  MOV EDX,dword ptr [EAX + 0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001335b  PUSH EDI
        _emit 0x57
        // 0001335c  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001335e  MOV EDI,dword ptr [EBX + 0x38]
        _emit 0x8b
        _emit 0x7b
        _emit 0x38
        // 00013361  LEA EAX,[EBX + 0x30]
        _emit 0x8d
        _emit 0x43
        _emit 0x30
        // 00013364  CMP EDI,EAX
        _emit 0x3b
        _emit 0xf8
        // 00013366  JZ 0x0041349a  (→ epilogue)
        _emit 0x0f
        _emit 0x84
        _emit 0x2e
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 0001336c  PUSH EBP
        _emit 0x55
        // 0001336d  PUSH ESI
        _emit 0x56
        // 0001336e  JMP 0x00413374  (→ loop body first iteration)
        _emit 0xeb
        _emit 0x04
        // 00013370  (loop-back entry) MOV EDI,dword ptr [ESP + 0x10]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 00013374  MOV EAX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00013376  MOV EDX,dword ptr [EAX + 0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013379  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0001337b  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001337d  MOV ECX,dword ptr [EAX + 0xc]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 00013380  MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00013382  MOV EDX,dword ptr [EAX + 0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013385  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00013387  MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 00013389  CMP byte ptr [ESI + 0x24],0x0
        _emit 0x80
        _emit 0x7e
        _emit 0x24
        _emit 0x00
        // 0001338d  MOV EAX,dword ptr [EDI + 0x8]
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // 00013390  MOV dword ptr [ESP + 0x10],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00013394  JNZ 0x0041348b  (→ loop bottom check)
        _emit 0x0f
        _emit 0x85
        _emit 0xf1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001339a  CMP dword ptr [ESI + 0x2c],0x0
        _emit 0x83
        _emit 0x7e
        _emit 0x2c
        _emit 0x00
        // 0001339e  JNZ 0x004133de  (→ has-parent path)
        _emit 0x75
        _emit 0x3e
        // 000133a0  CMP byte ptr [ESI + 0x25],0x0
        _emit 0x80
        _emit 0x7e
        _emit 0x25
        _emit 0x00
        // 000133a4  JNZ 0x004133de  (→ has-parent path)
        _emit 0x75
        _emit 0x38
        // 000133a6  MOV ECX,dword ptr [ESI + 0x1c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x1c
        // 000133a9  MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 000133ab  MOV EAX,dword ptr [EDX + 0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 000133ae  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000133b0  MOV ECX,dword ptr [ESI + 0x18]
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 000133b3  MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 000133b5  MOV EAX,dword ptr [EDX + 0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 000133b8  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000133ba  MOV EDX,dword ptr [ESI + 0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 000133bd  MOV EAX,dword ptr [EDX + 0xc]
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 000133c0  LEA EDI,[ESI + 0x4]
        _emit 0x8d
        _emit 0x7e
        _emit 0x04
        // 000133c3  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000133c5  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000133c7  MOV EDX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 000133c9  LEA EBP,[EAX + -0x1]
        _emit 0x8d
        _emit 0x68
        _emit 0xff
        // 000133cc  MOV EAX,dword ptr [EDX + 0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 000133cf  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000133d1  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000133d3  MOV ECX,dword ptr [ESI + 0x18]
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 000133d6  MOV EDI,dword ptr [ESI + 0x1c]
        _emit 0x8b
        _emit 0x7e
        _emit 0x1c
        // 000133d9  JMP 0x00413466  (→ shared tail)
        _emit 0xe9
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000133de  (has-parent path) MOV EAX,dword ptr [ESI + 0x2c]
        _emit 0x8b
        _emit 0x46
        _emit 0x2c
        // 000133e1  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000133e3  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 000133e5  JZ 0x004133f0  (parent is null, ECX = ESI)
        _emit 0x74
        _emit 0x09
        // 000133e7  (chain walk) MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 000133e9  MOV EAX,dword ptr [ECX + 0x2c]
        _emit 0x8b
        _emit 0x41
        _emit 0x2c
        // 000133ec  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000133ee  JNZ 0x004133e7  (continue chain walk)
        _emit 0x75
        _emit 0xf7
        // 000133f0  CMP byte ptr [ECX + 0x25],0x0
        _emit 0x80
        _emit 0x79
        _emit 0x25
        _emit 0x00
        // 000133f4  JZ 0x0041348b  (no root with flag, skip node)
        _emit 0x0f
        _emit 0x84
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000133fa  MOV ECX,dword ptr [ESI + 0x1c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x1c
        // 000133fd  MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 000133ff  MOV EAX,dword ptr [EDX + 0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 00013402  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013404  MOV EAX,dword ptr [EBX + 0xc]
        _emit 0x8b
        _emit 0x43
        _emit 0x0c
        // 00013407  MOV EBP,dword ptr [EAX]
        _emit 0x8b
        _emit 0x28
        // 00013409  MOV EDX,dword ptr [ESI + 0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0001340c  LEA EDI,[ESI + 0x4]
        _emit 0x8d
        _emit 0x7e
        _emit 0x04
        // 0001340f  MOV dword ptr [ESP + 0x14],EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00013413  MOV EAX,dword ptr [EDX + 0x10]
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 00013416  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00013418  ADD EBP,0xc
        _emit 0x83
        _emit 0xc5
        _emit 0x0c
        // 0001341b  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001341d  MOV EDX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 0001341f  PUSH EAX
        _emit 0x50
        // 00013420  MOV EAX,dword ptr [EDX + 0xc]
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 00013423  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00013425  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013427  MOV EDX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 00013429  PUSH EAX
        _emit 0x50
        // 0001342a  MOV EAX,dword ptr [EDX + 0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0001342d  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0001342f  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013431  MOV ECX,dword ptr [ESP + 0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00013435  MOV EDX,dword ptr [EBP]
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00013438  PUSH EAX
        _emit 0x50
        // 00013439  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001343b  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001343d  JZ 0x0041348b  (factory returned null, skip)
        _emit 0x74
        _emit 0x4c
        // 0001343f  MOV dword ptr [ESI + 0x20],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x20
        // 00013442  MOV EDX,dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 00013444  MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 00013446  MOV EAX,dword ptr [EDX + 0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 00013449  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001344b  MOV EDX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 0001344d  MOV EAX,dword ptr [EDX + 0xc]
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 00013450  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00013452  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013454  MOV EDX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 00013456  LEA EBP,[EAX + -0x1]
        _emit 0x8d
        _emit 0x68
        _emit 0xff
        // 00013459  MOV EAX,dword ptr [EDX + 0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0001345c  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0001345e  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013460  MOV ECX,dword ptr [ESI + 0x1c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x1c
        // 00013463  MOV EDI,dword ptr [ESI + 0x20]
        _emit 0x8b
        _emit 0x7e
        _emit 0x20
        // 00013466  (shared tail) MOV EDX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00013468  ADD EAX,EBP
        _emit 0x03
        _emit 0xc5
        // 0001346a  NOT EBP
        _emit 0xf7
        _emit 0xd5
        // 0001346c  AND EAX,EBP
        _emit 0x23
        _emit 0xc5
        // 0001346e  MOV EBP,dword ptr [EBX + 0x20]
        _emit 0x8b
        _emit 0x6b
        _emit 0x20
        // 00013471  PUSH EAX
        _emit 0x50
        // 00013472  MOV EAX,dword ptr [EDX + 0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00013475  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013477  MOV EDX,dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 00013479  PUSH EAX
        _emit 0x50
        // 0001347a  MOV EAX,dword ptr [EDX + 0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001347d  MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0001347f  CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00013481  PUSH EAX
        _emit 0x50
        // 00013482  CALL EBP
        _emit 0xff
        _emit 0xd5
        // 00013484  MOV byte ptr [ESI + 0x24],0x1
        _emit 0xc6
        _emit 0x46
        _emit 0x24
        _emit 0x01
        // 00013488  ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0001348b  (loop bottom) LEA EAX,[EBX + 0x30]
        _emit 0x8d
        _emit 0x43
        _emit 0x30
        // 0001348e  CMP dword ptr [ESP + 0x10],EAX
        _emit 0x39
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00013492  JNZ 0x00413370  (→ loop back)
        _emit 0x0f
        _emit 0x85
        _emit 0xd8
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00013498  POP ESI
        _emit 0x5e
        // 00013499  POP EBP
        _emit 0x5d
        // 0001349a  MOV EDX,dword ptr [EBX]
        _emit 0x8b
        _emit 0x13
        // 0001349c  MOV EAX,dword ptr [EDX + 0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 0001349f  POP EDI
        _emit 0x5f
        // 000134a0  MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 000134a2  POP EBX
        _emit 0x5b
        // 000134a3  ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 000134a6  JMP EAX  (tail call → vtable[0x30])
        _emit 0xff
        _emit 0xe0
    }
}
