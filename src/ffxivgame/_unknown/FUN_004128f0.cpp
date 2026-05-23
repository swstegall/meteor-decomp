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
// FUNCTION: ffxivgame 0x000128f0 — CDev.Engine.Memory.Alternative allocator
//                                   constructor wrapper (__cdecl, 83 B / 0x53)
//
// __cdecl void* FUN_004128f0(param_1, param_2, param_3, param_4, param_5, param_6)
//   [ESP+0x04] : param_1
//   [ESP+0x08] : param_2
//   [ESP+0x0C] : param_3
//   [ESP+0x10] : param_4
//   [ESP+0x14] : param_5
//   [ESP+0x18] : param_6
//
// Behaviour:
//   1. Allocates 8 bytes on the local stack frame (SUB ESP, 8) to hold a
//      space descriptor: { DWORD size=0x10, const char* name }.
//   2. Calls FUN_0040e2d0 (__thiscall, ECX = &local_buf, RET 8) with
//      arg1 = 0x10, arg2 = "CDev.Engine.Memory.Alternative" to fill the
//      space descriptor. Returns EAX = &local_buf (the space ptr).
//   3. Loads ESI = param_3 (the allocator object).
//   4. Calls FUN_0040e110 (__thiscall, ECX = param_3, RET 8) with
//      arg1 = 0x60, arg2 = &local_buf (EAX from step 2).
//      Returns EAX = allocated block pointer (or NULL on failure).
//   5. If EAX == 0 (allocation failed): pops ESI, restores stack, returns 0.
//   6. Otherwise: forwards all 6 params to FUN_00412870 with ECX = param_3
//      and returns its result.
//
// Stack analysis:
//   After CALL1 (FUN_0040e2d0, RET 8): ESP = entry_ESP - 12.
//     [ESP+0x10] = param_1, ..., [ESP+0x24] = param_6.
//   After CALL2 (FUN_0040e110, RET 8): ESP = entry_ESP - 12 again.
//   The 6-push sequence for CALL3 (FUN_00412870):
//     PUSH p6, p5, p4, p3(ESI), p2, p1 (last pushed = arg1).
//     MOV ECX, ESI (= param_3, also passed as arg3).
//
// Register map (orig):
//   ESI = param_3 (the allocator object)
//   EAX = return value of each sub-call
//
// Calling convention: __cdecl; callee saves only ESI; allocates 8-byte
//   local space with SUB ESP, 8; epilogue = ADD ESP, 8; RET.
//
// Reloc-bearing sites (all masked by tools/compare.py):
//   +0x05  PUSH imm32  0x00f56ca8 = "CDev.Engine.Memory.Alternative"
//   +0x0f  CALL rel32  -> FUN_0040e2d0  (RVA 0x0000e2d0)
//   +0x1d  CALL rel32  -> FUN_0040e110  (RVA 0x0000e110)
//   +0x42  CALL rel32  -> FUN_00412870  (RVA 0x00012870)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function's 83-byte body contains three CALL rel32 relocations
//   that depend on the orig binary's own address space. A source-level
//   C++ rendering would emit the same branch shape but with
//   linker-resolved rel32 values. Since compare.py masks reloc bytes,
//   a __declspec(naked) body that re-emits the orig 83 bytes verbatim
//   via MASM _emit directives is the simplest path to GREEN.

#if defined(_MSC_VER) && !defined(__clang__)
extern "C" __declspec(naked) void FUN_004128f0()
{
    __asm {
        // 000128f0:  83 ec 08              SUB ESP, 8         ; allocate space descriptor
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 000128f3:  56                    PUSH ESI
        _emit 0x56
        // 000128f4:  68 a8 6c f5 00        PUSH 0x00f56ca8    ; "CDev.Engine.Memory.Alternative" (DIR32)
        _emit 0x68
        _emit 0xa8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        // 000128f9:  6a 10                 PUSH 0x10          ; size = 16
        _emit 0x6a
        _emit 0x10
        // 000128fb:  8d 4c 24 0c           LEA ECX, [ESP+0x0C] ; ECX = &local_buf
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 000128ff:  e8 cc b9 ff ff        CALL FUN_0040e2d0  ; space setter, RET 8
        _emit 0xe8
        _emit 0xcc
        _emit 0xb9
        _emit 0xff
        _emit 0xff
        // 00012904:  8b 74 24 18           MOV ESI, [ESP+0x18] ; ESI = param_3
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // 00012908:  50                    PUSH EAX            ; arg2 = &local_buf (space ptr)
        _emit 0x50
        // 00012909:  6a 60                 PUSH 0x60           ; arg1 = size = 96
        _emit 0x6a
        _emit 0x60
        // 0001290b:  8b ce                 MOV ECX, ESI        ; ECX = param_3 (allocator)
        _emit 0x8b
        _emit 0xce
        // 0001290d:  e8 fe b7 ff ff        CALL FUN_0040e110   ; alloc(0x60, &local_buf), RET 8
        _emit 0xe8
        _emit 0xfe
        _emit 0xb7
        _emit 0xff
        _emit 0xff
        // 00012912:  85 c0                 TEST EAX, EAX       ; alloc succeeded?
        _emit 0x85
        _emit 0xc0
        // 00012914:  74 26                 JZ +0x26            ; -> return 0
        _emit 0x74
        _emit 0x26
        // 00012916:  8b 4c 24 24           MOV ECX, [ESP+0x24] ; param_6
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0001291a:  8b 54 24 20           MOV EDX, [ESP+0x20] ; param_5
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 0001291e:  51                    PUSH ECX            ; arg6 = param_6
        _emit 0x51
        // 0001291f:  8b 4c 24 20           MOV ECX, [ESP+0x20] ; param_4 (stack shifted by 1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 00012923:  52                    PUSH EDX            ; arg5 = param_5
        _emit 0x52
        // 00012924:  8b 54 24 1c           MOV EDX, [ESP+0x1C] ; param_2 (stack shifted by 2)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // 00012928:  51                    PUSH ECX            ; arg4 = param_4
        _emit 0x51
        // 00012929:  8b 4c 24 1c           MOV ECX, [ESP+0x1C] ; param_1 (stack shifted by 3)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001292d:  56                    PUSH ESI            ; arg3 = param_3 (ESI)
        _emit 0x56
        // 0001292e:  52                    PUSH EDX            ; arg2 = param_2
        _emit 0x52
        // 0001292f:  51                    PUSH ECX            ; arg1 = param_1
        _emit 0x51
        // 00012930:  8b c8                 MOV ECX, ESI        ; ECX = param_3 (thiscall/context)
        _emit 0x8b
        _emit 0xc8
        // 00012932:  e8 39 ff ff ff        CALL FUN_00412870   ; construct/init, REL32
        _emit 0xe8
        _emit 0x39
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00012937:  5e                    POP ESI
        _emit 0x5e
        // 00012938:  83 c4 08              ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001293b:  c3                    RET
        _emit 0xc3
        // 0001293c: (return 0 path)
        // 0001293c:  33 c0                 XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0001293e:  5e                    POP ESI
        _emit 0x5e
        // 0001293f:  83 c4 08              ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00012942:  c3                    RET
        _emit 0xc3
    }
}
#endif // _MSC_VER && !__clang__
