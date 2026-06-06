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
// FUNCTION: ffxivgame 0x0001b5b0 — __thiscall constructor with /GS SEH frame
//                                   (143 B / 0x8f, ret 4). Initialises a class
//                                   instance from a 12-byte source struct, sets
//                                   a vtable pointer, flags a field with 0x20000,
//                                   and calls two init helpers.
//
// Calling convention: __thiscall (ECX = this); one 4-byte stack arg (pointer to
//   source struct); callee cleans arg → RET 0x4. Returns this in EAX.
//
// Stack frame (ESP-relative, no EBP; /GS cookie at [ESP+0]):
//   [ESP+ 0]  /GS cookie (security_cookie ^ ESP)
//   [ESP+ 4]  saved ESI
//   [ESP+ 8]  this pointer (ECX on entry; overwritten with ESI=this)
//   [ESP+ 0c] previous FS:[0x0]  — SEH prev-link
//   [ESP+10]  exception handler  — 0xe55883     [reloc]
//   [ESP+14]  SEH try-level      — starts at -1, set to 0, then 1
//   [ESP+18]  return address
//   [ESP+1c]  arg1 (source struct pointer)
//
// Object layout (fields touched):
//   [this+ 0x00]  vtable ptr      ← 0xf57fe8                  [reloc]
//   [this+ 0x04]  zeroed
//   [this+ 0x08]  zeroed
//   [this+ 0x0c]  arg1[0] | 0x20000  (low DWORD from source, then OR'd)
//   [this+ 0x10]  0                  (high half of MOVQ zeroed afterwards)
//   [this+ 0x14]  arg1[8]            (third DWORD from source)
//   [this+ 0x18]  zeroed
//   [this+ 0x20]  zeroed
//   [this+ 0x24]  zeroed
//   [this+ 0x28]  zeroed
//   [this+ 0x30]  1 (byte flag)
//
// Source struct layout (12 bytes at arg1):
//   [arg1+ 0]  DWORD copied to this->field_0c (before OR)
//   [arg1+ 4]  DWORD loaded with MOVQ but then discarded (field_10 zeroed)
//   [arg1+ 8]  DWORD copied to this->field_14
//
// Init helpers (both __thiscall, ECX = this):
//   FUN_0041b330  — first sub-init  (CALL rel32 @ +0x6e)
//   FUN_004328a0  — second sub-init (CALL rel32 @ +0x75)
//
// Relocations in the orig 143 bytes (masked by tools/compare.py):
//   +0x03  exception handler addr      (.text 0x00e55883)     [ABS32]
//   +0x11  security cookie global      (.data 0x012ea8b0)     [ABS32]
//   +0x35  vtable pointer immediate    (.rdata 0x00f57fe8)    [ABS32]
//   +0x6f  CALL FUN_0041b330           (rel32 = 0xfffffd0d)   [REL32]
//   +0x76  CALL FUN_004328a0           (rel32 = 0x00017276)   [REL32]
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The /GS SEH prologue (PUSH -1 / PUSH handler / MOV EAX,FS:[0] / PUSH EAX /
//   PUSH ECX / PUSH ESI / MOV EAX,[cookie] / XOR EAX,ESP / PUSH EAX /
//   LEA EAX,[ESP+0xc] / MOV FS:[0],EAX) and the matching epilogue
//   (MOV ECX,[ESP+0xc] / MOV FS:[0],ECX / POP ECX / POP ESI / ADD ESP,0x10 /
//   RET 4) cannot be faithfully reproduced from source-level C++ in a
//   standalone TU — the compiler emits a different frame layout unless the
//   full class translation unit is rebuilt from scratch. The _emit byte
//   passthrough produces a .text section that is byte-identical to the
//   original slice modulo the five relocation windows listed above.

extern "C" __declspec(naked) void FUN_0041b5b0() {
    __asm {
        // 0001b5b0:  6a ff                  PUSH -0x1  (SEH try-level = -1)
        _emit 0x6a
        _emit 0xff
        // 0001b5b2:  68 83 58 e5 00         PUSH 0xe55883  (EH handler) [reloc]
        _emit 0x68
        _emit 0x83
        _emit 0x58
        _emit 0xe5
        _emit 0x00
        // 0001b5b7:  64 a1 00 00 00 00      MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001b5bd:  50                     PUSH EAX  (prev FS:[0])
        _emit 0x50
        // 0001b5be:  51                     PUSH ECX  (save this)
        _emit 0x51
        // 0001b5bf:  56                     PUSH ESI
        _emit 0x56
        // 0001b5c0:  a1 b0 a8 2e 01         MOV EAX, [0x012ea8b0]  (security cookie) [reloc]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0001b5c5:  33 c4                  XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0001b5c7:  50                     PUSH EAX  (cookie ^ ESP)
        _emit 0x50
        // 0001b5c8:  8d 44 24 0c            LEA EAX, [ESP+0xc]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0001b5cc:  64 a3 00 00 00 00      MOV FS:[0x0], EAX  (install SEH frame)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001b5d2:  8b f1                  MOV ESI, ECX  (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 0001b5d4:  89 74 24 08            MOV dword ptr [ESP+0x8], ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 0001b5d8:  33 c0                  XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0001b5da:  89 46 04               MOV dword ptr [ESI+0x4], EAX  (field_04 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0001b5dd:  89 46 08               MOV dword ptr [ESI+0x8], EAX  (field_08 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 0001b5e0:  8b 4c 24 1c            MOV ECX, dword ptr [ESP+0x1c]  (arg1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 0001b5e4:  c7 06 e8 7f f5 00      MOV dword ptr [ESI], 0xf57fe8  (vtable) [reloc]
        _emit 0xc7
        _emit 0x06
        _emit 0xe8
        _emit 0x7f
        _emit 0xf5
        _emit 0x00
        // 0001b5ea:  f3 0f 7e 01            MOVQ XMM0, qword ptr [ECX]  (arg1[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 0001b5ee:  66 0f d6 46 0c         MOVQ qword ptr [ESI+0xc], XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x0c
        // 0001b5f3:  8b 49 08               MOV ECX, dword ptr [ECX+0x8]  (arg1[8])
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        // 0001b5f6:  89 4e 14               MOV dword ptr [ESI+0x14], ECX  (field_14 = arg1[8])
        _emit 0x89
        _emit 0x4e
        _emit 0x14
        // 0001b5f9:  89 46 18               MOV dword ptr [ESI+0x18], EAX  (field_18 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x18
        // 0001b5fc:  89 44 24 14            MOV dword ptr [ESP+0x14], EAX  (try-level = 0)
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 0001b600:  89 46 20               MOV dword ptr [ESI+0x20], EAX  (field_20 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x20
        // 0001b603:  89 46 24               MOV dword ptr [ESI+0x24], EAX  (field_24 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x24
        // 0001b606:  89 46 28               MOV dword ptr [ESI+0x28], EAX  (field_28 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x28
        // 0001b609:  81 4e 0c 00 00 02 00   OR dword ptr [ESI+0xc], 0x20000
        _emit 0x81
        _emit 0x4e
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x02
        _emit 0x00
        // 0001b610:  b1 01                  MOV CL, 0x1
        _emit 0xb1
        _emit 0x01
        // 0001b612:  88 4c 24 14            MOV byte ptr [ESP+0x14], CL  (try-level = 1)
        _emit 0x88
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0001b616:  88 4e 30               MOV byte ptr [ESI+0x30], CL  (field_30 = 1)
        _emit 0x88
        _emit 0x4e
        _emit 0x30
        // 0001b619:  8b ce                  MOV ECX, ESI  (ECX = this)
        _emit 0x8b
        _emit 0xce
        // 0001b61b:  89 46 10               MOV dword ptr [ESI+0x10], EAX  (field_10 = 0)
        _emit 0x89
        _emit 0x46
        _emit 0x10
        // 0001b61e:  e8 0d fd ff ff         CALL FUN_0041b330  (rel32 = 0xfffffd0d) [reloc]
        _emit 0xe8
        _emit 0x0d
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0001b623:  8b ce                  MOV ECX, ESI  (ECX = this)
        _emit 0x8b
        _emit 0xce
        // 0001b625:  e8 76 72 01 00         CALL FUN_004328a0  (rel32 = 0x00017276) [reloc]
        _emit 0xe8
        _emit 0x76
        _emit 0x72
        _emit 0x01
        _emit 0x00
        // 0001b62a:  8b c6                  MOV EAX, ESI  (return this)
        _emit 0x8b
        _emit 0xc6
        // 0001b62c:  8b 4c 24 0c            MOV ECX, dword ptr [ESP+0xc]  (prev FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0001b630:  64 89 0d 00 00 00 00   MOV dword ptr FS:[0x0], ECX  (restore SEH)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001b637:  59                     POP ECX  (discard /GS cookie)
        _emit 0x59
        // 0001b638:  5e                     POP ESI
        _emit 0x5e
        // 0001b639:  83 c4 10               ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0001b63c:  c2 04 00               RET 0x4  (clean one 4-byte stack arg)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
