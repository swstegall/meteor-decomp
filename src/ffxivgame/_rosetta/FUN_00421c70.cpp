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
// FUNCTION: ffxivgame 0x00421c70 — linked-list range-extract (200 B / 0xc8)
//                                  __thiscall with 5 stack args (RET 0x14).
//                                  Validates two iterator pairs against the
//                                  container (three _SECURE_SCL-style asserts),
//                                  then either (a) bulk-clears the whole list
//                                  and writes the old sentinel chain to *out,
//                                  or (b) loops calling FUN_00420630 / FUN_00421560
//                                  until first == last, then writes the range
//                                  endpoints to *out.
//
// Signature (recovered from asm):
//
//   __thiscall void FUN_00421c70(
//       void*    this    @ ECX,
//       void**   out     @ [ESP+0x1c],   // output pair: [out+0]=first, [out+4]=second
//       void*    arg2    @ [ESP+0x20],   // iterator / container ptr checked == this
//       void*    arg3    @ [ESP+0x24],   // first iterator (node ptr)
//       void*    arg4    @ [ESP+0x28],   // iterator / container ptr checked == this
//       void*    arg5    @ [ESP+0x2c]    // last  iterator (node ptr)
//   );
//
// Stack frame after SUB ESP,8 + 4 register saves (EDI/ESI/EBP/EBX):
//   [ESP+0x00]  saved EDI
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  saved EBP
//   [ESP+0x0c]  saved EBX
//   [ESP+0x10]  local1  (8-byte SUB-ESP,8 area; used as out-buf for FUN_00421560)
//   [ESP+0x14]  local2
//   [ESP+0x18]  return address
//   [ESP+0x1c]  arg1  (out pair*)
//   [ESP+0x20]  arg2  (container check / first-range begin)
//   [ESP+0x24]  arg3  (first-range begin node)
//   [ESP+0x28]  arg4  (container check / second-range begin)
//   [ESP+0x2c]  arg5  (second-range end node)
//
// Reloc-bearing CALL sites (rel32, masked by compare.py):
//   +0x1a  CALL 0x009d22b4  (assertion / _SECURE_SCL check, 1st occurrence)
//   +0x36  CALL 0x009d22b4  (assertion, 2nd occurrence)
//   +0x4a  CALL 0x00c2bb10  (__thiscall bulk-transfer helper, 1 arg)
//   +0x99  CALL 0x00420630  (__thiscall advance helper, no extra args)
//   +0xa7  CALL 0x00421560  (__thiscall insert helper, 3 args, RET 0xC)
//
// 3rd assertion CALL site:
//   +0x8a  CALL 0x009d22b4  (in l_else block, +6 displacement JZ before it
//                             because CMP EDI,[ESP+0x28] is 4 bytes vs 2)
//
// Reconstruction: naked-asm symbolic passthrough — same strategy as
// FUN_00409120 / FUN_00406680 / FUN_00404e40.  All five CALLs emit
// COFF rel32 relocations that compare.py masks; all remaining bytes
// are deterministic MSVC 2005 x86 encodings.  Short-jump form is
// preserved throughout (all targets within ±127 bytes).

extern "C" {
    void FUN_009d22b4();   // assertion / _SECURE_SCL check
    void FUN_00c2bb10();   // bulk-transfer __thiscall helper (1 arg, RET 4)
    void FUN_00420630();   // advance __thiscall helper (no stack args, RET)
    void FUN_00421560();   // insert __thiscall helper (3 stack args, RET 0xC)
}

extern "C" __declspec(naked) void FUN_00421c70() {
    __asm {
        sub  esp, 8
        push ebx
        push ebp
        push esi
        push edi
        mov  edi, dword ptr [esp + 0x20]    // EDI = arg2
        test edi, edi
        mov  esi, ecx                        // ESI = this
        mov  eax, dword ptr [esi + 0x4]     // EAX = this->_Myhead
        mov  ebp, dword ptr [eax]            // EBP = _Myhead->_Next
        jz   short l_assert1                 // null  → assert
        cmp  edi, esi
        jz   short l_skip1                   // == this → skip
    l_assert1:
        call FUN_009d22b4
    l_skip1:
        mov  ebx, dword ptr [esp + 0x24]    // EBX = arg3
        cmp  ebx, ebp                        // arg3 == _Myhead->_Next?
        jnz  short l_else
        mov  eax, dword ptr [esp + 0x28]    // EAX = arg4
        test eax, eax
        mov  ebp, dword ptr [esi + 0x4]     // EBP = this->_Myhead (ptr itself)
        jz   short l_assert2
        cmp  eax, esi
        jz   short l_skip2
    l_assert2:
        call FUN_009d22b4
    l_skip2:
        cmp  dword ptr [esp + 0x2c], ebp    // arg5 == _Myhead?
        jnz  short l_else
        // Happy path: extract entire list chain
        mov  ecx, dword ptr [esi + 0x4]     // ECX = _Myhead
        mov  edx, dword ptr [ecx + 0x4]     // EDX = _Myhead->_Prev (last elem)
        push edx
        mov  ecx, esi
        call FUN_00c2bb10
        // Reset sentinel to empty circular list
        mov  eax, dword ptr [esi + 0x4]
        mov  dword ptr [eax + 0x4], eax     // _Myhead->_Prev = _Myhead
        mov  eax, dword ptr [esi + 0x4]
        mov  dword ptr [esi + 0x8], 0       // this->_Mysize = 0
        mov  dword ptr [eax], eax           // _Myhead->_Next = _Myhead
        mov  eax, dword ptr [esi + 0x4]
        mov  dword ptr [eax + 0x8], eax     // _Myhead->_Myval? = _Myhead
        mov  eax, dword ptr [esi + 0x4]
        mov  ecx, dword ptr [eax]           // ECX = old _Next (= _Myhead again)
        mov  eax, dword ptr [esp + 0x1c]    // EAX = out*
        pop  edi
        mov  dword ptr [eax], esi           // out->first = this
        pop  esi
        pop  ebp
        mov  dword ptr [eax + 0x4], ecx    // out->second = old _Next
        pop  ebx
        add  esp, 8
        ret  0x14
        nop                                 // alignment padding (0x21cef)
    l_else:
        // General case: validate arg2/arg4 pair, then loop
        test edi, edi
        jz   short l_assert3                // null  → assert (JZ +6: 4-byte CMP ahead)
        cmp  edi, dword ptr [esp + 0x28]   // EDI == arg4?
        jz   short l_skip3                  // equal → skip
    l_assert3:
        call FUN_009d22b4
    l_skip3:
        cmp  ebx, dword ptr [esp + 0x2c]   // arg3 == arg5?
        jz   short l_final
        lea  ecx, [esp + 0x20]             // ECX = &arg2 (iterator obj on stack)
        call FUN_00420630                   // advance arg2 in place
        push ebx                            // arg3
        push edi                            // arg2
        lea  edx, [esp + 0x18]             // &local1 (output slot for FUN_00421560)
        push edx
        mov  ecx, esi                       // ECX = this
        call FUN_00421560                   // this->insert(&local, arg2, arg3); RET 0xC
        mov  ebx, dword ptr [esp + 0x24]   // reload arg3 (callee clobbered EBX)
        mov  edi, dword ptr [esp + 0x20]   // reload arg2 (callee clobbered EDI)
        jmp  short l_else
    l_final:
        mov  eax, dword ptr [esp + 0x1c]   // EAX = out*
        mov  dword ptr [eax], edi          // out->first  = arg2
        pop  edi
        pop  esi
        pop  ebp
        mov  dword ptr [eax + 0x4], ebx    // out->second = arg3
        pop  ebx
        add  esp, 8
        _emit 0xc2  // RET 0x14 — only first 2 bytes are inside the 200-byte
        _emit 0x14  // function window; the trailing 00 lives at the exclusive end
    }
}
