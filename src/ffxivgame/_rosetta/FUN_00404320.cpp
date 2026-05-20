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
// FUNCTION: ffxivgame 0x00404320 — std::logic_error::logic_error(const std::string &)
// (or a thin __thiscall ctor with that shape) — 107 bytes.
//
// Shape (reconstructed from the asm):
//
//   logic_error::logic_error(const string *msg /* on stack */) : exception() {
//       // Inherited std::exception default ctor (FUN_009d18c9):
//       //   [this+0]   = std::exception::vftable
//       //   [this+4]   = 0      (_what)
//       //   [this+8]   = 0      (_DoFree)
//       // Override vtable with the logic_error variant:
//       *(void**)this = &logic_error::vftable;     // 0x00f54a2c
//       // Inline-initialise the embedded `basic_string _Mywhat` at offset 0xC
//       // to an empty SSO string (capacity = 0xF, size = 0, buf[0] = '\0'):
//       this->_Mywhat._Mysize = 0;                 // [this+0x20]
//       this->_Mywhat._Myres  = 0xF;               // [this+0x24]
//       this->_Mywhat._Bx._Buf[0] = '\0';          // [this+0x10]
//       // Copy the source string in via the assign-from-substring overload:
//       this->_Mywhat.assign(*msg, 0, npos);       // FUN_00404040
//   }
//
// Calling convention: __thiscall. `this` arrives in ECX, the single
// stack argument `msg` (const string *) sits at [esp+4] on entry, and
// the function cleans it with `ret 4`. The function emits a synchronous-
// exception SEH frame (state -1 → 0) because the embedded string's
// destructor needs to run if `assign()` throws std::length_error mid-
// construction (npos guard inside FUN_00404040). The scope table /
// frame-handler thunk lives at orig 0x00e54638.
//
// Naked __asm so the SEH prolog (state slot, scope-table reloc,
// fs:[0] chain, __security_cookie ^ esp), the per-arg stack offsets, and
// the in-line basic_string initialisation are pinned to the orig
// encoding. The five reloc-bearing positions in the resulting .obj are
// all masked out of the byte diff by tools/compare.py:
//
//   off 0x03  IMAGE_REL_I386_DIR32  → FUN_00e54638        (SEH handler)
//   off 0x11  IMAGE_REL_I386_DIR32  → __security_cookie
//   off 0x29  IMAGE_REL_I386_REL32  → FUN_009d18c9        (exception::exception)
//   off 0x33  IMAGE_REL_I386_DIR32  → logic_error_vftable (0x00f54a2c)
//   off 0x52  IMAGE_REL_I386_REL32  → FUN_00404040        (string::assign)

extern "C" {

// SEH handler thunk at orig 0x00e54638 — the per-function trampoline that
// loads the CXX func-info pointer (0x011a9064) and tail-jumps into
// __CxxFrameHandler3 (FUN_009d1db6). Declared as a function so MSVC inline
// asm accepts `push offset FUN_00e54638` and the linker emits a DIR32
// reloc; the byte at this position is masked in objdiff's diff.
int FUN_00e54638();

// std::exception::exception(void) — __thiscall, no args, sets the default
// std::exception::vftable (orig 0x01085CC8) and zeros _what / _DoFree.
void FUN_009d18c9();

// std::basic_string<char>::assign(const basic_string &, size_type off,
// size_type count) — __thiscall with three stack args, `ret 0xC`. Pulled
// in by the in-place copy from the source `msg` into the embedded
// _Mywhat after the SSO fields are zeroed.
void FUN_00404040();

// std::logic_error::vftable at orig 0x00f54a2c. Declared as `int` so the
// inline `mov dword ptr [esi], offset logic_error_vftable` produces a
// DIR32 reloc to the vftable address — the value is set by the linker
// and masked out of the byte diff.
int logic_error_vftable;

extern unsigned int __security_cookie;

}  // extern "C"

extern "C" __declspec(naked) void FUN_00404320() {
    __asm {
        // --- SEH / GS prologue --------------------------------------
        push    -1                              ; SEH state slot (initial = -1, "no destructor live")
        push    offset FUN_00e54638             ; scope-table / handler ptr (DIR32 reloc)
        mov     eax, dword ptr fs:[0]
        push    eax                             ; saved prev fs:[0]
        push    ecx                             ; reserve a `this` spill slot
        push    esi
        mov     eax, dword ptr [__security_cookie]
        xor     eax, esp
        push    eax                             ; cookie ^ esp (overflow guard)
        lea     eax, [esp + 0xC]                ; addr of "saved prev fs:[0]" field
        mov     dword ptr fs:[0], eax           ; install SEH registration
        mov     esi, ecx                        ; esi = this
        mov     dword ptr [esp + 8], esi        ; spill `this` for the unwind/destructor

        // --- Body ---------------------------------------------------
        call    FUN_009d18c9                    ; std::exception::exception(this)
        xor     eax, eax                        ; eax = 0 (reused as _Mysize / SSO-byte / SEH-state value)
        lea     ecx, [esi + 0xC]                ; ecx = &this->_Mywhat (embedded basic_string)
        mov     dword ptr [esi], offset logic_error_vftable   ; overwrite vftable

        // FUN_00404040 takes (basic_string &src, size_t off, size_t count).
        // Push count = npos first.
        push    -1                              ; count = (size_t)-1 (npos)
        mov     dword ptr [ecx + 0x14], eax     ; _Mywhat._Mysize = 0
        mov     dword ptr [ecx + 0x18], 0xF     ; _Mywhat._Myres  = 0xF
        push    eax                             ; off = 0
        mov     dword ptr [esp + 0x1C], eax     ; SEH state := 0 (now in "_Mywhat destructor live" region)
        mov     byte ptr [ecx + 4], al          ; _Mywhat._Bx._Buf[0] = '\0'
        mov     eax, dword ptr [esp + 0x24]     ; eax = caller's msg argument
        push    eax                             ; src = msg
        call    FUN_00404040                    ; this->_Mywhat.assign(*msg, 0, npos)
        mov     eax, esi                        ; return this

        // --- SEH teardown / epilogue --------------------------------
        mov     ecx, dword ptr [esp + 0xC]      ; ecx = saved prev fs:[0]
        mov     dword ptr fs:[0], ecx
        pop     ecx                             ; cookie
        pop     esi
        add     esp, 0x10                       ; discard spill slot + saved prev fs:[0] + scope + state
        ret     4
    }
}

// vim: ts=4 sts=4 sw=4 et
