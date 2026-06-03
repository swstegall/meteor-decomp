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
// FUNCTION: ffxivgame 0x009e1a2e — bounded byte-copy loop helper (32 B).
//
// Copies bytes from a source string (EDX) into *ECX (a pointer-to-pointer
// whose pointee advances in place) until the count at *EAX reaches zero
// or a NUL terminator is encountered in the source.  The count at *EAX
// is decremented for each byte written; *ECX is incremented past each
// written byte.
//
// Register calling convention:
//   EAX = pointer to remaining-count (int*); decremented per byte
//   ECX = pointer to destination pointer (char**); *ECX advanced per byte
//   EDX = source string (const char*)
//
// Returns: void (no stack frame; callee saves ESI + EDI only)
//
// Disassembly (32 bytes, no relocations):
//
//   009e1a2e: 83 38 00    cmp  dword ptr [eax], 0
//   009e1a31: 56          push esi
//   009e1a32: 8b f2       mov  esi, edx
//   009e1a34: 74 16       je   exit_outer              ; count == 0, done
//   009e1a36: 57          push edi
//   loop_body:
//   009e1a37: 8a 16       mov  dl, byte ptr [esi]
//   009e1a39: 84 d2       test dl, dl
//   009e1a3b: 74 0e       je   exit_inner              ; NUL found, done
//   009e1a3d: 8b 39       mov  edi, dword ptr [ecx]    ; dest = *pDest
//   009e1a3f: 88 17       mov  byte ptr [edi], dl      ; *dest = c
//   009e1a41: ff 01       inc  dword ptr [ecx]         ; (*pDest)++
//   009e1a43: 46          inc  esi                     ; src++
//   009e1a44: ff 08       dec  dword ptr [eax]         ; (*pCount)--
//   009e1a46: 83 38 00    cmp  dword ptr [eax], 0
//   009e1a49: 75 ec       jne  loop_body
//   exit_inner:
//   009e1a4b: 5f          pop  edi
//   exit_outer:
//   009e1a4c: 5e          pop  esi
//   009e1a4d: c3          ret

extern "C" __declspec(naked) void FUN_009e1a2e() {
    __asm {
        cmp  dword ptr [eax], 0             ; 83 38 00
        push esi                            ; 56
        mov  esi, edx                       ; 8b f2
        je   exit_outer                     ; 74 16
        push edi                            ; 57
    loop_body:
        mov  dl, byte ptr [esi]             ; 8a 16
        test dl, dl                         ; 84 d2
        je   exit_inner                     ; 74 0e
        mov  edi, dword ptr [ecx]           ; 8b 39
        mov  byte ptr [edi], dl             ; 88 17
        inc  dword ptr [ecx]                ; ff 01
        inc  esi                            ; 46
        dec  dword ptr [eax]                ; ff 08
        cmp  dword ptr [eax], 0             ; 83 38 00
        jne  loop_body                      ; 75 ec
    exit_inner:
        pop  edi                            ; 5f
    exit_outer:
        pop  esi                            ; 5e
        ret                                 ; c3
    }
}
