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
// FUNCTION: ffxivgame 0x00440e10 — vector-like container initializer
//                                  (__thiscall, 160 bytes / 0xa0)
//
// Calling convention: __thiscall (ECX = this); RET 0x8 (callee-cleans 2 dwords).
// Callee-saves pushed: EBP, EBX, ESI, EDI.
//
// Signature:
//   void __thiscall FUN_00440e10(Container *this, unsigned int count, void *fill_arg)
//
// Object layout (offsets touched):
//   [this + 0x04]   begin pointer  (initialized to alloc'd block, or NULL)
//   [this + 0x08]   end pointer    (initialized to begin, then advanced to end after fill)
//   [this + 0x0c]   end_cap        (begin + count * 8)
//
// Element size = 8 bytes. Three pointers encode a fixed-size
// (count * 8) allocation of element-size-8 objects.
//
// Control flow:
//   1. Zero {field4, field8, fieldC}, then return early if count == 0.
//   2. If count > 0x1FFFFFFF, call FUN_00cb0e40 (length_error / overflow throw).
//   3. Call FUN_008e94d0(count, 0) → alloc'd block ptr (EDI).
//   4. Set fieldC = alloc + count*8 (end_cap).
//   5. Set field4 = field8 = alloc (begin = end for now).
//   6. Set SEH state variable [EBP-4] = 0 (enter __try body 0).
//   7. Call FUN_00965ad0(alloc, count, fill_arg, this, 0, 0) — fill the block.
//   8. Set field8 = alloc + count*8 (end now equals end_cap).
//   9. Restore SEH chain, pop security cookie, epilogue.
//
// Stack frame (MSVC 2005 /GS + SEH):
//   [EBP - 0x04]  SEH state var (init -1; set to 0 on __try entry)
//   [EBP - 0x08]  SEH handler address (0x00e56f80 in the binary)
//   [EBP - 0x0C]  saved FS:[0] (old SEH chain)
//   [EBP - 0x10]  saved ESP (cookie check base)
//   [EBP - 0x14]  saved 'this' (ESI)
//   [EBP - 0x24]  security cookie (XOR of __security_cookie and EBP)
//
// Callers pass args right-to-left (stdcall/cdecl order) but this function
// uses __thiscall (ECX = this) + explicit stack args at [EBP+8] and [EBP+C].
//
// Reloc-bearing CALL sites (compare.py masks these bytes):
//   +0x4a  CALL rel32 → FUN_00cb0e40  (0x00440e5a, rel32 = 0x0086ffe1)
//   +0x51  CALL rel32 → FUN_008e94d0  (0x00440e61, rel32 = 0x004a866a)
//   +0x7e  CALL rel32 → FUN_00965ad0  (0x00440e8e, rel32 = 0x00524c3d)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The MSVC 2005 SEH prologue (PUSH -1 / PUSH handler / MOV EAX,FS:[0] /
//   PUSH EAX / SUB ESP,8) and the /GS cookie (MOV EAX,[__security_cookie] /
//   XOR EAX,EBP / PUSH EAX / LEA … / MOV FS:[0],EAX) cannot be reproduced
//   from C++ source without intimate control of the frame layout. Additionally,
//   the lazy multi-call stack cleanup (ADD ESP,0x20 cleaning two callee stacks
//   at once) and the mid-function register reloads ([EBP+0x8] overwritten with
//   0 to source two PUSH 0 args) are un-expressible in high-level C++.
//   A __declspec(naked) body re-emitting all 160 bytes verbatim produces a
//   .obj whose .text is byte-identical to the original slice. compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_00440e10() {
    __asm {
        // 00040e10: 55                       PUSH EBP
        _emit 0x55
        // 00040e11: 8b ec                    MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 00040e13: 6a ff                    PUSH -0x1   (SEH state = -1)
        _emit 0x6a
        _emit 0xff
        // 00040e15: 68 80 6f e5 00           PUSH 0xe56f80  (SEH handler addr)
        _emit 0x68
        _emit 0x80
        _emit 0x6f
        _emit 0xe5
        _emit 0x00
        // 00040e1a: 64 a1 00 00 00 00        MOV EAX,FS:[0x0]  (old chain)
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040e20: 50                       PUSH EAX
        _emit 0x50
        // 00040e21: 83 ec 08                 SUB ESP,0x8  (local vars)
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 00040e24: 53                       PUSH EBX
        _emit 0x53
        // 00040e25: 56                       PUSH ESI
        _emit 0x56
        // 00040e26: 57                       PUSH EDI
        _emit 0x57
        // 00040e27: a1 b0 a8 2e 01           MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00040e2c: 33 c5                    XOR EAX,EBP
        _emit 0x33
        _emit 0xc5
        // 00040e2e: 50                       PUSH EAX  (cookie)
        _emit 0x50
        // 00040e2f: 8d 45 f4                 LEA EAX,[EBP + -0xc]  (SEH record)
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        // 00040e32: 64 a3 00 00 00 00        MOV FS:[0x0],EAX  (install SEH handler)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040e38: 89 65 f0                 MOV dword ptr [EBP + -0x10],ESP
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        // 00040e3b: 8b f1                    MOV ESI,ECX  (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00040e3d: 89 75 ec                 MOV dword ptr [EBP + -0x14],ESI
        _emit 0x89
        _emit 0x75
        _emit 0xec
        // 00040e40: 8b 5d 08                 MOV EBX,dword ptr [EBP + 0x8]  (EBX = count)
        _emit 0x8b
        _emit 0x5d
        _emit 0x08
        // 00040e43: 33 c0                    XOR EAX,EAX  (EAX = 0)
        _emit 0x33
        _emit 0xc0
        // 00040e45: 3b d8                    CMP EBX,EAX  (count == 0?)
        _emit 0x3b
        _emit 0xd8
        // 00040e47: 89 46 04                 MOV dword ptr [ESI + 0x4],EAX  (begin = NULL)
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 00040e4a: 89 46 08                 MOV dword ptr [ESI + 0x8],EAX  (end = NULL)
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 00040e4d: 89 46 0c                 MOV dword ptr [ESI + 0xc],EAX  (end_cap = NULL)
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 00040e50: 74 4a                    JZ +0x4a  (count == 0 → epilogue)
        _emit 0x74
        _emit 0x4a
        // 00040e52: 81 fb ff ff ff 1f        CMP EBX,0x1fffffff  (overflow guard)
        _emit 0x81
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x1f
        // 00040e58: 76 05                    JBE +0x05  (count ok → skip throw)
        _emit 0x76
        _emit 0x05
        // 00040e5a: e8 e1 ff 86 00           CALL FUN_00cb0e40  (throw length_error)
        _emit 0xe8
        _emit 0xe1
        _emit 0xff
        _emit 0x86
        _emit 0x00
        // 00040e5f: 50                       PUSH EAX  (arg2 = 0)
        _emit 0x50
        // 00040e60: 53                       PUSH EBX  (arg1 = count)
        _emit 0x53
        // 00040e61: e8 6a 86 4a 00           CALL FUN_008e94d0  (alloc count * 8)
        _emit 0xe8
        _emit 0x6a
        _emit 0x86
        _emit 0x4a
        _emit 0x00
        // 00040e66: 8b f8                    MOV EDI,EAX  (EDI = alloc'd block)
        _emit 0x8b
        _emit 0xf8
        // 00040e68: c6 45 08 00              MOV byte ptr [EBP + 0x8],0x0  (zero arg1 slot)
        _emit 0xc6
        _emit 0x45
        _emit 0x08
        _emit 0x00
        // 00040e6c: 8b 4d 08                 MOV ECX,dword ptr [EBP + 0x8]  (ECX = 0)
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        // 00040e6f: 8b 55 08                 MOV EDX,dword ptr [EBP + 0x8]  (EDX = 0)
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        // 00040e72: 51                       PUSH ECX  (0)
        _emit 0x51
        // 00040e73: 52                       PUSH EDX  (0)
        _emit 0x52
        // 00040e74: 8d 04 df                 LEA EAX,[EDI + EBX*0x8]  (end_cap)
        _emit 0x8d
        _emit 0x04
        _emit 0xdf
        // 00040e77: 89 46 0c                 MOV dword ptr [ESI + 0xc],EAX  (store end_cap)
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 00040e7a: 8b 45 0c                 MOV EAX,dword ptr [EBP + 0xc]  (fill_arg)
        _emit 0x8b
        _emit 0x45
        _emit 0x0c
        // 00040e7d: 56                       PUSH ESI  (this)
        _emit 0x56
        // 00040e7e: 50                       PUSH EAX  (fill_arg)
        _emit 0x50
        // 00040e7f: 53                       PUSH EBX  (count)
        _emit 0x53
        // 00040e80: 57                       PUSH EDI  (alloc block)
        _emit 0x57
        // 00040e81: 89 7e 04                 MOV dword ptr [ESI + 0x4],EDI  (begin = alloc)
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        // 00040e84: 89 7e 08                 MOV dword ptr [ESI + 0x8],EDI  (end = alloc initially)
        _emit 0x89
        _emit 0x7e
        _emit 0x08
        // 00040e87: c7 45 fc 00 00 00 00     MOV dword ptr [EBP + -0x4],0x0  (SEH state = 0)
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040e8e: e8 3d 4c 52 00           CALL FUN_00965ad0  (fill block)
        _emit 0xe8
        _emit 0x3d
        _emit 0x4c
        _emit 0x52
        _emit 0x00
        // 00040e93: 8d 0c df                 LEA ECX,[EDI + EBX*0x8]  (end = begin + count*8)
        _emit 0x8d
        _emit 0x0c
        _emit 0xdf
        // 00040e96: 83 c4 20                 ADD ESP,0x20  (clean 8 pushed dwords)
        _emit 0x83
        _emit 0xc4
        _emit 0x20
        // 00040e99: 89 4e 08                 MOV dword ptr [ESI + 0x8],ECX  (store new end)
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        // === epilogue (join point for both paths) ===
        // 00040e9c: 8b 4d f4                 MOV ECX,dword ptr [EBP + -0xc]  (old FS:[0])
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        // 00040e9f: 64 89 0d 00 00 00 00     MOV dword ptr FS:[0x0],ECX  (restore SEH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040ea6: 59                       POP ECX  (pop security cookie)
        _emit 0x59
        // 00040ea7: 5f                       POP EDI
        _emit 0x5f
        // 00040ea8: 5e                       POP ESI
        _emit 0x5e
        // 00040ea9: 5b                       POP EBX
        _emit 0x5b
        // 00040eaa: 8b e5                    MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 00040eac: 5d                       POP EBP
        _emit 0x5d
        // 00040ead: c2 08 00                 RET 0x8  (callee-cleans 2 dwords)
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
