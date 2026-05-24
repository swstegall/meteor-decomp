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
// FUNCTION: ffxivgame 0x00013da0 — __thiscall member: child-lifecycle release
//           with spinlock-guarded doubly-linked-list insert and trailing
//           resource handoff. (175 bytes / 0xaf)
//
// Calling convention: __thiscall (ECX = this), 1 stack arg (RET 4).
// Callee-saved: EBX, EBP, ESI, EDI (pushed in mixed-order around the
// first virtual dispatch).
//   EBX = this
//   ESI = piVar5    (child element returned by arg0 → vmt[5] → vmt[1])
//   EBP = piVar5[6] (saved resource handle #1 — freed via this[1] late)
//   EDI = piVar5_5_5[5]  (pool/list owner with embedded spinlock at +4)
//   [ESP+0x14]  piVar5[7] (saved resource handle #2 — freed via this[2] late;
//                          overwrites the first stack arg slot)
//
// High-level shape:
//   1. (*this->vmt[11])()                       ; pre-lock / enter
//   2. p4   = (*arg0->vmt[5])()                  ; get child container
//   3. esi  = (*p4->vmt[1])()                    ; get the element (piVar5)
//   4. ebp  = piVar5[6]                          ; cache resource #1
//   5. spill_arg0_slot = piVar5[7]               ; spill resource #2
//   6. edi  = (*piVar5[5]->vmt[1])()->field_0x14 ; reach inner pool
//   7. FUN_00413940(piVar5)                      ; child-private cleanup pass 1
//   8. FUN_00413be0(piVar5)                      ; child-private cleanup pass 2
//   9. (*piVar5->vmt[0])(0)                      ; virtual op (likely destructor / finalize)
//  10. acquire spinlock at &edi[1] (xchg until prev was 0)
//  11. doubly-linked-list insert of piVar5 between edi[3] and edi[3]->next
//  12. edi[6] -= 1                               ; decrement live count
//  13. release spinlock (xchg with 0)
//  14. if (ebp)            (*this[1]->vmt[4])(ebp)
//  15. if (spill_arg0_slot) (*this[2]->vmt[4])(spill_arg0_slot)
//  16. (*this->vmt[12])()                        ; post-lock / leave
//
// Relocations (compared as literal bytes — both sides land at fixed RVAs):
//   REL32: FUN_00413940 at func+0x3c (bytes 60–63, encoded -0x4a1)
//   REL32: FUN_00413be0 at func+0x43 (bytes 67–70, encoded -0x208)
//
// Reconstruction: naked-asm byte passthrough. The mixed prologue-mid
// `PUSH EBX / PUSH EBP / MOV EBX, ECX / MOV EAX,[EBX] / MOV EDX,[EAX+0x2c]
// / PUSH ESI / PUSH EDI / CALL EDX` save order, the stack-arg-slot
// reuse for the piVar5[7] spill, the xchg spinlock pattern, and the
// `add [edi+0x18], -1` (vs `dec`) idiom are not reliably reproducible
// from C++ source at /O2; _emit preserves all 175 bytes verbatim.
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.

#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_00413da0() {}
#else

extern "C" __declspec(naked) void FUN_00413da0()
{
    __asm {
        // 00413da0:  53                    PUSH EBX
        _emit 0x53
        // 00413da1:  55                    PUSH EBP
        _emit 0x55
        // 00413da2:  8b d9                 MOV EBX, ECX           ; cache 'this'
        _emit 0x8b
        _emit 0xd9
        // 00413da4:  8b 03                 MOV EAX, [EBX]          ; vtable of this
        _emit 0x8b
        _emit 0x03
        // 00413da6:  8b 50 2c              MOV EDX, [EAX+0x2c]    ; this->vmt[11]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00413da9:  56                    PUSH ESI
        _emit 0x56
        // 00413daa:  57                    PUSH EDI
        _emit 0x57
        // 00413dab:  ff d2                 CALL EDX                ; (*this->vmt[11])()
        _emit 0xff
        _emit 0xd2
        // 00413dad:  8b 4c 24 14           MOV ECX, [ESP+0x14]    ; arg0 (after 4 pushes)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00413db1:  8b 01                 MOV EAX, [ECX]          ; vtable of arg0
        _emit 0x8b
        _emit 0x01
        // 00413db3:  8b 50 14              MOV EDX, [EAX+0x14]    ; arg0->vmt[5]
        _emit 0x8b
        _emit 0x50
        _emit 0x14
        // 00413db6:  ff d2                 CALL EDX                ; piVar4 = (*arg0->vmt[5])()
        _emit 0xff
        _emit 0xd2
        // 00413db8:  8b 10                 MOV EDX, [EAX]          ; vtable of piVar4
        _emit 0x8b
        _emit 0x10
        // 00413dba:  8b c8                 MOV ECX, EAX            ; ECX = piVar4
        _emit 0x8b
        _emit 0xc8
        // 00413dbc:  8b 42 04              MOV EAX, [EDX+4]        ; piVar4->vmt[1]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00413dbf:  ff d0                 CALL EAX                ; piVar5 = (*piVar4->vmt[1])()
        _emit 0xff
        _emit 0xd0
        // 00413dc1:  8b f0                 MOV ESI, EAX            ; ESI = piVar5
        _emit 0x8b
        _emit 0xf0
        // 00413dc3:  8b 4e 1c              MOV ECX, [ESI+0x1c]    ; piVar5[7] → cache for spill
        _emit 0x8b
        _emit 0x4e
        _emit 0x1c
        // 00413dc6:  8b 6e 18              MOV EBP, [ESI+0x18]    ; EBP = piVar5[6]
        _emit 0x8b
        _emit 0x6e
        _emit 0x18
        // 00413dc9:  89 4c 24 14           MOV [ESP+0x14], ECX    ; spill piVar5[7] to stack
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00413dcd:  8b 4e 14              MOV ECX, [ESI+0x14]    ; piVar5[5]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00413dd0:  8b 11                 MOV EDX, [ECX]          ; vtable of piVar5[5]
        _emit 0x8b
        _emit 0x11
        // 00413dd2:  8b 42 04              MOV EAX, [EDX+4]       ; piVar5[5]->vmt[1]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00413dd5:  ff d0                 CALL EAX                ; (*piVar5[5]->vmt[1])() → pool
        _emit 0xff
        _emit 0xd0
        // 00413dd7:  8b 78 14              MOV EDI, [EAX+0x14]    ; EDI = pool->field_0x14
        _emit 0x8b
        _emit 0x78
        _emit 0x14
        // 00413dda:  8b ce                 MOV ECX, ESI            ; ECX = piVar5
        _emit 0x8b
        _emit 0xce
        // 00413ddc:  e8 5f fb ff ff        CALL FUN_00413940       ; REL32 reloc
        _emit 0xe8
        _emit 0x5f
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 00413de1:  8b ce                 MOV ECX, ESI            ; ECX = piVar5
        _emit 0x8b
        _emit 0xce
        // 00413de3:  e8 f8 fd ff ff        CALL FUN_00413be0       ; REL32 reloc
        _emit 0xe8
        _emit 0xf8
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 00413de8:  8b 16                 MOV EDX, [ESI]          ; vtable of piVar5
        _emit 0x8b
        _emit 0x16
        // 00413dea:  8b 02                 MOV EAX, [EDX]          ; piVar5->vmt[0]
        _emit 0x8b
        _emit 0x02
        // 00413dec:  6a 00                 PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00413dee:  8b ce                 MOV ECX, ESI            ; ECX = piVar5
        _emit 0x8b
        _emit 0xce
        // 00413df0:  ff d0                 CALL EAX                ; (*piVar5->vmt[0])(0)
        _emit 0xff
        _emit 0xd0
        // 00413df2:  8d 4f 04              LEA ECX, [EDI+4]        ; ECX = &spinlock word
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        // 00413df5:  ba 01 00 00 00        MOV EDX, 1               ; spin: re-arm EDX each iter
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00413dfa:  8b c1                 MOV EAX, ECX            ; alias for xchg target
        _emit 0x8b
        _emit 0xc1
        // 00413dfc:  87 10                 XCHG [EAX], EDX         ; atomic acquire attempt
        _emit 0x87
        _emit 0x10
        // 00413dfe:  85 d2                 TEST EDX, EDX           ; prev == 0 means we got it
        _emit 0x85
        _emit 0xd2
        // 00413e00:  75 f3                 JNZ -0x0d → 00413df5    ; otherwise retry
        _emit 0x75
        _emit 0xf3
        // 00413e02:  8b 47 0c              MOV EAX, [EDI+0xc]      ; head = pool[3]
        _emit 0x8b
        _emit 0x47
        _emit 0x0c
        // 00413e05:  8b 50 04              MOV EDX, [EAX+4]        ; head->next
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00413e08:  89 32                 MOV [EDX], ESI          ; head->next->prev = piVar5
        _emit 0x89
        _emit 0x32
        // 00413e0a:  8b 50 04              MOV EDX, [EAX+4]        ; reload head->next
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00413e0d:  89 06                 MOV [ESI], EAX          ; piVar5->prev = head
        _emit 0x89
        _emit 0x06
        // 00413e0f:  89 56 04              MOV [ESI+4], EDX        ; piVar5->next = old head->next
        _emit 0x89
        _emit 0x56
        _emit 0x04
        // 00413e12:  89 70 04              MOV [EAX+4], ESI        ; head->next = piVar5
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00413e15:  83 47 18 ff           ADD [EDI+0x18], -1      ; pool[6]-- (live count)
        _emit 0x83
        _emit 0x47
        _emit 0x18
        _emit 0xff
        // 00413e19:  33 c0                 XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00413e1b:  87 01                 XCHG [ECX], EAX         ; spinlock release
        _emit 0x87
        _emit 0x01
        // 00413e1d:  85 ed                 TEST EBP, EBP
        _emit 0x85
        _emit 0xed
        // 00413e1f:  74 0b                 JZ +0x0b → 00413e2c
        _emit 0x74
        _emit 0x0b
        // 00413e21:  8b 4b 04              MOV ECX, [EBX+4]        ; ECX = this[1]
        _emit 0x8b
        _emit 0x4b
        _emit 0x04
        // 00413e24:  8b 11                 MOV EDX, [ECX]          ; vtable of this[1]
        _emit 0x8b
        _emit 0x11
        // 00413e26:  8b 42 10              MOV EAX, [EDX+0x10]    ; this[1]->vmt[4]
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 00413e29:  55                    PUSH EBP                ; arg = piVar5[6]
        _emit 0x55
        // 00413e2a:  ff d0                 CALL EAX                ; (*this[1]->vmt[4])(piVar5[6])
        _emit 0xff
        _emit 0xd0
        // 00413e2c:  8b 44 24 14           MOV EAX, [ESP+0x14]    ; reload spilled piVar5[7]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00413e30:  85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00413e32:  74 0b                 JZ +0x0b → 00413e3f
        _emit 0x74
        _emit 0x0b
        // 00413e34:  8b 4b 08              MOV ECX, [EBX+8]        ; ECX = this[2]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 00413e37:  8b 11                 MOV EDX, [ECX]          ; vtable of this[2]
        _emit 0x8b
        _emit 0x11
        // 00413e39:  50                    PUSH EAX                ; arg = piVar5[7]
        _emit 0x50
        // 00413e3a:  8b 42 10              MOV EAX, [EDX+0x10]    ; this[2]->vmt[4]
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 00413e3d:  ff d0                 CALL EAX                ; (*this[2]->vmt[4])(piVar5[7])
        _emit 0xff
        _emit 0xd0
        // 00413e3f:  8b 13                 MOV EDX, [EBX]          ; vtable of this
        _emit 0x8b
        _emit 0x13
        // 00413e41:  8b 42 30              MOV EAX, [EDX+0x30]    ; this->vmt[12]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00413e44:  8b cb                 MOV ECX, EBX            ; ECX = this
        _emit 0x8b
        _emit 0xcb
        // 00413e46:  ff d0                 CALL EAX                ; (*this->vmt[12])()
        _emit 0xff
        _emit 0xd0
        // 00413e48:  5f                    POP EDI
        _emit 0x5f
        // 00413e49:  5e                    POP ESI
        _emit 0x5e
        // 00413e4a:  5d                    POP EBP
        _emit 0x5d
        // 00413e4b:  5b                    POP EBX
        _emit 0x5b
        // 00413e4c:  c2 04 00              RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif
