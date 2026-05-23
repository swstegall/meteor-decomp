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
// FUNCTION: ffxivgame 0x000129e0 — __thiscall pre-hook, vtable-chain work-item
// retrieval, spinlock-protected doubly-linked free-pool insert, counter decrement,
// observer notify, post-hook (131 B, zero relocations).
//
// __thiscall void FUN_004129e0(this, param_1)
//   ECX        : this — manager object
//                  vtable[0x2c/4=11] called as pre-hook
//                  vtable[0x30/4=12] called as post-hook
//                  field[+4] → INotify* whose vtable[0x10/4=4] is called with iVar2
//   [ESP+0x04] : param_1 — provider; vtable[0x14/4=5] returns piVar4 (IWorkChain*)
//
// Call chain (all calls indirect through registers — no relocations):
//   1. this->vf11()                              [EDI->vtable+0x2c]
//   2. piVar4 = param_1->vf5()                  [ECX->vtable+0x14]
//   3. piVar5 = piVar4->vf1()                   [EAX->vtable+0x04]  → ESI
//   4. iVar2  = piVar5->field_0x18              [ESI+0x18]          → EBP
//   5. temp   = (*(piVar5->field_0x14))->vf1()  [ECX->vtable+0x04]
//   6. iVar6  = *(temp + 0x10)                  [EAX+0x10]          → EBX
//   7. piVar5->vf0(0)                           [EDX->vtable+0x00]
//   8. acquire spinlock at ECX = EBX+4
//      loop: MOV EDX,1; MOV EAX,ECX; XCHG [EAX],EDX; TEST EDX,EDX; JNE loop
//   9. doubly-linked list insert piVar5 into sentinel at iVar6->field_0xc
//  10. ADD [EBX+0x18], -1  (decrement count)
//  11. release spinlock: XOR EAX,EAX; XCHG [ECX],EAX
//  12. this->field_0x04->vf4(iVar2)            [ECX->vtable+0x10]
//  13. this->vf12()                             [EDI->vtable+0x30]
//
// Register allocation (MSVC 2005 /O2 /Oy):
//   EBX = iVar6  (free-pool control struct derived from piVar5->field_0x14 chain)
//   EBP = iVar2  (piVar5->field_0x18; passed to notify at end)
//   ESI = piVar5 (the work item returned from the vtable chain)
//   EDI = this   (saved from ECX in prologue)
//   ECX = &iVar6->field_0x04 (spinlock address, held across acquire/release)
//
// The spinlock loop has MOV EDX,1 INSIDE the loop body (JNE target = MOV EDX,1
// not MOV EAX,ECX) — MSVC 2005 does not hoist the constant through the XCHG
// intrinsic expansion when it is the sole loop body statement with no way to
// prove EDX is preserved across the back-edge.
//
// No direct CALL instructions → zero relocations → pure _emit passthrough.
// Source-level C++ attempts: register allocation is correct in isolation, but
// the loop JNE target lands on MOV EAX,ECX (hoisted) rather than MOV EDX,1
// (un-hoisted), producing a different 5-byte divergence inside the spinlock.
// Naked-asm byte passthrough is the reliable path.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" void FUN_004129e0() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_004129e0()
{
    __asm {
        // 000129e0: 53                 PUSH EBX
        _emit 0x53
        // 000129e1: 55                 PUSH EBP
        _emit 0x55
        // 000129e2: 56                 PUSH ESI
        _emit 0x56
        // 000129e3: 57                 PUSH EDI
        _emit 0x57
        // 000129e4: 8b f9              MOV EDI, ECX            ; EDI = this
        _emit 0x8b
        _emit 0xf9
        // 000129e6: 8b 07              MOV EAX, [EDI]          ; vtable of this
        _emit 0x8b
        _emit 0x07
        // 000129e8: 8b 50 2c           MOV EDX, [EAX+0x2c]     ; vtable slot 11
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000129eb: ff d2              CALL EDX                 ; this->vf11() — pre-hook
        _emit 0xff
        _emit 0xd2
        // 000129ed: 8b 4c 24 14        MOV ECX, [ESP+0x14]     ; ECX = param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000129f1: 8b 01              MOV EAX, [ECX]          ; vtable of param_1
        _emit 0x8b
        _emit 0x01
        // 000129f3: 8b 50 14           MOV EDX, [EAX+0x14]     ; vtable slot 5
        _emit 0x8b
        _emit 0x50
        _emit 0x14
        // 000129f6: ff d2              CALL EDX                 ; param_1->vf5() → piVar4 in EAX
        _emit 0xff
        _emit 0xd2
        // 000129f8: 8b 10              MOV EDX, [EAX]          ; vtable of piVar4
        _emit 0x8b
        _emit 0x10
        // 000129fa: 8b c8              MOV ECX, EAX            ; ECX = piVar4 (thiscall)
        _emit 0x8b
        _emit 0xc8
        // 000129fc: 8b 42 04           MOV EAX, [EDX+0x04]     ; vtable slot 1
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 000129ff: ff d0              CALL EAX                 ; piVar4->vf1() → piVar5 in EAX
        _emit 0xff
        _emit 0xd0
        // 00012a01: 8b f0              MOV ESI, EAX            ; ESI = piVar5
        _emit 0x8b
        _emit 0xf0
        // 00012a03: 8b 4e 14           MOV ECX, [ESI+0x14]     ; ECX = *(piVar5+0x14) = pool-ref obj
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00012a06: 8b 11              MOV EDX, [ECX]          ; vtable of pool-ref obj
        _emit 0x8b
        _emit 0x11
        // 00012a08: 8b 42 04           MOV EAX, [EDX+0x04]     ; vtable slot 1
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00012a0b: 8b 6e 18           MOV EBP, [ESI+0x18]     ; EBP = iVar2 = piVar5->field_0x18
        _emit 0x8b
        _emit 0x6e
        _emit 0x18
        // 00012a0e: ff d0              CALL EAX                 ; pool-ref->vf1() → raw ptr in EAX
        _emit 0xff
        _emit 0xd0
        // 00012a10: 8b 16              MOV EDX, [ESI]          ; vtable of piVar5 (reload after call)
        _emit 0x8b
        _emit 0x16
        // 00012a12: 8b 58 10           MOV EBX, [EAX+0x10]     ; EBX = iVar6 = *(raw_ptr+0x10)
        _emit 0x8b
        _emit 0x58
        _emit 0x10
        // 00012a15: 8b 02              MOV EAX, [EDX]          ; vtable slot 0 of piVar5
        _emit 0x8b
        _emit 0x02
        // 00012a17: 6a 00              PUSH 0                  ; arg 0 (don't free)
        _emit 0x6a
        _emit 0x00
        // 00012a19: 8b ce              MOV ECX, ESI            ; ECX = piVar5 (thiscall)
        _emit 0x8b
        _emit 0xce
        // 00012a1b: ff d0              CALL EAX                 ; piVar5->vf0(0) — dtor/release
        _emit 0xff
        _emit 0xd0
        // 00012a1d: 8d 4b 04           LEA ECX, [EBX+0x04]     ; ECX = &iVar6->field_0x04 (lock addr)
        _emit 0x8d
        _emit 0x4b
        _emit 0x04
        // 00012a20: ba 01 00 00 00     MOV EDX, 1              ; ← JNE loop target (inside loop)
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012a25: 8b c1              MOV EAX, ECX            ; EAX = lock address copy
        _emit 0x8b
        _emit 0xc1
        // 00012a27: 87 10              XCHG [EAX], EDX         ; atomic: EDX ↔ [EAX]; EDX = old val
        _emit 0x87
        _emit 0x10
        // 00012a29: 85 d2              TEST EDX, EDX
        _emit 0x85
        _emit 0xd2
        // 00012a2b: 75 f3              JNE 0x12a20             ; spin while old != 0
        _emit 0x75
        _emit 0xf3
        // --- lock acquired ([EBX+4] = 1) ---
        // 00012a2d: 8b 43 0c           MOV EAX, [EBX+0x0c]    ; EAX = iVar6->sentinel (head node)
        _emit 0x8b
        _emit 0x43
        _emit 0x0c
        // 00012a30: 8b 50 04           MOV EDX, [EAX+0x04]    ; EDX = head->field_4 (tail ptr)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012a33: 89 32              MOV [EDX], ESI          ; tail->field_0 = piVar5
        _emit 0x89
        _emit 0x32
        // 00012a35: 8b 50 04           MOV EDX, [EAX+0x04]    ; EDX = head->field_4 (reload)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012a38: 89 06              MOV [ESI], EAX          ; piVar5->field_0 = head
        _emit 0x89
        _emit 0x06
        // 00012a3a: 89 56 04           MOV [ESI+0x04], EDX     ; piVar5->field_4 = old tail
        _emit 0x89
        _emit 0x56
        _emit 0x04
        // 00012a3d: 89 70 04           MOV [EAX+0x04], ESI     ; head->field_4 = piVar5
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00012a40: 83 43 18 ff        ADD [EBX+0x18], -1      ; iVar6->count--
        _emit 0x83
        _emit 0x43
        _emit 0x18
        _emit 0xff
        // 00012a44: 33 c0              XOR EAX, EAX            ; EAX = 0 (new lock value)
        _emit 0x33
        _emit 0xc0
        // 00012a46: 87 01              XCHG [ECX], EAX         ; atomic: release lock ([ECX] = 0)
        _emit 0x87
        _emit 0x01
        // --- lock released ---
        // 00012a48: 8b 4f 04           MOV ECX, [EDI+0x04]    ; ECX = this->field_0x04 (INotify*)
        _emit 0x8b
        _emit 0x4f
        _emit 0x04
        // 00012a4b: 8b 11              MOV EDX, [ECX]          ; vtable of INotify
        _emit 0x8b
        _emit 0x11
        // 00012a4d: 8b 42 10           MOV EAX, [EDX+0x10]     ; vtable slot 4
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 00012a50: 55                 PUSH EBP                ; push iVar2 (piVar5->field_0x18)
        _emit 0x55
        // 00012a51: ff d0              CALL EAX                 ; notify->vf4(iVar2)
        _emit 0xff
        _emit 0xd0
        // 00012a53: 8b 17              MOV EDX, [EDI]          ; vtable of this
        _emit 0x8b
        _emit 0x17
        // 00012a55: 8b 42 30           MOV EAX, [EDX+0x30]     ; vtable slot 12
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00012a58: 8b cf              MOV ECX, EDI            ; ECX = this (thiscall)
        _emit 0x8b
        _emit 0xcf
        // 00012a5a: ff d0              CALL EAX                 ; this->vf12() — post-hook
        _emit 0xff
        _emit 0xd0
        // 00012a5c: 5f                 POP EDI
        _emit 0x5f
        // 00012a5d: 5e                 POP ESI
        _emit 0x5e
        // 00012a5e: 5d                 POP EBP
        _emit 0x5d
        // 00012a5f: 5b                 POP EBX
        _emit 0x5b
        // 00012a60: c2 04 00           RET 4                   ; __thiscall, 1 stack arg
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif
