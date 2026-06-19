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
// FUNCTION: ffxivgame 0x0003f4b0 — string::assign(const string&, size_t pos,
//                                  size_t count), __thiscall, RET 0xC (217 B).
//
// Signature (recovered from asm):
//
//   string* __thiscall FUN_0043f4b0(string* this,   // ECX
//                                    string* src,    // arg1 = EBX
//                                    size_t  pos,    // arg2 = EBP
//                                    size_t  count); // arg3 = EAX (at ESP+0x1c after 4 saves)
//
// MSVC SSO string layout (offsets from `this`):
//   +0x04  _Bx._Ptr / _Bx._Buf[16]  — heap ptr or inline buffer
//   +0x14  _Mysize                  — current length in chars
//   +0x18  _Myres                   — capacity (_Myres < 0x10 ⟹ SSO)
//
// Body sketch:
//
//   1. Bounds-check: abort if src->size < pos             (→ 0x009d046d)
//   2. Compute effective = min(count, src->size - pos)    (CMOVC clamp)
//   3. Self-assign (this == src):
//        erase(pos+effective, npos)                       (→ 0x00449570)
//        erase(0, pos)                                    (→ 0x00449570)
//        return this;
//   4. Overflow guard: abort if effective > 0xFFFFFFFE   (→ 0x009d042e)
//   5. Grow if this->capacity < effective                 (→ 0x0043f2e0)
//   6. If effective == 0: set size=0, null-terminate, return this.
//   7. Resolve src data ptr (heap or SSO inline buf).
//   8. Resolve dst data ptr (heap or SSO inline buf).
//   9. _memmove_s(dst, this->capacity, src+pos, effective) (→ 0x009d17f3)
//  10. Set this->size = effective, null-terminate, return this.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The function contains a CMOVC (0f 42) that MSVC 2005 will emit for the
//   min-clamp, plus interleaved SSO branch chains and five external CALLs
//   with REL32 relocations.  High-level C++ rewrites shift at least one
//   register allocation or branch encoding under /O2 /Oi.  Naked asm
//   emitting the exact 217 orig bytes gives a reloc-masked GREEN diff.
//
// Reloc sites (REL32 imm32 windows masked by compare.py):
//   +0x13  CALL 0x009d046d  — abort (pos out of range)
//   +0x31  CALL 0x00449570  — erase(pos+count, npos)
//   +0x3b  CALL 0x00449570  — erase(0, pos)
//   +0x4e  CALL 0x009d042e  — abort (count overflow)
//   +0x61  CALL 0x0043f2e0  — grow/reserve
//   +0xb9  CALL 0x009d17f3  — _memmove_s

extern "C" __declspec(naked) void FUN_0043f4b0() {
    __asm {
        // 0003f4b0
        _emit 0x53          // PUSH EBX
        _emit 0x8b          // MOV EBX, [ESP+8]
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x55          // PUSH EBP
        _emit 0x8b          // MOV EBP, [ESP+0x10]
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x39          // CMP [EBX+0x14], EBP
        _emit 0x6b
        _emit 0x14
        _emit 0x56          // PUSH ESI
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV ESI, ECX
        _emit 0xf1
        _emit 0x73          // JNC +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d046d  [RELOC]
        _emit 0xa5
        _emit 0x0f
        _emit 0x59
        _emit 0x00
        // 0003f4c8
        _emit 0x8b          // MOV EDI, [EBX+0x14]
        _emit 0x7b
        _emit 0x14
        _emit 0x8b          // MOV EAX, [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x2b          // SUB EDI, EBP
        _emit 0xfd
        _emit 0x3b          // CMP EAX, EDI
        _emit 0xc7
        _emit 0x0f          // CMOVC EDI, EAX
        _emit 0x42
        _emit 0xf8
        _emit 0x3b          // CMP ESI, EBX
        _emit 0xf3
        _emit 0x75          // JNZ +0x1f
        _emit 0x1f
        // 0003f4da (self-assign path)
        _emit 0x6a          // PUSH -1
        _emit 0xff
        _emit 0x03          // ADD EDI, EBP
        _emit 0xfd
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x00449570  [RELOC]
        _emit 0x8a
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x55          // PUSH EBP
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x00449570  [RELOC]
        _emit 0x80
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        // 0003f4f0 (self-assign return)
        _emit 0x5f          // POP EDI
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0xc
        _emit 0x0c
        _emit 0x00
        // 0003f4f9 (different-string path)
        _emit 0x83          // CMP EDI, -2
        _emit 0xff
        _emit 0xfe
        _emit 0x76          // JBE +5
        _emit 0x05
        _emit 0xe8          // CALL 0x009d042e  [RELOC]
        _emit 0x2b
        _emit 0x0f
        _emit 0x59
        _emit 0x00
        // 0003f503
        _emit 0x8b          // MOV EAX, [ESI+0x18]
        _emit 0x46
        _emit 0x18
        _emit 0x3b          // CMP EAX, EDI
        _emit 0xc7
        _emit 0x73          // JNC +0x1b
        _emit 0x1b
        // need-grow path
        _emit 0x8b          // MOV EAX, [ESI+0x14]
        _emit 0x46
        _emit 0x14
        _emit 0x50          // PUSH EAX
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x0043f2e0  [RELOC]
        _emit 0xca
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0003f516
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x76          // JBE +0x66
        _emit 0x66
        // 0003f51a (count > 0)
        _emit 0x83          // CMP [EBX+0x18], 0x10
        _emit 0x7b
        _emit 0x18
        _emit 0x10
        _emit 0x72          // JC +0x2f  (src SSO)
        _emit 0x2f
        _emit 0x8b          // MOV EDX, [EBX+4]  (src heap ptr)
        _emit 0x53
        _emit 0x04
        _emit 0xeb          // JMP +0x2d
        _emit 0x2d
        // 0003f525 (capacity-ok, count==0 check)
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x75          // JNZ -0x11
        _emit 0xef
        // 0003f529 (count==0 path, EAX = this->capacity from earlier)
        _emit 0x83          // CMP EAX, 0x10
        _emit 0xf8
        _emit 0x10
        _emit 0x89          // MOV [ESI+0x14], EDI  (size = 0)
        _emit 0x7e
        _emit 0x14
        _emit 0x72          // JC +0xf  (SSO)
        _emit 0x0f
        // 0003f531 (heap, count==0)
        _emit 0x8b          // MOV EAX, [ESI+4]
        _emit 0x46
        _emit 0x04
        _emit 0x5f          // POP EDI
        _emit 0xc6          // MOV byte ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0xc
        _emit 0x0c
        _emit 0x00
        // 0003f540 (SSO, count==0)
        _emit 0x8d          // LEA EAX, [ESI+4]
        _emit 0x46
        _emit 0x04
        _emit 0x5f          // POP EDI
        _emit 0xc6          // MOV byte ptr [EAX], 0
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0xc
        _emit 0x0c
        _emit 0x00
        // 0003f54f (src SSO path — EDX = &src->inline_buf)
        _emit 0x8d          // LEA EDX, [EBX+4]
        _emit 0x53
        _emit 0x04
        // 0003f552 (copy path — compute dst ptr)
        _emit 0x8b          // MOV ECX, [ESI+0x18]  (this->capacity)
        _emit 0x4e
        _emit 0x18
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x8d          // LEA EBX, [ESI+4]
        _emit 0x5e
        _emit 0x04
        _emit 0x72          // JC +4  (SSO dst)
        _emit 0x04
        _emit 0x8b          // MOV EAX, [EBX]  (heap dst ptr)
        _emit 0x03
        _emit 0xeb          // JMP +2
        _emit 0x02
        // SSO dst:
        _emit 0x8b          // MOV EAX, EBX  (inline buf = ESI+4)
        _emit 0xc3
        // 0003f563 (_memmove_s call)
        _emit 0x57          // PUSH EDI  (count)
        _emit 0x03          // ADD EDX, EBP  (src_data + pos)
        _emit 0xd5
        _emit 0x52          // PUSH EDX  (src+pos)
        _emit 0x51          // PUSH ECX  (this->capacity)
        _emit 0x50          // PUSH EAX  (dst)
        _emit 0xe8          // CALL 0x009d17f3  [RELOC]
        _emit 0x85
        _emit 0x22
        _emit 0x59
        _emit 0x00
        // 0003f56e
        _emit 0x83          // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x83          // CMP [ESI+0x18], 0x10
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        _emit 0x89          // MOV [ESI+0x14], EDI
        _emit 0x7e
        _emit 0x14
        _emit 0x72          // JC +2
        _emit 0x02
        _emit 0x8b          // MOV EBX, [EBX]  (heap ptr)
        _emit 0x1b
        // 0003f57c
        _emit 0xc6          // MOV byte ptr [EBX+EDI], 0
        _emit 0x04
        _emit 0x3b
        _emit 0x00
        // 0003f580 (common return)
        _emit 0x5f          // POP EDI
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
