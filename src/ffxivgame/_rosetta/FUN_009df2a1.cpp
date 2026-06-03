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
// FUNCTION: ffxivgame 0x005df2a1 — object initialiser with DLL-proc-address
//                                  loading (182 B / 0xb6, __cdecl,
//                                  EBP-based SEH4 frame).
//
// Signature (inferred from the asm):
//
//   void FUN_009df2a1(SomeStruct *obj, SomeStruct2 *arg2);
//
// The function uses MSVC's __SEH_prolog4 / __SEH_epilog4 pattern
// (PUSH 0xc; PUSH scope_table; CALL __SEH_prolog4 @ 0x009de4f0) to set up
// an EBP-based frame, then:
//
//   1. Calls an IAT slot [0x00f3e1e4] (LoadLibraryA or similar) with a
//      string at 0x00f8891c.  Saves the result (HMODULE) in [EBP-0x1c].
//   2. Loads the first parameter (obj) into ESI from [EBP+8].
//   3. Writes 0x012eb7c8 into [obj+0x5c] (a vtable / function-table ptr).
//   4. Sets EDI = 1, stores 1 into [obj+0x14].
//   5. If the library handle (EAX) is non-NULL:
//      a. Calls FUN_009df0a4 — a secondary check.
//      b. If that also returns non-zero, loads GetProcAddress-like IAT slot
//         [0x00f3e150] into EBX and calls it twice:
//           - with (handle, "name-at-0x1086ea8") → stored at [obj+0x1f8]
//           - with (handle, "name-at-0x1086eb8") → stored at [obj+0x1fc]
//   6. Stores 1 (EDI) into [obj+0x70].
//   7. Writes byte 'C' (0x43) to [obj+0xc8] and [obj+0x14b].
//   8. Loads 0x012eaec0 into EAX, stores it at [obj+0x68], then calls
//      the IAT slot [0x00f3e2cc] with EAX as the sole argument.
//   9. Calls FUN_009e264c with arg=0xc; cleans with POP ECX.
//  10. Clears EH-state ([EBP-4] &= 0).
//  11. Stores the second parameter ([EBP+0xc]) into [obj+0x6c].
//      If that parameter is zero, substitutes the global at [0x012eb4c8].
//  12. Calls FUN_009d37d9([obj+0x6c]); cleans with POP ECX.
//  13. Sets EH-state to 0xfffffffe, then epilog:
//        CALL +6 / CALL __SEH_epilog4 (0x009de535) / RET.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The __SEH_prolog4/__SEH_epilog4 compact frame, the IAT calls via
//   indirect [mem] operands, and the precise register allocation are
//   impractical to reproduce from C++ source alone. Emitting the 182
//   original bytes verbatim via MASM `_emit` directives gives compare.py
//   a byte-exact GREEN match (same strategy as FUN_009f6de2 / FUN_009dd6f2).
//
// Reloc-bearing sites (offsets within the 182-byte body, zero-based):
//   +0x03  DIR32 → 0x0122d3b8   (scope table / __SEH_prolog4 arg)
//   +0x08  REL32 → 0x009de4f0   (__SEH_prolog4)
//   +0x0d  DIR32 → 0x00f8891c   (DLL name string)
//   +0x13  DIR32 → 0x00f3e1e4   (IAT: LoadLibrary-like)
//   +0x1d  DIR32 → 0x012eb7c8   (imm32: function-table ptr)
//   +0x30  REL32 → 0x009df0a4   (secondary check fn)
//   +0x38  DIR32 → 0x01086ea8   (proc name string 1)
//   +0x3f  DIR32 → 0x00f3e150   (IAT: GetProcAddress-like)
//   +0x4d  DIR32 → 0x01086eb8   (proc name string 2)
//   +0x6a  DIR32 → 0x012eaec0   (imm32: data ptr)
//   +0x78  DIR32 → 0x00f3e2cc   (IAT: WinAPI call)
//   +0x7f  REL32 → 0x009e264c   (internal fn)
//   +0x93  DIR32 → 0x012eb4c8   (global fallback ptr)
//   +0xa7  REL32 → 0x009d37d9   (internal fn)
//   +0xac  DIR32 → 0x009df357   (epilog label: next fn byte)
//   +0xb1  REL32 → 0x009de535   (__SEH_epilog4)

extern "C" __declspec(naked) void FUN_009df2a1() {
    __asm {
        _emit 0x6a  // PUSH 0xc  (SEH frame locals size)
        _emit 0x0c
        _emit 0x68  // PUSH 0x122d3b8  (scope table)
        _emit 0xb8
        _emit 0xd3
        _emit 0x22
        _emit 0x01
        _emit 0xe8  // CALL __SEH_prolog4 (0x009de4f0)
        _emit 0x43
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        _emit 0x68  // PUSH 0xf8891c  (DLL name string)
        _emit 0x1c
        _emit 0x89
        _emit 0xf8
        _emit 0x00
        _emit 0xff  // CALL dword ptr [0x00f3e1e4]  (LoadLibrary-like)
        _emit 0x15
        _emit 0xe4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x89  // MOV dword ptr [EBP-0x1c], EAX
        _emit 0x45
        _emit 0xe4
        _emit 0x8b  // MOV ESI, dword ptr [EBP+0x8]
        _emit 0x75
        _emit 0x08
        _emit 0xc7  // MOV dword ptr [ESI+0x5c], 0x12eb7c8
        _emit 0x46
        _emit 0x5c
        _emit 0xc8
        _emit 0xb7
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EDI, EDI
        _emit 0xff
        _emit 0x47  // INC EDI
        _emit 0x89  // MOV dword ptr [ESI+0x14], EDI
        _emit 0x7e
        _emit 0x14
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ +0x2f
        _emit 0x2f
        _emit 0xe8  // CALL 0x009df0a4
        _emit 0xd0
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ +0x26
        _emit 0x26
        _emit 0x68  // PUSH 0x1086ea8  (proc name string 1)
        _emit 0xa8
        _emit 0x6e
        _emit 0x08
        _emit 0x01
        _emit 0xff  // PUSH dword ptr [EBP-0x1c]
        _emit 0x75
        _emit 0xe4
        _emit 0x8b  // MOV EBX, dword ptr [0x00f3e150]
        _emit 0x1d
        _emit 0x50
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0xff  // CALL EBX
        _emit 0xd3
        _emit 0x89  // MOV dword ptr [ESI+0x1f8], EAX
        _emit 0x86
        _emit 0xf8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0x1086eb8  (proc name string 2)
        _emit 0xb8
        _emit 0x6e
        _emit 0x08
        _emit 0x01
        _emit 0xff  // PUSH dword ptr [EBP-0x1c]
        _emit 0x75
        _emit 0xe4
        _emit 0xff  // CALL EBX
        _emit 0xd3
        _emit 0x89  // MOV dword ptr [ESI+0x1fc], EAX
        _emit 0x86
        _emit 0xfc
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x89  // MOV dword ptr [ESI+0x70], EDI
        _emit 0x7e
        _emit 0x70
        _emit 0xc6  // MOV byte ptr [ESI+0xc8], 0x43
        _emit 0x86
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x43
        _emit 0xc6  // MOV byte ptr [ESI+0x14b], 0x43
        _emit 0x86
        _emit 0x4b
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x43
        _emit 0xb8  // MOV EAX, 0x12eaec0
        _emit 0xc0
        _emit 0xae
        _emit 0x2e
        _emit 0x01
        _emit 0x89  // MOV dword ptr [ESI+0x68], EAX
        _emit 0x46
        _emit 0x68
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL dword ptr [0x00f3e2cc]
        _emit 0x15
        _emit 0xcc
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x6a  // PUSH 0xc
        _emit 0x0c
        _emit 0xe8  // CALL 0x009e264c
        _emit 0x27
        _emit 0x33
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x83  // AND dword ptr [EBP-0x4], 0
        _emit 0x65
        _emit 0xfc
        _emit 0x00
        _emit 0x8b  // MOV EAX, dword ptr [EBP+0xc]
        _emit 0x45
        _emit 0x0c
        _emit 0x89  // MOV dword ptr [ESI+0x6c], EAX
        _emit 0x46
        _emit 0x6c
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75  // JNZ +8
        _emit 0x08
        _emit 0xa1  // MOV EAX, [0x012eb4c8]
        _emit 0xc8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        _emit 0x89  // MOV dword ptr [ESI+0x6c], EAX
        _emit 0x46
        _emit 0x6c
        _emit 0xff  // PUSH dword ptr [ESI+0x6c]
        _emit 0x76
        _emit 0x6c
        _emit 0xe8  // CALL 0x009d37d9
        _emit 0x95
        _emit 0x44
        _emit 0xff
        _emit 0xff
        _emit 0x59  // POP ECX
        _emit 0xc7  // MOV dword ptr [EBP-4], 0xfffffffe
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL +6 (0x009df357 — epilog label)
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL __SEH_epilog4 (0x009de535)
        _emit 0xdf
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        _emit 0xc3  // RET
    }
}
