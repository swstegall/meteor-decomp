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
// FUNCTION: ffxivgame 0x0045a0f0 — assign-from-range helper for a custom
//                                  string type (81 B / 0x51, __cdecl, 2 args,
//                                  no local frame, no /GS).
//
// Signature (reconstructed from disassembly at orig RVA 0x0005a0f0):
//
//   __cdecl CustomString* FUN_0045a0f0(CustomString* dest, SourceRange* src);
//
// Behaviour:
//
//   SourceRange has (at minimum) two fields:
//     [+0x04]  char*  begin   — pointer to the first character
//     [+0x08]  char*  end     — pointer one-past the last character
//
//   CustomString has (at minimum) the following fields:
//     [+0x04]  char   _Buf[0] — first byte of the inline SSO buffer
//     [+0x14]  int    _Mysize — current string length
//     [+0x18]  int    _Myres  — capacity (0xf = 15 in SSO mode)
//
//   Logic:
//     size_t count = src->end - src->begin;
//     if (src->begin == NULL || count == 0) {
//         // "assign empty" fast path
//         dest->_Myres  = 0xf;   // SSO capacity = 15
//         dest->_Mysize = 0;
//         dest->_Buf[0] = '\0';
//         return dest;
//     }
//     // CMP count, 0; JA → always taken here (dead-code CALL follows)
//     // (the CALL to 0x009d22b4 is unreachable in all reachable cases)
//     return FUN_00459f00(dest, src->begin, count);
//
//   Return value: dest (the first argument) in both paths.
//
// Branch layout:
//   +0x12  JZ  +0x28  (begin == NULL → empty path)
//   +0x19  JZ  +0x21  (count == 0    → empty path)
//   +0x1d  JA  +0x05  (count > 0, unsigned — always taken; dead CALL at +0x1f)
//   +0x24  …         (normal assign path → FUN_00459f00)
//   +0x3c  …         (error / empty path — sets _Myres=0xf, _Mysize=0, _Buf[0]=0)
//
// Reloc-bearing instructions (REL32, masked by compare.py):
//   +0x1f  CALL 0x009d22b4  (dead code — likely a CRT error/assert helper)
//   +0x2e  CALL 0x00459f00  (the main assign implementation)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function is 81 bytes with two REL32 CALL relocs. The relative
//   operands in the orig PE are the post-link values; _emit reproduces them
//   verbatim so the .obj .text bytes match the original slice exactly.
//   compare.py masks the two 4-byte REL32 windows; all other bytes are
//   direct equality.

extern "C" __declspec(naked) void FUN_0045a0f0() {
    __asm {
        // +0x00: push ecx
        _emit 0x51
        // +0x01: push esi
        _emit 0x56
        // +0x02: push edi
        _emit 0x57
        // +0x03: mov edi, dword ptr [esp+14h]   ; EDI = arg2 (source range)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // +0x07: mov eax, dword ptr [edi+4]      ; EAX = src->begin
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // +0x0a: xor ecx, ecx                    ; ECX = 0
        _emit 0x33
        _emit 0xc9
        // +0x0c: cmp eax, ecx                    ; begin == NULL?
        _emit 0x3b
        _emit 0xc1
        // +0x0e: mov dword ptr [esp+8], ecx      ; zero out saved-ECX slot
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // +0x12: jz +0x28 (→ 0x0045a12c, empty path)
        _emit 0x74
        _emit 0x28
        // +0x14: mov esi, dword ptr [edi+8]      ; ESI = src->end
        _emit 0x8b
        _emit 0x77
        _emit 0x08
        // +0x17: sub esi, eax                    ; ESI = count = end - begin
        _emit 0x2b
        _emit 0xf0
        // +0x19: jz +0x21 (→ 0x0045a12c, empty path)
        _emit 0x74
        _emit 0x21
        // +0x1b: cmp esi, ecx                    ; compare count to 0
        _emit 0x3b
        _emit 0xf1
        // +0x1d: ja +0x05 (→ 0x0045a114, normal path; always taken when count != 0)
        _emit 0x77
        _emit 0x05
        // +0x1f: call 0x009d22b4   (dead code — REL32 reloc, masked by compare.py)
        _emit 0xe8
        _emit 0xa0
        _emit 0x81
        _emit 0x57
        _emit 0x00
        // +0x24: mov eax, dword ptr [edi+4]      ; reload src->begin
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // +0x27: push esi                        ; arg3: count
        _emit 0x56
        // +0x28: mov esi, dword ptr [esp+14h]    ; ESI = arg1 (dest) after the push
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // +0x2c: push eax                        ; arg2: begin
        _emit 0x50
        // +0x2d: push esi                        ; arg1: dest
        _emit 0x56
        // +0x2e: call FUN_00459f00   (REL32 reloc, masked by compare.py)
        _emit 0xe8
        _emit 0xdd
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // +0x33: add esp, 0ch                    ; cdecl caller cleanup (3 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // +0x36: pop edi
        _emit 0x5f
        // +0x37: mov eax, esi                    ; return dest
        _emit 0x8b
        _emit 0xc6
        // +0x39: pop esi
        _emit 0x5e
        // +0x3a: pop ecx
        _emit 0x59
        // +0x3b: ret
        _emit 0xc3
        // +0x3c: mov eax, dword ptr [esp+10h]    ; empty path: EAX = arg1 (dest)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // +0x40: pop edi
        _emit 0x5f
        // +0x41: mov dword ptr [eax+18h], 0fh   ; dest->_Myres = 15 (SSO capacity)
        _emit 0xc7
        _emit 0x40
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x48: mov dword ptr [eax+14h], ecx   ; dest->_Mysize = 0 (ECX=0)
        _emit 0x89
        _emit 0x48
        _emit 0x14
        // +0x4b: mov byte ptr [eax+4], cl       ; dest->_Buf[0] = '\0'
        _emit 0x88
        _emit 0x48
        _emit 0x04
        // +0x4e: pop esi
        _emit 0x5e
        // +0x4f: pop ecx
        _emit 0x59
        // +0x50: ret
        _emit 0xc3
    }
}
