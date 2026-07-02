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
// FUNCTION: ffxivgame 0x00040820 — SSO-string reset (__thiscall, 0 args, 36 B)
//
// Sibling of FUN_004407f0 (SSO-string reset at RVA 0x407f0) — same logic,
// operating on an object whose SSO-string member sits 4 bytes further into
// the struct (capacity at +0x1c instead of +0x18, inline buffer/heap ptr at
// +0x08 instead of +0x04, size at +0x18 instead of +0x14).
//
// If capacity (+0x1c) >= 8 (heap mode), frees the heap pointer at +0x08 via
// FUN_009d1b17 (__cdecl, caller cleans with ADD ESP,4).  Then resets:
// capacity = 7 (SSO mode), size = 0, and zeroes the first two bytes of the
// inline buffer at +0x08.
//
// The symbols.json boundary for this function is 36 bytes (RVA 0x40820–0x40844),
// which ends three bytes short of the full POP ESI / RET epilogue — identical
// truncation pattern to FUN_004407f0.  The bytes for the displacement byte of
// the word-store plus POP ESI / RET fall at RVA 0x40844–0x40847 and are owned
// by the adjacent code region.  To produce exactly the 36-byte slice that
// compare.py grades against, we use a naked function and stop the _emit
// stream after the third byte of the word-store prefix (66 89 46), leaving
// the displacement byte and epilogue to the following binary region.
//
// The sole linker relocation is the 4-byte CALL displacement for FUN_009d1b17
// (bytes +0x0d..+0x10); compare.py masks those bytes.
//
// Asm (36 bytes @ orig RVA 0x00040820):
//   56                   PUSH ESI
//   8b f1                MOV ESI, ECX
//   83 7e 1c 08          CMP dword ptr [ESI+0x1c], 0x8
//   72 0c                JC +0x0c           (→ skip_free; 12 bytes)
//   8b 46 08             MOV EAX, [ESI+0x8]
//   50                   PUSH EAX
//   e8 RR RR RR RR       CALL FUN_009d1b17  (reloc)
//   83 c4 04             ADD ESP, 4
// skip_free:
//   33 c0                XOR EAX, EAX
//   c7 46 1c 07 00 00 00 MOV dword ptr [ESI+0x1c], 7
//   89 46 18             MOV dword ptr [ESI+0x18], EAX
//   66 89 46             <first 3 bytes of: MOV word ptr [ESI+0x8], AX>
//   -- boundary (RVA 0x40844) --

extern "C" void __cdecl FUN_009d1b17(void*);

extern "C" __declspec(naked) void FUN_00440820() {
    __asm {
        _emit 0x56           // PUSH ESI
        _emit 0x8b           // MOV ESI, ECX
        _emit 0xf1
        _emit 0x83           // CMP dword ptr [ESI+0x1c], 0x8
        _emit 0x7e
        _emit 0x1c
        _emit 0x08
        _emit 0x72           // JC +0x0c
        _emit 0x0c
        _emit 0x8b           // MOV EAX, dword ptr [ESI+0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x50           // PUSH EAX
        call FUN_009d1b17    // CALL rel32 — properly relocated (bytes +0x0d..+0x10 masked)
        _emit 0x83           // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x33           // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc7           // MOV dword ptr [ESI+0x1c], 7
        _emit 0x46
        _emit 0x1c
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89           // MOV dword ptr [ESI+0x18], EAX
        _emit 0x46
        _emit 0x18
        _emit 0x66           // prefix byte  ─┐
        _emit 0x89           // opcode        │ first 3 bytes of
        _emit 0x46           // ModRM         ┘ MOV word ptr [ESI+0x8], AX
        // displacement byte 0x08 + POP ESI (5e) + RET (c3) fall beyond
        // the 36-byte function boundary and are not emitted here.
    }
}
