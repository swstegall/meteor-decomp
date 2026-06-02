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
// FUNCTION: ffxivgame 0x005d9d47 — `__errno` (19 B / 0x13)
//           Standard MSVC CRT internal: returns a pointer to the
//           per-thread `errno` if a per-thread data (tiddata) block
//           exists, or a pointer to the global `errno` otherwise.
//
// Calling convention: __cdecl, no args, returns int*.
//
// Asm (19 bytes @ RVA 0x005d9d47):
//   e8 [4-byte reloc]    CALL FUN_009df360          ; get per-thread data ptr
//   85 c0                TEST EAX, EAX
//   75 06                JNZ  +6                    ; non-null → jump to add
//   b8 [4-byte reloc]    MOV  EAX, offset _nterrno  ; null → return global
//   c3                   RET
//   83 c0 08             ADD  EAX, 0x8              ; non-null: ptd->_terrno @ offset 8
//   c3                   RET
//
// FUN_009df360 is the MSVC CRT per-thread-data getter (_getptd_noexit).
// It returns NULL if the TLS block has not been allocated yet (e.g.,
// during very early CRT init), in which case we fall back to the
// process-wide `_nterrno` global at .data.  When the block exists,
// `_terrno` (errno) lives at offset 0x08 inside the tiddata struct.
// This is the sibling of ___doserrno (FUN_009d9d5a) which uses offset 0x0c.

extern "C" void* __cdecl FUN_009df360(void);
extern "C" int _nterrno;

extern "C" int* __cdecl FUN_009d9d47(void)
{
    void* ptd = FUN_009df360();
    if (ptd == 0) {
        return &_nterrno;
    }
    return (int*)((char*)ptd + 8);
}
