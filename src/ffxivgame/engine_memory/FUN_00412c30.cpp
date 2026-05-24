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
// FUNCTION: ffxivgame 0x00012c30 — __thiscall, 1 stack arg (int*), 103 bytes
//
// Acquires spinlock on a pool object, inserts the supplied node
// into the pool's intrusive doubly-linked free-list (pprev-style),
// decrements the pool's free count, then releases the spinlock.
// Surrounds the operation with vtable[0x2c] / vtable[0x30] calls on
// this->field_0x10 (lock/unlock guard pair, slot 11 and slot 12).
//
// Register layout:
//   EBX  = `this` in prologue; repurposed to piVar1 (&iVar4->field_4)
//          inside the spinloop
//   ESI  = param_1  (the node to insert)
//   EDI  = piVar3   = this->field_0x10 (inner object with vtable)
//   EAX  = iVar4    = *(vtable[1]() + 0x14)  (pool descriptor)
//   EDX  = piVar1   = &iVar4->field_4        (spinlock word address)
//
// Asm (103 bytes @ orig RVA 0x00012c30):
//   53                    PUSH EBX
//   56                    PUSH ESI
//   8b d9                 MOV EBX, ECX
//   57                    PUSH EDI
//   8b 7b 10              MOV EDI, [EBX+0x10]
//   8b 07                 MOV EAX, [EDI]
//   8b 50 2c              MOV EDX, [EAX+0x2c]
//   8b cf                 MOV ECX, EDI
//   ff d2                 CALL EDX                    ; piVar3->vtable[11]()
//   8b 74 24 10           MOV ESI, [ESP+0x10]         ; param_1 (stack arg)
//   8b 06                 MOV EAX, [ESI]
//   8b 10                 MOV EDX, [EAX]
//   6a 00                 PUSH 0
//   8b ce                 MOV ECX, ESI
//   ff d2                 CALL EDX                    ; (*param_1)->vtable[0](0)
//   8b 4b 10              MOV ECX, [EBX+0x10]
//   8b 01                 MOV EAX, [ECX]
//   8b 50 04              MOV EDX, [EAX+0x4]
//   ff d2                 CALL EDX                    ; piVar3->vtable[1]() → iVar4 base
//   8b 40 14              MOV EAX, [EAX+0x14]         ; iVar4 = *(ret+0x14)
//   8d 50 04              LEA EDX, [EAX+0x4]          ; piVar1 = &iVar4->field_4
//   90                    NOP
// spinloop:
//   b9 01 00 00 00        MOV ECX, 1
//   8b da                 MOV EBX, EDX
//   87 0b                 XCHG [EBX], ECX             ; atomic swap *piVar1 ↔ 1
//   85 c9                 TEST ECX, ECX               ; was 0?
//   75 f3                 JNZ spinloop
//   8b 48 0c              MOV ECX, [EAX+0x0c]         ; iVar2 = iVar4->tail
//   8b 59 04              MOV EBX, [ECX+0x04]         ; EBX = iVar2->pprev
//   89 33                 MOV [EBX], ESI              ; *(iVar2->pprev) = param_1
//   8b 59 04              MOV EBX, [ECX+0x04]
//   89 5e 04              MOV [ESI+0x04], EBX         ; param_1->pprev = iVar2->pprev
//   89 0e                 MOV [ESI], ECX              ; param_1->next  = iVar2
//   89 71 04              MOV [ECX+0x04], ESI         ; iVar2->pprev   = param_1
//   83 40 18 ff           ADD [EAX+0x18], -1          ; iVar4->count--
//   33 c0                 XOR EAX, EAX
//   87 02                 XCHG [EDX], EAX             ; atomic: *piVar1 = 0 (release)
//   8b 17                 MOV EDX, [EDI]
//   8b 42 30              MOV EAX, [EDX+0x30]
//   8b cf                 MOV ECX, EDI
//   ff d0                 CALL EAX                    ; piVar3->vtable[12]()
//   5f                    POP EDI
//   5e                    POP ESI
//   5b                    POP EBX
//   c2 04 00              RET 4

extern "C" __declspec(naked) void FUN_00412c30()
{
    __asm {
        // 00012c30: 53                PUSH EBX
        _emit 0x53
        // 00012c31: 56                PUSH ESI
        _emit 0x56
        // 00012c32: 8b d9             MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00012c34: 57                PUSH EDI
        _emit 0x57
        // 00012c35: 8b 7b 10          MOV EDI, dword ptr [EBX+0x10]
        _emit 0x8b
        _emit 0x7b
        _emit 0x10
        // 00012c38: 8b 07             MOV EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00012c3a: 8b 50 2c          MOV EDX, dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00012c3d: 8b cf             MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00012c3f: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012c41: 8b 74 24 10       MOV ESI, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00012c45: 8b 06             MOV EAX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00012c47: 8b 10             MOV EDX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 00012c49: 6a 00             PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00012c4b: 8b ce             MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00012c4d: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012c4f: 8b 4b 10          MOV ECX, dword ptr [EBX+0x10]
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00012c52: 8b 01             MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00012c54: 8b 50 04          MOV EDX, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012c57: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012c59: 8b 40 14          MOV EAX, dword ptr [EAX+0x14]
        _emit 0x8b
        _emit 0x40
        _emit 0x14
        // 00012c5c: 8d 50 04          LEA EDX, [EAX+0x4]
        _emit 0x8d
        _emit 0x50
        _emit 0x04
        // 00012c5f: 90                NOP
        _emit 0x90
        // 00012c60: b9 01 00 00 00    MOV ECX, 1
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012c65: 8b da             MOV EBX, EDX
        _emit 0x8b
        _emit 0xda
        // 00012c67: 87 0b             XCHG dword ptr [EBX], ECX
        _emit 0x87
        _emit 0x0b
        // 00012c69: 85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00012c6b: 75 f3             JNZ -0xd (to 0x00012c60)
        _emit 0x75
        _emit 0xf3
        // 00012c6d: 8b 48 0c          MOV ECX, dword ptr [EAX+0x0c]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 00012c70: 8b 59 04          MOV EBX, dword ptr [ECX+0x04]
        _emit 0x8b
        _emit 0x59
        _emit 0x04
        // 00012c73: 89 33             MOV dword ptr [EBX], ESI
        _emit 0x89
        _emit 0x33
        // 00012c75: 8b 59 04          MOV EBX, dword ptr [ECX+0x04]
        _emit 0x8b
        _emit 0x59
        _emit 0x04
        // 00012c78: 89 5e 04          MOV dword ptr [ESI+0x04], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // 00012c7b: 89 0e             MOV dword ptr [ESI], ECX
        _emit 0x89
        _emit 0x0e
        // 00012c7d: 89 71 04          MOV dword ptr [ECX+0x04], ESI
        _emit 0x89
        _emit 0x71
        _emit 0x04
        // 00012c80: 83 40 18 ff       ADD dword ptr [EAX+0x18], -1
        _emit 0x83
        _emit 0x40
        _emit 0x18
        _emit 0xff
        // 00012c84: 33 c0             XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00012c86: 87 02             XCHG dword ptr [EDX], EAX
        _emit 0x87
        _emit 0x02
        // 00012c88: 8b 17             MOV EDX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 00012c8a: 8b 42 30          MOV EAX, dword ptr [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00012c8d: 8b cf             MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00012c8f: ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00012c91: 5f                POP EDI
        _emit 0x5f
        // 00012c92: 5e                POP ESI
        _emit 0x5e
        // 00012c93: 5b                POP EBX
        _emit 0x5b
        // 00012c94: c2 04 00          RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
