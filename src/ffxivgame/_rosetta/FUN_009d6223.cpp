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
// FUNCTION: ffxivgame 0x005d6223 — SEH __finally cleanup handler for
//                                  `eh_vector_constructor_iterator'` (24 B)
//
// MSVC compiles every __finally block into a frameless helper that shares
// the enclosing function's EBP.  This is that helper for the function
// `eh_vector_constructor_iterator'` at RVA 0x5d61d6 (confirmed by a
// direct CALL from offset 0x5d6216 inside the parent).
//
// Role: if element-array construction terminated abnormally (an exception
// escaped one of the pCtor calls), destroy the already-constructed
// elements by calling __ArrayUnwind.  If the loop finished normally
// ([EBP-0x20] was set to 1 by the parent before calling us), return
// immediately without action.
//
// Parent frame layout accessed by this handler:
//   [EBP+0x08] = pArray  — pointer to the first array element
//   [EBP+0x0C] = nSize   — element size in bytes
//   [EBP+0x18] = pDtor   — destructor function pointer (5th parameter)
//   [EBP-0x1C] = count   — number of elements constructed so far
//   [EBP-0x20] = flag    — 0 = abnormal (exception); 1 = normal completion
//
// Asm (24 bytes @ RVA 0x005d6223):
//   83 7d e0 00    CMP  dword ptr [EBP-0x20], 0   ; normal-completion flag
//   75 11          JNZ  done                       ; non-zero → normal exit
//   ff 75 18       PUSH dword ptr [EBP+0x18]       ; pDtor
//   ff 75 e4       PUSH dword ptr [EBP-0x1C]       ; count
//   ff 75 0c       PUSH dword ptr [EBP+0x0C]       ; nSize
//   ff 75 08       PUSH dword ptr [EBP+0x08]       ; pArray
//   e8 ?? ?? ?? ?? CALL __ArrayUnwind              ; rel32 reloc — masked
//   done:
//   c3             RET

extern "C" void __ArrayUnwind();

extern "C" __declspec(naked) void FUN_009d6223()
{
    __asm {
        cmp     dword ptr [ebp-0x20], 0
        jne     done
        push    dword ptr [ebp+0x18]
        push    dword ptr [ebp-0x1c]
        push    dword ptr [ebp+0x0c]
        push    dword ptr [ebp+0x08]
        call    __ArrayUnwind
    done:
        ret
    }
}
