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
// FUNCTION: ffxivgame 0x009f6f97 (RVA 0x005f6f97) — CRT flush helper ($LN8):
//           get global CRT base address, offset it by 0x20, and call the
//           stream-flush dispatch with index=1 (19 B / 0x13).
//
// Context: this is the MSVC-internal label $LN8 inside a larger compiled
// function (likely part of _flsbuf or a similar CRT flush path). It is
// matched here as a standalone 19-byte function for decomp coverage.
//
// Calling convention: __cdecl, no parameters, void return.
// Stack frame: none (no prologue / epilogue — frameless leaf).
//
// Asm shape (19 bytes at orig RVA 0x005f6f97 / abs 0x009f6f97):
//
//   e8 ?? ?? ?? ??   CALL FUN_009d4d67       ; get global CRT base (int in EAX)
//   83 c0 20         ADD  EAX, 0x20          ; offset to target field (+32 bytes)
//   50               PUSH EAX                ; push as 2nd arg (right-to-left push)
//   6a 01            PUSH 0x1               ; push 1 as 1st arg (PUSH imm8)
//   e8 ?? ?? ?? ??   CALL FUN_009d4ede       ; stream-flush dispatch (int, int)
//   59               POP  ECX               ; cdecl cleanup: pop arg 1
//   59               POP  ECX               ; cdecl cleanup: pop arg 2
//   c3               RET
//
// The two CALL rel32 displacements are IMAGE_REL_I386_REL32 relocations;
// compare.py masks those 4-byte windows. All other 9 bytes (83 c0 20 50
// 6a 01 59 59 c3) must be byte-identical.
//
// Reconstruction: MSVC 2005 /O2 generates ADD ESP,8 (3 bytes) instead of
// POP ECX;POP ECX (2 bytes) for the 2-arg cdecl cleanup when the outer
// function is a plain C++ function. To get the exact byte sequence from
// the orig, we use __declspec(naked) with inline MASM mnemonics — the
// CALL instructions produce proper IMAGE_REL_I386_REL32 relocations in
// the .obj (compare.py masks them), while ADD/PUSH/POP/RET match verbatim.

extern "C" int  FUN_009d4d67();
extern "C" void FUN_009d4ede(int, int);

extern "C" __declspec(naked) void FUN_009f6f97() {
    __asm {
        call FUN_009d4d67   // e8 ?? ?? ?? ??  ; get global CRT base → EAX
        add  eax, 0x20      // 83 c0 20        ; offset to target field
        push eax            // 50              ; push 2nd arg (right-to-left)
        push 1              // 6a 01           ; push 1st arg as imm8
        call FUN_009d4ede   // e8 ?? ?? ?? ??  ; stream-flush dispatch(1, base+0x20)
        pop  ecx            // 59              ; cdecl cleanup: discard arg 1
        pop  ecx            // 59              ; cdecl cleanup: discard arg 2
        ret                 // c3
    }
}
