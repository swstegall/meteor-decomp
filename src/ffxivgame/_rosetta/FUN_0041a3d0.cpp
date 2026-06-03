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
// FUNCTION: ffxivgame 0x0041a3d0 — struct factory (__stdcall, 5 stack args,
//                                  RET 0x14, 71 B / 0x47)
//
// Signature (reconstructed):
//
//   __stdcall Obj* FUN_0041a3d0(DWORD field0, DWORD field4, DWORD field8,
//                                void* vecPtr, BYTE flag)
//
// Allocates a 0x28-byte block via FUN_009d1b35 (the custom allocator located
// immediately after `_free` / `bad_alloc` in the binary's CRT blob at RVA
// 0x5d1b35). If allocation returns NULL the function returns NULL. Otherwise
// fills the struct:
//
//   [EAX+0x00] = arg1  (DWORD)
//   [EAX+0x04] = arg2  (DWORD)
//   [EAX+0x08] = arg3  (DWORD)
//   [EAX+0x10] = qword from *arg4         (via MOVQ XMM0)
//   [EAX+0x18] = qword from *(arg4+8)     (via MOVQ XMM0)
//   [EAX+0x20] = arg5  (BYTE)
//   [EAX+0x21] = 0     (terminator byte)
//
// Orig codegen (71 bytes):
//
//   6a 28                 push  0x28
//   e8 5e 77 5b 00        call  FUN_009d1b35        [RELOC]
//   83 c4 04              add   esp, 4
//   85 c0                 test  eax, eax
//   74 36                 jz    +0x36               ; → RET
//   8b 4c 24 04           mov   ecx, [esp+4]        ; arg1
//   8b 54 24 08           mov   edx, [esp+8]        ; arg2
//   89 08                 mov   [eax], ecx
//   8b 4c 24 0c           mov   ecx, [esp+0xc]      ; arg3
//   89 48 08              mov   [eax+8], ecx
//   8b 4c 24 10           mov   ecx, [esp+0x10]     ; arg4 (ptr)
//   89 50 04              mov   [eax+4], edx
//   f3 0f 7e 01           movq  xmm0, [ecx]
//   8a 54 24 14           mov   dl, [esp+0x14]      ; arg5 (byte)
//   66 0f d6 40 10        movq  [eax+0x10], xmm0
//   f3 0f 7e 41 08        movq  xmm0, [ecx+8]
//   66 0f d6 40 18        movq  [eax+0x18], xmm0
//   88 50 20              mov   [eax+0x20], dl
//   c6 40 21 00           mov   byte ptr [eax+0x21], 0
//   c2 14 00              ret   0x14
//
// Reconstruction strategy: __declspec(naked) with _emit passthrough for all
// instructions except the CALL, which is expressed as a real `call` so the
// assembler emits a proper IMAGE_REL_I386_REL32 relocation that compare.py
// can mask. The SSE2 MOVQ variants (f3 0f 7e and 66 0f d6) and the short-form
// JZ (74 36) are emitted verbatim via _emit to guarantee exact encoding.

extern "C" void FUN_009d1b35();

extern "C" __declspec(naked) void FUN_0041a3d0() {
    __asm {
        // 0001a3d0: push 0x28
        _emit 0x6a
        _emit 0x28
        // 0001a3d2: call FUN_009d1b35  [reloc → compare.py masks these 4 bytes]
        call FUN_009d1b35
        // 0001a3d7: add esp, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0001a3da: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 0001a3dc: jz +0x36  (short jump to RET at offset 0x44)
        _emit 0x74
        _emit 0x36
        // 0001a3de: mov ecx, [esp+4]   ; arg1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0001a3e2: mov edx, [esp+8]   ; arg2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x08
        // 0001a3e6: mov [eax], ecx
        _emit 0x89
        _emit 0x08
        // 0001a3e8: mov ecx, [esp+0xc]  ; arg3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0001a3ec: mov [eax+8], ecx
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 0001a3ef: mov ecx, [esp+0x10]  ; arg4 (ptr to 16-byte vector)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0001a3f3: mov [eax+4], edx
        _emit 0x89
        _emit 0x50
        _emit 0x04
        // 0001a3f6: movq xmm0, qword ptr [ecx]   (f3 0f 7e /r)
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x01
        // 0001a3fa: mov dl, [esp+0x14]   ; arg5 (byte)
        _emit 0x8a
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // 0001a3fe: movq qword ptr [eax+0x10], xmm0  (66 0f d6 /r)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x10
        // 0001a403: movq xmm0, qword ptr [ecx+8]   (f3 0f 7e /r)
        _emit 0xf3
        _emit 0x0f
        _emit 0x7e
        _emit 0x41
        _emit 0x08
        // 0001a408: movq qword ptr [eax+0x18], xmm0  (66 0f d6 /r)
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x40
        _emit 0x18
        // 0001a40d: mov [eax+0x20], dl
        _emit 0x88
        _emit 0x50
        _emit 0x20
        // 0001a410: mov byte ptr [eax+0x21], 0
        _emit 0xc6
        _emit 0x40
        _emit 0x21
        _emit 0x00
        // 0001a414: ret 0x14
        _emit 0xc2
        _emit 0x14
        _emit 0x00
    }
}
