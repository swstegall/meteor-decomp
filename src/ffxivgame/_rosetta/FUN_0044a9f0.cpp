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
// FUNCTION: ffxivgame 0x0004a9f0 — __thiscall constructor / initialiser
//           (126 B / 0x7e). Sets the vtable ptr, constructs a capacity-5
//           sub-object at [this+4], then initialises a self-referential
//           sentinel-node structure at [this+0x20..0x28].
//
// Calling convention: __thiscall (ECX = this; no stack args cleaned;
//   plain RET). Returns EAX = this.
//
// SEH / GS frame layout after prologue (ESP-relative):
//   [esp + 0x00] = GS cookie ^ esp-at-push
//   [esp + 0x04] = saved EDI
//   [esp + 0x08] = saved ESI
//   [esp + 0x0C] = ECX (this) spill  ← used by EH handler for recovery
//   [esp + 0x10] = prev FS:[0]       ← SEH chain "next" pointer
//   [esp + 0x14] = SEH handler (0x00e5859b)
//   [esp + 0x18] = SEH state slot    (-1 initially; set to 0 on try-entry)
//   [esp + 0x1C] = return address
//
// Body:
//   1. Write vtable ptr 0x00f672a8 to this[0].
//   2. __thiscall-call FUN_004588e0(5) on [this+4] — constructs a
//      capacity-5 sub-object (vector/list).
//   3. Enter SEH try (state 0). __thiscall-call FUN_0095e590 on [this+0x20]
//      — returns a new node pointer in EAX.
//   4. Store node at [this+0x24]; set node[0x15] = 1 (byte). Wire up
//      a three-way circular self-reference: node[0] = node[4] = node[8]
//      = node itself.
//   5. Zero [this+0x28].
//
// Reloc-bearing sites (all masked by compare.py):
//   off 0x03  DIR32 → SEH handler stub (orig VA 0x00e5859b)
//   off 0x09  FS:[0] moffs32 (special FS-prefixed encoding, addr field = 0)
//   off 0x12  DIR32 → __security_cookie (orig VA 0x012ea8b0)
//   off 0x1a  DIR32 → vtable (orig VA 0x00f672a8)
//   off 0x20  REL32 → FUN_004588e0
//   off 0x32  REL32 → FUN_0095e590

extern "C" {

// SEH handler trampoline at VA 0x00e5859b — unique symbol for the DIR32
// reloc; bytes at that offset are masked in the diff.
int FUN_00e5859b();

// Sub-object constructor called on [this+4] (__thiscall, receives ECX and
// one stack arg = 5).
int FUN_004588e0();

// Node factory / sentinel-init routine called on [this+0x20] (__thiscall).
int FUN_0095e590();

// Vtable at VA 0x00f672a8 — DIR32 reloc only; actual bytes masked.
extern int vtable_0f672a8;

extern unsigned __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_0044a9f0() {
    __asm {
        // --- SEH / GS prologue ------------------------------------------
        push    -1                             // SEH state = -1
        push    offset FUN_00e5859b            // SEH handler (DIR32 reloc)
        mov     eax, dword ptr fs:[0]
        push    eax                            // prev FS:[0]
        push    ecx                            // scratch / this-spill slot
        push    esi
        push    edi
        mov     eax, __security_cookie         // GS cookie (DIR32 reloc, a1 form)
        xor     eax, esp
        push    eax                            // cookie ^ esp
        lea     eax, [esp + 0x10]             // &prev_fs0 = start of SEH record
        mov     dword ptr fs:[0], eax          // install SEH frame

        // --- Body -------------------------------------------------------
        mov     edi, ecx                       // EDI = this
        mov     dword ptr [esp + 0xc], edi     // spill this (EH recovery slot)

        push    5
        lea     ecx, [edi + 4]
        mov     dword ptr [edi], offset vtable_0f672a8  // this[0] = vtable
        call    FUN_004588e0                   // construct sub-object at this+4

        lea     esi, [edi + 0x20]
        mov     ecx, esi
        mov     dword ptr [esp + 0x18], 0      // SEH state = 0 (enter try)
        call    FUN_0095e590                   // init / allocate node; EAX = node ptr

        mov     dword ptr [esi + 4], eax       // this+0x24 = node
        mov     byte ptr [eax + 0x15], 1       // node[0x15] = 1
        mov     eax, dword ptr [esi + 4]
        mov     dword ptr [eax + 4], eax       // node[4] = node
        mov     eax, dword ptr [esi + 4]
        mov     dword ptr [eax], eax           // node[0] = node
        mov     eax, dword ptr [esi + 4]
        mov     dword ptr [eax + 8], eax       // node[8] = node
        mov     dword ptr [esi + 8], 0         // this+0x28 = 0

        // --- Epilogue ---------------------------------------------------
        mov     eax, edi                       // return this
        mov     ecx, dword ptr [esp + 0x10]   // reload prev FS:[0]
        mov     dword ptr fs:[0], ecx          // restore SEH chain
        pop     ecx                            // discard GS cookie
        pop     edi
        pop     esi
        add     esp, 0x10                      // drop: this-spill, prev_fs0, handler, state
        ret
    }
}
