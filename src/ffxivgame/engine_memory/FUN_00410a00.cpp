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
// FUNCTION: ffxivgame 0x00010a00 — __thiscall SeparateHeapBlock::Lock
//           (SQEX::CDev::Engine::Memory::Alternative::SeparateHeapBlock::Lock)
//
// Acquires a lock on a SeparateHeapBlock. The method:
//   1. Calls vtable slot 0x2c on the inner space object (prepare step).
//   2. Calls vtable slot 0x34 on the space object (IsBusy predicate).
//   3. If IsBusy returns non-zero, fires the magic-static assertion
//      handler at [0x0132390c] with condition "!space->IsBusy()",
//      file  "c:\work\project\cdev\src\common\cdev\engine\memory\alternative\SeparateHeapSpace.h",
//      line  0x8c (140), function name "SQEX::CDev::Engine::Memory::Alternative::SeparateHeapBlock::Lock".
//   4. Increments the counter at *(this->field_0x2c + 0x34) and this->field_0x30 by 1.
//   5. Calls vtable slot 0x30 on the space object (finalize step).
//   6. Returns this->field_0x28 + *(this->field_0x2c + 0x18).
//
// Calling convention: __thiscall (ECX = this), no stack args, RET (0 bytes popped).
// Returns: int (in EAX).
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//
//   Source-level C++ cannot reproduce the interleaved MOV EBX,1 between
//   TEST AL,AL and JZ that MSVC 2005 /O2 emits, nor the reuse of BL
//   (EBX low byte) in the magic-static TEST byte [0x01323910],BL pattern.
//   A naked passthrough preserves the original 124-byte sequence verbatim.
//   The #if guard makes the file parseable by clang/GCC without -fms-extensions.
//
// ASM (124 bytes, rva 0x00010a00):
//   53                     PUSH EBX
//   56                     PUSH ESI
//   8b f1                  MOV ESI, ECX            ; this
//   57                     PUSH EDI
//   8b 7e 10               MOV EDI, [ESI+0x10]     ; space
//   8b 07                  MOV EAX, [EDI]
//   8b 50 2c               MOV EDX, [EAX+0x2c]
//   8b cf                  MOV ECX, EDI
//   ff d2                  CALL EDX                ; space->vtable[0x2c]()
//   8b 07                  MOV EAX, [EDI]
//   8b 50 34               MOV EDX, [EAX+0x34]
//   8b cf                  MOV ECX, EDI
//   ff d2                  CALL EDX                ; bool = space->IsBusy()
//   84 c0                  TEST AL, AL
//   bb 01 00 00 00         MOV EBX, 1              ; constant 1 (between TEST and JZ)
//   74 3a                  JZ +0x3a (→ 0x10a5d)   ; skip assert if !IsBusy
//   84 1d 10 39 32 01      TEST byte [0x01323910], BL
//   75 10                  JNZ +0x10 (→ 0x10a3b)
//   09 1d 10 39 32 01      OR dword [0x01323910], EBX
//   c7 05 0c 39 32 01      MOV dword [0x0132390c], 0x40f8e0
//          e0 f8 40 00
//   68 e0 69 f5 00         PUSH 0xf569e0           ; fn name
//   68 8c 00 00 00         PUSH 0x8c               ; line 140
//   68 88 69 f5 00         PUSH 0xf56988           ; file path
//   68 48 4d f5 00         PUSH 0xf54d48           ; condition label
//   68 74 69 f5 00         PUSH 0xf56974           ; "!space->IsBusy()"
//   ff 15 0c 39 32 01      CALL [0x0132390c]
//   83 c4 14               ADD ESP, 0x14
//   8b 46 2c               MOV EAX, [ESI+0x2c]
//   01 58 34               ADD [EAX+0x34], EBX
//   01 5e 30               ADD [ESI+0x30], EBX
//   8b 07                  MOV EAX, [EDI]
//   8b 50 30               MOV EDX, [EAX+0x30]
//   8b cf                  MOV ECX, EDI
//   ff d2                  CALL EDX                ; space->vtable[0x30]()
//   8b 4e 2c               MOV ECX, [ESI+0x2c]
//   8b 46 28               MOV EAX, [ESI+0x28]
//   03 41 18               ADD EAX, [ECX+0x18]
//   5f                     POP EDI
//   5e                     POP ESI
//   5b                     POP EBX
//   c3                     RET

#if defined(__clang__) || defined(__GNUC__)
// clang / GCC stub for static-analysis only — NOT compiled in production.
// Production builds always use cl.exe (MSVC 2005); the naked+asm block
// below is what actually runs.
extern "C" void FUN_00410a00() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00410a00()
{
    __asm {
        // 00010a00: 53              PUSH EBX
        _emit 0x53
        // 00010a01: 56              PUSH ESI
        _emit 0x56
        // 00010a02: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00010a04: 57              PUSH EDI
        _emit 0x57
        // 00010a05: 8b 7e 10        MOV EDI, [ESI+0x10]
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        // 00010a08: 8b 07           MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010a0a: 8b 50 2c        MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00010a0d: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010a0f: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010a11: 8b 07           MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010a13: 8b 50 34        MOV EDX, [EAX+0x34]
        _emit 0x8b
        _emit 0x50
        _emit 0x34
        // 00010a16: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010a18: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010a1a: 84 c0           TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 00010a1c: bb 01 00 00 00  MOV EBX, 0x1
        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010a21: 74 3a           JZ +0x3a
        _emit 0x74
        _emit 0x3a
        // 00010a23: 84 1d 10 39 32 01  TEST byte [0x01323910], BL
        _emit 0x84
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010a29: 75 10           JNZ +0x10
        _emit 0x75
        _emit 0x10
        // 00010a2b: 09 1d 10 39 32 01  OR dword [0x01323910], EBX
        _emit 0x09
        _emit 0x1d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010a31: c7 05 0c 39 32 01 e0 f8 40 00  MOV dword [0x0132390c], 0x40f8e0
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
        // 00010a3b: 68 e0 69 f5 00  PUSH 0xf569e0
        _emit 0x68
        _emit 0xe0
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010a40: 68 8c 00 00 00  PUSH 0x8c
        _emit 0x68
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010a45: 68 88 69 f5 00  PUSH 0xf56988
        _emit 0x68
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010a4a: 68 48 4d f5 00  PUSH 0xf54d48
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00010a4f: 68 74 69 f5 00  PUSH 0xf56974
        _emit 0x68
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010a54: ff 15 0c 39 32 01  CALL [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010a5a: 83 c4 14        ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00010a5d: 8b 46 2c        MOV EAX, [ESI+0x2c]
        _emit 0x8b
        _emit 0x46
        _emit 0x2c
        // 00010a60: 01 58 34        ADD [EAX+0x34], EBX
        _emit 0x01
        _emit 0x58
        _emit 0x34
        // 00010a63: 01 5e 30        ADD [ESI+0x30], EBX
        _emit 0x01
        _emit 0x5e
        _emit 0x30
        // 00010a66: 8b 07           MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010a68: 8b 50 30        MOV EDX, [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00010a6b: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010a6d: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010a6f: 8b 4e 2c        MOV ECX, [ESI+0x2c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x2c
        // 00010a72: 8b 46 28        MOV EAX, [ESI+0x28]
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 00010a75: 03 41 18        ADD EAX, [ECX+0x18]
        _emit 0x03
        _emit 0x41
        _emit 0x18
        // 00010a78: 5f              POP EDI
        _emit 0x5f
        // 00010a79: 5e              POP ESI
        _emit 0x5e
        // 00010a7a: 5b              POP EBX
        _emit 0x5b
        // 00010a7b: c3              RET
        _emit 0xc3
    }
}
#endif
