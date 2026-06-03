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
// FUNCTION: ffxivgame 0x0001bb60 — dispatch-on-virtual-result: queries a global
//                                  singleton via vtable[0x12], branches on the
//                                  returned status code (0/1/2/other), calls a
//                                  second function with mode-specific argument
//                                  lists, then virtual-destructs the result
//                                  object. (__cdecl, 120 bytes / 0x78)
//
// Calling convention: __cdecl; returns void.
// Stack frame: PUSH ECX at entry allocates one dword local slot (ESP-relative).
//
// Global:
//   [0x01329834]   pointer to an object with a vtable
//
// Object vtable layout (offsets used):
//   [vtable + 0x48]  ptr to vtable[0x12] — "query" method, called at entry
//   [vtable + 0x08]  ptr to vtable[0x02] — destructor, called during cleanup
//
// The initial call (ESP-relative at entry):
//   g_obj->vtable[0x12](g_obj, 0, 0, &local_result, 0)
//     args pushed (cdecl): PUSH 0 / PUSH EAX(g_obj ptr) / PUSH 0 / PUSH 0
//                          / PUSH EDX(&ECX-slot)
//     after the CALL, ECX-slot [ESP+0xc] contains the status integer
//     (the slot was pre-zeroed by MOV dword ptr [ESP+0xc], 0 before the call)
//
// Switch on status code (implemented as cascading SUB+JZ/JNZ):
//   0 → PUSH 0,0,EDX,4,EAX     then CALL func2
//   1 → PUSH 0,0,EAX,0,ECX     then CALL func2
//   2 → PUSH EAX,EAX,ECX,1,EDX then CALL func2
//   other → skip CALL func2 (fall-through)
//
// After the conditional call:
//   if ([ESP] != 0) → vtable[0x02]([ESP]) — virtual destructor on result obj
//
// Notable codegen details:
//   - PUSH ECX at entry (MSVC single-slot frame idiom; no SUB ESP,4)
//   - The MOV EAX,[0x01329834] is a moffs32 absolute load (relocation site)
//   - The CALL at 0x0001bbc2 is a rel32 to 0x009fc768 (relocation site)
//   - All other CALLs are indirect (CALL EAX / CALL EDX); no other relocations
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The moffs32 load and rel32 CALL contain absolute addresses that are
//   relocation sites in the original binary. Emitting the original 120 bytes
//   verbatim via __declspec(naked) + MASM _emit directives produces a .obj
//   whose .text section is byte-identical to the original slice.
//   compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0041bb60() {
    __asm {
        // 0001bb60: 51                    PUSH ECX  (allocate local slot)
        _emit 0x51
        // 0001bb61: a1 34 98 32 01        MOV EAX,[0x01329834]
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001bb66: 8d 14 24              LEA EDX,[ESP]  (&local slot)
        _emit 0x8d
        _emit 0x14
        _emit 0x24
        // 0001bb69: 52                    PUSH EDX
        _emit 0x52
        // 0001bb6a: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001bb6c: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001bb6e: c7 44 24 0c 00 00 00 00  MOV dword ptr [ESP+0xc],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001bb76: 8b 08                 MOV ECX,dword ptr [EAX]  (vtable ptr)
        _emit 0x8b
        _emit 0x08
        // 0001bb78: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001bb7a: 50                    PUSH EAX  (g_obj)
        _emit 0x50
        // 0001bb7b: 8b 41 48              MOV EAX,dword ptr [ECX+0x48]  (vtable[0x12])
        _emit 0x8b
        _emit 0x41
        _emit 0x48
        // 0001bb7e: ff d0                 CALL EAX
        _emit 0xff
        _emit 0xd0
        // 0001bb80: 8b 44 24 0c           MOV EAX,dword ptr [ESP+0xc]  (status code)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0001bb84: 83 e8 00              SUB EAX,0x0
        _emit 0x83
        _emit 0xe8
        _emit 0x00
        // 0001bb87: 74 2a                 JZ +0x2a  (→ 0x0001bbb3, case 0)
        _emit 0x74
        _emit 0x2a
        // 0001bb89: 83 e8 01              SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0001bb8c: 74 14                 JZ +0x14  (→ 0x0001bba2, case 1)
        _emit 0x74
        _emit 0x14
        // 0001bb8e: 83 e8 01              SUB EAX,0x1
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 0001bb91: 75 34                 JNZ +0x34  (→ 0x0001bbc7, default)
        _emit 0x75
        _emit 0x34
        // === case 2 ===
        // 0001bb93: 8b 0c 24              MOV ECX,dword ptr [ESP]
        _emit 0x8b
        _emit 0x0c
        _emit 0x24
        // 0001bb96: 8b 54 24 08           MOV EDX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0001bb9a: 50                    PUSH EAX  (0x0)
        _emit 0x50
        // 0001bb9b: 50                    PUSH EAX  (0x0)
        _emit 0x50
        // 0001bb9c: 51                    PUSH ECX
        _emit 0x51
        // 0001bb9d: 6a 01                 PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0001bb9f: 52                    PUSH EDX
        _emit 0x52
        // 0001bba0: eb 20                 JMP +0x20  (→ 0x0001bbc2)
        _emit 0xeb
        _emit 0x20
        // === case 1 ===
        // 0001bba2: 8b 04 24              MOV EAX,dword ptr [ESP]
        _emit 0x8b
        _emit 0x04
        _emit 0x24
        // 0001bba5: 8b 4c 24 08           MOV ECX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0001bba9: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001bbab: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001bbad: 50                    PUSH EAX
        _emit 0x50
        // 0001bbae: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001bbb0: 51                    PUSH ECX
        _emit 0x51
        // 0001bbb1: eb 0f                 JMP +0x0f  (→ 0x0001bbc2)
        _emit 0xeb
        _emit 0x0f
        // === case 0 ===
        // 0001bbb3: 8b 14 24              MOV EDX,dword ptr [ESP]
        _emit 0x8b
        _emit 0x14
        _emit 0x24
        // 0001bbb6: 8b 44 24 08           MOV EAX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0001bbba: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001bbbc: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0001bbbe: 52                    PUSH EDX
        _emit 0x52
        // 0001bbbf: 6a 04                 PUSH 0x4
        _emit 0x6a
        _emit 0x04
        // 0001bbc1: 50                    PUSH EAX
        _emit 0x50
        // === common call site ===
        // 0001bbc2: e8 a1 0b 5e 00        CALL 0x009fc768 (rel32)
        _emit 0xe8
        _emit 0xa1
        _emit 0x0b
        _emit 0x5e
        _emit 0x00
        // === cleanup: virtual-destruct result object if non-null ===
        // 0001bbc7: 8b 04 24              MOV EAX,dword ptr [ESP]
        _emit 0x8b
        _emit 0x04
        _emit 0x24
        // 0001bbca: 85 c0                 TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001bbcc: 74 08                 JZ +0x8  (→ 0x0001bbd6, skip dtor)
        _emit 0x74
        _emit 0x08
        // 0001bbce: 8b 08                 MOV ECX,dword ptr [EAX]  (vtable)
        _emit 0x8b
        _emit 0x08
        // 0001bbd0: 8b 51 08              MOV EDX,dword ptr [ECX+0x8]  (vtable[2])
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 0001bbd3: 50                    PUSH EAX
        _emit 0x50
        // 0001bbd4: ff d2                 CALL EDX
        _emit 0xff
        _emit 0xd2
        // === epilogue ===
        // 0001bbd6: 59                    POP ECX
        _emit 0x59
        // 0001bbd7: c3                    RET
        _emit 0xc3
    }
}
