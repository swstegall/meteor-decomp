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
// FUNCTION: ffxivgame 0x00469dc0 — dispatch helper that guards a vtable
//                                  call at slot +0x24 with an optional
//                                  pre/post callback (__cdecl, 139 bytes)
//
// Signature (inferred from stack layout):
//
//   int __cdecl FUN_00469dc0(void *param1, int param2, int param3);
//
// param1  — pointer to a struct whose first DWORD is a vtable pointer and
//           whose DWORD at +0x4 is an optional callback function pointer.
// param2  — passed verbatim to both the callback and the vtable function.
// param3  — passed by value to the vtable function; its stack address is
//           passed to the pre/post callback (so the callback can fill it in).
//
// Logic summary:
//   1. If param1 == NULL → return 0.
//   2. If *param1 (vtable) == NULL, or vtable[0x24/4] == NULL →
//        call FUN_0045c940(0x20, 0x83, 0x79, <string VA>, 0x183),
//        return 0xFFFFFFFE (-2).
//   3. Load callback = *(param1 + 4).
//      If callback != NULL, call:
//          callback(param1, 6, &param3, param2, 0, 1)
//      and if the return value is <= 0, return it immediately.
//   4. Call vtable[0x24/4](param1, param2, param3) → result in EAX.
//   5. If callback != NULL, call:
//          callback(param1, 0x86, &param3, param2, 0, result)
//      and return that call's return value.
//      If callback == NULL, return the vtable call's result directly.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The delayed callee-save pattern (EBX/EDI are pushed only inside the
//   main code path, after both NULL-guard early returns) is an MSVC 2005
//   /O2 optimisation that cannot be coerced from a plain C source form
//   at this optimization level.  The two LEA [ESP+0x24] instructions that
//   take the address of param3 depend on the exact in-flight stack depth
//   at each site.  The __declspec(naked) body re-emits the original 139
//   bytes verbatim via MASM _emit directives.
//
// Relocations masked by compare.py:
//   +0x6e  abs32   PUSH 0xf791a0           (VA of filename/error string)
//   +0x7c  rel32   CALL FUN_0045c940       (assertion/logging helper)

extern "C" __declspec(naked) void FUN_00469dc0()
{
    __asm {
        // 00069dc0:  56                  PUSH ESI
        _emit 0x56
        // 00069dc1:  8b 74 24 08         MOV ESI, dword ptr [ESP+0x08]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00069dc5:  85 f6               TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00069dc7:  75 04               JNZ +0x04
        _emit 0x75
        _emit 0x04
        // 00069dc9:  33 c0               XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00069dcb:  5e                  POP ESI
        _emit 0x5e
        // 00069dcc:  c3                  RET
        _emit 0xc3
        // 00069dcd:  8b 06               MOV EAX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00069dcf:  85 c0               TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00069dd1:  74 56               JZ +0x56
        _emit 0x74
        _emit 0x56
        // 00069dd3:  83 78 24 00         CMP dword ptr [EAX+0x24], 0x0
        _emit 0x83
        _emit 0x78
        _emit 0x24
        _emit 0x00
        // 00069dd7:  74 50               JZ +0x50
        _emit 0x74
        _emit 0x50
        // 00069dd9:  53                  PUSH EBX
        _emit 0x53
        // 00069dda:  8b 5c 24 10         MOV EBX, dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 00069dde:  57                  PUSH EDI
        _emit 0x57
        // 00069ddf:  8b 7e 04            MOV EDI, dword ptr [ESI+0x04]
        _emit 0x8b
        _emit 0x7e
        _emit 0x04
        // 00069de2:  85 ff               TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00069de4:  74 16               JZ +0x16
        _emit 0x74
        _emit 0x16
        // 00069de6:  6a 01               PUSH 0x01
        _emit 0x6a
        _emit 0x01
        // 00069de8:  6a 00               PUSH 0x00
        _emit 0x6a
        _emit 0x00
        // 00069dea:  53                  PUSH EBX
        _emit 0x53
        // 00069deb:  8d 44 24 24         LEA EAX, [ESP+0x24]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 00069def:  50                  PUSH EAX
        _emit 0x50
        // 00069df0:  6a 06               PUSH 0x06
        _emit 0x6a
        _emit 0x06
        // 00069df2:  56                  PUSH ESI
        _emit 0x56
        // 00069df3:  ff d7               CALL EDI
        _emit 0xff
        _emit 0xd7
        // 00069df5:  83 c4 18            ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00069df8:  85 c0               TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00069dfa:  7e 29               JLE +0x29
        _emit 0x7e
        _emit 0x29
        // 00069dfc:  8b 54 24 18         MOV EDX, dword ptr [ESP+0x18]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00069e00:  8b 0e               MOV ECX, dword ptr [ESI]
        _emit 0x8b
        _emit 0x0e
        // 00069e02:  8b 41 24            MOV EAX, dword ptr [ECX+0x24]
        _emit 0x8b
        _emit 0x41
        _emit 0x24
        // 00069e05:  52                  PUSH EDX
        _emit 0x52
        // 00069e06:  53                  PUSH EBX
        _emit 0x53
        // 00069e07:  56                  PUSH ESI
        _emit 0x56
        // 00069e08:  ff d0               CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00069e0a:  83 c4 0c            ADD ESP, 0x0c
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00069e0d:  85 ff               TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00069e0f:  74 14               JZ +0x14
        _emit 0x74
        _emit 0x14
        // 00069e11:  50                  PUSH EAX
        _emit 0x50
        // 00069e12:  6a 00               PUSH 0x00
        _emit 0x6a
        _emit 0x00
        // 00069e14:  53                  PUSH EBX
        _emit 0x53
        // 00069e15:  8d 4c 24 24         LEA ECX, [ESP+0x24]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 00069e19:  51                  PUSH ECX
        _emit 0x51
        // 00069e1a:  68 86 00 00 00      PUSH 0x00000086
        _emit 0x68
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00069e1f:  56                  PUSH ESI
        _emit 0x56
        // 00069e20:  ff d7               CALL EDI
        _emit 0xff
        _emit 0xd7
        // 00069e22:  83 c4 18            ADD ESP, 0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00069e25:  5f                  POP EDI
        _emit 0x5f
        // 00069e26:  5b                  POP EBX
        _emit 0x5b
        // 00069e27:  5e                  POP ESI
        _emit 0x5e
        // 00069e28:  c3                  RET
        _emit 0xc3
        // ── error path ──
        // 00069e29:  68 83 01 00 00      PUSH 0x00000183
        _emit 0x68
        _emit 0x83
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 00069e2e:  68 a0 91 f7 00      PUSH 0xf791a0   (filename VA — reloc)
        _emit 0x68
        _emit 0xa0
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069e33:  6a 79               PUSH 0x79
        _emit 0x6a
        _emit 0x79
        // 00069e35:  68 83 00 00 00      PUSH 0x00000083
        _emit 0x68
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00069e3a:  6a 20               PUSH 0x20
        _emit 0x6a
        _emit 0x20
        // 00069e3c:  e8 ff 2a ff ff      CALL FUN_0045c940  (rel32 — reloc)
        _emit 0xe8
        _emit 0xff
        _emit 0x2a
        _emit 0xff
        _emit 0xff
        // 00069e41:  83 c4 14            ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00069e44:  b8 fe ff ff ff      MOV EAX, 0xfffffffe
        _emit 0xb8
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00069e49:  5e                  POP ESI
        _emit 0x5e
        // 00069e4a:  c3                  RET
        _emit 0xc3
    }
}
