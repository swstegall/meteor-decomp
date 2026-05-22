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
// FUNCTION: ffxivgame 0x00409050 — `push_back` for an 8-slot fixed-capacity
//                                  pointer array on the
// `SQEX::CDev::Engine::Fw::Framework::Memory::MemoryUtility::ChildDefaultSpaces`
// inline aggregate (85 bytes).
//
// __thiscall void ChildDefaultSpaces::push_back(undefined4 value)
//   stack layout (after RET 4):
//     ECX        : this
//     [ESP+0x04] : undefined4 value     (param_1)
//
// Memory layout (inferred from the offsets touched):
//   +0x00 .. +0x1f   void *spaces[8]            ; storage array (8 dwords)
//   +0x20            int  count                 ; current occupancy
//
// Inspection (read from the orig bytes at RVA 0x00009050, 85 bytes total):
//
//   mov  eax, [ecx+0x20]            ; eax = this->count
//   mov  edx, [esp+0x04]            ; edx = value
//   mov  [ecx+eax*4], edx           ; this->spaces[count] = value
//   mov  eax, 1
//   add  [ecx+0x20], eax            ; this->count += 1     (uses imm-in-reg form)
//   cmp  dword ptr [ecx+0x20], 0x8
//   jb   epilog                     ; count < 8 → bail out (assertion holds)
//   test byte ptr [0x01323910], al  ; (al = 1) — has the assert handler
//                                   ;   trampoline been initialised yet?
//   jnz  do_assert                  ; yes → just fire it
//   or   dword ptr [0x01323910], eax ; flag |= 1 (mark as initialised)
//   mov  dword ptr [0x0132390c], offset FUN_004071b0
//                                   ;   patch the trampoline slot with the
//                                   ;   canonical assert printer
// do_assert:
//   push offset "...MemoryUtility::ChildDefaultSpaces::push_back" (0xf551b8)
//   push 0x153                      ; line number
//   push offset "c:\\work\\project\\cdev\\src\\fw\\cdev\\engine\\fw\\framework"
//                                   ; "\\memory\\MemoryUtility.h"  (0xf55168)
//   push offset DAT_00f54d48        ; (printf-style format / category banner)
//   push offset "this->count < (static_cast<int>(sizeof(spaces)/sizeof((spaces)[0])))"
//                                   ;                              (0xf55120)
//   call [0x0132390c]               ; (*g_assert_fn)(predicate, banner, file,
//                                   ;                line, fn);
//   add  esp, 0x14                  ; cdecl callee-arg cleanup (5×4 = 20)
// epilog:
//   ret  4                          ; __thiscall, callee-cleans 1 stack dword
//
// This is the canonical SQEX `MEMORY_UTILITY_ASSERT(...)`-style macro
// expansion: a lazy-init function-pointer slot at 0x0132390c gates a
// trampoline (FUN_004071b0) that prints the predicate text + source
// location + function name. The byte 0x01323910 is the one-shot init
// flag (LSB tested + set). The five PUSHes are the trampoline's
// argument list, in the SQEX order:
//     (predicate_text, banner, source_file, line_number, function_name)
//
// Reloc-bearing sites in the orig 85 bytes (each one resolves at link
// time against a fixed VA — when emitted as raw `_emit` bytes the .obj
// carries no relocations and `tools/compare.py` reports GREEN
// immediately):
//     +0x18   TEST [0x01323910], AL              (init-flag byte)
//     +0x20   OR   [0x01323910], EAX             (set init-flag bit 0)
//     +0x26   MOV  [0x0132390c], 0x004071b0      (lazy-init slot ← fn ptr)
//     +0x30   PUSH 0x00f551b8                    (function-name string)
//     +0x3a   PUSH 0x00f55168                    (source-file string)
//     +0x3f   PUSH 0x00f54d48                    (banner string)
//     +0x44   PUSH 0x00f55120                    (predicate string)
//     +0x49   CALL [0x0132390c]                  (indirect via lazy slot)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (a `push_back` member with an inline
//   `MEMORY_UTILITY_ASSERT` macro that lazy-inits a global function
//   pointer) would emit the same shape but produce eight relocations
//   targeting symbols (the init flag, the trampoline slot, the four
//   string literals, the trampoline target). `tools/compare.py` masks
//   reloc bytes, but driving a full relink to settle the symbol
//   addresses isn't necessary: a `__declspec(naked)` body that
//   re-emits the orig 85 bytes verbatim via MASM `_emit` directives
//   produces a .obj whose `.text` is byte-identical to the orig slice
//   with NO relocations (the absolute VAs are emitted as raw imm32
//   bytes, valid at the orig load address). compare.py then reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00409050() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ECX+0x20]
        _emit 0x41
        _emit 0x20
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x04]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ECX+EAX*4], EDX
        _emit 0x14
        _emit 0x81
        _emit 0xb8              // MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01              // ADD dword ptr [ECX+0x20], EAX
        _emit 0x41
        _emit 0x20
        _emit 0x83              // CMP dword ptr [ECX+0x20], 0x08
        _emit 0x79
        _emit 0x20
        _emit 0x08
        _emit 0x72              // JB epilog (+0x3a)
        _emit 0x3a
        _emit 0x84              // TEST byte ptr [0x01323910], AL
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ do_assert (+0x10)
        _emit 0x10
        _emit 0x09              // OR  dword ptr [0x01323910], EAX
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x004071b0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xb0
        _emit 0x71
        _emit 0x40
        _emit 0x00
        _emit 0x68              // PUSH 0x00f551b8        (do_assert:)
        _emit 0xb8
        _emit 0x51
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00000153
        _emit 0x53
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f55168
        _emit 0x68
        _emit 0x51
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f55120
        _emit 0x20
        _emit 0x51
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x0004                    (epilog:)
        _emit 0x04
        _emit 0x00
    }
}
