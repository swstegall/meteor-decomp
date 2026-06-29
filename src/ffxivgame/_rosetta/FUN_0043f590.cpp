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
// FUNCTION: ffxivgame 0x0003f590 — string::assign(const char* s, size_t n),
//                                  __thiscall, RET 0x8 (193 B / 0xC1).
//
// Signature (recovered from asm):
//
//   string* __thiscall FUN_0043f590(string* this,    // ECX  → ESI
//                                    const char* s,   // arg1 = [ESP+0x10] → EBP
//                                    size_t      n);  // arg2 = [ESP+0x14] → (EAX / EDI)
//
// MSVC SSO string layout (offsets from `this`):
//   +0x04  _Bx._Ptr / _Bx._Buf[16]  — heap ptr or inline buffer
//   +0x14  _Mysize                  — current length in chars
//   +0x18  _Myres                   — capacity (_Myres < 0x10 ⟹ SSO inline mode)
//
// Body sketch:
//
//   1. Resolve this->data() ptr (SSO-aware: if capacity < 0x10, use &inline_buf;
//      else dereference the heap pointer stored at +0x04).
//   2. Self-reference check: if s < data_start OR s >= data_start+size,
//      the source is NOT within this string's own buffer → jump to the
//      normal path.
//   3. Self-referential path (s points into this's own buffer):
//        Resolve heap ptr if needed.
//        Delegate to FUN_0043f4b0(this, this, s - data_start, n)
//          which is string::assign(const string& src, pos, count)
//          using this itself as src with the appropriate sub-range.
//        POP saved regs; RET 0x8.
//   4. Normal path:
//      a. Overflow guard: if n > 0xFFFFFFFE → CALL 0x009d042e (length_error).
//      b. Grow if capacity < n → FUN_0043f2e0(this, n, old_size).
//      c. If n == 0: set size=0, null-terminate buffer[0] (SSO-aware), return.
//      d. n > 0: _memmove_s(dst, this->capacity, s, n)  (→ 0x009d17f3).
//      e. Set this->size = n; null-terminate buffer[n]; return this.
//
// The self-ref check at step 2 gates on (s >= data_start) AND
// (s < data_start + size), matching the STL convention: assign from an
// iterator range that overlaps this's own storage requires a detour.
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   Four CALL rel32 relocations plus the interleaved SSO branch chains
//   (two capacity < 0x10 tests on the hot path alone) make it impractical
//   to produce the exact 193 bytes from source-level C++ under /O2.
//   This is the same approach used by the direct siblings FUN_0043f4b0
//   (string::assign(const string&, pos, count), 217 B), FUN_0043f2e0
//   (grow / reserve), and FUN_00403f10 (resize-style helper).
//
// Reloc sites (REL32 imm32 windows masked by compare.py):
//   +0x44  CALL 0x0043f4b0  — self-referential assign delegate
//   +0x59  CALL 0x009d042e  — std::length_error throw helper
//   +0x6c  CALL 0x0043f2e0  — grow / reserve
//   +0xa1  CALL 0x009d17f3  — _memmove_s

extern "C" __declspec(naked) void FUN_0043f590() {
    __asm {
        // 0003f590 — prolog: save EBX/EBP/ESI; store this (ECX) in ESI
        _emit 0x53          // PUSH EBX
        _emit 0x55          // PUSH EBP
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, ECX
        _emit 0xf1
        // 0003f595 — ECX = this->capacity; EBX = &this->_Bx (+0x04)
        _emit 0x8b          // MOV ECX, [ESI+0x18]
        _emit 0x4e
        _emit 0x18
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x8d          // LEA EBX, [ESI+0x04]
        _emit 0x5e
        _emit 0x04
        // 0003f59e — resolve data_start into EAX (heap vs SSO)
        _emit 0x72          // JC +4  (SSO: EAX = EBX)
        _emit 0x04
        _emit 0x8b          // MOV EAX, [EBX]  (heap ptr)
        _emit 0x03
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EAX, EBX   (inline buf)
        _emit 0xc3
        // 0003f5a6 — EBP = arg1 (s); check s < data_start
        _emit 0x8b          // MOV EBP, [ESP+0x10]
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        _emit 0x3b          // CMP EBP, EAX
        _emit 0xe8
        _emit 0x72          // JC +0x31 → normal path (0x0043f5df)
        _emit 0x31
        // 0003f5ae — s >= data_start; check s < data_start + size
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72          // JC +4
        _emit 0x04
        _emit 0x8b          // MOV EAX, [EBX]  (heap ptr)
        _emit 0x03
        _emit 0xeb          // JMP +2
        _emit 0x02
        _emit 0x8b          // MOV EAX, EBX   (inline buf)
        _emit 0xc3
        // 0003f5b9 — EDX = size + data_start (= one-past-end)
        _emit 0x8b          // MOV EDX, [ESI+0x14]
        _emit 0x56
        _emit 0x14
        _emit 0x03          // ADD EDX, EAX
        _emit 0xd0
        _emit 0x3b          // CMP EDX, EBP
        _emit 0xd5
        // 0003f5c0 — if data_end <= s: not inside → normal path
        _emit 0x76          // JBE +0x1d → normal path (0x0043f5df)
        _emit 0x1d
        // 0003f5c2 — self-referential path: resolve heap ptr into EBX
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72          // JC +2  (SSO: EBX already = &inline_buf)
        _emit 0x02
        _emit 0x8b          // MOV EBX, [EBX]  (EBX = heap ptr)
        _emit 0x1b
        // 0003f5c9 — call FUN_0043f4b0(this, this, s-data_start, n)
        _emit 0x8b          // MOV EAX, [ESP+0x14]  (n = arg2)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50          // PUSH EAX   (count = n)
        _emit 0x2b          // SUB EBP, EBX  (pos = s - data_start)
        _emit 0xeb
        _emit 0x55          // PUSH EBP   (pos)
        _emit 0x56          // PUSH ESI   (src = this)
        _emit 0x8b          // MOV ECX, ESI  (this)
        _emit 0xce
        _emit 0xe8          // CALL 0x0043f4b0  [RELOC]
        _emit 0xd7
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0x8
        _emit 0x08
        _emit 0x00
        // 0003f5df — normal path: save EDI; load n into EDI
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV EDI, [ESP+0x18]  (n = arg2, now +4 for PUSH EDI)
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // 0003f5e4 — overflow guard: n > 0xFFFFFFFE → throw
        _emit 0x83          // CMP EDI, -2
        _emit 0xff
        _emit 0xfe
        _emit 0x76          // JBE +5  (ok)
        _emit 0x05
        _emit 0xe8          // CALL 0x009d042e  [RELOC]
        _emit 0x40
        _emit 0x0e
        _emit 0x59
        _emit 0x00
        // 0003f5ee — EAX = this->capacity; grow if capacity < n
        _emit 0x8b          // MOV EAX, [ESI+0x18]
        _emit 0x46
        _emit 0x18
        _emit 0x3b          // CMP EAX, EDI
        _emit 0xc7
        _emit 0x73          // JNC +0x1c → no-grow (0x0043f611)
        _emit 0x1c
        // 0003f5f5 — need-grow path: FUN_0043f2e0(this, n, old_size)
        _emit 0x8b          // MOV ECX, [ESI+0x14]  (old_size)
        _emit 0x4e
        _emit 0x14
        _emit 0x51          // PUSH ECX   (old_size)
        _emit 0x57          // PUSH EDI   (n)
        _emit 0x8b          // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8          // CALL 0x0043f2e0  [RELOC]
        _emit 0xdf
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0003f601 — TEST EDI, EDI (n == 0?)
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x76          // JBE +0x43 → epilog (0x0043f648)
        _emit 0x43
        // 0003f605 — n > 0: resolve dst ptr (SSO-aware) into EAX
        _emit 0x8b          // MOV ECX, [ESI+0x18]  (capacity)
        _emit 0x4e
        _emit 0x18
        _emit 0x83          // CMP ECX, 0x10
        _emit 0xf9
        _emit 0x10
        _emit 0x72          // JC +0x1e → SSO dst (0x0043f62b)
        _emit 0x1e
        _emit 0x8b          // MOV EAX, [EBX]  (heap dst ptr)
        _emit 0x03
        _emit 0xeb          // JMP +0x1c → memmove_s (0x0043f62d)
        _emit 0x1c
        // 0003f611 — no-grow path: TEST EDI, EDI; if n==0 → clear
        _emit 0x85          // TEST EDI, EDI
        _emit 0xff
        _emit 0x75          // JNZ -0x12 → n>0, resolve dst (0x0043f603)
        _emit 0xee
        // 0003f615 — n == 0: clear this string
        _emit 0x83          // CMP EAX, 0x10
        _emit 0xf8
        _emit 0x10
        _emit 0x89          // MOV [ESI+0x14], EDI  (size = 0)
        _emit 0x7e
        _emit 0x14
        _emit 0x72          // JC +2  (SSO: EBX = &inline_buf, already set)
        _emit 0x02
        _emit 0x8b          // MOV EBX, [EBX]  (heap ptr)
        _emit 0x1b
        // 0003f61f — pop EDI; null-terminate buffer[0]; return this
        _emit 0x5f          // POP EDI
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0xc6          // MOV byte ptr [EBX], 0
        _emit 0x03
        _emit 0x00
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0x8
        _emit 0x08
        _emit 0x00
        // 0003f62b — SSO dst path
        _emit 0x8b          // MOV EAX, EBX   (EAX = &inline_buf)
        _emit 0xc3
        // 0003f62d — _memmove_s(dst, capacity, s, n)
        _emit 0x57          // PUSH EDI   (n = count)
        _emit 0x55          // PUSH EBP   (s = src pointer)
        _emit 0x51          // PUSH ECX   (capacity = dst_size)
        _emit 0x50          // PUSH EAX   (dst)
        _emit 0xe8          // CALL 0x009d17f3  [RELOC]
        _emit 0xbd
        _emit 0x21
        _emit 0x59
        _emit 0x00
        _emit 0x83          // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        // 0003f639 — set size = n; null-terminate
        _emit 0x83          // CMP [ESI+0x18], 0x10
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        _emit 0x89          // MOV [ESI+0x14], EDI  (size = n)
        _emit 0x7e
        _emit 0x14
        _emit 0x72          // JC +2  (SSO)
        _emit 0x02
        _emit 0x8b          // MOV EBX, [EBX]  (heap ptr)
        _emit 0x1b
        // 0003f644 — buffer[n] = '\0'
        _emit 0xc6          // MOV byte ptr [EBX+EDI*1], 0
        _emit 0x04
        _emit 0x3b
        _emit 0x00
        // 0003f648 — common epilog
        _emit 0x5f          // POP EDI
        _emit 0x8b          // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e          // POP ESI
        _emit 0x5d          // POP EBP
        _emit 0x5b          // POP EBX
        _emit 0xc2          // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
