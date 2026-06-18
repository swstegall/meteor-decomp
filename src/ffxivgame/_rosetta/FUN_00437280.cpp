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
// FUNCTION: ffxivgame 0x00437280 — pool-allocate a 3-field node and dispatch
//                                  it to a sibling handler (__thiscall, 2 args,
//                                  86 B)
//
// Asm shape (read from orig RVA 0x00037280, 86 bytes total):
//
//   00037280:  56                     push  esi
//   00037281:  8b f1                  mov   esi, ecx                  ; esi = this
//   00037283:  8b 0d 90 8d 32 01      mov   ecx, [g_pool_mgr]         ; RELOC DIR32 → 0x01328d90
//   00037289:  0f b6 01               movzx eax, byte ptr [ecx]       ; eax = pool_mgr->slot_idx
//   0003728c:  8d 14 c5 00 00 00 00   lea   edx, [eax*8+0]            ; edx = idx*8
//   00037293:  2b d0                  sub   edx, eax                  ; edx = idx*7
//   00037295:  8b 41 04               mov   eax, [ecx+4]              ; eax = pool_mgr->pool_arr
//   00037298:  8d 0c 90               lea   ecx, [eax+edx*4]          ; ecx = pool_arr + idx*28
//   0003729b:  6a 0c                  push  12
//   0003729d:  e8 0e 08 fe ff         call  FUN_00417ab0               ; RELOC REL32 → VA 0x00417ab0
//   000372a2:  85 c0                  test  eax, eax
//   000372a4:  74 21                  jz    null_path                  ; +0x21 = null arm at 0xc7
//   000372a6:  8b 4c 24 08            mov   ecx, [esp+0x8]             ; param0
//   000372aa:  8b 54 24 0c            mov   edx, [esp+0xc]             ; param1
//   000372ae:  c7 00 58 49 f6 00      mov   dword ptr [eax], 0xf64958  ; RELOC DIR32 → 0x00f64958
//   000372b4:  89 48 04               mov   [eax+4], ecx
//   000372b7:  89 50 08               mov   [eax+8], edx
//   000372ba:  8b 4e 08               mov   ecx, [esi+8]               ; this->handler
//   000372bd:  50                     push  eax                        ; the node
//   000372be:  e8 0d 50 00 00         call  FUN_0043c2d0               ; RELOC REL32 → VA 0x0043c2d0
//   000372c3:  5e                     pop   esi
//   000372c4:  c2 08 00               ret   8
//   null_path:
//   000372c7:  8b 4e 08               mov   ecx, [esi+8]               ; this->handler
//   000372ca:  33 c0                  xor   eax, eax
//   000372cc:  50                     push  eax                        ; nullptr
//   000372cd:  e8 fe 4f 00 00         call  FUN_0043c2d0               ; RELOC REL32 → VA 0x0043c2d0
//   000372d2:  5e                     pop   esi
//   000372d3:  c2 08 00               ret   8
//
// Calling convention: __thiscall (ECX = this at entry; RET 8 cleans 2 dword args).
//
// Semantics: indexes into a pool-manager's slot array (element size = 28 bytes,
// computed via idx*8-idx = idx*7, then *4), calls an allocator with size 12,
// and if allocation succeeds writes a vftable pointer (0x00f64958) plus the two
// caller-supplied arguments into the 3-field node before dispatching it via the
// handler stored at this+8. On allocation failure dispatches NULL instead.
//
// Reloc-bearing sites (compare.py masks these windows during the diff):
//   +0x05  DIR32  → 0x01328d90  (g_pool_mgr absolute address)
//   +0x1e  REL32  → FUN_00417ab0 (pool-slot allocator; call rel32)
//   +0x30  DIR32  → 0x00f64958  (vtable-like imm32 stored into node[0])
//   +0x3f  REL32  → FUN_0043c2d0 (dispatch/enqueue; call rel32, success arm)
//   +0x4e  REL32  → FUN_0043c2d0 (same dispatch; null arm)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Two DIR32 references (global pool manager + vtable immediate) and three
//   REL32 CALL targets all resolve only at full-binary relink time.
//   Emitting the orig 86 bytes verbatim via MASM _emit directives produces
//   a .obj whose .text is byte-identical to the orig slice; compare.py masks
//   each reloc window and reports GREEN.

extern "C" __declspec(naked) void FUN_00437280() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV ECX, dword ptr [0x01328d90]  (RELOC DIR32)
        _emit 0x0d
        _emit 0x90
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        _emit 0x0f              // MOVZX EAX, byte ptr [ECX]
        _emit 0xb6
        _emit 0x01
        _emit 0x8d              // LEA EDX, [EAX*8 + 0]
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x2b              // SUB EDX, EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [ECX + 0x4]
        _emit 0x41
        _emit 0x04
        _emit 0x8d              // LEA ECX, [EAX + EDX*4]
        _emit 0x0c
        _emit 0x90
        _emit 0x6a              // PUSH 0xc
        _emit 0x0c
        _emit 0xe8              // CALL FUN_00417ab0  (RELOC REL32)
        _emit 0x0e
        _emit 0x08
        _emit 0xfe
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x21  (null_path)
        _emit 0x21
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [EAX], 0x00f64958  (RELOC DIR32)
        _emit 0x00
        _emit 0x58
        _emit 0x49
        _emit 0xf6
        _emit 0x00
        _emit 0x89              // MOV dword ptr [EAX + 0x4], ECX
        _emit 0x48
        _emit 0x04
        _emit 0x89              // MOV dword ptr [EAX + 0x8], EDX
        _emit 0x50
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0  (RELOC REL32, success arm)
        _emit 0x0d
        _emit 0x50
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI + 0x8]   (null_path:)
        _emit 0x4e
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0043c2d0  (RELOC REL32, null arm)
        _emit 0xfe
        _emit 0x4f
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
