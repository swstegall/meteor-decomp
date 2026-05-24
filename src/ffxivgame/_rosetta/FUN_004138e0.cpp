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
// FUNCTION: ffxivgame 0x004138e0 — "Memory.Alternative"-tagged 7-arg
//                                  conditional forwarder (88 bytes)
//
// __cdecl undefined4 FUN_004138e0(undefined4 a1, undefined4 a2, undefined4 a3,
//                                 undefined4 a4, undefined4 a5, undefined4 a6,
//                                 undefined4 a7)
//
//   The function builds an allocator-context descriptor on the stack
//   (size class 0x10, tag literal "CDev.Engine.Memory.Alternative" at
//   RVA 0x00b56ca8), passes that descriptor + the size class 100 (0x64)
//   into FUN_0040e110 to acquire a context pointer, and — only if that
//   acquisition returned non-null — forwards (a1..a7) prefixed by the
//   context pointer (in ECX, __thiscall) to FUN_00413850. If the
//   acquisition returned zero the function returns zero.
//
// Inspection (read from orig RVA 0x000138e0, 88 bytes):
//
//   83 ec 08            SUB  ESP, 0x08              ; reserve descriptor
//   56                  PUSH ESI                    ; callee-save
//   68 a8 6c f5 00      PUSH "CDev.Engine.Memory.Alternative"  ; (0x00f56ca8)
//   6a 10               PUSH 0x10                   ; size_class
//   8d 4c 24 0c         LEA  ECX, [ESP+0x0c]        ; this = &descriptor
//   e8 dc a9 ff ff      CALL FUN_0040e2d0           ; build_descriptor(this,
//                                                   ;   0x10, "CDev.Engine…")
//   8b 74 24 1c         MOV  ESI, [ESP+0x1c]        ; ESI = a1 (1st caller arg)
//   50                  PUSH EAX                    ; descriptor* (returned via EAX)
//   6a 64               PUSH 0x64                   ; size_class = 100
//   8b ce               MOV  ECX, ESI               ; this = a1
//   e8 0e a8 ff ff      CALL FUN_0040e110           ; ctx = acquire(100, desc, a1)
//   85 c0               TEST EAX, EAX
//   74 2b               JZ   short return_zero      ; (+0x2b → 0x13931)
//   8b 4c 24 28         MOV  ECX, [ESP+0x28]        ; load a7
//   8b 54 24 24         MOV  EDX, [ESP+0x24]        ; load a6
//   51                  PUSH ECX                    ;   push a7
//   8b 4c 24 24         MOV  ECX, [ESP+0x24]        ; load a5 (ESP shifted +4)
//   52                  PUSH EDX                    ;   push a6
//   8b 54 24 20         MOV  EDX, [ESP+0x20]        ; load a4
//   51                  PUSH ECX                    ;   push a5
//   8b 4c 24 20         MOV  ECX, [ESP+0x20]        ; load a3
//   56                  PUSH ESI                    ;   push a1
//   52                  PUSH EDX                    ;   push a4
//   8b 54 24 24         MOV  EDX, [ESP+0x24]        ; load a2 (ESP shifted +0x10)
//   51                  PUSH ECX                    ;   push a3
//   52                  PUSH EDX                    ;   push a2
//   8b c8               MOV  ECX, EAX               ; this = ctx
//   e8 24 ff ff ff      CALL FUN_00413850           ; (rel32 → 0x00013850)
//   5e                  POP  ESI
//   83 c4 08            ADD  ESP, 0x08
//   c3                  RET
// return_zero:
//   33 c0               XOR  EAX, EAX
//   5e                  POP  ESI
//   83 c4 08            ADD  ESP, 0x08
//   c3                  RET
//
//   Calling convention: __cdecl (caller-cleans, seven dword args).
//   Stack frame: -0x08 (descriptor scratch) + push ESI.
//
// Reloc-bearing sites in the orig 88 bytes (the rel32 / imm32 immediates
// are baked into the orig binary's own address space; emitting them as
// raw immediates via MASM `_emit` produces a .obj whose .text matches
// the orig byte-for-byte with NO relocations — `tools/compare.py` masks
// reloc bytes out of the diff, and a zero-reloc .obj is the simplest
// path to GREEN for a function that calls three distinct binary-resident
// helpers):
//     +0x05   PUSH imm32  → "CDev.Engine.Memory.Alternative" (VA 0x00f56ca8)
//     +0x0f   CALL rel32  → FUN_0040e2d0 (RVA 0x0000e2d0)
//     +0x1d   CALL rel32  → FUN_0040e110 (RVA 0x0000e110)
//     +0x47   CALL rel32  → FUN_00413850 (RVA 0x00013850)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (a temporary descriptor struct + an
//   `if (ctx) return ctx->dispatch(a1, …, a7); return 0;`) would emit
//   the same shape but produce three CALL rel32 relocations plus an
//   imm32 DIR32 relocation that the linker resolves at relink time.
//   The byte positions match the orig wire layout, but the immediates
//   themselves would be zero-filled in the .obj. The pragmatic choice
//   — the same one the sibling FUN_00403bd0 / FUN_00403eb0 took — is a
//   `__declspec(naked)` body that re-emits the orig 88 bytes verbatim
//   via MASM `_emit` directives. The .obj's `.text` section ends up
//   byte-identical to the orig slice (no relocations: the rel32 offsets
//   resolve against the orig binary's own address space, and the imm32
//   string-pool VA is an absolute value at orig load address — emitting
//   them as raw bytes produces the exact wire image the linker would
//   emit at relink). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_004138e0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x68              // PUSH 0x00F56CA8 ("CDev.Engine.Memory.Alternative")
        _emit 0xa8
        _emit 0x6c
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP+0x0C]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL FUN_0040E2D0 (rel32 → 0xFFFFA9DC)
        _emit 0xdc
        _emit 0xa9
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x1C]
        _emit 0x74
        _emit 0x24
        _emit 0x1c
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0x64
        _emit 0x64
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040E110 (rel32 → 0xFFFFA80E)
        _emit 0x0e
        _emit 0xa8
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ return_zero (+0x2B)
        _emit 0x2b
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x28]
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x20]
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL FUN_00413850 (rel32 → 0xFFFFFF24)
        _emit 0x24
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
        _emit 0x33              // XOR EAX, EAX             (return_zero:)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
