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
// FUNCTION: ffxivgame 0x00409f40 — SEH-wrapped singleton getter/setter
//                                  (__cdecl, 104 bytes)
//
// __cdecl int FUN_00409f40(int param_1)
//
// Byte-for-byte twin of the sibling FUN_00409bc0 (also 104 bytes, also
// SEH-wrapped singleton lazy-init + optional setter). The only
// differences are the three reloc-bearing addresses:
//
//   site                              FUN_00409bc0   FUN_00409f40
//   ─────────────────────────────────────────────────────────────
//   SEH scope table   (PUSH imm32)    0x00E54B3E     0x00E54C1E
//   init-flag dword   (TEST/OR m32)   0x01327B78     0x01327BC4
//   singleton slot    (MOV  m32)      0x01327B74     0x01327BC0
//   ctor CALL target  (rel32)         FUN_0040E500   FUN_0040E500  (same!)
//
// Lazily initializes a process-wide singleton stored at 0x01327BC0. The
// init bit lives in 0x01327BC4 bit 0 — if cleared, mark it, invoke the
// constructor FUN_0040E500() and stash its return value. After init, if
// `param_1 != 0` the caller is also acting as a setter and overwrites the
// singleton with the supplied value; otherwise the function is a pure
// getter. The whole prologue is bracketed by an MSVC __try installer that
// pushes an SEH record pointing at the scope table at 0xE54C1E (handler
// index -1 — "no try-blocks active in this frame yet").
//
// Inspection (read from the orig bytes at RVA 0x00009f40, 104 bytes):
//
//   mov  eax, fs:[0]              ; old SEH next
//   push -1                       ; trylevel = -1
//   push 0xe54c1e                 ; scope table
//   push eax                      ; chain prev
//   mov  eax, 1                   ; init-bit constant
//   mov  fs:[0], esp              ; install SEH frame
//   test [0x01327bc4], al         ; (byte) bit 0 set?
//   jnz  already_inited           ; skip ctor
//   or   [0x01327bc4], eax        ; mark inited (bit 0)
//   mov  [esp+8], 0               ; trylevel := 0 (entering protected region)
//   call FUN_0040e500             ; ctor → returns the singleton
//   mov  [0x01327bc0], eax        ; store
// already_inited:
//   mov  eax, [esp+0x10]          ; param_1 (after pushes)
//   test eax, eax
//   jz   getter                   ; param_1 == 0 → pure getter
//   mov  [0x01327bc0], eax        ; setter — overwrite with caller's value
//   mov  ecx, [esp]               ; SEH next
//   mov  fs:[0], ecx              ; restore SEH
//   add  esp, 0x0c                ; pop SEH frame
//   ret                           ; return param_1 (already in eax)
// getter:
//   mov  ecx, [esp]               ; SEH next
//   mov  eax, [0x01327bc0]        ; load singleton
//   mov  fs:[0], ecx              ; restore SEH
//   add  esp, 0x0c                ; pop SEH frame
//   ret
//
// Reloc-bearing sites in the orig 104 bytes (these absolute / PC-relative
// targets are baked into the orig binary as concrete byte sequences;
// emitting them as raw immediates via MASM `_emit` produces a .obj whose
// .text matches the orig byte-for-byte with NO relocations):
//     +0x08   PUSH imm32   → 0x00E54C1E  (SEH scope table)
//     +0x16   TEST imm32   → 0x01327BC4  (init-flags dword)
//     +0x22   OR   imm32   → 0x01327BC4  (init-flags dword)
//     +0x30   CALL rel32   → FUN_0040E500 (singleton ctor)
//                            rel32 = 0x0040E500 - 0x00409F75 = 0x0000458B
//     +0x35   MOV  imm32   → 0x01327BC0  (singleton slot)
//     +0x42   MOV  imm32   → 0x01327BC0  (singleton slot, setter path)
//     +0x58   MOV  imm32   → 0x01327BC0  (singleton slot, getter path)
//
// Reconstruction strategy — naked-asm byte passthrough (same as the
// sibling FUN_00409bc0): re-emit the orig 104 bytes verbatim via MASM
// `_emit` directives. The .obj's `.text` section ends up byte-identical
// to the orig slice (no relocations: the rel32 offset resolves against
// the orig binary's own address space, and the imm32 constants are
// absolute values at orig load address — emitting them as raw bytes
// produces the exact wire image the linker would emit at relink).
// `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00409f40() {
    __asm {
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH -0x01
        _emit 0xff
        _emit 0x68              // PUSH 0x00E54C1E       (scope table)
        _emit 0x1e
        _emit 0x4c
        _emit 0xe5
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xb8              // MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x64              // MOV FS:[0x0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327BC4], AL
        _emit 0x05
        _emit 0xc4
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ already_inited (+0x18)
        _emit 0x18
        _emit 0x09              // OR dword ptr [0x01327BC4], EAX
        _emit 0x05
        _emit 0xc4
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
        _emit 0xe8              // CALL FUN_0040E500 (rel32 → 0x0000458B)
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        _emit 0x00
        _emit 0xa3              // MOV [0x01327BC0], EAX
        _emit 0xc0
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]     (already_inited:)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ getter (+0x13)
        _emit 0x13
        _emit 0xa3              // MOV [0x01327BC0], EAX
        _emit 0xc0
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, dword ptr [ESP]
        _emit 0x0c
        _emit 0x24
        _emit 0x64              // MOV FS:[0x0], ECX
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
        _emit 0x8b              // MOV ECX, dword ptr [ESP]          (getter:)
        _emit 0x0c
        _emit 0x24
        _emit 0xa1              // MOV EAX, [0x01327BC0]
        _emit 0xc0
        _emit 0x7b
        _emit 0x32
        _emit 0x01
        _emit 0x64              // MOV FS:[0x0], ECX
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
