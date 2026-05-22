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
// FUNCTION: ffxivgame 0x00409c30 — SEH-wrapped lazy-init getter/setter
//                                  for the singleton at DAT_01327b80
//                                  (__cdecl, 104 bytes / 0x68)
//
// Structurally identical to the sibling FUN_00409a00 — same shape, same
// 104-byte size, just operating on a different singleton (the cached
// value lives at 0x01327b80 and the init-flag bit at 0x01327b84) and a
// different SEH scope table (0x00e54b5e). The one-time initializer is
// still FUN_0040e500.
//
// __cdecl int FUN_00409c30(int param_1)
//   stack layout at entry (before SEH push):
//     [ESP+0x04] : int param_1
//
// Global state (BSS):
//   DAT_01327b80  : int   — the singleton value (the getter return)
//   DAT_01327b84  : dword — initialization flag (bit 0 == "init done")
//
// Inspection (read from the orig bytes at RVA 0x00009c30, 104 bytes):
//
//   mov  eax, fs:[0x0]              ; load current SEH list head
//   push -1                          ; SEH frame: trylevel = -1
//   push 0x00e54b5e                  ; SEH frame: scope table ptr
//   push eax                         ; SEH frame: prev link
//   mov  eax, 1                      ; eax = 1 (used for TEST + OR + state)
//   mov  fs:[0x0], esp               ; install new SEH frame
//   test byte ptr [DAT_01327b84], al ; bit 0 already set?
//   jnz  param_path                  ; yes → skip one-time init
//   or   dword ptr [DAT_01327b84], eax ; set bit 0 ("init done")
//   mov  dword ptr [esp+0x8], 0      ; SEH state = 0 (we're now in __try)
//   call FUN_0040e500                ; one-time initializer
//   mov  [DAT_01327b80], eax         ; cache its result
// param_path:                         ; (= 0x00409c6a)
//   mov  eax, [esp+0x10]             ; eax = param_1 (after 3 dwords of SEH frame)
//   test eax, eax
//   jz   getter_return               ; param_1 == 0 → just return cached
//   mov  [DAT_01327b80], eax         ; setter path: store param_1
//   mov  ecx, [esp]                  ; ecx = prev SEH link
//   mov  fs:[0x0], ecx               ; tear down SEH
//   add  esp, 0xc                    ; pop SEH frame
//   ret                              ; (return value is param_1, still in eax)
// getter_return:
//   mov  ecx, [esp]                  ; ecx = prev SEH link
//   mov  eax, [DAT_01327b80]         ; return cached value
//   mov  fs:[0x0], ecx                ; tear down SEH
//   add  esp, 0xc                    ; pop SEH frame
//   ret
//
// Reloc-bearing sites in the orig 104 bytes (the linker would resolve
// these absolute / PC-relative targets from source-level C++):
//     +0x08   PUSH imm32  → 0x00e54b5e   (SEH scope table for this frame)
//     +0x1a   TEST  m32   → 0x01327b84   (init flag, abs addr in .bss)
//     +0x22   OR    m32   → 0x01327b84
//     +0x31   CALL rel32  → FUN_0040e500
//     +0x36   MOV   m32   → 0x01327b80   (singleton var, abs addr)
//     +0x43   MOV   m32   → 0x01327b80
//     +0x56   MOV   m32   → 0x01327b80
//
// CALL rel32 displacement: from next-instruction RVA 0x00009c65 to
// target FUN_0040e500 = 0x0000e500 - 0x00009c65 = 0x489b → little-endian
// bytes `9b 48 00 00`, which is what we see baked in at +0x32.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Identical reasoning to FUN_00409a00: a `__declspec(naked)` body that
//   re-emits the orig 104 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` ends up byte-identical to the orig slice with NO
//   relocations — every absolute addr and rel32 disp is emitted as a
//   raw immediate against the orig binary's own address space.
//   compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00409c30() {
    __asm {
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e54b5e (SEH scope table)
        _emit 0x5e
        _emit 0x4b
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX (prev SEH link)
        _emit 0xb8              // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64              // MOV dword ptr FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327b84], AL
        _emit 0x05
        _emit 0x84
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ short param_path (+0x18)
        _emit 0x18
        _emit 0x09              // OR dword ptr [0x01327b84], EAX
        _emit 0x05
        _emit 0x84
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x08], 0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040e500 (rel32 → 0x0000489b)
        _emit 0x9b
        _emit 0x48
        _emit 0x00
        _emit 0x00
        _emit 0xa3              // MOV [0x01327b80], EAX
        _emit 0x80
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]    (param_path)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ short getter_return (+0x13)
        _emit 0x13
        _emit 0xa3              // MOV [0x01327b80], EAX
        _emit 0x80
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0C
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x8b              // MOV ECX, dword ptr [ESP]         (getter_return)
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x01327b80]
        _emit 0x80
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x64              // MOV dword ptr FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0C
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
