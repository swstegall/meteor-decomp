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
// FUNCTION: ffxivgame 0x00012fb0 — ReceivableHeapBlock::Receive (170 bytes / 0xaa)
//           __thiscall, 4 stack args
//
// ECX = this (ReceivableHeapBlock*)
// this->field_0x10 = piVar1 (pointer to heap Space with vtable)
// this->field_0x20 = base offset (added to param_1)
// this->field_0x24 = pointer to sub-object (vtable at [+0x18], sentinel at [+0x44])
//
// Behaviour:
//   piVar1 = this->field_0x10
//   piVar1->vtable[0x2c](piVar1)                        ; BeginLock()
//   uVar3 = this->field_24->field_18->vtable[0x24](
//                   this->field_20+p1, p2, p3, p4)      ; receive-into call
//   block  = this->field_10->vtable[0x4]()              ; GetBlock()
//   node   = FUN_004109a0(block->field_0x10)            ; allocate element
//   if (node) {
//     node = FUN_00412a80(node, piVar1, uVar3, field_20+p1, field_24)
//     if (node) { iVar5 = node + 8; goto list_insert; }
//   }
//   node = 0; iVar5 = 0;
// list_insert:
//   append iVar5 before sentinel = this->field_24->field_44
//   piVar1->vtable[0x30](piVar1)                        ; EndLock()
//   return node ? node + 4 : 0
//
// Reconstruction: naked-asm byte passthrough.
//
// Register layout:
//   ESI = this (cached immediately)
//   EDI = this->field_10 (piVar1, cached at entry)
//   EBP = uVar3 (result of vtable[0x24] call, saved across GetBlock)
//   EBX = node (result of FUN_00412a80; also used transiently for params)
//   ECX = node+8 (list element) or 0 (null path)
//
// Relocations (masked by tools/compare.py):
//   REL32: CALL FUN_004109a0 (@ rva 0x00012ff7)
//   REL32: CALL FUN_00412a80 (@ rva 0x0001300e)

extern "C" void FUN_004109a0();
extern "C" void FUN_00412a80();

extern "C" __declspec(naked) void FUN_00412fb0()
{
    __asm {
        // Prologue — save EBX EBP ESI EDI; cache this in ESI; load piVar1 into EDI
        // 00012fb0:  53               PUSH EBX
        _emit 0x53
        // 00012fb1:  55               PUSH EBP
        _emit 0x55
        // 00012fb2:  56               PUSH ESI
        _emit 0x56
        // 00012fb3:  8b f1            MOV ESI, ECX          ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00012fb5:  57               PUSH EDI
        _emit 0x57
        // 00012fb6:  8b 7e 10         MOV EDI, [ESI+0x10]   ; EDI = this->field_10 = piVar1
        _emit 0x8b
        _emit 0x7e
        _emit 0x10

        // BeginLock: piVar1->vtable[0x2c](piVar1)
        // 00012fb9:  8b 07            MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00012fbb:  8b 50 2c         MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00012fbe:  8b cf            MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00012fc0:  ff d2            CALL EDX
        _emit 0xff
        _emit 0xd2

        // Load the 4 stack args into temporaries; push them in reverse
        // order for the incoming virtual call.
        // EBP (still holding old saved value at this point) is clobbered
        // after the call. Stack layout after 4 callee-saves:
        //   [ESP+0x14]=p1 [ESP+0x18]=p2 [ESP+0x1c]=p3 [ESP+0x20]=p4
        // 00012fc2:  8b 5c 24 20      MOV EBX, [ESP+0x20]   ; EBX = param_4
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00012fc6:  8b 46 24         MOV EAX, [ESI+0x24]   ; AX = this->field_24
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 00012fc9:  8b 48 18         MOV ECX, [EAX+0x18]   ; ECX = field_24->field_18
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        // 00012fcc:  8b 46 20         MOV EAX, [ESI+0x20]   ; AX = this->field_20
        _emit 0x8b
        _emit 0x46
        _emit 0x20
        // 00012fcf:  8b 11            MOV EDX, [ECX]        ; EDX = vtable
        _emit 0x8b
        _emit 0x11

        // Push param_4, param_3, param_2 one-by-one (reading from
        // progressively lower addresses after each PUSH shifts ESP).
        // 00012fd1:  53               PUSH EBX               ; push param_4
        _emit 0x53
        // 00012fd2:  8b 5c 24 20      MOV EBX, [ESP+0x20]   ; now param_3
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00012fd6:  53               PUSH EBX               ; push param_3
        _emit 0x53
        // 00012fd7:  8b 5c 24 20      MOV EBX, [ESP+0x20]   ; now param_2
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00012fdb:  53               PUSH EBX               ; push param_2
        _emit 0x53
        // 00012fdc:  8b 5c 24 20      MOV EBX, [ESP+0x20]   ; now param_1 (stays in EBX)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00012fe0:  03 c3            ADD EAX, EBX           ; EAX = field_20 + param_1
        _emit 0x03
        _emit 0xc3
        // 00012fe2:  50               PUSH EAX               ; push (field_20 + param_1) as 1st arg
        _emit 0x50

        // Dispatch: ECX = field_24->field_18; EDX[0x24] = method
        // 00012fe3:  8b 42 24         MOV EAX, [EDX+0x24]
        _emit 0x8b
        _emit 0x42
        _emit 0x24
        // 00012fe6:  ff d0            CALL EAX
        _emit 0xff
        _emit 0xd0

        // GetBlock: reload this->field_10 and dispatch vtable[0x4]
        // EBP = uVar3 (call result)
        // 00012fe8:  8b 4e 10         MOV ECX, [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00012feb:  8b 11            MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 00012fed:  8b e8            MOV EBP, EAX           ; EBP = uVar3
        _emit 0x8b
        _emit 0xe8
        // 00012fef:  8b 42 04         MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00012ff2:  ff d0            CALL EAX               ; GetBlock() → EAX = block
        _emit 0xff
        _emit 0xd0

        // Allocate element: ECX = block->field_0x10
        // 00012ff4:  8b 48 10         MOV ECX, [EAX+0x10]
        _emit 0x8b
        _emit 0x48
        _emit 0x10
        // 00012ff7:  e8 a4 d9 ff ff   CALL FUN_004109a0      ; REL32 reloc
        _emit 0xe8
        _emit 0xa4
        _emit 0xd9
        _emit 0xff
        _emit 0xff

        // Test allocation; jump to null path on failure
        // 00012ffc:  85 c0            TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00012ffe:  74 1e            JZ +0x1e (→ 0x0001301e)
        _emit 0x74
        _emit 0x1e

        // Non-null path: call FUN_00412a80(node=ECX, piVar1, uVar3, field_20+p1, field_24)
        // Push args right-to-left (this->field_24 goes last as 4th stack arg)
        // 00013000:  8b 4e 24         MOV ECX, [ESI+0x24]    ; field_24
        _emit 0x8b
        _emit 0x4e
        _emit 0x24
        // 00013003:  8b 56 20         MOV EDX, [ESI+0x20]    ; field_20
        _emit 0x8b
        _emit 0x56
        _emit 0x20
        // 00013006:  51               PUSH ECX               ; push field_24 (arg 4)
        _emit 0x51
        // 00013007:  03 d3            ADD EDX, EBX           ; EDX = field_20 + param_1
        _emit 0x03
        _emit 0xd3
        // 00013009:  52               PUSH EDX               ; push field_20+p1 (arg 3)
        _emit 0x52
        // 0001300a:  55               PUSH EBP               ; push uVar3 (arg 2)
        _emit 0x55
        // 0001300b:  57               PUSH EDI               ; push piVar1 (arg 1)
        _emit 0x57
        // 0001300c:  8b c8            MOV ECX, EAX           ; ECX = node (allocated element)
        _emit 0x8b
        _emit 0xc8
        // 0001300e:  e8 6d fa ff ff   CALL FUN_00412a80      ; REL32 reloc
        _emit 0xe8
        _emit 0x6d
        _emit 0xfa
        _emit 0xff
        _emit 0xff

        // Test Init result; EBX = node pointer (or 0 if Init failed)
        // 00013013:  8b d8            MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 00013015:  85 db            TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 00013017:  74 07            JZ +0x7 (→ 0x0001301e)
        _emit 0x74
        _emit 0x07

        // Non-null Init result: ECX = node+8 (list element), jump over null path
        // 00013019:  8d 4b 08         LEA ECX, [EBX+0x8]
        _emit 0x8d
        _emit 0x4b
        _emit 0x08
        // 0001301c:  eb 04            JMP +0x4 (→ 0x00013022)
        _emit 0xeb
        _emit 0x04

        // Null path: EBX = 0, ECX = 0
        // 0001301e:  33 db            XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 00013020:  33 c9            XOR ECX, ECX
        _emit 0x33
        _emit 0xc9

        // LAB_00013022 — doubly-linked list insertion before sentinel
        // sentinel = this->field_24->field_44
        // Insert iVar5 (ECX) before sentinel (tail append)
        // 00013022:  8b 56 24         MOV EDX, [ESI+0x24]
        _emit 0x8b
        _emit 0x56
        _emit 0x24
        // 00013025:  8b 42 44         MOV EAX, [EDX+0x44]   ; EAX = sentinel head
        _emit 0x8b
        _emit 0x42
        _emit 0x44
        // 00013028:  8b 50 08         MOV EDX, [EAX+0x8]    ; EDX = sentinel->bwd
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0001302b:  89 4a 04         MOV [EDX+0x4], ECX    ; prev->fwd = node
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 0001302e:  8b 50 08         MOV EDX, [EAX+0x8]    ; reload sentinel->bwd
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00013031:  89 51 08         MOV [ECX+0x8], EDX    ; node->bwd = prev
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00013034:  89 41 04         MOV [ECX+0x4], EAX    ; node->fwd = sentinel
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 00013037:  89 48 08         MOV [EAX+0x8], ECX    ; sentinel->bwd = node
        _emit 0x89
        _emit 0x48
        _emit 0x08

        // EndLock: piVar1->vtable[0x30](piVar1)
        // 0001303a:  8b 07            MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 0001303c:  8b 50 30         MOV EDX, [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 0001303f:  8b cf            MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00013041:  ff d2            CALL EDX               ; EndLock()
        _emit 0xff
        _emit 0xd2

        // Return: EBX != 0 → return EBX+4; else return 0
        // 00013043:  85 db            TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 00013045:  74 0a            JZ +0xa (→ 0x00013051)
        _emit 0x74
        _emit 0x0a

        // Non-null epilogue: POP in LIFO order, LEA EAX = EBX+4
        // 00013047:  5f               POP EDI
        _emit 0x5f
        // 00013048:  5e               POP ESI
        _emit 0x5e
        // 00013049:  5d               POP EBP
        _emit 0x5d
        // 0001304a:  8d 43 04         LEA EAX, [EBX+0x4]
        _emit 0x8d
        _emit 0x43
        _emit 0x04
        // 0001304d:  5b               POP EBX
        _emit 0x5b
        // 0001304e:  c2 10 00         RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00

        // Null epilogue: POP in LIFO order, XOR EAX=0
        // 00013051:  5f               POP EDI
        _emit 0x5f
        // 00013052:  5e               POP ESI
        _emit 0x5e
        // 00013053:  5d               POP EBP
        _emit 0x5d
        // 00013054:  33 c0            XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00013056:  5b               POP EBX
        _emit 0x5b
        // 00013057:  c2 10 00         RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
