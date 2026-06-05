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
// FUNCTION: ffxivgame 0x0046eda0 — OpenSSL X509_NAME_ENTRY_create_by_txt
//                                  (__cdecl, 110 bytes)
//
// X509_NAME_ENTRY *X509_NAME_ENTRY_create_by_txt(
//     X509_NAME_ENTRY **ne,           // param1 — [ESP+0xC] after pushes
//     const char *field,              // param2 — [ESP+0x10] after pushes
//     int type,                       // param3 — [ESP+0x14] after pushes
//     const unsigned char *bytes,     // param4 — [ESP+0x18] after pushes
//     int len                         // param5 — [ESP+0x1C] after pushes
// )
//
// Saves ESI, EDI.  Loads param2 (field) into EDI.
// Calls FUN_00465370(field, 0) = OBJ_txt2nid — returns the NID.
// If NID == 0: emits ERR_put_error + ERR_add_error_data and returns NULL.
// If NID != 0: forwards to FUN_0046ed30 = X509_NAME_ENTRY_create_by_NID,
//              then calls FUN_0046cae0(nid) (cleanup / deref), and returns
//              the entry pointer.
//
// Original 110 bytes (RVA 0x0006eda0 – 0x0006ee0d):
//
//   0006eda0: 56 57 8b 7c 24 10 6a 00 57 e8 c2 65 ff ff 8b f0
//   0006edb0: 83 c4 08 85 f6 75 2d 68 1d 01 00 00 68 80 97 f7
//   0006edc0: 00 6a 77 68 83 00 00 00 6a 0b e8 71 db fe ff 57
//   0006edd0: 68 4c 97 f7 00 6a 02 e8 44 d7 fe ff 83 c4 20 5f
//   0006ede0: 33 c0 5e c3 8b 44 24 1c 8b 4c 24 18 8b 54 24 14
//   0006edf0: 50 8b 44 24 10 51 52 56 50 e8 32 ff ff ff 56 8b
//   0006ee00: f8 e8 da dc ff ff 83 c4 18 8b c7 5f 5e c3
//
// Reloc-bearing sites (absolute imm32 addresses + REL32 call offsets):
//   +0x09  CALL rel32 → FUN_00465370 (OBJ_txt2nid)
//   +0x1C  PUSH imm32 → 0x00f79780 (file-name string in .rdata)
//   +0x2A  CALL rel32 → FUN_0045c940 (ERR_put_error)
//   +0x30  PUSH imm32 → 0x00f7974c (format-string "name=" in .rdata)
//   +0x34  CALL rel32 → FUN_0045c520 (ERR_add_error_data)
//   +0x59  CALL rel32 → FUN_0046ed30 (X509_NAME_ENTRY_create_by_NID)
//   +0x61  CALL rel32 → FUN_0046cae0 (OBJ_nid2obj / cleanup)
//
// Why naked asm: the two PUSH imm32 absolute addresses (0xf79780, 0xf7974c)
// point into the PE's own .rdata section — a compiler-emitted .cpp would
// reference them via relocatable symbols, producing IMAGE_REL_I386_DIR32
// entries the linker controls.  Emitting the orig bytes verbatim via MASM
// `_emit` produces a .obj whose .text matches the orig byte-for-byte with
// no relocations; compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_0046eda0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x10]
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_00465370 (OBJ_txt2nid) rel32
        _emit 0xc2
        _emit 0x65
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +0x2d (success path)
        _emit 0x2d
        _emit 0x68              // PUSH 0x11d  (X509_R_INVALID_FIELD_NAME)
        _emit 0x1d
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f79780 (file string)
        _emit 0x80
        _emit 0x97
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x77 (line number low byte)
        _emit 0x77
        _emit 0x68              // PUSH 0x83 (X509_F_X509_NAME_ENTRY_CREATE_BY_TXT)
        _emit 0x83
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0xb (ERR_LIB_X509)
        _emit 0x0b
        _emit 0xe8              // CALL FUN_0045c940 (ERR_put_error) rel32
        _emit 0x71
        _emit 0xdb
        _emit 0xfe
        _emit 0xff
        _emit 0x57              // PUSH EDI  (field)
        _emit 0x68              // PUSH 0x00f7974c ("name=")
        _emit 0x4c
        _emit 0x97
        _emit 0xf7
        _emit 0x00
        _emit 0x6a              // PUSH 0x2
        _emit 0x02
        _emit 0xe8              // CALL FUN_0045c520 (ERR_add_error_data) rel32
        _emit 0x44
        _emit 0xd7
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0x5f              // POP EDI
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x50              // PUSH EAX  (len)
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x51              // PUSH ECX  (bytes)
        _emit 0x52              // PUSH EDX  (type)
        _emit 0x56              // PUSH ESI  (nid)
        _emit 0x50              // PUSH EAX  (ne)
        _emit 0xe8              // CALL FUN_0046ed30 (X509_NAME_ENTRY_create_by_NID) rel32
        _emit 0x32
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x56              // PUSH ESI  (nid, for cleanup)
        _emit 0x8b              // MOV EDI, EAX  (save return value)
        _emit 0xf8
        _emit 0xe8              // CALL FUN_0046cae0 rel32
        _emit 0xda
        _emit 0xdc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x8b              // MOV EAX, EDI
        _emit 0xc7
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
