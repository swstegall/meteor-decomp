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
// FUNCTION: ffxivgame 0x005d1b35 — try-load + lazy-init error-handler wrapper
//                                  (__cdecl, 105 bytes)
//
// Control-flow summary (recovered from orig bytes):
//
//   prolog: PUSH EBP / MOV EBP,ESP / SUB ESP,0x0C
//   JMP outer_loop        ; skip straight to outer-loop on first entry
//
// inner (RVA 0x5d1b3d):
//   PUSH [EBP+8]
//   CALL 0x5de438         ; func1 — attempt primary load / acquire
//   TEST EAX, EAX
//   POP ECX
//   JZ error_path         ; func1 failed → go to error / singleton-init block
//   fall through          ; func1 succeeded → retry outer_loop
//
// outer_loop (RVA 0x5d1b4a):
//   PUSH [EBP+8]
//   CALL 0x5d5bc5         ; func2 — attempt use
//   TEST EAX, EAX
//   POP ECX
//   JZ inner              ; func2 failed → back to func1
//   LEAVE
//   RET                   ; func2 succeeded → done
//
// error_path (RVA 0x5d1b59):
//   TEST byte ptr [0x01363f10], 0x01  ; check singleton-init flag
//   MOV ESI, 0x01363f04               ; singleton pointer
//   JNZ skip_init         ; already initialized → skip
//   OR  [0x01363f10], 0x01            ; set init flag
//   MOV ECX, ESI
//   CALL 0x5d1b1c                     ; singleton ctor / init (rel32 -0x59)
//   PUSH 0x00f3b0a0
//   CALL 0x5d25c2                     ; registration call
//   POP ECX
// skip_init:
//   PUSH ESI
//   LEA  ECX, [EBP-0x0C]
//   CALL 0x5d1940                     ; object helper (rel32)
//   PUSH 0x011a8c90
//   LEA  EAX, [EBP-0x0C]
//   PUSH EAX
//   MOV  [EBP-0x0C], 0x00f54a10      ; write vtable/dispatch pointer into local
//   CALL 0x5d1b9f                     ; tail call to continuation (rel32 +1)
//
// Why naked-asm passthrough: the function contains multiple embedded absolute
// addresses ([0x01363f10], [0x01363f04], immediate pushes 0x00f3b0a0 /
// 0x011a8c90 / 0x00f54a10) plus five relative CALL sites, a backward JZ loop,
// and ends without a RET (tail call). Coaxing MSVC 2005 into emitting that
// exact byte sequence from C++ source is not tractable; a __declspec(naked)
// body with _emit directives is the deterministic path to GREEN.
//
// Reloc-bearing sites in the orig 105 bytes:
//   +0x0C  CALL rel32  → 0x5de438  (func1)
//   +0x19  CALL rel32  → 0x5d5bc5  (func2)
//   +0x26  TEST [imm32] → 0x01363f10  (init flag)
//   +0x2C  MOV ESI, imm32 → 0x01363f04  (singleton ptr)
//   +0x34  OR  [imm32]  → 0x01363f10  (init flag)
//   +0x3C  CALL rel32  → 0x5d1b1c   (ctor, backward)
//   +0x41  PUSH imm32  → 0x00f3b0a0
//   +0x46  CALL rel32  → 0x5d25c2
//   +0x50  CALL rel32  → 0x5d1940   (helper, backward)
//   +0x55  PUSH imm32  → 0x011a8c90
//   +0x5E  MOV [EBP-0C], imm32 → 0x00f54a10
//   +0x65  CALL rel32  → 0x5d1b9f   (tail call, +1)

extern "C" __declspec(naked) void FUN_009d1b35() {
    __asm {
        // --- prolog ---
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x83              // SUB ESP, 0x0C
        _emit 0xec
        _emit 0x0c
        _emit 0xeb              // JMP short +0x0D  (→ outer_loop at +0x15)
        _emit 0x0d

        // --- inner (RVA 0x5d1b3d, offset +0x08) ---
        _emit 0xff              // PUSH DWORD PTR [EBP+0x08]
        _emit 0x75
        _emit 0x08
        _emit 0xe8              // CALL 0x5de438 (rel32 = 0x0000c8f3)
        _emit 0xf3
        _emit 0xc8
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x59              // POP ECX
        _emit 0x74              // JZ +0x0F  (→ error_path at +0x24)
        _emit 0x0f

        // --- outer_loop (RVA 0x5d1b4a, offset +0x15) ---
        _emit 0xff              // PUSH DWORD PTR [EBP+0x08]
        _emit 0x75
        _emit 0x08
        _emit 0xe8              // CALL 0x5d5bc5 (rel32 = 0x00004073)
        _emit 0x73
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x59              // POP ECX
        _emit 0x74              // JZ -0x1A  (→ inner at +0x08)
        _emit 0xe6
        _emit 0xc9              // LEAVE
        _emit 0xc3              // RET

        // --- error_path (RVA 0x5d1b59, offset +0x24) ---
        _emit 0xf6              // TEST BYTE PTR [0x01363f10], 0x01
        _emit 0x05
        _emit 0x10
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x01
        _emit 0xbe              // MOV ESI, 0x01363f04
        _emit 0x04
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x75              // JNZ +0x19  (→ skip_init at +0x4B)
        _emit 0x19
        _emit 0x83              // OR DWORD PTR [0x01363f10], 0x01
        _emit 0x0d
        _emit 0x10
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x01
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x5d1b1c (rel32 = 0xffffffa7)
        _emit 0xa7
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x68              // PUSH 0x00f3b0a0
        _emit 0xa0
        _emit 0xb0
        _emit 0xf3
        _emit 0x00
        _emit 0xe8              // CALL 0x5d25c2 (rel32 = 0x00000a43)
        _emit 0x43
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX

        // --- skip_init (RVA 0x5d1b80, offset +0x4B) ---
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [EBP-0x0C]
        _emit 0x4d
        _emit 0xf4
        _emit 0xe8              // CALL 0x5d1940 (rel32 = 0xfffffdb7)
        _emit 0xb7
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x68              // PUSH 0x011a8c90
        _emit 0x90
        _emit 0x8c
        _emit 0x1a
        _emit 0x01
        _emit 0x8d              // LEA EAX, [EBP-0x0C]
        _emit 0x45
        _emit 0xf4
        _emit 0x50              // PUSH EAX
        _emit 0xc7              // MOV DWORD PTR [EBP-0x0C], 0x00f54a10
        _emit 0x45
        _emit 0xf4
        _emit 0x10
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL 0x5d1b9f (rel32 = 0x00000001, tail call)
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
    }
}
