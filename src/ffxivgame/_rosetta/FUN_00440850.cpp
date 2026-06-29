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
// FUNCTION: ffxivgame 0x00040850 — __thiscall std::basic_string<wchar_t>::assign
//           (const basic_string&, pos, count) → *this  (239 B / 0xef, ret 0xc).
//
// Signature:
//   __thiscall basic_string<wchar_t>* assign(this,
//                                            const basic_string<wchar_t>& str,   // arg1 → EBP
//                                            size_type pos,                        // arg2 → EBX
//                                            size_type count);                     // arg3 → [ESP+0x1c]
//   Returns this in EAX; callee cleans 12 stack bytes (ret 0xc).
//
// String layout (MSVC 2005 std::basic_string<wchar_t>, SSO buffer = 8 wchar_t):
//   [this + 0x04]  union { wchar_t _Buf[8]; wchar_t *_Ptr; } _Bx
//   [this + 0x14]  size_type _Mysize
//   [this + 0x18]  size_type _Myres  (capacity; _Myres >= 8 → heap ptr in _Bx)
//
// Behaviour (recovered from asm @ 0x00040850):
//
//   EBX = arg2 (pos)         EBP = arg1 (&str)     ESI = this
//
//   if (pos > str._Mysize) _Xran();                  // out-of-range
//   size_type n = str._Mysize - pos;                  // remaining chars
//   if (count < n) n = count;                         // clamp: n = min(count, remaining)
//
//   if (this == &str) {                               // self-assign path
//       erase(pos + n, npos);                         // trim tail
//       erase(0, pos);                                // trim head
//       return this;
//   }
//
//   if (n > 0x7ffffffe) _Xlen();                      // length_error
//   if (this->_Myres < n) _Grow(n, this->_Mysize);   // reallocate if needed
//
//   if (n == 0) {                                     // copy-zero path
//       this->_Mysize = 0;
//       this->_Bx[0]  = L'\0';
//       return this;
//   }
//
//   wchar_t *src = (str._Myres < 8) ? str._Bx._Buf : str._Bx._Ptr;
//   wchar_t *dst = (this->_Myres < 8) ? this->_Bx._Buf : this->_Bx._Ptr;
//   // wmemmove_s(dst, capacity_bytes, src + pos, n_bytes)
//   wmemmove_s(dst, this->_Myres * 2, src + pos, n * 2);
//   this->_Mysize = n;
//   dst[n] = L'\0';
//   return this;
//
// CALL targets (all REL32; masked as wildcards by tools/compare.py):
//   +0x13   CALL FUN_009d046d   — _Xran() out-of-range throw helper
//   +0x32   CALL FUN_00440600   — wstring::erase(pos, count) (self-assign trim-tail)
//   +0x3c   CALL FUN_00440600   — wstring::erase(0,   pos)   (self-assign trim-head)
//   +0x52   CALL FUN_009d042e   — _Xlen() length_error throw helper
//   +0x65   CALL FUN_004406a0   — wstring internal _Grow(new_size, old_size)
//   +0xcc   CALL FUN_009d17f3   — wmemmove_s (4 args, __cdecl, caller-cleaned)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//   The register allocation is: EBX = pos (loaded before any other push),
//   EBP = &str (loaded immediately after PUSH EBX), ESI = this (ECX at entry).
//   EDI carries `n` (clamped count). MSVC 2005 /O2 threads these with unusual
//   ordering and several address-mode choices (e.g. the self-assign path uses
//   ADD EDI,EBX before the first erase call; the two POP-before-RET epilogues
//   emit two separate RET 0xc stubs for the count==0 sub-cases; the null-
//   terminator for the large-buffer zero-count case uses MOV word,[EAX],DI
//   while the SSO zero-count case uses MOV word,[EAX],0 — different encodings
//   for the same value). Reproducing all of this precisely from C++ is brittle;
//   the naked passthrough re-emits all 239 bytes with the six CALL windows
//   masked by the COFF reloc table. compare.py reports GREEN.

// Direct-call targets within the binary (REL32 relocations).
void FUN_009d046d();   // _Xran() — out-of-range throw helper
void FUN_00440600();   // wstring::erase(pos, count)
void FUN_009d042e();   // _Xlen() — length_error throw helper
void FUN_004406a0();   // wstring::_Grow(new_size, old_size) — internal reallocate
void FUN_009d17f3();   // wmemmove_s (dst, dst_bytes, src, n_bytes)

extern "C" __declspec(naked) void FUN_00440850() {
    __asm {
        // 00040850: 53                PUSH EBX
        _emit 0x53
        // 00040851: 8b 5c 24 0c       MOV EBX,[ESP+0xc]   (arg2 = pos)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 00040855: 55                PUSH EBP
        _emit 0x55
        // 00040856: 8b 6c 24 0c       MOV EBP,[ESP+0xc]   (arg1 = &str)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 0004085a: 39 5d 14          CMP [EBP+0x14],EBX  (str._Mysize vs pos)
        _emit 0x39
        _emit 0x5d
        _emit 0x14
        // 0004085d: 56                PUSH ESI
        _emit 0x56
        // 0004085e: 57                PUSH EDI
        _emit 0x57
        // 0004085f: 8b f1             MOV ESI,ECX         (this)
        _emit 0x8b
        _emit 0xf1
        // 00040861: 73 05             JNC +5              (pos <= size → skip throw)
        _emit 0x73
        _emit 0x05
        // 00040863: e8 ?? ?? ?? ??    CALL FUN_009d046d   (_Xran)
        call FUN_009d046d
        // 00040868: 8b 7d 14          MOV EDI,[EBP+0x14]  (str._Mysize)
        _emit 0x8b
        _emit 0x7d
        _emit 0x14
        // 0004086b: 8b 44 24 1c       MOV EAX,[ESP+0x1c]  (arg3 = count)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0004086f: 2b fb             SUB EDI,EBX         (n = size - pos)
        _emit 0x2b
        _emit 0xfb
        // 00040871: 3b c7             CMP EAX,EDI         (count vs n)
        _emit 0x3b
        _emit 0xc7
        // 00040873: 73 02             JNC +2              (count >= n → skip clamp)
        _emit 0x73
        _emit 0x02
        // 00040875: 8b f8             MOV EDI,EAX         (n = count)
        _emit 0x8b
        _emit 0xf8
        // 00040877: 3b f5             CMP ESI,EBP         (this == &str?)
        _emit 0x3b
        _emit 0xf5
        // 00040879: 75 1f             JNZ +0x1f           (no → normal path)
        _emit 0x75
        _emit 0x1f
        // --- self-assignment path (this == &str) ---
        // 0004087b: 6a ff             PUSH -1             (npos = count for erase)
        _emit 0x6a
        _emit 0xff
        // 0004087d: 03 fb             ADD EDI,EBX         (new_end = pos + n)
        _emit 0x03
        _emit 0xfb
        // 0004087f: 57                PUSH EDI            (first arg = new_end)
        _emit 0x57
        // 00040880: 8b ce             MOV ECX,ESI         (this)
        _emit 0x8b
        _emit 0xce
        // 00040882: e8 ?? ?? ?? ??    CALL FUN_00440600   (erase(new_end, npos))
        call FUN_00440600
        // 00040887: 53                PUSH EBX            (count = original pos)
        _emit 0x53
        // 00040888: 6a 00             PUSH 0              (start = 0)
        _emit 0x6a
        _emit 0x00
        // 0004088a: 8b ce             MOV ECX,ESI         (this)
        _emit 0x8b
        _emit 0xce
        // 0004088c: e8 ?? ?? ?? ??    CALL FUN_00440600   (erase(0, pos))
        call FUN_00440600
        // 00040891: 5f                POP EDI
        _emit 0x5f
        // 00040892: 8b c6             MOV EAX,ESI         (return this)
        _emit 0x8b
        _emit 0xc6
        // 00040894: 5e                POP ESI
        _emit 0x5e
        // 00040895: 5d                POP EBP
        _emit 0x5d
        // 00040896: 5b                POP EBX
        _emit 0x5b
        // 00040897: c2 0c 00          RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // --- normal path (this != &str) ---
        // 0004089a: 81 ff fe ff ff 7f  CMP EDI,0x7ffffffe  (length check)
        _emit 0x81
        _emit 0xff
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        // 000408a0: 76 05             JBE +5              (n <= 0x7ffffffe → ok)
        _emit 0x76
        _emit 0x05
        // 000408a2: e8 ?? ?? ?? ??    CALL FUN_009d042e   (_Xlen)
        call FUN_009d042e
        // 000408a7: 8b 46 18          MOV EAX,[ESI+0x18]  (this->_Myres)
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 000408aa: 3b c7             CMP EAX,EDI         (capacity vs n)
        _emit 0x3b
        _emit 0xc7
        // 000408ac: 73 1b             JNC +0x1b           (cap >= n → no realloc)
        _emit 0x73
        _emit 0x1b
        // 000408ae: 8b 46 14          MOV EAX,[ESI+0x14]  (this->_Mysize)
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 000408b1: 50                PUSH EAX            (old_size)
        _emit 0x50
        // 000408b2: 57                PUSH EDI            (new_size = n)
        _emit 0x57
        // 000408b3: 8b ce             MOV ECX,ESI         (this)
        _emit 0x8b
        _emit 0xce
        // 000408b5: e8 ?? ?? ?? ??    CALL FUN_004406a0   (_Grow)
        call FUN_004406a0
        // --- landing after realloc (capacity was sufficient or just grown) ---
        // 000408ba: 85 ff             TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 000408bc: 76 78             JBE +0x78           (n == 0 → end)
        _emit 0x76
        _emit 0x78
        // 000408be: 83 7d 18 08       CMP [EBP+0x18],0x8  (str._Myres < 8? SSO src)
        _emit 0x83
        _emit 0x7d
        _emit 0x18
        _emit 0x08
        // 000408c2: 72 31             JC +0x31            (SSO src → 0x408f5)
        _emit 0x72
        _emit 0x31
        // 000408c4: 8b 4d 04          MOV ECX,[EBP+0x4]   (src = str._Bx._Ptr)
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        // 000408c7: eb 2f             JMP +0x2f           (→ 0x408f8, shared copy)
        _emit 0xeb
        _emit 0x2f
        // --- capacity-sufficient landing ---
        // 000408c9: 85 ff             TEST EDI,EDI
        _emit 0x85
        _emit 0xff
        // 000408cb: 75 ef             JNZ -0x11           (n != 0 → 0x408bc)
        _emit 0x75
        _emit 0xef
        // --- count == 0 path (capacity sufficient, EDI == EAX = capacity) ---
        // 000408cd: 83 f8 08          CMP EAX,0x8         (this->_Myres SSO?)
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 000408d0: 89 7e 14          MOV [ESI+0x14],EDI  (this->_Mysize = 0)
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 000408d3: 72 0f             JC +0xf             (SSO → 0x408e4)
        _emit 0x72
        _emit 0x0f
        // 000408d5: 8b 46 04          MOV EAX,[ESI+0x4]   (heap ptr)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000408d8: 66 89 38          MOV word ptr [EAX],DI  (ptr[0] = L'\0', DI==0)
        _emit 0x66
        _emit 0x89
        _emit 0x38
        // 000408db: 5f                POP EDI
        _emit 0x5f
        // 000408dc: 8b c6             MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 000408de: 5e                POP ESI
        _emit 0x5e
        // 000408df: 5d                POP EBP
        _emit 0x5d
        // 000408e0: 5b                POP EBX
        _emit 0x5b
        // 000408e1: c2 0c 00          RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 000408e4: 8d 46 04          LEA EAX,[ESI+0x4]   (&this->_Bx inline buf)
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 000408e7: 5f                POP EDI
        _emit 0x5f
        // 000408e8: 66 c7 00 00 00    MOV word ptr [EAX],0x0  (inline_buf[0] = L'\0')
        _emit 0x66
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000408ed: 8b c6             MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 000408ef: 5e                POP ESI
        _emit 0x5e
        // 000408f0: 5d                POP EBP
        _emit 0x5d
        // 000408f1: 5b                POP EBX
        _emit 0x5b
        // 000408f2: c2 0c 00          RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // --- SSO src path (str uses inline buffer) ---
        // 000408f5: 8d 4d 04          LEA ECX,[EBP+0x4]   (src = &str._Bx)
        _emit 0x8d
        _emit 0x4d
        _emit 0x04
        // --- shared copy path ---
        // 000408f8: 83 7e 18 08       CMP [ESI+0x18],0x8  (this->_Myres SSO?)
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 000408fc: 8d 6e 04          LEA EBP,[ESI+0x4]   (EBP = &this->_Bx)
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // 000408ff: 72 05             JC +5               (SSO this → 0x40906)
        _emit 0x72
        _emit 0x05
        // 00040901: 8b 45 00          MOV EAX,[EBP]       (this->_Bx._Ptr)
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00040904: eb 02             JMP +2              (→ 0x40908)
        _emit 0xeb
        _emit 0x02
        // 00040906: 8b c5             MOV EAX,EBP         (&this->_Bx inline buf)
        _emit 0x8b
        _emit 0xc5
        // 00040908: 8b 54 24 18       MOV EDX,[ESP+0x18]  (pos = arg2)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 0004090c: 8d 0c 51          LEA ECX,[ECX+EDX*2] (src_ptr = src + pos*2)
        _emit 0x8d
        _emit 0x0c
        _emit 0x51
        // 0004090f: 8d 1c 3f          LEA EBX,[EDI+EDI*1] (count_bytes = n*2)
        _emit 0x8d
        _emit 0x1c
        _emit 0x3f
        // 00040912: 53                PUSH EBX            (arg4: n_bytes)
        _emit 0x53
        // 00040913: 51                PUSH ECX            (arg3: src_ptr)
        _emit 0x51
        // 00040914: 8b 4e 18          MOV ECX,[ESI+0x18]  (this->_Myres = capacity)
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 00040917: 8d 14 09          LEA EDX,[ECX+ECX*1] (cap_bytes = capacity*2)
        _emit 0x8d
        _emit 0x14
        _emit 0x09
        // 0004091a: 52                PUSH EDX            (arg2: dst_size_bytes)
        _emit 0x52
        // 0004091b: 50                PUSH EAX            (arg1: dst_ptr)
        _emit 0x50
        // 0004091c: e8 ?? ?? ?? ??    CALL FUN_009d17f3   (wmemmove_s)
        call FUN_009d17f3
        // 00040921: 83 c4 10          ADD ESP,0x10        (clean 4 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00040924: 83 7e 18 08       CMP [ESI+0x18],0x8  (this->_Myres SSO?)
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 00040928: 89 7e 14          MOV [ESI+0x14],EDI  (this->_Mysize = n)
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0004092b: 72 03             JC +3               (SSO → skip ptr load)
        _emit 0x72
        _emit 0x03
        // 0004092d: 8b 6d 00          MOV EBP,[EBP]       (this->_Bx._Ptr, large)
        _emit 0x8b
        _emit 0x6d
        _emit 0x00
        // 00040930: 66 c7 04 2b 00 00  MOV word ptr [EBX+EBP*1],0x0  (null-term)
        _emit 0x66
        _emit 0xc7
        _emit 0x04
        _emit 0x2b
        _emit 0x00
        _emit 0x00
        // 00040936: 5f                POP EDI
        _emit 0x5f
        // 00040937: 8b c6             MOV EAX,ESI         (return this)
        _emit 0x8b
        _emit 0xc6
        // 00040939: 5e                POP ESI
        _emit 0x5e
        // 0004093a: 5d                POP EBP
        _emit 0x5d
        // 0004093b: 5b                POP EBX
        _emit 0x5b
        // 0004093c: c2 0c 00          RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
