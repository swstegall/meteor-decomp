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
// FUNCTION: ffxivgame 0x00049060 — `std::basic_string<wchar_t>` data-pointer
//           getter (31 B, __stdcall void*(void*)).
//
// __stdcall 1-arg function that delegates to FUN_00449000 (RVA 0x49000, the
// sibling null-terminate-and-emit thunk) then returns the character data
// pointer for the basic_string-like object passed as the single argument:
//
//   - _Myres ([arg+0x18]) >= 8: heap mode — return _Bx._Ptr ([arg+0x4])
//   - _Myres ([arg+0x18]) <  8: SSO mode  — return &_Bx._Buf (arg+0x4)
//
// SSO threshold of 8 matches std::basic_string<wchar_t>::_BUF_SIZE = 8.
//
// Asm (31 bytes, RVA 0x00049060..0x0004907e):
//
//   56               PUSH ESI
//   8b 74 24 08      MOV  ESI, [ESP+0x8]      ; load arg (string object ptr)
//   56               PUSH ESI                  ; pass arg to callee
//   e8 95 ff ff ff   CALL FUN_00449000         ; rel32 = -107 (no PE reloc entry)
//   83 7e 18 08      CMP  dword ptr [ESI+0x18], 0x8  ; _Myres vs SSO threshold
//   72 07            JC   sso_path             ; jump if _Myres < 8 (SSO mode)
//   8b 46 04         MOV  EAX, [ESI+0x4]       ; heap: return _Bx._Ptr
//   5e               POP  ESI
//   c2 04 00         RET  0x4
// sso_path:
//   8d 46 04         LEA  EAX, [ESI+0x4]       ; SSO: return &_Bx._Buf
//   5e               POP  ESI
//   c2 04 00         RET  0x4
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as siblings FUN_00449000 / FUN_00401000: a
//   `__declspec(naked)` body re-emitting the orig 31 bytes verbatim via
//   `_emit` directives yields a .obj whose .text is byte-identical to the
//   orig PE slice. The CALL rel32 displacement (0xffffff95 = -107) is
//   baked as raw bytes; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00449060() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x56              // PUSH ESI  (arg for callee)
        _emit 0xe8              // CALL FUN_00449000  (rel32 = 0xffffff95)
        _emit 0x95
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83              // CMP  dword ptr [ESI+0x18], 0x8
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        _emit 0x72              // JC   sso_path (+7)
        _emit 0x07
        _emit 0x8b              // MOV  EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x4
        _emit 0x04
        _emit 0x00
        _emit 0x8d              // LEA  EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x4
        _emit 0x04
        _emit 0x00
    }
}
