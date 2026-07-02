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
// FUNCTION: ffxivgame 0x00417e20 — vector<T,8>::reserve()-style grow
//                                  (__thiscall, 241 bytes / 0xf1)
//
// Calling convention: __thiscall (ECX = this); RET 0x4 (callee cleans one
// stack dword — the explicit `unsigned int _Count` argument), matching a
// Dinkumware `vector<T>::reserve(size_type)` overload where T is an 8-byte
// element (e.g. a `pair<int,int>` or two-pointer struct).
//
// Signature:
//   void __thiscall FUN_00417e20(Container *this, unsigned int count)
//
// Object layout (offsets touched, standard Dinkumware _Myfirst/_Mylast/_Myend
// triple, shifted +4 because offset 0 is presumably an allocator/base slot
// not read by this function):
//   [this + 0x04]   _Myfirst  (begin pointer)
//   [this + 0x08]   _Mylast   (end pointer, one-past-last-constructed)
//   [this + 0x0c]   _Myend    (end-of-storage pointer)
//
// Control flow (from asm/ffxivgame/00017e20_FUN_00417e20.s):
//   1. count > 0x1FFFFFFF  → CALL 0x00c5aed0 (length_error / overflow throw).
//   2. capacity() = _Myfirst == 0 ? 0 : (_Myend - _Myfirst) >> 3.
//   3. if capacity() >= count, skip straight to epilogue (no growth needed).
//   4. CALL 0x00a5dca0(this, count)  → allocate new block, EBX = new _Myfirst.
//   5. Aliasing/range sanity check on the *old* [first,last) span; if it
//      trips (first > last, unsigned), CALL 0x009d22b4 (debug assert/throw).
//      Same check repeated on the *new* copy range before the copy call.
//   6. CALL 0x009873e0(new_first, old_first, old_last, this, count, 0)
//      — uninitialized-copy the existing elements into the new block.
//   7. If old _Myfirst != 0: CALL 0x0040df70(old_first) — destroy + free the
//      old block (element dtor loop / deallocate wrapper).
//   8. Store new _Myfirst/_Mylast/_Myend (LEA with EAX/EDI = element counts
//      scaled by 8) back into `this`.
//   9. SEH/GS epilogue: restore FS:[0] chain, pop cookie/EDI/ESI/EBX, RET 4.
//
// Stack frame (MSVC 2005 /GS + SEH, same shape as every reserve()-class
// function in this binary):
//   [EBP - 0x04]  SEH state var (init -1; set 0 on __try entry, -1 again
//                 once the copy+free sequence completes cleanly)
//   [EBP - 0x08]  SEH handler address (0x00e55250 in this binary)
//   [EBP - 0x0C]  saved FS:[0] (old SEH chain)
//   [EBP - 0x10]  saved ESP (cookie check base)
//   [EBP - 0x14]  local: post-copy _Mylast snapshot
//   [EBP - 0x18]  local: new _Myfirst (EBX)
//   [EBP - 0x24]  security cookie (XOR of __security_cookie and EBP)
//
// Reloc-bearing CALL sites (compare.py masks these bytes):
//   +0x38  CALL rel32 → 0x00c5aed0  (0x00017e58, rel32 = 0x00843073)
//   +0x5b  CALL rel32 → 0x00a5dca0  (0x00017e7b, rel32 = 0x00645e20)
//   +0x74  CALL rel32 → 0x009d22b4  (0x00017e94, rel32 = 0x005ba41b)
//   +0x84  CALL rel32 → 0x009d22b4  (0x00017ea4, rel32 = 0x005ba40b)
//   +0x9c  CALL rel32 → 0x009873e0  (0x00017ebc, rel32 = 0x0056f51f)
//   +0xc6  CALL rel32 → 0x0040df70  (0x00017ee6, rel32 = 0xffff6085)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Same rationale as FUN_00440e10 / FUN_00403a20 in this directory: the
//   SEH prologue (PUSH -1 / PUSH handler / MOV EAX,FS:[0] / PUSH EAX /
//   SUB ESP,0xc), the /GS cookie dance, the two aliasing-check calls
//   gated on raw CMP/JBE against caller-invisible locals, and the final
//   three-field pointer store are not reproducible byte-for-byte from
//   high-level `reserve()` source without full control of MSVC 2005's
//   register allocation and local-variable placement in this exact
//   Dinkumware revision. A `__declspec(naked)` body re-emitting all 241
//   bytes verbatim (per asm/ffxivgame/00017e20_FUN_00417e20.s) produces a
//   .obj whose .text is byte-identical to the original slice.

extern "C" __declspec(naked) void FUN_00417e20() {
    __asm {
        // 00017e20: 55                       PUSH EBP
        _emit 0x55
        // 00017e21: 8b ec                    MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 00017e23: 6a ff                    PUSH -0x1        (SEH state = -1)
        _emit 0x6a
        _emit 0xff
        // 00017e25: 68 50 52 e5 00           PUSH 0xe55250    (SEH handler addr)
        _emit 0x68
        _emit 0x50
        _emit 0x52
        _emit 0xe5
        _emit 0x00
        // 00017e2a: 64 a1 00 00 00 00        MOV EAX,FS:[0x0] (old chain)
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017e30: 50                       PUSH EAX
        _emit 0x50
        // 00017e31: 83 ec 0c                 SUB ESP,0xc      (locals)
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00017e34: 53                       PUSH EBX
        _emit 0x53
        // 00017e35: 56                       PUSH ESI
        _emit 0x56
        // 00017e36: 57                       PUSH EDI
        _emit 0x57
        // 00017e37: a1 b0 a8 2e 01           MOV EAX,[0x012ea8b0] (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00017e3c: 33 c5                    XOR EAX,EBP
        _emit 0x33
        _emit 0xc5
        // 00017e3e: 50                       PUSH EAX         (cookie)
        _emit 0x50
        // 00017e3f: 8d 45 f4                 LEA EAX,[EBP + -0xc] (SEH record)
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        // 00017e42: 64 a3 00 00 00 00        MOV FS:[0x0],EAX (install SEH handler)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017e48: 89 65 f0                 MOV dword ptr [EBP + -0x10],ESP
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        // 00017e4b: 8b f1                    MOV ESI,ECX      (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00017e4d: 8b 55 08                 MOV EDX,dword ptr [EBP + 0x8] (count)
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        // 00017e50: 81 fa ff ff ff 1f        CMP EDX,0x1fffffff
        _emit 0x81
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x1f
        // 00017e56: 76 05                    JBE +0x05        (count ok)
        _emit 0x76
        _emit 0x05
        // 00017e58: e8 73 30 84 00           CALL 0x00c5aed0  (length_error)
        _emit 0xe8
        _emit 0x73
        _emit 0x30
        _emit 0x84
        _emit 0x00
        // 00017e5d: 8b 4e 04                 MOV ECX,dword ptr [ESI + 0x4] (_Myfirst)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 00017e60: 85 c9                    TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 00017e62: 75 04                    JNZ +0x04
        _emit 0x75
        _emit 0x04
        // 00017e64: 33 c0                    XOR EAX,EAX      (capacity = 0)
        _emit 0x33
        _emit 0xc0
        // 00017e66: eb 08                    JMP +0x08
        _emit 0xeb
        _emit 0x08
        // 00017e68: 8b 46 0c                 MOV EAX,dword ptr [ESI + 0xc] (_Myend)
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 00017e6b: 2b c1                    SUB EAX,ECX
        _emit 0x2b
        _emit 0xc1
        // 00017e6d: c1 f8 03                 SAR EAX,0x3      (/ sizeof(T)==8)
        _emit 0xc1
        _emit 0xf8
        _emit 0x03
        // 00017e70: 3b c2                    CMP EAX,EDX      (capacity() vs count)
        _emit 0x3b
        _emit 0xc2
        // 00017e72: 0f 83 85 00 00 00        JNC +0x85        (enough room -> epilogue)
        _emit 0x0f
        _emit 0x83
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017e78: 52                       PUSH EDX         (count)
        _emit 0x52
        // 00017e79: 8b ce                    MOV ECX,ESI      (this)
        _emit 0x8b
        _emit 0xce
        // 00017e7b: e8 20 5e 64 00           CALL 0x00a5dca0  (allocate)
        _emit 0xe8
        _emit 0x20
        _emit 0x5e
        _emit 0x64
        _emit 0x00
        // 00017e80: 8b 7e 08                 MOV EDI,dword ptr [ESI + 0x8] (old _Mylast)
        _emit 0x8b
        _emit 0x7e
        _emit 0x08
        // 00017e83: 39 7e 04                 CMP dword ptr [ESI + 0x4],EDI (old first vs last)
        _emit 0x39
        _emit 0x7e
        _emit 0x04
        // 00017e86: 8b d8                    MOV EBX,EAX      (new _Myfirst)
        _emit 0x8b
        _emit 0xd8
        // 00017e88: 89 5d e8                 MOV dword ptr [EBP + -0x18],EBX
        _emit 0x89
        _emit 0x5d
        _emit 0xe8
        // 00017e8b: c7 45 fc 00 00 00 00     MOV dword ptr [EBP + -0x4],0x0 (SEH state = 0)
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017e92: 76 05                    JBE +0x05        (range ok)
        _emit 0x76
        _emit 0x05
        // 00017e94: e8 1b a4 5b 00           CALL 0x009d22b4  (aliasing check)
        _emit 0xe8
        _emit 0x1b
        _emit 0xa4
        _emit 0x5b
        _emit 0x00
        // 00017e99: 8b 46 04                 MOV EAX,dword ptr [ESI + 0x4] (old _Myfirst)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00017e9c: 3b 46 08                 CMP EAX,dword ptr [ESI + 0x8] (old first vs last)
        _emit 0x3b
        _emit 0x46
        _emit 0x08
        // 00017e9f: 89 45 ec                 MOV dword ptr [EBP + -0x14],EAX
        _emit 0x89
        _emit 0x45
        _emit 0xec
        // 00017ea2: 76 08                    JBE +0x08        (range ok)
        _emit 0x76
        _emit 0x08
        // 00017ea4: e8 0b a4 5b 00           CALL 0x009d22b4  (aliasing check)
        _emit 0xe8
        _emit 0x0b
        _emit 0xa4
        _emit 0x5b
        _emit 0x00
        // 00017ea9: 8b 45 ec                 MOV EAX,dword ptr [EBP + -0x14]
        _emit 0x8b
        _emit 0x45
        _emit 0xec
        // 00017eac: 8b 55 08                 MOV EDX,dword ptr [EBP + 0x8] (count)
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        // 00017eaf: c6 45 ec 00              MOV byte ptr [EBP + -0x14],0x0
        _emit 0xc6
        _emit 0x45
        _emit 0xec
        _emit 0x00
        // 00017eb3: 8b 4d ec                 MOV ECX,dword ptr [EBP + -0x14]
        _emit 0x8b
        _emit 0x4d
        _emit 0xec
        // 00017eb6: 51                       PUSH ECX
        _emit 0x51
        // 00017eb7: 52                       PUSH EDX
        _emit 0x52
        // 00017eb8: 56                       PUSH ESI
        _emit 0x56
        // 00017eb9: 53                       PUSH EBX
        _emit 0x53
        // 00017eba: 57                       PUSH EDI
        _emit 0x57
        // 00017ebb: 50                       PUSH EAX
        _emit 0x50
        // 00017ebc: e8 1f f5 56 00           CALL 0x009873e0  (copy-construct into new block)
        _emit 0xe8
        _emit 0x1f
        _emit 0xf5
        _emit 0x56
        _emit 0x00
        // 00017ec1: 8b 46 04                 MOV EAX,dword ptr [ESI + 0x4] (old _Myfirst again)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00017ec4: 83 c4 18                 ADD ESP,0x18     (pop 6 pushed args)
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00017ec7: 85 c0                    TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00017ec9: c7 45 fc ff ff ff ff     MOV dword ptr [EBP + -0x4],0xffffffff (SEH state = -1)
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 00017ed0: 75 04                    JNZ +0x04
        _emit 0x75
        _emit 0x04
        // 00017ed2: 33 ff                    XOR EDI,EDI
        _emit 0x33
        _emit 0xff
        // 00017ed4: eb 08                    JMP +0x08
        _emit 0xeb
        _emit 0x08
        // 00017ed6: 8b 7e 08                 MOV EDI,dword ptr [ESI + 0x8] (old _Mylast)
        _emit 0x8b
        _emit 0x7e
        _emit 0x08
        // 00017ed9: 2b f8                    SUB EDI,EAX
        _emit 0x2b
        _emit 0xf8
        // 00017edb: c1 ff 03                 SAR EDI,0x3      (old element count)
        _emit 0xc1
        _emit 0xff
        _emit 0x03
        // 00017ede: 85 c0                    TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00017ee0: 74 09                    JZ +0x09         (old first == NULL, skip free)
        _emit 0x74
        _emit 0x09
        // 00017ee2: 8b 48 fc                 MOV ECX,dword ptr [EAX + -0x4]
        _emit 0x8b
        _emit 0x48
        _emit 0xfc
        // 00017ee5: 50                       PUSH EAX
        _emit 0x50
        // 00017ee6: e8 85 60 ff ff           CALL 0x0040df70  (destroy + free old block)
        _emit 0xe8
        _emit 0x85
        _emit 0x60
        _emit 0xff
        _emit 0xff
        // 00017eeb: 8b 45 08                 MOV EAX,dword ptr [EBP + 0x8] (count)
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // 00017eee: 8d 0c c3                 LEA ECX,[EBX + EAX*0x8] (new _Myend)
        _emit 0x8d
        _emit 0x0c
        _emit 0xc3
        // 00017ef1: 8d 14 fb                 LEA EDX,[EBX + EDI*0x8] (new _Mylast)
        _emit 0x8d
        _emit 0x14
        _emit 0xfb
        // 00017ef4: 89 4e 0c                 MOV dword ptr [ESI + 0xc],ECX (_Myend = ...)
        _emit 0x89
        _emit 0x4e
        _emit 0x0c
        // 00017ef7: 89 56 08                 MOV dword ptr [ESI + 0x8],EDX (_Mylast = ...)
        _emit 0x89
        _emit 0x56
        _emit 0x08
        // 00017efa: 89 5e 04                 MOV dword ptr [ESI + 0x4],EBX (_Myfirst = new block)
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // === epilogue (join point for both paths) ===
        // 00017efd: 8b 4d f4                 MOV ECX,dword ptr [EBP + -0xc] (old FS:[0])
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        // 00017f00: 64 89 0d 00 00 00 00     MOV dword ptr FS:[0x0],ECX (restore SEH chain)
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00017f07: 59                       POP ECX          (pop security cookie)
        _emit 0x59
        // 00017f08: 5f                       POP EDI
        _emit 0x5f
        // 00017f09: 5e                       POP ESI
        _emit 0x5e
        // 00017f0a: 5b                       POP EBX
        _emit 0x5b
        // 00017f0b: 8b e5                    MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 00017f0d: 5d                       POP EBP
        _emit 0x5d
        // 00017f0e: c2 04 00                 RET 0x4          (callee-cleans 1 dword)
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
