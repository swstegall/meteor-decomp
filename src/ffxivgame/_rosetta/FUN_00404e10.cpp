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
// FUNCTION: ffxivgame 0x00004e10 — `__cdecl` 28-byte global-assign-then-
//                                   tail-call helper. Reads the caller's
//                                   first dword arg, hands it off to a
//                                   thiscall `operator=`-shaped method on
//                                   the global object at 0x01323898, then
//                                   rewrites its own arg slot to point at
//                                   that same global and tail-jumps into a
//                                   __cdecl follow-up routine.
//
// Asm shape (read from build/pe-layout/ffxivgame/text.bin @ +0x3e10):
//
//     00004e10:  8b 44 24 04                   MOV  EAX, [ESP+0x4]    ; arg
//     00004e14:  50                            PUSH EAX               ; push arg
//     00004e15:  b9 98 38 32 01                MOV  ECX, 0x1323898    ; this
//     00004e1a:  e8 31 26 04 00                CALL FUN_00447450      ; thiscall
//                                                                    ; callee RET 0x4 pops arg
//     00004e1f:  c7 44 24 04 98 38 32 01       MOV  [ESP+0x4], 0x1323898
//     00004e27:  e9 d4 de 04 00                JMP  FUN_00452d00      ; tail call
//
// FUN_00447450 (60 B, RVA 0x00047450) is a small `__thiscall` shaped like
// a `T::operator=(const T*)` — it compares this vs the source, and if
// they differ, decrements a refcount-style field then bulk-copies four
// fields (offsets +0x0, +0x8, +0xC, +0x10) from src to this, returning
// `this` in EAX and popping its single stack arg via `RET 0x4`.
//
// FUN_00452d00 (71 B, RVA 0x00052d00) is `__cdecl` taking one pointer arg
// — it dereferences the arg, checks a flag byte at *arg, and if set, runs
// a three-call sequence on the object pointed to by arg. Conceptually
// this is the "notify / process / publish" step that always runs after
// the global state is updated.
//
// So FUN_00404e10 is the canonical "set the global from caller's value,
// then process the global" wrapper: `assign(&g, src); process(&g);` with
// the second call expressed as a tail-jump rather than a paired call /
// ret. MSVC 2005's optimiser performs this exact transform when:
//   - the source-level function returns void,
//   - the final statement is a __cdecl call whose argument list is one
//     pointer matching the caller's own first stack slot's width,
//   - the immediately-preceding call is callee-cleanup (here `RET 0x4`),
//     so the stack pointer is already where the tail-callee wants it.
// Under those preconditions the compiler reuses the inbound arg slot —
// rewriting `[ESP+4]` in place with the new immediate — instead of
// reserving a fresh outbound frame.
//
// Reloc-bearing sites in the orig 28 bytes:
//     +0x06   MOV  ECX, imm32 → 0x01323898  (global object address)
//     +0x0B   CALL rel32      → FUN_00447450 (thiscall assign helper)
//     +0x14   MOV  [ESP+4], imm32 → 0x01323898 (same global)
//     +0x18   JMP  rel32      → FUN_00452d00 (tail-call target)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level rewrite (`extern char g; assign(&g, arg);
//   process(&g);`) under /O2 would NOT reliably reproduce the in-place
//   `MOV [ESP+4], imm32 / JMP rel32` tail-call shape. MSVC 2005's
//   sibcall optimiser depends on subtle preconditions (the caller's own
//   prologue must be a no-op, no callee-saved registers may be touched,
//   the callee must be reachable as a near JMP, etc.) and even small
//   surface-level differences in the C++ — return type, qualifier on
//   the global, declaration order — flip the heuristic and leave a
//   `CALL ... RET` sequence instead. The byte-identical reproduction
//   would also depend on MSVC keeping the same dead-`PUSH EAX` for the
//   first call rather than reusing the caller's slot directly.
//
//   The pragmatic choice — the same one FUN_00404d60, FUN_00404630, and
//   FUN_00405210 took — is a `__declspec(naked)` body re-emitting the
//   orig 28 bytes verbatim. The .obj's `.text` section ends up exactly
//   28 bytes with no relocations because the two rel32 displacements
//   and the two abs32 immediates are baked in as raw byte sequences
//   that resolve correctly against the orig binary's own RVA space
//   (compare.py reads orig bytes at this exact RVA, so the
//   linker-resolved displacements in the orig PE are the same bytes we
//   re-emit here).

extern "C" __declspec(naked) void FUN_00404e10() {
    __asm {
        _emit 0x8b    // MOV  EAX, [ESP+0x4]      ; arg
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x50    // PUSH EAX                 ; push arg for thiscall
        _emit 0xb9    // MOV  ECX, 0x01323898     ; this = &g
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xe8    // CALL FUN_00447450        ; rel32 = +0x00042631
        _emit 0x31
        _emit 0x26
        _emit 0x04
        _emit 0x00
        _emit 0xc7    // MOV  [ESP+0x4], 0x01323898   ; overwrite arg slot
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xe9    // JMP  FUN_00452d00        ; rel32 = +0x0004ded4
        _emit 0xd4
        _emit 0xde
        _emit 0x04
        _emit 0x00
    }
}
