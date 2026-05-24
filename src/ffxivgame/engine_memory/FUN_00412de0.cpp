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
// FUNCTION: ffxivgame 0x00012de0 — SQEX::CDev::Engine::Memory::Alternative::Link
//           scalar-deleting-destructor (vtable-reset + doubly-linked-list unlink)
//
// Asm (29 bytes):
//   8b c1              MOV EAX, ECX              ; EAX = this (return value)
//   8b 48 04           MOV ECX, dword ptr [EAX+4]; ECX = this->prev  (hoisted before vtable write)
//   8b 50 08           MOV EDX, dword ptr [EAX+8]; EDX = this->next  (hoisted before vtable write)
//   c7 00 c4 67 f5 00  MOV dword ptr [EAX], Link::vftable  ; reset vtable (reloc)
//   89 51 08           MOV dword ptr [ECX+8], EDX; prev->next = next
//   8b 48 08           MOV ECX, dword ptr [EAX+8]; reload next (conservative alias after store)
//   8b 50 04           MOV EDX, dword ptr [EAX+4]; reload prev
//   89 51 04           MOV dword ptr [ECX+4], EDX; next->prev = prev
//   c2 04 00           RET 4                     ; __thiscall, pop 1 DWORD arg (delete flag)
//
// SQEX::CDev::Engine::Memory::Alternative::Link is an intrusive doubly-linked
// list node with:
//   offset +0: void*  vfptr  (vtable pointer)
//   offset +4: Link*  prev   (previous node)
//   offset +8: Link*  next   (next node)
//
// The scalar-deleting-destructor resets the vtable (virtual destructor ABI),
// then unlinks `this` from the list (prev->next = next; next->prev = prev).
// It does NOT free `this` regardless of the delete flag — Link objects are
// embedded in pool structures and freed by their owning allocator.
//
// MSVC 2005 /O2 codegen notes:
//   - MOV EAX, ECX is emitted first so the return value is established before
//     ECX is clobbered to hold `prev`/`next` as memory-access bases.
//   - The loads of prev and next are hoisted before the vtable store by the
//     instruction scheduler (no data dependency: [EAX+4/8] vs [EAX]).
//   - After `prev->next = next`, MSVC reloads both fields from `this` for the
//     second assignment (`next->prev = prev`): conservative alias analysis
//     cannot rule out that [ECX+8] aliases [EAX+4] or [EAX+8].
//
// RTTI: SQEX::CDev::Engine::Memory::Alternative::Link::vftable @ VA 0x00F567C4
//       (RVA 0x00B567C4, 2 vtable slots)

extern "C" void *Link_vftable;  // SQEX::CDev::Engine::Memory::Alternative::Link::vftable

struct Link_t {
    void  *vfptr;    // +0  vtable pointer
    Link_t *prev;   // +4  previous node in the intrusive list
    Link_t *next;   // +8  next node in the intrusive list

    void *scalar_dtor(unsigned int flags);
};

void *Link_t::scalar_dtor(unsigned int /*flags*/) {
    vfptr    = &Link_vftable;
    prev->next = next;
    next->prev = prev;
    return this;
}
