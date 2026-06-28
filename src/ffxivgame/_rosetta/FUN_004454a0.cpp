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
// FUNCTION: ffxivgame 0x000454a0 — __thiscall in-place char-replace on a
//                                   char* field at offset 0 (36 B / 0x24).
//
// void FUN_004454a0::replace(char old_char, char new_char)
//
// ECX = this  (copied to EAX immediately to free ECX for CL = new_char)
// EDX = this->m_str   (char* at offset 0)
//
// Iterates the NUL-terminated string stored at this->m_str, replacing
// every occurrence of old_char (BL / first stack arg) with new_char
// (CL / second stack arg). The string is walked via EDX; the function
// is a no-op if the string is already empty.
//
// Register allocation (MSVC 2005 /O2 /Oy):
//   EAX  = this        (freed immediately after EDX = [EAX] is loaded)
//   EDX  = p           (walking char* pointer)
//   CL   = new_char    (byte-reg from ECX, freed once this was copied to EAX)
//   BL   = old_char    (byte-reg from EBX, callee-saved — PUSH EBX before use)
//
// The unusual two-instruction preamble (MOV EAX,ECX / MOV EDX,[EAX])
// instead of the single-instruction MOV EDX,[ECX] is caused by the
// register allocator deciding to use CL for new_char: once it commits
// to CL it must evict `this` from ECX into EAX first, then indirect
// through EAX. EBX is saved after CL is already loaded because EBX
// is the callee-save that carries old_char (BL).
//
// Asm (36 bytes @ orig RVA 0x000454a0):
//   8b c1              MOV EAX, ECX
//   8b 10              MOV EDX, [EAX]
//   80 3a 00           CMP byte [EDX], 0x0
//   74 18              JZ  exit
//   8a 4c 24 08        MOV CL,  [ESP+0x8]      ; new_char (before PUSH EBX)
//   53                 PUSH EBX
//   8a 5c 24 08        MOV BL,  [ESP+0x8]      ; old_char (after  PUSH EBX)
//   38 1a              CMP [EDX], BL            ; *p == old_char?
//   75 02              JNZ +2  → advance
//   88 0a              MOV [EDX], CL            ; *p = new_char
//   83 c2 01           ADD EDX, 0x1             ; p++
//   80 3a 00           CMP byte [EDX], 0x0      ; *p == '\0'?
//   75 f2              JNZ loop                 ; loop if not null
//   5b                 POP EBX
//   c2 08 00           RET 0x8                  ; __thiscall, 2 stack args

struct FUN_004454a0_cls {
    char *m_str;
    void replace(char old_char, char new_char);
};

void FUN_004454a0_cls::replace(char old_char, char new_char) {
    char *p = m_str;
    if (*p != '\0') {
        do {
            if (*p == old_char)
                *p = new_char;
            p++;
        } while (*p != '\0');
    }
}
