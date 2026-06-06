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
// FUNCTION: ffxivgame 0x00058a60 — __thiscall null-guarded forwarder that
//                                  passes this->_ptr (offset 0) to the
//                                  __cdecl helper FUN_0045b9e0 (19 B / 0x13).
//
// Asm (19 bytes @ 0x00058a60):
//   8b 01               MOV  EAX, [ECX]        ; EAX = this->_ptr  (offset 0)
//   85 c0               TEST EAX, EAX          ; null?
//   74 0a               JZ   +0x0a → 0x00058a70
//   50                  PUSH EAX               ; arg = this->_ptr
//   e8 74 2f 00 00      CALL FUN_0045b9e0      ; rel32 reloc, __cdecl
//   83 c4 04            ADD  ESP, 4            ; caller cleanup (1 arg)
//   c3                  RET                    ; return helper result (EAX)
//   33 c0               XOR  EAX, EAX          ; null branch → return 0
//   c3                  RET
//
//   Calling convention: __thiscall (ECX = this, no stack args, bare RET).
//   The inner call is __cdecl with one arg (ADD ESP,4 cleanup). When the
//   stored pointer is null the function short-circuits to a zeroed EAX.
//   FUN_0045b9e0's return value flows back through EAX untouched. The rel32
//   slot after the E8 is masked by the diff tool.
//
// Reconstructed via naked-asm passthrough — same idiom as the sibling
// __thiscall guard FUN_00446f50 — to keep the conditional epilogue layout
// byte-exact in a function where every byte is reloc-adjacent.

extern "C" void FUN_0045b9e0();

extern "C" __declspec(naked) void FUN_00458a60() {
    __asm {
        mov eax, [ecx]
        test eax, eax
        jz zero
        push eax
        call FUN_0045b9e0
        add esp, 4
        ret
    zero:
        xor eax, eax
        ret
    }
}
