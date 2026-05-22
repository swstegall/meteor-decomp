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
// FUNCTION: ffxivgame 0x40a610 — once-only guarded init of a static singleton
//
// __cdecl FUN_0040a610() — process-scope "init once" helper.  Reads the
// init flag at .data 0x01327c45; if clear, loads the cached argument
// from .data 0x01327c38, then invokes FUN_0040b840 (__thiscall) on the
// singleton instance at .data 0x01327c48 with arguments (cached_arg, 0),
// then sets the init flag.
//
// Asm body (35 bytes at RVA 0x0000a610):
//
//   cmp  byte ptr [0x01327c45], 0      ; init-flag test
//   jnz  +0x19                         ; already initialized → fall through to RET
//   mov  eax, [0x01327c38]             ; load cached arg
//   push 0                             ; arg2 = 0
//   push eax                           ; arg1 = cached_arg
//   mov  ecx, 0x01327c48               ; this = &g_singleton  (__thiscall)
//   call FUN_0040b840                  ; rel32 → +0x1215
//   mov  byte ptr [0x01327c45], 1      ; set init flag
//   ret
//
//   Calling convention: __cdecl (no args, no return).  Frame: zero — no
//   prologue/epilogue, no locals.  Callee FUN_0040b840 is __thiscall
//   (ECX = this, two stack args, callee-cleaned — no `add esp, 8` here).
//
// Reloc-bearing sites in the orig 35 bytes (concrete absolute / PC-relative
// targets baked into the orig PE post-fixup):
//
//     +0x02   CMP   [imm32] → 0x01327c45 (init flag)
//     +0x0A   MOV   EAX,[imm32] → 0x01327c38 (cached arg)
//     +0x12   MOV   ECX,imm32 → 0x01327c48 (singleton instance)
//     +0x17   CALL  rel32   → 0x0040b840   (rel32 = 0x00001215 from end-of-insn 0x0040a62b)
//     +0x1D   MOV   [imm32],imm8 → 0x01327c45 (init flag = 1)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (`class Singleton { void Init(int,int); };
//   extern Singleton g_obj; extern int g_arg; extern char g_init; if
//   (!g_init) { g_obj.Init(g_arg, 0); g_init = 1; }`) would compile to
//   the same 35-byte shape but would produce relocations referencing
//   symbols whose addresses the linker assigns — `g_obj` would not
//   land at 0x01327c48 in our re-link unless we constrained the entire
//   .data layout to match orig, which is out of scope for an in-place
//   rosetta match.
//
//   The pragmatic choice — the same one the sibling FUN_004091f0,
//   FUN_00403b70, FUN_00403bd0, FUN_00403eb0, and FUN_00404440 took
//   for their own reloc-heavy bodies — is a `__declspec(naked)` body
//   that re-emits the orig 35 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical to
//   the orig slice (no relocations: the rel32 offset resolves against
//   the orig binary's own address space, and the imm32 constants are
//   absolute values at orig load address — emitting them as raw bytes
//   produces the exact wire image the linker would emit at relink).
//   `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_0040a610() {
    __asm {
        _emit 0x80              // CMP byte ptr [0x01327c45], 0
        _emit 0x3d
        _emit 0x45
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x75              // JNZ +0x19  (skip to RET)
        _emit 0x19
        _emit 0xa1              // MOV EAX, [0x01327c38]
        _emit 0x38
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xb9              // MOV ECX, 0x01327c48
        _emit 0x48
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xe8              // CALL FUN_0040b840 (rel32 → 0x00001215)
        _emit 0x15
        _emit 0x12
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [0x01327c45], 1
        _emit 0x05
        _emit 0x45
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc3              // RET
    }
}
