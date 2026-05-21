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
// FUNCTION: ffxivgame 0x00006330 — 2-arg __cdecl wrapper that conditionally
//                                  rebases its second argument and forwards
//                                  to FUN_009d4c25 (25 B / 0x19).
//
// Inspection (read from the disassembly at orig RVA 0x00006330):
//
//   __cdecl <something> FUN_00406330(<arg1>, int arg2);
//
//     if (arg2 == 1) return;                     // early-out, no fwd
//     arg2 = FUN_009d4d67() + 0x20;              // EAX-returning helper
//     return FUN_009d4c25(arg1, arg2);           // tail call (JMP rel32)
//
//   CALL FUN_009d4d67 takes no stack args (no PUSH before it) and
//   returns a value in EAX — the wrapper then biases the result by
//   0x20 and writes it back to the in-place second argument slot at
//   [ESP+8] before tail-jumping to FUN_009d4c25, which inherits the
//   caller's frame verbatim (this is why FUN_00406330 is __cdecl with
//   a final bare `RET` rather than `RET N` — caller still owns the
//   stack args at JMP time).
//
//   Calling convention: __cdecl (caller cleans up; bare `RET`).
//   Stack frame: none (no prologue / epilogue — direct ESP-relative).
//
// Asm shape (25 bytes):
//
//     00006330:  83 7c 24 08 01    CMP dword ptr [ESP+0x8], 0x1
//     00006335:  74 11             JZ  +0x11 → 0x00006348 (RET)
//     00006337:  e8 ?? ?? ?? ??    CALL FUN_009d4d67       ; rel32 reloc
//     0000633c:  83 c0 20          ADD  EAX, 0x20
//     0000633f:  89 44 24 08       MOV  [ESP+0x8], EAX
//     00006343:  e9 ?? ?? ?? ??    JMP  FUN_009d4c25       ; rel32 reloc, tail
//     00006348:  c3                RET
//
//   The two rel32 reloc slots after the E8 / E9 opcodes are masked by
//   the diff tool (IMAGE_REL_I386_REL32 in the .obj relocation table
//   covers each as a 4-byte wildcard window), so the .obj's `.text`
//   matches the orig slice byte-for-byte despite the linker not having
//   resolved FUN_009d4d67 / FUN_009d4c25 at .obj time.

extern "C" int  FUN_009d4d67();
extern "C" void FUN_009d4c25();

extern "C" __declspec(naked) void FUN_00406330() {
    __asm {
        cmp dword ptr [esp + 8], 1
        jz  already_one
        call FUN_009d4d67
        add eax, 0x20
        mov dword ptr [esp + 8], eax
        jmp FUN_009d4c25
    already_one:
        ret
    }
}
