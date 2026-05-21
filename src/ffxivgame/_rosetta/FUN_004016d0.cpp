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
// FUNCTION: ffxivgame 0x004016d0 — single-arg "dispatch event to bound
//                                  handler" thunk (84 B, __cdecl, void)
//
// Asm shape (read from orig RVA 0x000016d0):
//
//   void __cdecl FUN_004016d0(void *arg) {
//       if ((unsigned char)arg == 0) goto fall_through;       // 80 7c 24 04 00  74 44
//       if (!FUN_0090ed60()) goto fall_through;                // e8 ?? ?? ?? ??  84 c0  74 3b
//       Obj *outer = FUN_0090ece0();                            // e8 ?? ?? ?? ??
//       if (outer == 0) goto fall_through;                     // 85 c0  74 32
//       Inner *inner = outer->get_inner();                      // ECX=outer; call FUN_0090ea30
//                                                                 ;   (mov eax, [ecx+0x228]; ret)
//                                                                 ;   pushed &arg before the call —
//                                                                 ;   it stays on the stack as the
//                                                                 ;   4th arg to FUN_00403b20.
//       if (FUN_00403b20(g_event_table, 11, inner, &arg) != 1) // push inner; push 0xb;
//           goto fall_through;                                  ; push offset g_event_table;
//                                                                ; call FUN_00403b20; add esp, 0x10;
//                                                                ; cmp al, 1; jne fall_through.
//       if (arg == 0) goto fall_through;                        // 8b 44 24 04  85 c0  74 0a
//       (*g_callback)(*(void**)arg);                            // 8b 10  52
//                                                                ; ff 15 28 e4 f3 00; ret
//       return;
//   fall_through:
//       (*g_callback)(NULL);                                    // 6a 00
//                                                                ; ff 15 28 e4 f3 00; ret
//   }
//
// Externals touched:
//   FUN_0090ed60      sibling, "is enabled?" gate         (returns AL=0/1)
//   FUN_0090ece0      sibling, "get outer object" lookup  (returns EAX, or NULL)
//   FUN_0090ea30      sibling, __thiscall { mov eax,[ecx+0x228]; ret }
//                     i.e. outer->inner_ptr (a `Inner*` stored at +0x228).
//   FUN_00403b20      already matched at _rosetta/FUN_00403b20.cpp —
//                     case-insensitive key/value lookup in an Entry[] table.
//   g_event_table     @ 0x01265018  — the 11-entry Entry[] table searched
//                                     by FUN_00403b20 (key=cstr, value=u32).
//   g_callback        @ 0x00f3e428  — function-pointer slot (void (*)(void *))
//                                     populated by some init code; the
//                                     dword-ptr indirect call dispatches
//                                     either the looked-up event payload or
//                                     a NULL on every fall-through arm.
//
// Reconstruction strategy — naked-asm passthrough:
//
//   Five reloc-bearing operands (four CALL REL32s + one PUSH offset for
//   the lookup table + one CALL dword ptr [DIR32] used twice for the
//   indirect-callback site) all resolve only at full-binary re-link
//   time. The compiled .obj's .text contributes the exact 84 verbatim
//   bytes; tools/compare.py masks each reloc window during the diff.
//
//   This mirrors the pattern used by FUN_004013d0 / FUN_00401750 /
//   FUN_00401b70 in the same _rosetta/ directory.

extern "C" {

// Sibling-function call targets (direct CALL near, e8 + REL32 reloc).
void FUN_0090ed60();
void FUN_0090ece0();
void FUN_0090ea30();
void FUN_00403b20();

// Globals touched.
//   g_event_table — pushed by `68 RR RR RR RR` (DIR32 on the immediate).
//   g_callback    — read by `ff 15 RR RR RR RR` (DIR32 on the operand).
// The link-time addresses (0x01265018 / 0x00f3e428) come from the
// full-binary relink; compare.py masks the 4-byte reloc window in the
// diff so the chosen extern symbol name is immaterial.
int  g_event_table;
void (*g_callback)();

__declspec(naked) void FUN_004016d0() {
    __asm {
        // if ((unsigned char)arg == 0) goto fall_through;
        cmp     byte ptr [esp+4], 0
        jz      fall_through

        // if (!FUN_0090ed60()) goto fall_through;
        call    FUN_0090ed60
        test    al, al
        jz      fall_through

        // Obj *outer = FUN_0090ece0(); if (!outer) goto fall_through;
        call    FUN_0090ece0
        test    eax, eax
        jz      fall_through

        // Inner *inner = outer->get_inner();  (push &arg first — survives
        // the __thiscall and is consumed by FUN_00403b20 as its 4th arg.)
        lea     ecx, [esp+4]
        push    ecx
        mov     ecx, eax
        call    FUN_0090ea30

        // FUN_00403b20(g_event_table, 11, inner, &arg)
        push    eax                          ; inner
        push    0Bh                          ; count = 11
        push    offset g_event_table         ; arr
        call    FUN_00403b20
        add     esp, 10h
        cmp     al, 1
        jnz     fall_through

        // if (arg == 0) goto fall_through;
        mov     eax, [esp+4]
        test    eax, eax
        jz      fall_through

        // (*g_callback)(*(void**)arg);
        mov     edx, [eax]
        push    edx
        call    dword ptr [g_callback]
        ret

    fall_through:
        // (*g_callback)(NULL);
        push    0
        call    dword ptr [g_callback]
        ret
    }
}

}  // extern "C"
