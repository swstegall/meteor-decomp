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
// FUNCTION: ffxivgame 0x00012b80 — __thiscall, 1 stack arg (int*), 174 bytes
//
// Acquires lock guard (vtable[0x2c] on this->field_0x10), advances a
// multi-level vtable chain from param_1 to get a node (piVar4 via
// vtable[0x14] then vtable[0x4]), drains a linked list on piVar4 by
// calling piVar4[1]->vtable[0x20] while piVar4[0xb] != 0, calls
// piVar4->vtable[0](0), acquires a spinlock on a pool descriptor,
// inserts piVar4 into the pool's intrusive free-list, decrements the
// pool's count, releases the spinlock, calls vtable[0x28] on a
// sub-object of this->field_0x24 with a saved pointer, then
// releases the lock guard (vtable[0x30] on this->field_0x10).
//
// Register layout:
//   EBX  = `this` throughout
//   EDI  = this->field_0x10  (lock guard object)
//   ESI  = piVar4            (the node inserted)
//   EBP  = scratch (spinlock address / pprev pointer)
//   ECX  = pool descriptor field_0x10 (held across spinloop for
//          iVar5 / piVar1 / iVar2 sub-operations)
//   EDX  = piVar1 = &ECX[1]  (spinlock word address)
//
// Asm (174 bytes @ orig RVA 0x00012b80):
//   53                    PUSH EBX
//   55                    PUSH EBP
//   56                    PUSH ESI
//   8b d9                 MOV EBX, ECX                  ; cache this
//   57                    PUSH EDI
//   8b 7b 10              MOV EDI, [EBX+0x10]           ; EDI = this->field_0x10
//   8b 07                 MOV EAX, [EDI]                ; vtable
//   8b 50 2c              MOV EDX, [EAX+0x2c]
//   8b cf                 MOV ECX, EDI
//   ff d2                 CALL EDX                      ; field_0x10->vtable[0x2c]()
//   8b 4c 24 14           MOV ECX, [ESP+0x14]           ; param_1
//   8b 01                 MOV EAX, [ECX]                ; vtable
//   8b 50 14              MOV EDX, [EAX+0x14]
//   ff d2                 CALL EDX                      ; (*param_1)->vtable[0x14]()
//   8b 10                 MOV EDX, [EAX]                ; vtable of result
//   8b c8                 MOV ECX, EAX
//   8b 42 04              MOV EAX, [EDX+0x4]
//   ff d0                 CALL EAX                      ; ->vtable[0x4]() -> piVar4
//   8b f0                 MOV ESI, EAX                  ; ESI = piVar4
//   83 7e 2c 00           CMP dword ptr [ESI+0x2c], 0
//   74 13                 JZ  skip_loop
//   8d 6e 04              LEA EBP, [ESI+4]              ; EBP = &piVar4[1]
//   8b 55 00              MOV EDX, [EBP]                ; EDX = piVar4[1]
//   8b 42 20              MOV EAX, [EDX+0x20]
//   8b cd                 MOV ECX, EDX
//   ff d0                 CALL EAX                      ; piVar4[1]->vtable[0x20]()
//   83 7e 2c 00           CMP dword ptr [ESI+0x2c], 0
//   75 f0                 JNZ loop_body
// skip_loop:
//   8b 4e 18              MOV ECX, [ESI+0x18]           ; save piVar4[6]
//   8b 16                 MOV EDX, [ESI]                ; vtable of piVar4
//   8b 02                 MOV EAX, [EDX]                ; vtable[0]
//   89 4c 24 14           MOV [ESP+0x14], ECX           ; stash piVar4[6] on stack
//   6a 00                 PUSH 0
//   8b ce                 MOV ECX, ESI
//   ff d0                 CALL EAX                      ; piVar4->vtable[0](0)
//   8b 4b 10              MOV ECX, [EBX+0x10]           ; this->field_0x10
//   8b 11                 MOV EDX, [ECX]                ; vtable
//   8b 42 04              MOV EAX, [EDX+0x4]
//   ff d0                 CALL EAX                      ; field_0x10->vtable[0x4]() -> iVar5 base
//   8b 48 10              MOV ECX, [EAX+0x10]           ; iVar5 = *(ret+0x10)
//   8d 51 04              LEA EDX, [ECX+4]              ; piVar1 = &iVar5[1]
// spinloop:
//   b8 01 00 00 00        MOV EAX, 1
//   8b ea                 MOV EBP, EDX                  ; EBP = piVar1
//   87 45 00              XCHG [EBP], EAX               ; atomic swap *piVar1 <-> 1
//   85 c0                 TEST EAX, EAX
//   75 f2                 JNZ spinloop
//   8b 41 0c              MOV EAX, [ECX+0x0c]           ; iVar2 = iVar5->field_0xc
//   8b 68 04              MOV EBP, [EAX+0x4]            ; EBP = *(iVar2+4)
//   89 75 00              MOV [EBP], ESI                ; *(*(iVar2+4)) = piVar4
//   8b 68 04              MOV EBP, [EAX+0x4]
//   89 6e 04              MOV [ESI+0x4], EBP            ; piVar4[1] = *(iVar2+4)
//   89 06                 MOV [ESI], EAX                ; *piVar4 = iVar2
//   89 70 04              MOV [EAX+0x4], ESI            ; *(iVar2+4) = piVar4
//   83 41 18 ff           ADD dword ptr [ECX+0x18], -1  ; iVar5->field_0x18--
//   33 c9                 XOR ECX, ECX
//   87 0a                 XCHG [EDX], ECX               ; *piVar1 = 0 (release)
//   8b 53 24              MOV EDX, [EBX+0x24]           ; this->field_0x24
//   8b 4a 18              MOV ECX, [EDX+0x18]
//   8b 01                 MOV EAX, [ECX]                ; vtable
//   8b 54 24 14           MOV EDX, [ESP+0x14]           ; stashed pointer
//   8b 40 28              MOV EAX, [EAX+0x28]
//   52                    PUSH EDX
//   ff d0                 CALL EAX                      ; ->vtable[0x28](stash)
//   8b 17                 MOV EDX, [EDI]                ; *piVar3 = vtable
//   8b 42 30              MOV EAX, [EDX+0x30]
//   8b cf                 MOV ECX, EDI
//   ff d0                 CALL EAX                      ; piVar3->vtable[0x30]()
//   5f                    POP EDI
//   5e                    POP ESI
//   5d                    POP EBP
//   5b                    POP EBX
//   c2 04 00              RET 4

extern "C" __declspec(naked) void FUN_00412b80()
{
    __asm {
        // 00012b80: 53                PUSH EBX
        _emit 0x53
        // 00012b81: 55                PUSH EBP
        _emit 0x55
        // 00012b82: 56                PUSH ESI
        _emit 0x56
        // 00012b83: 8b d9             MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00012b85: 57                PUSH EDI
        _emit 0x57
        // 00012b86: 8b 7b 10          MOV EDI, dword ptr [EBX+0x10]
        _emit 0x8b
        _emit 0x7b
        _emit 0x10
        // 00012b89: 8b 07             MOV EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 00012b8b: 8b 50 2c          MOV EDX, dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00012b8e: 8b cf             MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00012b90: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012b92: 8b 4c 24 14       MOV ECX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00012b96: 8b 01             MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00012b98: 8b 50 14          MOV EDX, dword ptr [EAX+0x14]
        _emit 0x8b
        _emit 0x50
        _emit 0x14
        // 00012b9b: ff d2             CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012b9d: 8b 10             MOV EDX, dword ptr [EAX]
        _emit 0x8b
        _emit 0x10
        // 00012b9f: 8b c8             MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00012ba1: 8b 42 04          MOV EAX, dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00012ba4: ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00012ba6: 8b f0             MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00012ba8: 83 7e 2c 00       CMP dword ptr [ESI+0x2c], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x2c
        _emit 0x00
        // 00012bac: 74 13             JZ +0x13
        _emit 0x74
        _emit 0x13
        // 00012bae: 8d 6e 04          LEA EBP, [ESI+0x4]
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // 00012bb1: 8b 55 00          MOV EDX, dword ptr [EBP]
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00012bb4: 8b 42 20          MOV EAX, dword ptr [EDX+0x20]
        _emit 0x8b
        _emit 0x42
        _emit 0x20
        // 00012bb7: 8b cd             MOV ECX, EDX
        _emit 0x8b
        _emit 0xcd
        // 00012bb9: ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00012bbb: 83 7e 2c 00       CMP dword ptr [ESI+0x2c], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x2c
        _emit 0x00
        // 00012bbf: 75 f0             JNZ -0x10 (to 0x00012bae)
        _emit 0x75
        _emit 0xf0
        // 00012bc1: 8b 4e 18          MOV ECX, dword ptr [ESI+0x18]
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 00012bc4: 8b 16             MOV EDX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x16
        // 00012bc6: 8b 02             MOV EAX, dword ptr [EDX]
        _emit 0x8b
        _emit 0x02
        // 00012bc8: 89 4c 24 14       MOV dword ptr [ESP+0x14], ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00012bcc: 6a 00             PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00012bce: 8b ce             MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00012bd0: ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00012bd2: 8b 4b 10          MOV ECX, dword ptr [EBX+0x10]
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00012bd5: 8b 11             MOV EDX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x11
        // 00012bd7: 8b 42 04          MOV EAX, dword ptr [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00012bda: ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00012bdc: 8b 48 10          MOV ECX, dword ptr [EAX+0x10]
        _emit 0x8b
        _emit 0x48
        _emit 0x10
        // 00012bdf: 8d 51 04          LEA EDX, [ECX+0x4]
        _emit 0x8d
        _emit 0x51
        _emit 0x04
        // 00012be2: b8 01 00 00 00    MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012be7: 8b ea             MOV EBP, EDX
        _emit 0x8b
        _emit 0xea
        // 00012be9: 87 45 00          XCHG dword ptr [EBP], EAX
        _emit 0x87
        _emit 0x45
        _emit 0x00
        // 00012bec: 85 c0             TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00012bee: 75 f2             JNZ -0xe (to 0x00012be2)
        _emit 0x75
        _emit 0xf2
        // 00012bf0: 8b 41 0c          MOV EAX, dword ptr [ECX+0x0c]
        _emit 0x8b
        _emit 0x41
        _emit 0x0c
        // 00012bf3: 8b 68 04          MOV EBP, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x68
        _emit 0x04
        // 00012bf6: 89 75 00          MOV dword ptr [EBP], ESI
        _emit 0x89
        _emit 0x75
        _emit 0x00
        // 00012bf9: 8b 68 04          MOV EBP, dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x68
        _emit 0x04
        // 00012bfc: 89 6e 04          MOV dword ptr [ESI+0x4], EBP
        _emit 0x89
        _emit 0x6e
        _emit 0x04
        // 00012bff: 89 06             MOV dword ptr [ESI], EAX
        _emit 0x89
        _emit 0x06
        // 00012c01: 89 70 04          MOV dword ptr [EAX+0x4], ESI
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 00012c04: 83 41 18 ff       ADD dword ptr [ECX+0x18], -1
        _emit 0x83
        _emit 0x41
        _emit 0x18
        _emit 0xff
        // 00012c08: 33 c9             XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 00012c0a: 87 0a             XCHG dword ptr [EDX], ECX
        _emit 0x87
        _emit 0x0a
        // 00012c0c: 8b 53 24          MOV EDX, dword ptr [EBX+0x24]
        _emit 0x8b
        _emit 0x53
        _emit 0x24
        // 00012c0f: 8b 4a 18          MOV ECX, dword ptr [EDX+0x18]
        _emit 0x8b
        _emit 0x4a
        _emit 0x18
        // 00012c12: 8b 01             MOV EAX, dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 00012c14: 8b 54 24 14       MOV EDX, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00012c18: 8b 40 28          MOV EAX, dword ptr [EAX+0x28]
        _emit 0x8b
        _emit 0x40
        _emit 0x28
        // 00012c1b: 52                PUSH EDX
        _emit 0x52
        // 00012c1c: ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00012c1e: 8b 17             MOV EDX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x17
        // 00012c20: 8b 42 30          MOV EAX, dword ptr [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00012c23: 8b cf             MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00012c25: ff d0             CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00012c27: 5f                POP EDI
        _emit 0x5f
        // 00012c28: 5e                POP ESI
        _emit 0x5e
        // 00012c29: 5d                POP EBP
        _emit 0x5d
        // 00012c2a: 5b                POP EBX
        _emit 0x5b
        // 00012c2b: c2 04 00          RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
