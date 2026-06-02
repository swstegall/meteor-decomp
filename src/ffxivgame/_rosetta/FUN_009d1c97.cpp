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
// FUNCTION: ffxivgame 0x009d1c97 — local EH-unwind thunk for
//                                  eh_vector_destructor_iterator (24 B)
//
// This naked helper lives at the address registered as the EH cleanup
// callback for `eh_vector_destructor_iterator` (VA 0x009d1c4c). It is
// called both:
//   (a) at the end of the normal destructor loop (CALL @ VA 0x009d1c8a),
//       at which point [EBP-0x1C] has already been set to 1 so the
//       function returns immediately; and
//   (b) (conceptually) via the EH frame during unwinding, when the
//       destructor call at `CALL [EBP+0x14]` threw — [EBP-0x1C] remains
//       0 and the function forwards all four iterator args to __ArrayUnwind
//       to destroy the elements the partial loop left alive.
//
// Because the function runs inside the caller's live EBP frame (no
// prologue of its own), it accesses the parent's arguments and locals
// directly through EBP-relative addressing.
//
// Asm (24 bytes @ 0x009d1c97):
//   83 7d e4 00          CMP  dword ptr [EBP-0x1C], 0
//   75 11                JNZ  +0x11 → RET        ; completed normally
//   ff 75 14             PUSH dword ptr [EBP+0x14] ; destructor ptr
//   ff 75 10             PUSH dword ptr [EBP+0x10] ; element count
//   ff 75 0c             PUSH dword ptr [EBP+0x0C] ; element size
//   ff 75 08             PUSH dword ptr [EBP+0x08] ; array base ptr
//   e8 RR RR RR RR       CALL __ArrayUnwind         ; rel32 reloc → VA 0x009d1bee
//   c3                   RET

extern "C" void __ArrayUnwind(void *ptr, unsigned int sz, int count, void *dtor);

extern "C" __declspec(naked) void FUN_009d1c97() {
    __asm {
        cmp  dword ptr [ebp-0x1c], 0
        jnz  done
        push dword ptr [ebp+0x14]
        push dword ptr [ebp+0x10]
        push dword ptr [ebp+0x0c]
        push dword ptr [ebp+0x08]
        call __ArrayUnwind
    done:
        ret
    }
}
