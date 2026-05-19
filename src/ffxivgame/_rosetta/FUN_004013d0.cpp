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
// FUNCTION: ffxivgame 0x004013d0 — Main::~Main (composite-class destructor,
//                                  130 B, MSVC /EHsc /GS frame, 4 member
//                                  sub-objects destroyed in reverse decl)
//
// Asm shape (from asm/ffxivgame/000013d0_FUN_004013d0.s):
//
//   ; Standard MSVC SEH+/GS prologue for a __thiscall function whose
//   ; body owns destructible C++ sub-objects.
//   push -1                       ; initial EH state index
//   push offset eh_handler        ; SEH handler / funcinfo table (reloc)
//   mov  eax, fs:[0]              ; chain prev FS:[0]
//   push eax
//   push ecx                      ; placeholder for `this` slot
//   push esi                      ; callee-saved
//   mov  eax, __security_cookie   ; /GS cookie (reloc)
//   xor  eax, esp                 ; cookie ^ ESP
//   push eax
//   lea  eax, [esp+0xc]           ; &EXCEPTION_REGISTRATION_RECORD
//   mov  fs:[0], eax              ; install registration
//
//   ; Body — destructor proper.
//   mov  esi, ecx                 ; ESI = this
//   mov  [esp+0x8], esi           ; rewrite the `this` slot (EH funcinfo
//                                 ;   reads it to find `this` during unwind)
//   mov  [esi], offset main_vftable   ; restore Main's own vftable (so
//                                 ;   virtual calls in member dtors land
//                                 ;   on Main, not on any derived class)
//
//   ; Destroy each member in reverse declaration order.
//   ; The EH state index at [esp+0x14] records which members are still
//   ; alive so that, on an exception during a dtor, the unwind handler
//   ; only destroys the previously-completed members.
//   ;
//   ; State value before each call:
//   ;   `2`  members 0..2 still alive (about to destroy m3 at +0x8d8)
//   ;   `1`  members 0..1 still alive (about to destroy m2 at +0x880)
//   ;   `0`  member  0    still alive (about to destroy m1 at +0x3a0)
//   ;   `-1` nothing alive             (about to destroy m0 at +0x30)
//   ;
//   ; MSVC uses BYTE writes for the 1 and 0 transitions because the
//   ; upper three bytes of the state slot are known-zero from the
//   ; preceding dword write of `2`. The -1 transition reverts to a
//   ; dword write because all four bytes must change (0x00 → 0xff).
//
//   lea  ecx, [esi+0x8d8]
//   mov  dword ptr [esp+0x14], 2
//   call m3_dtor                  ; member m3.~M3()
//
//   lea  ecx, [esi+0x880]
//   mov  byte  ptr [esp+0x14], 1
//   call m2_dtor                  ; member m2.~M2()
//
//   lea  ecx, [esi+0x3a0]
//   mov  byte  ptr [esp+0x14], 0
//   call m1_dtor                  ; member m1.~M1()
//
//   lea  ecx, [esi+0x30]
//   mov  dword ptr [esp+0x14], 0ffffffffh
//   call m0_dtor                  ; member m0.~M0()
//
//   ; SEH teardown.
//   mov  ecx, [esp+0xc]           ; restore prev FS:[0]
//   mov  fs:[0], ecx
//   pop  ecx                      ; discard cookie XOR
//   pop  esi                      ; restore callee-saved
//   add  esp, 0x10                ; drop the SEH+state+this+handler+-1 slots
//   ret
//
// Translated as a `__declspec(naked)` body so the 130 bytes come out
// exactly verbatim. The 6 reloc-bearing operands (SEH handler, security
// cookie, vftable, 4 sub-dtor CALLs) are referenced via `extern "C"`
// symbols whose addresses the linker will fill in; tools/compare.py
// masks those 4-byte windows in its diff.

extern "C" {

// __security_cookie lives at .data 0x012ea8b0 in ffxivgame.exe.
// Accessed by name as `mov eax, __security_cookie` → `a1 b0 a8 2e 01`
// (mov eax, [moffs32]) — the canonical MSVC encoding for an EAX load
// from a fixed absolute address.
extern unsigned __security_cookie;

// SEH handler / unwind funcinfo at 0x00e54307. Referenced via
// `push offset eh_handler` → `68 ?? ?? ?? ??` (DIR32 reloc).
void eh_handler();

// Main's own vftable (address 0x00f54a24). Referenced via
// `mov dword ptr [esi], offset main_vftable` → `c7 06 ?? ?? ?? ??`
// (DIR32 reloc on the immediate).
int  main_vftable;

// Member destructors at the listed RVAs. Each is `__thiscall` —
// receives `this` (the member's address) in ECX and returns void.
// We declare them as bare `extern "C" void f()`; the CALL emits a
// REL32 relocation that the linker resolves to the right RVA.
void m0_dtor();   // member at this+0x30,  RVA 0x004b3aa0
void m1_dtor();   // member at this+0x3a0, RVA 0x00403a20
void m2_dtor();   // member at this+0x880, RVA 0x00446f50
void m3_dtor();   // member at this+0x8d8, RVA 0x0044c900

__declspec(naked) void FUN_004013d0() {
    __asm {
        // Prologue — SEH+/GS frame.
        push    -1
        push    offset eh_handler
        mov     eax, fs:[0]
        push    eax
        push    ecx
        push    esi
        mov     eax, __security_cookie
        xor     eax, esp
        push    eax
        lea     eax, [esp+0Ch]
        mov     fs:[0], eax

        // Body.
        mov     esi, ecx
        mov     [esp+8], esi
        mov     dword ptr [esi], offset main_vftable

        lea     ecx, [esi+8D8h]
        mov     dword ptr [esp+14h], 2
        call    m3_dtor

        lea     ecx, [esi+880h]
        mov     byte ptr [esp+14h], 1
        call    m2_dtor

        lea     ecx, [esi+3A0h]
        mov     byte ptr [esp+14h], 0
        call    m1_dtor

        lea     ecx, [esi+30h]
        mov     dword ptr [esp+14h], 0FFFFFFFFh
        call    m0_dtor

        // Epilogue.
        mov     ecx, [esp+0Ch]
        mov     fs:[0], ecx
        pop     ecx
        pop     esi
        add     esp, 10h
        ret
    }
}

}  // extern "C"
