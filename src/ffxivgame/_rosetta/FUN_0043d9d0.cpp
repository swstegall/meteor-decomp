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
// FUNCTION: ffxivgame 0x0043d9d0 — extract-and-store with optional virtual
//                                   call under an MSVC C++ SEH/GS frame
//                                   (__cdecl, 4 args, 133 bytes).
//
// Stack layout after prologue (relative to the final ESP, which is 0x1c
// below the function-entry ESP):
//
//   [esp + 0x00]  GS cookie  (xored with final ESP)
//   [esp + 0x04]  saved ESI
//   [esp + 0x08]  local1     (dword; init 0, later set to 1)
//   [esp + 0x0c]  local2     (dword; output slot for FUN_0043d580)
//   [esp + 0x10]  prev fs:[0] (SEH chain link)
//   [esp + 0x14]  SEH handler RVA (0x00e56b1d)
//   [esp + 0x18]  unwind state (−1 initially, 0 after the first call)
//   [esp + 0x1c]  return address
//   [esp + 0x20]  arg1 — pointer to receive the extracted value
//   [esp + 0x24]  arg2 — object pointer (ECX for the __thiscall callee)
//   [esp + 0x28]  arg3 — extra argument forwarded to FUN_0043d580
//   [esp + 0x2c]  arg4 — unused (not accessed after prologue adjustments)
//
// Behaviour:
//   1. Calls FUN_0043d580 as __thiscall on arg2, passing (&local2, arg3).
//      That function returns a pointer whose first dword holds a value.
//   2. Saves that value and zeroes the source slot.
//   3. Stores the saved value into *arg1.
//   4. If local2 (written by step 1) is non-null, calls local2->vtable[0](1).
//   5. Returns arg1.

extern "C" {

// SEH handler trampoline at VA 0x00e56b1d in the orig binary.
int FUN_00e56b1d();

// __thiscall method called with (this=arg2, &local2, arg3).
int FUN_0043d580();

extern unsigned __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_0043d9d0() {
    __asm {
        // --- SEH / GS prologue -------------------------------------------------
        push    -1                          // unwind state = -1
        push    offset FUN_00e56b1d         // SEH handler (DIR32 reloc)
        mov     eax, dword ptr fs:[0]       // save prev SEH head
        push    eax
        sub     esp, 8                      // reserve 8 bytes (local1 + local2)
        push    esi                         // preserve ESI
        mov     eax, __security_cookie      // GS cookie (DIR32 reloc)
        xor     eax, esp                    // anchor to current ESP
        push    eax                         // push cookie
        lea     eax, [esp + 0x10]           // → EXCEPTION_REGISTRATION.prev
        mov     dword ptr fs:[0], eax       // install SEH frame

        // --- Body --------------------------------------------------------------
        mov     dword ptr [esp + 0x8], 0    // local1 = 0

        mov     eax, dword ptr [esp + 0x28] // eax = arg3
        push    eax                         // push arg3 (2nd callee arg)
        lea     ecx, [esp + 0x10]           // ecx = &local2 (was [esp+0xc] before push)
        push    ecx                         // push &local2 (1st callee arg)
        mov     ecx, dword ptr [esp + 0x2c] // ecx = arg2 (this ptr for __thiscall)
        call    FUN_0043d580                 // result → EAX (REL32 reloc)

        mov     edx, dword ptr [eax]        // edx = *result (the value to extract)
        mov     esi, dword ptr [esp + 0x28] // esi = arg1 (adjusted for the 2 pushes)
        mov     ecx, edx
        mov     dword ptr [eax], 0          // zero the source slot
        add     esp, 8                      // clean the 2 pushes

        mov     dword ptr [esi], ecx        // *arg1 = extracted value

        mov     ecx, dword ptr [esp + 0xc]  // ecx = local2 (written by the callee)
        test    ecx, ecx

        mov     dword ptr [esp + 0x18], 0   // advance SEH state to 0
        mov     dword ptr [esp + 0x8], 1    // local1 = 1

        jz      done                        // if local2 == NULL skip the virtual call

        mov     eax, dword ptr [ecx]        // eax = vtable pointer
        mov     edx, dword ptr [eax]        // edx = vtable[0]
        push    1                           // argument = 1
        call    edx                         // local2->vtable[0](1)  (callee cleans arg)

    done:
        mov     eax, esi                    // return arg1

        // --- SEH / GS epilogue -------------------------------------------------
        mov     ecx, dword ptr [esp + 0x10] // restore prev fs:[0]
        mov     dword ptr fs:[0], ecx
        pop     ecx                         // drop GS cookie
        pop     esi                         // restore ESI
        add     esp, 0x14                   // drop locals + SEH record
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
