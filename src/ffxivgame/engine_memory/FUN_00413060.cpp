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
// FUNCTION: ffxivgame 0x00013060 — __thiscall member: lock acquire, allocate
//           HandleListenerElement, initialise it, insert into doubly-linked
//           list, unlock; return new element. (109 bytes / 0x6d)
//
// Calling convention: __thiscall (ECX = this), callee cleans 3 DWORD stack
// args (RET 0xc).  Callee-saved: EBX, ESI, EDI.
//   EBX = this
//   ESI = this->field_0x10 (inner object / IObj* piVar1, cached throughout)
//   EDI = newly allocated HandleListenerElement (or 0 on failure)
//
// Asm (109 bytes @ orig RVA 0x00013060):
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
//   8b 48 14              MOV ECX, [EAX+0x14]              ; [retval+0x14]
//   e8 1d d9 ff ff        CALL FUN_004109a0                 ; allocate — REL32 reloc
//   85 c0                 TEST EAX, EAX
//   74 1e                 JE  null_path
//   8b 4c 24 10           MOV ECX, [ESP+0x10]              ; param_1
//   8b 54 24 14           MOV EDX, [ESP+0x14]              ; param_2
//   89 40 04              MOV [EAX+4], EAX                 ; puVar3->field4 = puVar3
//   89 40 08              MOV [EAX+8], EAX                 ; puVar3->field8 = puVar3
//   c7 00 7c 6e f5 00     MOV dword ptr [EAX], 0xf56e7c    ; vftable — DIR32 reloc
//   89 48 0c              MOV [EAX+0xc], ECX               ; puVar3->fieldC = param_1
//   89 50 10              MOV [EAX+0x10], EDX              ; puVar3->field10 = param_2
//   8b f8                 MOV EDI, EAX
//   eb 02                 JMP common_path
// null_path:
//   33 ff                 XOR EDI, EDI
// common_path:
//   8b 5b 34              MOV EBX, [EBX+0x34]              ; ebx = this->field_34
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
//   DIR32: ReceivableHeapBlock::HandleListenerElement::vftable (VA 0xf56e7c)
//          at func+0x37 (bytes 55–58)
//
// Reconstruction: naked-asm byte passthrough.
// The vtable dispatch / reload pattern and precise scheduling are
// not reproducible reliably from C++ source with MSVC 2005 /O2.
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.

#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_00413060() {}
#else

extern "C" __declspec(naked) void FUN_00413060()
{
    __asm {
        // 00013060:  53                    PUSH EBX
        _emit 0x53
        // 00013061:  8b d9                 MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00013063:  56                    PUSH ESI
        _emit 0x56
        // 00013064:  8b 73 10              MOV ESI, [EBX+0x10]
        _emit 0x8b
        _emit 0x73
        _emit 0x10
        // 00013067:  8b 06                 MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 00013069:  8b 50 2c              MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001306c:  57                    PUSH EDI
        _emit 0x57
        // 0001306d:  8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001306f:  ff d2                 CALL EDX         ; vtable[11](field_10) — lock acquire
        _emit 0xff
        _emit 0xd2
        // 00013071:  8b 4b 10              MOV ECX, [EBX+0x10]
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00013074:  8b 01                 MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00013076:  8b 50 04              MOV EDX, [EAX+4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013079:  ff d2                 CALL EDX         ; vtable[1](field_10)
        _emit 0xff
        _emit 0xd2
        // 0001307b:  8b 48 14              MOV ECX, [EAX+0x14]
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // 0001307e:  e8 1d d9 ff ff        CALL FUN_004109a0  (REL32 reloc)
        _emit 0xe8
        _emit 0x1d
        _emit 0xd9
        _emit 0xff
        _emit 0xff
        // 00013083:  85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00013085:  74 1e                 JE +0x1e → null_path (0x413060+0x45)
        _emit 0x74
        _emit 0x1e
        // 00013087:  8b 4c 24 10           MOV ECX, [ESP+0x10]  ; param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001308b:  8b 54 24 14           MOV EDX, [ESP+0x14]  ; param_2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001308f:  89 40 04              MOV [EAX+4], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 00013092:  89 40 08              MOV [EAX+8], EAX
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 00013095:  c7 00 7c 6e f5 00     MOV dword ptr [EAX], 0xf56e7c  (DIR32 reloc)
        _emit 0xc7
        _emit 0x00
        _emit 0x7c
        _emit 0x6e
        _emit 0xf5
        _emit 0x00
        // 0001309b:  89 48 0c              MOV [EAX+0xc], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x0c
        // 0001309e:  89 50 10              MOV [EAX+0x10], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x10
        // 000130a1:  8b f8                 MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 000130a3:  eb 02                 JMP +2 → common_path
        _emit 0xeb
        _emit 0x02
        // null_path (000130a5):
        // 000130a5:  33 ff                 XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // common_path (000130a7):
        // 000130a7:  8b 5b 34              MOV EBX, [EBX+0x34]  ; ebx = this->field_34
        _emit 0x8b
        _emit 0x5b
        _emit 0x34
        // 000130aa:  8b 43 08              MOV EAX, [EBX+8]
        _emit 0x8b
        _emit 0x43
        _emit 0x08
        // 000130ad:  89 78 04              MOV [EAX+4], EDI
        _emit 0x89
        _emit 0x78
        _emit 0x04
        // 000130b0:  8b 4b 08              MOV ECX, [EBX+8]
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 000130b3:  89 4f 08              MOV [EDI+8], ECX
        _emit 0x89
        _emit 0x4f
        _emit 0x08
        // 000130b6:  89 5f 04              MOV [EDI+4], EBX
        _emit 0x89
        _emit 0x5f
        _emit 0x04
        // 000130b9:  89 7b 08              MOV [EBX+8], EDI
        _emit 0x89
        _emit 0x7b
        _emit 0x08
        // 000130bc:  8b 16                 MOV EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 000130be:  8b 42 30              MOV EAX, [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 000130c1:  8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000130c3:  ff d0                 CALL EAX         ; vtable[12](field_10) — lock release
        _emit 0xff
        _emit 0xd0
        // 000130c5:  8b c7                 MOV EAX, EDI
        _emit 0x8b
        _emit 0xc7
        // 000130c7:  5f                    POP EDI
        _emit 0x5f
        // 000130c8:  5e                    POP ESI
        _emit 0x5e
        // 000130c9:  5b                    POP EBX
        _emit 0x5b
        // 000130ca:  c2 0c 00              RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
#endif
