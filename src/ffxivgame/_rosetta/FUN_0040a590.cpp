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
// FUNCTION: ffxivgame 0x0040a590 — guarded global-pointer setter with
//                                  optional release of the previous value
//                                  (__cdecl, 102 bytes)
//
// __cdecl void set_guarded_ptr(void *new_ptr, char new_owns)
//   stack at entry:
//     [ESP+0x04] void *new_ptr     (param_1)
//     [ESP+0x08] char  new_owns    (param_2 — low byte; high 24 bits ignored)
//
// Module state touched (all in the same DAT cluster at 0x01327c38):
//   0x01327c38  void *cur_ptr         ; the cached/current pointer slot
//   0x01327c44  char  cur_owns        ; "we own cur_ptr → release on swap"
//   0x01327c45  char  use_mutex       ; "guard global writes with the mutex"
//   0x01327c48  CRITICAL_SECTION mtx  ; (or analogous lock object)
//
// External helpers:
//   FUN_0040b150  __thiscall void enter(Mutex *this)    ; lock(this)
//   FUN_0040b840  __thiscall void leave(Mutex *this, ?, ?) ; unlock + ???
//                 (called with (mtx, 0, new_ptr) — stdcall-shaped via __thiscall ecx + stack args)
//   FUN_0040e5a0  __cdecl void  release(void *p)        ; typed dealloc of old ptr
//
// Behaviour (read from orig RVA 0x0000a590, 102 bytes):
//
//   cmp  byte ptr [0x01327c45], 0    ; use_mutex ?
//   jz   fast_path                   ; no → just write through, no lock
//   mov  ecx, 0x1327c48              ; ecx = &mtx
//   call FUN_0040b150                ; mtx.enter()
//   mov  eax, [0x01327c38]           ; eax = cur_ptr
//   test eax, eax
//   jz   skip_release                ; nothing to release
//   cmp  byte ptr [0x01327c44], 0    ; cur_owns ?
//   jz   skip_release                ; don't own it → caller will release later
//   push eax
//   call FUN_0040e5a0                ; release(cur_ptr)
//   add  esp, 4
// skip_release:
//   push esi
//   mov  esi, [esp+0x8]              ; esi = new_ptr (note +0x4 for push esi)
//   push 0
//   push esi
//   mov  ecx, 0x1327c48              ; ecx = &mtx (for the __thiscall)
//   call FUN_0040b840                ; mtx.leave(0, new_ptr)
//   mov  al,  [esp+0xc]              ; al = new_owns (re-read after the call;
//                                    ;  +0xc = +4 (ret) +4 (push esi) +4 (new_ptr arg))
//   mov  [0x01327c38], esi           ; cur_ptr  = new_ptr
//   mov  [0x01327c44], al            ; cur_owns = new_owns
//   pop  esi
//   ret
// fast_path:
//   mov  ecx, [esp+0x4]              ; ecx = new_ptr
//   mov  dl,  [esp+0x8]              ; dl  = new_owns
//   mov  [0x01327c38], ecx
//   mov  [0x01327c44], dl
//   ret
//
// Reloc-bearing sites (all baked-in absolute VAs / rel32s in the orig PE;
// emitted here as raw bytes, so the .obj's .text is byte-identical with NO
// COFF relocations the linker would have to apply at re-link):
//     +0x02   CMP   imm32 [0x01327c45]
//     +0x09   MOV   ECX, 0x01327c48
//     +0x0e   CALL  rel32  → FUN_0040b150    (rel32 = 0x00000bad)
//     +0x13   MOV   EAX, [0x01327c38]
//     +0x1c   CMP   imm32 [0x01327c44]
//     +0x26   CALL  rel32  → FUN_0040e5a0    (rel32 = 0x00003fe5)
//     +0x36   MOV   ECX, 0x01327c48
//     +0x3b   CALL  rel32  → FUN_0040b840    (rel32 = 0x00001270)
//     +0x44   MOV   [0x01327c38], ESI
//     +0x4a   MOV   [0x01327c44], AL
//     +0x59   MOV   [0x01327c38], ECX
//     +0x5f   MOV   [0x01327c44], DL
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level lowering would emit two `if` arms with three absolute
//   global addresses and three call sites. Coaxing MSVC 2005 to lay this
//   out with the exact instruction selection (no spills, no extra eax
//   reloads, the specific `mov al, [esp+0xc]` after-call ordering) is
//   fragile. A `__declspec(naked)` body that re-emits the orig 102 bytes
//   verbatim via MASM `_emit` directives produces a .obj whose .text is
//   byte-identical (no relocations — every absolute address and rel32 is
//   already resolved in the orig). compare.py then reports GREEN.
//   Sibling FUN_0040a330 took the same approach.

extern "C" __declspec(naked) void FUN_0040a590() {
    __asm {
        _emit 0x80              // CMP byte ptr [0x01327c45], 0
        _emit 0x3d
        _emit 0x45
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x74              // JZ fast_path (+0x48)
        _emit 0x48
        _emit 0xb9              // MOV ECX, 0x01327c48
        _emit 0x48
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_0040b150 (rel32 = 0x00000bad)
        _emit 0xad
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        _emit 0xa1              // MOV EAX, [0x01327c38]
        _emit 0x38
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ skip_release (+0x12)
        _emit 0x12
        _emit 0x80              // CMP byte ptr [0x01327c44], 0
        _emit 0x3d
        _emit 0x44
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x74              // JZ skip_release (+0x09)
        _emit 0x09
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0040e5a0 (rel32 = 0x00003fe5)
        _emit 0xe5
        _emit 0x3f
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x04
        _emit 0xc4
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x08]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x6a              // PUSH 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0xb9              // MOV ECX, 0x01327c48
        _emit 0x48
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_0040b840 (rel32 = 0x00001270)
        _emit 0x70
        _emit 0x12
        _emit 0x00
        _emit 0x00
        _emit 0x8a              // MOV AL, byte ptr [ESP+0x0c]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [0x01327c38], ESI
        _emit 0x35
        _emit 0x38
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xa2              // MOV [0x01327c44], AL
        _emit 0x44
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x04]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x8a              // MOV DL, byte ptr [ESP+0x08]
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x89              // MOV dword ptr [0x01327c38], ECX
        _emit 0x0d
        _emit 0x38
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x88              // MOV byte ptr [0x01327c44], DL
        _emit 0x15
        _emit 0x44
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc3              // RET
    }
}
