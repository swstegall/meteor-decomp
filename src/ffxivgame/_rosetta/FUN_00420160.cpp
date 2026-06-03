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
// FUNCTION: ffxivgame 0x00420160 — 5-arg __stdcall thunk to FUN_00431080 (32 B).
//
// Forwards its first four args to FUN_00431080 as a 6-arg call, inserting
// two zero literals as the 4th and 6th callee arguments. The fifth arg of
// this wrapper (RET 0x14 = 20 B = 5 dword args) is declared but never read.
//
// C equivalent:
//   void __stdcall FUN_00420160(int a1, int a2, int a3, int a4, int a5) {
//       FUN_00431080(a1, a2, a3, 0, a4, 0);
//   }
//
// MSVC evaluates __stdcall args right-to-left. The compiler pre-loads a4
// into EAX and a3 into EDX before any pushes (to avoid shifted-offset
// re-reads for those two), then reuses the freed registers as it walks
// toward a1.
//
// Disassembly (RVA 0x00020160, 32 bytes):
//   8b 44 24 10    MOV EAX, [ESP+0x10]     ; a4
//   8b 54 24 0c    MOV EDX, [ESP+0x0c]     ; a3
//   6a 00          PUSH 0                  ; callee arg6 = 0
//   50             PUSH EAX               ; callee arg5 = a4
//   8b 44 24 10    MOV EAX, [ESP+0x10]     ; a2  (after 2 pushes, +8)
//   6a 00          PUSH 0                  ; callee arg4 = 0
//   52             PUSH EDX               ; callee arg3 = a3
//   8b 54 24 14    MOV EDX, [ESP+0x14]     ; a1  (after 4 pushes, +16)
//   50             PUSH EAX               ; callee arg2 = a2
//   52             PUSH EDX               ; callee arg1 = a1
//   e8 03 0f 01 00 CALL FUN_00431080       ; rel32
//   c2 14 00       RET 0x14               ; __stdcall, clean 5 args

extern "C" void FUN_00431080();

extern "C" __declspec(naked) void FUN_00420160() {
    __asm {
        mov eax, dword ptr [esp + 0x10]
        mov edx, dword ptr [esp + 0x0c]
        push 0
        push eax
        mov eax, dword ptr [esp + 0x10]
        push 0
        push edx
        mov edx, dword ptr [esp + 0x14]
        push eax
        push edx
        call FUN_00431080
        ret 0x14
    }
}
