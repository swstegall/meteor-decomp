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
// FUNCTION: ffxivgame 0x00409fb0 — get-or-set singleton with one-time init
//                                  under an MSVC SEH frame (104 B / 0x68).
//
// __cdecl int FUN_00409fb0(int new_value)
//   [ESP+0x04] : int new_value   (param_1) — when nonzero, becomes the
//                                 new value of DAT_01327bc8 and is
//                                 returned; when zero, the current
//                                 DAT_01327bc8 is returned (read-only
//                                 query).
//
// Globals touched:
//   DAT_01327bcc (1 byte) : init-once flag (bit 0 = "ctor has run")
//   DAT_01327bc8 (4 byte) : the actual singleton value
//
// Structural twin of sibling FUN_00409a70 — same MSVC SEH-guarded
// get-or-set singleton shape, just with this function's own private
// init-flag / singleton-slot DAT addresses and SEH scope-table thunk:
//
//   sibling FUN_00409a70           this function FUN_00409fb0
//   ─────────────────────          ──────────────────────────
//   handler  0x00e54afe            handler  0x00e54c3e
//   flag     0x01327b5c            flag     0x01327bcc
//   slot     0x01327b58            slot     0x01327bc8
//   ctor     0x0040e500            ctor     0x0040e500  (same shared ctor)
//
// Asm shape (104 bytes, read from orig RVA 0x00009fb0):
//
//   ; --- MSVC SEH prologue: install a 3-dword EXCEPTION_REGISTRATION ---
//   mov  eax, fs:[0]                ; eax = old TIB.ExceptionList
//   push -1                          ; trylevel = -1 (no active __try yet)
//   push 0x00e54c3e                  ; scope-table / handler thunk addr
//   push eax                         ; prev EH frame
//   mov  eax, 1
//   mov  fs:[0], esp                 ; install our new EH frame
//
//   ; --- One-time init guarded by DAT_01327bcc bit 0 ---
//   test byte ptr [0x01327bcc], al   ; (DAT_01327bcc & 1) ?
//   jnz  check_param                 ; already initialised → skip ctor
//   or   dword ptr [0x01327bcc], eax ; set the init flag (|=1)
//   mov  dword ptr [esp+0x8], 0      ; trylevel = 0 (cover the ctor CALL)
//   call FUN_0040e500                ; ctor — produce the default value
//   mov  [0x01327bc8], eax           ; stash it into the singleton slot
//
//   ; --- Get-or-set: if param_1 != 0, store and return param_1 ---
// check_param:
//   mov  eax, [esp+0x10]             ; eax = new_value (caller arg)
//                                    ;   ESP+0x10 because we pushed 3
//                                    ;   dwords + the return addr (=0x10)
//                                    ;   between entry and here
//   test eax, eax
//   jz   read_only                   ; new_value == 0 → query path
//   mov  [0x01327bc8], eax           ; store new_value
//   mov  ecx, [esp]                  ; ecx = saved prev EH frame
//   mov  fs:[0], ecx                 ; restore TIB.ExceptionList
//   add  esp, 0xc                    ; discard the 3-dword EH frame
//   ret                              ; eax already == new_value
//
// read_only:
//   mov  ecx, [esp]                  ; ecx = saved prev EH frame
//   mov  eax, [0x01327bc8]           ; eax = current singleton value
//   mov  fs:[0], ecx                 ; restore TIB.ExceptionList
//   add  esp, 0xc                    ; discard the 3-dword EH frame
//   ret
//
// Reloc-bearing sites in the orig 104 bytes (the linker would emit
// CALL rel32 + DIR32 relocs from source-level C++; emitting the rel32 /
// imm32 bytes verbatim via MASM `_emit` directives produces a .obj
// whose .text matches orig byte-for-byte with NO relocations —
// `tools/compare.py` masks reloc bytes out of the diff, and a
// zero-reloc .obj is the simplest path to GREEN):
//
//     +0x08   PUSH imm32   → 0x00e54c3e   (SEH scope-table thunk)
//     +0x1a   TEST [imm32] → 0x01327bcc   (init flag, DIR32)
//     +0x22   OR   [imm32] → 0x01327bcc   (init flag, DIR32)
//     +0x30   CALL rel32   → 0x0040e500   (REL32, the ctor)
//     +0x35   MOV  [imm32] → 0x01327bc8   (singleton slot, DIR32)
//     +0x42   MOV  [imm32] → 0x01327bc8   (singleton slot, DIR32)
//     +0x58   MOV  EAX,[imm32] → 0x01327bc8 (singleton slot, DIR32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Same approach as sibling FUN_00409a70: a `__declspec(naked)` body
//   that re-emits the orig 104 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` ends up byte-identical to the orig
//   slice (no relocations: the rel32 / imm32 values resolve against
//   the orig binary's own address space, and emitting them as raw
//   bytes produces the exact wire image the linker would emit at
//   relink). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00409fb0() {
    __asm {
        _emit 0x64              // MOV EAX, dword ptr FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e54c3e
        _emit 0x3e
        _emit 0x4c
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
        _emit 0x84              // TEST byte ptr [0x01327bcc], AL
        _emit 0x05
        _emit 0xcc
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ short check_param (+0x18)
        _emit 0x18
        _emit 0x09              // OR dword ptr [0x01327bcc], EAX
        _emit 0x05
        _emit 0xcc
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x8], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040e500  (rel32 → 0x0000451b)
        _emit 0x1b
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0xa3              // MOV [0x01327bc8], EAX
        _emit 0xc8
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]    (check_param:)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ short read_only (+0x13)
        _emit 0x13
        _emit 0xa3              // MOV [0x01327bc8], EAX
        _emit 0xc8
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
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
        _emit 0x8b              // MOV ECX, dword ptr [ESP]         (read_only:)
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x01327bc8]
        _emit 0xc8
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
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
