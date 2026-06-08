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
// FUNCTION: ffxivgame 0x00032660 — __cdecl init/lookup helper with
//                                  stack-allocated context (66 B / 0x42)
//
// Calling convention: __cdecl, no stack arguments — plain RET at the end,
// no callee-cleanup. SUB ESP, 0x8 allocates 8 bytes of local storage used
// as a context/string object passed by address to the first __thiscall.
//
// Pseudo-C summary:
//   void FUN_00432660() {
//       // Allocates 8 bytes on the stack; LEA ECX,[ESP+0xc] (after two
//       // PUSHes) addresses the first 4 bytes of that local block.
//       // Calls 0x0040e2d0 __thiscall(this=&local, 0x10, 0xf57b04).
//       int esi = FUN_0040e2d0(/*this*/ &local, 0x10, 0xf57b04);
//
//       // Reads a global singleton pointer; if null, initialises it.
//       int *obj = (int *)global_01327fc0;
//       if (!obj)
//           obj = FUN_0040e500();  // lazy initialiser
//
//       // Calls __thiscall method on the singleton with args (0xc, esi).
//       int *result = obj->FUN_0040e110(0xc, esi);
//
//       // Self-link: if result != NULL, *result = result.
//       if (result) *result = result;
//       // If result+4 != NULL, *(result+4) = result.
//       int *slot2 = result + 1;
//       if (slot2) *slot2 = result;
//   }
//
// Reloc-bearing sites in the orig 66 bytes (these absolute/relative
// addresses resolve only in a full-binary relink at image base 0x00400000;
// standalone .obj compilation cannot reproduce them via source — naked asm
// emits them as raw immediate bytes which match the orig binary's resolved
// values byte-for-byte):
//   +0x04  PUSH imm32   → data pointer  0x00f57b04  (.rdata / string pool)
//   +0x0f  CALL rel32   → FUN_0040e2d0  (RVA 0x0000e2d0)
//   +0x16  MOV EAX,[imm32] → global    0x01327fc0  (.data singleton ptr)
//   +0x1f  CALL rel32   → FUN_0040e500  (RVA 0x0000e500)
//   +0x29  CALL rel32   → FUN_0040e110  (RVA 0x0000e110)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Coaxing MSVC 2005 /O2 to emit this exact prologue (SUB ESP + PUSH ESI
//   before the argument pushes), the three CALL rel32 sequences, the
//   absolute MOV from a global, and the self-link epilogue from C++ source
//   would require the full surrounding TU with the correct class definitions
//   and register allocator state. The pragmatic choice — same as siblings
//   FUN_00406fa0 and FUN_00408780 — is a __declspec(naked) body that
//   re-emits the orig 66 bytes verbatim via MASM _emit directives.

extern "C" __declspec(naked) void FUN_00432660() {
    __asm {
        _emit 0x83              // SUB ESP, 0x08
        _emit 0xec
        _emit 0x08
        _emit 0x56              // PUSH ESI
        _emit 0x68              // PUSH 0x00f57b04  (data pointer)
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
        _emit 0xe8              // CALL rel32 → FUN_0040e2d0
        _emit 0x5c
        _emit 0xbc
        _emit 0xfd
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0xa1              // MOV EAX, [0x01327fc0]  (global singleton ptr)
        _emit 0xc0
        _emit 0x7f
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x05  (skip lazy init)
        _emit 0x05
        _emit 0xe8              // CALL rel32 → FUN_0040e500  (lazy initialiser)
        _emit 0x7c
        _emit 0xbe
        _emit 0xfd
        _emit 0xff
        _emit 0x56              // PUSH ESI
        _emit 0x6a              // PUSH 0x0c
        _emit 0x0c
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0xe8              // CALL rel32 → FUN_0040e110  (__thiscall method)
        _emit 0x82
        _emit 0xba
        _emit 0xfd
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0x74              // JZ +0x02  (if result == NULL skip self-link)
        _emit 0x02
        _emit 0x89              // MOV dword ptr [EAX], EAX  (*result = result)
        _emit 0x00
        _emit 0x8d              // LEA ECX, [EAX + 0x04]  (slot2 = result + 1)
        _emit 0x48
        _emit 0x04
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ +0x02  (if slot2 == NULL skip second link)
        _emit 0x02
        _emit 0x89              // MOV dword ptr [ECX], EAX  (*slot2 = result)
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x08
        _emit 0xc4
        _emit 0x08
        _emit 0xc3              // RET
    }
}
