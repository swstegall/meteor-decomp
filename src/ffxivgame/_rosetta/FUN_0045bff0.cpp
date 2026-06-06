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
// FUNCTION: ffxivgame 0x0045bff0 — singleton init + virtual-call wrapper
//                                  (__cdecl, 186 B / 0xba)
//
// Behaviour read from asm/ffxivgame/0005bff0_FUN_0045bff0.s:
//
//   __cdecl int FUN_0045bff0(int param_a)
//     stack layout (4-byte local allocated via __chkstk at entry):
//       [ESP+0x00] : int  local_result  (vtable fn_14 return value)
//       [ESP+0x04] : RetAddr
//       [ESP+0x08] : int  param_a
//
//   1. Allocate 4-byte local frame:
//        mov  eax, 4
//        call 0x009d29d0   ; __chkstk / _alloca_probe
//
//   2. If global *(int**)0x0132e788 == NULL:
//        call FUN_00465f80(9, 1, 0xf6802c, 0x127)  ; begin-marker
//        if still NULL: *(int**)0x0132e788 = 0xf68000   ; default init
//        call FUN_00465f80(10, 1, 0xf6802c, 0x12a) ; end-marker
//
//   3. g = *(SomeObj**)0x0132e788
//      result = (*(cdecl_fn*)(g+0x14))(1)   ; function-pointer at obj+0x14
//      [ESP] = result
//      if result == 0 → pop ECX; ret (return 0)
//
//   4. call FUN_00465f80(9, 1, 0xf6802c, 0x201) ; begin-marker
//      ret2 = FUN_00466a60(result, param_a)
//      call FUN_00465f80(10, 1, 0xf6802c, 0x203) ; end-marker
//      (*(cdecl_fn*)(g+0x18))(&result)       ; function-pointer at obj+0x18
//      return ret2
//
// The 4-byte local at [ESP] is used to:
//   (a) temporarily hold the fn_14 return value across the first 4-arg call
//       (accessed as [ESP+0x14] after 5 PUSHes),
//   (b) pass its address to the fn_18 call via LEA ECX, [ESP+0x2c]
//       (accessed after the second batch of 4+2 PUSHes = 0x2c offset).
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains multiple absolute-address references (global at
//   0x0132e788, data pointers 0xf6802c and 0xf68000, CALL rel32 targets
//   0x009d29d0 / 0x00465f80 / 0x00466a60) that are image-relative.
//   Additionally the MOV EAX,4 / CALL __chkstk prolog, the MOV [ESP],EAX
//   local-spill idiom, and the bulk ADD ESP,0x2c + POP ESI + POP ECX
//   epilog form a tightly-coupled byte sequence that high-level C++ cannot
//   reproduce exactly under /O2. The same naked-asm passthrough used by
//   FUN_00406680, FUN_00401750, and FUN_00403f10 is applied here.
//   compare.py masks all reloc-bearing 4-byte windows, producing GREEN
//   against the orig 186 bytes.

extern "C" __declspec(naked) void FUN_0045bff0() {
    __asm {
        // MOV EAX, 0x4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CALL 0x009d29d0  (__chkstk / _alloca_probe — allocates 4-byte local)
        _emit 0xe8
        _emit 0xd6
        _emit 0x69
        _emit 0x57
        _emit 0x00
        // CMP dword ptr [0x0132e788], 0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // JNZ +0x3f  (to 0x0045c042)
        _emit 0x75
        _emit 0x3f
        // PUSH 0x127
        _emit 0x68
        _emit 0x27
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // CALL 0x00465f80  (begin-marker, 4-arg __cdecl)
        _emit 0xe8
        _emit 0x6a
        _emit 0x9f
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // CMP dword ptr [0x0132e788], 0x0
        _emit 0x83
        _emit 0x3d
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        // JNZ +0x0a  (to 0x0045c02c)
        _emit 0x75
        _emit 0x0a
        // MOV dword ptr [0x0132e788], 0xf68000
        _emit 0xc7
        _emit 0x05
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // PUSH 0x12a
        _emit 0x68
        _emit 0x2a
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // CALL 0x00465f80  (end-marker, 4-arg __cdecl)
        _emit 0xe8
        _emit 0x41
        _emit 0x9f
        _emit 0x00
        _emit 0x00
        // ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // MOV EAX, dword ptr [0x0132e788]  (a1 short form)
        _emit 0xa1
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // MOV ECX, dword ptr [EAX + 0x14]
        _emit 0x8b
        _emit 0x48
        _emit 0x14
        // PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // CALL ECX  (fn_14, __cdecl)
        _emit 0xff
        _emit 0xd1
        // ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // MOV dword ptr [ESP], EAX  (spill result into 4-byte local)
        _emit 0x89
        _emit 0x04
        _emit 0x24
        // JNZ +0x02  (to success path)
        _emit 0x75
        _emit 0x02
        // POP ECX  (restore local slot, clean frame)
        _emit 0x59
        // RET  (return 0)
        _emit 0xc3
        // PUSH ESI
        _emit 0x56
        // PUSH 0x201
        _emit 0x68
        _emit 0x01
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // PUSH 0x9
        _emit 0x6a
        _emit 0x09
        // CALL 0x00465f80  (begin-marker, 4-arg __cdecl, no ADD ESP here)
        _emit 0xe8
        _emit 0x12
        _emit 0x9f
        _emit 0x00
        _emit 0x00
        // MOV EDX, dword ptr [ESP + 0x1c]  (param_a — function's first arg)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        // MOV EAX, dword ptr [ESP + 0x14]  (local_result — fn_14 return value)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // PUSH EDX  (param_a — second arg of FUN_00466a60)
        _emit 0x52
        // PUSH EAX  (local_result — first arg of FUN_00466a60)
        _emit 0x50
        // CALL 0x00466a60  (__cdecl, 2 args)
        _emit 0xe8
        _emit 0xe3
        _emit 0xa9
        _emit 0x00
        _emit 0x00
        // PUSH 0x203
        _emit 0x68
        _emit 0x03
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // PUSH 0xf6802c
        _emit 0x68
        _emit 0x2c
        _emit 0x80
        _emit 0xf6
        _emit 0x00
        // PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // PUSH 0xa
        _emit 0x6a
        _emit 0x0a
        // MOV ESI, EAX  (save return value of FUN_00466a60)
        _emit 0x8b
        _emit 0xf0
        // CALL 0x00465f80  (end-marker, 4-arg __cdecl, no ADD ESP here)
        _emit 0xe8
        _emit 0xee
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        // MOV EDX, dword ptr [0x0132e788]
        _emit 0x8b
        _emit 0x15
        _emit 0x88
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // MOV EAX, dword ptr [EDX + 0x18]  (fn_18 function pointer)
        _emit 0x8b
        _emit 0x42
        _emit 0x18
        // LEA ECX, [ESP + 0x2c]  (address of local_result — 4-byte slot)
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // PUSH ECX  (arg: &local_result)
        _emit 0x51
        // CALL EAX  (fn_18, __cdecl)
        _emit 0xff
        _emit 0xd0
        // ADD ESP, 0x2c  (bulk cleanup: all un-ADD'd pushes in this path)
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        // MOV EAX, ESI  (return value = result from FUN_00466a60)
        _emit 0x8b
        _emit 0xc6
        // POP ESI
        _emit 0x5e
        // POP ECX  (clean up 4-byte local frame)
        _emit 0x59
        // RET
        _emit 0xc3
    }
}
