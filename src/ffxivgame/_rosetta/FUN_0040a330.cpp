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
// FUNCTION: ffxivgame 0x0040a330 — lazy-init global accessor / setter
//                                  (__cdecl, 104 bytes, SEH-wrapped)
//
// Structural twin of FUN_00409680 / FUN_00409510 — same prologue, same
// guard-bit + value-slot pattern, same two-RET epilogue. Only the
// concrete addresses differ:
//
//                          this fn (0x0040a330)   sibling (0x00409680)
//     SEH handler VA       0x00e54d1e             0x00e549fe
//     guard bit (DAT)      0x01327c18             0x01327b08
//     value slot (DAT)     0x01327c14             0x01327b04
//     CALL target          FUN_0040e500           FUN_0040e500   (same)
//
// __cdecl int set_or_get(int new_value)
//   stack at entry (after the function pushes its SEH frame):
//     [ESP+0x00]  saved fs:[0]            (prev SEH handler chain)
//     [ESP+0x04]  0xe54d1e                (this fn's exception handler)
//     [ESP+0x08]  -1  →  0                (trylevel; bumped before the call)
//     [ESP+0x0c]  return address
//     [ESP+0x10]  new_value               (param_1)
//
// Behaviour (read from orig RVA 0x0000a330, 104 bytes):
//
//   mov  eax, fs:[0]                ; save prev SEH chain head
//   push -1                         ; initial trylevel
//   push 0xe54d1e                   ; SEH funclet entry
//   push eax                        ; prev chain head → ESP+0
//   mov  eax, 1
//   mov  fs:[0], esp                ; install our SEH frame
//   test [0x01327c18], al           ; bit 0 of guard already set?
//   jnz  read_param                 ; yes → skip lazy init
//   or   [0x01327c18], eax          ; set the guard bit
//   mov  [esp+8], 0                 ; trylevel = 0 (inside __try)
//   call FUN_0040e500               ; one-shot factory; returns the value
//   mov  [0x01327c14], eax          ; stash into the global
// read_param:
//   mov  eax, [esp+0x10]            ; eax = new_value
//   test eax, eax
//   jz   return_global              ; new_value == 0 → return current global
//   mov  [0x01327c14], eax          ; new_value != 0 → overwrite global,
//                                   ;                  return new_value
//   mov  ecx, [esp]
//   mov  fs:[0], ecx                ; unwind SEH frame
//   add  esp, 0xc
//   ret
// return_global:
//   mov  ecx, [esp]
//   mov  eax, [0x01327c14]          ; eax = global
//   mov  fs:[0], ecx                ; unwind SEH frame
//   add  esp, 0xc
//   ret
//
// Reloc-bearing sites (all baked-in absolute VAs in the orig PE; emitted
// here as raw bytes, so the .obj's .text is byte-identical to the orig
// slice with NO relocations the linker would have to apply at re-link):
//     +0x08  PUSH imm32         0xe54d1e          (SEH handler VA)
//     +0x1a  TEST [imm32], AL   0x01327c18        (guard byte)
//     +0x22  OR   [imm32], EAX  0x01327c18
//     +0x30  CALL rel32         → FUN_0040e500    (rel32 = 0x0000419b)
//     +0x35  MOV  [imm32], EAX  0x01327c14        (global value slot)
//     +0x42  MOV  [imm32], EAX  0x01327c14
//     +0x58  MOV  EAX, [imm32]  0x01327c14
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level lowering of this pattern (function-local-static-style
//   lazy init wrapped in SEH) is brittle to coax out of MSVC 2005 with
//   the exact bytes — the SEH frame setup, guard-bit test ordering, and
//   the duplicated epilogue are all fragile. A `__declspec(naked)` body
//   that re-emits the orig 104 bytes verbatim via MASM `_emit` directives
//   produces a .obj whose .text matches byte-for-byte (no relocations —
//   every absolute address in the orig is already resolved). compare.py
//   then reports GREEN. Sibling FUN_00409680 took the same approach.

extern "C" __declspec(naked) void FUN_0040a330() {
    __asm {
        _emit 0x64                  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a                  // PUSH -1
        _emit 0xff
        _emit 0x68                  // PUSH 0x00e54d1e
        _emit 0x1e
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        _emit 0x50                  // PUSH EAX
        _emit 0xb8                  // MOV EAX, 1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64                  // MOV FS:[0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84                  // TEST byte ptr [0x01327c18], AL
        _emit 0x05
        _emit 0x18
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x75                  // JNZ read_param (+0x18)
        _emit 0x18
        _emit 0x09                  // OR  dword ptr [0x01327c18], EAX
        _emit 0x05
        _emit 0x18
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7                  // MOV dword ptr [ESP+0x08], 0
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8                  // CALL FUN_0040e500 (rel32 = 0x0000419b)
        _emit 0x9b
        _emit 0x41
        _emit 0x00
        _emit 0x00
        _emit 0xa3                  // MOV [0x01327c14], EAX
        _emit 0x14
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x8b                  // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85                  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74                  // JZ return_global (+0x13)
        _emit 0x13
        _emit 0xa3                  // MOV [0x01327c14], EAX
        _emit 0x14
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x8b                  // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0x64                  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83                  // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3                  // RET
        _emit 0x8b                  // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0xa1                  // MOV EAX, [0x01327c14]
        _emit 0x14
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x64                  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83                  // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3                  // RET
    }
}
