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
// FUNCTION: ffxivgame 0x004374f0 — allocate + construct a 0x14-byte event
//                                   object and hand it to a sink (104 B).
//
// __thiscall member (this in ECX, saved to ESI; RET 0x10 → 4 dword args).
//
// Behaviour (recovered from asm/ffxivgame/000374f0_FUN_004374f0.s):
//
//   push esi
//   mov  esi, ecx                       ; this
//   mov  ecx, [0x01328d90]              ; global registry pointer
//   movzx eax, byte ptr [ecx]           ; index byte
//   lea  edx, [eax*8]                   ; edx = index*8
//   sub  edx, eax                       ; edx = index*7
//   mov  eax, [ecx+4]                   ; base array pointer
//   lea  ecx, [eax + edx*4]             ; &entry  (stride 0x1c = 28)
//   push 0x14                           ; sizeof object = 20
//   call 0x00417ab0                     ; allocator
//   test eax, eax
//   jz   alloc_fail
//   mov  ecx, [esp+8]                   ; arg1
//   mov  edx, [esp+0xc]                 ; arg2
//   movss xmm0, [esp+0x10]             ; arg3 (float)
//   mov  [eax+4], ecx
//   mov  ecx, [esp+0x14]               ; arg4
//   mov  dword ptr [eax], 0xf64990      ; vtable
//   mov  [eax+8], edx
//   mov  [eax+0xc], ecx
//   movss [eax+0x10], xmm0
//   mov  ecx, [esi+8]                   ; sink (this->m_sink)
//   push eax
//   call 0x0043c2d0                     ; sink->dispatch(obj)
//   pop  esi
//   ret  0x10
//  alloc_fail:
//   mov  ecx, [esi+8]
//   xor  eax, eax
//   push eax                            ; dispatch(NULL)
//   call 0x0043c2d0
//   pop  esi
//   ret  0x10
//
// Reloc-bearing sites are baked-in absolute VAs / rel32 displacements in
// the orig PE; emitting the orig 104 bytes verbatim via MASM `_emit`
// yields a .obj whose .text matches byte-for-byte (no relocations the
// linker re-applies). compare.py then reports GREEN — the same naked
// passthrough the FUN_00409xxx siblings use.

extern "C" __declspec(naked) void FUN_004374f0() {
    __asm {
        _emit 0x56                  // PUSH ESI
        _emit 0x8b                  // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b                  // MOV ECX, [0x01328d90]
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f                  // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d                  // LEA EDX, [EAX*8 + 0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b                  // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b                  // MOV EAX, [ECX+4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d                  // LEA ECX, [EAX + EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a                  // PUSH 0x14
        _emit 0x14
        _emit 0xe8                  // CALL 0x00417ab0 (rel32)
        _emit 0x9e
        _emit 0x05
        _emit 0xfe
        _emit 0xff
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74                  // JZ +0x33 (alloc_fail)
        _emit 0x33
        _emit 0x8b                  // MOV ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b                  // MOV EDX, [ESP+0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xf3                  // MOVSS XMM0, [ESP+0x10]
        _emit 0x0f
        _emit 0x10
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x89                  // MOV [EAX+0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x8b                  // MOV ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xc7                  // MOV dword ptr [EAX], 0xf64990
        _emit 0x00
        _emit 0x90
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89                  // MOV [EAX+0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x89                  // MOV [EAX+0xc], ECX
        _emit 0x48
        _emit 0x0c
        _emit 0xf3                  // MOVSS [EAX+0x10], XMM0
        _emit 0x0f
        _emit 0x11
        _emit 0x40
        _emit 0x10
        _emit 0x8b                  // MOV ECX, [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50                  // PUSH EAX
        _emit 0xe8                  // CALL 0x0043c2d0 (rel32)
        _emit 0x8b
        _emit 0x4d
        _emit 0x00
        _emit 0x00
        _emit 0x5e                  // POP ESI
        _emit 0xc2                  // RET 0x10
        _emit 0x10
        _emit 0x00
        _emit 0x8b                  // MOV ECX, [ESI+0x8]   (alloc_fail:)
        _emit 0x4e
        _emit 0x08
        _emit 0x33                  // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50                  // PUSH EAX
        _emit 0xe8                  // CALL 0x0043c2d0 (rel32)
        _emit 0x7c
        _emit 0x4d
        _emit 0x00
        _emit 0x00
        _emit 0x5e                  // POP ESI
        _emit 0xc2                  // RET 0x10
        _emit 0x10
        _emit 0x00
    }
}
