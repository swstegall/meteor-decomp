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
// FUNCTION: ffxivgame 0x00013ab0 — thread-safe block-recycle helper:
//           drain inner state, push freed block into a spinlocked free-list,
//           then notify chained sibling.  (__thiscall, 1 stack arg, void,
//           RET 4)
//
// Asm (189 bytes @ orig RVA 0x00013ab0):
//
//   Prologue:  PUSH EBX / PUSH EBP / PUSH ESI / PUSH EDI
//   EDI = this                       (callee-saved alias)
//   EBP = this->field_0x10           (vtable-bearing inner / "owner")
//   Call EBP->vtable[0x2c/4]()       (begin/lock owner, slot 11)
//   ECX = param_1 ([ESP+0x14])
//   Call param_1->vtable[0x14/4]()   → EAX  (slot 5: obtain proxy)
//   ECX = EAX
//   Call EAX->vtable[0x4/4]()        → EAX  (slot 1: obtain block)
//   ESI = EAX                        (the block / queue head)
//   if (ESI->field_0x30 != 0) {
//       EBX = &ESI->field_4          (inner sub-object base)
//       do { call (*EBX)->vtable[0x20/4](EBX);  // slot 8 drain
//       } while (ESI->field_0x30 != 0);
//   }
//   Stash ESI->field_0x1c into [ESP+0x14]  (overwrite first stack arg)
//   Call ESI->vtable[0](0)           (slot 0 dtor-like w/ deleting flag = 0)
//   Call this->field_0x10->vtable[0x4/4]()  → EAX (slot 1: fetch list owner)
//   ECX = EAX->field_0x14            (inner list / spinlock node)
//   EDX = &ECX->field_4              (spinlock address)
//   Spin loop:
//     EAX = 1 ; EBX = EDX ; XCHG [EBX], EAX ; TEST EAX,EAX ; JNZ loop
//   ECX2 = ECX->field_0xc            (list sentinel / head)
//   EBX  = ECX2->field_4             (sentinel->next)
//   Insert ESI between sentinel and old-next (push_front):
//     [EBX]     = ESI
//     EBX       = ECX2->field_4      (re-read; MSVC didn't hoist)
//     ESI[4]    = EBX
//     [ESI]     = ECX2
//     ECX2[4]   = ESI
//   ECX->field_0x18 -= 1             (free-slot count decrement)
//   ECX2 = 0 ; XCHG [EDX], ECX2      (release spinlock; reg = ECX scratched)
//   Walk this->field_0x28 chain via field_0x2c "next":
//     EAX = this->field_0x28 ; ECX = this - 4
//     if EAX != 0 do { ECX = EAX ; EAX = ECX->field_0x2c } while EAX != 0
//   Call (*ECX->field_0x1c)->vtable[0x28/4](stashed_value)  (slot 10)
//   Call EBP->vtable[0x30/4]()       (end/unlock owner, slot 12)
//   Epilogue: POP EDI / POP ESI / POP EBP / POP EBX / RET 4
//
// Register notes:
//   * EDI holds `this` from prologue to epilogue.
//   * EBP holds this->field_0x10 across the whole body (used for first
//     and last vtable calls), preserved through every interior call.
//   * ESI holds the recycled "block" / queue head from the first
//     proxy-fetch call all the way through the doubly-linked insert.
//   * The "this - 4" idiom (LEA ECX,[EDI-4]) seeds the chain-walk ECX
//     with a value that is overwritten by the loop body as soon as
//     EAX != 0; if EAX == 0 the LEA value is what we use, meaning the
//     walk's starting sentinel sits one slot before `this` (the prev
//     node in a doubly-linked owner chain).
//   * MSVC reloads EBX from ECX2->field_4 twice (instead of caching the
//     first read in another reg) — preserve that to match byte-exact.
//
// Mirrors FUN_00411f30 (push_front into the same free-list shape) but
// adds the proxy-fetch + drain prelude and the chain-walk notify
// postlude.  Same spinlock pattern, same insert sequence.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" void FUN_00413ab0() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00413ab0()
{
    __asm {
        // --- prologue ---
        push    ebx
        push    ebp
        push    esi
        push    edi
        mov     edi, ecx                    // EDI = this
        mov     ebp, dword ptr [edi + 0x10] // EBP = this->field_0x10
        // --- call EBP->vtable[0x2c/4]() ---
        mov     eax, dword ptr [ebp + 0]
        mov     edx, dword ptr [eax + 0x2c]
        mov     ecx, ebp
        call    edx
        // --- call param_1->vtable[0x14/4]() ---
        mov     ecx, dword ptr [esp + 0x14]
        mov     eax, dword ptr [ecx]
        mov     edx, dword ptr [eax + 0x14]
        call    edx
        // --- call retval->vtable[0x4/4]() ---
        mov     edx, dword ptr [eax]
        mov     ecx, eax
        mov     eax, dword ptr [edx + 0x4]
        call    eax
        // --- ESI = block ---
        mov     esi, eax
        // --- if (ESI->field_0x30 != 0) drain loop ---
        cmp     dword ptr [esi + 0x30], 0
        je      _post_drain
        lea     ebx, [esi + 0x4]
    _drain:
        mov     edx, dword ptr [ebx]
        mov     eax, dword ptr [edx + 0x20]
        mov     ecx, ebx
        call    eax
        cmp     dword ptr [esi + 0x30], 0
        jne     _drain
    _post_drain:
        // --- stash ESI->field_0x1c, call ESI->vtable[0](0) ---
        mov     ecx, dword ptr [esi + 0x1c]
        mov     edx, dword ptr [esi]
        mov     eax, dword ptr [edx]
        mov     dword ptr [esp + 0x14], ecx
        push    0
        mov     ecx, esi
        call    eax
        // --- call this->field_0x10->vtable[0x4/4]() ---
        mov     ecx, dword ptr [edi + 0x10]
        mov     edx, dword ptr [ecx]
        mov     eax, dword ptr [edx + 0x4]
        call    eax
        // --- ECX = EAX->field_0x14 ; EDX = &ECX->field_4 ---
        mov     ecx, dword ptr [eax + 0x14]
        lea     edx, [ecx + 0x4]
        // --- spinlock acquire loop ---
    _spin:
        mov     eax, 1
        mov     ebx, edx
        xchg    dword ptr [ebx], eax
        test    eax, eax
        jne     _spin
        // --- doubly-linked push_front ---
        mov     eax, dword ptr [ecx + 0xc]
        mov     ebx, dword ptr [eax + 0x4]
        mov     dword ptr [ebx], esi
        mov     ebx, dword ptr [eax + 0x4]
        mov     dword ptr [esi + 0x4], ebx
        mov     dword ptr [esi], eax
        mov     dword ptr [eax + 0x4], esi
        // --- decrement free-slot count ---
        add     dword ptr [ecx + 0x18], -1
        // --- release spinlock ---
        xor     ecx, ecx
        xchg    dword ptr [edx], ecx
        // --- walk this->field_0x28 chain (next at +0x2c) ---
        mov     eax, dword ptr [edi + 0x28]
        test    eax, eax
        lea     ecx, [edi - 0x4]
        je      _walk_done
    _walk:
        mov     ecx, eax
        mov     eax, dword ptr [ecx + 0x2c]
        test    eax, eax
        jne     _walk
    _walk_done:
        // --- call (*ECX->field_0x1c)->vtable[0x28/4](stashed) ---
        mov     ecx, dword ptr [ecx + 0x1c]
        mov     edx, dword ptr [ecx]
        mov     eax, dword ptr [esp + 0x14]
        mov     edx, dword ptr [edx + 0x28]
        push    eax
        call    edx
        // --- call EBP->vtable[0x30/4]() ---
        mov     eax, dword ptr [ebp + 0]
        mov     edx, dword ptr [eax + 0x30]
        mov     ecx, ebp
        call    edx
        // --- epilogue ---
        pop     edi
        pop     esi
        pop     ebp
        pop     ebx
        ret     4
    }
}
#endif
