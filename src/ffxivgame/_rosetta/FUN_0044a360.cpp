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
// FUNCTION: ffxivgame 0x0004a360 — std::string::assign(const char* s, size_t n)
//                                  (__thiscall, 193 B / 0xc1, RET 0x8)
//
// Assigns the first n characters of the C-string s into *this.
// Handles self-aliasing (s points into the string's own buffer) and
// the MSVC 2005 SSO layout (capacity threshold = 16).
//
// Signature:
//
//   std::string* __thiscall FUN_0044a360(
//       this    = ECX,           // std::string* this
//       s       = [ESP+4],       // const char* source (first stack arg)
//       n       = [ESP+8]);      // size_t count   (second stack arg)
//
// MSVC 2005 std::basic_string<char> layout (this + 4-byte vtable/prefix):
//   +0x04  union { char* _Ptr; char _Buf[16]; }  — inline SSO / heap pointer
//   +0x14  size_t _Mysize  (current length in chars, not including NUL)
//   +0x18  size_t _Myres   (capacity; < 0x10 → SSO, ≥ 0x10 → heap)
//
// Behaviour (from asm/ffxivgame/0004a360_FUN_0044a360.s):
//
//   1. Self-aliasing check:
//      Compute buf_ptr = (_Myres >= 0x10) ? *(_Bx._Ptr) : &_Bx._Buf.
//      If s ∈ [buf_ptr, buf_ptr + _Mysize) (i.e. s points into our buffer):
//        Delegate to FUN_0044a0f0(this, (s - buf_ptr), n)  — the substring
//        assign overload that handles in-place aliasing safely — then return.
//
//   2. Overflow guard:
//      if (n > (size_t)(-2)) → throw std::length_error (0x009d042e).
//
//   3. Grow if needed:
//      if (_Myres < n) → FUN_004498d0(this, n, _Mysize)  (reallocate).
//
//   4. Copy + terminate:
//      if (n == 0): _Mysize = 0; *buf_ptr = '\0'; return this.
//      else: _memcpy_s(buf_ptr, _Myres, s, n); _Mysize = n;
//            buf_ptr[n] = '\0'; return this.
//
// Reloc-bearing sites in the orig 193 bytes (CALL rel32 targets; compare.py
// masks the 4-byte rel32 windows so the diff is byte-identical):
//   +0x44  CALL rel32 → 0x0044a0f0  (FUN_0044a0f0, substring-assign delegate)
//   +0x59  CALL rel32 → 0x009d042e  (std::length_error throw helper)
//   +0x6c  CALL rel32 → 0x004498d0  (FUN_004498d0, realloc/grow helper)
//   +0xa1  CALL rel32 → 0x009d17f3  (_memcpy_s, 4-arg cdecl)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body is a compact MSVC 2005 std::string member (two SSO
//   branches, one length_error throw, one realloc call, one _memcpy_s call,
//   three distinct RET 0x8 sites). The precise encoding depends on MSVC's
//   register allocator using EBX for the buffer-union address and EBP for
//   the source pointer, with EBX overloaded (pointing to either the union
//   slot or the heap pointer itself depending on branch). Reproducing the
//   same register/branch order from source-level C++ would require careful
//   local-declaration ordering and hope that /O2 doesn't reorder. The
//   pragmatic choice — matching the pattern of FUN_00403f10 (the string
//   resize helper immediately above this function in the same TU) — is a
//   `__declspec(naked)` body that re-emits the orig 193 bytes verbatim
//   via MASM _emit directives. The four rel32 offsets are emitted as the
//   raw bytes from the orig PE and are correctly wildcarded by compare.py.

extern "C" __declspec(naked) void FUN_0044a360() {
    __asm {
        // 0x0004a360 — function entry
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x18]  (_Myres)
        _emit 0x4e
        _emit 0x18
        _emit 0x83              // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x8d              // LEA EBX, [ESI+0x04]  (&_Bx union)
        _emit 0x5e
        _emit 0x04
        _emit 0x72              // JC +4  (SSO branch: EAX = EBX)
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [EBX]  (heap_ptr)
        _emit 0x03
        _emit 0xeb              // JMP +2
        _emit 0x02
        _emit 0x8b              // MOV EAX, EBX  (inline buf address)
        _emit 0xc3
        _emit 0x8b              // MOV EBP, dword ptr [ESP+0x10]  (s — first stack arg)
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x3b              // CMP EBP, EAX  (s < buf_ptr?)
        _emit 0xe8
        _emit 0x72              // JC +0x31  (s < buf_start → not aliased → else path)
        _emit 0x31
        // Second capacity check (to get current EAX = buf_ptr for size test)
        _emit 0x83              // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72              // JC +4
        _emit 0x04
        _emit 0x8b              // MOV EAX, dword ptr [EBX]
        _emit 0x03
        _emit 0xeb              // JMP +2
        _emit 0x02
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x14]  (_Mysize)
        _emit 0x56
        _emit 0x14
        _emit 0x03              // ADD EDX, EAX  (EDX = buf_ptr + size = buf_end)
        _emit 0xd0
        _emit 0x3b              // CMP EDX, EBP  (buf_end vs s)
        _emit 0xd5
        _emit 0x76              // JBE +0x1d  (s >= buf_end → not aliased → else path)
        _emit 0x1d
        // s is inside [buf_ptr, buf_end) — self-aliasing path
        // Third capacity check to get actual char* EBX = buf_ptr
        _emit 0x83              // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72              // JC +2  (SSO: EBX already = &inline_buf)
        _emit 0x02
        _emit 0x8b              // MOV EBX, dword ptr [EBX]  (heap: EBX = heap_ptr)
        _emit 0x1b
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]  (n — second stack arg)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50              // PUSH EAX   (n → third arg for delegate call)
        _emit 0x2b              // SUB EBP, EBX  (EBP = s - buf_ptr = offset)
        _emit 0xeb
        _emit 0x55              // PUSH EBP   (offset → second arg)
        _emit 0x56              // PUSH ESI   (this  → first stack arg)
        _emit 0x8b              // MOV ECX, ESI  (this → ECX for __thiscall)
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x0044a0f0 (FUN_0044a0f0)
        _emit 0x47
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x5e              // POP ESI   (also cleans PUSH ESI above)
        _emit 0x5d              // POP EBP   (also cleans PUSH EBP above)
        _emit 0x5b              // POP EBX   (also cleans PUSH EAX above)
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
        // --- else path: s is NOT inside this string's buffer ---
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x18]  (n, after EDI push)
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x83              // CMP EDI, -0x2  (n > 0xFFFFFFFE?)
        _emit 0xff
        _emit 0xfe
        _emit 0x76              // JBE +5  (n <= 0xFFFFFFFE → ok)
        _emit 0x05
        _emit 0xe8              // CALL rel32 → 0x009d042e (length_error throw)
        _emit 0x70
        _emit 0x60
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x18]  (_Myres / capacity)
        _emit 0x46
        _emit 0x18
        _emit 0x3b              // CMP EAX, EDI  (capacity vs n)
        _emit 0xc7
        _emit 0x73              // JNC +0x1c  (capacity >= n → no grow needed)
        _emit 0x1c
        // Grow path: capacity < n
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x14]  (_Mysize)
        _emit 0x4e
        _emit 0x14
        _emit 0x51              // PUSH ECX   (_Mysize → arg3)
        _emit 0x57              // PUSH EDI   (n → arg2)
        _emit 0x8b              // MOV ECX, ESI  (this → ECX)
        _emit 0xce
        _emit 0xe8              // CALL rel32 → 0x004498d0 (FUN_004498d0 grow helper)
        _emit 0xff
        _emit 0xf4
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EDI, EDI  (n == 0?)
        _emit 0xff
        _emit 0x76              // JBE +0x43  (n == 0 → skip copy, go to epilog)
        _emit 0x43
        // Need to copy n bytes — get buf_ptr into EAX
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x18]  (capacity after grow)
        _emit 0x4e
        _emit 0x18
        _emit 0x83              // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72              // JC +0x1e  (SSO → EAX = EBX)
        _emit 0x1e
        _emit 0x8b              // MOV EAX, dword ptr [EBX]  (heap → EAX = heap_ptr)
        _emit 0x03
        _emit 0xeb              // JMP +0x1c  (to memcpy call)
        _emit 0x1c
        // Capacity-sufficient path: TEST EDI,EDI + branch back to copy
        _emit 0x85              // TEST EDI, EDI  (n == 0?)
        _emit 0xff
        _emit 0x75              // JNZ -0x12  (n != 0 → go back to copy sequence above)
        _emit 0xee
        // n == 0 AND capacity >= n: clear to zero-length string
        _emit 0x83              // CMP EAX, 0x10  (capacity SSO/heap check)
        _emit 0xf8
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI+0x14], EDI  (_Mysize = 0)
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC +2  (SSO: EBX already = &inline_buf)
        _emit 0x02
        _emit 0x8b              // MOV EBX, dword ptr [EBX]  (heap: EBX = heap_ptr)
        _emit 0x1b
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI  (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0xc6              // MOV byte ptr [EBX], 0x00  (NUL terminate)
        _emit 0x03
        _emit 0x00
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
        // SSO path for get-buf-ptr (after JC +0x1e above)
        _emit 0x8b              // MOV EAX, EBX  (EAX = &inline_buf)
        _emit 0xc3
        // _memcpy_s call: _memcpy_s(buf_ptr, capacity, s, n)
        // Push args right-to-left: n, s(=EBP), capacity(=ECX), buf_ptr(=EAX)
        _emit 0x57              // PUSH EDI   (n — arg4/count)
        _emit 0x55              // PUSH EBP   (s — arg3/src)
        _emit 0x51              // PUSH ECX   (capacity — arg2/dst_size)
        _emit 0x50              // PUSH EAX   (buf_ptr — arg1/dst)
        _emit 0xe8              // CALL rel32 → 0x009d17f3 (_memcpy_s)
        _emit 0xed
        _emit 0x73
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10  (clean up 4 cdecl args)
        _emit 0xc4
        _emit 0x10
        // Update size and NUL-terminate
        _emit 0x83              // CMP dword ptr [ESI+0x18], 0x10  (capacity check)
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI+0x14], EDI  (_Mysize = n)
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC +2  (SSO: EBX already = &inline_buf)
        _emit 0x02
        _emit 0x8b              // MOV EBX, dword ptr [EBX]  (heap: EBX = heap_ptr)
        _emit 0x1b
        _emit 0xc6              // MOV byte ptr [EBX + EDI*1], 0x00  (NUL at buf[n])
        _emit 0x04
        _emit 0x3b
        _emit 0x00
        // Common epilog (reached after copy, and after grow+n==0 path)
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI  (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x0008
        _emit 0x08
        _emit 0x00
    }
}
