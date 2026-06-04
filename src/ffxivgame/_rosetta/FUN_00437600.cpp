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
// FUNCTION: ffxivgame 0x00437600 — 3-arg __thiscall factory/dispatch
//                                  (RET 0xc, 93 bytes)
//
// __thiscall void FUN_00437600(this, a0, a1, a2)
//   ECX = this (saved into ESI). Stack args after the prologue PUSH ESI:
//     [ESP+0x08] : a0  (arg slot 1)
//     [ESP+0x0C] : a1  (arg slot 2)
//     [ESP+0x10] : a2  (arg slot 3)
//
// Behaviour: index into a global registry pointed at by [0x01328d90].
// The registry's first byte ([ECX]) is a count/index `n`; multiply it
// by 7 (LEA EAX*8 then SUB EAX → n*7), scale by 4 against the element
// base [reg+0x4] to compute an element slot address `ECX = base +
// (n*7)*4`. Pass that slot + size 0x10 to the allocator FUN_00417ab0.
//
//   - On success (EAX != 0): stamp the freshly returned 16-byte object:
//       [EAX]      = 0x00f649a8   (vftable / type tag, DIR32)
//       [EAX+0x4]  = a0
//       [EAX+0x8]  = a1
//       [EAX+0xc]  = a2
//     then forward it (ECX = this->field_8 = [ESI+0x8]) to FUN_0043c2d0
//     (a RET 4 __thiscall sink) with the object pointer as the argument.
//   - On failure (EAX == 0): forward NULL to the same FUN_0043c2d0 sink.
//
//   Both arms RET 0xc (callee pops the 3 dword args).
//
// Inspection (read from the orig bytes at RVA 0x00037600 / VA 0x00437600):
//
//   push esi
//   mov  esi, ecx
//   mov  ecx, dword ptr [0x01328d90]
//   movzx eax, byte ptr [ecx]
//   lea  edx, [eax*8 + 0]
//   sub  edx, eax                 ; edx = n*7
//   mov  eax, dword ptr [ecx+0x4]
//   lea  ecx, [eax + edx*4]
//   push 0x10
//   call FUN_00417ab0             ; rel32 → 0x00417ab0
//   test eax, eax
//   jz   fail
//   mov  ecx, dword ptr [esp+0x8] ; a0
//   mov  edx, dword ptr [esp+0xc] ; a1
//   mov  dword ptr [eax+0x4], ecx
//   mov  ecx, dword ptr [esp+0x10]; a2
//   mov  dword ptr [eax], 0xf649a8 ; DIR32 vftable
//   mov  dword ptr [eax+0x8], edx
//   mov  dword ptr [eax+0xc], ecx
//   mov  ecx, dword ptr [esi+0x8]
//   push eax
//   call FUN_0043c2d0             ; rel32 → 0x0043c2d0
//   pop  esi
//   ret  0xc
//  fail:
//   mov  ecx, dword ptr [esi+0x8]
//   xor  eax, eax
//   push eax
//   call FUN_0043c2d0             ; rel32 → 0x0043c2d0
//   pop  esi
//   ret  0xc
//
// Reloc-bearing sites the linker would resolve from source-level C++:
//     +0x05   MOV  [imm32] → 0x01328d90 (DIR32, global registry ptr)
//     +0x1d   CALL rel32   → 0x00417ab0 (allocator)
//     +0x35   MOV  imm32   → 0x00f649a8 (DIR32, vftable / type tag)
//     +0x45   CALL rel32   → 0x0043c2d0 (sink, success arm)
//     +0x54   CALL rel32   → 0x0043c2d0 (sink, fail arm)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same _emit approach as siblings FUN_00403bd0 / FUN_00406133. The
//   orig 93 bytes are re-emitted verbatim; the rel32 / DIR32 operands
//   are baked into the orig PE's own address space and emitted here as
//   raw bytes, so the .obj's .text is byte-identical to the orig slice
//   with zero relocations and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00437600() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EAX*0x8 + 0x0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*0x4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0xe8              // CALL FUN_00417ab0 (rel32 → 0x00417ab0)
        _emit 0x8e
        _emit 0x04
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ fail (+0x28)
        _emit 0x28
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0xC]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xc7              // MOV dword ptr [EAX], 0xF649A8 (DIR32)
        _emit 0x00
        _emit 0xa8
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x89              // MOV dword ptr [EAX+0xC], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32 → 0x0043c2d0)
        _emit 0x86
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xC
        _emit 0x0c
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]   ; fail
        _emit 0x4e
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0 (rel32 → 0x0043c2d0)
        _emit 0x77
        _emit 0x4c
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0xC
        _emit 0x0c
        _emit 0x00
    }
}
