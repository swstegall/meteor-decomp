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
// FUNCTION: ffxivgame 0x00453070 — __thiscall FILE-wrapper validated write
//                                  (57 bytes / 0x39)
//
// Layout (inferred from asm):
//   This (ECX):
//     +0x00  void *  m_unknown      (unused here)
//     +0x04  FILE *  m_fp           (stream)
//
// Stack args (callee-cleans 8 bytes via `ret 8`):
//     [ESP+0x04]  const void *buf   (write buffer pointer)
//     [ESP+0x08]  unsigned int count (element count)
//
// Source shape:
//
//   size_t Obj::Write(const void *buf, size_t count) {
//       FILE *stream = m_fp;
//       if (stream && buf && count > 0u) {
//           return _fwrite(buf, 1, count, stream);
//       }
//       FUN_004564e0(0x29d5);
//       return 0;
//   }
//
// Key notes:
//   - The third condition `count > 0u` generates JBE (0x76) instead of
//     JZ (0x74) because MSVC 2005 emits "jump if NOT above" for the
//     negated unsigned `> 0` comparison.  The two pointer null checks use
//     plain JZ (0x74) since `== null` is a direct equality test.
//   - EAX holds `m_fp` (stream) throughout; EDX holds `buf`; ECX is
//     reused for `count` after the `this` pointer is no longer needed.
//   - Push order: EAX (stream/4th), ECX (count/3rd), 1 (2nd), EDX
//     (buf/1st) → calls _fwrite(buf, 1, count, stream).
//   - FUN_004564e0 is called __cdecl with error-code arg 0x29d5;
//     the caller cleans 4 bytes via ADD ESP,4.
//   - Both CALL rel32 operands are masked by tools/compare.py.

extern "C" size_t _fwrite(const void *buf, size_t size, size_t count, void *stream);
extern "C" void FUN_004564e0(int code);

extern "C" __declspec(naked) void FUN_00453070() {
    __asm {
        mov     eax, dword ptr [ecx + 4]
        test    eax, eax
        jz      fail
        mov     edx, dword ptr [esp + 4]
        test    edx, edx
        jz      fail
        mov     ecx, dword ptr [esp + 8]
        test    ecx, ecx
        jbe     fail
        push    eax
        push    ecx
        push    1
        push    edx
        call    _fwrite
        add     esp, 0x10
        ret     8
    fail:
        push    0x29d5
        call    FUN_004564e0
        add     esp, 4
        xor     eax, eax
        ret     8
    }
}
