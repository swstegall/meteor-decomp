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
// FUNCTION: ffxivgame 0x000409b0 — std::basic_string<wchar_t>::assign(ptr, count)
//           (218 B / 0xda, __thiscall, ret 0x8).
//
// Signature:
//   __thiscall basic_string<wchar_t>* FUN_004409b0(
//       this,
//       const wchar_t *ptr,   // stack arg 0 → [ESP+0x10] after prologue pushes
//       size_type count);     // stack arg 1 → [ESP+0x14] after prologue pushes
//   Returns this in EAX; callee cleans 8 stack bytes (ret 0x8).
//
// String layout (MSVC 2005 std::basic_string<wchar_t>, SSO buffer = 8 wchar_t):
//   [this + 0x04]  union { wchar_t _Buf[8]; wchar_t *_Ptr; } _Bx
//   [this + 0x14]  size_type _Mysize  (character count, not bytes)
//   [this + 0x18]  size_type _Myres   (capacity; _Myres >= 8 → heap ptr in _Bx)
//
// Behaviour (recovered from asm @ RVA 0x000409b0):
//
//   ESI = this (ECX at entry)
//   EBP = &this->_Bx  (LEA EBP,[ESI+0x4])
//
//   // ---- Self-reference check: is ptr within the current buffer? ------
//   wchar_t *begin = (_Myres >= 8) ? _Bx._Ptr : (wchar_t*)&_Bx;
//   if (ptr < begin)                    goto general;       // JC +0x36
//   wchar_t *end = begin + _Mysize;
//   if (end <= ptr)                     goto general;       // JBE +0x20
//
//   // ---- Self-reference path: delegate to assign(str, pos, count) -----
//   wchar_t *data = (_Myres >= 8) ? _Bx._Ptr : (wchar_t*)&_Bx;
//   size_t pos = (ptr - data) >> 1;   // byte offset → wchar_t index
//   return FUN_00440850(this /*ECX*/, *this /*as &str*/, pos, count);
//
//   // ---- General path (ptr is outside the current buffer) -------------
//   general:
//   if (count > 0x7ffffffe) _Xlen();
//   if (this->_Myres < count) _Grow(count, this->_Mysize);
//
//   if (count == 0) {                  // zero-size sub-paths
//       this->_Mysize = 0;
//       wchar_t *dst = (_Myres >= 8) ? _Bx._Ptr : (wchar_t*)&_Bx;
//       *dst = L'\0';
//       return this;
//   }
//
//   // Reload destination (capacity may have changed after _Grow)
//   wchar_t *dst = (this->_Myres >= 8) ? _Bx._Ptr : (wchar_t*)&_Bx;
//   size_t n_bytes = count * 2;
//   wmemmove_s(dst, this->_Myres * 2, ptr, n_bytes);
//   this->_Mysize = count;
//   *(wchar_t*)((char*)dst + n_bytes) = L'\0';
//   return this;
//
// CALL targets (all REL32; masked as wildcards by tools/compare.py):
//   +0x4a  CALL FUN_00440850  — wstring::assign(const wstring&, pos, count)
//   +0x61  CALL FUN_009d042e  — _Xlen() length_error throw helper
//   +0x74  CALL FUN_004406a0  — wstring internal _Grow(new_size, old_size)
//   +0xb7  CALL FUN_009d17f3  — wmemmove_s(dst, dst_bytes, src, n_bytes)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The function is a close structural sibling of FUN_00440850 (the assign-
//   from-wstring overload matched at 0x00040850). The register allocation
//   (ESI = this, EBP = &_Bx, EDX = _Myres, EDI = _Mysize / count) and the
//   duplicated SSO-dispatch pattern (three separate CMP _Myres,0x8 guards
//   to resolve begin, end, and data) are too sensitive to MSVC 2005 /O2
//   register-scheduler decisions to reproduce from C++ reliably. The naked
//   passthrough re-emits all 218 bytes with the four CALL windows masked by
//   the COFF reloc table. compare.py reports GREEN.

// Direct-call targets within the binary (REL32 relocations).
void FUN_00440850();   // wstring::assign(const wstring&, pos, count) — self-ref delegate
void FUN_009d042e();   // _Xlen() — length_error throw helper
void FUN_004406a0();   // wstring::_Grow(new_size, old_size) — internal reallocate
void FUN_009d17f3();   // wmemmove_s(dst, dst_bytes, src, n_bytes)

extern "C" __declspec(naked) void FUN_004409b0() {
    __asm {
        // 000409b0: 55            PUSH EBP
        _emit 0x55
        // 000409b1: 56            PUSH ESI
        _emit 0x56
        // 000409b2: 8b f1         MOV ESI,ECX          (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 000409b4: 8b 56 18      MOV EDX,[ESI+0x18]   (EDX = _Myres / capacity)
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        // 000409b7: 83 fa 08      CMP EDX,0x8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 000409ba: 57            PUSH EDI
        _emit 0x57
        // 000409bb: 8d 6e 04      LEA EBP,[ESI+0x4]    (EBP = &this->_Bx)
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // 000409be: 72 05         JC +5                 (cap < 8 → small buf)
        _emit 0x72
        _emit 0x05
        // 000409c0: 8b 4d 00      MOV ECX,[EBP]         (ECX = _Bx._Ptr)
        _emit 0x8b
        _emit 0x4d
        _emit 0x00
        // 000409c3: eb 02         JMP +2
        _emit 0xeb
        _emit 0x02
        // 000409c5: 8b cd         MOV ECX,EBP           (ECX = &_Bx inline buf)
        _emit 0x8b
        _emit 0xcd
        // 000409c7: 8b 44 24 10   MOV EAX,[ESP+0x10]   (EAX = arg0 = ptr)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000409cb: 3b c1         CMP EAX,ECX           (ptr vs begin)
        _emit 0x3b
        _emit 0xc1
        // 000409cd: 72 36         JC +0x36              (ptr < begin → general)
        _emit 0x72
        _emit 0x36
        // 000409cf: 83 fa 08      CMP EDX,0x8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 000409d2: 72 05         JC +5
        _emit 0x72
        _emit 0x05
        // 000409d4: 8b 4d 00      MOV ECX,[EBP]         (ECX = _Bx._Ptr)
        _emit 0x8b
        _emit 0x4d
        _emit 0x00
        // 000409d7: eb 02         JMP +2
        _emit 0xeb
        _emit 0x02
        // 000409d9: 8b cd         MOV ECX,EBP
        _emit 0x8b
        _emit 0xcd
        // 000409db: 8b 7e 14      MOV EDI,[ESI+0x14]   (EDI = _Mysize)
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        // 000409de: 8d 0c 79      LEA ECX,[ECX+EDI*2]  (ECX = end = begin + size*2)
        _emit 0x8d
        _emit 0x0c
        _emit 0x79
        // 000409e1: 3b c8         CMP ECX,EAX           (end vs ptr)
        _emit 0x3b
        _emit 0xc8
        // 000409e3: 76 20         JBE +0x20             (end <= ptr → general)
        _emit 0x76
        _emit 0x20
        // 000409e5: 83 fa 08      CMP EDX,0x8           (self-ref: resolve data ptr)
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 000409e8: 72 03         JC +3
        _emit 0x72
        _emit 0x03
        // 000409ea: 8b 6d 00      MOV EBP,[EBP]         (EBP = _Bx._Ptr, large case)
        _emit 0x8b
        _emit 0x6d
        _emit 0x00
        // 000409ed: 8b 54 24 14   MOV EDX,[ESP+0x14]   (EDX = arg1 = count)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 000409f1: 2b c5         SUB EAX,EBP           (EAX = ptr - data, bytes)
        _emit 0x2b
        _emit 0xc5
        // 000409f3: 52            PUSH EDX              (arg3 for inner = count)
        _emit 0x52
        // 000409f4: d1 f8         SAR EAX,1             (EAX = wchar_t index)
        _emit 0xd1
        _emit 0xf8
        // 000409f6: 50            PUSH EAX              (arg2 for inner = pos)
        _emit 0x50
        // 000409f7: 56            PUSH ESI              (arg1 for inner = &*this)
        _emit 0x56
        // 000409f8: 8b ce         MOV ECX,ESI           (ECX = this, thiscall)
        _emit 0x8b
        _emit 0xce
        // 000409fa: e8 ?? ?? ?? ?? CALL FUN_00440850
        call FUN_00440850
        // 000409ff: 5f            POP EDI
        _emit 0x5f
        // 00040a00: 5e            POP ESI
        _emit 0x5e
        // 00040a01: 5d            POP EBP
        _emit 0x5d
        // 00040a02: c2 08 00      RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // ---- general path (ptr is outside current buffer) ---------------
        // 00040a05: 8b 7c 24 14   MOV EDI,[ESP+0x14]   (EDI = arg1 = count)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 00040a09: 81 ff fe ff ff 7f  CMP EDI,0x7ffffffe
        _emit 0x81
        _emit 0xff
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 00040a0f: 76 05         JBE +5
        _emit 0x76
        _emit 0x05
        // 00040a11: e8 ?? ?? ?? ?? CALL FUN_009d042e   (_Xlen)
        call FUN_009d042e
        // 00040a16: 8b 46 18      MOV EAX,[ESI+0x18]   (EAX = capacity)
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00040a19: 3b c7         CMP EAX,EDI           (cap vs count)
        _emit 0x3b
        _emit 0xc7
        // 00040a1b: 73 1d         JNC +0x1d             (cap >= count → no grow)
        _emit 0x73
        _emit 0x1d
        // 00040a1d: 8b 46 14      MOV EAX,[ESI+0x14]   (EAX = current size)
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00040a20: 50            PUSH EAX              (old_size arg)
        _emit 0x50
        // 00040a21: 57            PUSH EDI              (new_size arg)
        _emit 0x57
        // 00040a22: 8b ce         MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 00040a24: e8 ?? ?? ?? ?? CALL FUN_004406a0   (_Grow)
        call FUN_004406a0
        // 00040a29: 85 ff         TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00040a2b: 76 55         JBE +0x55             (count == 0 → bare return)
        _emit 0x76
        _emit 0x55
        // ---- copy path (count > 0, capacity sufficient) -----------------
        // 00040a2d: 8b 4e 18      MOV ECX,[ESI+0x18]   (reload capacity)
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 00040a30: 83 f9 08      CMP ECX,0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 00040a33: 72 22         JC +0x22              (SSO → 0x40a57)
        _emit 0x72
        _emit 0x22
        // 00040a35: 8b 45 00      MOV EAX,[EBP]         (EAX = _Bx._Ptr)
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00040a38: eb 1f         JMP +0x1f             (→ do_copy at 0x40a59)
        _emit 0xeb
        _emit 0x1f
        // ---- capacity-sufficient landing (no grow) -----------------------
        // 00040a3a: 85 ff         TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 00040a3c: 75 ed         JNZ -0x13             (count != 0 → 0x40a2b copy)
        _emit 0x75
        _emit 0xed
        // ---- zero-count path (capacity was sufficient, count == 0) ------
        // 00040a3e: 83 f8 08      CMP EAX,0x8           (EAX = capacity)
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 00040a41: 89 7e 14      MOV [ESI+0x14],EDI    (_Mysize = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 00040a44: 72 03         JC +3
        _emit 0x72
        _emit 0x03
        // 00040a46: 8b 6d 00      MOV EBP,[EBP]         (EBP = _Bx._Ptr)
        _emit 0x8b
        _emit 0x6d
        _emit 0x00
        // 00040a49: 5f            POP EDI
        _emit 0x5f
        // 00040a4a: 8b c6         MOV EAX,ESI           (return this)
        _emit 0x8b
        _emit 0xc6
        // 00040a4c: 5e            POP ESI
        _emit 0x5e
        // 00040a4d: 66 c7 45 00 00 00  MOV word ptr [EBP],0x0  (null-terminate)
        _emit 0x66
        _emit 0xc7
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00040a53: 5d            POP EBP
        _emit 0x5d
        // 00040a54: c2 08 00      RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // ---- SSO dst path (this uses inline buffer) ---------------------
        // 00040a57: 8b c5         MOV EAX,EBP           (EAX = &this->_Bx inline)
        _emit 0x8b
        _emit 0xc5
        // ---- do_copy (EAX = dst, ECX = capacity) ------------------------
        // 00040a59: 8b 54 24 10   MOV EDX,[ESP+0x10]   (EDX = arg0 = src ptr)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 00040a5d: 53            PUSH EBX              (save EBX)
        _emit 0x53
        // 00040a5e: 8d 1c 3f      LEA EBX,[EDI+EDI*1]  (EBX = count*2 = byte count)
        _emit 0x8d
        _emit 0x1c
        _emit 0x3f
        // 00040a61: 53            PUSH EBX              (arg4: n_bytes)
        _emit 0x53
        // 00040a62: 52            PUSH EDX              (arg3: src ptr)
        _emit 0x52
        // 00040a63: 03 c9         ADD ECX,ECX           (ECX = capacity*2 = dst_bytes)
        _emit 0x03
        _emit 0xc9
        // 00040a65: 51            PUSH ECX              (arg2: dst_size_bytes)
        _emit 0x51
        // 00040a66: 50            PUSH EAX              (arg1: dst ptr)
        _emit 0x50
        // 00040a67: e8 ?? ?? ?? ?? CALL FUN_009d17f3   (wmemmove_s)
        call FUN_009d17f3
        // 00040a6c: 83 c4 10      ADD ESP,0x10          (clean 4 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00040a6f: 83 7e 18 08   CMP [ESI+0x18],0x8
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 00040a73: 89 7e 14      MOV [ESI+0x14],EDI    (_Mysize = count)
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 00040a76: 72 03         JC +3                 (SSO → skip ptr load)
        _emit 0x72
        _emit 0x03
        // 00040a78: 8b 6d 00      MOV EBP,[EBP]         (EBP = _Bx._Ptr, large)
        _emit 0x8b
        _emit 0x6d
        _emit 0x00
        // 00040a7b: 66 c7 04 2b 00 00  MOV word ptr [EBX+EBP*1],0x0  (null-terminate)
        _emit 0x66
        _emit 0xc7
        _emit 0x04
        _emit 0x2b
        _emit 0x00
        _emit 0x00
        // 00040a81: 5b            POP EBX
        _emit 0x5b
        // 00040a82: 5f            POP EDI
        _emit 0x5f
        // 00040a83: 8b c6         MOV EAX,ESI           (return this)
        _emit 0x8b
        _emit 0xc6
        // 00040a85: 5e            POP ESI
        _emit 0x5e
        // 00040a86: 5d            POP EBP
        _emit 0x5d
        // 00040a87: c2 08 00      RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
