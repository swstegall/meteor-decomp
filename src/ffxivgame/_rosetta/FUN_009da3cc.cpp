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
// FUNCTION: ffxivgame 0x005da3cc — __thiscall virtual-inheritance pointer
//                                   adjustment thunk (22 B)
//
// Calling convention: __thiscall (ECX = this, no stack args, plain RET).
// No prologue — leaf function with no callee-saved registers.
//
// This is a virtual base offset thunk emitted by MSVC 2005 to navigate
// virtual inheritance.  When a derived object pointer is cast to one of
// its virtual base classes, the adjustment amount cannot be known
// statically; instead MSVC generates a thunk that reads the required
// displacement from a vbase-table structure stored right before the
// vtable (vftable[-1]).
//
// Trace:
//   EAX  = [ECX]         — load vfptr (first field of object)
//   EDX  = [EAX - 4]     — load vbase-table ptr (stored at vftable[-1])
//   EAX  = ECX           — copy raw `this` as the result base
//   EAX -= [EDX + 4]     — subtract primary displacement
//   EDX  = [EDX + 8]     — load secondary displacement
//   if (EDX == 0) → done — no virtual-base indirection needed
//   ECX -= EDX           — advance ECX to the vbptr slot
//   EAX -= [ECX]         — subtract the vbase-slot value
//   RET
//
// Asm (22 bytes @ orig RVA 0x005da3cc):
//   8b 01                MOV  EAX, dword ptr [ECX]
//   8b 50 fc             MOV  EDX, dword ptr [EAX - 0x4]
//   8b c1                MOV  EAX, ECX
//   2b 42 04             SUB  EAX, dword ptr [EDX + 0x4]
//   8b 52 08             MOV  EDX, dword ptr [EDX + 0x8]
//   85 d2                TEST EDX, EDX
//   74 04                JZ   +4   → done
//   2b ca                SUB  ECX, EDX
//   2b 01                SUB  EAX, dword ptr [ECX]
//   c3                   RET

extern "C" __declspec(naked) void FUN_009da3cc() {
    __asm {
        mov  eax, dword ptr [ecx]
        mov  edx, dword ptr [eax - 4]
        mov  eax, ecx
        sub  eax, dword ptr [edx + 4]
        mov  edx, dword ptr [edx + 8]
        test edx, edx
        jz   done
        sub  ecx, edx
        sub  eax, dword ptr [ecx]
    done:
        ret
    }
}
