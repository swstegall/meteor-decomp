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
// FUNCTION: ffxivgame 0x0046ee10 — OpenSSL X509_NAME_add_entry_by_txt
//                                  (__cdecl, 77 bytes / 0x4d)
//
// int X509_NAME_add_entry_by_txt(
//     X509_NAME *name,             // param1  [ESP+0x04]
//     const char *field,           // param2  [ESP+0x08]
//     int type,                    // param3  [ESP+0x0c]
//     const unsigned char *bytes,  // param4  [ESP+0x10]
//     int len,                     // param5  [ESP+0x14]
//     int loc,                     // param6  [ESP+0x18]
//     int set                      // param7  [ESP+0x1c]
// )
//
// Converts the text field name to an X509_NAME_ENTRY via
// X509_NAME_ENTRY_create_by_txt(NULL, field, type, bytes, len),
// returns 0 immediately if that fails, otherwise calls
// X509_NAME_add_entry(name, ne, loc, set), frees ne via
// X509_NAME_ENTRY_free(ne), and returns the add_entry result.
//
// MSVC 2005 /O2 codegen: pre-loads params 3/4/5 into EDX/ECX/EAX before
// PUSH ESI so that after the push the compiler can use [ESP+0x10] (a
// shorter encoding) to reload param2 (field) rather than [ESP+0x14]
// (one byte longer with ESP having grown by one push).  The same
// pre-load pattern appears in the immediately-preceding sibling
// X509_NAME_ENTRY_create_by_txt (FUN_0046eda0).
//
// The epilogue mirrors X509_NAME_ENTRY_create_by_txt: PUSH the "object
// to free" before saving the result of the preceding call into EDI,
// call the free function, clean the combined arg frame in one ADD ESP,
// then MOV EAX,EDI / POP EDI / POP ESI / RET.
//
// Reloc-bearing sites (REL32 call displacements, wildcarded by compare.py):
//   +0x17  CALL FUN_0046eda0  (X509_NAME_ENTRY_create_by_txt)  rel32 = 0xffffff74
//   +0x38  CALL FUN_0046eac0  (X509_NAME_add_entry)            rel32 = 0xfffffc73
//   +0x40  CALL FUN_0045dd70  (X509_NAME_ENTRY_free)           rel32 = 0xffffeef1b
//
// Original bytes (RVA 0x0006ee10 – 0x0006ee5c, 77 bytes):
//
//   8b 44 24 14  8b 4c 24 10  8b 54 24 0c  56 50
//   8b 44 24 10  51 52 50 6a 00 e8 74 ff ff ff
//   8b f0 83 c4 14 85 f6 75 02 5e c3
//   8b 4c 24 20  8b 54 24 1c  8b 44 24 08
//   57 51 52 56 50 e8 73 fc ff ff
//   56 8b f8 e8 1b ef fe ff
//   83 c4 14 8b c7 5f 5e c3

extern "C" __declspec(naked) void FUN_0046ee10() {
    __asm {
        // --- prologue: pre-load params 5/4/3 before PUSH ESI -----------
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]  (len)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]  (bytes)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x0c]  (type)
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x56              // PUSH ESI  (save callee-saved)
        // --- push args for X509_NAME_ENTRY_create_by_txt (right-to-left) --
        _emit 0x50              // PUSH EAX  (len  — 5th arg)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]  (field; ESP shifted by PUSH ESI+PUSH EAX)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX  (bytes — 4th arg)
        _emit 0x52              // PUSH EDX  (type  — 3rd arg)
        _emit 0x50              // PUSH EAX  (field — 2nd arg)
        _emit 0x6a              // PUSH 0x0  (NULL  — 1st arg, X509_NAME_ENTRY **ne)
        _emit 0x00
        _emit 0xe8              // CALL FUN_0046eda0 (X509_NAME_ENTRY_create_by_txt)
        _emit 0x74
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // --- save result, clean frame, test for NULL ---
        _emit 0x8b              // MOV ESI, EAX  (ne = result)
        _emit 0xf0
        _emit 0x83              // ADD ESP, 0x14  (clean 5 args)
        _emit 0xc4
        _emit 0x14
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +0x02  (ne != NULL → success path)
        _emit 0x02
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET  (return 0 — EAX = 0 from NULL ne)
        // --- success path: pre-load params 7/6/1 before PUSH EDI -------
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x20]  (set  — param7)
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x1c]  (loc  — param6)
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x08]  (name — param1)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x57              // PUSH EDI  (save callee-saved)
        // --- push args for X509_NAME_add_entry (right-to-left) ----------
        _emit 0x51              // PUSH ECX  (set  — 4th arg)
        _emit 0x52              // PUSH EDX  (loc  — 3rd arg)
        _emit 0x56              // PUSH ESI  (ne   — 2nd arg)
        _emit 0x50              // PUSH EAX  (name — 1st arg)
        _emit 0xe8              // CALL FUN_0046eac0 (X509_NAME_add_entry)
        _emit 0x73
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // --- push ne for free, save ret, call X509_NAME_ENTRY_free ------
        _emit 0x56              // PUSH ESI  (ne — arg for X509_NAME_ENTRY_free)
        _emit 0x8b              // MOV EDI, EAX  (save add_entry result)
        _emit 0xf8
        _emit 0xe8              // CALL FUN_0045dd70 (X509_NAME_ENTRY_free)
        _emit 0x1b
        _emit 0xef
        _emit 0xfe
        _emit 0xff
        // --- epilogue: clean combined frame, return result ---------------
        _emit 0x83              // ADD ESP, 0x14  (clean 4 add_entry args + 1 free arg)
        _emit 0xc4
        _emit 0x14
        _emit 0x8b              // MOV EAX, EDI  (return value)
        _emit 0xc7
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
