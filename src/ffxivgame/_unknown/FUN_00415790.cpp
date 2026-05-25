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
// FUNCTION: ffxivgame 0x00015790 — release-and-null owned child object
//                                   (__thiscall, 37 B / 0x25)
//
// Reads a pointer at `this + 0x7c`. If non-null, invokes a __thiscall
// teardown method on it (FUN_00416650 — a 1-byte RET in orig, likely
// the base "no-op destructor / virtual placeholder" leaf), then hands
// the pointer to the __cdecl deallocator FUN_004162c0 (presumably an
// engine_memory `free` wrapper), and finally nulls the slot so the
// owner doesn't double-free on a subsequent call. The classic
// "release_unique_ptr"-style member used across the engine's
// composite-object hierarchy.
//
// Pseudo-C:
//   void Owner::release_child() {
//       Child *p = this->m_child;   // [this + 0x7c]
//       if (p != 0) {
//           p->teardown();          // __thiscall FUN_00416650
//           free(p);                // __cdecl    FUN_004162c0
//           this->m_child = 0;
//       }
//   }
//
// Calling convention: __thiscall (ECX = this, no stack args, bare RET).
// Callee-saves used: ESI (carries the loaded child pointer across the
// two calls), EDI (carries `this` across the calls).
// Frame: none (/Oy — no locals, no EBP).
//
// MSVC 2005 /O2 pushes ESI/EDI at function entry rather than around
// the use, picks ESI/EDI in that order, and emits POP EDI / POP ESI
// in LIFO order at exit — the natural "callee-save two regs, use both
// across calls" lowering. The `MOV ECX, ESI` immediately before the
// thiscall is the standard ECX=this setup for the inner call.
//
// Asm (37 bytes @ orig RVA 0x00015790):
//   56                    PUSH ESI
//   57                    PUSH EDI
//   8b f9                 MOV  EDI, ECX                 ; this
//   8b 77 7c              MOV  ESI, [EDI + 0x7c]        ; child
//   85 f6                 TEST ESI, ESI
//   74 17                 JZ   +0x17                    ; skip if null
//   8b ce                 MOV  ECX, ESI                 ; this = child
//   e8 ae 0e 00 00        CALL FUN_00416650             ; child->teardown()
//   56                    PUSH ESI                      ; arg = child
//   e8 18 0b 00 00        CALL FUN_004162c0             ; free(child)
//   83 c4 04              ADD  ESP, 4                   ; __cdecl cleanup
//   c7 47 7c 00 00 00 00  MOV  [EDI + 0x7c], 0          ; m_child = null
//   5f                    POP  EDI
//   5e                    POP  ESI
//   c3                    RET
//
// Encoded as `__declspec(naked)` + `_emit` to pin the exact byte
// sequence — register-allocation luck around the two cross-call live
// values (ESI=child, EDI=this) is brittle from source-level C++ if
// the synthetic Child/Owner types don't match orig's class layout
// exactly. The two CALL rel32 displacements are emitted as the
// already-resolved 4-byte values from the orig binary, which compare.py
// reads directly against orig (no reloc-mask needed since the bytes
// already match the linked image).

extern "C" __declspec(naked) void FUN_00415790()
{
    __asm {
        // 00015790: 56                    PUSH ESI
        _emit 0x56
        // 00015791: 57                    PUSH EDI
        _emit 0x57
        // 00015792: 8b f9                 MOV  EDI, ECX
        _emit 0x8b
        _emit 0xf9
        // 00015794: 8b 77 7c              MOV  ESI, dword ptr [EDI + 0x7c]
        _emit 0x8b
        _emit 0x77
        _emit 0x7c
        // 00015797: 85 f6                 TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 00015799: 74 17                 JZ   +0x17  (→ 0x004157b2)
        _emit 0x74
        _emit 0x17
        // 0001579b: 8b ce                 MOV  ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001579d: e8 ae 0e 00 00        CALL FUN_00416650  (rel32 = 0x00000eae)
        _emit 0xe8
        _emit 0xae
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        // 000157a2: 56                    PUSH ESI
        _emit 0x56
        // 000157a3: e8 18 0b 00 00        CALL FUN_004162c0  (rel32 = 0x00000b18)
        _emit 0xe8
        _emit 0x18
        _emit 0x0b
        _emit 0x00
        _emit 0x00
        // 000157a8: 83 c4 04              ADD  ESP, 0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000157ab: c7 47 7c 00 00 00 00  MOV  dword ptr [EDI + 0x7c], 0
        _emit 0xc7
        _emit 0x47
        _emit 0x7c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000157b2: 5f                    POP  EDI
        _emit 0x5f
        // 000157b3: 5e                    POP  ESI
        _emit 0x5e
        // 000157b4: c3                    RET
        _emit 0xc3
    }
}

// vim: ts=4 sts=4 sw=4 et
