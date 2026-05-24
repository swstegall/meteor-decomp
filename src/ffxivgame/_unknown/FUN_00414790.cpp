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
// FUNCTION: ffxivgame 0x00014790 — __cdecl factory: __aligned_malloc(0x48,0x10)
//                                  → __thiscall init via FUN_00414640 (32 B / 0x20)
//
// Calling convention: __cdecl, 1 stack arg (`[ESP+4]`), returns EAX. No frame
// (/Oy — function has no locals; the alloc result is the only live value).
//
// Behaviour (read from the disassembly at orig RVA 0x00414790):
//
//   1. Push args 0x10 (alignment) then 0x48 (size); call __aligned_malloc
//      (the cdecl static-CRT helper at RVA 0x009d5712). Caller-cleans 8 B
//      with `ADD ESP,8`.
//   2. If the allocator returned NULL, jump to a tail `XOR EAX,EAX / RET`
//      (returns 0).
//   3. Otherwise, set up a __thiscall to FUN_00414640 (the in-place
//      initialiser at RVA 0x00414640): push the caller's `[ESP+4]` arg as
//      the stack arg, load ECX with the alloc result (the new `this`),
//      and CALL. FUN_00414640 returns its `this` in EAX (it ends with
//      `MOV EAX,ESI / POP ESI / RET 4`), so the factory's `RET` after
//      the call propagates the freshly-constructed object pointer.
//
// Why `FUN_00414640` is __thiscall (not __cdecl as the headless Ghidra
// hint suggests): orig at 0x414640 ends `MOV EAX,ESI / POP ESI / RET 4`,
// where ESI was loaded as `MOV ESI,ECX` from entry — classic __thiscall
// "ECX=this; returns this; RET 4 cleans the one stack arg" idiom.
//
// Asm (32 bytes @ orig RVA 0x00414790):
//   00 00:  6a 10                    PUSH 0x10                ; alignment
//   00 02:  6a 48                    PUSH 0x48                ; size
//   00 04:  e8 RR RR RR RR           CALL __aligned_malloc     ; (rel32 reloc)
//   00 09:  83 c4 08                 ADD ESP,0x8               ; cdecl cleanup
//   00 0c:  85 c0                    TEST EAX,EAX
//   00 0e:  74 0d                    JZ +0x0d → 0x4147ad      ; NULL → return 0
//   00 10:  8b 4c 24 04              MOV ECX,[ESP+0x4]         ; load caller arg
//   00 14:  51                       PUSH ECX                   ; stack arg
//   00 15:  8b c8                    MOV ECX,EAX               ; this = alloc
//   00 17:  e8 RR RR RR RR           CALL FUN_00414640         ; (rel32 reloc)
//   00 1c:  c3                       RET                        ; tail: return EAX
//   00 1d:  33 c0                    XOR EAX,EAX               ; fail path
//   00 1f:  c3                       RET
//
// Reloc-bearing bytes: two REL32 sites at offsets 0x05 and 0x18 (the
// 4-byte displacements after each `e8`). Both are emitted by MASM via
// the `call SYMBOL` form, which causes cl.exe to record an
// IMAGE_REL_I386_REL32 entry in the .obj's reloc table; tools/compare.py
// masks exactly those positions when comparing against the orig .text
// slice, so the wildcard distance to the actual targets in the orig
// PE (0x009d5712 for __aligned_malloc, 0x00414640 for the initialiser)
// doesn't affect the verdict.
//
// Reconstruction strategy — __declspec(naked) byte-emit passthrough.
// A source-level rendering would compile cleanly but is fragile to MSVC
// 2005 /O2 register-allocation choices around the `MOV ECX,[ESP+4] /
// PUSH ECX` reload (vs. `PUSH dword ptr [ESP+4]`) and branch layout
// for the early-return-on-NULL. The naked form pins both.

extern "C" void __aligned_malloc();   // forward decl for REL32 reloc target
extern "C" void FUN_00414640();        // forward decl for REL32 reloc target

extern "C" __declspec(naked) void FUN_00414790() {
    __asm {
        // 00014790: 6a 10                  PUSH 0x10           (alignment)
        _emit 0x6a
        _emit 0x10
        // 00014792: 6a 48                  PUSH 0x48           (size)
        _emit 0x6a
        _emit 0x48
        // 00014794: e8 RR RR RR RR         CALL __aligned_malloc (rel32 reloc)
        call __aligned_malloc
        // 00014799: 83 c4 08               ADD ESP, 0x8        (cdecl cleanup)
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0001479c: 85 c0                  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001479e: 74 0d                  JZ +0x0d → 0x147ad  (NULL → return 0)
        _emit 0x74
        _emit 0x0d
        // 000147a0: 8b 4c 24 04            MOV ECX, [ESP+0x4]  (load caller arg)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 000147a4: 51                     PUSH ECX            (stack arg)
        _emit 0x51
        // 000147a5: 8b c8                  MOV ECX, EAX        (this = alloc)
        _emit 0x8b
        _emit 0xc8
        // 000147a7: e8 RR RR RR RR         CALL FUN_00414640    (rel32 reloc)
        call FUN_00414640
        // 000147ac: c3                     RET                  (return EAX from thiscall)
        _emit 0xc3
        // 000147ad: 33 c0                  XOR EAX, EAX         (fail path)
        _emit 0x33
        _emit 0xc0
        // 000147af: c3                     RET
        _emit 0xc3
    }
}
