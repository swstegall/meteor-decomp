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
// FUNCTION: ffxivgame 0x00403a20 — __thiscall destructor with 12 subobject
// teardowns under an MSVC C++ SEH unwind frame (248 B).
//
// Behaviour reconstructed from the asm (Ghidra pseudo-C at
// build/ghidra-decomp/ffxivgame/00003a20_FUN_00403a20.c agrees on shape):
//
//   void __thiscall FUN_00403a20(SomeClass *this) {
//       try {
//           // state 10: composite member at [this+0x4d8]
//           FUN_0044c7f0(&this->member_4d8);
//           // states 9..0: array[10] of 0x54-byte members spanning
//           //             [this+0x48 .. this+0x33c], destructed in reverse
//           FUN_00446f50(&this->arr[9]);   // +0x33c
//           FUN_00446f50(&this->arr[8]);   // +0x2e8
//           FUN_00446f50(&this->arr[7]);   // +0x294
//           FUN_00446f50(&this->arr[6]);   // +0x240
//           FUN_00446f50(&this->arr[5]);   // +0x1ec
//           FUN_00446f50(&this->arr[4]);   // +0x198
//           FUN_00446f50(&this->arr[3]);   // +0x144
//           FUN_00446f50(&this->arr[2]);   // +0xf0
//           FUN_00446f50(&this->arr[1]);   // +0x9c
//           FUN_00446f50(&this->arr[0]);   // +0x48
//           // state -1: base subobject at [this+0]
//           FUN_00444200(this);
//       } catch (...) { /* funclet emitted separately by MSVC */ }
//   }
//
// The 0x54-byte stride between consecutive `FUN_00446f50` targets
// (0x33c, 0x2e8, 0x294, 0x240, 0x1ec, 0x198, 0x144, 0xf0, 0x9c, 0x48 —
// each 0x54 below the previous) makes the array layout obvious: 10
// identical 0x54-byte (84-byte) subobjects packed starting at +0x48.
// FUN_00446f50 is the per-element destructor (presumably non-virtual,
// per the `ecx = &member` direct setup and no vtable load).
//
// Calling convention: __thiscall (ECX = this; no `ret N` epilogue
// because the only stack args are SEH-frame data, not parameters).
//
// SEH frame layout after the prologue (relative to the final esp):
//   [esp + 0x00] = saved GS cookie (xor'd with original esp)
//   [esp + 0x04] = saved esi (registers preserved across SEH)
//   [esp + 0x08] = `this` spill (used by the EH handler to recover ECX)
//   [esp + 0x0c] = saved prev fs:[0] (links into SEH chain)
//   [esp + 0x10] = SEH handler RVA (push'd as 0x00e545f1)
//   [esp + 0x14] = unwind-state slot (-1 initial, then 0xa, 9, 8, ..., 0, -1)
//   [esp + 0x18] = return address
//
// The state slot tracks "which destructor is currently in flight" so
// that if FUN_004{4c7f0,46f50,44200} throws, the MSVC unwind dispatcher
// at 0xe545f1 knows which still-live subobjects need to be torn down.
// The first store uses `MOV DWORD` (clears all 4 bytes from the
// initial -1 to the constructor index 10); subsequent stores use
// `MOV BYTE` (the upper 3 bytes stay 0 once initialised); the final
// store before `FUN_00444200` uses `MOV DWORD` again to write all-ones
// for -1.
//
// Why naked __asm rather than a source-level destructor:
//
//   Reproducing this exact byte sequence from C++ source would require
//   matching MSVC 2005's EH state-machine codegen choices verbatim:
//   the SEH push order, the cookie register choice, the spill slot for
//   `this`, the per-state byte/dword mov mix, and the funclet
//   placement. All of those are determined by MSVC's internal funclet
//   generator and aren't directly controllable from source. A
//   `__declspec(naked)` body lets us pin every instruction encoding to
//   the orig (lea-with-disp32 vs disp8, mov-byte vs mov-dword, mov-eax
//   moffs32 short-form, IMM8 sign-extended push for -1) while the
//   relocs (handler push, security_cookie load, three call sites) are
//   masked by `tools/compare.py` in the byte-level diff.
//
// Reloc-bearing positions in the resulting .obj (all masked in the diff):
//
//   off 0x03   IMAGE_REL_I386_DIR32  → SEH handler stub (orig 0x00e545f1)
//   off 0x11   IMAGE_REL_I386_DIR32  → __security_cookie (orig 0x012ea8b0)
//   off 0x37   IMAGE_REL_I386_REL32  → FUN_0044c7f0
//   off 0x47   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 9, +0x33c)
//   off 0x57   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 8, +0x2e8)
//   off 0x67   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 7, +0x294)
//   off 0x77   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 6, +0x240)
//   off 0x87   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 5, +0x1ec)
//   off 0x97   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 4, +0x198)
//   off 0xa7   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 3, +0x144)
//   off 0xb7   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 2, +0xf0)
//   off 0xc7   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 1, +0x9c)
//   off 0xd4   IMAGE_REL_I386_REL32  → FUN_00446f50 (state 0, +0x48)
//   off 0xe3   IMAGE_REL_I386_REL32  → FUN_00444200

extern "C" {

// SEH handler stub trampoline at 0x00e545f1 in orig. Provides a unique
// symbol for the DIR32 reloc; the actual bytes at this position are
// masked out of the diff.
int FUN_00e545f1();

// Subobject destructors (all __thiscall — receive their object pointer
// in ECX from the LEA instructions; declarations here are just to
// satisfy the inline-asm `call <name>` REL32 reloc).
int FUN_0044c7f0();   // composite member at +0x4d8
int FUN_00446f50();   // per-element destructor for 0x54-byte members
int FUN_00444200();   // base-subobject destructor at +0

extern unsigned __security_cookie;

} // extern "C"

extern "C" __declspec(naked) void FUN_00403a20() {
    __asm {
        // --- SEH / GS prologue --------------------------------------
        push    -1                            ; initial unwind state (top-of-frame)
        push    offset FUN_00e545f1           ; SEH handler trampoline (DIR32 reloc)
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

        // state 10: destruct composite member at +0x4d8
        lea     ecx, [esi + 0x4d8]
        mov     dword ptr [esp + 0x14], 0xa   ; full-dword init clears the initial -1
        call    FUN_0044c7f0

        // states 9..0: destruct array[10] of 0x54-byte members in reverse
        lea     ecx, [esi + 0x33c]
        mov     byte ptr [esp + 0x14], 9
        call    FUN_00446f50

        lea     ecx, [esi + 0x2e8]
        mov     byte ptr [esp + 0x14], 8
        call    FUN_00446f50

        lea     ecx, [esi + 0x294]
        mov     byte ptr [esp + 0x14], 7
        call    FUN_00446f50

        lea     ecx, [esi + 0x240]
        mov     byte ptr [esp + 0x14], 6
        call    FUN_00446f50

        lea     ecx, [esi + 0x1ec]
        mov     byte ptr [esp + 0x14], 5
        call    FUN_00446f50

        lea     ecx, [esi + 0x198]
        mov     byte ptr [esp + 0x14], 4
        call    FUN_00446f50

        lea     ecx, [esi + 0x144]
        mov     byte ptr [esp + 0x14], 3
        call    FUN_00446f50

        lea     ecx, [esi + 0xf0]
        mov     byte ptr [esp + 0x14], 2
        call    FUN_00446f50

        lea     ecx, [esi + 0x9c]
        mov     byte ptr [esp + 0x14], 1
        call    FUN_00446f50

        lea     ecx, [esi + 0x48]             ; disp8 form (positive 0x48 fits in signed byte)
        mov     byte ptr [esp + 0x14], 0
        call    FUN_00446f50

        // state -1: destruct base subobject at this+0
        mov     ecx, esi
        mov     dword ptr [esp + 0x14], -1    ; full-dword write to set all ones
        call    FUN_00444200

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
