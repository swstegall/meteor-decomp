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
// FUNCTION: ffxivgame 0x00418f00 — __cdecl, 3-arg, GS+SEH prologue; extracts
//           a node from a container via FUN_004323d0, stores it, then
//           optionally fires a vtable[0] callback (134 B / 0x86).
//
// Stack frame layout after prologue (relative to the post-prologue ESP):
//   [esp + 0x00] = GS cookie (XOR'd with original esp)
//   [esp + 0x04] = saved ESI
//   [esp + 0x08] = local_1   (flag: 0 before call, 1 after)
//   [esp + 0x0c] = local_2   (out-param from FUN_004323d0; callback obj ptr)
//   [esp + 0x10] = saved prev FS:[0]   ← SEH frame start
//   [esp + 0x14] = SEH handler stub (DIR32 → FUN_00e5557a)
//   [esp + 0x18] = SEH state (-1 initial, then 0)
//   [esp + 0x1c] = return address
//   [esp + 0x20] = arg1  (output: void** — receives extracted node)
//   [esp + 0x24] = arg2  (key / selector passed to FUN_004323d0)
//   [esp + 0x28] = arg3  (context / container passed to FUN_004323d0)
//
// High-level logic:
//
//   void* __cdecl FUN_00418f00(void** arg1, arg2, arg3) {
//       void *local_2 = NULL;
//       local_1 = 0;
//       void *result = FUN_004323d0(&local_2, arg2, 0, arg3);
//       void *node = *result;
//       *result = NULL;
//       *arg1 = node;
//       local_1 = 1;
//       // state 0: callback firing is the guarded resource
//       if (local_2) {
//           (*(void (**)(void*, int))(*(void**)local_2))(local_2, 1);
//       }
//       return arg1;
//   }
//
// Calling convention: __cdecl (RET with no stack-pop; caller cleans 3 args).
//
// Reloc-bearing positions in the .obj:
//   off 0x03   IMAGE_REL_I386_DIR32  → SEH handler stub (FUN_00e5557a)
//   off 0x13   IMAGE_REL_I386_DIR32  → __security_cookie (0x012ea8b0)
//   off 0x3e   IMAGE_REL_I386_REL32  → FUN_004323d0

extern "C" {

// SEH handler stub trampoline at 0x00e5557a in the original binary.
// Provides the DIR32 reloc target for the "push offset" at offset 0x02.
int FUN_00e5557a();

// Container-extraction helper called with (&local_2, arg2, 0, arg3).
// Returns a pointer whose first dword is extracted and zeroed.
int FUN_004323d0();

extern unsigned __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_00418f00() {
    __asm {
        // --- GS / SEH prologue -----------------------------------------
        push    -1                          // initial SEH state
        push    offset FUN_00e5557a         // SEH handler stub (DIR32 reloc)
        mov     eax, dword ptr fs:[0]       // save previous SEH chain head
        push    eax
        sub     esp, 8                      // allocate local_1 + local_2
        push    esi
        mov     eax, __security_cookie      // DIR32 reloc; A1 opcode (5 B)
        xor     eax, esp
        push    eax                         // cookie
        lea     eax, [esp + 0x10]           // &prev_FS0 = start of SEH frame
        mov     dword ptr fs:[0], eax       // install SEH registration

        // --- Body -------------------------------------------------------
        // Load arg2 into ECX and clear local_1.
        mov     ecx, dword ptr [esp + 0x24]
        mov     dword ptr [esp + 0x8], 0x0      // local_1 = 0

        // Push 4 args for FUN_004323d0(&local_2, arg2, 0, arg3).
        mov     eax, dword ptr [esp + 0x28]     // EAX = arg3
        push    eax                             // arg3
        push    0x0                             // 0
        push    ecx                             // arg2
        lea     edx, [esp + 0x18]               // EDX = &local_2
        push    edx
        call    FUN_004323d0                    // REL32 reloc; returns node ptr in EAX

        // Extract first dword from returned node, stash caller's out-ptr.
        mov     ecx, dword ptr [eax]            // ECX = *node (linked-list head)
        mov     esi, dword ptr [esp + 0x30]     // ESI = arg1 (with 4 extra slots)
        mov     dword ptr [eax], 0x0            // clear node->next
        add     esp, 0x10                       // discard the 4 pushed args

        // Store extracted head into *arg1.
        mov     dword ptr [esi], ecx

        // Test local_2 (the callback object pointer).
        mov     ecx, dword ptr [esp + 0x0c]
        test    ecx, ecx
        mov     dword ptr [esp + 0x18], 0x0     // SEH state = 0 (callback guard)
        mov     dword ptr [esp + 0x08], 0x1     // local_1 = 1
        jz      done                            // no callback? skip

        // Fire callback: (*(*local_2)[0])(local_2, 1)  [__thiscall vtable[0]]
        mov     edx, dword ptr [ecx]            // EDX = vtable ptr
        mov     eax, dword ptr [edx]            // EAX = vtable[0]
        push    0x1
        call    eax                             // callee cleans its own arg

    done:
        // --- Epilogue ---------------------------------------------------
        mov     eax, esi                        // return arg1
        mov     ecx, dword ptr [esp + 0x10]     // recover saved prev FS:[0]
        mov     dword ptr fs:[0], ecx           // restore SEH chain
        pop     ecx                             // drop cookie
        pop     esi
        add     esp, 0x14                       // drop locals + SEH frame slots
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
