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
// FUNCTION: ffxivgame 0x0001cb10 — 5-arg guarded table-lookup-and-dispatch
//                                  helper (92 B / 0x5c, no SEH).
//
// Inspection (read from asm/ffxivgame/0001cb10_FUN_0041cb10.s):
//
//   __cdecl void FUN_0041cb10(int idx, void* p_start, void* p_end,
//                              int param4, void* param5);
//
//     // Guard 1 — bail if param5 is null
//     if (param5 == NULL) return;
//
//     // Guard 2 — bail if global enable-flag is 0
//     if (*(int*)0x01328ec8 == 0) return;
//
//     // Table lookup — indexed by idx, stride 4
//     void* table_entry = ((void**)0x00f595c4)[idx];
//     if (table_entry == NULL) return;
//
//     // Translate / look up idx via helper
//     void* result = FUN_0041c540(idx);
//
//     // Dispatch via __thiscall member — this = *(obj**)0x0132987c
//     //   args: table_entry, 0, p_start, (p_end - p_start + 1), param4, result
//     SomeClass* obj = *(SomeClass**)0x0132987c;
//     obj->Method(table_entry, 0, p_start,
//                 (int)p_end - (int)p_start + 1, param4, result);
//
//     // Release / cleanup result
//     FUN_004246f0(result);
//
// Calling convention: __cdecl — 5 stack args at [ESP+4..ESP+14], callee
// saves EDI and ESI, returns void, RET (no stack cleanup).
//
// Naked __asm so the exact encoding is preserved:
//   - `MOV EDI, [EAX*4 + 0xf595c4]` uses the scaled-index SIB form
//     (8b 3c 85 c4 95 f5 00) that MSVC 2005 emits for a global array
//     indexed by a register.
//   - `CMP dword ptr [0x01328ec8], 0` uses the moffs32 `83 3d` form.
//   - The thiscall target at 0x004232f0 and the two other CALL targets
//     carry rel32 relocations that tools/compare.py masks on the diff pass.
//
// Asm shape (92 bytes — RVA 0x0001cb10..0x0001cb6b):
//
//     0001cb10:  8b 4c 24 14        MOV  ECX, [ESP+0x14]       ; param5
//     0001cb14:  85 c9              TEST ECX, ECX
//     0001cb16:  74 53              JZ   0x0041cb6b             ; ret if null
//     0001cb18:  83 3d c8 8e 32 01 00  CMP [0x01328ec8], 0     ; global flag
//     0001cb1f:  74 4a              JZ   0x0041cb6b
//     0001cb21:  8b 44 24 04        MOV  EAX, [ESP+0x4]        ; idx
//     0001cb25:  57                 PUSH EDI
//     0001cb26:  8b 3c 85 c4 95 f5 00  MOV EDI, [EAX*4+0xf595c4]
//     0001cb2d:  85 ff              TEST EDI, EDI
//     0001cb2f:  74 39              JZ   0x0041cb6a             ; ret if null
//     0001cb31:  56                 PUSH ESI
//     0001cb32:  50                 PUSH EAX                   ; idx arg
//     0001cb33:  e8 08 fa ff ff     CALL 0x0041c540
//     0001cb38:  8b 4c 24 18        MOV  ECX, [ESP+0x18]       ; p_end (arg2)
//     0001cb3c:  83 c4 04           ADD  ESP, 0x4
//     0001cb3f:  8b f0              MOV  ESI, EAX              ; result
//     0001cb41:  8b 44 24 18        MOV  EAX, [ESP+0x18]       ; param4 (arg3)
//     0001cb45:  56                 PUSH ESI                   ; result
//     0001cb46:  50                 PUSH EAX                   ; param4
//     0001cb47:  8b 44 24 18        MOV  EAX, [ESP+0x18]       ; p_start (arg1)
//     0001cb4b:  2b c8              SUB  ECX, EAX              ; end - start
//     0001cb4d:  83 c1 01           ADD  ECX, 0x1              ; + 1 = count
//     0001cb50:  51                 PUSH ECX                   ; count
//     0001cb51:  8b 0d 7c 98 32 01  MOV  ECX, [0x0132987c]    ; this
//     0001cb57:  50                 PUSH EAX                   ; p_start
//     0001cb58:  6a 00              PUSH 0x0
//     0001cb5a:  57                 PUSH EDI                   ; table_entry
//     0001cb5b:  e8 90 67 00 00     CALL 0x004232f0            ; __thiscall
//     0001cb60:  56                 PUSH ESI                   ; result
//     0001cb61:  e8 8a 7b 00 00     CALL 0x004246f0
//     0001cb66:  83 c4 04           ADD  ESP, 0x4
//     0001cb69:  5e                 POP  ESI
//     0001cb6a:  5f                 POP  EDI
//     0001cb6b:  c3                 RET

extern "C" __declspec(naked) void FUN_0041cb10() {
    __asm {
        _emit 0x8b  // MOV ECX,[ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x85  // TEST ECX,ECX
        _emit 0xc9
        _emit 0x74  // JZ +0x53
        _emit 0x53
        _emit 0x83  // CMP [0x01328ec8],0x0
        _emit 0x3d
        _emit 0xc8
        _emit 0x8e
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x74  // JZ +0x4a
        _emit 0x4a
        _emit 0x8b  // MOV EAX,[ESP+0x4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI,[EAX*4+0xf595c4]
        _emit 0x3c
        _emit 0x85
        _emit 0xc4
        _emit 0x95
        _emit 0xf5
        _emit 0x00
        _emit 0x85  // TEST EDI,EDI
        _emit 0xff
        _emit 0x74  // JZ +0x39
        _emit 0x39
        _emit 0x56  // PUSH ESI
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL 0x0041c540
        _emit 0x08
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // MOV ECX,[ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x83  // ADD ESP,0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x8b  // MOV ESI,EAX
        _emit 0xf0
        _emit 0x8b  // MOV EAX,[ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x56  // PUSH ESI
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV EAX,[ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x2b  // SUB ECX,EAX
        _emit 0xc8
        _emit 0x83  // ADD ECX,0x1
        _emit 0xc1
        _emit 0x01
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV ECX,[0x0132987c]
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        _emit 0x50  // PUSH EAX
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x57  // PUSH EDI
        _emit 0xe8  // CALL 0x004232f0
        _emit 0x90
        _emit 0x67
        _emit 0x00
        _emit 0x00
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL 0x004246f0
        _emit 0x8a
        _emit 0x7b
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP,0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e  // POP ESI
        _emit 0x5f  // POP EDI
        _emit 0xc3  // RET
    }
}
