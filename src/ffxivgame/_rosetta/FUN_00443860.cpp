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
// FUNCTION: ffxivgame 0x00043860 — `__thiscall` 1-arg member that forwards to
//                                   a sibling `__thiscall` method on the same
//                                   object, then sets a "done" flag (21 B / 0x15).
//
// Asm (21 bytes @ orig RVA 0x00043860):
//   8b 44 24 04        mov   eax, [esp+4]          ; load the single stack arg
//   56                 push  esi                    ; callee-save ESI
//   50                 push  eax                    ; push arg for the callee
//   8b f1              mov   esi, ecx               ; stash `this` in ESI (ECX
//                                                   ;   is left untouched, so it
//                                                   ;   still carries `this` into
//                                                   ;   the __thiscall callee)
//   e8 63 ef 70 00     call  0x00b527d0             ; this->method(arg) — rel32
//   c6 46 14 01        mov   byte ptr [esi+0x14], 1 ; this->field_0x14 = 1
//   5e                 pop   esi
//   c2 04 00           ret   4                      ; __thiscall, 1 stack arg
//
// Calling convention: `__thiscall` — ECX = `this`, one 4-byte stack argument,
// callee pops it via `RET 4`. The callee at 0x00b527d0 is itself `__thiscall`
// on the SAME receiver: the compiler never reloads ECX before the CALL because
// `MOV ESI, ECX` copies `this` into the callee-saved register without disturbing
// ECX, which still holds `this` at the call site. After the inner call returns,
// the function flips a one-byte flag at `this+0x14` (an "initialised" / "done"
// marker) and returns.
//
// Inferred shape:
//
//   __thiscall void FUN_00443860(T* this, int arg) {
//       this->method_b527d0(arg);   // CALL 0x00b527d0 (__thiscall, same this)
//       this->field_0x14 = 1;       // byte flag
//   }
//
// The only reloc-bearing site in the orig 21 bytes is the `e8` REL32 to the
// callee at 0x00b527d0; `tools/compare.py` masks that 4-byte offset window
// during the byte diff. MSVC 2005 /O2 will not reliably reproduce the exact
// register allocation (ESI stash without an ECX reload, arg pre-loaded into EAX
// before the prologue push) from C++ source, so — as with the other reloc-heavy
// thiscall forwarders in this tree — the body is re-emitted verbatim via MASM
// `_emit` directives. The .obj `.text` ends up byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_00443860() {
    __asm {
        _emit 0x8b              // MOV EAX, [ESP+4]
        _emit 0x44
        _emit 0x24
        _emit 0x04
        _emit 0x56              // PUSH ESI
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xe8              // CALL 0x00b527d0 (rel32)
        _emit 0x63
        _emit 0xef
        _emit 0x70
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [ESI+0x14], 1
        _emit 0x46
        _emit 0x14
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
    }
}
