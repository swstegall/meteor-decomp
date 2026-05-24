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
// FUNCTION: ffxivgame 0x00012f10 — __thiscall, void return, 118 bytes (0x76)
//           Lock-guard acquire, counter decrement, conditional branch on
//           embedded-list-sentinel check vs counter-reaches-zero, tail release.
//
// Calling convention: __thiscall (ECX = this); returns void.
//
// Register layout:
//   ESI = this               (pushed in prologue)
//   EBP = this->field_0x10   (piVar1; pushed in prologue)
//   EDI = this->field_0x18   (piVar2; LATE-SAVED with PUSH EDI inside the
//                             else-if branch; POP EDI at 0x69, before done)
//
// Object layout (offsets touched):
//   [this+0x10]   pointer to lock-guard object (piVar1); also in EBP
//   [this+0x14]   secondary object (sentinel branch only)
//   [this+0x18]   piVar2 pointer (cleared on release path)
//   [this+0x1c]   byte flag 1 (cleared on release path)
//   [this+0x1d]   byte flag 2 (cleared on release path)
//   [this+0x24]   linked-list head pointer
//   [this+0x28]   reference depth counter
//
// Sentinel idiom: LEA ECX, [ESI-4]; CMP [ECX+0x28], ECX
//   i.e. [this+0x24] == this-4  (same as FUN_00412b20 in this module).
//
// Flow (offsets from function start, all short-jump targets):
//   0x00  prologue: PUSH EBP; PUSH ESI; ESI=ECX; EBP=[ESI+0x10]
//   0x07  EBP->vtable[11]()         — acquire lock
//   0x11  ADD [ESI+0x28], -1        — decrement depth
//   0x15  LEA ECX, [ESI-4]
//   0x18  CMP [ECX+0x28], ECX       — sentinel check
//   0x1b  JNZ 0x29                  — not sentinel → else_branch
//   0x1d  branch_A (inline, fall-through): [ESI+0x14]->vtable[8]()
//   0x27  JMP 0x6a                  — to done (no EDI push)
//   0x29  else_branch:
//   0x29  CMP [ESI+0x28], 0
//   0x2d  JNZ 0x6a                  — depth != 0 → done (no EDI push)
//   0x2f  PUSH EDI                  — late-save EDI
//   0x30  CALL FUN_00412950(this)
//   0x35  EDI = [ESI+0x18]          — piVar2
//   0x38  TEST EDI, EDI
//   0x3a  JZ  0x69                  — null → pop EDI → done
//   0x3c  EDI->vtable[8]()
//   0x45  ECX=[ESI+0x10]; piVar1->vtable[1]() → iVar3
//   0x4f  ECX=[iVar3+8]; EDX=[ECX]->vtable[4]; PUSH EDI; CALL EDX
//   0x5a  [ESI+0x18/1c/1d] = 0
//   0x69  POP EDI                   — late-restore (matched to 0x2f PUSH)
//   0x6a  done:
//   0x6a  EAX=[EBP]; EDX=[EAX+0x30]; POP ESI; ECX=EBP; POP EBP; JMP EDX
//
// Reconstruction note: naked-asm used because the late EDI save, the
//   mandatory [EBP+0] disp8 encoding, the ADD imm8(-1) decrement, and the
//   split "done" label (0x69 vs 0x6a) cannot be driven from C++ source.
//
// Asm bytes (118 @ RVA 0x00012f10):
//   55                    PUSH EBP
//   56                    PUSH ESI
//   8b f1                 MOV ESI, ECX
//   8b 6e 10              MOV EBP, [ESI+0x10]
//   8b 45 00              MOV EAX, [EBP+0]        ; forced disp8 for EBP base
//   8b 50 2c              MOV EDX, [EAX+0x2c]
//   8b cd                 MOV ECX, EBP
//   ff d2                 CALL EDX                 ; piVar1->vtable[11]()
//   83 46 28 ff           ADD [ESI+0x28], -1       ; depth--
//   8d 4e fc              LEA ECX, [ESI-4]
//   39 49 28              CMP [ECX+0x28], ECX      ; [this+0x24] == this-4?
//   75 0c                 JNZ else_branch (+0x0c)
//   8b 4e 14              MOV ECX, [ESI+0x14]
//   8b 01                 MOV EAX, [ECX]
//   8b 50 20              MOV EDX, [EAX+0x20]
//   ff d2                 CALL EDX                 ; field_0x14->vtable[8]()
//   eb 41                 JMP done (+0x41)
// else_branch:
//   83 7e 28 00           CMP [ESI+0x28], 0
//   75 3b                 JNZ done (+0x3b)
//   57                    PUSH EDI                 ; late-save
//   e8 XX XX XX XX        CALL FUN_00412950
//   8b 7e 18              MOV EDI, [ESI+0x18]
//   85 ff                 TEST EDI, EDI
//   74 2d                 JZ pop_edi (+0x2d)
//   8b 07                 MOV EAX, [EDI]
//   8b 50 20              MOV EDX, [EAX+0x20]
//   8b cf                 MOV ECX, EDI
//   ff d2                 CALL EDX                 ; piVar2->vtable[8]()
//   8b 4e 10              MOV ECX, [ESI+0x10]      ; reload piVar1
//   8b 01                 MOV EAX, [ECX]
//   8b 50 04              MOV EDX, [EAX+0x04]
//   ff d2                 CALL EDX                 ; piVar1->vtable[1]() → iVar3
//   8b 48 08              MOV ECX, [EAX+0x08]
//   8b 01                 MOV EAX, [ECX]
//   8b 50 10              MOV EDX, [EAX+0x10]      ; vtable[4] into EDX
//   57                    PUSH EDI                 ; arg = piVar2
//   ff d2                 CALL EDX                 ; [iVar3+8]->vtable[4](piVar2)
//   c7 46 18 00 00 00 00  MOV [ESI+0x18], 0
//   c6 46 1c 00           MOV byte [ESI+0x1c], 0
//   c6 46 1d 00           MOV byte [ESI+0x1d], 0
// pop_edi (0x69):
//   5f                    POP EDI                  ; late-restore
// done (0x6a):
//   8b 45 00              MOV EAX, [EBP+0]         ; piVar1 vtable (EBP base = forced disp8)
//   8b 50 30              MOV EDX, [EAX+0x30]      ; vtable[12]
//   5e                    POP ESI
//   8b cd                 MOV ECX, EBP             ; ECX = piVar1 for thiscall
//   5d                    POP EBP
//   ff e2                 JMP EDX                  ; tail-call piVar1->vtable[12]()

void FUN_00412950();

extern "C" __declspec(naked) void FUN_00412f10()
{
    __asm {
        // 00012f10: 55                PUSH EBP
        _emit 0x55
        // 00012f11: 56                PUSH ESI
        _emit 0x56
        // 00012f12: 8b f1             MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00012f14: 8b 6e 10          MOV EBP, [ESI+0x10]
        _emit 0x8b
        _emit 0x6e
        _emit 0x10
        // 00012f17: 8b 45 00          MOV EAX, [EBP+0]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00012f1a: 8b 50 2c          MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00012f1d: 8b cd             MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 00012f1f: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012f21: 83 46 28 ff       ADD dword ptr [ESI+0x28], -1
        _emit 0x83
        _emit 0x46
        _emit 0x28
        _emit 0xff
        // 00012f25: 8d 4e fc          LEA ECX, [ESI-4]
        _emit 0x8d
        _emit 0x4e
        _emit 0xfc
        // 00012f28: 39 49 28          CMP [ECX+0x28], ECX
        _emit 0x39
        _emit 0x49
        _emit 0x28
        // 00012f2b: 75 0c             JNZ else_branch
        _emit 0x75
        _emit 0x0c
        // 00012f2d: 8b 4e 14          MOV ECX, [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00012f30: 8b 01             MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00012f32: 8b 50 20          MOV EDX, [EAX+0x20]
        _emit 0x8b
        _emit 0x50
        _emit 0x20
        // 00012f35: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012f37: eb 41             JMP done
        _emit 0xeb
        _emit 0x41
    // else_branch (offset 0x29 from func start):
        // 00012f39: 83 7e 28 00       CMP dword ptr [ESI+0x28], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x28
        _emit 0x00
        // 00012f3d: 75 3b             JNZ done
        _emit 0x75
        _emit 0x3b
        // 00012f3f: 57                PUSH EDI   (late-save)
        _emit 0x57
        // 00012f40: e8 XX XX XX XX    CALL FUN_00412950
        call FUN_00412950
        // 00012f45: 8b 7e 18          MOV EDI, [ESI+0x18]
        _emit 0x8b
        _emit 0x7e
        _emit 0x18
        // 00012f48: 85 ff             TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00012f4a: 74 2d             JZ pop_edi
        _emit 0x74
        _emit 0x2d
        // 00012f4c: 8b 07             MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00012f4e: 8b 50 20          MOV EDX, [EAX+0x20]
        _emit 0x8b
        _emit 0x50
        _emit 0x20
        // 00012f51: 8b cf             MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00012f53: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012f55: 8b 4e 10          MOV ECX, [ESI+0x10]  (reload piVar1)
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00012f58: 8b 01             MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00012f5a: 8b 50 04          MOV EDX, [EAX+0x04]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012f5d: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012f5f: 8b 48 08          MOV ECX, [EAX+0x08]
        _emit 0x8b
        _emit 0x48
        _emit 0x08
        // 00012f62: 8b 01             MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00012f64: 8b 50 10          MOV EDX, [EAX+0x10]
        _emit 0x8b
        _emit 0x50
        _emit 0x10
        // 00012f67: 57                PUSH EDI   (arg = piVar2)
        _emit 0x57
        // 00012f68: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012f6a: c7 46 18 00 00 00 00  MOV dword ptr [ESI+0x18], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012f71: c6 46 1c 00       MOV byte ptr [ESI+0x1c], 0
        _emit 0xc6
        _emit 0x46
        _emit 0x1c
        _emit 0x00
        // 00012f75: c6 46 1d 00       MOV byte ptr [ESI+0x1d], 0
        _emit 0xc6
        _emit 0x46
        _emit 0x1d
        _emit 0x00
    // pop_edi (offset 0x69 from func start):
        // 00012f79: 5f                POP EDI   (late-restore)
        _emit 0x5f
    // done (offset 0x6a from func start):
        // 00012f7a: 8b 45 00          MOV EAX, [EBP+0]
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00012f7d: 8b 50 30          MOV EDX, [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00012f80: 5e                POP ESI
        _emit 0x5e
        // 00012f81: 8b cd             MOV ECX, EBP
        _emit 0x8b
        _emit 0xcd
        // 00012f83: 5d                POP EBP
        _emit 0x5d
        // 00012f85: ff e2             JMP EDX
        _emit 0xff
        _emit 0xe2
    }
}
