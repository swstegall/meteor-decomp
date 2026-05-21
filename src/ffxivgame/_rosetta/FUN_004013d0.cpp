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
// FUNCTION: ffxivgame 0x004013d0 — __thiscall destructor with vtable
// reset + 4 subobject teardowns under an MSVC C++ SEH unwind frame (130 B).
//
// Behaviour reconstructed from the asm (Ghidra pseudo-C at
// build/ghidra-decomp/ffxivgame/000013d0_FUN_004013d0.c agrees on shape):
//
//   void __thiscall FUN_004013d0(Main *this) {
//       // Reset vtable to most-derived "Main" vftable as we enter the
//       // destructor body — standard MSVC layout for a class with
//       // sub-object destructors that may run virtual calls during
//       // teardown.
//       this->vftable = &Main::vftable;     // [this+0] = 0x00f54a24
//
//       try {
//           // state 2: composite member at [this+0x8d8]
//           FUN_0044c900(&this->member_8d8);
//           // state 1: composite member at [this+0x880]
//           FUN_00446f50(&this->member_880);
//           // state 0: composite member at [this+0x3a0]
//           FUN_00403a20(&this->member_3a0);
//           // state -1: base subobject at [this+0x30]
//           FUN_004b3aa0(&this->member_30);
//       } catch (...) { /* funclet emitted separately by MSVC */ }
//   }
//
// The structure mirrors FUN_00403a20 (the +0x3a0 sub-destructor that
// this very function chains into one frame down) — same SEH-prologue
// shape, same `[esp + 0x14]` state slot, same dword/byte/byte/dword
// mov mix on the state writes (full-dword first to clear the initial
// -1, byte mov's for in-range positive states, final full-dword to
// write all-ones for state -1). The only differences from FUN_00403a20:
//
//   1. Adds a `MOV dword ptr [esi], 0x00f54a24` vtable-reset right
//      after spilling `this` (offset 0x28 in the body) — this is the
//      `c7 06 24 4a f5 00` byte sequence, the standard
//      mov-mem-imm32-with-[esi]-base form.
//   2. Only 4 subobject teardowns (states 2..-1) instead of 12
//      (states 10..-1).
//   3. The final LEA uses disp8 form (`8d 4e 30`) because +0x30 fits
//      in a signed byte; the other three use disp32 (`8d 8e XX XX XX
//      XX`) because their displacements exceed 0x7f.
//
// Calling convention: __thiscall (ECX = this; no `ret N` epilogue
// because the only stack args are SEH-frame data, not parameters).
//
// SEH frame layout after the prologue (relative to the final esp):
//   [esp + 0x00] = saved GS cookie (xor'd with original esp)
//   [esp + 0x04] = saved esi (preserved register)
//   [esp + 0x08] = `this` spill (used by the EH handler to recover ECX)
//   [esp + 0x0c] = saved prev fs:[0] (links into SEH chain)
//   [esp + 0x10] = SEH handler RVA (push'd as 0x00e54307)
//   [esp + 0x14] = unwind-state slot (-1 initial, then 2, 1, 0, -1)
//   [esp + 0x18] = return address
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//
//   off 0x03   IMAGE_REL_I386_DIR32  → SEH handler stub (orig 0x00e54307)
//   off 0x11   IMAGE_REL_I386_DIR32  → __security_cookie (orig 0x012ea8b0)
//   off 0x2a   IMAGE_REL_I386_DIR32  → Main::vftable (orig 0x00f54a24)
//   off 0x3d   IMAGE_REL_I386_REL32  → FUN_0044c900 (state 2, +0x8d8)
//   off 0x4d   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 1, +0x880)
//   off 0x5d   IMAGE_REL_I386_REL32  → FUN_00403a20 (state 0, +0x3a0)
//   off 0x6d   IMAGE_REL_I386_REL32  → FUN_004b3aa0 (state -1, +0x30)

extern "C" {

// SEH handler stub trampoline at 0x00e54307 in orig. Provides a unique
// symbol for the DIR32 reloc; the actual bytes at this position are
// masked out of the diff.
int FUN_00e54307();

// Subobject destructors (all __thiscall — receive their object pointer
// in ECX from the LEA instructions; declarations here are just to
// satisfy the inline-asm `call <name>` REL32 reloc).
int FUN_0044c900();   // composite member at +0x8d8
int FUN_00446f50();   // composite member at +0x880
int FUN_00403a20();   // composite member at +0x3a0 (the sibling destructor)
int FUN_004b3aa0();   // base subobject at +0x30

// Most-derived vftable address (DIR32 reloc — orig 0x00f54a24). Just a
// unique symbol for the relocation; bytes are masked.
extern int Main_vftable;

extern unsigned __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_004013d0() {
    __asm {
        // --- SEH / GS prologue --------------------------------------
        push    -1                            ; initial unwind state (top-of-frame)
        push    offset FUN_00e54307           ; SEH handler trampoline (DIR32 reloc)
        mov     eax, dword ptr fs:[0]
        push    eax                           ; prev fs:[0] → SEH chain link
        push    ecx                           ; scratch slot (overwritten below with `this`)
        push    esi                           ; preserved register
        mov     eax, __security_cookie        ; load GS cookie (DIR32 reloc, `a1` form)
        xor     eax, esp                      ; anchor cookie to current esp
        push    eax                           ; spill cookie for epilogue check
        lea     eax, [esp + 0xc]              ; eax = &prev_fs0 slot
        mov     dword ptr fs:[0], eax         ; install our SEH registration

        // --- Body ---------------------------------------------------
        mov     esi, ecx                      ; esi = this
        mov     dword ptr [esp + 8], esi      ; spill `this` into the scratch slot
                                              ; (EH handler reads it back from there)

        // Reset vtable to most-derived (Main::vftable) before tearing
        // down subobjects.
        mov     dword ptr [esi], offset Main_vftable

        // state 2: destruct composite member at +0x8d8
        lea     ecx, [esi + 0x8d8]
        mov     dword ptr [esp + 0x14], 2     ; full-dword init clears the initial -1
        call    FUN_0044c900

        // state 1: destruct composite member at +0x880
        lea     ecx, [esi + 0x880]
        mov     byte ptr [esp + 0x14], 1
        call    FUN_00446f50

        // state 0: destruct composite member at +0x3a0
        lea     ecx, [esi + 0x3a0]
        mov     byte ptr [esp + 0x14], 0
        call    FUN_00403a20

        // state -1: destruct base subobject at +0x30
        lea     ecx, [esi + 0x30]             ; disp8 form (positive 0x30 fits in signed byte)
        mov     dword ptr [esp + 0x14], -1    ; full-dword write to set all ones
        call    FUN_004b3aa0

        // --- SEH teardown / epilogue --------------------------------
        mov     ecx, dword ptr [esp + 0xc]    ; ecx = saved prev fs:[0]
        mov     dword ptr fs:[0], ecx         ; restore SEH chain
        pop     ecx                           ; drop cookie
        pop     esi                           ; restore preserved register
        add     esp, 0x10                     ; drop spill, prev fs:[0], handler, state
        ret
    }
}

// vim: ts=4 sts=4 sw=4 et
