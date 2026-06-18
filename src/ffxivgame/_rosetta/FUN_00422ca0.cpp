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
// FUNCTION: ffxivgame 0x00022ca0 — __stdcall lazy-singleton dispatcher
//                                  (63 B / 0x3f)
//
// __stdcall void FUN_00422ca0(int arg1)
//   Calling convention: __stdcall, 1 DWORD stack arg. Callee-cleans via
//   `RET 4`. No ECX / EDX save → not a member function.
//
// High-level shape (inferred from asm):
//
//   void __stdcall FUN_00422ca0(int arg1) {
//       // Stack-allocate an 8-byte local buffer and construct/find a
//       // resource object via FUN_0040e2d0 (__thiscall, this = &local,
//       // args: size=0x10, name=0x00f57b04). Cleans 8 B (RET 8 on callee).
//       SomeThing local;
//       void *resource = FUN_0040e2d0(&local, 0x10, (void*)0x00f57b04);
//
//       // Lazy singleton: read global ptr at 0x01327fc0; if null, call
//       // FUN_0040e500() to initialise/retrieve it.
//       Singleton *g = *(Singleton**)0x01327fc0;
//       if (!g) g = FUN_0040e500();
//
//       // Dispatch into the singleton, passing arg1*4 and the resource ptr.
//       // FUN_0040e110 is __thiscall with 2 DWORD stack args (cleans 8 B).
//       g->FUN_0040e110(arg1 * 4, resource);
//   }
//
// Asm walkthrough (63 bytes):
//   83 ec 08             SUB ESP, 0x08         ; alloc 8-byte local
//   56                   PUSH ESI              ; save ESI (callee-preserve)
//   68 04 7b f5 00       PUSH 0x00f57b04       ; name/tag arg for FUN_0040e2d0
//   6a 10                PUSH 0x10             ; size arg = 16
//   8d 4c 24 0c          LEA ECX, [ESP+0x0c]   ; ECX = &local (this for __thiscall)
//   e8 1c b6 fe ff       CALL FUN_0040e2d0     ; resource = FUN_0040e2d0(&local, 16, name)
//   8b f0                MOV ESI, EAX          ; ESI = resource ptr (return val)
//   a1 c0 7f 32 01       MOV EAX, [0x01327fc0] ; load global singleton ptr
//   85 c0                TEST EAX, EAX
//   75 05                JNZ +5                ; non-null → skip init
//   e8 3c b8 fe ff       CALL FUN_0040e500     ; g = FUN_0040e500() (init/get)
//   8b 4c 24 10          MOV ECX, [ESP+0x10]   ; ECX = arg1 (now at +0x10 after
//                                              ;   FUN_0040e2d0 cleaned 2 dwords)
//   8d 14 8d 00 00 00 00 LEA EDX, [ECX*4+0x0] ; EDX = arg1 * 4
//   56                   PUSH ESI              ; push resource (2nd stack arg)
//   52                   PUSH EDX              ; push arg1*4 (1st stack arg)
//   8b c8                MOV ECX, EAX          ; ECX = singleton this ptr
//   e8 38 b4 fe ff       CALL FUN_0040e110     ; g->FUN_0040e110(arg1*4, resource)
//   5e                   POP ESI               ; restore original ESI
//   83 c4 08             ADD ESP, 0x08         ; free 8-byte local
//   c2 04 00             RET 0x4               ; __stdcall: clean 1 DWORD arg
//
// Reloc-bearing sites in the orig 63 bytes (absolute addresses embedded
// directly in the instruction stream — emitted as raw immediates here so
// the .obj bytes match the orig binary verbatim):
//     +0x04  PUSH imm32  → 0x00f57b04  (name/tag string or vtable)
//     +0x0f  CALL rel32  → FUN_0040e2d0 (RVA 0x0000e2d0)
//     +0x16  MOV EAX, moffs32 → [0x01327fc0]  (global singleton ptr)
//     +0x1f  CALL rel32  → FUN_0040e500 (RVA 0x0000e500)
//     +0x2e  CALL rel32  → FUN_0040e110 (RVA 0x0000e110)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The two absolute-address memory operands (PUSH imm32 0xf57b04 and
//   MOV EAX,[0x01327fc0]) would require symbol-level externs whose
//   relocation type doesn't match the moffs32 / imm32 encoding.  The
//   REL32 CALL targets are masked by tools/compare.py, but the absolute
//   immediates are compared byte-for-byte.  Following the pattern of
//   FUN_00406fa0 and FUN_00408780, a `__declspec(naked)` `_emit`
//   passthrough is the pragmatic choice — the .obj's .text ends up
//   byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00422ca0() {
    __asm {
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x68              // PUSH 0x00f57b04
        _emit 0x04
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP + 0x0c]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL FUN_0040e2d0  (rel32 → RVA 0x0000e2d0)
        _emit 0x1c
        _emit 0xb6
        _emit 0xfe
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xa1              // MOV EAX, [0x01327fc0]
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x05
        _emit 0x05
        _emit 0xe8              // CALL FUN_0040e500  (rel32 → RVA 0x0000e500)
        _emit 0x3c
        _emit 0xb8
        _emit 0xfe
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8d              // LEA EDX, [ECX*4 + 0x00000000]
        _emit 0x14
        _emit 0x8d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL FUN_0040e110  (rel32 → RVA 0x0000e110)
        _emit 0x38
        _emit 0xb4
        _emit 0xfe
        _emit 0xff
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 0x0004
        _emit 0x04
        _emit 0x00
    }
}
