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
// FUNCTION: ffxivgame 0x00046f50 — __thiscall guard that conditionally
//                                  forwards an (ptr, len) pair to the
//                                  realloc/emit helper FUN_0044d350 (24 B).
//
// Asm (24 bytes @ 0x00046f50):
//   80 79 11 00         CMP byte ptr [ECX+0x11], 0   ; this->flag set?
//   75 11               JNZ +0x11 → 0x00046f67 (RET) ; bail if non-zero
//   8b 41 04            MOV EAX, [ECX+4]             ; arg2 = this->_len
//   8b 09               MOV ECX, [ECX]               ; arg1 = this->_buf
//   6a 0b               PUSH 0xb                     ; arg3 = 0xb
//   50                  PUSH EAX
//   51                  PUSH ECX
//   e8 ec 63 00 00      CALL FUN_0044d350            ; rel32 reloc, __cdecl
//   83 c4 0c            ADD ESP, 0xc                 ; caller cleanup (3 args)
//   c3                  RET
//
//   Calling convention: __thiscall, no stack args (bare `RET`). The inner
//   call is __cdecl with 3 args (ADD ESP,0xc cleanup). FUN_0044d350 is the
//   same vector realloc/emit helper several siblings forward to. The rel32
//   reloc slot after the E8 is masked by the diff tool.

extern "C" void FUN_0044d350();

extern "C" __declspec(naked) void FUN_00446f50() {
    __asm {
        cmp byte ptr [ecx + 0x11], 0
        jnz done
        mov eax, [ecx + 4]
        mov ecx, [ecx]
        push 0xb
        push eax
        push ecx
        call FUN_0044d350
        add esp, 0xc
    done:
        ret
    }
}
