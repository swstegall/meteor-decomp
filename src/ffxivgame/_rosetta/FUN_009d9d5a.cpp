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
// FUNCTION: ffxivgame 0x005d9d5a — `___doserrno` (19 B / 0x13)
//           Standard MSVC CRT internal: returns a pointer to the
//           per-thread `_doserrno` if a per-thread data (tiddata) block
//           exists, or a pointer to the global `_doserrno` otherwise.
//
// Calling convention: __cdecl, no args, returns int*.
//
// Asm (19 bytes @ RVA 0x005d9d5a):
//   e8 [4-byte reloc]    CALL FUN_009df360       ; get per-thread data ptr
//   85 c0                TEST EAX, EAX
//   75 06                JNZ  +6                 ; non-null → jump to add
//   b8 [4-byte reloc]    MOV  EAX, offset _g     ; null → return global
//   c3                   RET
//   83 c0 0c             ADD  EAX, 0xc           ; non-null: ptd->_tdoserrno
//   c3                   RET
//
// FUN_009df360 is the MSVC CRT per-thread-data getter (_getptd_noexit).
// It returns NULL if the TLS block has not been allocated yet (e.g.,
// during very early CRT init), in which case we fall back to the
// process-wide `_doserrno` global at .data.  When the block exists,
// `_tdoserrno` lives at offset 0x0c inside the tiddata struct.

extern "C" void* __cdecl FUN_009df360(void);
extern "C" int _doserrno;

extern "C" int* __cdecl ___doserrno(void)
{
    void* ptd = FUN_009df360();
    if (ptd == 0) {
        return &_doserrno;
    }
    return (int*)((char*)ptd + 12);
}
