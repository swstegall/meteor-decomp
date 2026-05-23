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
// FUNCTION: ffxivgame 0x00411c70 — __stdcall forwarder: routes 3-arg call
//           to __thiscall FUN_00411330, discarding param_1
//           (17 bytes / 0x11)
//
// Asm (17 bytes @ orig RVA 0x00011c70):
//   8b 44 24 0c  MOV EAX, [ESP+0xc]   ; EAX = param_3
//   8b 4c 24 08  MOV ECX, [ESP+0x8]   ; ECX = param_2  (this for inner call)
//   50           PUSH EAX             ; push param_3 as stack arg
//   e8 ?? ?? ??  CALL FUN_00411330    ; __thiscall(ECX=param_2, param_3)
//   c2 0c 00     RET 0xc              ; __stdcall: pop 3 args * 4 = 12
//
// Outer calling convention: __stdcall, 3 stack args (12 bytes cleaned by
// callee via RET 0xc). param_1 at [ESP+0x4] is never read — the compiler
// generated this wrapper to adapt a 3-arg __stdcall interface to an
// inner __thiscall with one stack argument.

void __thiscall FUN_00411330(void *thisptr, void *param);

void __stdcall FUN_00411c70(void * /*param1*/, void *param2, void *param3)
{
    FUN_00411330(param2, param3);
}
