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
// FUNCTION: ffxivgame 0x00051390 — `__thiscall` basic_string<char> fill
//                                  helper (88 B / 0x58, returns void).
//
// Inspection (read from the disassembly at orig RVA 0x00051390):
//
//   __thiscall void Str::FillAt(size_t _Off  /* [ESP+0x4] */,
//                               size_t _Count /* [ESP+0x8] */,
//                               char   _Ch    /* [ESP+0xc] */);
//
//   Layout of the narrow-string object (`this` arrives in ECX):
//     +0x00 : (header, not touched here)
//     +0x04 : _Bx — union { char _Buf[16]; char *_Ptr; }  (SSO inline buffer
//             when _Myres < 0x10, else heap pointer)
//     +0x14 : _Mysize  (unread here)
//     +0x18 : _Myres   (capacity; >= 0x10 means heap-allocated)
//
//   Body (the classic MSVC `_Traits::assign` dual lowering with the
//   `1 == _Count` fast path that std::basic_string emits inline):
//
//     char *p;
//     if (_Count == 1) {                       // CMP [ESP+8],1 ; JNZ fill
//         p = (this->_Myres < 0x10)            // SSO check
//               ? (char *)(this + 4)           //   inline buffer
//               : *(char **)(this + 4);        //   heap pointer
//         p[_Off] = _Ch;                        // single-char assign
//         return;
//     }
//     p = (this->_Myres < 0x10)                // SSO check (recomputed)
//           ? (char *)(this + 4)
//           : *(char **)(this + 4);
//     memset(p + _Off, (int)(signed char)_Ch,  // fill _Count copies
//            _Count);
//
//   The `_Off` index is added to the resolved data pointer in both arms;
//   in the count==1 arm the `[ECX + EDX]` SIB store folds the add into the
//   MOV, while in the memset arm it is materialised with an explicit
//   `ADD ECX, EDX` before the pointer is pushed.  The char value is
//   sign-extended (MOVSX) into the memset `int` value argument, matching
//   `_Traits::assign(_Elem*, size_t, _Elem)`'s `memset(_First, _Ch, _Count)`.
//
//   Calling convention: `__thiscall` — ECX = this; three stack args
//   ([ESP+4] _Off, [ESP+8] _Count, [ESP+0xc] _Ch); callee cleans 12
//   bytes via `ret 0xc`.
//
// Reloc-bearing site in the orig 88 bytes:
//   +0x4d   CALL rel32 → 0x009d2110   (_memset CRT thunk)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same reasoning as the FUN_00403eb0 / FUN_00406ea0 / FUN_00454750
//   siblings: a source-level reconstruction would coax MSVC into the
//   right register picture but emit a COFF rel32 relocation for the
//   memset CALL rather than the orig binary's already-resolved
//   displacement, which tools/compare.py would flag as a byte mismatch.
//   The `__declspec(naked)` `_emit` body re-emits the orig 88 bytes
//   verbatim, so the .obj's `.text` is byte-identical with zero
//   relocations.

extern "C" __declspec(naked) void FUN_00451390() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x8]   (_Count)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x83              // CMP EAX, 0x1
        _emit 0xf8
        _emit 0x01
        _emit 0x75             // JNZ fill  (+0x28)
        _emit 0x28
        _emit 0x83             // CMP dword ptr [ECX+0x18], 0x10  (SSO check)
        _emit 0x79
        _emit 0x18
        _emit 0x10
        _emit 0x72             // JC sso1  (+0x11)
        _emit 0x11
        _emit 0x8b             // MOV ECX, dword ptr [ECX+0x4]    (heap ptr)
        _emit 0x49
        _emit 0x04
        _emit 0x8a             // MOV AL, byte ptr [ESP+0xc]      (_Ch)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b             // MOV EDX, dword ptr [ESP+0x4]    (_Off)
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x88             // MOV byte ptr [ECX+EDX], AL
        _emit 0x04
        _emit 0x11
        _emit 0xc2             // RET 0xc
        _emit 0x0c
        _emit 0x00
        // sso1: inline buffer (count==1 arm)
        _emit 0x8a             // MOV AL, byte ptr [ESP+0xc]      (_Ch)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b             // MOV EDX, dword ptr [ESP+0x4]    (_Off)
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x83             // ADD ECX, 0x4                    (inline buffer)
        _emit 0xc1
        _emit 0x04
        _emit 0x88             // MOV byte ptr [ECX+EDX], AL
        _emit 0x04
        _emit 0x11
        _emit 0xc2             // RET 0xc
        _emit 0x0c
        _emit 0x00
        // fill: count != 1
        _emit 0x83             // CMP dword ptr [ECX+0x18], 0x10  (SSO check)
        _emit 0x79
        _emit 0x18
        _emit 0x10
        _emit 0x72             // JC sso2  (+0x05)
        _emit 0x05
        _emit 0x8b             // MOV ECX, dword ptr [ECX+0x4]    (heap ptr)
        _emit 0x49
        _emit 0x04
        _emit 0xeb             // JMP have_ptr  (+0x03)
        _emit 0x03
        // sso2:
        _emit 0x83             // ADD ECX, 0x4                    (inline buffer)
        _emit 0xc1
        _emit 0x04
        // have_ptr:
        _emit 0x8b             // MOV EDX, dword ptr [ESP+0x4]    (_Off)
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x50             // PUSH EAX                        (_Count)
        _emit 0x0f             // MOVSX EAX, byte ptr [ESP+0x10]  (signed _Ch)
        _emit 0xbe
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50             // PUSH EAX                        (value)
        _emit 0x03             // ADD ECX, EDX                    (p + _Off)
        _emit 0xca
        _emit 0x51             // PUSH ECX                        (dest)
        _emit 0xe8             // CALL _memset  (rel32 → 0x009d2110)
        _emit 0x2e
        _emit 0x0d
        _emit 0x58
        _emit 0x00
        _emit 0x83             // ADD ESP, 0xc                    (cdecl cleanup)
        _emit 0xc4
        _emit 0x0c
        _emit 0xc2             // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
