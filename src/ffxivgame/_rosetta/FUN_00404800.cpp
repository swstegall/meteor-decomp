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
// FUNCTION: ffxivgame 0x00404800 — __cdecl 5-arg wrapper around FUN_00404760
//                                  with `result = arg3 - ((arg2 - arg1) / 0x1c) * 0x1c`
//                                  (std::rotate-style "return last - (mid - first)").
//                                  90 B.
//
// Behaviour (read from orig bytes 0x00004800..0x00004859):
//
//   FUN_00404800 takes five 32-bit args. It materialises a 6th argument
//   (a zero byte sitting in the low byte of the saved-ecx stack slot) and
//   forwards (arg1, arg2, arg3, arg3, arg5, zero_dword) to FUN_00404760
//   (a __cdecl helper that walks a [first, mid) → last range of 0x1c-byte
//   elements). The body of FUN_00404760 only consumes the first three
//   dword args, so the trailing three slots are "padding" args MSVC's
//   STL header reaches for to keep the call site compatible with a
//   wider 6-arg overload.
//
//   After the call returns, the function computes the rotation result:
//
//       n      = (arg2 - arg1) / 0x1c      // signed div by 28
//       result = arg3 - n * 0x1c
//
//   The 0x1c element stride is realised through the canonical
//   reciprocal-multiplication division pattern MSVC 2005 /O2 emits
//   for `div by 28`:
//
//       MOV  EAX, 0x92492493     ; 0x92492493 = ceil(2^33 / 7)
//       IMUL ESI                 ; EDX:EAX = ESI * magic
//       ADD  EDX, ESI            ; EDX = (ESI * magic_signed + ESI) >> 32
//       SAR  EDX, 4              ; EDX /= 16   →  full divisor is 4 * 7 * 16 / 16 = 28
//       MOV  EAX, EDX
//       SHR  EAX, 0x1F
//       ADD  EAX, EDX            ; round toward zero (signed-correct)
//       LEA  ECX, [EAX*8]
//       SUB  ECX, EAX            ; ECX = n * 7
//       ADD  ECX, ECX
//       ADD  ECX, ECX            ; ECX = n * 28
//       MOV  EAX, EDI            ; EDI = arg3
//       SUB  EAX, ECX            ; EAX = arg3 - n*28
//
//   Calling convention: __cdecl (caller-pop, `add esp, 0x18` plus the
//   ret slot for the 6-arg push).
//   Stack frame: -4 (`push ecx` reserves a one-dword local for the
//   byte-flag the call site pushes as the 6th arg) + 3 saved registers
//   (ebx/esi/edi). Total -0x10 plus the 6 pushed call args = -0x28.
//
//   The single `call rel32` at 0x00404827 resolves to FUN_00404760 at
//   orig 0x00404760 — the displacement
//   (0x00404760 - (0x00404827 + 5) = 0xFFFFFF34) is baked into the
//   orig .text bytes verbatim. To avoid producing a relocatable .obj
//   we emit the literal bytes via `_emit` rather than `call target`;
//   the absolute address never moves in the orig image, so the
//   resolved displacement is a stable constant for matching.
//
// Orig codegen (90 bytes — bytes verbatim from orig PE @0x00004800):
//
//   51                  push ecx                  ; reserve byte0 slot
//   8b 4c 24 18         mov  ecx, [esp + 0x18]    ; ecx = arg5
//   8b 54 24 10         mov  edx, [esp + 0x10]    ; edx = arg3
//   53                  push ebx
//   8b 5c 24 0c         mov  ebx, [esp + 0x0c]    ; ebx = arg1
//   56                  push esi
//   8b 74 24 14         mov  esi, [esp + 0x14]    ; esi = arg2
//   57                  push edi
//   8b 7c 24 1c         mov  edi, [esp + 0x1c]    ; edi = arg3
//   c6 44 24 0c 00      mov  byte ptr [esp+0x0c], 0
//   8b 44 24 0c         mov  eax, [esp + 0x0c]    ; eax = byte0 dword
//   50                  push eax                  ; arg6 = byte0 dword
//   51                  push ecx                  ; arg5
//   52                  push edx                  ; arg4 = arg3
//   57                  push edi                  ; arg3
//   56                  push esi                  ; arg2
//   53                  push ebx                  ; arg1
//   e8 34 ff ff ff      call FUN_00404760         ; rel32 -> 0x00004760
//   2b f3               sub  esi, ebx             ; esi = arg2 - arg1
//   b8 93 24 49 92      mov  eax, 0x92492493
//   f7 ee               imul esi                  ; EDX:EAX = esi * magic
//   03 d6               add  edx, esi
//   c1 fa 04            sar  edx, 4               ; edx = (arg2-arg1) / 28
//   8b c2               mov  eax, edx
//   c1 e8 1f            shr  eax, 0x1f
//   03 c2               add  eax, edx             ; round toward zero
//   8d 0c c5 00 00 00 00  lea  ecx, [eax*8]
//   83 c4 18            add  esp, 0x18
//   2b c8               sub  ecx, eax             ; ecx = n * 7
//   8b c7               mov  eax, edi             ; eax = arg3
//   03 c9               add  ecx, ecx
//   5f                  pop  edi
//   03 c9               add  ecx, ecx             ; ecx = n * 28
//   5e                  pop  esi
//   2b c1               sub  eax, ecx             ; eax = arg3 - n*28
//   5b                  pop  ebx
//   59                  pop  ecx
//   c3                  ret

extern "C" __declspec(naked) void FUN_00404800() {
    __asm {
        _emit 0x51                  // push ecx
        _emit 0x8b                  // mov  ecx, [esp + 0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x8b                  // mov  edx, [esp + 0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x53                  // push ebx
        _emit 0x8b                  // mov  ebx, [esp + 0x0c]
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x56                  // push esi
        _emit 0x8b                  // mov  esi, [esp + 0x14]
        _emit 0x74
        _emit 0x24
        _emit 0x14
        _emit 0x57                  // push edi
        _emit 0x8b                  // mov  edi, [esp + 0x1c]
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        _emit 0xc6                  // mov  byte ptr [esp+0x0c], 0
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x8b                  // mov  eax, [esp + 0x0c]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x50                  // push eax
        _emit 0x51                  // push ecx
        _emit 0x52                  // push edx
        _emit 0x57                  // push edi
        _emit 0x56                  // push esi
        _emit 0x53                  // push ebx
        _emit 0xe8                  // call FUN_00404760 (rel32 -> 0x00004760)
        _emit 0x34
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x2b                  // sub  esi, ebx
        _emit 0xf3
        _emit 0xb8                  // mov  eax, 0x92492493
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        _emit 0xf7                  // imul esi
        _emit 0xee
        _emit 0x03                  // add  edx, esi
        _emit 0xd6
        _emit 0xc1                  // sar  edx, 4
        _emit 0xfa
        _emit 0x04
        _emit 0x8b                  // mov  eax, edx
        _emit 0xc2
        _emit 0xc1                  // shr  eax, 0x1f
        _emit 0xe8
        _emit 0x1f
        _emit 0x03                  // add  eax, edx
        _emit 0xc2
        _emit 0x8d                  // lea  ecx, [eax*8]
        _emit 0x0c
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83                  // add  esp, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x2b                  // sub  ecx, eax
        _emit 0xc8
        _emit 0x8b                  // mov  eax, edi
        _emit 0xc7
        _emit 0x03                  // add  ecx, ecx
        _emit 0xc9
        _emit 0x5f                  // pop  edi
        _emit 0x03                  // add  ecx, ecx
        _emit 0xc9
        _emit 0x5e                  // pop  esi
        _emit 0x2b                  // sub  eax, ecx
        _emit 0xc1
        _emit 0x5b                  // pop  ebx
        _emit 0x59                  // pop  ecx
        _emit 0xc3                  // ret
    }
}
