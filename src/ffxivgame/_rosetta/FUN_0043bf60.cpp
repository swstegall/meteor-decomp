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
// FUNCTION: ffxivgame 0x0003bf60 — double-buffer pointer swap (__thiscall, 37 B / 0x25)
//
// Toggles two internal pointer fields at offsets 0x24 and 0xbc between
// two embedded data blobs at this+0x10 and this+0xa8.
//
// On each call:
//   if (this->ptr_bc == &this->blob_10)  →  ptr_bc = &blob_a8, ptr_24 = &blob_10
//   else                                 →  ptr_24 = &blob_a8, ptr_bc = &blob_10
//
// Calling convention : __thiscall (ECX = this, plain RET — no stack args)
// Frame              : none (/Oy, no callee-saves)
// Relocations        : zero (no CALL sites) — naked passthrough is exact.
//
// Asm (37 bytes @ orig RVA 0x0003bf60):
//   8d 41 10              LEA EAX, [ECX + 0x10]       ; &blob_10
//   39 81 bc 00 00 00     CMP dword ptr [ECX+0xbc], EAX  ; ptr_bc == &blob_10?
//   8d 91 a8 00 00 00     LEA EDX, [ECX + 0xa8]       ; &blob_a8 (hoisted before JNZ)
//   75 0a                 JNZ  +0x0a                  ; not equal → else branch
//   89 91 bc 00 00 00     MOV dword ptr [ECX+0xbc], EDX  ; ptr_bc = &blob_a8
//   89 41 24              MOV dword ptr [ECX+0x24], EAX  ; ptr_24 = &blob_10
//   c3                    RET
//   89 51 24              MOV dword ptr [ECX+0x24], EDX  ; ptr_24 = &blob_a8
//   89 81 bc 00 00 00     MOV dword ptr [ECX+0xbc], EAX  ; ptr_bc = &blob_10
//   c3                    RET

extern "C" __declspec(naked) void FUN_0043bf60()
{
    __asm {
        // 0003bf60: 8d 41 10   LEA EAX, [ECX + 0x10]
        _emit 0x8d
        _emit 0x41
        _emit 0x10
        // 0003bf63: 39 81 bc 00 00 00   CMP dword ptr [ECX + 0xbc], EAX
        _emit 0x39
        _emit 0x81
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003bf69: 8d 91 a8 00 00 00   LEA EDX, [ECX + 0xa8]
        _emit 0x8d
        _emit 0x91
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003bf6f: 75 0a   JNZ +0x0a (→ 0x0043bf7b)
        _emit 0x75
        _emit 0x0a
        // 0003bf71: 89 91 bc 00 00 00   MOV dword ptr [ECX + 0xbc], EDX
        _emit 0x89
        _emit 0x91
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003bf77: 89 41 24   MOV dword ptr [ECX + 0x24], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x24
        // 0003bf7a: c3   RET
        _emit 0xc3
        // 0003bf7b: 89 51 24   MOV dword ptr [ECX + 0x24], EDX
        _emit 0x89
        _emit 0x51
        _emit 0x24
        // 0003bf7e: 89 81 bc 00 00 00   MOV dword ptr [ECX + 0xbc], EAX
        _emit 0x89
        _emit 0x81
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003bf84: c3   RET
        _emit 0xc3
    }
}
