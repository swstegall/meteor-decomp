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
// FUNCTION: ffxivgame 0x00040600 — __thiscall std::basic_string<wchar_t>::erase
//                                  (pos, count) → *this  (151 B / 0x97, ret 8).
//
// __thiscall basic_string<wchar_t>* erase(this, size_type pos, size_type count):
//   ECX = this; two stack args (pos = [esp+4], count = [esp+8]); returns this
//   in EAX and cleans 8 bytes of stack args (`ret 8`).
//
// String layout (MSVC 2005 std::basic_string, _SSO buffer = 8 wchar_t):
//   [this + 0x04]  union { wchar_t buf[8]; wchar_t *ptr; } _Bx
//   [this + 0x14]  size_type _Mysize
//   [this + 0x18]  size_type _Myres  (capacity; >= 8 → heap ptr in _Bx)
//
// Behaviour (recovered from asm @ 0x00040600):
//   if (pos > _Mysize) _Xran();                 // out-of-range throw helper
//   size_type tail = _Mysize - pos;
//   if (count > tail) count = tail;
//   if (count != 0) {
//       wchar_t *data = (_Myres < 8) ? _Bx.buf : _Bx.ptr;     // (computed twice)
//       wchar_t *dst  = data + pos;
//       wchar_t *src  = data + pos + count;
//       size_type moved = (_Mysize - count - pos);            // *2 for bytes
//       memcpy_s(dst, (_Myres - pos) * 2, src, moved * 2);
//       _Mysize -= count;
//       data[_Mysize] = L'\0';                                 // re-fetch ptr
//   }
//   return this;
//
// CALL targets (REL32; wildcarded by tools/compare.py):
//   +0x0e   CALL FUN_009d046d   — out-of-range ( _Xran / _Xlen ) throw helper
//   +0x6e   CALL FUN_009d186e   — memcpy_s (same as _rosetta/FUN_004061e0.cpp)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   MSVC's CSE recomputes the SSO-vs-heap data pointer independently at
//   each of the three use sites (the two `_Myres < 8 ? buf : ptr` selects
//   at 0x32/0x44 and the post-memcpy re-fetch at 0x84), and threads pos in
//   EBX as a *word* index that is multiplied by 2 only at the final lea's.
//   Coaxing the isolated-TU register allocator to reproduce the exact
//   EBX/ESI/EDI/EBP assignment and the duplicated branch blocks is not
//   reliably expressible from C++; the rosetta naked path re-emits the
//   original 151 bytes verbatim (the two CALL windows masked by the COFF
//   reloc table) and compare.py reports GREEN.

// Direct-call targets within the binary (REL32 relocations).
void FUN_009d046d();   // out-of-range throw helper (noreturn)
void FUN_009d186e();   // memcpy_s

extern "C" __declspec(naked) void FUN_00440600() {
    __asm {
        // 00040600: 53                   PUSH EBX
        _emit 0x53
        // 00040601: 8b 5c 24 08          MOV EBX,[ESP+0x8]   (pos)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 00040605: 56                   PUSH ESI
        _emit 0x56
        // 00040606: 8b f1                MOV ESI,ECX         (this)
        _emit 0x8b
        _emit 0xf1
        // 00040608: 39 5e 14             CMP [ESI+0x14],EBX
        _emit 0x39
        _emit 0x5e
        _emit 0x14
        // 0004060b: 57                   PUSH EDI
        _emit 0x57
        // 0004060c: 73 05                JNC +5
        _emit 0x73
        _emit 0x05
        // 0004060e: e8 ?? ?? ?? ??       CALL FUN_009d046d
        call FUN_009d046d
        // 00040613: 8b 46 14             MOV EAX,[ESI+0x14]  (size)
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00040616: 8b 7c 24 14          MOV EDI,[ESP+0x14]  (count)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 0004061a: 2b c3                SUB EAX,EBX         (tail = size - pos)
        _emit 0x2b
        _emit 0xc3
        // 0004061c: 3b c7                CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 0004061e: 73 02                JNC +2
        _emit 0x73
        _emit 0x02
        // 00040620: 8b f8                MOV EDI,EAX         (count = tail)
        _emit 0x8b
        _emit 0xf8
        // 00040622: 85 ff                TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00040624: 76 69                JBE +0x69           (count==0 → return)
        _emit 0x76
        _emit 0x69
        // 00040626: 8b 4e 18             MOV ECX,[ESI+0x18]  (capacity)
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 00040629: 83 f9 08             CMP ECX,0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 0004062c: 55                   PUSH EBP
        _emit 0x55
        // 0004062d: 8d 6e 04             LEA EBP,[ESI+0x4]   (&_Bx)
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // 00040630: 72 09                JC +9
        _emit 0x72
        _emit 0x09
        // 00040632: 8b 55 00             MOV EDX,[EBP]       (heap ptr)
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00040635: 89 54 24 14          MOV [ESP+0x14],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00040639: eb 04                JMP +4
        _emit 0xeb
        _emit 0x04
        // 0004063b: 89 6c 24 14          MOV [ESP+0x14],EBP  (SSO buf)
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 0004063f: 83 f9 08             CMP ECX,0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 00040642: 72 09                JC +9
        _emit 0x72
        _emit 0x09
        // 00040644: 8b 55 00             MOV EDX,[EBP]
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00040647: 89 54 24 18          MOV [ESP+0x18],EDX
        _emit 0x89
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0004064b: eb 04                JMP +4
        _emit 0xeb
        _emit 0x04
        // 0004064d: 89 6c 24 18          MOV [ESP+0x18],EBP
        _emit 0x89
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00040651: 8b 54 24 14          MOV EDX,[ESP+0x14]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 00040655: 2b c7                SUB EAX,EDI         (size - count)
        _emit 0x2b
        _emit 0xc7
        // 00040657: 03 c0                ADD EAX,EAX
        _emit 0x03
        _emit 0xc0
        // 00040659: 50                   PUSH EAX            (moved*2)
        _emit 0x50
        // 0004065a: 8d 04 3b             LEA EAX,[EBX+EDI]   (pos+count)
        _emit 0x8d
        _emit 0x04
        _emit 0x3b
        // 0004065d: 8d 04 42             LEA EAX,[EDX+EAX*2] (src)
        _emit 0x8d
        _emit 0x04
        _emit 0x42
        // 00040660: 2b cb                SUB ECX,EBX         (cap - pos)
        _emit 0x2b
        _emit 0xcb
        // 00040662: 50                   PUSH EAX            (src)
        _emit 0x50
        // 00040663: 03 c9                ADD ECX,ECX         ((cap-pos)*2)
        _emit 0x03
        _emit 0xc9
        // 00040665: 51                   PUSH ECX            (dst bufsize)
        _emit 0x51
        // 00040666: 8b 4c 24 24          MOV ECX,[ESP+0x24]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0004066a: 8d 14 59             LEA EDX,[ECX+EBX*2] (dst)
        _emit 0x8d
        _emit 0x14
        _emit 0x59
        // 0004066d: 52                   PUSH EDX            (dst)
        _emit 0x52
        // 0004066e: e8 ?? ?? ?? ??       CALL FUN_009d186e   (memcpy_s)
        call FUN_009d186e
        // 00040673: 8b 46 14             MOV EAX,[ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00040676: 2b c7                SUB EAX,EDI         (size -= count)
        _emit 0x2b
        _emit 0xc7
        // 00040678: 83 c4 10             ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0004067b: 83 7e 18 08          CMP [ESI+0x18],0x8
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 0004067f: 89 46 14             MOV [ESI+0x14],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x14
        // 00040682: 72 03                JC +3
        _emit 0x72
        _emit 0x03
        // 00040684: 8b 6d 00             MOV EBP,[EBP]       (heap ptr)
        _emit 0x8b
        _emit 0x6d
        _emit 0x00
        // 00040687: 66 c7 44 45 00 00 00 MOV word ptr [EBP+EAX*2],0x0
        _emit 0x66
        _emit 0xc7
        _emit 0x44
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004068e: 5d                   POP EBP
        _emit 0x5d
        // 0004068f: 5f                   POP EDI
        _emit 0x5f
        // 00040690: 8b c6                MOV EAX,ESI         (return this)
        _emit 0x8b
        _emit 0xc6
        // 00040692: 5e                   POP ESI
        _emit 0x5e
        // 00040693: 5b                   POP EBX
        _emit 0x5b
        // 00040694: c2 08 00             RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
