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
// FUNCTION: ffxivgame 0x004178c0 — `__thiscall` container initialise-and-push
//                                   (175 B / 0xaf)
//
// void __thiscall FUN_004178c0(this /*ECX*/, arg1 /*[ESP+0x4 on entry]*/)
//   → RET 4  (one caller-cleaned stack argument)
//
// Stack frame after prologue (SUB ESP,8 / PUSH ESI):
//   [ESP + 0x00]  saved ESI
//   [ESP + 0x04]  local scratch low  (filled by FUN_009d04ac return)
//   [ESP + 0x08]  local scratch high (filled later as second local slot)
//   [ESP + 0x0C]  return address
//   [ESP + 0x10]  arg1
//
// After PUSH EDI (saves EDI):
//   [ESP + 0x00]  saved EDI
//   [ESP + 0x04]  saved ESI
//   [ESP + 0x08..0x0C] 8-byte scratch
//   [ESP + 0x14]  arg1
//
// Behaviour:
//   1. Compute element count: begin=[this+4]; if begin==0 count=0 else
//      count = (this->m8 - begin) >> 3.
//   2. If count != 0 trigger a one-shot debug assert (sets flag at
//      [0x01323910], installs handler fn-ptr [0x0132390c]=0x417760,
//      then calls [0x0132390c] with five args: str, 0x2c, str, str, str).
//   3. PUSH EDI (save); PUSH 5; call FUN_00417e20 (__thiscall reserve(5)).
//   4. Load arg1 into EDI from [ESP+0x14].
//   5. Call FUN_009d04ac(arg1) → store result in local scratch.
//   6. Store arg1 into second local slot, call FUN_00417fc0(__thiscall push).
//   7. Assert container now non-empty (re-check begin/count; call
//      FUN_009d22b4 if the guard fails).
//   8. Set this->m14 = this->m4 (cursor = begin); this->m18 = 0 (index=0).
//   9. POP ESI; ADD ESP,8; RET 4.
//
// This is structurally the "initialise + first-push + reset-cursor" wrapper
// around FUN_004176b0's cursor-reset tail.  The sibling FUN_00417fc0 (the
// inner vector-push helper) and FUN_004176b0 (cursor reset, 42 B) provide
// the types in context.
//
// Reloc-bearing sites (masked by compare.py — these 4-byte windows are
// wildcarded):
//   +0x23   DIR32  data [0x01323910]   (assert-flag byte)
//   +0x2b   DIR32  data [0x01323910]   (same, OR-write)
//   +0x32   DIR32  data [0x0132390c]   (assert fn-ptr)
//   +0x36   DIR32  code 0x00417760     (assert handler pushed as imm32)
//   +0x3b   DIR32  str  0x00f57920     (assert arg 5)
//   +0x42   DIR32  str  0x00f57970     (assert arg 3)
//   +0x47   DIR32  str  0x00f5796f     (assert arg 2)
//   +0x4c   DIR32  str  0x00f579c8     (assert arg 1)
//   +0x52   DIR32  data [0x0132390c]   (indirect CALL through fn-ptr)
//   +0x5f   REL32  code FUN_00417e20   (reserve)
//   +0x69   REL32  code FUN_009d04ac   (convert)
//   +0x80   REL32  code FUN_00417fc0   (push)
//   +0x97   REL32  code FUN_009d22b4   (out_of_range thunk)

extern "C" __declspec(naked) void FUN_004178c0() {
    __asm {
        // 000178c0: 83 ec 08     SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 000178c3: 56           PUSH ESI
        _emit 0x56
        // 000178c4: 8b f1        MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 000178c6: 8b 4e 04     MOV ECX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 000178c9: 85 c9        TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 000178cb: 75 04        JNZ +4  (→ 0x004178d1)
        _emit 0x75
        _emit 0x04
        // 000178cd: 33 c0        XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000178cf: eb 08        JMP +8  (→ 0x004178d9)
        _emit 0xeb
        _emit 0x08
        // 000178d1: 8b 46 08     MOV EAX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 000178d4: 2b c1        SUB EAX, ECX
        _emit 0x2b
        _emit 0xc1
        // 000178d6: c1 f8 03     SAR EAX, 0x3
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 000178d9: 85 c0        TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000178db: 74 3c        JZ +0x3c  (→ 0x00417919)
        _emit 0x74
        _emit 0x3c
        // 000178dd: b8 01 00 00 00  MOV EAX, 0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000178e2: 84 05 10 39 32 01  TEST byte ptr [0x01323910], AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 000178e8: 75 10        JNZ +0x10  (→ 0x004178fa)
        _emit 0x75
        _emit 0x10
        // 000178ea: 09 05 10 39 32 01  OR dword ptr [0x01323910], EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 000178f0: c7 05 0c 39 32 01 60 77 41 00
        //           MOV dword ptr [0x0132390c], 0x417760
        _emit 0xc7
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x60
        _emit 0x77
        _emit 0x41
        _emit 0x00
        // 000178fa: 68 20 79 f5 00  PUSH 0xf57920
        _emit 0x68
        _emit 0x20
        _emit 0x79
        _emit 0xf5
        _emit 0x00
        // 000178ff: 6a 2c        PUSH 0x2c
        _emit 0x6a
        _emit 0x2c
        // 00017901: 68 70 79 f5 00  PUSH 0xf57970
        _emit 0x68
        _emit 0x70
        _emit 0x79
        _emit 0xf5
        _emit 0x00
        // 00017906: 68 6f 79 f5 00  PUSH 0xf5796f
        _emit 0x68
        _emit 0x6f
        _emit 0x79
        _emit 0xf5
        _emit 0x00
        // 0001790b: 68 c8 79 f5 00  PUSH 0xf579c8
        _emit 0x68
        _emit 0xc8
        _emit 0x79
        _emit 0xf5
        _emit 0x00
        // 00017910: ff 15 0c 39 32 01  CALL dword ptr [0x0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00017916: 83 c4 14     ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00017919: 57           PUSH EDI
        _emit 0x57
        // 0001791a: 6a 05        PUSH 0x5
        _emit 0x6a
        _emit 0x05
        // 0001791c: 8b ce        MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001791e: e8 fd 04 00 00  CALL FUN_00417e20 (rel32)
        _emit 0xe8
        _emit 0xfd
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00017923: 8b 7c 24 14  MOV EDI, dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 00017927: 57           PUSH EDI
        _emit 0x57
        // 00017928: e8 7f 8b 5b 00  CALL FUN_009d04ac (rel32)
        _emit 0xe8
        _emit 0x7f
        _emit 0x8b
        _emit 0x5b
        _emit 0x00
        // 0001792d: 89 44 24 0c  MOV dword ptr [ESP+0xc], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00017931: 83 c4 04     ADD ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00017934: 8d 44 24 08  LEA EAX, [ESP+0x8]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00017938: 50           PUSH EAX
        _emit 0x50
        // 00017939: 8b ce        MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001793b: 89 7c 24 10  MOV dword ptr [ESP+0x10], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0001793f: e8 7c 06 00 00  CALL FUN_00417fc0 (rel32)
        _emit 0xe8
        _emit 0x7c
        _emit 0x06
        _emit 0x00
        _emit 0x00
        // 00017944: 8b 46 04     MOV EAX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00017947: 85 c0        TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00017949: 5f           POP EDI
        _emit 0x5f
        // 0001794a: 74 0a        JZ +0xa  (→ 0x00417956)
        _emit 0x74
        _emit 0x0a
        // 0001794c: 8b 4e 08     MOV ECX, dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x4e
        _emit 0x08
        // 0001794f: 2b c8        SUB ECX, EAX
        _emit 0x2b
        _emit 0xc8
        // 00017951: c1 f9 03     SAR ECX, 0x3
        _emit 0xc1
        _emit 0xf9
        _emit 0x03
        // 00017954: 75 05        JNZ +5  (→ 0x0041795b)
        _emit 0x75
        _emit 0x05
        // 00017956: e8 59 a9 5b 00  CALL FUN_009d22b4 (out_of_range thunk, rel32)
        _emit 0xe8
        _emit 0x59
        _emit 0xa9
        _emit 0x5b
        _emit 0x00
        // 0001795b: 8b 56 04     MOV EDX, dword ptr [ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0001795e: 89 56 14     MOV dword ptr [ESI+0x14], EDX
        _emit 0x89
        _emit 0x56
        _emit 0x14
        // 00017961: c7 46 18 00 00 00 00  MOV dword ptr [ESI+0x18], 0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017968: 5e           POP ESI
        _emit 0x5e
        // 00017969: 83 c4 08     ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001796c: c2 04 00     RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
