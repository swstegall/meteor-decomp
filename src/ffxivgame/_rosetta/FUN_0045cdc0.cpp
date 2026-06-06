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
// FUNCTION: ffxivgame 0x0005cdc0 — thin __cdecl forwarder into the custom-
//                                  convention helper FUN_0045ccc0 (24 B / 0x18).
//
// Inspection (read from the disassembly at orig RVA 0x0005cdc0):
//
//   FUN_0045ccc0 is a non-standard-convention helper: it consumes ESI and
//   ECX as register parameters (it opens with `TEST ESI,ESI`, dereferences
//   [ESI+...] throughout, and does `MOV EDI,ECX`), plus two caller-cleaned
//   stack arguments. This wrapper sets that contract up:
//
//     ESI = arg1   ([ESP+4] on entry)
//     ECX = 0      (null register param)
//     push -1, push arg2   (the two stack args, in reverse order)
//     call FUN_0045ccc0
//     add esp, 8           (caller cleans the two stack args → __cdecl)
//
//   ESI is preserved across the call (push/pop) and the helper reads it as
//   an incoming register operand, so the load is real, not dead.
//
//   Calling convention: __cdecl (bare `RET`, caller owns the stack args).
//   Stack frame: none (direct ESP-relative; ESI is the only saved reg).
//
// Asm shape (24 bytes):
//
//     0005cdc0:  8b 44 24 08    MOV  EAX, [ESP+0x8]   ; arg2
//     0005cdc4:  56             PUSH ESI
//     0005cdc5:  8b 74 24 08    MOV  ESI, [ESP+0x8]   ; arg1 (post-push)
//     0005cdc9:  6a ff          PUSH -1
//     0005cdcb:  50             PUSH EAX
//     0005cdcc:  33 c9          XOR  ECX, ECX
//     0005cdce:  e8 ?? ?? ?? ?? CALL FUN_0045ccc0     ; rel32 reloc
//     0005cdd3:  83 c4 08       ADD  ESP, 0x8
//     0005cdd6:  5e             POP  ESI
//     0005cdd7:  c3             RET
//
//   The rel32 slot after the E8 opcode is masked by the diff tool
//   (IMAGE_REL_I386_REL32 in the .obj relocation table), so the .obj's
//   `.text` matches the orig slice byte-for-byte despite the linker not
//   having resolved FUN_0045ccc0 at .obj time.

extern "C" void FUN_0045ccc0();

extern "C" __declspec(naked) void FUN_0045cdc0() {
    __asm {
        mov  eax, [esp + 8]
        push esi
        mov  esi, [esp + 8]
        push -1
        push eax
        xor  ecx, ecx
        call FUN_0045ccc0
        add  esp, 8
        pop  esi
        ret
    }
}
