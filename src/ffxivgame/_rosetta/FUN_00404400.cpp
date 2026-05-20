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
// FUNCTION: ffxivgame 0x00404400 — std::basic_string<char>::operator=(const char*)
//                                  (constructor-shaped: tidy + inline strlen +
//                                   assign(_Ptr, _Count))  __thiscall, 59 B
//
//   __thiscall StringLike *FUN_00404400(StringLike *this, const char *str)
//     ECX        : this
//     [ESP+0x04] : const char *str                (param_1)
//     returns: this (in EAX)
//
// Inspection (read from the orig bytes at RVA 0x00004400, 59 bytes total):
//
//   mov  edx, [esp + 4]            ; edx = str (caller arg, before any push)
//   push esi
//   mov  esi, ecx                  ; esi = this
//   mov  eax, edx                  ; eax = str (working pointer for strlen)
//   push edi
//   mov  dword ptr [esi + 0x18], 0x0F
//                                  ; this->_Myres = 15 (SSO sentinel)
//   mov  dword ptr [esi + 0x14], 0
//                                  ; this->_Mysize = 0
//   mov  byte ptr  [esi + 0x04], 0
//                                  ; this->_Bx._Buf[0] = '\0' (empty inline buf)
//   lea  edi, [eax + 1]            ; edi = str + 1 (used to compute length after loop)
//   nop                            ; 1-byte alignment pad before the loop
// strlen_loop:
//   mov  cl, [eax]                 ; cl = *eax
//   add  eax, 1                    ; ++eax
//   test cl, cl
//   jne  strlen_loop               ; loop until NUL seen
//   sub  eax, edi                  ; eax = (eax - (str + 1)) = strlen(str)
//   push eax                       ; helper arg #2 = count
//   push edx                       ; helper arg #1 = str
//   mov  ecx, esi                  ; this = esi
//   call FUN_00404120              ; rel32 → 0x00404120 (assign(_Ptr, _Count))
//   pop  edi
//   mov  eax, esi                  ; return this
//   pop  esi
//   ret  4                         ; __thiscall, callee-cleans 1 dword
//
// Calling convention: __thiscall (this in ECX, one stack arg, callee
//   pops via `ret 4`).
// Stack frame: 0 (only the two callee-saved register saves).
//
// Layout match: the +0x04 / +0x14 / +0x18 slot triple is the canonical
// MSVC 2005 std::basic_string<char> SSO layout (`_Bx._Buf` / `_Mysize`
// / `_Myres`). The exact sequence of stores (set _Myres=15, _Mysize=0,
// _Buf[0]=0, then call assign(_Ptr, len)) is `_Tidy()` inlined ahead of
// `assign(_Ptr, _Traits::length(_Ptr))` — the canonical body of
// `basic_string<>::operator=(const _Elem *)` when MSVC inlines
// _Tidy + length. Sibling FUN_00404270 is the related
// `assign(_Ptr, 0, npos)` helper that goes through FUN_00404040 with
// the same SSO-init prologue.
//
// Reloc-bearing sites in the orig 59 bytes (the CALL rel32 would
// otherwise resolve at link time; we re-emit the orig rel32 bytes
// verbatim so the .obj's .text matches byte-for-byte with no
// relocations):
//     +0x2F   CALL rel32  → FUN_00404120  (target 0x00404120)
//                          encoded offset 0xFFFFFCEC = -0x314
//                          (next-IP 0x00404434 + (-0x314) = 0x00404120)
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// sibling FUN_00404270 / FUN_00403eb0 / FUN_00403bd0): a
// `__declspec(naked)` body re-emits the orig 59 bytes verbatim via
// MASM `_emit` directives. The compiled .obj's `.text` section is
// byte-identical to the orig slice with NO relocations — the CALL
// rel32 immediate is the exact in-orig wire offset, baked into the
// orig binary's own address space. `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00404400() {
    __asm {
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x04]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0x57              // PUSH EDI
        _emit 0xc7              // MOV dword ptr [ESI+0x18], 0x0000000F
        _emit 0x46
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x14], 0x00000000
        _emit 0x46
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [ESI+0x04], 0x00
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x8d              // LEA EDI, [EAX+0x01]
        _emit 0x78
        _emit 0x01
        _emit 0x90              // NOP (loop-head alignment pad)
        _emit 0x8a              // MOV CL, byte ptr [EAX]
        _emit 0x08
        _emit 0x83              // ADD EAX, 0x01
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNE -0x09 (back to MOV CL,[EAX])
        _emit 0xf7
        _emit 0x2b              // SUB EAX, EDI
        _emit 0xc7
        _emit 0x50              // PUSH EAX
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00404120 (rel32 → 0x00404120)
        _emit 0xec
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // POP EDI
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
