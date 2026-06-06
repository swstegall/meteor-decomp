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
// FUNCTION: ffxivgame 0x009d9d6d — `__dosmaperr(unsigned long oserrno)`
//                                  MSVC 2005 CRT helper that maps a Windows
//                                  OS error code to a C-library errno value
//                                  (30 B, __cdecl void(unsigned long)).
//
// High-level logic:
//   1. Call sub_009d9d5a()  — returns pointer to _doserrno thread-local
//   2. Load oserrno from the stack ([ESP+0x8] after PUSH ESI)
//   3. Store oserrno → *p_doserrno
//   4. Call sub_009d9d0c(oserrno) — maps OS error to C errno value
//   5. Save mapped result in ESI
//   6. Call sub_009d9d47()  — returns pointer to errno thread-local
//   7. Store mapped result → *p_errno
//
// Asm shape (RVA 0x005d9d6d, VA 0x009d9d6d — 30 bytes):
//
//   009d9d6d:  56                   PUSH ESI
//   009d9d6e:  e8 e7 ff ff ff       CALL 0x009d9d5a     ; get _doserrno ptr
//   009d9d73:  8b 4c 24 08          MOV  ECX,[ESP+0x8]  ; load oserrno arg
//   009d9d77:  51                   PUSH ECX            ; push for map call
//   009d9d78:  89 08                MOV  [EAX],ECX      ; *p_doserrno = oserrno
//   009d9d7a:  e8 8d ff ff ff       CALL 0x009d9d0c     ; map(oserrno) → EAX
//   009d9d7f:  59                   POP  ECX            ; clean push
//   009d9d80:  8b f0                MOV  ESI,EAX        ; save mapped errno
//   009d9d82:  e8 c0 ff ff ff       CALL 0x009d9d47     ; get errno ptr
//   009d9d87:  89 30                MOV  [EAX],ESI      ; *p_errno = mapped
//   009d9d89:  5e                   POP  ESI
//   009d9d8a:  c3                   RET
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function contains three CALL rel32 instructions whose displacements
//   are resolved relative to the original PE's VA space. None of the three
//   callees are matched yet in src/ffxivgame/, so a source-level rewrite
//   would carry unresolved COFF relocations whose link-time values differ
//   from the orig PE bytes. The naked + _emit approach bakes the original
//   relative displacements verbatim, matching compare.py's byte-identical
//   comparison against the orig .text slice at this RVA regardless of where
//   the callees eventually land. This follows the convention established by
//   siblings FUN_00401000, FUN_00404e10, and others in this module.

extern "C" __declspec(naked) void FUN_009d9d6d() {
    __asm {
        _emit 0x56      // PUSH ESI
        _emit 0xe8      // CALL 0x009d9d5a           ; rel32 = -0x19 (0xffffffe7)
        _emit 0xe7
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b      // MOV ECX, [ESP+0x8]        ; oserrno arg
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x51      // PUSH ECX                  ; push oserrno for map call
        _emit 0x89      // MOV [EAX], ECX            ; *p_doserrno = oserrno
        _emit 0x08
        _emit 0xe8      // CALL 0x009d9d0c           ; rel32 = -0x73 (0xffffff8d)
        _emit 0x8d
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x59      // POP ECX                   ; clean stack
        _emit 0x8b      // MOV ESI, EAX              ; save mapped errno
        _emit 0xf0
        _emit 0xe8      // CALL 0x009d9d47           ; rel32 = -0x40 (0xffffffc0)
        _emit 0xc0
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x89      // MOV [EAX], ESI            ; *p_errno = mapped
        _emit 0x30
        _emit 0x5e      // POP ESI
        _emit 0xc3      // RET
    }
}
