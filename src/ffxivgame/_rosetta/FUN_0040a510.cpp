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
// FUNCTION: ffxivgame 0x0040a510 — engine_memory global-free dispatcher.
//
// Free-function front door over the FFXIV custom allocator. The
// allocator stores a pointer to its owning Allocator instance in the
// 4 bytes immediately *before* the user-visible block; this wrapper
// nul-guards the input, fishes the Allocator out of `[p - 4]` into
// ECX, and tail-jumps to its __thiscall Free routine (FUN_0040df70).
//
// Asm (23 bytes):
//   8b 44 24 04        MOV  EAX, [ESP+4]    ; eax = p
//   85 c0              TEST EAX, EAX
//   74 0c              JZ   done
//   8b 48 fc           MOV  ECX, [EAX-4]    ; ecx = owning Allocator
//   89 44 24 04        MOV  [ESP+4], EAX    ; re-pin p for the tail call
//   e9 RR RR RR RR     JMP  FUN_0040df70    ; __thiscall, takes p
// done:
//   c2 04 00           RET  4               ; __stdcall, 1 stack arg
//
// MSVC 2005 emits the redundant `MOV [ESP+4], EAX` because tail-call
// lowering treats the outgoing stack-arg slot as the destination of
// an explicit store — even when the source operand already lives in
// that slot.

extern "C" int FUN_0040df70();

extern "C" __declspec(naked) void engine_memory_global_free() {
    __asm {
        mov  eax, [esp + 4]
        test eax, eax
        jz   done
        mov  ecx, [eax - 4]
        mov  [esp + 4], eax
        jmp  FUN_0040df70
    done:
        ret  4
    }
}
