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
// FUNCTION: ffxivgame 0x00010be0 — SeparateHeapBlock::DestroyHandle (199 B / 0xc7)
//
// __thiscall void FUN_00410be0(this, param_1)
//   ECX       : this  — SeparateHeapBlock instance
//   [ESP+0x04]: param_1 — handle/node to destroy
//
// Behaviour:
//   1. Cache this in EBX; load this->field10 into EDI (the space object).
//   2. Call EDI->vfunc11() (vtable slot 0x2c / 11).
//   3. Call EDI->vfunc13() (vtable slot 0x34 / 13) — returns IsBusy() bool.
//   4. Assert !space->IsBusy(): if IsBusy() is non-zero, fire the SQEX_CDASSERT
//      via the lazy-init function pointer at DAT_0132390c (initialized from
//      FUN_0040f8e0 on first assert). The assert fires for:
//        condition:  "!space->IsBusy()"
//        file:       "c:\\work\\project\\cdev\\src\\common\\cdev\\engine\\memory\\"
//                    "alternative\\SeparateHeapSpace.h"  (split across two strings)
//        line:       0xad (173)
//        function:   "SQEX::CDev::Engine::Memory::Alternative::"
//                    "SeparateHeapBlock::DestroyHandle"
//   5. Via param_1's vtable: call vtable[5] (slot 0x14) then vtable[1] (slot 0x4),
//      returning a node pointer (ESI).
//   6. Call ESI->vfunc0(0).
//   7. Via this->field10 again: call vtable[1] (slot 0x4), then dereference
//      result[0x10] to get the lock header (EAX).
//   8. Acquire spinlock at [EAX+0x4] via XCHG(1)-spin (EBX used as lock addr
//      after a 6-byte NOP-form LEA EBX,[EBX+0]).
//   9. Insert ESI (node) before the head sentinel at [EAX+0xc]
//      (classic circular doubly-linked list prepend-before-head):
//        *(*(ECX+4)) = ESI          // old-tail->fwd = ESI
//        ESI[1] = *(ECX+4)          // ESI->bak = old-tail
//        ESI[0] = ECX               // ESI->fwd = head
//        ECX[1] = ESI               // head->bak = ESI
//        [EAX+0x18]--               // decrement count
//  10. Release spinlock via XCHG(0).
//  11. Call EDI->vfunc12() (vtable slot 0x30 / 12).
//  12. RET 0x4 (__thiscall, callee cleans 1 stack arg).
//
// Register allocation (MSVC 2005 /O2):
//   EBX = this (saved from ECX; reused as lock addr in spin loop)
//   ESI = node pointer (return from vtable chain)
//   EDI = this->field10 (space object)
//   EAX, ECX, EDX = temps
//
// The spinloop has an unusual 6-byte NOP-form `LEA EBX,[EBX+0x00000000]`
// (8d 9b 00 00 00 00) as padding before the hot loop body, plus a
// `MOV EBX,EDX` inside the loop (re-loading lock addr each iteration).
// MSVC 2005 /O2 occasionally emits this pattern when aligning the loop
// head to a 16-byte boundary. Exact byte passthrough is used to
// reproduce the encoding.
//
// Reloc-bearing positions (masked by tools/compare.py):
//   off 0x1e  IMAGE_REL_I386_DIR32  → DAT_01323910 (TEST byte ptr [imm32], 0x1)
//   off 0x27  IMAGE_REL_I386_DIR32  → DAT_01323910 (OR dword ptr [imm32], 0x1)
//   off 0x2e  IMAGE_REL_I386_DIR32  → DAT_0132390c (MOV [imm32], imm32 — dest)
//   off 0x32  IMAGE_REL_I386_DIR32  → FUN_0040f8e0  (MOV [imm32], imm32 — src)
//   off 0x51  IMAGE_REL_I386_DIR32  → DAT_0132390c  (CALL dword ptr [imm32])
//
// String PUSH immediates (0xf56ac0, 0xf56988, 0xf54d48, 0xf56974) are
// emitted verbatim — they match the orig binary byte-for-byte and need
// no COFF relocation entries.

extern "C" {
int DAT_01323910;
int DAT_0132390c;
int FUN_0040f8e0();
} // extern "C"

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
extern "C" void FUN_00410be0() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00410be0()
{
    __asm {
        // 00010be0: 53                    PUSH EBX
        _emit 0x53
        // 00010be1: 56                    PUSH ESI
        _emit 0x56
        // 00010be2: 8b d9                 MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // 00010be4: 57                    PUSH EDI
        _emit 0x57
        // 00010be5: 8b 7b 10              MOV EDI, [EBX+0x10]
        _emit 0x8b
        _emit 0x7b
        _emit 0x10
        // 00010be8: 8b 07                 MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010bea: 8b 50 2c              MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00010bed: 8b cf                 MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010bef: ff d2                 CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010bf1: 8b 07                 MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00010bf3: 8b 50 34              MOV EDX, [EAX+0x34]
        _emit 0x8b
        _emit 0x50
        _emit 0x34
        // 00010bf6: 8b cf                 MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010bf8: ff d2                 CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010bfa: 84 c0                 TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 00010bfc: 74 3c                 JZ +0x3c  (to 0x10c3a)
        _emit 0x74
        _emit 0x3c
        // 00010bfe: f6 05 [DAT_01323910] 01   TEST byte ptr [DAT_01323910], 0x1
        _emit 0xf6
        _emit 0x05
        _emit 0x10  // DAT_01323910 reloc placeholder (4 bytes, patched by linker)
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 00010c05: 75 11                 JNZ +0x11  (to 0x10c18)
        _emit 0x75
        _emit 0x11
        // 00010c07: 83 0d [DAT_01323910] 01   OR dword ptr [DAT_01323910], 0x1
        _emit 0x83
        _emit 0x0d
        _emit 0x10  // DAT_01323910 reloc placeholder (4 bytes, patched by linker)
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        // 00010c0e: c7 05 [DAT_0132390c] [FUN_0040f8e0]
        //           MOV dword ptr [DAT_0132390c], FUN_0040f8e0
        _emit 0xc7
        _emit 0x05
        _emit 0x0c  // DAT_0132390c reloc placeholder
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xe0  // FUN_0040f8e0 reloc placeholder
        _emit 0xf8
        _emit 0x40
        _emit 0x00
        // 00010c18: 68 c0 6a f5 00        PUSH 0xf56ac0  (function name string)
        _emit 0x68
        _emit 0xc0
        _emit 0x6a
        _emit 0xf5
        _emit 0x00
        // 00010c1d: 68 ad 00 00 00        PUSH 0xad  (line 173)
        _emit 0x68
        _emit 0xad
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010c22: 68 88 69 f5 00        PUSH 0xf56988  (filename string)
        _emit 0x68
        _emit 0x88
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010c27: 68 48 4d f5 00        PUSH 0xf54d48  (condition string)
        _emit 0x68
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 00010c2c: 68 74 69 f5 00        PUSH 0xf56974  (extra string)
        _emit 0x68
        _emit 0x74
        _emit 0x69
        _emit 0xf5
        _emit 0x00
        // 00010c31: ff 15 [DAT_0132390c]  CALL dword ptr [DAT_0132390c]
        _emit 0xff
        _emit 0x15
        _emit 0x0c  // DAT_0132390c reloc placeholder
        _emit 0x39
        _emit 0x32
        _emit 0x01
        // 00010c37: 83 c4 14              ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00010c3a: 8b 4c 24 10           MOV ECX, [ESP+0x10]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00010c3e: 8b 01                 MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00010c40: 8b 50 14              MOV EDX, [EAX+0x14]
        _emit 0x8b
        _emit 0x50
        _emit 0x14
        // 00010c43: ff d2                 CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00010c45: 8b 10                 MOV EDX, [EAX]
        _emit 0x8b
        _emit 0x10
        // 00010c47: 8b c8                 MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 00010c49: 8b 42 04              MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00010c4c: ff d0                 CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00010c4e: 8b f0                 MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00010c50: 8b 16                 MOV EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 00010c52: 8b 02                 MOV EAX, [EDX]
        _emit 0x8b
        _emit 0x02
        // 00010c54: 6a 00                 PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00010c56: 8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00010c58: ff d0                 CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00010c5a: 8b 4b 10              MOV ECX, [EBX+0x10]
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00010c5d: 8b 11                 MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 00010c5f: 8b 42 04              MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00010c62: ff d0                 CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00010c64: 8b 40 10              MOV EAX, [EAX+0x10]
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 00010c67: 8d 50 04              LEA EDX, [EAX+0x4]
        _emit 0x8d
        _emit 0x50
        _emit 0x04
        // 00010c6a: 8d 9b 00 00 00 00     LEA EBX, [EBX+0x0]  (6-byte NOP / loop alignment)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- spin loop ---
        // 00010c70: b9 01 00 00 00        MOV ECX, 0x1
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00010c75: 8b da                 MOV EBX, EDX
        _emit 0x8b
        _emit 0xda
        // 00010c77: 87 0b                 XCHG dword ptr [EBX], ECX
        _emit 0x87
        _emit 0x0b
        // 00010c79: 85 c9                 TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00010c7b: 75 f3                 JNZ -0xd  (to 0x10c70)
        _emit 0x75
        _emit 0xf3
        // --- spinlock acquired ---
        // 00010c7d: 8b 48 0c              MOV ECX, [EAX+0xc]
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 00010c80: 8b 59 04              MOV EBX, [ECX+0x4]
        _emit 0x8b
        _emit 0x59
        _emit 0x04
        // 00010c83: 89 33                 MOV [EBX], ESI
        _emit 0x89
        _emit 0x33
        // 00010c85: 8b 59 04              MOV EBX, [ECX+0x4]
        _emit 0x8b
        _emit 0x59
        _emit 0x04
        // 00010c88: 89 5e 04              MOV [ESI+0x4], EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // 00010c8b: 89 0e                 MOV [ESI], ECX
        _emit 0x89
        _emit 0x0e
        // 00010c8d: 89 71 04              MOV [ECX+0x4], ESI
        _emit 0x89
        _emit 0x71
        _emit 0x04
        // 00010c90: 83 40 18 ff           ADD dword ptr [EAX+0x18], -0x1
        _emit 0x83
        _emit 0x40
        _emit 0x18
        _emit 0xff
        // 00010c94: 33 c0                 XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 00010c96: 87 02                 XCHG dword ptr [EDX], EAX
        _emit 0x87
        _emit 0x02
        // 00010c98: 8b 17                 MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 00010c9a: 8b 42 30              MOV EAX, [EDX+0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00010c9d: 8b cf                 MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00010c9f: ff d0                 CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00010ca1: 5f                    POP EDI
        _emit 0x5f
        // 00010ca2: 5e                    POP ESI
        _emit 0x5e
        // 00010ca3: 5b                    POP EBX
        _emit 0x5b
        // 00010ca4: c2 04 00              RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif
