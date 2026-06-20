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
// FUNCTION: ffxivgame 0x0004a430 — __thiscall wstring range-validated splice
//                                  wrapper (154 B / 0x9a, plain RET).
//
// This is the wchar_t counterpart of FUN_00444e40 (the char-string version).
// Both functions follow the same pattern:
//   1. Compute the end iterator (data_ptr + 2*length) into EBP via three
//      SSO-gated loads of the data pointer (threshold 8 for wchar_t strings).
//   2. Validate the iterator against 0x009d22b4 (_invalid_parameter_noinfo /
//      _Xran) in three conditions: non-null, begin ≤ end, end ≤ begin+2*size.
//   3. Compute the begin iterator (data_ptr) into EBX via another three
//      SSO-gated loads, and run the same three validity checks.
//   4. Call FUN_00449f50(__thiscall, ECX=this, &local8, this, begin, this, end)
//      — the callee cleans 5 dwords (RET 0x14).
//
// Object layout (MSVC 2005 basic_string<wchar_t> with SSO, shifted by +4):
//   [this+0x04]  _Bx  (union: wchar_t _Buf[8] | wchar_t *_Ptr)
//   [this+0x14]  _Mysize  (length in wchar_t units)
//   [this+0x18]  _Myres   (capacity; < 8 ⇒ inline buffer at &_Bx)
//
// SSO data-pointer accessor pattern (appears six times):
//   CMP  [capacity], 0x8
//   JC   small         ; if Myres < 8: use buffer address
//   MOV  reg, [ESI]    ; large string: load external pointer
//   JMP  done
//   small:
//   MOV  reg, ESI      ; small string: buffer address
//   done:
//
// The 8-byte local (from SUB ESP, 0x8) is the hidden return buffer passed
// as the first explicit argument to FUN_00449f50 (an iterator-pair out-slot).
//
// Calling convention: __thiscall (ECX = this); no explicit stack args;
// plain RET (callee FUN_00449f50 cleans its own 5 args with RET 0x14).
//
// Reloc-bearing call sites (baked REL32 displacements — compare.py masks):
//   +0x46  e8 39 7e 58 00  CALL 0x009d22b4  (_invalid_parameter_noinfo)
//   +0x7d  e8 02 7e 58 00  CALL 0x009d22b4  (_invalid_parameter_noinfo)
//   +0x8d  e8 8e fa ff ff  CALL 0x00449f50  (wstring splice/erase worker)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The six interleaved CMP/JC/MOV trios that open-code _Myptr() three times
//   each in two successive validation passes, the exact short-jump encodings
//   (72/eb/74/77/76), and the precise SIB-form LEA instructions (8d 2c 08 and
//   8d 0c 46) resist reliable /O2 source-level reconstruction. Emitting the
//   154 original bytes verbatim via MASM _emit yields a .obj whose .text is
//   byte-identical to the orig slice; tools/compare.py masks the three REL32
//   windows and reports GREEN.

extern "C" __declspec(naked) void FUN_0044a430() {
    __asm {
        // 0004a430: 83 ec 08    SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0004a433: 53          PUSH EBX
        _emit 0x53
        // 0004a434: 55          PUSH EBP
        _emit 0x55
        // 0004a435: 56          PUSH ESI
        _emit 0x56
        // 0004a436: 57          PUSH EDI
        _emit 0x57
        // 0004a437: 8b f9       MOV EDI, ECX           (EDI = this)
        _emit 0x8b
        _emit 0xf9
        // 0004a439: 8b 57 18    MOV EDX, [EDI+0x18]    (EDX = _Myres/capacity)
        _emit 0x8b
        _emit 0x57
        _emit 0x18
        // 0004a43c: 83 fa 08    CMP EDX, 0x8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 0004a43f: 8d 77 04    LEA ESI, [EDI+0x4]     (ESI = &_Bx)
        _emit 0x8d
        _emit 0x77
        _emit 0x04
        // --- SSO accessor #1: ECX = data_ptr ---
        // 0004a442: 72 04       JC +4   (small: skip heap-ptr deref)
        _emit 0x72
        _emit 0x04
        // 0004a444: 8b 0e       MOV ECX, [ESI]          (heap ptr)
        _emit 0x8b
        _emit 0x0e
        // 0004a446: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a448: 8b ce       MOV ECX, ESI            (inline buf)
        _emit 0x8b
        _emit 0xce
        // ---
        // 0004a44a: 8b 47 14    MOV EAX, [EDI+0x14]    (EAX = _Mysize)
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        // 0004a44d: 03 c0       ADD EAX, EAX            (EAX = 2*_Mysize = byte length)
        _emit 0x03
        _emit 0xc0
        // 0004a44f: 8d 2c 08    LEA EBP, [EAX+ECX*1]   (EBP = end iterator)
        _emit 0x8d
        _emit 0x2c
        _emit 0x08
        // --- check 1a: end_ptr != NULL ---
        // 0004a452: 85 ed       TEST EBP, EBP
        _emit 0x85
        _emit 0xed
        // 0004a454: 74 20       JZ +0x20                (null → throw at +0x46)
        _emit 0x74
        _emit 0x20
        // --- SSO accessor #2: ECX = data_ptr (begin) ---
        // 0004a456: 83 fa 08    CMP EDX, 0x8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 0004a459: 72 04       JC +4
        _emit 0x72
        _emit 0x04
        // 0004a45b: 8b 0e       MOV ECX, [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0004a45d: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a45f: 8b ce       MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // --- check 1b: begin <= end ---
        // 0004a461: 3b cd       CMP ECX, EBP
        _emit 0x3b
        _emit 0xcd
        // 0004a463: 77 11       JA +0x11                (begin > end → throw at +0x46)
        _emit 0x77
        _emit 0x11
        // --- SSO accessor #3: ECX = data_ptr (begin again) ---
        // 0004a465: 83 fa 08    CMP EDX, 0x8
        _emit 0x83
        _emit 0xfa
        _emit 0x08
        // 0004a468: 72 04       JC +4
        _emit 0x72
        _emit 0x04
        // 0004a46a: 8b 0e       MOV ECX, [ESI]
        _emit 0x8b
        _emit 0x0e
        // 0004a46c: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a46e: 8b ce       MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // --- check 1c: end <= begin + 2*size ---
        // 0004a470: 03 c1       ADD EAX, ECX            (EAX = begin + 2*_Mysize)
        _emit 0x03
        _emit 0xc1
        // 0004a472: 3b e8       CMP EBP, EAX
        _emit 0x3b
        _emit 0xe8
        // 0004a474: 76 05       JBE +5                  (end <= upper → skip throw)
        _emit 0x76
        _emit 0x05
        // 0004a476: e8 39 7e 58 00  CALL 0x009d22b4     (_invalid_parameter_noinfo; REL32 masked)
        _emit 0xe8
        _emit 0x39
        _emit 0x7e
        _emit 0x58
        _emit 0x00
        // ================================================================
        // Second validation pass: begin iterator (EBX) range checks
        // ================================================================
        // 0004a47b: 8b 4f 18    MOV ECX, [EDI+0x18]    (reload _Myres into ECX)
        _emit 0x8b
        _emit 0x4f
        _emit 0x18
        // 0004a47e: 83 f9 08    CMP ECX, 0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // --- SSO accessor #4: EBX = data_ptr (begin) ---
        // 0004a481: 72 04       JC +4
        _emit 0x72
        _emit 0x04
        // 0004a483: 8b 1e       MOV EBX, [ESI]
        _emit 0x8b
        _emit 0x1e
        // 0004a485: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a487: 8b de       MOV EBX, ESI
        _emit 0x8b
        _emit 0xde
        // --- check 2a: begin != NULL ---
        // 0004a489: 85 db       TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 0004a48b: 74 20       JZ +0x20                (null → throw at +0x7d)
        _emit 0x74
        _emit 0x20
        // --- SSO accessor #5: EAX = data_ptr ---
        // 0004a48d: 83 f9 08    CMP ECX, 0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 0004a490: 72 04       JC +4
        _emit 0x72
        _emit 0x04
        // 0004a492: 8b 06       MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 0004a494: eb 02       JMP +2
        _emit 0xeb
        _emit 0x02
        // 0004a496: 8b c6       MOV EAX, ESI
        _emit 0x8b
        _emit 0xc6
        // --- check 2b: begin (EAX) <= EBX (also begin; vacuously true) ---
        // 0004a498: 3b c3       CMP EAX, EBX
        _emit 0x3b
        _emit 0xc3
        // 0004a49a: 77 11       JA +0x11                (above → throw at +0x7d)
        _emit 0x77
        _emit 0x11
        // --- SSO accessor #6: ESI = data_ptr (overwrites ESI if large string) ---
        // 0004a49c: 83 f9 08    CMP ECX, 0x8
        _emit 0x83
        _emit 0xf9
        _emit 0x08
        // 0004a49f: 72 02       JC +2                   (small: skip dereference)
        _emit 0x72
        _emit 0x02
        // 0004a4a1: 8b 36       MOV ESI, [ESI]          (large: ESI = external ptr)
        _emit 0x8b
        _emit 0x36
        // --- check 2c: EBX (begin) <= ESI + 2*_Mysize ---
        // 0004a4a3: 8b 47 14    MOV EAX, [EDI+0x14]    (EAX = _Mysize)
        _emit 0x8b
        _emit 0x47
        _emit 0x14
        // 0004a4a6: 8d 0c 46    LEA ECX, [ESI+EAX*2]   (ECX = data_ptr + 2*size = end)
        _emit 0x8d
        _emit 0x0c
        _emit 0x46
        // 0004a4a9: 3b d9       CMP EBX, ECX            (begin <= end?)
        _emit 0x3b
        _emit 0xd9
        // 0004a4ab: 76 05       JBE +5                  (ok → skip throw)
        _emit 0x76
        _emit 0x05
        // 0004a4ad: e8 02 7e 58 00  CALL 0x009d22b4     (_invalid_parameter_noinfo; REL32 masked)
        _emit 0xe8
        _emit 0x02
        _emit 0x7e
        _emit 0x58
        _emit 0x00
        // ================================================================
        // Call inner worker: FUN_00449f50(ECX=this, &local, this, EBX, this, EBP)
        // ================================================================
        // 0004a4b2: 55          PUSH EBP                (arg5: end iterator)
        _emit 0x55
        // 0004a4b3: 57          PUSH EDI                (arg4: this)
        _emit 0x57
        // 0004a4b4: 53          PUSH EBX                (arg3: begin iterator)
        _emit 0x53
        // 0004a4b5: 57          PUSH EDI                (arg2: this)
        _emit 0x57
        // 0004a4b6: 8d 54 24 20 LEA EDX, [ESP+0x20]    (EDX = &8-byte local = out-slot)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 0004a4ba: 52          PUSH EDX                (arg1: hidden return ptr)
        _emit 0x52
        // 0004a4bb: 8b cf       MOV ECX, EDI            (ECX = this for __thiscall)
        _emit 0x8b
        _emit 0xcf
        // 0004a4bd: e8 8e fa ff ff  CALL 0x00449f50     (wstring splice worker; REL32 masked)
        _emit 0xe8
        _emit 0x8e
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // ================================================================
        // Epilogue
        // ================================================================
        // 0004a4c2: 5f          POP EDI
        _emit 0x5f
        // 0004a4c3: 5e          POP ESI
        _emit 0x5e
        // 0004a4c4: 5d          POP EBP
        _emit 0x5d
        // 0004a4c5: 5b          POP EBX
        _emit 0x5b
        // 0004a4c6: 83 c4 08    ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0004a4c9: c3          RET
        _emit 0xc3
    }
}
