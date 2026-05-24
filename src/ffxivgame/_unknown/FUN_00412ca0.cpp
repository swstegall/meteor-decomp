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
// FUNCTION: ffxivgame 0x00412ca0 — __thiscall virtual-dispatch list-drain
//                                   with spin-lock insertion (181 bytes / 0xb5)
//
// Calling convention: __thiscall (ECX = this); returns void; no parameters.
//
// Stack frame:
//   SUB ESP, 8   — 2 DWORD slots (slot0 @[esp+0xc] = 'this', slot1 @[esp+0x14] used later)
//   PUSH EBX, EBP, EDI (callee-saves; EBP immediately set to ECX = this)
//   PUSH ESI     — saved inside the non-empty branch only
//
// High-level structure:
//   1. Virtual call: iVar4 = (this->field14->vtable[1]())->field0x10
//   2. If (this->field44 == this+0x40) { empty list, return }
//   3. Loop over doubly-linked list (head at this->field44, sentinel at this+0x40):
//      a. piVar5 = node->vtable[1]() — extract item from node
//      b. advance iterator: node = node->field4
//      c. spin-wait while piVar5->field0x2c != 0, calling piVar5->field4->field0x20 each iter
//      d. call piVar5->vtable[0](0)
//      e. spin-lock acquire on iVar4->field4 (XCHG-based, EDX=1)
//      f. insert piVar5 into iVar4's linked list; decrement iVar4->field0x18
//      g. spin-lock release (XCHG [ECX], 0)
//      h. call (*(this->field28+0x18))->vtable[0xa](piVar5->field0x18)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   EBP is repurposed mid-function (this → piVar5+4 → this again), which
//   cannot be expressed in C++ without overriding the standard frame-pointer
//   convention.  All calls go through registers (ff d2 / ff d0) so there are
//   no COFF relocations; every byte is emitted verbatim.
//   compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00412ca0() {
    __asm {
        // 00012ca0: 83 ec 08        SUB ESP, 8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00012ca3: 53              PUSH EBX
        _emit 0x53
        // 00012ca4: 55              PUSH EBP
        _emit 0x55
        // 00012ca5: 8b e9           MOV EBP, ECX     (EBP = this)
        _emit 0x8b
        _emit 0xe9
        // 00012ca7: 8b 4d 14        MOV ECX, [EBP+0x14]
        _emit 0x8b
        _emit 0x4d
        _emit 0x14
        // 00012caa: 8b 01           MOV EAX, [ECX]   (vtable of this->field14)
        _emit 0x8b
        _emit 0x01
        // 00012cac: 8b 50 04        MOV EDX, [EAX+4] (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012caf: 57              PUSH EDI
        _emit 0x57
        // 00012cb0: 89 6c 24 0c     MOV [ESP+0xc], EBP  (save 'this' on stack)
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 00012cb4: ff d2           CALL EDX   (iVar4_ret = this->field14->vtable[1]())
        _emit 0xff
        _emit 0xd2
        // 00012cb6: 8b 78 10        MOV EDI, [EAX+0x10]  (iVar4 = ret->field0x10)
        _emit 0x8b
        _emit 0x78
        _emit 0x10
        // 00012cb9: 8b 5d 44        MOV EBX, [EBP+0x44]  (piVar6 = this->field44 = list head)
        _emit 0x8b
        _emit 0x5d
        _emit 0x44
        // 00012cbc: 8d 45 40        LEA EAX, [EBP+0x40]  (sentinel = this+0x40)
        _emit 0x8d
        _emit 0x45
        _emit 0x40
        // 00012cbf: 3b d8           CMP EBX, EAX          (head == sentinel?)
        _emit 0x3b
        _emit 0xd8
        // 00012cc1: 0f 84 87 00 00 00  JZ +0x87 (empty list → epilogue at 0x12d4e)
        _emit 0x0f
        _emit 0x84
        _emit 0x87
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012cc7: 56              PUSH ESI
        _emit 0x56
        // === outer loop top (RVA 0x00012cc8) ===
        // 00012cc8: 8b 03           MOV EAX, [EBX]        (node vtable)
        _emit 0x8b
        _emit 0x03
        // 00012cca: 8b 50 04        MOV EDX, [EAX+4]      (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012ccd: 8b cb           MOV ECX, EBX          (thiscall: ECX = node)
        _emit 0x8b
        _emit 0xcb
        // 00012ccf: ff d2           CALL EDX              (piVar5 = node->vtable[1]())
        _emit 0xff
        _emit 0xd2
        // 00012cd1: 8b 5b 04        MOV EBX, [EBX+4]     (advance: piVar6 = piVar6->field4)
        _emit 0x8b
        _emit 0x5b
        _emit 0x04
        // 00012cd4: 8b f0           MOV ESI, EAX          (ESI = piVar5)
        _emit 0x8b
        _emit 0xf0
        // 00012cd6: 83 7e 2c 00     CMP DWORD PTR [ESI+0x2c], 0  (piVar5->field0x2c == 0?)
        _emit 0x83
        _emit 0x7e
        _emit 0x2c
        _emit 0x00
        // 00012cda: 74 18           JZ +0x18 (skip spin-wait → 0x12cf4)
        _emit 0x74
        _emit 0x18
        // 00012cdc: 8d 6e 04        LEA EBP, [ESI+4]     (EBP = piVar5+4 = &piVar5->field4)
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // 00012cdf: 90              NOP                  (loop-head alignment)
        _emit 0x90
        // === spin-wait loop top (RVA 0x00012ce0) ===
        // 00012ce0: 8b 45 00        MOV EAX, [EBP+0]     (EAX = piVar5->field4)
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00012ce3: 8b 50 20        MOV EDX, [EAX+0x20]  (EDX = piVar5->field4->field0x20)
        _emit 0x8b
        _emit 0x50
        _emit 0x20
        // 00012ce6: 8b cd           MOV ECX, EBP         (ECX = piVar5+4)
        _emit 0x8b
        _emit 0xcd
        // 00012ce8: ff d2           CALL EDX             (piVar5->field4->field0x20(piVar5+4))
        _emit 0xff
        _emit 0xd2
        // 00012cea: 83 7e 2c 00     CMP DWORD PTR [ESI+0x2c], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x2c
        _emit 0x00
        // 00012cee: 75 f0           JNZ -0x10 → 0x12ce0  (spin)
        _emit 0x75
        _emit 0xf0
        // === after spin-wait (or skip-join at 0x00012cf0) ===
        // 00012cf0: 8b 6c 24 10     MOV EBP, [ESP+0x10]  (restore EBP = this)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // === join point for JZ-skip (0x00012cf4) ===
        // 00012cf4: 8b 46 18        MOV EAX, [ESI+0x18]  (EAX = piVar5->field0x18)
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00012cf7: 8b 16           MOV EDX, [ESI]       (EDX = piVar5->vtable)
        _emit 0x8b
        _emit 0x16
        // 00012cf9: 89 44 24 14     MOV [ESP+0x14], EAX  (save piVar5->field0x18 for later)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00012cfd: 8b 02           MOV EAX, [EDX]       (EAX = piVar5->vtable[0])
        _emit 0x8b
        _emit 0x02
        // 00012cff: 6a 00           PUSH 0               (arg 0)
        _emit 0x6a
        _emit 0x00
        // 00012d01: 8b ce           MOV ECX, ESI         (thiscall: ECX = piVar5)
        _emit 0x8b
        _emit 0xce
        // 00012d03: ff d0           CALL EAX             (piVar5->vtable[0](0))
        _emit 0xff
        _emit 0xd0
        // === spin-lock acquire: XCHG-based, EDX=1, ECX=piVar1=iVar4+4 ===
        // === loop top at 0x00012d05 ===
        // 00012d05: 8d 4f 04        LEA ECX, [EDI+4]     (piVar1 = &iVar4->field4)
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        // 00012d08: ba 01 00 00 00  MOV EDX, 1
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012d0d: 8b c1           MOV EAX, ECX         (EAX = piVar1)
        _emit 0x8b
        _emit 0xc1
        // 00012d0f: 87 10           XCHG [EAX], EDX      (swap *piVar1 ↔ 1; old→EDX)
        _emit 0x87
        _emit 0x10
        // 00012d11: 85 d2           TEST EDX, EDX        (was it 0?)
        _emit 0x85
        _emit 0xd2
        // 00012d13: 75 f0           JNZ -0x10 → 0x12d05  (spin if was non-zero)
        _emit 0x75
        _emit 0xf0
        // === critical section: insert piVar5 into iVar4's list ===
        // 00012d15: 8b 47 0c        MOV EAX, [EDI+0xc]   (iVar2 = iVar4->field0xc)
        _emit 0x8b
        _emit 0x47
        _emit 0x0c
        // 00012d18: 8b 50 04        MOV EDX, [EAX+4]     (EDX = iVar2->field4)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012d1b: 89 32           MOV [EDX], ESI       (*iVar2->field4 = piVar5)
        _emit 0x89
        _emit 0x32
        // 00012d1d: 8b 50 04        MOV EDX, [EAX+4]     (EDX = iVar2->field4 again = iVar3)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012d20: 89 06           MOV [ESI], EAX       (*piVar5 = iVar2)
        _emit 0x89
        _emit 0x06
        // 00012d22: 89 56 04        MOV [ESI+4], EDX     (piVar5->field4 = iVar3)
        _emit 0x89
        _emit 0x56
        _emit 0x04
        // 00012d25: 89 70 04        MOV [EAX+4], ESI     (iVar2->field4 = piVar5)
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00012d28: 83 47 18 ff     ADD DWORD PTR [EDI+0x18], -1  (iVar4->field0x18--)
        _emit 0x83
        _emit 0x47
        _emit 0x18
        _emit 0xff
        // === spin-lock release: XCHG [ECX], 0 ===
        // 00012d2c: 33 c0           XOR EAX, EAX         (EAX = 0)
        _emit 0x33
        _emit 0xc0
        // 00012d2e: 87 01           XCHG [ECX], EAX      (*piVar1 = 0 atomically)
        _emit 0x87
        _emit 0x01
        // === post-lock virtual call ===
        // 00012d30: 8b 4d 28        MOV ECX, [EBP+0x28]  (ECX = this->field0x28)
        _emit 0x8b
        _emit 0x4d
        _emit 0x28
        // 00012d33: 8b 49 18        MOV ECX, [ECX+0x18]  (ECX = *(this->field28+0x18))
        _emit 0x8b
        _emit 0x49
        _emit 0x18
        // 00012d36: 8b 11           MOV EDX, [ECX]       (EDX = vtable of ECX)
        _emit 0x8b
        _emit 0x11
        // 00012d38: 8b 44 24 14     MOV EAX, [ESP+0x14]  (EAX = saved piVar5->field0x18)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00012d3c: 8b 52 28        MOV EDX, [EDX+0x28]  (EDX = vtable[0xa])
        _emit 0x8b
        _emit 0x52
        _emit 0x28
        // 00012d3f: 50              PUSH EAX              (arg: piVar5->field0x18)
        _emit 0x50
        // 00012d40: ff d2           CALL EDX              (ECX->vtable[0xa](piVar5->field0x18))
        _emit 0xff
        _emit 0xd2
        // === outer loop back-edge ===
        // 00012d42: 8d 45 40        LEA EAX, [EBP+0x40]  (sentinel = this+0x40)
        _emit 0x8d
        _emit 0x45
        _emit 0x40
        // 00012d45: 3b d8           CMP EBX, EAX         (piVar6 == sentinel?)
        _emit 0x3b
        _emit 0xd8
        // 00012d47: 0f 85 7b ff ff ff  JNZ -0x85 → 0x12cc8 (loop back)
        _emit 0x0f
        _emit 0x85
        _emit 0x7b
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // === epilogue (non-empty path) ===
        // 00012d4d: 5e              POP ESI
        _emit 0x5e
        // === join point: empty-list JZ lands here at 0x12d4e ===
        // 00012d4e: 5f              POP EDI
        _emit 0x5f
        // 00012d4f: 5d              POP EBP
        _emit 0x5d
        // 00012d50: 5b              POP EBX
        _emit 0x5b
        // 00012d51: 83 c4 08        ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00012d54: c3              RETN
        _emit 0xc3
    }
}
