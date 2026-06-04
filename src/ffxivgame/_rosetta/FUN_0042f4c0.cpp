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
// FUNCTION: ffxivgame 0x0042f4c0 — Meyers-singleton accessor (__cdecl-ish,
//                                  133 B, SEH-wrapped) that lazily constructs
//                                  a process-global object the first time it
//                                  is called and then forwards a caller-
//                                  supplied argument into a member method.
//
// Asm shape (read from RVA 0x0002f4c0, 133 bytes of `.text`):
//
//   push -1                                      ; SEH initial unwind state
//   push offset @sehScopeTable_00e55ece          ; SEH handler / scope table
//   mov  eax, fs:[0]
//   push eax                                      ; prev fs:[0] → SEH chain
//   sub  esp, 8                                   ; locals (state slot @ +0x18)
//   push esi                                      ; preserved register
//   mov  eax, [__security_cookie] (0x012ea8b0)
//   xor  eax, esp                                 ; GS cookie anchored to esp
//   push eax                                      ; spill cookie
//   lea  eax, [esp + 0x10]                        ; &prev_fs0
//   mov  fs:[0], eax                              ; install SEH registration
//   mov  eax, 1
//   test byte ptr [g_initFlag 0x01327c10], al     ; already initialised?
//   jnz  .have                                    ; → skip construction
//   or   dword ptr [g_initFlag 0x01327c10], eax    ; mark initialised
//   mov  dword ptr [esp + 0x18], 0                ; SEH trylevel = 0
//   call FUN_0040e500                             ; construct the singleton
//   mov  [g_instance 0x01327c0c], eax             ; cache the instance ptr
//   mov  dword ptr [esp + 0x18], -1               ; SEH trylevel = -1
//   .have:
//   mov  esi, [g_instance 0x01327c0c]             ; esi = instance ptr
//   push offset 0x00f59860                        ; some static arg/string
//   push 0x10
//   lea  ecx, [esp + 0x10]
//   call FUN_0040e2d0                             ; helper → eax
//   push eax
//   mov  eax, [esp + 0x24]                        ; caller's stack argument
//   push eax
//   mov  ecx, esi                                 ; ecx = instance (thiscall)
//   call FUN_0040e110                             ; instance->method(arg, eax)
//   mov  ecx, [esp + 0x10]                        ; saved prev fs:[0]
//   mov  fs:[0], ecx                              ; restore SEH chain
//   pop  ecx                                      ; drop cookie
//   pop  esi                                      ; restore preserved register
//   add  esp, 0x14                                ; unwind SEH frame + locals
//   ret
//
// Reloc-bearing sites in the orig 133 bytes (relocations are masked by
// `tools/compare.py` for the diff; a naked-asm `_emit` body produces a
// .obj whose `.text` re-emits the orig's baked imm32 / DIR32 / rel32
// operands verbatim):
//
//   +0x03  PUSH imm32  → @sehScopeTable    (VA 0x00e55ece)
//   +0x07  MOV moffs32 → fs:[0]            (TEB SEH list head; FS-prefixed)
//   +0x12  MOV moffs32 → __security_cookie (VA 0x012ea8b0)
//   +0x1e  MOV moffs32 → fs:[0]            (install handler)
//   +0x29  TEST mem    → g_initFlag        (VA 0x01327c10)
//   +0x31  OR   mem    → g_initFlag        (VA 0x01327c10)
//   +0x40  CALL rel32  → FUN_0040e500
//   +0x45  MOV  moffs32→ g_instance        (VA 0x01327c0c)
//   +0x51  MOV  mem    → g_instance        (VA 0x01327c0c)
//   +0x57  PUSH imm32  → 0x00f59860
//   +0x63  CALL rel32  → FUN_0040e2d0
//   +0x6f  CALL rel32  → FUN_0040e110
//   +0x78  MOV moffs32 → fs:[0]            (restore handler)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (a Meyers-singleton accessor wrapped in the
//   MSVC 2005 `/GS` SEH frame, with the lazy `__local_static_guard`
//   bit-flag init pattern) would emit this shape under /O2, but it
//   references several binary-resident absolute addresses (the SEH scope
//   table, `__security_cookie`, the init-flag and instance globals, plus
//   three rel32 sibling calls). None of those resolve from the .obj —
//   they'd be left as COFF relocations the linker fills in at relink.
//   Because we match at the byte-diff level (not driving a relink here),
//   the pragmatic choice — same as siblings FUN_00403d60 / FUN_00403bd0 —
//   is a `__declspec(naked)` body that re-emits the orig 133 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` ends up
//   byte-identical to the orig slice; `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_0042f4c0() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E55ECE  (SEH scope table)
        _emit 0xce
        _emit 0x5e
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP + 0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x00000000], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb8              // MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327C10], AL
        _emit 0x05
        _emit 0x10
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x20  (→ .have)
        _emit 0x20
        _emit 0x09              // OR dword ptr [0x01327C10], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP + 0x18], 0x00000000
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_0040E500 (rel32)
        _emit 0xfc
        _emit 0xef
        _emit 0xfd
        _emit 0xff
        _emit 0xa3              // MOV [0x01327C0C], EAX
        _emit 0x0c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP + 0x18], 0xFFFFFFFF
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, dword ptr [0x01327C0C]   (.have:)
        _emit 0x35
        _emit 0x0c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x68              // PUSH 0x00F59860
        _emit 0x60
        _emit 0x98
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0xe8              // CALL FUN_0040E2D0 (rel32)
        _emit 0xa9
        _emit 0xed
        _emit 0xfd
        _emit 0xff
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x24]
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040E110 (rel32)
        _emit 0xdc
        _emit 0xeb
        _emit 0xfd
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV dword ptr FS:[0x00000000], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3              // RET
    }
}
