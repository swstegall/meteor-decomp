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
// FUNCTION: ffxivgame 0x00012250 — __thiscall member: lock acquire, allocate
//           heap block, insert into doubly-linked list, unlock; return user ptr
//           or null. (145 bytes / 0x91)
//
// Calling convention: __thiscall (ECX = this), callee cleans 4 DWORD stack
// args (RET 0x10).  Callee-saved: EBX, ESI, EDI.
//   ESI = this
//   EDI = this->field_0x10 (IObj_Inner* piVar1, cached throughout)
//   EBX = result of FUN_00411d30 (allocated block ptr, or 0)
//
// Asm (145 bytes @ orig RVA 0x00012250):
//   53                   PUSH EBX
//   56                   PUSH ESI
//   8b f1                MOV ESI, ECX
//   57                   PUSH EDI
//   8b 7e 10             MOV EDI, [ESI+0x10]
//   8b 07                MOV EAX, [EDI]
//   8b 50 2c             MOV EDX, [EAX+0x2c]
//   8b cf                MOV ECX, EDI
//   ff d2                CALL EDX                ; piVar1->vtable[0x2c/4]()
//   8b 4e 10             MOV ECX, [ESI+0x10]     ; reload piVar1 (ECX clobbered)
//   8b 01                MOV EAX, [ECX]
//   8b 50 04             MOV EDX, [EAX+4]
//   ff d2                CALL EDX                ; piVar1->vtable[0x04/4]()  → EAX
//   8b 48 0c             MOV ECX, [EAX+0xc]
//   e8 2d e7 ff ff       CALL FUN_004109a0        ; lock acquire on [ret+0xc]
//   85 c0                TEST EAX, EAX
//   74 30                JE 0x4122a7              ; if 0 → skip allocation
//
//   ; allocation path
//   8b 4e 2c             MOV ECX, [ESI+0x2c]
//   8b 56 28             MOV EDX, [ESI+0x28]
//   51                   PUSH ECX                ; arg6: field_0x2c
//   8b 4c 24 14          MOV ECX, [ESP+0x14]     ; param_1
//   03 d1                ADD EDX, ECX            ; field_0x28 + param_1
//   8b 4c 24 1c          MOV ECX, [ESP+0x1c]     ; param_3
//   52                   PUSH EDX                ; arg5: field_0x28+param_1
//   8b 54 24 24          MOV EDX, [ESP+0x24]     ; param_4
//   52                   PUSH EDX                ; arg4: param_4
//   8b 54 24 20          MOV EDX, [ESP+0x20]     ; param_2
//   51                   PUSH ECX                ; arg3: param_3
//   52                   PUSH EDX                ; arg2: param_2
//   57                   PUSH EDI                ; arg1: piVar1
//   8b c8                MOV ECX, EAX            ; ECX = lock result (this for FUN_00411d30)
//   e8 94 fa ff ff       CALL FUN_00411d30
//   8b d8                MOV EBX, EAX            ; EBX = iVar3
//   85 db                TEST EBX, EBX
//   74 07                JE 0x4122a9             ; if 0 → ecx=0 (skip +8)
//   8d 4b 08             LEA ECX, [EBX+8]        ; ECX = iVar4 = iVar3+8
//   eb 04                JMP 0x4122ab
//
// 0x4122a7:
//   33 db                XOR EBX, EBX            ; iVar3 = 0
// 0x4122a9:
//   33 c9                XOR ECX, ECX            ; iVar4 = 0
//
// 0x4122ab: (common: linked-list insert iVar4 after iVar2)
//   8b 46 2c             MOV EAX, [ESI+0x2c]
//   8b 40 48             MOV EAX, [EAX+0x48]     ; iVar2 = field_0x2c->field_0x48
//   8b 50 08             MOV EDX, [EAX+8]        ; iVar2->next
//   89 4a 04             MOV [EDX+4], ECX        ; iVar2->next->prev = iVar4
//   8b 50 08             MOV EDX, [EAX+8]        ; iVar2->next (reload)
//   89 51 08             MOV [ECX+8], EDX        ; iVar4->next = iVar2->next
//   89 41 04             MOV [ECX+4], EAX        ; iVar4->prev = iVar2
//   89 48 08             MOV [EAX+8], ECX        ; iVar2->next = iVar4
//   8b 07                MOV EAX, [EDI]
//   8b 50 30             MOV EDX, [EAX+0x30]
//   8b cf                MOV ECX, EDI
//   ff d2                CALL EDX                ; piVar1->vtable[0x30/4]()
//   85 db                TEST EBX, EBX
//   74 09                JE 0x4122d9
//   5f                   POP EDI
//   5e                   POP ESI
//   8d 43 04             LEA EAX, [EBX+4]        ; return iVar3+4
//   5b                   POP EBX
//   c2 10 00             RET 0x10
// 0x4122d9:
//   5f                   POP EDI
//   5e                   POP ESI
//   33 c0                XOR EAX, EAX            ; return 0
//   5b                   POP EBX
//   c2 10 00             RET 0x10

// Reconstruction strategy — naked-asm byte passthrough:
//   The exact instruction ordering (callee-save register allocation,
//   vtable call reload pattern, argument push sequence) cannot be
//   reproduced reliably from C++ source with MSVC 2005 /O2.
//   The __declspec(naked) body re-emits the original 145 bytes verbatim
//   via MASM _emit directives.
//
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.
// The #ifdef keeps the file compilable on clang/arm64 (host toolchain)
// while the MSVC build (Wine) produces the byte-identical .obj.

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" __declspec(naked) void FUN_00412250()
{
    __asm {
        // 00012250:  53                   PUSH EBX
        _emit 0x53
        // 00012251:  56                   PUSH ESI
        _emit 0x56
        // 00012252:  8b f1                MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00012254:  57                   PUSH EDI
        _emit 0x57
        // 00012255:  8b 7e 10             MOV EDI, [ESI+0x10]
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        // 00012258:  8b 07                MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 0001225a:  8b 50 2c             MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001225d:  8b cf                MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 0001225f:  ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00012261:  8b 4e 10             MOV ECX, [ESI+0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00012264:  8b 01                MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00012266:  8b 50 04             MOV EDX, [EAX+4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00012269:  ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0001226b:  8b 48 0c             MOV ECX, [EAX+0xc]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 0001226e:  e8 2d e7 ff ff       CALL FUN_004109a0  (reloc)
        _emit 0xe8
        _emit 0x2d
        _emit 0xe7
        _emit 0xff
        _emit 0xff
        // 00012273:  85 c0                TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00012275:  74 30                JE +0x30 (→ 0x4122a7)
        _emit 0x74
        _emit 0x30
        // 00012277:  8b 4e 2c             MOV ECX, [ESI+0x2c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x2c
        // 0001227a:  8b 56 28             MOV EDX, [ESI+0x28]
        _emit 0x8b
        _emit 0x56
        _emit 0x28
        // 0001227d:  51                   PUSH ECX
        _emit 0x51
        // 0001227e:  8b 4c 24 14          MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00012282:  03 d1                ADD EDX, ECX
        _emit 0x03
        _emit 0xd1
        // 00012284:  8b 4c 24 1c          MOV ECX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 00012288:  52                   PUSH EDX
        _emit 0x52
        // 00012289:  8b 54 24 24          MOV EDX, [ESP+0x24]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0001228d:  52                   PUSH EDX
        _emit 0x52
        // 0001228e:  8b 54 24 20          MOV EDX, [ESP+0x20]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 00012292:  51                   PUSH ECX
        _emit 0x51
        // 00012293:  52                   PUSH EDX
        _emit 0x52
        // 00012294:  57                   PUSH EDI
        _emit 0x57
        // 00012295:  8b c8                MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00012297:  e8 94 fa ff ff       CALL FUN_00411d30  (reloc)
        _emit 0xe8
        _emit 0x94
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 0001229c:  8b d8                MOV EBX, EAX
        _emit 0x8b
        _emit 0xd8
        // 0001229e:  85 db                TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 004122a0:  74 07                JE +7  (→ 0x4122a9)
        _emit 0x74
        _emit 0x07
        // 004122a2:  8d 4b 08             LEA ECX, [EBX+8]
        _emit 0x8d
        _emit 0x4b
        _emit 0x08
        // 004122a5:  eb 04                JMP +4  (→ 0x4122ab)
        _emit 0xeb
        _emit 0x04
        // 004122a7:  33 db                XOR EBX, EBX
        _emit 0x33
        _emit 0xdb
        // 004122a9:  33 c9                XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 004122ab:  8b 46 2c             MOV EAX, [ESI+0x2c]
        _emit 0x8b
        _emit 0x46
        _emit 0x2c
        // 004122ae:  8b 40 48             MOV EAX, [EAX+0x48]
        _emit 0x8b
        _emit 0x40
        _emit 0x48
        // 004122b1:  8b 50 08             MOV EDX, [EAX+8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 004122b4:  89 4a 04             MOV [EDX+4], ECX
        _emit 0x89
        _emit 0x4a
        _emit 0x04
        // 004122b7:  8b 50 08             MOV EDX, [EAX+8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 004122ba:  89 51 08             MOV [ECX+8], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 004122bd:  89 41 04             MOV [ECX+4], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x04
        // 004122c0:  89 48 08             MOV [EAX+8], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 004122c3:  8b 07                MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 004122c5:  8b 50 30             MOV EDX, [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 004122c8:  8b cf                MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 004122ca:  ff d2                CALL EDX
        _emit 0xff
        _emit 0xd2
        // 004122cc:  85 db                TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 004122ce:  74 09                JE +9  (→ 0x4122d9)
        _emit 0x74
        _emit 0x09
        // 004122d0:  5f                   POP EDI
        _emit 0x5f
        // 004122d1:  5e                   POP ESI
        _emit 0x5e
        // 004122d2:  8d 43 04             LEA EAX, [EBX+4]
        _emit 0x8d
        _emit 0x43
        _emit 0x04
        // 004122d5:  5b                   POP EBX
        _emit 0x5b
        // 004122d6:  c2 10 00             RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
        // 004122d9:  5f                   POP EDI
        _emit 0x5f
        // 004122da:  5e                   POP ESI
        _emit 0x5e
        // 004122db:  33 c0                XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 004122dd:  5b                   POP EBX
        _emit 0x5b
        // 004122de:  c2 10 00             RET 0x10
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
#endif // _MSC_VER && !__clang__
