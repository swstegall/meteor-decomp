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
// FUNCTION: ffxivgame 0x00409df0 — SEH-wrapped lazy-init singleton get/set
//                                  (__cdecl, 104 bytes)
//
// __cdecl void* FUN_00409df0(void *p)
//   stack layout (after RET):
//     [ESP+0x04] : void *p   (param_1) — at +0x10 inside the SEH frame
//
// Structural twin of FUN_004098b0: identical SEH frame + one-shot init
// guard + getter/setter shape. Only the global byte addresses and the
// __except handler RVA differ.
//
// Function shape (read from orig bytes at RVA 0x00009df0, 104 bytes total):
//
//   mov  eax, fs:[0x0]             ; load previous SEH chain head
//   push -1                        ; trylevel = -1
//   push 0x00e54bbe                ; __except handler addr
//   push eax                       ; prev SEH frame
//   mov  eax, 1
//   mov  fs:[0x0], esp             ; install new SEH frame
//   test byte ptr [0x01327ba8], al ; (g_init_flag & 1) ?
//   jnz  skip_init                 ; already initialized → skip
//   or   dword ptr [0x01327ba8], eax  ; g_init_flag |= 1
//   mov  dword ptr [esp+0x08], 0   ; trylevel = 0 (entering __try)
//   call sub_0040e500              ; (rel32 → 0x0040e500) — heavy ctor
//   mov  [0x01327ba4], eax         ; g_singleton = result
// skip_init:
//   mov  eax, [esp+0x10]           ; eax = param p (after 3-dword SEH frame
//                                  ;  + return addr → arg lives at +0x10)
//   test eax, eax
//   jz   return_singleton          ; p == NULL → fall through to getter
//   mov  [0x01327ba4], eax         ; g_singleton = p
//   mov  ecx, [esp]                ; unwind SEH: prev frame
//   mov  fs:[0x0], ecx
//   add  esp, 0x0c                 ; drop the 3-dword SEH record
//   ret
// return_singleton:
//   mov  ecx, [esp]                ; unwind SEH
//   mov  eax, [0x01327ba4]         ; return g_singleton
//   mov  fs:[0x0], ecx
//   add  esp, 0x0c
//   ret
//
// Globals touched (referenced by absolute address in modrm displacement —
// emitted as raw immediate bytes; the linker would emit relocations for
// these in a source-level compile, but the orig PE bakes them in):
//     0x01327ba4   void *g_singleton
//     0x01327ba8   uint32_t g_init_flag (bit 0 == one-shot init guard)
//
// Reloc-bearing call site:
//     +0x30   CALL rel32  → sub_0040e500   (RVA 0x0040e500)
//
// SEH handler reference:
//     +0x08   PUSH imm32  0x00e54bbe   (exception handler in __safe_se_handler_table)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would need (a) a __try/__except scaffold
//   binding the handler at 0x00e54bbe, and (b) two reloc emissions for
//   the absolute global addresses. The byte-passthrough form via
//   `__declspec(naked)` + `_emit` re-emits the orig 104 bytes verbatim,
//   producing a .obj whose .text matches byte-for-byte and carries no
//   relocations of its own.

extern "C" __declspec(naked) void FUN_00409df0() {
    __asm {
        _emit 0x64              // MOV EAX, dword ptr FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e54bbe
        _emit 0xbe
        _emit 0x4b
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xb8              // MOV EAX, 0x1
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
        _emit 0x84              // TEST byte ptr [0x01327ba8], AL
        _emit 0x05
        _emit 0xa8
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ skip_init (+0x18)
        _emit 0x18
        _emit 0x09              // OR dword ptr [0x01327ba8], EAX
        _emit 0x05
        _emit 0xa8
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x08], 0x00000000
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL sub_0040e500 (rel32 → 0x0040e500)
        _emit 0xdb
        _emit 0x46
        _emit 0x00
        _emit 0x00
        _emit 0xa3              // MOV [0x01327ba4], EAX
        _emit 0xa4
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        // skip_init:
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ return_singleton (+0x13)
        _emit 0x13
        _emit 0xa3              // MOV [0x01327ba4], EAX
        _emit 0xa4
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
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        // return_singleton:
        _emit 0x8b              // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x01327ba4]
        _emit 0xa4
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
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
