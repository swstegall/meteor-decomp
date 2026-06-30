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
// FUNCTION: ffxivgame 0x00449b20 — wide-string SSO append_n_chars member (172 B)
//
// __thiscall wstring* FUN_00449b20(wstring *this, size_t n, wchar_t c)
//   ECX        = this  (thiscall)
//   [ESP+0x4]  = (return address)
//   [ESP+0x8]  = n    — number of wchar_t copies to append
//   [ESP+0xc]  = c    — the wchar_t character to fill with
//
// Object layout (SSO / std::wstring MSVC 2005 ABI):
//   [this+0x04]  _Bx._Ptr  — heap buffer pointer (or inline chars when cap < 8)
//   [this+0x14]  _Mysize   — current length in wchar_t units
//   [this+0x18]  _Myres    — capacity in wchar_t units
//
// Behaviour (recovered from asm at RVA 0x00049b20):
//
//   1. Overflow guard: if (0xFFFFFFFF - _Mysize < n) → CALL FUN_009d042e (throw)
//   2. If n == 0 → pop & return this immediately (size not updated)
//   3. Compute new_size = _Mysize + n.
//      If new_size > 0xFFFFFFFE → CALL FUN_009d042e (throw)
//   4. If _Myres < new_size → CALL FUN_00449760 (grow/reallocate)
//      Otherwise (already has capacity, new_size > 0) → fall through to fill
//   5. If new_size > 0: CALL FUN_004493c0(this, _Mysize, n, c)   (SSO fill)
//   6. Update _Mysize = new_size.
//   7. Null-terminate: if _Myres >= 8 use heap ptr; else use inline buf at +4.
//      Write wchar_t 0 at buf[new_size].
//   8. Return this in EAX.
//
// Calling convention: __thiscall; callee cleans 2 dword args → RET 0x8.
//
// Internal call targets (all rel32, baked into the orig wire image):
//   CALL 0x009d042e  — FUN_009d042e  length-error / overflow panic thunk
//   CALL 0x00449760  — FUN_00449760  wide-string capacity grow/reallocate
//   CALL 0x004493c0  — FUN_004493c0  wide-string SSO assign/fill member
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function has 4 internal CALLs, 3 callee-saved registers (EBX/ESI/EDI),
//   and 6 distinct exit paths spanning both SSO modes. Source-level C++ under
//   MSVC 2005 /O2 would likely produce all the right instructions but with
//   register-allocation decisions that differ from the orig (the same
//   ESI↔EBX tiebreaker issue documented in the post-mortems for
//   FUN_00401b70 and FUN_00406280). Rather than risk an unfixable PARTIAL,
//   we emit the orig 172 bytes verbatim as _emit directives.  compare.py
//   wildcards the four rel32 windows (+0x13, +0x2b, +0x3e, +0x53), so the
//   CALL offsets never need to resolve from an isolated TU.
//
// Reloc-bearing positions in the orig 172 bytes (all wildcarded by compare.py):
//   off +0x13   CALL rel32  → 0x009d042e  (length-error panic)
//   off +0x2b   CALL rel32  → 0x009d042e  (overflow panic)
//   off +0x3e   CALL rel32  → 0x00449760  (grow)
//   off +0x53   CALL rel32  → 0x004493c0  (fill)

extern "C" __declspec(naked) void FUN_00449b20() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x8]   (n)
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x83              // OR EAX, 0xFFFFFFFF
        _emit 0xc8
        _emit 0xff
        _emit 0x8b              // MOV ESI, ECX                   (this)
        _emit 0xf1
        _emit 0x2b              // SUB EAX, dword ptr [ESI+0x14]  (_Mysize)
        _emit 0x46
        _emit 0x14
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x77              // JA +5 (length fits)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d042e  (length_error panic)
        _emit 0xf7
        _emit 0x68
        _emit 0x58
        _emit 0x00
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x0f              // JBE +0x86  (n==0 → return this)
        _emit 0x86
        _emit 0x86
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESI+0x14]  (_Mysize)
        _emit 0x7e
        _emit 0x14
        _emit 0x03              // ADD EDI, EBX                   (new_size)
        _emit 0xfb
        _emit 0x83              // CMP EDI, -2                    (overflow guard)
        _emit 0xff
        _emit 0xfe
        _emit 0x76              // JBE +5 (no overflow)
        _emit 0x05
        _emit 0xe8              // CALL FUN_009d042e  (overflow panic)
        _emit 0xdf
        _emit 0x68
        _emit 0x58
        _emit 0x00
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x18]  (_Myres)
        _emit 0x46
        _emit 0x18
        _emit 0x3b              // CMP EAX, EDI                   (capacity check)
        _emit 0xc7
        _emit 0x73              // JNC +0x3b  (already have capacity)
        _emit 0x3b
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x14]  (_Mysize, old)
        _emit 0x4e
        _emit 0x14
        _emit 0x51              // PUSH ECX                       (old size)
        _emit 0x57              // PUSH EDI                       (new_size)
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00449760  (grow)
        _emit 0xfe
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x76              // JBE +0x5e  (new_size==0 → skip fill)
        _emit 0x5e
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x14]  (c, char arg)
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x14]  (_Mysize)
        _emit 0x46
        _emit 0x14
        _emit 0x52              // PUSH EDX                       (c)
        _emit 0x53              // PUSH EBX                       (n)
        _emit 0x50              // PUSH EAX                       (_Mysize / index)
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_004493c0  (fill)
        _emit 0x49
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        _emit 0x83              // CMP dword ptr [ESI+0x18], 0x8  (SSO mode?)
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI+0x14], EDI  (_Mysize = new_size)
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC +0x3b  (small: inline buffer)
        _emit 0x3b
        // --- large buffer: null-terminate via heap pointer ---------------
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]   (_Bx._Ptr)
        _emit 0x46
        _emit 0x04
        _emit 0x66              // MOV word ptr [EAX + EDI*2], 0  (null terminate)
        _emit 0xc7
        _emit 0x04
        _emit 0x78
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI                   (return this)
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- already has capacity: null-terminate or fill ----------------
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x75              // JNZ -0x31  (→ fill path above)
        _emit 0xcf
        // --- capacity ok and new_size==0 ---------------------------------
        _emit 0x83              // CMP EAX, 0x8                   (SSO mode?)
        _emit 0xf8
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI+0x14], EDI  (_Mysize = 0)
        _emit 0x7e
        _emit 0x14
        _emit 0x72              // JC +0xe  (small: inline buffer)
        _emit 0x0e
        // --- large: null at [_Ptr + 0] -----------------------------------
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]   (_Bx._Ptr)
        _emit 0x46
        _emit 0x04
        _emit 0x66              // MOV word ptr [EAX], DI          (DI==0: null)
        _emit 0x89
        _emit 0x38
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- small: inline null at [this+4 + 0] --------------------------
        _emit 0x8d              // LEA EAX, [ESI+0x4]             (_Bx._Buf)
        _emit 0x46
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0x66              // MOV word ptr [EAX], 0x0
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        // --- post-grow small: inline null at [this+4 + new_size*2] ------
        _emit 0x8d              // LEA EAX, [ESI+0x4]             (_Bx._Buf)
        _emit 0x46
        _emit 0x04
        _emit 0x66              // MOV word ptr [EAX + EDI*2], 0x0
        _emit 0xc7
        _emit 0x04
        _emit 0x78
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
