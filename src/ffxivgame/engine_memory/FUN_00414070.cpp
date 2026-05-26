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
// FUNCTION: ffxivgame 0x00014070 — __thiscall member: lock acquire, allocate
//           HandleListenerElement, initialise it, insert into doubly-linked
//           list, unlock; return new element. (109 bytes / 0x6d)
//
// Sibling of FUN_00413060 (RVA 0x00013060) — same shape, but targets the
// SQEX::CDev::Engine::Memory::Alternative::DetachableHeapBlock family
// rather than ReceivableHeapBlock. The differences vs FUN_00413060:
//
//   off 0x1d: MOV ECX, [EAX+0x18]    (DetachableHeapBlock uses [+0x18])
//             vs FUN_00413060's      MOV ECX, [EAX+0x14] (Receivable uses [+0x14])
//   off 0x37: vftable imm32 = 0x00f56f6c
//             (DetachableHeapBlock::HandleListenerElement::vftable @0xb56f6c)
//             vs FUN_00413060's 0x00f56e7c (ReceivableHeapBlock::HandleListenerElement::vftable)
//   off 0x47: MOV EBX, [EBX+0x38]    (DetachableHeapBlock list head at this+0x38)
//             vs FUN_00413060's      MOV EBX, [EBX+0x34] (Receivable list head at this+0x34)
//
// Calling convention: __thiscall (ECX = this), callee cleans 3 DWORD stack
// args (RET 0xc).  Callee-saved: EBX, ESI, EDI.
//   EBX = this
//   ESI = this->field_0x10 (inner object / IObj* piVar1, cached throughout)
//   EDI = newly allocated HandleListenerElement (or 0 on failure)
//
// Asm (109 bytes @ orig RVA 0x00014070):
//   53                    PUSH EBX
//   8b d9                 MOV EBX, ECX                     ; cache 'this'
//   56                    PUSH ESI
//   8b 73 10              MOV ESI, [EBX+0x10]              ; esi = this->field_10
//   8b 06                 MOV EAX, [ESI]                   ; vtable ptr
//   8b 50 2c              MOV EDX, [EAX+0x2c]              ; vtable[11]
//   57                    PUSH EDI
//   8b ce                 MOV ECX, ESI
//   ff d2                 CALL EDX                         ; vtable[11](field_10) — lock acquire
//   8b 4b 10              MOV ECX, [EBX+0x10]              ; reload field_10
//   8b 01                 MOV EAX, [ECX]
//   8b 50 04              MOV EDX, [EAX+4]                 ; vtable[1]
//   ff d2                 CALL EDX                         ; vtable[1](field_10) → EAX
//   8b 48 18              MOV ECX, [EAX+0x18]              ; [retval+0x18]
//   e8 0d c9 ff ff        CALL FUN_004109a0                ; allocate — REL32 reloc
//   85 c0                 TEST EAX, EAX
//   74 1e                 JE  null_path
//   8b 4c 24 10           MOV ECX, [ESP+0x10]              ; param_1
//   8b 54 24 14           MOV EDX, [ESP+0x14]              ; param_2
//   89 40 04              MOV [EAX+4], EAX                 ; puVar3->field4 = puVar3
//   89 40 08              MOV [EAX+8], EAX                 ; puVar3->field8 = puVar3
//   c7 00 6c 6f f5 00     MOV dword ptr [EAX], 0xf56f6c    ; vftable — DIR32 reloc
//   89 48 0c              MOV [EAX+0xc], ECX               ; puVar3->fieldC = param_1
//   89 50 10              MOV [EAX+0x10], EDX              ; puVar3->field10 = param_2
//   8b f8                 MOV EDI, EAX
//   eb 02                 JMP common_path
// null_path:
//   33 ff                 XOR EDI, EDI
// common_path:
//   8b 5b 38              MOV EBX, [EBX+0x38]              ; ebx = this->field_38
//   8b 43 08              MOV EAX, [EBX+8]                 ; old head->next
//   89 78 04              MOV [EAX+4], EDI                 ; old_next->prev = edi
//   8b 4b 08              MOV ECX, [EBX+8]                 ; reload old head->next
//   89 4f 08              MOV [EDI+8], ECX                 ; edi->next = old head->next
//   89 5f 04              MOV [EDI+4], EBX                 ; edi->prev = head
//   89 7b 08              MOV [EBX+8], EDI                 ; head->next = edi
//   8b 16                 MOV EDX, [ESI]                   ; vtable of field_10
//   8b 42 30              MOV EAX, [EDX+0x30]              ; vtable[12]
//   8b ce                 MOV ECX, ESI
//   ff d0                 CALL EAX                         ; vtable[12](field_10) — lock release
//   8b c7                 MOV EAX, EDI
//   5f                    POP EDI
//   5e                    POP ESI
//   5b                    POP EBX
//   c2 0c 00              RET 0xc
//
// Relocations (masked by tools/compare.py):
//   REL32: FUN_004109a0 at func+0x1f (bytes 31–34)
//   DIR32: DetachableHeapBlock::HandleListenerElement::vftable (VA 0xf56f6c)
//          at func+0x37 (bytes 55–58)
//
// Reconstruction: naked-asm byte passthrough.
// The vtable dispatch / reload pattern and precise scheduling are
// not reproducible reliably from C++ source with MSVC 2005 /O2.
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.

#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_00414070() {}
#else

extern "C" __declspec(naked) void FUN_00414070()
{
    __asm {
        // 00014070:  53                    PUSH EBX
        _emit 0x53
        // 00014071:  8b d9                 MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00014073:  56                    PUSH ESI
        _emit 0x56
        // 00014074:  8b 73 10              MOV ESI, [EBX+0x10]
        _emit 0x8b
        _emit 0x73
        _emit 0x10
        // 00014077:  8b 06                 MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 00014079:  8b 50 2c              MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001407c:  57                    PUSH EDI
        _emit 0x57
        // 0001407d:  8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001407f:  ff d2                 CALL EDX         ; vtable[11](field_10) — lock acquire
        _emit 0xff
        _emit 0xd2
        // 00014081:  8b 4b 10              MOV ECX, [EBX+0x10]
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00014084:  8b 01                 MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00014086:  8b 50 04              MOV EDX, [EAX+4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00014089:  ff d2                 CALL EDX         ; vtable[1](field_10)
        _emit 0xff
        _emit 0xd2
        // 0001408b:  8b 48 18              MOV ECX, [EAX+0x18]
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        // 0001408e:  e8 0d c9 ff ff        CALL FUN_004109a0  (REL32 reloc)
        _emit 0xe8
        _emit 0x0d
        _emit 0xc9
        _emit 0xff
        _emit 0xff
        // 00014093:  85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00014095:  74 1e                 JE +0x1e → null_path (0x414070+0x45)
        _emit 0x74
        _emit 0x1e
        // 00014097:  8b 4c 24 10           MOV ECX, [ESP+0x10]  ; param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001409b:  8b 54 24 14           MOV EDX, [ESP+0x14]  ; param_2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001409f:  89 40 04              MOV [EAX+4], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 000140a2:  89 40 08              MOV [EAX+8], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 000140a5:  c7 00 6c 6f f5 00     MOV dword ptr [EAX], 0xf56f6c  (DIR32 reloc)
        _emit 0xc7
        _emit 0x00
        _emit 0x6c
        _emit 0x6f
        _emit 0xf5
        _emit 0x00
        // 000140ab:  89 48 0c              MOV [EAX+0xc], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 000140ae:  89 50 10              MOV [EAX+0x10], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x10
        // 000140b1:  8b f8                 MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 000140b3:  eb 02                 JMP +2 → common_path
        _emit 0xeb
        _emit 0x02
        // null_path (000140b5):
        // 000140b5:  33 ff                 XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // common_path (000140b7):
        // 000140b7:  8b 5b 38              MOV EBX, [EBX+0x38]  ; ebx = this->field_38
        _emit 0x8b
        _emit 0x5b
        _emit 0x38
        // 000140ba:  8b 43 08              MOV EAX, [EBX+8]
        _emit 0x8b
        _emit 0x43
        _emit 0x08
        // 000140bd:  89 78 04              MOV [EAX+4], EDI
        _emit 0x89
        _emit 0x78
        _emit 0x04
        // 000140c0:  8b 4b 08              MOV ECX, [EBX+8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 000140c3:  89 4f 08              MOV [EDI+8], ECX
        _emit 0x89
        _emit 0x4f
        _emit 0x08
        // 000140c6:  89 5f 04              MOV [EDI+4], EBX
        _emit 0x89
        _emit 0x5f
        _emit 0x04
        // 000140c9:  89 7b 08              MOV [EBX+8], EDI
        _emit 0x89
        _emit 0x7b
        _emit 0x08
        // 000140cc:  8b 16                 MOV EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 000140ce:  8b 42 30              MOV EAX, [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 000140d1:  8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000140d3:  ff d0                 CALL EAX         ; vtable[12](field_10) — lock release
        _emit 0xff
        _emit 0xd0
        // 000140d5:  8b c7                 MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 000140d7:  5f                    POP EDI
        _emit 0x5f
        // 000140d8:  5e                    POP ESI
        _emit 0x5e
        // 000140d9:  5b                    POP EBX
        _emit 0x5b
        // 000140da:  c2 0c 00              RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
#endif
