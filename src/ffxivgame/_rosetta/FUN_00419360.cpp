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
// FUNCTION: ffxivgame 0x00419360 — copy three 16-byte structs to global memory,
//                                   then dispatch via a global object's function
//                                   pointers (__cdecl, 162 bytes / 0xa2)
//
// __cdecl void FUN_00419360(void *arg1, void *arg2, void *arg3)
//     [ESP+0x04] = arg1  (pointer to a 16-byte struct: two qwords at +0, +8)
//     [ESP+0x08] = arg2  (pointer to a 16-byte struct: two qwords at +0, +8)
//     [ESP+0x0c] = arg3  (pointer to a 16-byte struct: two qwords at +0, +8)
//
// Behaviour:
//   1. Load arg1 into ECX; copy its two qwords (via MOVQ XMM0 / MOVQ m64,XMM0)
//      to globals [0x01328fac] and [0x01328fb4].
//   2. Load the global pointer at [0x01328db4] into EAX and TEST it.
//      (The TEST result is used later — JZ skips all callbacks.)
//   3. Copy arg2's two qwords to [0x01328fbc] and [0x01328fc4].
//   4. Copy arg3's two qwords to [0x01328fcc] and [0x01328fd4].
//   5. If [0x01328db4] == NULL: skip to epilogue.
//   6. If EAX->field_0x14 != NULL: call FUN_0041a7d0(__thiscall on EAX->field_0x4,
//        args: EAX->field_0x14, arg1-ptr). Reload EAX.
//   7. If EAX->field_0x18 != NULL: call FUN_0041a7d0(__thiscall on EAX->field_0x4,
//        args: EAX->field_0x18, arg2-ptr). Reload EAX.
//   8. If EAX->field_0x1c != NULL: call FUN_0041a7d0(__thiscall on EAX->field_0x4,
//        args: EAX->field_0x1c, arg3-ptr).
//   9. POP EDI, POP ESI, RET.
//
// Globals:
//   0x01328db4  g_dispatch_obj  — global pointer to dispatch struct
//   0x01328fac  g_slot0_lo      — copy target for arg1 qword 0
//   0x01328fb4  g_slot0_hi      — copy target for arg1 qword 1
//   0x01328fbc  g_slot1_lo      — copy target for arg2 qword 0
//   0x01328fc4  g_slot1_hi      — copy target for arg2 qword 1
//   0x01328fcc  g_slot2_lo      — copy target for arg3 qword 0
//   0x01328fd4  g_slot2_hi      — copy target for arg3 qword 1
//
// Calling convention: __cdecl (no RET N; caller cleans stack).
//   Callee-saved: ESI, EDI (PUSH/POP pair).
//   ECX is used as a scratch register (arg1 ptr) and is not saved.
//
// MOVQ XMM form: MSVC 2005 emits F3 0F 7E (MOVQ xmm, m64) and
//   66 0F D6 (MOVQ m64, xmm) for 8-byte non-FP copies.
//
// Reloc-bearing sites in the orig 162 bytes (masked by compare.py):
//   +0x08  MOV EAX,[imm32]  → 0x01328db4
//   +0x0f  MOVQ m64,XMM0   → 0x01328fac
//   +0x19  MOVQ m64,XMM0   → 0x01328fb4
//   +0x1d  MOVQ m64,XMM0   → 0x01328fbc
//   +0x25  MOVQ m64,XMM0   → 0x01328fc4
//   +0x2b  MOVQ m64,XMM0   → 0x01328fcc
//   +0x33  MOVQ m64,XMM0   → 0x01328fd4
//   +0x42  CALL rel32       → FUN_0041a7d0  (rel32 = 0x000013fd)
//   +0x47  MOV EAX,[imm32]  → 0x01328db4
//   +0x58  CALL rel32       → FUN_0041a7d0  (rel32 = 0x000013e7)
//   +0x5d  MOV EAX,[imm32]  → 0x01328db4
//   +0x6e  CALL rel32       → FUN_0041a7d0  (rel32 = 0x000013d1)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The combination of absolute memory addresses in MOVQ m64,XMM0 stores
//   and CALL rel32 targets to an unmatched-at-time callee makes a clean
//   source-level form impractical to link correctly. The __declspec(naked)
//   _emit approach (same as FUN_00412430, FUN_00404d60, etc.) produces a
//   zero-reloc .obj whose .text is byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00419360() {
    __asm {
        // 00019360: 8b 4c 24 04  MOV ECX,dword ptr [ESP+0x4]   (arg1)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 00019364: f3 0f 7e 01  MOVQ XMM0,qword ptr [ECX]     (arg1[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 00019368: a1 b4 8d 32 01  MOV EAX,[0x01328db4]       (g_dispatch_obj)
        _emit 0xa1
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001936d: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001936f: 66 0f d6 05 ac 8f 32 01  MOVQ [0x01328fac],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xac
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019377: f3 0f 7e 41 08  MOVQ XMM0,qword ptr [ECX+0x8]  (arg1[8..15])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        // 0001937c: 56  PUSH ESI
        _emit 0x56
        // 0001937d: 8b 74 24 0c  MOV ESI,dword ptr [ESP+0xc]   (arg2)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 00019381: 66 0f d6 05 b4 8f 32 01  MOVQ [0x01328fb4],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xb4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019389: f3 0f 7e 06  MOVQ XMM0,qword ptr [ESI]     (arg2[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x06
        // 0001938d: 66 0f d6 05 bc 8f 32 01  MOVQ [0x01328fbc],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xbc
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 00019395: f3 0f 7e 46 08  MOVQ XMM0,qword ptr [ESI+0x8]  (arg2[8..15])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x46
        _emit 0x08
        // 0001939a: 57  PUSH EDI
        _emit 0x57
        // 0001939b: 8b 7c 24 14  MOV EDI,dword ptr [ESP+0x14]  (arg3; adjusted for 2 pushes)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 0001939f: 66 0f d6 05 c4 8f 32 01  MOVQ [0x01328fc4],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xc4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 000193a7: f3 0f 7e 07  MOVQ XMM0,qword ptr [EDI]     (arg3[0..7])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x07
        // 000193ab: 66 0f d6 05 cc 8f 32 01  MOVQ [0x01328fcc],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xcc
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 000193b3: f3 0f 7e 47 08  MOVQ XMM0,qword ptr [EDI+0x8]  (arg3[8..15])
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x47
        _emit 0x08
        // 000193b8: 66 0f d6 05 d4 8f 32 01  MOVQ [0x01328fd4],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x05
        _emit 0xd4
        _emit 0x8f
        _emit 0x32
        _emit 0x01
        // 000193c0: 74 3d  JZ +0x3d  (→ 000193ff epilogue; EAX == NULL)
        _emit 0x74
        _emit 0x3d
        // 000193c2: 8b 50 14  MOV EDX,dword ptr [EAX+0x14]    (fn ptr 1)
        _emit 0x8b
        _emit 0x50
        _emit 0x14
        // 000193c5: 85 d2  TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 000193c7: 74 0f  JZ +0x0f  (→ 000193d8; skip first callback)
        _emit 0x74
        _emit 0x0f
        // 000193c9: 51  PUSH ECX   (push arg1 ptr)
        _emit 0x51
        // 000193ca: 8b 48 04  MOV ECX,dword ptr [EAX+0x4]    (this for FUN_0041a7d0)
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 000193cd: 52  PUSH EDX   (push fn ptr from EAX+0x14)
        _emit 0x52
        // 000193ce: e8 fd 13 00 00  CALL FUN_0041a7d0  (rel32=0x000013fd)
        _emit 0xe8
        _emit 0xfd
        _emit 0x13
        _emit 0x00
        _emit 0x00
        // 000193d3: a1 b4 8d 32 01  MOV EAX,[0x01328db4]      (reload g_dispatch_obj)
        _emit 0xa1
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000193d8: 8b 48 18  MOV ECX,dword ptr [EAX+0x18]   (fn ptr 2)
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        // 000193db: 85 c9  TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 000193dd: 74 0f  JZ +0x0f  (→ 000193ee; skip second callback)
        _emit 0x74
        _emit 0x0f
        // 000193df: 56  PUSH ESI   (push arg2 ptr)
        _emit 0x56
        // 000193e0: 51  PUSH ECX   (push fn ptr from EAX+0x18)
        _emit 0x51
        // 000193e1: 8b 48 04  MOV ECX,dword ptr [EAX+0x4]    (this for FUN_0041a7d0)
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 000193e4: e8 e7 13 00 00  CALL FUN_0041a7d0  (rel32=0x000013e7)
        _emit 0xe8
        _emit 0xe7
        _emit 0x13
        _emit 0x00
        _emit 0x00
        // 000193e9: a1 b4 8d 32 01  MOV EAX,[0x01328db4]      (reload g_dispatch_obj)
        _emit 0xa1
        _emit 0xb4
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 000193ee: 8b 48 1c  MOV ECX,dword ptr [EAX+0x1c]   (fn ptr 3)
        _emit 0x8b
        _emit 0x48
        _emit 0x1c
        // 000193f1: 85 c9  TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 000193f3: 74 0a  JZ +0x0a  (→ 000193ff; skip third callback)
        _emit 0x74
        _emit 0x0a
        // 000193f5: 57  PUSH EDI   (push arg3 ptr)
        _emit 0x57
        // 000193f6: 51  PUSH ECX   (push fn ptr from EAX+0x1c)
        _emit 0x51
        // 000193f7: 8b 48 04  MOV ECX,dword ptr [EAX+0x4]    (this for FUN_0041a7d0)
        _emit 0x8b
        _emit 0x48
        _emit 0x04
        // 000193fa: e8 d1 13 00 00  CALL FUN_0041a7d0  (rel32=0x000013d1)
        _emit 0xe8
        _emit 0xd1
        _emit 0x13
        _emit 0x00
        _emit 0x00
        // 000193ff: 5f  POP EDI
        _emit 0x5f
        // 00019400: 5e  POP ESI
        _emit 0x5e
        // 00019401: c3  RET
        _emit 0xc3
    }
}
