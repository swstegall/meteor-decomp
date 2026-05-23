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
// FUNCTION: ffxivgame 0x00410a80 — SeparateHeapBlock::Unlock (115 bytes / 0x73)
//           __thiscall, no stack args
//
// ECX = this (a SeparateHeapBlock or similar heap-management object).
// this->field_0x10 is a pointer to a SeparateHeapSpace object (with a vtable).
// this->field_0x2c is a pointer to an associated object whose field_0x34
//   is a reference count.
// this->field_0x30 is a local reference count on this object.
//
// Behaviour:
//   space = this->field_0x10
//   space->vtable[0x2c](space)    ; e.g. Lock() — take the lock
//   ok = space->vtable[0x34](space)  ; e.g. IsBusy() — check state
//   if (ok) {                     ; assert(!space->IsBusy())
//       if (!(g_assert_flag & 1)) {
//           g_assert_flag |= 1;
//           g_assert_fn = FUN_0040f8e0;
//       }
//       g_assert_fn("!space->IsBusy()", &DAT_00f54d48,
//           "c:\\work\\project\\cdev\\src\\common\\cdev\\engine\\memory"
//           "\\alternative\\SeparateHeapSpace.h",
//           0x97,
//           "SQEX::CDev::Engine::Memory::Alternative::SeparateHeapBlock::Unlock");
//   }
//   *(this->field_0x2c + 0x34) -= 1;
//   this->field_0x30           -= 1;
//   // tail-call: space->vtable[0x30](space)   ; e.g. Unlock()
//
// Reconstruction: naked-asm byte passthrough.
//
// Root cause of non-reproducibility from plain C++:
//   1. Absolute data-section addresses (g_assert_flag at 0x01323910,
//      g_assert_fn at 0x0132390c) appear as immediate operands and cannot
//      be encoded as COFF relocations from standalone compilation.
//   2. The decrement of two fields uses the MSVC idiom
//        OR  ECX, 0xFFFFFFFF   ; ECX = -1
//        ADD [mem], ECX
//      which a plain C++ "--" would lower differently without the right
//      register allocation context.
//   3. The final JMP EDX (tail call through vtable slot 0x30) requires
//      the callee-saves to already be popped before the jump — achievable
//      only in naked asm.
//
// Globals / constants referenced (all absolute VAs in the 1.23b image):
//   0x01323910  — one-time-init flag dword (low bit = initialised)
//   0x0132390c  — assert-handler function pointer slot
//   0x0040f8e0  — FUN_0040f8e0 (the assert handler installed on first call)
//   0x00f56974  — "!space->IsBusy()"
//   0x00f54d48  — &DAT_00f54d48 (condition/expression string, assert arg 2)
//   0x00f56988  — filename string (SeparateHeapSpace.h)
//   0x00f56a28  — function name string (SeparateHeapBlock::Unlock)

extern "C" __declspec(naked) void FUN_00410a80()
{
    __asm {
        // 00010a80:  56                     PUSH ESI
        _emit 0x56
        // 00010a81:  57                     PUSH EDI
        _emit 0x57
        // 00010a82:  8b f9                  MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 00010a84:  8b 77 10               MOV ESI,dword ptr [EDI+0x10]
        _emit 0x8b
        _emit 0x77
        _emit 0x10
        // 00010a87:  8b 06                  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00010a89:  8b 50 2c               MOV EDX,dword ptr [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00010a8c:  8b ce                  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00010a8e:  ff d2                  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010a90:  8b 06                  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00010a92:  8b 50 34               MOV EDX,dword ptr [EAX+0x34]
        _emit 0x8b
        _emit 0x50
        _emit 0x34
        // 00010a95:  8b ce                  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00010a97:  ff d2                  CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010a99:  84 c0                  TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 00010a9b:  74 3f                  JZ +0x3f (→ 0x410adc)
        _emit 0x74
        _emit 0x3f
        // 00010a9d:  b8 01 00 00 00         MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010aa2:  84 05 10 39 32 01      TEST byte ptr [0x01323910],AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010aa8:  75 10                  JNZ +0x10 (→ 0x410aba)
        _emit 0x75
        _emit 0x10
        // 00010aaa:  09 05 10 39 32 01      OR dword ptr [0x01323910],EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010ab0:  c7 05 0c 39 32 01 e0 f8 40 00  MOV dword ptr [0x0132390c],0x0040f8e0
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0
        _emit 0xf8
        _emit 0x40
        _emit 0x00
        // 00010aba:  68 28 6a f5 00         PUSH 0x00f56a28  (function name)
        _emit 0x68
        _emit 0x28
        _emit 0x6a
        _emit 0xf5
        _emit 0x00
        // 00010abf:  68 97 00 00 00         PUSH 0x97  (line number 151)
        _emit 0x68
        _emit 0x97
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010ac4:  68 88 69 f5 00         PUSH 0x00f56988  (filename)
        _emit 0x68
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010ac9:  68 48 4d f5 00         PUSH 0x00f54d48  (cond expression)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00010ace:  68 74 69 f5 00         PUSH 0x00f56974  ("!space->IsBusy()")
        _emit 0x68
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010ad3:  ff 15 0c 39 32 01      CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010ad9:  83 c4 14               ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00010adc:  8b 47 2c               MOV EAX,dword ptr [EDI+0x2c]
        _emit 0x8b
        _emit 0x47
        _emit 0x2c
        // 00010adf:  83 c9 ff               OR ECX,0xffffffff
        _emit 0x83
        _emit 0xc9
        _emit 0xff
        // 00010ae2:  01 48 34               ADD dword ptr [EAX+0x34],ECX
        _emit 0x01
        _emit 0x48
        _emit 0x34
        // 00010ae5:  01 4f 30               ADD dword ptr [EDI+0x30],ECX
        _emit 0x01
        _emit 0x4f
        _emit 0x30
        // 00010ae8:  8b 06                  MOV EAX,dword ptr [ESI]
        _emit 0x8b
        _emit 0x06
        // 00010aea:  8b 50 30               MOV EDX,dword ptr [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00010aed:  5f                     POP EDI
        _emit 0x5f
        // 00010aee:  8b ce                  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00010af0:  5e                     POP ESI
        _emit 0x5e
        // 00010af1:  ff e2                  JMP EDX
        _emit 0xff
        _emit 0xe2
    }
}
